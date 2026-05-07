//
// Created by szymon on 7.05.2026.
//

#include "communication.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int send_data(const int fd, const void *data, const ssize_t size) {
    ssize_t bytes_written = write(fd, &size, sizeof(ssize_t));
    if (bytes_written != sizeof(ssize_t)) {
        perror("Error when sending message.");
        return -1;
    }
    bytes_written = write(fd, data, size);
    if (bytes_written != size) {
        perror("Error when sending message.");
        return -1;
    }
    return 1;
}

void* receive_data(const int fd) {
    ssize_t msg_size = 0;
    ssize_t bytes_received = read(fd, &msg_size, sizeof(ssize_t));
    if (bytes_received != sizeof(ssize_t)) {
        perror("Error when receiving message.");
        return 0;
    }
    void* ptr = malloc(msg_size);
    bytes_received = read(fd, ptr, msg_size);
    if (bytes_received != sizeof(ssize_t)) {
        perror("Error when receiving message.");
        free(ptr);
        return 0;
    }
    return ptr;
}
