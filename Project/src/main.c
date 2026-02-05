#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "config.h"
#include "utils.h"
#include "accounts.h"
#include "transactions.h"
#include "storage.h"
#include "ui.h"

/* ---------------- Admin Auth ---------------- */

static void load_admin(char *user, size_t un, char *pass, size_t pn) {
    strncpy(user, "admin", un - 1); user[un - 1] = '\0';
    strncpy(pass, "admin123", pn - 1); pass[pn - 1] = '\0';

    FILE *fp = fopen(ADMIN_FILE, "r");
    if (!fp) return;

    char u[128], p[128];
    if (fgets(u, sizeof(u), fp) && fgets(p, sizeof(p), fp)) {
        u[strcspn(u, "\r\n")] = 0;
        p[strcspn(p, "\r\n")] = 0;
        if (strlen(u) > 0 && strlen(p) > 0) {
            strncpy(user, u, un - 1); user[un - 1] = '\0';
            strncpy(pass, p, pn - 1); pass[pn - 1] = '\0';
        }
    }
    fclose(fp);
}

static int admin_login(void) {
    char realUser[128], realPass[128];
    load_admin(realUser, sizeof(realUser), realPass, sizeof(realPass));

    printf("=====================================\n");
    printf("         ADMIN LOGIN REQUIRED         \n");
    printf("=====================================\n");
    printf("Default: user=admin  pass=admin123\n");
    printf("To change: create admin.cfg with 2 lines (user then pass)\n");
    printf("-------------------------------------\n");

    char u[128], p[128];
    read_nonempty("Username: ", u, sizeof(u));
    read_nonempty("Password: ", p, sizeof(p));

    if (strcmp(u, realUser) == 0 && strcmp(p, realPass) == 0) {
        printf("Login success.\n\n");
        return 1;
    }
    printf("Login failed.\n\n");
    return 0;
}

/* ---------------- Helpers ---------------- */

static int find_index_by_accno(int accNo) {
    for (int i = 0; i < g_count; i++) {
        if (g_accounts[i].accountNumber == accNo) return i;
    }
    return -1;
}

static int generate_unique_accno(void) {
    srand((unsigned)time(NULL));
    for (int tries = 0; tries < 20000; tries++) {
        int candidate = 100000 + (rand() % 900000);
        if (find_index_by_accno(candidate) == -1) return candidate;
    }
    for (int candidate = 100000; candidate <= 999999; candidate++) {
        if (find_index_by_accno(candidate) == -1) return candidate;
    }
    return -1;
}

static void log_tx(int accNo, TxType type, double amt, int related, double balAfter, const char *note) {
    Transaction tx;
    memset(&tx, 0, sizeof(tx));
    tx.txId = g_nextTxId++;
    tx.accountNumber = accNo;
    tx.type = type;
    tx.amount = amt;
    tx.relatedAccount = related;
    tx.balanceAfter = balAfter;
    tx.when = now_dt();
    if (note && note[0]) {
        strncpy(tx.note, note, sizeof(tx.note) - 1);
        tx.note[sizeof(tx.note) - 1] = '\0';
    }
    append_tx(tx);
}

static void reset_daily_withdraw_if_new_day(Account *a) {
    DateTime d = now_dt();
    if (a->lastWDYear != d.year || a->lastWDMonth != d.month || a->lastWDDay != d.day) {
        a->lastWDYear = d.year;
        a->lastWDMonth = d.month;
        a->lastWDDay = d.day;
        a->dailyWithdrawn = 0.0;
    }
}

/* ---------------- Features ---------------- */

static void bank_dashboard(void) {
    printf("\n=== Bank Dashboard ===\n");

    int active = 0, deleted = 0;
    double total = 0, savingsTotal = 0, currentTotal = 0;

    for (int i = 0; i < g_count; i++) {
        Account *a = &g_accounts[i];
        if (a->isActive) {
            active++;
            total += a->balance;
            if (a->type == ACC_SAVINGS) savingsTotal += a->balance;
            else currentTotal += a->balance;
        } else {
            deleted++;
        }
    }

    printf("Total accounts (all): %d\n", g_count);
    printf("Active accounts     : %d\n", active);
    printf("Deleted accounts    : %d\n", deleted);
    printf("Total money (active): %.2f\n", total);
    printf("Savings total       : %.2f\n", savingsTotal);
    printf("Current total       : %.2f\n", currentTotal);
}

