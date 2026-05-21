//
// Created by szymon on 7.05.2026.
//

#ifndef THELIMITLESSBANKSIMULATOR_SERVER_H
#define THELIMITLESSBANKSIMULATOR_SERVER_H
#include <signal.h>
#include "structures.h"

extern volatile sig_atomic_t server_running;
extern volatile int active_threads;
extern pthread_mutex_t count_mutex;

typedef struct {
    int socket_desc;
} thread_args;

void log_info(const char *msg);

int record_transaction(long src, long dest, int amount, int fd_trans);

int get_balance(long user_id, int fd_accounts);

int change_balance(long user_id, int amount, int fd_accounts);

int handle_create_user(int socket, int fd_users, int fd_accounts);

int handle_balance_op(int socket, enum Operation op, long user_id, int fd_acc,
                      int fd_trans);

struct sockaddr_in config_addr();

void *connection_handler(void *args);

long authenticate(const char *login, const char *password, int fd_users);

int handle_transfer(int socket, long src_id, int fd_acc, int fd_trans);

void handle_shutdown(int sig);

int run_server();

void process_session(int socket, int fd_users, int fd_accounts, int fd_transactions);

int init();

#endif //THELIMITLESSBANKSIMULATOR_SERVER_H
