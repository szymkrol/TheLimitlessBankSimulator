//
// Created by szymon on 7.05.2026.
//

#include "server.h"
#include "communication.h"
#include "structures.h"
#include "db_manager.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <asm-generic/errno-base.h>


volatile sig_atomic_t server_running = 1;
volatile int active_threads = 0;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

void log_info(const char *msg) {
    printf("[Thread %lu] %s\n", (unsigned long) pthread_self(), msg);
}

int record_transaction(const int src, const int dest, const int amount, const int fd_trans) {
    lock_record(fd_trans, F_WRLCK, 0, sizeof(long));

    const long current_id = get_num_entries(fd_trans, 0);
    const Transaction t = {
        .transaction_id = (int) current_id,
        .source_id = src,
        .destination_id = dest,
        .amount = amount
    };

    const int res = add_entry(&t, sizeof(Transaction), fd_trans, 0);

    unlock_record(fd_trans, 0, sizeof(long));

    if (res <= 0) {
        perror("Failed to log transaction");
        return -1;
    }

    printf("[DB] Transaction logged: ID %ld, Src %d, Dest %d, Amount %d\n", current_id, src, dest, amount);
    return 1;
}

int get_balance(const long user_id, const int fd_accounts) {
    Account *ptr = (Account *) get_entry(user_id, sizeof(Account), fd_accounts, 1);
    if (!ptr) {
        perror("Database reading error");
        return -1;
    }
    const int balance = ptr->balance;
    free(ptr);
    return balance;
}

int change_balance(const long user_id, const int amount, const int fd_accounts) {
    Account *ptr = (Account *) get_entry(user_id, sizeof(Account), fd_accounts, 1);
    if (!ptr) {
        perror("Database reading error");
        return -1;
    }

    if (amount < 0 && (ptr->balance + amount < 0)) {
        log_info("Operation failed: Insufficient funds");
        free(ptr);
        return -2;
    }

    ptr->balance += amount;
    const int res = modify_entry(ptr, sizeof(Account), user_id, fd_accounts, 1);
    free(ptr);

    if (res <= 0) {
        perror("Couldn't complete operation");
        return 0;
    }
    return 1;
}

int handle_create_user(const int socket, const int fd_users, const int fd_accounts) {
    char *received_login = (char *) receive_data(socket);
    char *received_password = (char *) receive_data(socket);
    int result = -1;

    if (received_login && received_password) {
        printf("[AUTH] Attempting to create user: %s\n", received_login);
        lock_record(fd_users, F_WRLCK, 0, sizeof(long));

        const long new_id = get_num_entries(fd_users, 0);


        const User new_u = {
            .id = new_id
        };

        strncpy((char *) new_u.login, received_login, 255);
        strncpy((char *) new_u.password, received_password, 255);

        const Account new_acc = {
            .id = (int) new_id,
            .balance = 0
        };

        if (add_entry(&new_u, sizeof(User), fd_users, 0) > 0 &&
            add_entry(&new_acc, sizeof(Account), fd_accounts, 0) > 0) {
            result = 1;
            printf("[AUTH] User created successfully: %s (ID: %ld)\n", received_login, new_id);
        } else {
            log_info("Error: Failed to add user to database");
        }

        unlock_record(fd_users, 0, sizeof(long));
    }

    free(received_login);
    free(received_password);
    return send_data(socket, &result, sizeof(int));
}

int handle_balance_op(const int socket, const enum Operation op, const long user_id, const int fd_acc,
                      const int fd_trans) {
    int amount = 0;
    int result = 0;

    if (op == GET_BALANCE) {
        result = get_balance(user_id, fd_acc);
        printf("[OP] User %ld requested balance. Result: %d\n", user_id, result);
        return send_data(socket, &result, sizeof(int));
    }

    int *amount_ptr = (int *) receive_data(socket);
    if (!amount_ptr) {
        return -1;
    }
    amount = *amount_ptr;
    free(amount_ptr);
    if (amount < 0) {
        return -1;
    }

    if (op == DEPOSIT) {
        printf("[OP] User %ld depositing amount: %d\n", user_id, amount);
        result = change_balance(user_id, amount, fd_acc);
        if (result > 0) record_transaction(-1, (int) user_id, amount, fd_trans);
    } else {
        // WITHDRAW
        printf("[OP] User %ld withdrawing amount: %d\n", user_id, amount);
        result = change_balance(user_id, -amount, fd_acc);
        if (result > 0) record_transaction((int) user_id, -1, amount, fd_trans);
    }

    return send_data(socket, &result, sizeof(int));
}

