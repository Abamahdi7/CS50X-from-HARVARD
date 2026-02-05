#include "storage.h"
#include "config.h"
#include <stdio.h>

Account g_accounts[MAX_ACCOUNTS];
int g_count = 0;
long long g_nextTxId = 1;

void save_accounts(void) {
    FILE *fp = fopen(ACC_FILE, "wb");
    if (!fp) {
        printf("ERROR: cannot write %s\n", ACC_FILE);
        return;
    }
    fwrite(&g_count, sizeof(int), 1, fp);
    fwrite(g_accounts, sizeof(Account), (size_t)g_count, fp);
    fclose(fp);
}

void load_accounts(void) {
    FILE *fp = fopen(ACC_FILE, "rb");
    if (!fp) {
        g_count = 0;
        return;
    }
    int count = 0;
    if (fread(&count, sizeof(int), 1, fp) != 1) {
        fclose(fp);
        g_count = 0;
        return;
    }
    if (count < 0 || count > MAX_ACCOUNTS) {
        fclose(fp);
        g_count = 0;
        return;
    }
    int got = (int)fread(g_accounts, sizeof(Account), (size_t)count, fp);
    fclose(fp);
    if (got != count) {
        g_count = 0;
        return;
    }
    g_count = count;
}

void append_tx(Transaction tx) {
    FILE *fp = fopen(TX_FILE, "ab");
    if (!fp) {
        printf("WARNING: cannot append tx to %s\n", TX_FILE);
        return;
    }
    fwrite(&tx, sizeof(Transaction), 1, fp);
    fclose(fp);
}

void load_next_txid(void) {
    FILE *fp = fopen(TX_FILE, "rb");
    if (!fp) {
        g_nextTxId = 1;
        return;
    }
    Transaction tx;
    long long lastId = 0;
    while (fread(&tx, sizeof(Transaction), 1, fp) == 1) {
        if (tx.txId > lastId) lastId = tx.txId;
    }
    fclose(fp);
    g_nextTxId = lastId + 1;
}