static void create_account(void) {
    if (g_count >= MAX_ACCOUNTS) {
        printf("Storage full.\n");
        return;
    }

    printf("\n=== Create New Account ===\n");

    int autoNo = generate_unique_accno();
    int choice;
    printf("1) Auto account number (%d)\n", autoNo);
    printf("2) Enter manually\n");
    if (!read_int("Choose: ", &choice)) return;

    int accNo = autoNo;
    if (choice == 2) {
        if (!read_int("Enter new account number: ", &accNo)) return;
        if (accNo <= 0 || find_index_by_accno(accNo) != -1) {
            printf("Invalid or already exists.\n");
            return;
        }
    } else if (choice != 1) {
        printf("Invalid choice.\n");
        return;
    }

    char name[MAX_NAME], phone[MAX_PHONE];
    read_nonempty("Customer name: ", name, sizeof(name));
    read_nonempty("Phone number : ", phone, sizeof(phone));

    int t;
    printf("Account type: 1) SAVINGS  2) CURRENT\n");
    if (!read_int("Choose type: ", &t)) return;
    if (t != 1 && t != 2) {
        printf("Invalid type.\n");
        return;
    }

    double initial;
    if (!read_double("Initial deposit: ", &initial)) return;
    if (initial < MIN_INITIAL_DEPOSIT) {
        printf("Minimum initial deposit: %.2f\n", MIN_INITIAL_DEPOSIT);
        return;
    }

    Account a;
    memset(&a, 0, sizeof(a));
    a.accountNumber = accNo;
    strncpy(a.name, name, sizeof(a.name) - 1);
    strncpy(a.phone, phone, sizeof(a.phone) - 1);
    a.type = (AccountType)t;
    a.balance = initial;
    a.isActive = 1;
    a.createdAt = now_dt();
    a.updatedAt = a.createdAt;

    a.lastWDYear = a.createdAt.year;
    a.lastWDMonth = a.createdAt.month;
    a.lastWDDay = a.createdAt.day;
    a.dailyWithdrawn = 0;

    g_accounts[g_count++] = a;
    save_accounts();
    log_tx(accNo, TX_CREATE, initial, 0, initial, "Account created");
    save_accounts();

    printf("Account created successfully!\n");
    print_account_card(&a);
}

static void view_account(void) {
    printf("\n=== View Account ===\n");
    int accNo;
    if (!read_int("Account number: ", &accNo)) return;

    int idx = find_index_by_accno(accNo);
    if (idx == -1) {
        printf("Account not found.\n");
        return;
    }
    print_account_card(&g_accounts[idx]);
}

static void edit_account(void) {
    printf("\n=== Edit Account ===\n");
    int accNo;
    if (!read_int("Account number: ", &accNo)) return;

    int idx = find_index_by_accno(accNo);
    if (idx == -1) {
        printf("Not found.\n");
        return;
    }

    Account *a = &g_accounts[idx];
    if (!a->isActive) {
        printf("Deleted account. Can't edit.\n");
        return;
    }

    printf("Leave empty to keep.\n");
    char line[128];

    printf("New name (current: %s): ", a->name);
    read_line(line, sizeof(line));
    if (strlen(line) > 0) strncpy(a->name, line, sizeof(a->name) - 1);

    printf("New phone (current: %s): ", a->phone);
    read_line(line, sizeof(line));
    if (strlen(line) > 0) strncpy(a->phone, line, sizeof(a->phone) - 1);

    a->updatedAt = now_dt();
    save_accounts();
    log_tx(a->accountNumber, TX_EDIT, 0.0, 0, a->balance, "Account edited");
    printf("Updated.\n");
    print_account_card(a);
}

static void delete_account(void) {
    printf("\n=== Delete Account (Soft) ===\n");
    int accNo;
    if (!read_int("Account number: ", &accNo)) return;

    int idx = find_index_by_accno(accNo);
    if (idx == -1) {
        printf("Not found.\n");
        return;
    }

    Account *a = &g_accounts[idx];
    if (!a->isActive) {
        printf("Already deleted.\n");
        return;
    }

    char yn[8];
    printf("Confirm delete (y/n): ");
    read_line(yn, sizeof(yn));
    if (!(yn[0] == 'y' || yn[0] == 'Y')) {
        printf("Cancelled.\n");
        return;
    }

    a->isActive = 0;
    a->updatedAt = now_dt();
    save_accounts();
    log_tx(a->accountNumber, TX_DELETE, 0.0, 0, a->balance, "Account deleted");
    printf("Deleted.\n");
}

