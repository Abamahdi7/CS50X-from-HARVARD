#ifndef ACCOUNTS_H
#define ACCOUNTS_H

#include "utils.h"
#include "config.h"

typedef enum {
    ACC_SAVINGS = 1,
    ACC_CURRENT = 2
} AccountType;

typedef struct {
    int accountNumber;
    char name[MAX_NAME];
    char phone[MAX_PHONE];
    AccountType type;
    double balance;

    int isActive;

    DateTime createdAt;
    DateTime updatedAt;

    int lastWDYear, lastWDMonth, lastWDDay;
    double dailyWithdrawn;
} Account;

const char* acc_type_name(AccountType t);

#endif
