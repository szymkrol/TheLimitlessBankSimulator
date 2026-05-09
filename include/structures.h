//
// Created by Szymon Król on 06.05.2026.
//

#ifndef THELIMITLESSBANKSIMULATOR_STRUCTURES_H
#define THELIMITLESSBANKSIMULATOR_STRUCTURES_H

#include <stdbool.h>

typedef struct {
    const int id;
    int balance;
    bool is_active;
} Account;

typedef struct {
    const long id;
    char login[256];
    char password[256];
} User;

typedef struct {
    const int transaction_id;
    const int source_id;
    const int destination_id;
    const int amount;
} Transaction;

enum Operation {
    AUTHENTICATE,
    GET_BALANCE,
    DEPOSIT,
    WITHDRAW,
    TRANSFER,
    CREATE_USER
};

#endif //THELIMITLESSBANKSIMULATOR_STRUCTURES_H
