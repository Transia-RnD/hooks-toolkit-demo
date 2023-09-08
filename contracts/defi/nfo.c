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


#include "hookapi.h"

// -----------------------------------------------------------------------------

/**
 * 
 * These functions should be moved into the macro.c file
*/

#define FLIP_ENDIAN_64(n) ((uint64_t)(((n & 0xFFULL) << 56ULL) |             \
                                      ((n & 0xFF00ULL) << 40ULL) |           \
                                      ((n & 0xFF0000ULL) << 24ULL) |         \
                                      ((n & 0xFF000000ULL) << 8ULL) |        \
                                      ((n & 0xFF00000000ULL) >> 8ULL) |      \
                                      ((n & 0xFF0000000000ULL) >> 24ULL) |   \
                                      ((n & 0xFF000000000000ULL) >> 40ULL) | \
                                      ((n & 0xFF00000000000000ULL) >> 56ULL)))

#define NOPE(x)                                    \
    {                                              \
        return rollback((x), sizeof(x), __LINE__); \
    }


// -----------------------------------------------------------------------------

/*

"NFO"
New Funding Opportunity

Hook Account: Authorized Trustlines

Trading Window?: YES - 16 hours
Price Determination: 24 hours - Net Asset Value
Risk Profile: LRLR - Low Risk - Low Reward (98/2) (Stable Coins)
Min Investors: 20 (Must be reached to start investment purchasing)
Max Investment: 25% of outstanding
Redemption Fee: 0.5% (0.05% - 2%)
Redemption Period: 30 days (30 to 180)
Closed NFO: 1 (cannot purchase after end of nfo period)
Ledgers in Days: 21600 (4 second ledger)
Start Ledger: 20
Initial Offering Period: 4 days
*/

uint8_t txn[283] =
    {
        /* size,upto */
        /*   3,  0 */ 0x12U, 0x00U, 0x00U,                                                                                             /* tt = Payment */
        /*   5,  3*/ 0x22U, 0x80U, 0x00U, 0x00U, 0x00U,                                                                                /* flags = tfCanonical */
        /*   5,  8 */ 0x24U, 0x00U, 0x00U, 0x00U, 0x00U,                                                                               /* sequence = 0 */
        /*   5, 13 */ 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                                                                               /* dtag, flipped */
        /*   6, 18 */ 0x20U, 0x1AU, 0x00U, 0x00U, 0x00U, 0x00U,                                                                        /* first ledger seq */
        /*   6, 24 */ 0x20U, 0x1BU, 0x00U, 0x00U, 0x00U, 0x00U,                                                                        /* last ledger seq */
        /*  49, 30 */ 0x61U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                                                          /* amount field 9 or 49 bytes */
        0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,
        0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,
        0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,
        0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,
        0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99,
        /*   9, 79 */ 0x68U, 0x40U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,                                                   /* fee      */
        /*  35, 88 */ 0x73U, 0x21U, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* pubkey   */
        /*  22,123 */ 0x81U, 0x14U, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                                        /* src acc  */
        /*  22,145 */ 0x83U, 0x14U, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                                        /* dst acc  */
        /* 116,167 */                                                                                                                  /* emit details */
        /*   0,283 */
};

// TX BUILDER
#define HOOK_ACC (txn + 125U)
#define OTXN_ACC (txn + 147U)
#define FLS_OUT (txn + 20U)
#define LLS_OUT (txn + 26U)
#define DTAG_OUT (txn + 14U)
#define AMOUNT_OUT (txn + 30U)
#define EMIT_OUT (txn + 167U)
#define FEE_OUT (txn + 80U)

// BUY TX BUILDER
// #define BUY_SRC_ACC (txn + 125U)
// #define BUY_DEST_ACC (txn + 147U)

// TXN PREPARE: Currency - NAV
uint8_t curr_nav[20] = {
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x4EU, 0x41U, 0x56U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};

