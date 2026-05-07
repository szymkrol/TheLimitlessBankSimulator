//
// Created by szymon on 7.05.2026.
//

#ifndef THELIMITLESSBANKSIMULATOR_SERVER_H
#define THELIMITLESSBANKSIMULATOR_SERVER_H
typedef struct {
    int socket_desc;
} thread_args;
int init();
struct sockaddr_in config_addr();
void* connection_handler(void *args);
int authenticate(const char* login, const char* password);
int perform_transaction(int source_id, int dest_id, int amount);
#endif //THELIMITLESSBANKSIMULATOR_SERVER_H