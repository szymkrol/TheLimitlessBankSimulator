//
// Created by Szymon Król on 06.05.2026.
//

#ifndef THELIMITLESSBANKSIMULATOR_STRUCTURES_H
#define THELIMITLESSBANKSIMULATOR_STRUCTURES_H

#include <stdbool.h>

typedef struct {
    int id;
    int balance;
    bool is_active;
} Account;

typedef struct {
    long id;
    char login[256];
    char password[256];
} User;

typedef struct {
    int transaction_id;
    long source_id;
    long destination_id;
    int amount;
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
