/**
 * 
 */
#include "hookapi.h"

int64_t hook(uint32_t reserved) {
    TRACESTR("nav.c: Start.");

    // ACCOUNT: Hook Account
    uint8_t hook_acc[20];
    hook_account(hook_acc, 20);

    // ACCOUNT: Origin Tx Account
    uint8_t otxn_acc[SFS_ACCOUNT];
    otxn_field(otxn_acc, SFS_ACCOUNT, sfAccount);

    // FILTER ON: ACCOUNT
    if (BUFFER_EQUAL_20(hook_acc, otxn_acc))
        DONE("mutualfund: outgoing tx on `Account`.");

    // PARAMATER: Hook Parameter - Public Key
    uint8_t pk[33];
    uint8_t pk_key[2] = {'P', 'K'};
    if (hook_param(pk, 33, SBUF(pk_key)) != 33)
    {
        DONE("nav.c: invalid hook parameter`.");
    }
    
    // PARAMATER: Txn Parameter - Net Asset Value
    uint8_t nav[8];
    uint8_t nav_key[4] = {'N', 'A', 'V'};
    if (otxn_param(nav, 8, nav_key, 3) != 8)
    {
        DONE("nav.c: invalid otxn hook parameter`.");
    }

    // PARAMATER: Txn Parameter - Signature
    uint8_t sig[71];
    uint8_t sig_key[3] = {'S', 'I', 'G'};
    if (otxn_param(sig, 71, SBUF(sig_key)) != 71)
    {
         DONE("nav.c: invalid otxn hook parameter`.");
    }

    if (util_verify(SBUF(nav), SBUF(sig), SBUF(pk)) == 0)
    {
        accept(SBUF("hunt_one.c: Invalid signature."), __LINE__);
    }

    TRACEHEX(nav);
    TRACEHEX(hook_acc);

    state_set(SBUF(nav), hook_acc, 20);
    
    TRACESTR("nav.c: End.");
    accept(SBUF("nav.c: Finished."), __LINE__);
    _g(1,1);   // every hook needs to import guard function and use it at least once
    // unreachable
    return 0;
}