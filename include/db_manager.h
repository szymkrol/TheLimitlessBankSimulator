//
// Created by Szymon Król on 06.05.2026.
//

#ifndef THELIMITLESSBANKSIMULATOR_DB_MANAGER_H
#define THELIMITLESSBANKSIMULATOR_DB_MANAGER_H
#include <stdio.h>

int lock_record(int f, short type, long offset, long len);

int unlock_record(int f, long offset, long len);

int get_file_descriptor(const char *table_name, int flag);

int create_table(int f);

long get_num_entries(int f, int use_lock);

int add_entry(const void *structure, int entry_size, int f, int use_lock);

void *get_entry(long index, int entry_size, int f, int use_lock);

int modify_entry(const void *structure, int entry_size, long index, int f, int use_lock);

int lock_account(int fd, long user_id, ssize_t account_size);

int unlock_account(int fd, long user_id, ssize_t account_size);
#endif //THELIMITLESSBANKSIMULATOR_DB_MANAGER_H