static void deposit_money(void) {
    printf("\n=== Deposit ===\n");
    int accNo;
    if (!read_int("Account number: ", &accNo)) return;

    int idx = find_index_by_accno(accNo);
    if (idx == -1) { printf("Not found.\n"); return; }

    Account *a = &g_accounts[idx];
    if (!a->isActive) { printf("Deleted.\n"); return; }

    double amt;
    if (!read_double("Amount: ", &amt)) return;
    if (amt <= 0) { printf("Invalid.\n"); return; }

    a->balance += amt;
    a->updatedAt = now_dt();
    save_accounts();
    log_tx(a->accountNumber, TX_DEPOSIT, amt, 0, a->balance, "Deposit");
    printf("Done. New balance: %.2f\n", a->balance);
}

static void withdraw_money(void) {
    printf("\n=== Withdraw ===\n");
    int accNo;
    if (!read_int("Account number: ", &accNo)) return;

    int idx = find_index_by_accno(accNo);
    if (idx == -1) { printf("Not found.\n"); return; }

    Account *a = &g_accounts[idx];
    if (!a->isActive) { printf("Deleted.\n"); return; }

    reset_daily_withdraw_if_new_day(a);

    double amt;
    if (!read_double("Amount: ", &amt)) return;
    if (amt <= 0) { printf("Invalid.\n"); return; }
    if (amt > a->balance) { printf("Insufficient.\n"); return; }
    if (a->dailyWithdrawn + amt > DAILY_WITHDRAW_LIMIT) {
        printf("Daily limit exceeded. Limit %.2f, already %.2f\n",
               DAILY_WITHDRAW_LIMIT, a->dailyWithdrawn);
        return;
    }

    a->balance -= amt;
    a->dailyWithdrawn += amt;
    a->updatedAt = now_dt();
    save_accounts();
    log_tx(a->accountNumber, TX_WITHDRAW, amt, 0, a->balance, "Withdraw");
    printf("Done. New balance: %.2f\n", a->balance);
}

static void transfer_money(void) {
    printf("\n=== Transfer ===\n");
    int fromNo, toNo;
    if (!read_int("From: ", &fromNo)) return;
    if (!read_int("To  : ", &toNo)) return;

    if (fromNo == toNo) { printf("Same account.\n"); return; }

    int fromIdx = find_index_by_accno(fromNo);
    int toIdx = find_index_by_accno(toNo);
    if (fromIdx == -1 || toIdx == -1) { printf("Not found.\n"); return; }

    Account *from = &g_accounts[fromIdx];
    Account *to = &g_accounts[toIdx];
    if (!from->isActive || !to->isActive) { printf("Deleted account.\n"); return; }

    reset_daily_withdraw_if_new_day(from);

    double amt;
    if (!read_double("Amount: ", &amt)) return;
    if (amt <= 0) { printf("Invalid.\n"); return; }
    if (amt > from->balance) { printf("Insufficient.\n"); return; }
    if (from->dailyWithdrawn + amt > DAILY_WITHDRAW_LIMIT) { printf("Daily limit exceeded.\n"); return; }

    from->balance -= amt;
    from->dailyWithdrawn += amt;
    from->updatedAt = now_dt();

    to->balance += amt;
    to->updatedAt = now_dt();

    save_accounts();
    log_tx(from->accountNumber, TX_TRANSFER_OUT, amt, to->accountNumber, from->balance, "Transfer out");
    log_tx(to->accountNumber, TX_TRANSFER_IN, amt, from->accountNumber, to->balance, "Transfer in");

    printf("Transfer done.\n");
}

static void list_accounts(int onlyActive) {
    printf("\n=== Accounts List ===\n");
    int shown = 0;
    for (int i = 0; i < g_count; i++) {
        if (onlyActive && !g_accounts[i].isActive) continue;
        print_account_card(&g_accounts[i]);
        shown = 1;
    }
    if (!shown) printf("No accounts.\n");
}

static void search_by_name(void) {
    printf("\n=== Search By Name ===\n");
    char key[128];
    read_nonempty("Name part: ", key, sizeof(key));

    char keyLower[128];
    strncpy(keyLower, key, sizeof(keyLower) - 1);
    keyLower[sizeof(keyLower) - 1] = '\0';
    str_to_lower(keyLower);

    int found = 0;
    for (int i = 0; i < g_count; i++) {
        char tmp[MAX_NAME];
        strncpy(tmp, g_accounts[i].name, sizeof(tmp) - 1);
        tmp[sizeof(tmp) - 1] = '\0';
        str_to_lower(tmp);

        if (strstr(tmp, keyLower)) {
            print_account_card(&g_accounts[i]);
            found = 1;
        }
    }
    if (!found) printf("No matches.\n");
}

