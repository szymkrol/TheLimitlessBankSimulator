//
// Created by szymon on 7.05.2026.
//

#include "../include/server.h"
#include "../include/structures.h"
#include "../include/db_manager.h"


#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include "communication.h"

int record_transaction(const int src, const int dest, const int amount, const int fd_trans) {
    const long current_id = get_num_entries(fd_trans);

    const Transaction t = {
        .transaction_id = (int) current_id,
        .source_id = src,
        .destination_id = dest,
        .amount = amount
    };

    if (add_entry(&t, sizeof(Transaction), fd_trans) <= 0) {
        perror("Failed to log transaction");
        return -1;
    }
    return 1;
}

int get_balance(const long user_id, const int fd_accounts) {
    Account *ptr = (Account *) get_entry(user_id, sizeof(Account), fd_accounts);
    if (!ptr) {
        perror("Database reading error");
        return -1;
    }
    const int balance = ptr->balance;
    free(ptr);
    return balance;
}

int change_balance(const long user_id, const int amount, const int fd_accounts) {
    Account *ptr = (Account *) get_entry(user_id, sizeof(Account), fd_accounts);
    if (!ptr) {
        perror("Database reading error");
        return -1;
    }

    if (amount < 0 && (ptr->balance + amount < 0)) {
        perror("Not enough funds");
        free(ptr);
        return -2;
    }

    ptr->balance += amount;
    int res = modify_entry(ptr, sizeof(Account), user_id, fd_accounts);
    free(ptr);

    if (res <= 0) {
        perror("Couldn't complete operation");
        return 0;
    }
    return 1;
}

int handle_balance_op(const int socket, const enum Operation op, const long user_id, const int fd_acc,
                      const int fd_trans) {
    int amount = 0;
    int result = 0;

    if (op == GET_BALANCE) {
        result = get_balance(user_id, fd_acc);
        return send_data(socket, &result, sizeof(int));
    }

    if (read(socket, &amount, sizeof(int)) <= 0 || amount < 0) return -1;

    if (op == DEPOSIT) {
        result = change_balance(user_id, amount, fd_acc);
        if (result > 0) record_transaction(-1, (int) user_id, amount, fd_trans);
    } else {
        // WITHDRAW
        result = change_balance(user_id, -amount, fd_acc);
        if (result > 0) record_transaction((int) user_id, -1, amount, fd_trans);
    }

    return send_data(socket, &result, sizeof(int));
}

long authenticate(const char *login, const char *password, const int fd_users) {
    const long user_count = get_num_entries(fd_users);
    for (long i = 0; i < user_count; i++) {
        User *u = (User *) get_entry(i, sizeof(User), fd_users);
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

    if (read(socket, &dest_id, sizeof(long)) <= 0 || read(socket, &amount, sizeof(int)) <= 0 || amount < 0) {
        return -1;
    }

    int result = change_balance(src_id, -amount, fd_acc);
    if (result > 0) {
        result = change_balance(dest_id, amount, fd_acc);
        if (result > 0) {
            record_transaction((int) src_id, (int) dest_id, amount, fd_trans);
        } else {
            change_balance(src_id, amount, fd_acc);
            perror("Transfer failed at destination, rolled back.");
        }
    }

    return send_data(socket, &result, sizeof(int));
}

struct sockaddr_in config_addr() {
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);
    return address;
}

int main() {
    struct sockaddr_in address = config_addr();
    const int opt = 1;
    int addrlen = sizeof(address);
    const int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    bind(server_fd, (struct sockaddr *) &address, sizeof(address));
    listen(server_fd, 10);

    while (1) {
        const int new_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t *) &addrlen);

        thread_args *t_args = malloc(sizeof(thread_args));
        t_args->socket_desc = new_socket;
        // NOTE: t_args->fd_users, fd_accounts, etc., should be populated here
        // before passing to the thread depending on how you manage global FDs.

        pthread_t thread;
        if (pthread_create(&thread, NULL, connection_handler, (void *) t_args) != 0) {
            perror("Could not create thread");
            free(t_args);
            close(new_socket);
        } else {
            pthread_detach(thread);
        }
    }
    return 0;
}

void process_session(const int socket, const int fd_users, const int fd_accounts, const int fd_transactions) {
    enum Operation op;
    char login[256] = {0};
    char password[256] = {0};

    if (read(socket, &op, sizeof(enum Operation)) <= 0 || op != AUTHENTICATE) {
        return;
    }

    if (read(socket, login, 255) <= 0 || read(socket, password, 255) <= 0) {
        return;
    }

    const long auth_id = authenticate(login, password, fd_users);
    if (send_data(socket, &auth_id, sizeof(long)) <= 0 || auth_id < 0) {
        return;
    }

    if (read(socket, &op, sizeof(enum Operation)) <= 0) {
        return;
    }

    long request_user_id = -1;
    if (read(socket, &request_user_id, sizeof(long)) <= 0 || request_user_id != auth_id) {
        int err = -1;
        send_data(socket, &err, sizeof(int));
        return;
    }

    switch (op) {
        case GET_BALANCE:
        case DEPOSIT:
        case WITHDRAW:
            handle_balance_op(socket, op, auth_id, fd_accounts, fd_transactions);
            break;
        case TRANSFER:
            handle_transfer(socket, auth_id, fd_accounts, fd_transactions);
            break;
        default:
            perror("Unsupported operation");
            break;
    }
}

void *connection_handler(void *args) {
    thread_args *t_args = (thread_args *) args;

    const int socket = t_args->socket_desc;
    const int fd_users = t_args->fd_users;
    const int fd_accounts = t_args->fd_accounts;
    const int fd_transactions = t_args->fd_transactions;
    free(t_args);

    process_session(socket, fd_users, fd_accounts, fd_transactions);

    close(socket);
    return NULL;
}

int init() {
    const char *files[] = {"users.db", "accounts.db", "transactions.db"};
    for (int i = 0; i < 3; i++) {
        int f = get_file_descriptor(files[i], O_CREAT | O_WRONLY);
        if (f != -1) {
            create_table(f);
            close(f);
        }
    }
    return 1;
}
