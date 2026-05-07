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
    char* path = malloc(sizeof(char) * (strlen(path_begin) + strlen(table_name) + 1));
    snprintf(path, sizeof(char) * (strlen(path_begin) + strlen(table_name) + 1), "%s%s", path_begin, table_name);
    const int f = open(path, flag, 0644);
    free(path);
    return f;
}

int create_table(int f) {
    // const int f = get_file_descriptor(table_name, O_CREAT | O_WRONLY);
    if (f == -1) {
        perror("Error creating file");
        // close(f);
        return 2;
    }

    const long entry_count = 0;

    struct flock lock = {0};
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = sizeof(long);

    if (fcntl(f, F_SETLKW, &lock) == -1) {
        perror("Locking failed");
        return 4;
    }
    pwrite(f, &entry_count, sizeof(long), 0);
    lock.l_type = F_UNLCK;
    fcntl(f, F_SETLK, &lock);
    // close(f);
    return 1;
}

long get_num_entries(const int f) {
    // const int f = get_file_descriptor(table_name, O_CREAT | O_WRONLY);

    long entry_count = 0;
    struct flock lock = {0};
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = sizeof(long);
    if (fcntl(f, F_SETLKW, &lock) == -1) {
        perror("Locking failed");
        return 4;
    }
    pread(f, &entry_count, sizeof(long), 0);
    lock.l_type = F_UNLCK;
    fcntl(f, F_SETLK, &lock);
    // close(f);
    return entry_count;
}

int add_entry(const void* structure, const int entry_size, const int f) {
    // Entries are stored in such a way that in the beginning there is a long signifying current entry count.

    // const int f = get_file_descriptor(table_name, O_RDWR);
    // if (f == -1) {
    //     perror("Error opening file");
    //     close(f);
    //     return 2;
    // }

    long entry_count;
    struct flock lock = {0};
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = sizeof(long);
    if (fcntl(f, F_SETLKW, &lock) == -1) {
        perror("Locking failed");
        return 4;
    }

    const ssize_t bytes_read = pread(f, &entry_count, sizeof(long), 0);

    if (bytes_read != sizeof(long)) {
        perror("Couldn't read entry count because reached EOF before");
        lock.l_type = F_UNLCK;
        fcntl(f, F_SETLK, &lock);
        // close(f);
        return 3;
    }

    pwrite(f, structure, entry_size, entry_count * entry_size + sizeof(long));
    ++entry_count;
    pwrite(f, &entry_count, sizeof(long), 0);

    lock.l_type = F_UNLCK;
    fcntl(f, F_SETLK, &lock);
    // close(f);
    return 1;
}

void* get_entry(const int index, const int entry_size, const int f) {
    // int f = get_file_descriptor(table_name, O_RDONLY);
    // if (f == -1) {
    //     perror("Error opening file");
    //     close(f);
    //     return 0;
    // }
    struct flock lock = {0};
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = sizeof(long) + index * entry_size;
    lock.l_len = entry_size;
    if (fcntl(f, F_SETLKW, &lock) == -1) {
        perror("Locking failed");
        return 0;
    }

    void* ptr = malloc(entry_size);
    pread(f, ptr, entry_size, sizeof(long) + index * entry_size);

    lock.l_type = F_UNLCK;
    fcntl(f, F_SETLK, &lock);
    // close(f);

    return ptr;
}

int modify_entry(const void *structure, const int entry_size, const int index, const int f) {
    // const int f = get_file_descriptor(table_name, O_RDWR);
    // if (f == -1) {
    //     perror("Error opening file");
    //     close(f);
    //     return 2;
    // }

    struct flock lock = {0};
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = sizeof(long) + index * entry_size;
    lock.l_len = entry_size;
    if (fcntl(f, F_SETLKW, &lock) == -1) {
        perror("Locking failed");
        return 4;
    }

    pwrite(f, structure, entry_size, sizeof(long) + index * entry_size);

    lock.l_type = F_UNLCK;
    fcntl(f, F_SETLK, &lock);
    // close(f);
    return 1;
}