long authenticate(const char *login, const char *password, const int fd_users) {
    const long user_count = get_num_entries(fd_users, 1);
    for (long i = 0; i < user_count; i++) {
        User *u = (User *) get_entry(i, sizeof(User), fd_users, 1);
        if (!u) {
            perror("Database reading error");
            return -1;
        }

        long result = -1;
        if (strcmp(u->login, login) == 0 && strcmp(u->password, password) == 0) {
            result = u->id;
        }

        free(u);
        if (result != -1) {
            return result;
        }
    }
    return -1;
}

int handle_transfer(const int socket, const long src_id, const int fd_acc, const int fd_trans) {
    long dest_id = -1;
    int amount = 0;

    long *dest_id_ptr = (long *) receive_data(socket);
    if (!dest_id_ptr) {
        return -1;
    }
    dest_id = *dest_id_ptr;
    free(dest_id_ptr);
    if (dest_id < 0) {
        return -1;
    }

    int *amount_ptr = (int *) receive_data(socket);
    if (!amount_ptr) {
        return -1;
    }
    amount = *amount_ptr;
    free(amount_ptr);

    printf("[OP] Transfer: User %ld -> User %ld, Amount: %d\n", src_id, dest_id, amount);

    const long first = (src_id < dest_id) ? src_id : dest_id;
    const long second = (src_id < dest_id) ? dest_id : src_id;

    lock_account(fd_acc, first, sizeof(Account));
    lock_account(fd_acc, second, sizeof(Account));

    Account *src_acc = get_entry(src_id, sizeof(Account), fd_acc, 0);
    Account *dest_acc = get_entry(dest_id, sizeof(Account), fd_acc, 0);

    int result = 0;
    if (!src_acc) {
        log_info("Transfer failed: Couldn't open source account");
    }
    if (!dest_acc) {
        log_info("Transfer failed: Couldn't open destination account");
    }
    if (src_acc && dest_acc && src_acc->balance >= amount) {
        src_acc->balance -= amount;
        dest_acc->balance += amount;

        modify_entry(src_acc, sizeof(Account), src_id, fd_acc, 0);
        modify_entry(dest_acc, sizeof(Account), dest_id, fd_acc, 0);

        result = 1;
        record_transaction((int) src_id, (int) dest_id, amount, fd_trans);
        printf("[OP] Transfer successful\n");
    } else {
        log_info("Transfer failed: Account opening error or insufficient funds");
        result = -1;
    }

    unlock_account(fd_acc, second, sizeof(Account));
    unlock_account(fd_acc, first, sizeof(Account));

    free(src_acc);
    free(dest_acc);

    return send_data(socket, &result, sizeof(int));
}

struct sockaddr_in config_addr() {
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);
    return address;
}

void handle_shutdown(int sig) {
    server_running = 0;
}

int run_server() {
    /*
     * Setting up signal so that Ctrl+C closes the server.
     * It does it in a way that makes the signal be captured by main process
     * and not child one and so that it works correctly with accept().
     */
    struct sigaction sa;
    sa.sa_handler = handle_shutdown;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error setting up sigaction");
        return -1;
    }

    /* Setting up network communication */
    struct sockaddr_in address = config_addr();
    const int opt = 1;
    int addrlen = sizeof(address);
    const int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0) {
        return -1;
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        close(server_fd);
        return -1;
    }

    listen(server_fd, 10);

    printf("Server works (Ctrl+C to shutdown)\n");

    while (server_running) {
        const int new_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t *) &addrlen);

        if (new_socket < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("Accept failed");
            break;
        }

        pthread_mutex_lock(&count_mutex);
        active_threads++;
        pthread_mutex_unlock(&count_mutex);

        printf("[NET] New connection accepted. Active clients: %d\n", active_threads);
        thread_args *t_args = malloc(sizeof(thread_args));
        t_args->socket_desc = new_socket;

        pthread_t thread;
        if (pthread_create(&thread, NULL, connection_handler, (void *) t_args) != 0) {
            perror("Could not create thread");
            free(t_args);
            pthread_mutex_lock(&count_mutex);
            active_threads--;
            pthread_mutex_unlock(&count_mutex);
            close(new_socket);
        } else {
            pthread_detach(thread);
        }
    }

    printf("\n[SYS] Server stopping...\n");
    close(server_fd);

    printf("[SYS] Waiting for %d active threads to finish...\n", active_threads);
    while (1) {
        pthread_mutex_lock(&count_mutex);
        const int current_count = active_threads;
        pthread_mutex_unlock(&count_mutex);

        if (current_count <= 0) {
            break;
        }
        usleep(100000);
    }

    printf("[SYS] All clients closed. Server shutdown complete.\n");
    return 0;
}

