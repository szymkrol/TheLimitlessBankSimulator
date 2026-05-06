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

int create_table(const char *table_name) {
    const int f = get_file_descriptor(table_name, O_CREAT | O_WRONLY);
    if (f == -1) {
        perror("Error creating file");
        close(f);
        return 2;
    }

    const long entry_count = 0;
    write(f, &entry_count, sizeof(long));
    close(f);
    return 1;
}

long get_num_entries(const char *table_name) {
    const int f = get_file_descriptor(table_name, O_CREAT | O_WRONLY);
    if (f == -1) {
        perror("Error creating file");
        close(f);
        return 2;
    }

    long entry_count = 0;
    read(f, &entry_count, sizeof(long));
    close(f);
    return entry_count;
}

int add_entry(const char* table_name, const void* structure, const int entry_size) {
    // Entries are stored in such a way that in the beginning there is a long signifying current entry count.
    const int f = get_file_descriptor(table_name, O_RDWR);
    if (f == -1) {
        perror("Error opening file");
        close(f);
        return 2;
    }

    long entry_count;
    const ssize_t bytes_read = read(f, &entry_count, sizeof(long));
    if (bytes_read != sizeof(long)) {
        perror("Couldn't read entry count because reached EOF before");
        close(f);
        return 3;
    }

    lseek(f, entry_count * entry_size, SEEK_CUR);
    write(f, structure, entry_size);
    ++entry_count;

    lseek(f, 0, SEEK_SET);
    write(f, &entry_count, sizeof(long));
    close(f);
    return 1;
}

void* get_entry(const char *table_name, const int index, const int entry_size) {
    int f = get_file_descriptor(table_name, O_RDONLY);
    if (f == -1) {
        perror("Error opening file");
        close(f);
        return 0;
    }
    lseek(f, sizeof(long) + index * entry_size, SEEK_SET);
    void* ptr = malloc(entry_size);
    read(f, ptr, entry_size);
    return ptr;
}

