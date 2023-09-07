#include "hookapi.h"

int64_t hook(uint32_t reserved)
{

    TRACESTR("Carbon: started");

    etxn_reserve(1);

    uint8_t carbon_accid[20];
    int64_t ret = util_accid(
            SBUF(carbon_accid),
            SBUF("rfCarbonVNTuXckX6x2qTMFmFSnm6dEWGX") );
    TRACEVAR(ret);

    unsigned char hook_accid[20];
    hook_account((uint32_t)hook_accid, 20);

    uint8_t account_field[20];
    int32_t account_field_len = otxn_field(SBUF(account_field), sfAccount);
    TRACEVAR(account_field_len);
    if (account_field_len < 20)
        rollback(SBUF("Carbon: sfAccount field missing!!!"), 1);

    int equal = 0; BUFFER_EQUAL(equal, hook_accid, account_field, 20);
    if (!equal)
    {
        accept(SBUF("Carbon: Incoming transaction"), 2);
    }

    unsigned char amount_buffer[48];
    int64_t amount_len = otxn_field(SBUF(amount_buffer), sfAmount);
    int64_t drops_to_send = 1000; // this will be the default


    if (amount_len != 8)
    {
        TRACESTR("Carbon: Non-xrp transaction detected, sending default 1000 drops to rfCarbon");
    } else
    {
        TRACESTR("Carbon: XRP transaction detected, computing 1% to send to rfCarbon");
        int64_t otxn_drops = AMOUNT_TO_DROPS(amount_buffer);
        TRACEVAR(otxn_drops);
        if (otxn_drops > 100000)   // if its less we send the default amount. or if there was an error we send default
            drops_to_send = (int64_t)((double)otxn_drops * 0.01f); // otherwise we send 1%
    }

    TRACEVAR(drops_to_send);

    unsigned char tx[PREPARE_PAYMENT_SIMPLE_SIZE];

    PREPARE_PAYMENT_SIMPLE(tx, drops_to_send, carbon_accid, 0, 0);
    
    uint8_t emithash[32];
    int64_t emit_result = emit(SBUF(emithash), SBUF(tx));
    TRACEVAR(emit_result);

    accept(SBUF("Carbon: Emitted transaction"), 0);
    return 0;

}