void process_session(const int socket, const int fd_users, const int fd_accounts, const int fd_transactions) {
    long auth_id = -1;
    enum Operation *op = NULL;

    while (auth_id < 0) {
        if (op) free(op);
        op = (enum Operation *) receive_data(socket);
        if (!op) {
            log_info("Client disconnected during authentication");
            return;
        }

        if (*op == CREATE_USER) {
            handle_create_user(socket, fd_users, fd_accounts);
        } else if (*op == AUTHENTICATE) {
            char *login = (char *) receive_data(socket);
            char *password = (char *) receive_data(socket);

            printf("[AUTH] Login attempt for user: %s\n", login);
            auth_id = authenticate(login, password, fd_users);

            if (auth_id >= 0) {
                printf("[AUTH] User '%s' authenticated successfully (ID: %ld)\n", login, auth_id);
            } else {
                printf("[AUTH] Authentication failed for user: %s\n", login);
            }

            free(login);
            free(password);
            send_data(socket, &auth_id, sizeof(long));
        } else {
            log_info("Security alert: Unauthorized operation attempt");
            free(op);
            return;
        }
    }

    while (1) {
        if (op) {
            free(op);
        }
        op = (enum Operation *) receive_data(socket);
        if (!op) {
            printf("[NET] User %ld disconnected\n", auth_id);
            break;
        }

        long *request_user_id = (long *) receive_data(socket);
        if (!request_user_id) {
            break;
        }

        if (*request_user_id == auth_id) {
            switch (*op) {
                case GET_BALANCE:
                case DEPOSIT:
                case WITHDRAW:
                    handle_balance_op(socket, *op, auth_id, fd_accounts, fd_transactions);
                    break;
                case TRANSFER:
                    handle_transfer(socket, auth_id, fd_accounts, fd_transactions);
                    break;
                default:
                    printf("[WARN] User %ld sent unknown operation\n", auth_id);
                    break;
            }
        } else {
            log_info("Security alert: User ID mismatch in request!");
        }
        free(request_user_id);
    }
    free(op);
}

void *connection_handler(void *args) {
    /* Masking the Ctr+C signal so that only main process could capture it */
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    thread_args *t_args = (thread_args *) args;
    const int socket = t_args->socket_desc;
    free(t_args);

    if (access("data/users.db", F_OK) == -1 ||
        access("data/accounts.db", F_OK) == -1 ||
        access("data/transactions.db", F_OK) == -1) {
        init();
    }

    const int fd_users = get_file_descriptor("users.db", O_RDWR);
    const int fd_accounts = get_file_descriptor("accounts.db", O_RDWR);
    const int fd_transactions = get_file_descriptor("transactions.db", O_RDWR);

    if (fd_users > 0 && fd_accounts > 0 && fd_transactions > 0) {
        process_session(socket, fd_users, fd_accounts, fd_transactions);
    } else {
        perror("Thread failed to open database files");
    }

    if (fd_users > 0) {
        close(fd_users);
    }
    if (fd_accounts > 0) {
        close(fd_accounts);
    }
    if (fd_transactions > 0) {
        close(fd_transactions);
    }

    close(socket);
    pthread_mutex_lock(&count_mutex);
    active_threads--;
    pthread_mutex_unlock(&count_mutex);
    return NULL;
}

int init() {
    printf("[SYS] Initializing missing database files...\n");
    for (int i = 0; i < 3; i++) {
        const char *files[] = {"users.db", "accounts.db", "transactions.db"};
        const int f = get_file_descriptor(files[i], O_CREAT | O_EXCL | O_RDWR);
        if (f != -1) {
            create_table(f);
            printf("[SYS] Created table: %s\n", files[i]);
            close(f);
        }
    }
    return 1;
}
