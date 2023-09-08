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


#include <stdint.h>
#include "hookapi.h"

// -----------------------------------------------------------------------------

/**
 * 
 * These functions should be moved into the macro.c file
*/

#define INT8_TO_BUF(buf_raw, i)\
{\
    unsigned char* buf = (unsigned char*)buf_raw;\
    buf[0] = (((uint8_t)i) >> 0) & 0xFFUL;\
    if (i < 0) buf[0] |= 0x80U;\
}

#define BUFFER_EQUAL_64(buf1, buf2) \
    ( \
        (*((uint64_t*)(buf1) + 0) == *((uint64_t*)(buf2) + 0)) && \
        (*((uint64_t*)(buf1) + 1) == *((uint64_t*)(buf2) + 1)) && \
        (*((uint64_t*)(buf1) + 2) == *((uint64_t*)(buf2) + 2)) && \
        (*((uint64_t*)(buf1) + 3) == *((uint64_t*)(buf2) + 3)) && \
        (*((uint64_t*)(buf1) + 4) == *((uint64_t*)(buf2) + 4)) && \
        (*((uint64_t*)(buf1) + 5) == *((uint64_t*)(buf2) + 5)) && \
        (*((uint64_t*)(buf1) + 6) == *((uint64_t*)(buf2) + 6)) && \
        (*((uint64_t*)(buf1) + 7) == *((uint64_t*)(buf2) + 7)) \
    )

#define ACCOUNT_TO_BUF(buf_raw, i)\
{\
    unsigned char* buf = (unsigned char*)buf_raw;\
    *(uint64_t*)(buf + 0) = *(uint64_t*)(i +  0);\
    *(uint64_t*)(buf + 8) = *(uint64_t*)(i +  8);\
    *(uint32_t*)(buf + 16) = *(uint32_t*)(i + 16);\
}

#define UINT256_TO_BUF(buf_raw, i)\
{\
    unsigned char* buf = (unsigned char*)buf_raw;\
    *(uint64_t*)(buf + 0) = *(uint64_t*)(i +  0);\
    *(uint64_t*)(buf + 8) = *(uint64_t*)(i +  8);\
    *(uint64_t*)(buf + 16) = *(uint64_t*)(i + 16);\
    *(uint64_t*)(buf + 24) = *(uint64_t*)(i + 24);\
}

#define NOPE(x)                                \
{                                              \
    return rollback((x), sizeof(x), __LINE__); \
}

#define FLIP_ENDIAN_64(n) ((uint64_t)(((n & 0xFFULL) << 56ULL) |             \
                                      ((n & 0xFF00ULL) << 40ULL) |           \
                                      ((n & 0xFF0000ULL) << 24ULL) |         \
                                      ((n & 0xFF000000ULL) << 8ULL) |        \
                                      ((n & 0xFF00000000ULL) >> 8ULL) |      \
                                      ((n & 0xFF0000000000ULL) >> 24ULL) |   \
                                      ((n & 0xFF000000000000ULL) >> 40ULL) | \
                                      ((n & 0xFF00000000000000ULL) >> 56ULL)))

// -----------------------------------------------------------------------------


/*

"3MM"
3rd Party Money Manager

Offer Transactions:
- OfferCreate
- OfferCancel

URIToken Transactions:
- URITokenCreateSellOffer
- URITokenCancelSellOffer
- URITokenBuy

Investment Limit: $1000 per day
Trading Limit: 10 transactions per month
Model: LRLR - Low Risk Low Reward (0:100)
Seat Count: 20
Quorum: 51%

*/

