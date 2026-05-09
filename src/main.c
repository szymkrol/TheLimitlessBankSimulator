#include <stdio.h>
#include <string.h>

#include "db_manager.h"
#include "server.h"

void run_client(void);

//
// Created by Szymon Król on 06.05.2026.
//
int main(const int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [server|client]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "server") == 0) {
        return run_server();
    } else if (strcmp(argv[1], "client") == 0) {
        run_client();
    } else {
        fprintf(stderr, "Unknown argument: %s\n", argv[1]);
        return 1;
    }

    return 0;
}
