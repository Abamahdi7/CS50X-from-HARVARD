#include "accounts.h"

const char* acc_type_name(AccountType t) {
    return (t == ACC_SAVINGS) ? "SAVINGS" : "CURRENT";
}
