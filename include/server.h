//
// Created by szymon on 7.05.2026.
//

#ifndef THELIMITLESSBANKSIMULATOR_SERVER_H
#define THELIMITLESSBANKSIMULATOR_SERVER_H
#include "structures.h"
typedef struct {
    int socket_desc;
    int fd_users;
    int fd_accounts;
    int fd_transactions;
} thread_args;

int record_transaction(int src, int dest, int amount, int fd_trans);

int init();

int get_balance(long user_id, int fd_accounts);

int change_balance(long user_id, int amount, int fd_accounts);

int handle_balance_op(int socket, enum Operation op, long user_id, int fd_acc,
                      int fd_trans);

struct sockaddr_in config_addr();

void *connection_handler(void *args);

long authenticate(const char *login, const char *password, int fd_users);

int handle_transfer(int socket, long src_id, int fd_acc, int fd_trans);

void process_session(int socket, int fd_users, int fd_accounts, int fd_transactions);

int perform_transaction(int source_id, int dest_id, int amount);
#endif //THELIMITLESSBANKSIMULATOR_SERVER_H
