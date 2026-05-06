//
// Created by Szymon Król on 06.05.2026.
//
#include "db_manager.h"

int add_entry(const char* base_name, void* structure, int size) {
    char* path = malloc(strlen(base_name) + 1);
    int fd = open("data/accounts.bin", O_RDWR);
    if (fd == -1) {
        perror("Error opening file");
        return -1;
    }
}

