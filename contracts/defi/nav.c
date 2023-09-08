//------------------------------------------------------------------------------
/*
    Copyright (c) 2023 Transia, LLC

    This financial product is intended for use by individuals or entities who 
    possess the necessary licenses and qualifications to solicit and utilize 
    such products in accordance with applicable laws and regulations. 
    Unauthorized use or distribution of this product may be subject to legal 
    consequences.

    The information provided in this financial product is for informational 
    purposes only and should not be considered as financial advice or a 
    recommendation to engage in any specific investment or financial strategy. 
    It is important to consult with a qualified professional or financial 
    advisor before making any investment decisions.
*/
//==============================================================================


/*

"NAV"
Net Asset Value
*/

#include "hookapi.h"

int64_t hook(uint32_t reserved) {
    _g(1,1);
    TRACESTR("nav.c: Start.");

    // ACCOUNT: Hook Account
    uint8_t hook_acc[20];
    hook_account(hook_acc, 20);

    // ACCOUNT: Origin Tx Account
    uint8_t otxn_acc[SFS_ACCOUNT];
    otxn_field(otxn_acc, SFS_ACCOUNT, sfAccount);

    // FILTER ON: ACCOUNT
    if (BUFFER_EQUAL_20(hook_acc, otxn_acc))
        DONE("nav.c: outgoing tx on `Account`.");

    // PARAMATER: Hook Parameter - Public Key
    uint8_t pk[33];
    uint8_t pk_key[2] = {'P', 'K'};
    if (hook_param(pk, 33, SBUF(pk_key)) != 33)
    {
        DONE("nav.c: invalid hook parameter `PK`.");
    }
    
    // PARAMATER: Txn Parameter - Net Asset Value
    uint8_t nav[8];
    uint8_t nav_key[4] = {'N', 'A', 'V'};
    if (otxn_param(nav, 8, nav_key, 3) != 8)
    {
        DONE("nav.c: invalid otxn parameter `NAV`.");
    }

    // PARAMATER: Txn Parameter - Signature
    uint8_t sig[70];
    uint8_t sig_key[3] = {'S', 'I', 'G'};
    if (otxn_param(sig, 70, SBUF(sig_key)) != 70)
    {
        rollback(SBUF("nav.c: invalid otxn hook parameter `SIG`."), __LINE__);
    }

    if (util_verify(SBUF(nav), SBUF(sig), SBUF(pk)) == 0)
    {
        rollback(SBUF("nav.c: invalid signature."), __LINE__);
    }

    // DA: Add Firewall Whitelist
    state_set(SBUF(nav), hook_acc, 20);
    accept(SBUF("nav.c: finished."), __LINE__);
    return 0;
}