static void account_statement(void) {
    printf("\n=== Account Statement (Last N) ===\n");
    int accNo;
    if (!read_int("Account number: ", &accNo)) return;

    int n;
    if (!read_int("Last how many tx? ", &n)) return;
    if (n <= 0) { printf("Invalid.\n"); return; }

    FILE *fp = fopen(TX_FILE, "rb");
    if (!fp) { printf("No tx file.\n"); return; }

    Transaction *buf = (Transaction *)calloc((size_t)n, sizeof(Transaction));
    if (!buf) { fclose(fp); printf("Memory error.\n"); return; }

    int count = 0, pos = 0;
    Transaction tx;
    while (fread(&tx, sizeof(Transaction), 1, fp) == 1) {
        if (tx.accountNumber == accNo) {
            buf[pos] = tx;
            pos = (pos + 1) % n;
            if (count < n) count++;
        }
    }
    fclose(fp);

    if (count == 0) {
        printf("No transactions.\n");
        free(buf);
        return;
    }

    printf("\n--- Last %d transactions for %d ---\n", count, accNo);
    int start = (count == n) ? pos : 0;
    for (int i = 0; i < count; i++) {
        int k = (start + i) % n;
        char dt[32];
        format_dt(buf[k].when, dt, sizeof(dt));
        printf("TX#%lld | %s | %s | Amt:%.2f | BalAfter:%.2f | Note:%s\n",
               buf[k].txId, dt, tx_type_name(buf[k].type), buf[k].amount, buf[k].balanceAfter, buf[k].note);
    }

    free(buf);
}

static void apply_monthly_interest(void) {
    printf("\n=== Apply Monthly Interest (Savings) ===\n");
    double rate;
    if (!read_double("Rate (%): ", &rate)) return;
    if (rate < 0) { printf("Invalid.\n"); return; }

    int applied = 0;
    for (int i = 0; i < g_count; i++) {
        Account *a = &g_accounts[i];
        if (!a->isActive) continue;
        if (a->type != ACC_SAVINGS) continue;
        if (a->balance <= 0) continue;

        double interest = a->balance * (rate / 100.0);
        if (interest <= 0) continue;

        a->balance += interest;
        a->updatedAt = now_dt();
        save_accounts();
        log_tx(a->accountNumber, TX_INTEREST, interest, 0, a->balance, "Interest applied");
        applied++;
    }

    save_accounts();
    printf("Applied interest to %d account(s).\n", applied);
}

static void export_accounts_csv(void) {
    printf("\n=== Export CSV ===\n");
    char filename[128];
    printf("CSV filename (default accounts.csv): ");
    read_line(filename, sizeof(filename));
    if (strlen(filename) == 0) strncpy(filename, "accounts.csv", sizeof(filename) - 1);

    FILE *fp = fopen(filename, "w");
    if (!fp) { printf("Cannot write.\n"); return; }

    fprintf(fp, "accountNumber,name,phone,type,balance,isActive\n");
    for (int i = 0; i < g_count; i++) {
        Account *a = &g_accounts[i];
        fprintf(fp, "%d,\"%s\",\"%s\",%s,%.2f,%d\n",
                a->accountNumber, a->name, a->phone,
                acc_type_name(a->type), a->balance, a->isActive);
    }
    fclose(fp);
    printf("Exported to %s\n", filename);
}

int main(void) {
    load_accounts();
    load_next_txid();

    if (!admin_login()) {
        printf("Access denied.\n");
        return 0;
    }

    while (1) {
        print_main_menu();

        int choice;
        if (!read_int("Choose: ", &choice)) break;

        switch (choice) {
            case 1: bank_dashboard(); break;
            case 2: create_account(); break;
            case 3: view_account(); break;
            case 4: edit_account(); break;
            case 5: delete_account(); break;
            case 6: deposit_money(); break;
            case 7: withdraw_money(); break;
            case 8: transfer_money(); break;
            case 9: list_accounts(1); break;
            case 10: list_accounts(0); break;
            case 11: search_by_name(); break;
            case 12: account_statement(); break;
            case 13: apply_monthly_interest(); break;
            case 14: export_accounts_csv(); break;
            case 0: printf("Goodbye!\n"); return 0;
            default: printf("Invalid.\n");
        }
    }

    printf("Goodbye!\n");
    return 0;
}
