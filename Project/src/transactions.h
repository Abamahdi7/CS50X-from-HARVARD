#ifndef TRANSACTIONS_H
#define TRANSACTIONS_H

#include "utils.h"

typedef enum {
    TX_DEPOSIT = 1,
    TX_WITHDRAW = 2,
    TX_TRANSFER_IN = 3,
    TX_TRANSFER_OUT = 4,
    TX_CREATE = 5,
    TX_EDIT = 6,
    TX_DELETE = 7,
    TX_INTEREST = 8
} TxType;

typedef struct {
    long long txId;
    int accountNumber;
    TxType type;
    double amount;
    int relatedAccount;
    double balanceAfter;
    DateTime when;
    char note[120];
} Transaction;

const char* tx_type_name(TxType t);

#endif
