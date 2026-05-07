//
// Created by szymon on 7.05.2026.
//

#ifndef THELIMITLESSBANKSIMULATOR_COMMUNICATION_H
#define THELIMITLESSBANKSIMULATOR_COMMUNICATION_H
#include <stdio.h>

int send_data(int fd, const void* data, ssize_t size);
void* receive_data(int fd);

#endif //THELIMITLESSBANKSIMULATOR_COMMUNICATION_H