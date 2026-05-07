//
// Created by szymon on 7.05.2026.
//

#include "../include/client.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "communication.h"

int connect_to_server() {
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    return client_fd;
}
int send_authentication_data(const char* login, const char* password, const int fd) {
    send_data(fd, )
}

int get_balance() {
    int fd = connect_to_server();
    send_data(fd, )
}