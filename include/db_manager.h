//
// Created by Szymon Król on 06.05.2026.
//

#ifndef THELIMITLESSBANKSIMULATOR_DB_MANAGER_H
#define THELIMITLESSBANKSIMULATOR_DB_MANAGER_H
int add_entry(const char* base_name, void* structure, int size);
void* get_entry(const char* base_name, int index);
int get_num_entries(const char* base_name);
#endif //THELIMITLESSBANKSIMULATOR_DB_MANAGER_H
