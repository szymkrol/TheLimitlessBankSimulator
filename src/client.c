//
// Created by szymon on 7.05.2026.
//

#include "../include/client.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "structures.h"
#include "../include/communication.h"

int connect_to_server() {
    const int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        return -1;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        return -1;
    }

    if (connect(client_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        close(client_fd);
        return -1;
    }
    return client_fd;
}

int perform_create_user(const char *login, const char *password, const int fd) {
    enum Operation msg_type = CREATE_USER;

    if (strlen(login) > 255 || strlen(password) > 255) {
        fprintf(stderr, "Credentials too long\n");
        return -1;
    }

    send_data(fd, (void *) &msg_type, sizeof(enum Operation));
    send_data(fd, login, (strlen(login) + 1));
    send_data(fd, password, (strlen(password) + 1));

    void *res = receive_data(fd);
    if (!res) return -1;

    const int status = *(int *) res;
    free(res);
    return status;
}

long send_authentication_data(const char *login, const char *password, const int fd) {
    enum Operation msg_type = AUTHENTICATE;

    if (strlen(login) > 255 || strlen(password) > 255) {
        fprintf(stderr, "Login and password cannot have more than 255 characters\n");
        return -1;
    }

    send_data(fd, (void *) &msg_type, sizeof(enum Operation));

    send_data(fd, login, (strlen(login) + 1));
    send_data(fd, password, (strlen(password) + 1));

    void *res = receive_data(fd);
    if (!res) {
        return -1;
    }

    const long user_id = *(long *) res;
    free(res);
    return user_id;
}

static void prepare_op(const int fd, const enum Operation op, const long user_id) {
    send_data(fd, &op, sizeof(enum Operation));
    send_data(fd, &user_id, sizeof(long));
}

int perform_balance_op(const int fd, const enum Operation op, const long user_id, const int amount) {
    prepare_op(fd, op, user_id);
    send_data(fd, &amount, sizeof(int));

    void *res = receive_data(fd);
    const int status = *(int *) res;
    free(res);
    return status;
}

int perform_transfer(const int fd, const long src_id, const long dest_id, const int amount) {
    const enum Operation op = TRANSFER;
    prepare_op(fd, op, src_id);

    send_data(fd, &dest_id, sizeof(long));
    send_data(fd, &amount, sizeof(int));

    void *res = receive_data(fd);
    const int status = *(int *) res;
    free(res);
    return status;
}

int run_client() {
    const int fd = connect_to_server();
    if (fd < 0) {
        perror("Could not connect to server");
        return -1;
    }

    char login[256], password[256];
    long user_id = -1;
    int choice;

    printf("Welcome to The Limitless Bank\n");
    while (user_id < 0) {
        printf("\n1. Login\n2. Create Account\n0. Exit\nChoice: ");
        if (scanf("%d", &choice) != 1 || choice == 0) {
            close(fd);
            return 0;
        }

        if (choice == 1 || choice == 2) {
            printf("Login: ");
            scanf("%255s", login);
            printf("Password: ");
            scanf("%255s", password);

            if (choice == 1) {
                user_id = send_authentication_data(login, password, fd);
                if (user_id < 0) printf("Authentication failed.\n");
            } else {
                const int res = perform_create_user(login, password, fd);
                if (res > 0) {
                    printf("Account created successfully! You can now login.\n");
                } else {
                    printf("Account creation failed (Login might be taken).\n");
                }
            }
        }
    }

    printf("Logged in! User ID: %ld\n", user_id);
    while (1) {
        printf("\n1. Balance\n2. Deposit\n3. Withdraw\n4. Transfer\n0. Exit\nChoice: ");
        if (scanf("%d", &choice) != 1 || choice == 0) {
            break;
        }

        int amount, status;
        long dest_id;

        switch (choice) {
            case 1: // Get Balance
                prepare_op(fd, GET_BALANCE, user_id);
                void *res = receive_data(fd);
                if (res) {
                    printf("Current Balance: %d\n", *(int *) res);
                    free(res);
                }
                break;
            case 2: // Deposit
                printf("Amount to deposit: ");
                scanf("%d", &amount);
                status = perform_balance_op(fd, DEPOSIT, user_id, amount);
                printf(status > 0 ? "Success\n" : "Error\n");
                break;
            case 3: // Withdraw
                printf("Amount to withdraw: ");
                scanf("%d", &amount);
                status = perform_balance_op(fd, WITHDRAW, user_id, amount);
                printf(status > 0 ? "Success\n" : "Insufficient funds/Error\n");
                break;
            case 4: // Transfer
                printf("Destination User ID: ");
                scanf("%ld", &dest_id);
                printf("Amount: ");
                scanf("%d", &amount);
                status = perform_transfer(fd, user_id, dest_id, amount);
                printf(status > 0 ? "Transfer Complete\n" : "Transfer Failed\n");
                break;
            default:
                printf("Wrong operation chosen.\n");
                break;
        }
    }

    close(fd);
    return 0;
}
