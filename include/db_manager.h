//
// Created by Szymon Król on 06.05.2026.
//

#ifndef THELIMITLESSBANKSIMULATOR_DB_MANAGER_H
#define THELIMITLESSBANKSIMULATOR_DB_MANAGER_H
int get_file_descriptor(const char* table_name, int flag);
int create_table(const char* table_name);
long get_num_entries(const char* table_name);
int add_entry(const char* table_name, const void* structure, int entry_size);
void* get_entry(const char* table_name, int index, int entry_size);
#endif //THELIMITLESSBANKSIMULATOR_DB_MANAGER_H
