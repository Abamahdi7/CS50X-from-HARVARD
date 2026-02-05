#include "transactions.h"

const char* tx_type_name(TxType t) {
    switch (t) {
        case TX_DEPOSIT: return "DEPOSIT";
        case TX_WITHDRAW: return "WITHDRAW";
        case TX_TRANSFER_IN: return "TRANSFER_IN";
        case TX_TRANSFER_OUT: return "TRANSFER_OUT";
        case TX_CREATE: return "CREATE";
        case TX_EDIT: return "EDIT";
        case TX_DELETE: return "DELETE";
        case TX_INTEREST: return "INTEREST";
        default: return "UNKNOWN";
    }
}