uint8_t o_txn[310] =
    {
        /* size,upto */
        /*   3,  0  */ 0x12U, 0x00U, 0x07U,                                                             /* tt = OfferCreate */
        /*   5,  3  */ 0x22U, 0x80U, 0x00U, 0x00U, 0x00U,                                               /* flags = tfCanonical */
        /*   5,  8  */ 0x24U, 0x00U, 0x00U, 0x00U, 0x00U,                                               /* sequence = 0 */
        /*   5, 13  */ 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                                               /* dtag, flipped */
        /*   6, 18  */ 0x20U, 0x1AU, 0x00U, 0x00U, 0x00U, 0x00U,                                        /* first ledger seq */
        /*   6, 24  */ 0x20U, 0x1BU, 0x00U, 0x00U, 0x00U, 0x00U,                                        /* last ledger seq */
        /*  49, 30  */ 0x64U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                          /* taker pays field 9 or 49 bytes */
                       0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                          /* cont...  */
                       0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                          /* cont...  */
                       0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                          /* cont...  */
                       0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                          /* cont...  */
                       0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                          /* cont...  */
                       0x99,                                                                            /* cont...  */
        /*  49, 79  */ 0x65U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                          /* taker gets field 9 or 49 bytes */
                       0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                          /* cont...  */
                       0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                          /* cont...  */
                       0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                          /* cont...  */
                       0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                          /* cont...  */
                       0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                          /* cont...  */
                       0x99,                                                                            /* cont...  */
        /*   9, 128 */ 0x68U, 0x40U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,                          /* fee      */
                       0x00U,                                                                           /* cont...  */
        /*  35, 137 */ 0x73U, 0x21U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* pubkey   */
        /*  22, 172 */ 0x81U, 0x14U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                           /* src acc  */
        /* 116, 194 */                                                                                  /* emit details */
        /*   0, 310 */
};

uint8_t u_txn[295] =
{
    /* size,upto */
    /*   3,  0  */ 0x12U, 0x00U, 0x2FU,                                                             /* tt = URITokenBuy */
    /*   5,  3  */ 0x22U, 0x80U, 0x00U, 0x00U, 0x00U,                                               /* flags = tfCanonical */
    /*   5,  8  */ 0x24U, 0x00U, 0x00U, 0x00U, 0x00U,                                               /* sequence = 0 */
    /*   5, 13  */ 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                                               /* dtag, flipped */
    /*   6, 18  */ 0x20U, 0x1AU, 0x00U, 0x00U, 0x00U, 0x00U,                                        /* first ledger seq */
    /*   6, 24  */ 0x20U, 0x1BU, 0x00U, 0x00U, 0x00U, 0x00U,                                        /* last ledger seq */
    /*  49, 30  */ 0x61U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                          /* amount field 9 or 49 bytes */
                    0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                          /* cont...  */
                    0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                          /* cont...  */
                    0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                          /* cont...  */
                    0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                          /* cont...  */
                    0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                          /* cont...  */
                    0x99,                                                                            /* cont...  */
    /*  34, 79  */ 0x50U, 0x24U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /* hash256 = URITokenID  */
    /*   9, 113 */ 0x68U, 0x40U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,                          /* fee      */
                    0x00U,                                                                           /* cont...  */
    /*  35, 122 */ 0x73U, 0x21U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* pubkey   */
    /*  22, 157 */ 0x81U, 0x14U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                           /* src acc  */
    /* 116, 179 */                                                                                  /* emit details */
    /*   0, 295 */
};

// OFFER TX BUILDER
#define O_FLS_OUT (o_txn + 20U)
#define O_LLS_OUT (o_txn + 26U)
#define O_DTAG_OUT (o_txn + 14U)
#define O_TAKER_PAYS_OUT (o_txn + 30U)
#define O_TAKER_GETS_OUT (o_txn + 79U)
#define O_HOOK_ACC (o_txn + 174U)
#define O_FEE_OUT (o_txn + 129U)
#define O_EMIT_OUT (o_txn + 194U)

// URITOKEN TX BUILDER
#define U_TT_OUT (u_txn + 2U)
#define U_FLS_OUT (u_txn + 20U)
#define U_LLS_OUT (u_txn + 26U)
#define U_DTAG_OUT (u_txn + 14U)
#define U_INDEXID_OUT (u_txn + 81U)
#define U_AMOUNT_OUT (u_txn + 30U)
#define U_HOOK_ACC (u_txn + 159U)
#define U_FEE_OUT (u_txn + 114U)
#define U_EMIT_OUT (u_txn + 179U)

// BINARY MODEL
#define MAX_MODEL_BYTES 94
#define UPDATE_STATUS (topic_data + 0U);
#define PRICE_OFFSET 14U // field offset from 0
#define CURRENCY_OFFSET 22U // field offset from 0
#define ISSUER_OFFSET 42U // field offset from 0
#define ID_OFFSET 62U // field offset from 0

