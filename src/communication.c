//
// Created by szymon on 7.05.2026.
//

#include "../include/communication.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int send_data(const int fd, const void *data, const ssize_t size) {
    if (write(fd, &size, sizeof(ssize_t)) != sizeof(ssize_t)) {
        perror("Error when sending message size.");
        return -1;
    }
    if (write(fd, data, size) != size) {
        perror("Error when sending message data.");
        return -1;
    }
    return 1;
}

void *receive_data(const int fd) {
    ssize_t msg_size = 0;
    if (read(fd, &msg_size, sizeof(ssize_t)) != sizeof(ssize_t)) {
        perror("Error when receiving message size.");
        return NULL;
    }

    void *ptr = malloc(msg_size);
    if (!ptr) {
        perror("Memory allocation failed during receive.");
        return NULL;
    }

    if (read(fd, ptr, msg_size) != msg_size) {
        perror("Error when receiving message data.");
        free(ptr);
        return NULL;
    }

    return ptr;
}
