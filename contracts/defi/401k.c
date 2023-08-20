#include <stdint.h>
#include "hookapi.h"

/*

Is PreTax?
Yearly Contribution Limit?
Matching Percentage?

*/

#define PARAM_PRETAX_SIZE 2
#define PARAM_PRETAX_N_SIZE 3
uint8_t PARAM_PRETAX[PARAM_PRETAX_N_SIZE] = 
{
    0x52, 0x41, 0x58
};

int64_t hook(uint32_t r)
{
    // HookOn: Invoke
    if (otxn_type() != ttINVOKE)  // ttINVOKE only
        DONE("401k.c: Passing non-Invoke txn. HookOn should be changed to avoid this.");

    // ACCOUNT: Transaction
    uint8_t otxn_accid[32];
    otxn_field(otxn_accid + 12, 20, sfAccount);

    TRACEHEX(otxn_accid);

    uint8_t param_value[8];
    hook_param(param_value, 8, SBUF(PARAM_PRETAX));

    accept(SBUF("401k.c: Finished"), __LINE__);
    _g(1,1);
    // unreachable
    return 0;
}