// HARD CODED
#define SEAT_COUNT 20
#define QUORUM 0.51

int64_t hook(uint32_t r)
{
    // HookOn: Invoke
    if (otxn_type() != ttINVOKE) // ttINVOKE only
        DONE("3mm.c: Passing non-Invoke txn. HookOn should be changed to avoid this.");

    // ACCOUNT: Transaction
    uint8_t otxn_accid[32];
    otxn_field(otxn_accid + 12, 20, sfAccount);

    // ACCOUNT: Hook
    uint8_t hook_accid[32];
    hook_account(hook_accid + 12, 20);

    // PARAMATER: Txn Parameter - Net Asset Value
    uint8_t nav[8];
    uint8_t nav_key[4] = {'N', 'A', 'V'};
    if (otxn_param(nav, 8, nav_key, 3) == 8)
    {
        DONE("3mm.c: Skipping nav update.");
    }

    int64_t member_count = state(0, 0, "MC", 2);
    TRACEVAR(member_count);

    // initial execution, setup hook
    if (BUFFER_EQUAL_20(hook_accid + 12, otxn_accid + 12) && member_count == DOESNT_EXIST)
    {
        // uint64_t irr, ird;
        // gather hook parameters
        uint8_t imc;
        TRACEVAR(imc);
        if (hook_param(SVAR(imc), "IMC", 3) < 0)
            NOPE("3mm.c: initial Member Count Parameter missing (IMC).");

        TRACEVAR(imc);

        if (imc == 0)
            NOPE("3mm.c: initial Member Count must be > 0.");

        if (imc > SEAT_COUNT)
            NOPE("3mm.c: initial Member Count must be <= Seat Count (20).");

        // set member count
        ASSERT(state_set(SBUF(imc), "MC", 2));

        member_count = imc;

        for (uint8_t i = 0; GUARD(SEAT_COUNT), i < member_count; ++i)
        {
            uint8_t member_acc[20];
            uint8_t member_pkey[3] = {'I', 'S', i};
            if (hook_param(SBUF(member_acc), member_pkey, 3) != 20)
                NOPE("3mm.c: one or more initial member account ID's is missing.");

            // 0... X where X is member id started from 1
            // maps to the member's account ID
            trace(SBUF("3mm.c: Member: "), SBUF(member_acc), 1);
            // reverse key
            ASSERT(state_set(SBUF(member_acc), SVAR(i)) == 20);
            // 0, 0... ACCOUNT ID maps to member_id (as above)

            // forward key
            ASSERT(state_set(SVAR(i), SBUF(member_acc)) == 1);
        }

        DONE("3mm.c: setup completed successfully.");
    }

    if (BUFFER_EQUAL_20(hook_accid + 12, otxn_accid + 12))
        NOPE("3mm.c: outgoing tx on `Account`.");

    int64_t member_id = state(0, 0, otxn_accid + 12, 20);
    if (member_id < 0)
        NOPE("3mm.c: you are not currently a governance member at this table.");

    // { 'S|H|R', '\0 + topicid' }
    uint8_t topic[2];
    int64_t result = otxn_param(SBUF(topic), "T", 1);
    uint8_t t = topic[0]; // topic type
    uint8_t n = topic[1]; // number (seats)

    if (result != 2 || (t != 'S' && // topic type: seat (L1)
                        t != 'O' && // topic type: offer
                        t != 'U'))  // topic type: uritoken
        NOPE("3mm.c: valid TOPIC must be specified as otxn parameter.");

    if (t == 'S' && n > (SEAT_COUNT - 1))
        NOPE("3mm.c: valid seat topics are 0 through 19.");

    uint8_t topic_data[MAX_MODEL_BYTES];
    uint8_t topic_size = t == 'O' ? 64 : t == 'U' ? MAX_MODEL_BYTES
                                     : t == 'S'   ? 20
                                                  : 0;

    uint8_t padding = MAX_MODEL_BYTES - topic_size;
    result = otxn_param(topic_data + padding, topic_size, "V", 1);
    if (result != topic_size)
        NOPE("3mm.c: missing or incorrect size of VOTE data for TOPIC type.");

    // reuse otxn_accid to create vote key
    otxn_accid[0] = 'V';
    otxn_accid[1] = t;
    otxn_accid[2] = n;

    // get their previous vote if any on this topic
    uint8_t previous_topic_data[MAX_MODEL_BYTES];
    int64_t previous_topic_size = state(previous_topic_data + padding, topic_size, SBUF(otxn_accid));

    if (previous_topic_size == topic_size && previous_topic_data[0] != 0)
    {
        DONE("3mm.c: proposal is closed.");
    }

    // check if the vote they're making has already been cast before,
    // if it is identical to their existing vote for this topic then just end with tesSUCCESS
    if (previous_topic_size == topic_size && BUFFER_EQUAL_64(previous_topic_data, topic_data))
        DONE("3mm.c: your vote is already cast this way for this topic.");

    // execution to here means the vote is different
    // we might have to decrement the old voting if they voted previously
    // and we will have to increment the new voting

    // write vote to their voting key
    ASSERT(state_set(topic_data, topic_size, SBUF(otxn_accid)) == topic_size);
    TRACESTR("3mm.c: writing vote.");

    // decrement old vote counter for this option
    if (previous_topic_size > 0)
    {
        TRACESTR("3mm.c: previous_topic_size > 0.");
        uint8_t votes = 0;
        // override the first two bytes to turn it into a vote count key
        previous_topic_data[0] = 'C';
        previous_topic_data[1] = t;
        previous_topic_data[2] = n;

        if (state(&votes, 1, SBUF(previous_topic_data)) && votes > 0)
        {
            votes--;
            // delete the state entry if votes hit zero
            ASSERT(state_set(votes == 0 ? 0 : &votes, votes == 0 ? 0 : 1, SBUF(previous_topic_data)));
        }
    }

    // increment new counter
    uint8_t votes = 0;
    {
        // we're going to clobber the topic data to turn it into a vote count key
        // so store the first bytes
        uint64_t saved_data = *((uint64_t *)topic_data);
        topic_data[0] = 'C';
        topic_data[1] = t;
        topic_data[2] = n;

        state(&votes, 1, SBUF(topic_data));
        votes++;
        ASSERT(state_set(&votes, 1, SBUF(topic_data)));

        // restore the saved bytes
        *((uint64_t *)topic_data) = saved_data;
    }

    // set this flag if the topic data is all zeros
    uint8_t zero[MAX_MODEL_BYTES];
    int topic_data_zero = BUFFER_EQUAL_64(topic_data, zero);

    if (DEBUG)
    {
        TRACEVAR(topic_data_zero);
        TRACEVAR(votes);
        TRACEVAR(member_count);
        trace(SBUF("topic"), topic, 2, 1);
    }

    int64_t req_quorum = member_count * QUORUM;
    TRACEVAR(req_quorum);
    if (votes < req_quorum) // 51% threshold for all voting
        DONE("3mm.c: vote recorded. Not yet enough votes to action.");

    switch (t)
    {
    case 'O':
    {
        // TXN: PREPARE: Init
        etxn_reserve(1);

        // ACCOUNT
        ACCOUNT_TO_BUF(O_HOOK_ACC, hook_accid + 12);

        // AMOUNT
        uint64_t txn_amount = 6107881094714392576; // 10
        uint64_t txn_drops = 10000000;             // 10 XRP in drops
        // CURRENCY
        uint8_t txn_currency[20] = {
            0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
            0x00U, 0x00U, 0x55U, 0x53U, 0x44U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};
        // ISSUER
        uint8_t txn_issuer[20] = {
            0xA4U, 0x07U, 0xAF, 0x58U, 0x56U, 0xCCU, 0xF3U, 0xC4U, 0x26U, 0x19U,
            0xDAU, 0xA9U, 0x25U, 0x81U, 0x3FU, 0xC9U, 0x55U, 0xC7U, 0x29U, 0x83U};

        // TXN PREPARE: FirstLedgerSequence
        uint32_t fls = (uint32_t)ledger_seq() + 1;
        *((uint32_t *)(O_FLS_OUT)) = FLIP_ENDIAN(fls);

        // TXN PREPARE: LastLedgerSequense
        uint32_t lls = fls + 4;
        *((uint32_t *)(O_LLS_OUT)) = FLIP_ENDIAN(lls);

        // TXN PREPARE: Dest Tag <- Source Tag
        if (otxn_field(O_DTAG_OUT, 4, sfSourceTag) == 4)
            *(O_DTAG_OUT - 1) = 0x2EU;

        uint64_t mode = 0; // 0 - BUY 1 - SELL
        switch (mode)
        {
        case 0:
        {
            if (DEBUG)
                TRACESTR("BUY FTOKEN");
            // BUY FTOKEN

            // TXN PREPARE: TakerPays
            float_sto(
                O_TAKER_PAYS_OUT,
                49,
                txn_currency,
                20,
                txn_issuer,
                20,
                txn_amount,
                sfTakerPays);

            // TXN PREPARE: TakerGets
            uint64_t drops = txn_drops;
            uint8_t *b = O_TAKER_GETS_OUT + 1;
            *b++ = 0b01000000 + ((drops >> 56) & 0b00111111);
            *b++ = (drops >> 48) & 0xFFU;
            *b++ = (drops >> 40) & 0xFFU;
            *b++ = (drops >> 32) & 0xFFU;
            *b++ = (drops >> 24) & 0xFFU;
            *b++ = (drops >> 16) & 0xFFU;
            *b++ = (drops >> 8) & 0xFFU;
            *b++ = (drops >> 0) & 0xFFU;
            break;
        }
        case 1:
        {
            if (DEBUG)
                TRACESTR("SELL FTOKEN");
            // SELL FTOKEN

            // TXN PREPARE: TakerPays
            uint64_t drops = txn_drops;
            uint8_t *b = O_TAKER_PAYS_OUT + 1;
            *b++ = 0b01000000 + ((drops >> 56) & 0b00111111);
            *b++ = (drops >> 48) & 0xFFU;
            *b++ = (drops >> 40) & 0xFFU;
            *b++ = (drops >> 32) & 0xFFU;
            *b++ = (drops >> 24) & 0xFFU;
            *b++ = (drops >> 16) & 0xFFU;
            *b++ = (drops >> 8) & 0xFFU;
            *b++ = (drops >> 0) & 0xFFU;

            // TXN PREPARE: TakerGets
            float_sto(
                O_TAKER_GETS_OUT,
                49,
                txn_currency,
                20,
                txn_issuer,
                20,
                txn_amount,
                sfTakerGets);
            break;
        }
        case 2:
        {
            if (DEBUG)
                TRACESTR("CANCEL OFFER");
            // CANCEL OFFER
            break;
        }
        }

        // TXN PREPARE: Emit Metadata
        etxn_details(O_EMIT_OUT, 116U);

        // TXN PREPARE: Fee
        {
            int64_t fee = etxn_fee_base(SBUF(o_txn));
            if (DEBUG)
                TRACEVAR(fee);
            uint8_t *b = O_FEE_OUT;
            *b++ = 0b01000000 + ((fee >> 56) & 0b00111111);
            *b++ = (fee >> 48) & 0xFFU;
            *b++ = (fee >> 40) & 0xFFU;
            *b++ = (fee >> 32) & 0xFFU;
            *b++ = (fee >> 24) & 0xFFU;
            *b++ = (fee >> 16) & 0xFFU;
            *b++ = (fee >> 8) & 0xFFU;
            *b++ = (fee >> 0) & 0xFFU;
        }

        TRACEHEX(o_txn);

        if (DEBUG)
            trace(SBUF("txnraw"), SBUF(o_txn), 1);

        // TXN: Emit/Send Txn
        uint8_t emithash[32];
        int64_t emit_result = emit(SBUF(emithash), SBUF(o_txn));
        if (emit_result <= 0)
        {
            rollback(SBUF("3mm.c: proposal (Offer) Emit Failure."), __LINE__);
        }

        INT8_TO_BUF(UPDATE_STATUS, 1);

        // STATE SET: Status Update
        ASSERT(state_set(topic_data, 64, SBUF(otxn_accid)) == 64);
        DONE("3mm.c: proposal (Offer) Emit Success.");
    }
    case 'U':
    {
        // TXN: PREPARE: Init
        etxn_reserve(1);

        // ACCOUNT
        ACCOUNT_TO_BUF(U_HOOK_ACC, hook_accid + 12);

        // URITOKEN ID
        UINT256_TO_BUF(U_INDEXID_OUT, topic_data + ID_OFFSET);

        // TXN PREPARE: FirstLedgerSequence
        uint32_t fls = (uint32_t)ledger_seq() + 1;
        *((uint32_t *)(U_FLS_OUT)) = FLIP_ENDIAN(fls);

        // TXN PREPARE: LastLedgerSequense
        uint32_t lls = fls + 4;
        *((uint32_t *)(U_LLS_OUT)) = FLIP_ENDIAN(lls);

        // TXN PREPARE: Dest Tag <- Source Tag
        if (otxn_field(U_DTAG_OUT, 4, sfSourceTag) == 4)
            *(U_DTAG_OUT - 1) = 0x2EU;

        uint64_t mode = 0; // 0 - BUY 1 - SELL
        switch (mode)
        {
        case 0:
        {
            if (DEBUG)
                TRACESTR("BUY URITOKEN");
            // BUY URITOKEN

            // TXN PREPARE: Transaction Type
            *U_TT_OUT = 0x2FU;

            // AMOUNT
            int64_t amount_native = 1;
            uint8_t zero_buf[20];
            if (
                !BUFFER_EQUAL_20(topic_data + CURRENCY_OFFSET, zero_buf) 
                && !BUFFER_EQUAL_20(topic_data + ISSUER_OFFSET, zero_buf)
            )
            {
                amount_native = 0;
            }

            uint64_t amount = FLIP_ENDIAN_64(UINT64_FROM_BUF(topic_data + PRICE_OFFSET));
            if (float_compare(amount, 0, COMPARE_LESS | COMPARE_EQUAL) == 1)
                rollback(SBUF("3mm.c: invalid buy model parameter `AMT`."), __LINE__);

            // TXN PREPARE: Amount
            if (amount_native)
            {
                uint64_t drops = float_int(amount, 6, 1);
                uint8_t *b = U_AMOUNT_OUT + 1;
                *b++ = 0b01000000 + ((drops >> 56) & 0b00111111);
                *b++ = (drops >> 48) & 0xFFU;
                *b++ = (drops >> 40) & 0xFFU;
                *b++ = (drops >> 32) & 0xFFU;
                *b++ = (drops >> 24) & 0xFFU;
                *b++ = (drops >> 16) & 0xFFU;
                *b++ = (drops >> 8) & 0xFFU;
                *b++ = (drops >> 0) & 0xFFU;
            }
            else
                float_sto(
                    U_AMOUNT_OUT,
                    49,
                    topic_data + CURRENCY_OFFSET,
                    20,
                    topic_data + ISSUER_OFFSET,
                    20,
                    amount,
                    sfAmount);
            break;
        }
        case 1:
        {
            if (DEBUG)
                TRACESTR("SELL URITOKEN");
            // SELL URITOKEN

            // TXN PREPARE: Transaction Type
            *U_TT_OUT = 0x30U;

            // AMOUNT
            int64_t amount_native = 1;
            uint8_t zero_buf[20];
            if (
                !BUFFER_EQUAL_20(topic_data + CURRENCY_OFFSET, zero_buf)
                && !BUFFER_EQUAL_20(topic_data + ISSUER_OFFSET, zero_buf)
            )
            {
                amount_native = 0;
            }

            uint64_t amount = FLIP_ENDIAN_64(UINT64_FROM_BUF(topic_data + PRICE_OFFSET));
            if (float_compare(amount, 0, COMPARE_LESS | COMPARE_EQUAL) == 1)
                rollback(SBUF("3mm.c: invalid sell model parameter `AMT`."), __LINE__);

            // TXN PREPARE: Amount
            if (amount_native)
            {
                uint64_t drops = float_int(amount, 6, 1);
                uint8_t *b = U_AMOUNT_OUT + 1;
                *b++ = 0b01000000 + ((drops >> 56) & 0b00111111);
                *b++ = (drops >> 48) & 0xFFU;
                *b++ = (drops >> 40) & 0xFFU;
                *b++ = (drops >> 32) & 0xFFU;
                *b++ = (drops >> 24) & 0xFFU;
                *b++ = (drops >> 16) & 0xFFU;
                *b++ = (drops >> 8) & 0xFFU;
                *b++ = (drops >> 0) & 0xFFU;
            }
            else
                float_sto(
                    U_AMOUNT_OUT,
                    49,
                    topic_data + CURRENCY_OFFSET,
                    20,
                    topic_data + ISSUER_OFFSET,
                    20,
                    amount,
                    sfAmount);
            break;
        }
        case 2:
        {
            if (DEBUG)
                TRACESTR("CANCEL URITOKEN OFFER");
            // CANCEL URITOKEN OFFER
            break;
        }
        }

        // TXN PREPARE: Emit Metadata
        etxn_details(U_EMIT_OUT, 116U);

        // TXN PREPARE: Fee
        {
            int64_t fee = etxn_fee_base(SBUF(u_txn));
            if (DEBUG)
                TRACEVAR(fee);
            uint8_t *b = U_FEE_OUT;
            *b++ = 0b01000000 + ((fee >> 56) & 0b00111111);
            *b++ = (fee >> 48) & 0xFFU;
            *b++ = (fee >> 40) & 0xFFU;
            *b++ = (fee >> 32) & 0xFFU;
            *b++ = (fee >> 24) & 0xFFU;
            *b++ = (fee >> 16) & 0xFFU;
            *b++ = (fee >> 8) & 0xFFU;
            *b++ = (fee >> 0) & 0xFFU;
        }

        TRACEHEX(u_txn);

        if (DEBUG)
            trace(SBUF("txnraw"), SBUF(u_txn), 1);

        // TXN: Emit/Send Txn
        uint8_t emithash[32];
        int64_t emit_result = emit(SBUF(emithash), SBUF(u_txn));
        if (emit_result <= 0)
        {
            rollback(SBUF("3mm.c: proposal (URIToken) emit failure."), __LINE__);
        }

        // STATE SET: Status Update
        INT8_TO_BUF(UPDATE_STATUS, 1);
        ASSERT(state_set(topic_data, 64, SBUF(otxn_accid)) == 64);
        DONE("3mm.c: proposal (URIToken) emit success.");
    }
    case 'S':
    {
        // add / change member
        uint8_t previous_member[32];
        int previous_present = (state(previous_member + 12, 20, n, 1) == 20);

        if (previous_present && !topic_data_zero)
        {
            // we will not change member count, we're adding a member and removing a member

            // pass
        }
        else
        {
            // adjust member count
            if (previous_present)
                member_count--;
            else
                member_count++;

            ASSERT(member_count > 0); // just bail out if the last member is trying to self remove

            ASSERT(state_set(&member_count, 1, SBUF(zero)) == 1);
        }

        // we need to garbage collect all their votes
        if (previous_present)
        {
            previous_member[0] = 'V';

            for (int i = 1; GUARD(32), i < 32; ++i)
            {
                previous_member[1] = i < 2 ? 'R' : i < 12 ? 'H'
                                                          : 'S';
                previous_member[2] = i < 2 ? i : i < 12 ? i - 2
                                                        : i - 12;

                uint8_t vote_key[32];
                if (state(SBUF(vote_key), SBUF(previous_member)) == 32)
                {
                    uint8_t vote_count = 0;

                    // find and decrement the vote counter
                    vote_key[0] = 'C';
                    vote_key[1] = previous_member[1];
                    vote_key[2] = previous_member[2];
                    if (state(&vote_count, 1, SBUF(vote_key)) == 1)
                    {
                        // if we're down to 1 vote then delete state
                        if (vote_count <= 1)
                        {
                            ASSERT(state_set(0, 0, SBUF(vote_key)) == 0);
                        }
                        else // otherwise decrement
                        {
                            vote_count--;
                            ASSERT(state_set(&vote_count, 1, SBUF(vote_key)) == 1);
                        }
                    }

                    // delete the vote entry
                    ASSERT(state_set(0, 0, SBUF(previous_member)) == 0);
                }
            }
        }

        if (!topic_data_zero)
        {
            // add the new member
            // reverse key
            ASSERT(state_set(topic_data, 20, n, 1) == 20);

            // forward key
            ASSERT(state_set(n, 1, SBUF(topic_data)) == 20);
        }

        DONE("3mm.c: member change success.");
    }
    }

    accept(SBUF("3mm.c: fallthrough."), __LINE__);
}