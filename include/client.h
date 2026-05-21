//
// Created by szymon on 7.05.2026.
//

#ifndef THELIMITLESSBANKSIMULATOR_CLIENT_H
#define THELIMITLESSBANKSIMULATOR_CLIENT_H
#include "structures.h"

int connect_to_server();

int perform_create_user(const char *login, const char *password, int fd);

long send_authentication_data(const char *login, const char *password, int fd);

int perform_balance_op(int fd, enum Operation op, long user_id, int amount);

int perform_transfer(int fd, long src_id, long dest_id, int amount);

int run_client();

#endif //THELIMITLESSBANKSIMULATOR_CLIENT_H
