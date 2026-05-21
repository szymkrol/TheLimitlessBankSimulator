//
// Created by Szymon Król on 06.05.2026.
//

#define _GNU_SOURCE
#include "db_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int lock_record(const int f, const short type, const long offset, const long len) {
    struct flock lock = {0};
    lock.l_type = type;
    lock.l_whence = SEEK_SET;
    lock.l_start = offset;
    lock.l_len = len;

    if (fcntl(f, F_OFD_SETLKW, &lock) == -1) {
        perror("Locking failed");
        return -1;
    }
    return 0;
}

int unlock_record(const int f, const long offset, const long len) {
    return lock_record(f, F_UNLCK, offset, len);
}

int get_file_descriptor(const char *table_name, const int flag) {
    char path_begin[] = "data/";
    const size_t path_len = strlen(path_begin) + strlen(table_name) + 1;
    char *path = malloc(path_len);
    if (!path) return -1;

    snprintf(path, path_len, "%s%s", path_begin, table_name);
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
    lock_record(f, F_WRLCK, 0, sizeof(long));
    pwrite(f, &entry_count, sizeof(long), 0);
    unlock_record(f, 0, sizeof(long));
    return 1;
}

long get_num_entries(const int f, const int use_lock) {
    long entry_count = 0;
    if (use_lock) {
        lock_record(f, F_RDLCK, 0, sizeof(long));
    }

    if (pread(f, &entry_count, sizeof(long), 0) != sizeof(long)) {
        entry_count = 0;
    }
    if (use_lock) {
        unlock_record(f, 0, sizeof(long));
    }
    return entry_count;
}

int add_entry(const void *structure, const int entry_size, const int f, const int use_lock) {
    long entry_count;
    if (use_lock) {
        lock_record(f, F_WRLCK, 0, sizeof(long));
    }

    if (pread(f, &entry_count, sizeof(long), 0) != sizeof(long)) {
        if (use_lock) {
            unlock_record(f, 0, sizeof(long));
        }
        return -3;
    }

    pwrite(f, structure, entry_size, sizeof(long) + (entry_count * entry_size));
    ++entry_count;
    pwrite(f, &entry_count, sizeof(long), 0);

    if (use_lock) {
        unlock_record(f, 0, sizeof(long));
    }
    return 1;
}

void *get_entry(const long index, const int entry_size, const int f, const int use_lock) {
    const long offset = sizeof(long) + (index * entry_size);

    if (use_lock) {
        lock_record(f, F_RDLCK, offset, entry_size);
    }

    void *ptr = malloc(entry_size);
    if (ptr) {
        if (pread(f, ptr, entry_size, offset) != entry_size) {
            free(ptr);
            ptr = NULL;
        }
    }

    if (use_lock) {
        unlock_record(f, offset, entry_size);
    }
    return ptr;
}

int modify_entry(const void *structure, const int entry_size, const long index, const int f, const int use_lock) {
    const long offset = sizeof(long) + (index * entry_size);

    if (use_lock) {
        lock_record(f, F_WRLCK, offset, entry_size);
    }

    const ssize_t written = pwrite(f, structure, entry_size, offset);

    if (use_lock) {
        unlock_record(f, offset, entry_size);
    }

    return (written == entry_size) ? 1 : -1;
}

int lock_account(const int fd, const long user_id, const ssize_t account_size) {
    return lock_record(fd, F_WRLCK, sizeof(long) + (user_id * account_size), account_size);
}

int unlock_account(const int fd, const long user_id, const ssize_t account_size) {
    return unlock_record(fd, sizeof(long) + (user_id * account_size), account_size);
}
