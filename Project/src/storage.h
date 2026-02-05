#ifndef STORAGE_H
#define STORAGE_H

#include "accounts.h"
#include "transactions.h"

extern Account g_accounts[MAX_ACCOUNTS];
extern int g_count;
extern long long g_nextTxId;

void load_accounts(void);
void save_accounts(void);

void load_next_txid(void);
void append_tx(Transaction tx);

#endif
