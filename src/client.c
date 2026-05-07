//
// Created by szymon on 7.05.2026.
//

#include "../include/client.h"

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "structures.h"
#include "../include/communication.h"

int connect_to_server() {
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    connect(client_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    return client_fd;
}

long send_authentication_data(const char *login, const char *password, const int fd) {
    enum Operation msg_type = AUTHENTICATE;
    if (strlen(login) > 255 || strlen(password)) {
        perror("Login and password cannot have more then 255 characters");
        return -1;
    }
    send_data(fd, (void *) &msg_type, sizeof(enum Operation));

    send_data(fd, login, sizeof(char) * strlen(login));
    send_data(fd, password, sizeof(char) * strlen(password));
    void *res = receive_data(fd);
    return *(long *) res;
}

int get_balance(const char *login, const char *password) {
    int fd = connect_to_server();
    long user_id = send_authentication_data(login, password, fd);
    if (user_id == -1) {
        perror("Authentication failed");
        close(fd);
        return -1;
    }
    enum Operation msg_type = GET_BALANCE;
    send_data(fd, (void *) &msg_type, sizeof(enum Operation));
    send_data(fd, (void *) &user_id, sizeof(long));
    int balance = *(int *) receive_data(fd);
    return balance;
}
