//
// Created by Szymon Król on 06.05.2026.
//

#include <stdio.h>
#include <string.h>
#include "server.h"
#include "../include/client.h"

int main(const int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [server|client]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "server") == 0) {
        init();
        return run_server();
    } else if (strcmp(argv[1], "client") == 0) {
        return run_client();
    } else {
        fprintf(stderr, "Unknown argument: %s\n", argv[1]);
        return 1;
    }
}