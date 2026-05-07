//
// Created by szymon on 7.05.2026.
//

#ifndef THELIMITLESSBANKSIMULATOR_SERVER_H
#define THELIMITLESSBANKSIMULATOR_SERVER_H

int authenticate(const char* login, const char* password);
int perform_transaction(int source_id, int dest_id, int amount);
#endif //THELIMITLESSBANKSIMULATOR_SERVER_H