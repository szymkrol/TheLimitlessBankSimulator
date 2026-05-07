//
// Created by szymon on 7.05.2026.
//

#include "../include/server.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include "db_manager.h"

void* connection_handler(void *args) {
    const int socket = ((thread_args*)args)->socket_desc;
    free(args);

    char buffer[1024] = {0};

    int val_read = read(socket, buffer, 1024);
    if (val_read > 0) {
        printf("Thread [%lu] handling request: %s\n", pthread_self(), buffer);

        // 2. Perform Bank Logic (Requirement 11 - would need Mutexes here)
        char *message = "Transaction Processed Successfully\n";
        write(socket, message, strlen(message));
    }

    printf("Thread [%lu] finishing. Closing socket.\n", pthread_self());
    close(socket);
    return NULL;
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
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 10);

    while (1) {
        int new_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t *) &addrlen);

        thread_args *t_args = malloc(sizeof(thread_args));
        t_args->socket_desc = new_socket;

        pthread_t thread;
        if (pthread_create(&thread, NULL, connection_handler, (void*)t_args) > 0) {
            perror("Could not create thread");
            free(t_args);
            return 1;
        }
        pthread_detach(thread);
    }

    return 0;
}

int init() {
    int f = get_file_descriptor("users.db", O_CREAT | O_WRONLY);
    create_table(f);
    close(f);
    f = get_file_descriptor("accounts.db", O_CREAT | O_WRONLY);
    create_table(f);
    close(f);
    f = get_file_descriptor("transactions.db", O_CREAT | O_WRONLY);
    create_table(f);
    close(f);
    return 1;
}
