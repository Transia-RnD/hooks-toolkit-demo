/**
 *
 */
#include "hookapi.h"

uint8_t txn[283] =
{
/* size,upto */
/*   3,  0 */   0x12U, 0x00U, 0x00U,                                                               /* tt = Payment */
/*   5,  3*/    0x22U, 0x80U, 0x00U, 0x00U, 0x00U,                                          /* flags = tfCanonical */
/*   5,  8 */   0x24U, 0x00U, 0x00U, 0x00U, 0x00U,                                                 /* sequence = 0 */
/*   5, 13 */   0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                                                /* dtag, flipped */
/*   6, 18 */   0x20U, 0x1AU, 0x00U, 0x00U, 0x00U, 0x00U,                                      /* first ledger seq */
/*   6, 24 */   0x20U, 0x1BU, 0x00U, 0x00U, 0x00U, 0x00U,                                       /* last ledger seq */
/*  49, 30 */   0x61U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,              /* amount field 9 or 49 bytes */
                0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,
                0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,
                0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,
                0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,
                0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99,
/*   9, 79 */   0x68U, 0x40U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,                         /* fee      */
/*  35, 88 */   0x73U, 0x21U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,       /* pubkey   */
/*  22,123 */   0x81U, 0x14U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                                 /* src acc  */
/*  22,145 */   0x83U, 0x14U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                                 /* dst acc  */
/* 116,167 */   /* emit details */
/*   0,283 */
};

// TX BUILDER
#define HOOK_ACC (txn + 125U)
#define OTX_ACC (txn + 147U)
#define FLS_OUT (txn + 20U)
#define LLS_OUT (txn + 26U)
#define DTAG_OUT (txn + 14U)
#define AMOUNT_OUT (txn + 30U)
#define EMIT_OUT (txn + 167U)
#define FEE_OUT (txn + 80U)

int64_t hook(uint32_t reserved) {

    TRACESTR("mutualfund.c: Called.");
    _g(1,1);

    // ACCOUNT: Origin Tx Account
    otxn_field(OTX_ACC, SFS_ACCOUNT, sfAccount);

    // ACCOUNT: Hook Account
    hook_account(HOOK_ACC, SFS_ACCOUNT);

    // FILTER ON: ACCOUNT
    if (BUFFER_EQUAL_20(HOOK_ACC, OTX_ACC))
        DONE("mutualfund: outgoing tx on `Account`.");

    // Incoming Tx
    uint8_t amount_buffer[SFS_AMOUNT_IOU];
    int64_t amount_len = otxn_field(SBUF(amount_buffer), sfAmount);
    if (amount_len == 8) {
        DONE("mutualfund: Ignoring XRP Transaction");
    }

    int64_t oslot = otxn_slot(0);
    if (oslot < 0)
        rollback(SBUF("mutualfund: Could not slot originating txn."), 1);

    int64_t amt_slot = slot_subfield(oslot, sfAmount, 0);
    if (amt_slot < 0)
        rollback(SBUF("mutualfund: Could not slot otxn.sfAmount"), 2);

    int64_t amount_xfl = slot_float(amt_slot);
    if (amount_xfl < 0)
        rollback(SBUF("mutualfund: Could not parse amount."), 1);

    int64_t amount_token = float_int(amount_xfl, 0, 1);
    TRACEVAR(amount_token); // <- amount as token

    // DA: TODO BUFFER COST BASIS
    uint8_t cost_basis[SFS_AMOUNT_IOU];

    // DA: TODO Calculate NAV
    int64_t liabilities = 0;
    int64_t assets = 10;
    int64_t outstanding = 100;
    int64_t nav = liabilities - assets / outstanding;

    // TXN: PREPARE: Init
    etxn_reserve(1);

    // TXN PREPARE: FirstLedgerSequence
    uint32_t fls = (uint32_t)ledger_seq() + 1;
    *((uint32_t*)(FLS_OUT)) = FLIP_ENDIAN(fls);

    // TXN PREPARE: LastLedgerSequense
    uint32_t lls = fls + 4 ;
    *((uint32_t*)(LLS_OUT)) = FLIP_ENDIAN(lls);

    // TRACEHEX(amount_buffer);

    // DA: TODO CHECK THAT USER HAS TL

    // TXN PREPARE: Amount
    uint8_t currency[20] = {
        0x00U,0x00U,0x00U,0x00U,0x00U,0x00U,0x00U,0x00U,0x00U,0x00U,
        0x00U,0x00U,0x4EU,0x41U,0x56U,0x00U,0x00U,0x00U,0x00U,0x00U
    };
    TRACEHEX(currency);
    float_sto(
        AMOUNT_OUT,
        49,
        currency,
        20,
        HOOK_ACC,
        20,
        amount_xfl,
        sfAmount
    );

    // TXN PREPARE: Dest Tag <- Source Tag
    if (otxn_field(DTAG_OUT, 4, sfSourceTag) == 4)
        *(DTAG_OUT-1) = 0x2EU;

    // TXN PREPARE: Emit Metadata
    etxn_details(EMIT_OUT, 116U);

    // TXN PREPARE: Fee                                                                                                 
    {                                                                                                                  
        int64_t fee = etxn_fee_base(SBUF(txn));                                                                        
        if (DEBUG)                                                                                                     
            TRACEVAR(fee);                                                                                             
        uint8_t* b = FEE_OUT;                                                                                          
        *b++ = 0b01000000 + (( fee >> 56 ) & 0b00111111 );                                                             
        *b++ = (fee >> 48) & 0xFFU;                                                                                    
        *b++ = (fee >> 40) & 0xFFU;                                                                                    
        *b++ = (fee >> 32) & 0xFFU;                                                                                    
        *b++ = (fee >> 24) & 0xFFU;                                                                                    
        *b++ = (fee >> 16) & 0xFFU;                                                                                    
        *b++ = (fee >>  8) & 0xFFU;                                                                                    
        *b++ = (fee >>  0) & 0xFFU;                                                                                    
    }

    TRACEHEX(txn);

    if (DEBUG)
        trace(SBUF("txnraw"), SBUF(txn), 1);

    // TXN: Emit/Send Txn
    uint8_t emithash[32];
    int64_t emit_result = emit(SBUF(emithash), SBUF(txn));
    if (emit_result > 0)
    {
        // STATE: SET
        // state_set(SBUF(cost_basis), OTX_ACC, 20);
        accept(SBUF("mutualfund: Successfully emitted"), __LINE__);
    }
    return rollback(SBUF("mutualfund: Emit unsuccessful"), __LINE__);
}