#define MIN_INVESTORS 20
#define MAX_INVESTMENT 6073352297695428608 // 25 %
#define REDEM_FEE 6021809102166982656      // 0.5 %
#define REDEM_PERIOD 4                     // 4 days
#define LID 21600                          // 21600 ((60*60*24) / 4 "ledger close seconds")
#define CLOSED_OFFERING 1                  // open or closed (cannot purchase after close)
#define START_LEDGER 10                    // ledger index
#define OFFERING_PERIOD 4                  // 4 days

#define ISSUER_ACC (issuer_buf + 28U)

// STABLE COIN ACCEPTED
uint8_t curr_usd[20] = {
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x55U, 0x53U, 0x44U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};
uint8_t issuer_usd[20] = {
    0xAE, 0x12, 0x3A, 0x85, 0x56, 0xF3, 0xCF, 0x91, 0x15, 0x47,
    0x11, 0x37, 0x6A, 0xFB, 0x0F, 0x89, 0x4F, 0x83, 0x2B, 0x3D};

int64_t hook(uint32_t reserved)
{

    TRACESTR("nfo.c: Called.");
    _g(1, 1);

    // ACCOUNT: Origin Tx Account
    otxn_field(OTXN_ACC, SFS_ACCOUNT, sfAccount);

    // ACCOUNT: Dest Tx Account
    uint8_t dest_buf[SFS_AMOUNT_IOU];
    otxn_field(dest_buf, SFS_ACCOUNT, sfDestination);

    // ACCOUNT: Hook Account
    hook_account(HOOK_ACC, 20);

    // FILTER ON: ACCOUNT
    if (BUFFER_EQUAL_20(HOOK_ACC, OTXN_ACC))
        DONE("nfo.c: outgoing tx on `Account`.");

    // Incoming Tx
    uint8_t amount_buffer[SFS_AMOUNT_IOU];
    int64_t amount_len = otxn_field(SBUF(amount_buffer), sfAmount);
    if (amount_len == 8)
    {
        DONE("nfo.c: Ignoring XRP Transaction.");
    }

    int64_t oslot = otxn_slot(0);

    if (oslot < 0)
        rollback(SBUF("nfo.c: Could not slot originating txn."), 1);

    int64_t amt_slot = slot_subfield(oslot, sfAmount, 0);
    if (amt_slot < 0)
        rollback(SBUF("nfo.c: Could not slot otxn.sfAmount"), 2);

    int64_t amount_xfl = slot_float(amt_slot);
    if (amount_xfl < 0)
        rollback(SBUF("nfo.c: Could not parse amount."), 1);

    uint8_t issuer_buf[48];
    int64_t len1 = slot(issuer_buf, sizeof(issuer_buf), 0);
    int64_t mode = BUFFER_EQUAL_20(dest_buf, ISSUER_ACC);

    // DA: TODO: IN TRADING WINDOW - FIREWALL
    // DA: TODO: HAS RISK PROFILE

    int64_t ll_seq = ledger_seq();
    int64_t nfo_ll = START_LEDGER + LID;
    int64_t nfo_ended = ll_seq > nfo_ll;
    if (ll_seq < START_LEDGER)
    {
        NOPE("nfo.c: NFO has not started.");
    }

    // STATE: Nav
    uint8_t nav_buffer[8];
    if (state(SBUF(nav_buffer), HOOK_ACC, 20) != 8)
    {
        DONE("nfo.c: No current NAV.");
    }
    uint64_t price_xfl = FLIP_ENDIAN_64(INT64_FROM_BUF(nav_buffer));
    int64_t token_total = float_multiply(amount_xfl, price_xfl);

    // TXN: PREPARE: Init
    etxn_reserve(1);

    // TXN PREPARE: FirstLedgerSequence
    uint32_t fls = (uint32_t)ledger_seq() + 1;
    *((uint32_t *)(FLS_OUT)) = FLIP_ENDIAN(fls);

    // TXN PREPARE: LastLedgerSequense
    uint32_t lls = fls + 4;
    *((uint32_t *)(LLS_OUT)) = FLIP_ENDIAN(lls);

    switch (mode)
    {
    case 0:
        TRACESTR("nfo.c: Buy.");
        if (CLOSED_OFFERING && nfo_ended)
        {
            NOPE("nfo.c: NFO has closed");
        }

        // KEYLET: TrustLine - Dest NAV
        uint8_t bbal_kl[34];
        if (util_keylet(SBUF(bbal_kl), KEYLET_LINE, OTXN_ACC, SFS_ACCOUNT, (uint32_t)HOOK_ACC, SFS_ACCOUNT, curr_nav, 20) != 34)
        {
            NOPE("nfo.c: Missing NAV trustline on destination account.");
        }

        // SLOT SET: Slot 1
        if (slot_set(SBUF(bbal_kl), 1) != 1)
            accept(SBUF("nfo.c: Could not load target balance 1."), __LINE__);

        // // SLOT SUBFIELD: sfBalance
        // if (slot_subfield(1, sfBalance, 1) != 1)
        //     accept(SBUF("nfo.c: Could not load target balance 1.2."), __LINE__);

        // int64_t balance = slot_float(1); // <- amount as token
        // int64_t final_bal = float_sum(balance, amount_xfl);

        // int64_t outstanding_tokens = slot_float(2); // <- amount as token

        // int64_t percent = float_divide(final_bal, outstanding_tokens);
        // if (percent > MAX_INVESTMENT)
        // {
        //     NOPE("nfo.c: Max investment reached: 25 Percent.");
        // }

        // // ACCOUNT: Update DEST ACCT
        // otxn_field(BUY_DEST_ACC, SFS_ACCOUNT, sfAccount);

        // // ACCOUNT: Update SRC ACCT
        // hook_account(BUY_SRC_ACC, 20);

        float_sto(
            AMOUNT_OUT,
            49,
            curr_nav,
            20,
            HOOK_ACC,
            20,
            token_total,
            sfAmount);
        break;
    case 1:
        TRACESTR("nfo.c: Sell.");
        // KEYLET: TrustLine - Dest USD
        uint8_t sbal_kl[34];
        if (util_keylet(SBUF(sbal_kl), KEYLET_LINE, OTXN_ACC, SFS_ACCOUNT, (uint32_t)issuer_usd, SFS_ACCOUNT, curr_usd, 20) != 34)
        {
            NOPE("nfo.c: Missing trustline.");
        }

        int64_t early_redem = ll_seq < (nfo_ll + (REDEM_PERIOD * LID));

        if (early_redem)
        {
            TRACESTR("nfo.c: Early Redemption.");
            token_total = float_multiply(token_total, REDEM_FEE);
        }

        float_sto(
            AMOUNT_OUT,
            49,
            curr_usd,
            20,
            issuer_usd,
            20,
            token_total,
            sfAmount);
        break;
    }

    // TXN PREPARE: Dest Tag <- Source Tag
    if (otxn_field(DTAG_OUT, 4, sfSourceTag) == 4)
        *(DTAG_OUT - 1) = 0x2EU;

    // TXN PREPARE: Emit Metadata
    etxn_details(EMIT_OUT, 116U);

    // TXN PREPARE: Fee
    {
        int64_t fee = etxn_fee_base(SBUF(txn));
        if (DEBUG)
            TRACEVAR(fee);
        uint8_t *b = FEE_OUT;
        *b++ = 0b01000000 + ((fee >> 56) & 0b00111111);
        *b++ = (fee >> 48) & 0xFFU;
        *b++ = (fee >> 40) & 0xFFU;
        *b++ = (fee >> 32) & 0xFFU;
        *b++ = (fee >> 24) & 0xFFU;
        *b++ = (fee >> 16) & 0xFFU;
        *b++ = (fee >> 8) & 0xFFU;
        *b++ = (fee >> 0) & 0xFFU;
    }

    TRACEHEX(txn);

    // TXN: Emit/Send Txn
    uint8_t emithash[32];
    int64_t emit_result = emit(SBUF(emithash), SBUF(txn));
    if (emit_result > 0)
    {
        accept(SBUF("nfo.c: Transaction Complete."), __LINE__);
    }
    return rollback(SBUF("nfo.c: Transaction Failed."), __LINE__);
}