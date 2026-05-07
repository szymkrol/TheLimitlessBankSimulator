//
// Created by Szymon Król on 06.05.2026.
//
#include "db_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int get_file_descriptor(const char *table_name, const int flag) {
    char path_begin[] = "data/";
    char *path = malloc(sizeof(char) * (strlen(path_begin) + strlen(table_name) + 1));
    snprintf(path, sizeof(char) * (strlen(path_begin) + strlen(table_name) + 1), "%s%s", path_begin, table_name);
    const int f = open(path, flag, 0644);
    free(path);
    return f;
}

int create_table(int f) {
    if (f == -1) {
        perror("Invalid file descriptor");
        return -2;
    }

    const long entry_count = 0;
    struct flock lock = {0};
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(long);

    if (fcntl(f, F_SETLKW, &lock) == -1) {
        perror("Locking failed");
        return -4;
    }

    pwrite(f, &entry_count, sizeof(long), 0);

    lock.l_type = F_UNLCK;
    fcntl(f, F_SETLK, &lock);
    return 1;
}

long get_num_entries(const int f) {
    long entry_count = 0;
    struct flock lock = {0};
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(long);

    if (fcntl(f, F_SETLKW, &lock) == -1) {
        perror("Locking failed");
        return -4;
    }

    pread(f, &entry_count, sizeof(long), 0);

    lock.l_type = F_UNLCK;
    fcntl(f, F_SETLK, &lock);
    return entry_count;
}

int add_entry(const void *structure, const int entry_size, const int f) {
    long entry_count;
    struct flock lock = {0};
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_len = sizeof(long);

    if (fcntl(f, F_SETLKW, &lock) == -1) {
        perror("Locking failed");
        return -4;
    }

    if (pread(f, &entry_count, sizeof(long), 0) != sizeof(long)) {
        perror("Couldn't read entry count / EOF reached");
        lock.l_type = F_UNLCK;
        fcntl(f, F_SETLK, &lock);
        return -3;
    }

    pwrite(f, structure, entry_size, sizeof(long) + (entry_count * entry_size));
    ++entry_count;
    pwrite(f, &entry_count, sizeof(long), 0);

    lock.l_type = F_UNLCK;
    fcntl(f, F_SETLK, &lock);
    return 1;
}

void *get_entry(const long index, const int entry_size, const int f) {
    struct flock lock = {0};
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = sizeof(long) + (index * entry_size);
    lock.l_len = entry_size;

    if (fcntl(f, F_SETLKW, &lock) == -1) {
        perror("Locking failed");
        return NULL;
    }

    void *ptr = malloc(entry_size);
    if (ptr) {
        pread(f, ptr, entry_size, sizeof(long) + (index * entry_size));
    }

    lock.l_type = F_UNLCK;
    fcntl(f, F_SETLK, &lock);
    return ptr;
}

int modify_entry(const void *structure, const int entry_size, const long index, const int f) {
    struct flock lock = {0};
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = sizeof(long) + (index * entry_size);
    lock.l_len = entry_size;

    if (fcntl(f, F_SETLKW, &lock) == -1) {
        perror("Locking failed");
        return -4;
    }

    pwrite(f, structure, entry_size, sizeof(long) + (index * entry_size));

    lock.l_type = F_UNLCK;
    fcntl(f, F_SETLK, &lock);
    return 1;
}

