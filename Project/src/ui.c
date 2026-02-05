#include "ui.h"
#include "utils.h"
#include <stdio.h>

void print_account_card(const Account *a) {
    char c[32], u[32];
    format_dt(a->createdAt, c, sizeof(c));
    format_dt(a->updatedAt, u, sizeof(u));

    printf("--------------------------------------------------\n");
    printf("Account: %d   [%s]   Status: %s\n",
           a->accountNumber, acc_type_name(a->type), a->isActive ? "ACTIVE" : "DELETED");
    printf("Name   : %s\n", a->name);
    printf("Phone  : %s\n", a->phone);
    printf("Balance: %.2f\n", a->balance);
    printf("Created: %s\n", c);
    printf("Updated: %s\n", u);
    printf("--------------------------------------------------\n");
}

void print_main_menu(void) {
    printf("\n=====================================\n");
    printf("           BankVault (CS50)          \n");
    printf("     CLI Bank Management System      \n");
    printf("=====================================\n");
    printf("1) Dashboard (stats)\n");
    printf("2) Create account\n");
    printf("3) View account\n");
    printf("4) Edit account\n");
    printf("5) Delete account (soft)\n");
    printf("6) Deposit\n");
    printf("7) Withdraw\n");
    printf("8) Transfer\n");
    printf("9) List accounts (active)\n");
    printf("10) List accounts (all)\n");
    printf("11) Search by name\n");
    printf("12) Account statement (last N tx)\n");
    printf("13) Apply monthly interest (savings)\n");
    printf("14) Export accounts to CSV\n");
    printf("0) Exit\n");
}
