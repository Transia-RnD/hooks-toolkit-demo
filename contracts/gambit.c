#include "hookapi.h"

#define DEBUG 1

#define INT8_TO_BUF(buf_raw, i)\
{\
    unsigned char* buf = (unsigned char*)buf_raw;\
    buf[0] = (((uint8_t)i) >> 0) & 0xFFUL;\
    if (i < 0) buf[0] |= 0x80U;\
}

#define STATEKEY_TO_BUF(buf_raw, key_value) \
    uint8_t* buf = (uint8_t*)(buf_raw); \
    for (int i = 0; GUARD(20), i < 20; i++) { \
        buf[i] = key_value[12 + i]; \
    } \

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

// GAMBIT BET MODEL
#define BET_MS 161U // MODEL SIZE
#define BET_OS 8U // ODD SIZE
#define BET_OSO 1U // ODD SIZE OFFSET
#define ODD_FROM_BUF(buf_raw, buf_from) \
    uint8_t* buf = (uint8_t*)(buf_raw); \
    for (int i = 0; GUARD(BET_OS), i < BET_OS; i++) { \
        buf[i] = buf_from[BET_OSO + i]; \
    } \


// GAMBIT SLIP MODEL
#define BINARY_MODEL_SIZE 34
uint8_t b_model[BINARY_MODEL_SIZE] = {};
#define VALUE_S 8
#define VALUE_FROM_BUF(buf_raw, buf_from, offset) \
    uint8_t* v_buf = (uint8_t*)(buf_raw); \
    for (int i = 0; GUARD(VALUE_S), i < VALUE_S; i++) { \
        v_buf[i] = buf_from[offset + i]; \
    } \

#define WIN_S 8
#define WIN_FROM_BUF(buf_raw, buf_from, offset) \
    uint8_t* w_buf = (uint8_t*)(buf_raw); \
    for (int i = 0; GUARD(WIN_S), i < WIN_S; i++) { \
        w_buf[i] = buf_from[offset + i]; \
    } \

// FIELDS
#define VALUE_OUT (b_model + 1U)
#define WIN_OUT (b_model + 9U)
#define TO_SEAL_OUT (b_model + 17U)

uint8_t index_buffer[65] = {};

// HOOK PARAMS
#define BET_PS 3
#define BET_C_P ((uint8_t[BET_PS]){0x47U, 0x42U, 0x43U})
#define BET_S_P ((uint8_t[BET_PS]){0x47U, 0x42U, 0x5AU})

// TX PARAMS
#define SLIP_PS 3
#define SLIP_S_P ((uint8_t[SLIP_PS]){0x47U, 0x53U, 0x53U})


uint8_t namespace[32] = {
    0x5EU, 0x05U, 0xCBU, 0x51U, 0x5BU, 0x5EU, 0x6AU, 0xEEU, 0x84U, 0x32U,
    0x5AU, 0x8CU, 0xEEU, 0xB2U, 0x8AU, 0x92U, 0xC2U, 0xD8U, 0xD0U, 0x1CU,
    0x6BU, 0x0FU, 0x4BU, 0xB0U, 0xE7U, 0x43U, 0xBDU, 0x04U, 0xECU, 0xE7U,
    0x55U, 0x7EU
};

// 02 0254C71AFD498D0000 54C38D7EA4C680000 0000000000000 01 00

int64_t hook(uint32_t reserved) {
    // _g(1,1);
    TRACESTR("gambit.c: Start.");

    // ACCOUNT: Hook Account
    uint8_t hook_acc[SFS_ACCOUNT];
    hook_account(SBUF(hook_acc));

    // ACCOUNT: Origin Tx Account
    uint8_t otx_acc[SFS_ACCOUNT];
    otxn_field(otx_acc, SFS_ACCOUNT, sfAccount);
    TRACEHEX(otx_acc);

    // CONSTRUCTOR
    // HOOK PARAMS: GBC
    uint8_t gb_buf [BET_MS];
    int64_t gb_size = hook_param(
        gb_buf,
        BET_MS,
        BET_C_P,
        BET_PS
    );

    // HOOK ON: TT
    // FILTER BY: HOOK ACCOUNT
    if (otxn_type() == ttINVOKE && BUFFER_EQUAL_20(hook_acc, otx_acc))
    {
        // SETTLE
        // TX PARAMS: GBS | UTF8
        uint8_t winning_pos_buf[8] = {};
        otxn_param(winning_pos_buf, 8, BET_S_P, BET_PS);
        uint64_t winning_pos = float_int(*((int64_t*)winning_pos_buf), 0, 1);
        // TRACEVAR(winning_pos);

        // KEYLET: Hook State Dir
        uint8_t hook_state_dir[34];
        util_keylet(SBUF(hook_state_dir), KEYLET_HOOK_STATE_DIR, hook_acc, SFS_ACCOUNT,  SBUF(namespace), 0, 0);

        // SLOT SET:
        if (slot_set(SBUF(hook_state_dir), 2) != 2)
            accept(SBUF("gambit.c: Could not load keylet"), __LINE__);

        // SLOT SUBFIELD: Indexs (array of ledger entry keys)
        if (slot_subfield(2, sfIndexes, 3) != 3)
            accept(SBUF("gambit.c: Could not load sfIndexes"), __LINE__);

        int64_t arrays_len = slot(SBUF(index_buffer), 3);
        #define INDEX_LENGTH 20
        size_t numIndexes = arrays_len / INDEX_LENGTH;
        for (size_t i = 0; GUARD(10), i < numIndexes; i++) {
            TRACESTR(SBUF("startIndex"));
            size_t startIndex = 1 + i * INDEX_LENGTH;
            uint8_t hook_state[34];
            util_keylet(SBUF(hook_state), KEYLET_UNCHECKED, index_buffer + startIndex, 32, 0, 0, 0, 0);
            if (slot_set(SBUF(hook_state), 4) != 4)
                accept(SBUF("gambit.c: Could not load inner keylet"), __LINE__);

            if (slot_subfield(4, sfHookStateData, 5) != 5)
                accept(SBUF("gambit.c: Could not load sfHookStateData"), __LINE__);

            if (slot_subfield(4, sfHookStateKey, 6) != 6)
                accept(SBUF("gambit.c: Could not load sfHookStateData"), __LINE__);

            uint8_t b_model_key[slot_size(6)];
            slot(SBUF(b_model_key), 6);

            uint8_t state_key[20];
            #define STATE_KEY_OUT (state_key + 0U)
            STATEKEY_TO_BUF(STATE_KEY_OUT, b_model_key);

            uint8_t b_model_buf[slot_size(5)];
            slot(SBUF(b_model_buf), 5);

            int64_t slip_pos = (int64_t)b_model_buf[1];
            if (winning_pos != slip_pos) {
                TRACESTR("gambit.c: Skipping Slip - wrong win position");
                continue;
            }
            
            int64_t executed = (int64_t)b_model_buf[25U + 1U];
            if (executed == 1) {
                TRACESTR("gambit.c: Skipping Slip - executed win position");
                continue;
            }

            uint8_t to_seal[8];
            to_seal[0] = b_model_buf[18];
            to_seal[1] = b_model_buf[19];
            to_seal[2] = b_model_buf[20];
            to_seal[3] = b_model_buf[21];
            to_seal[4] = b_model_buf[22];
            to_seal[5] = b_model_buf[23];
            to_seal[6] = b_model_buf[24];
            to_seal[7] = b_model_buf[25];

            int64_t to_seal_amt = INT64_FROM_BUF(to_seal);
            if (to_seal != 0)
            {
                TRACESTR("gambit.c: Skipping Slip - not sealed.");
                continue;
            }

            uint8_t value_buf[VALUE_S];
            VALUE_FROM_BUF(value_buf, b_model_buf, 1U);
            uint64_t value_drops = float_int(*((int64_t*)value_buf), 6, 1);

            uint8_t win_buf[WIN_S];
            WIN_FROM_BUF(win_buf, b_model_buf, 9U);
            uint64_t win_drops = float_int(*((int64_t*)win_buf), 6, 1);

            // DA: REWRITE
            uint8_t index_portion[34];
            for (int i = 0; GUARD(34), i < 34; i++) {
                index_portion[i] = b_model_buf[1+i];
            }

            // TRACEHEX(index_portion);
            #define UPDATE_EXP_OUT (index_portion + 25U)
            INT8_TO_BUF(UPDATE_EXP_OUT, 1);

            // TRACEHEX(index_portion);

            #define HOOK_ACC (txn + 125U)
            #define OTX_ACC (txn + 147U)
            #define FLS_OUT (txn + 20U)                                                                                            
            #define LLS_OUT (txn + 26U)                                                                                            
            #define DTAG_OUT (txn + 14U)                                                                                           
            #define AMOUNT_OUT (txn + 30U)                                                                                                                                                                                
            #define EMIT_OUT (txn + 167U)                                                                                          
            #define FEE_OUT (txn + 80U)

            // TXN: PREPARE: Init
            etxn_reserve(1);

            // DA: CREATE MACRO
            for (int i = 0; GUARD(20), i < 20; i++) {
                txn[147 + i] = state_key[i];
            }

            // DA: CREATE MACRO
            for (int i = 0; GUARD(20), i < 20; i++) {
                txn[125 + i] = hook_acc[i];
            }

            // TXN PREPARE: FirstLedgerSequence
            uint32_t fls = (uint32_t)ledger_seq() + 1;
            *((uint32_t*)(FLS_OUT)) = FLIP_ENDIAN(fls);

            // TXN PREPARE: LastLedgerSequense
            uint32_t lls = fls + 4 ;
            *((uint32_t*)(LLS_OUT)) = FLIP_ENDIAN(lls);

            // TXN PREPARE: Amount
            uint64_t drops = value_drops + win_drops;
            uint8_t* b = AMOUNT_OUT + 1;
            *b++ = 0b01000000 + (( drops >> 56 ) & 0b00111111 );
            *b++ = (drops >> 48) & 0xFFU;
            *b++ = (drops >> 40) & 0xFFU;
            *b++ = (drops >> 32) & 0xFFU;
            *b++ = (drops >> 24) & 0xFFU;
            *b++ = (drops >> 16) & 0xFFU;
            *b++ = (drops >>  8) & 0xFFU;
            *b++ = (drops >>  0) & 0xFFU;

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
                state_set(SBUF(index_portion), b_model_key, slot_size(6));
                TRACESTR(SBUF("gambit.c: Emitted."));
            }
            TRACESTR(SBUF("gambit.c: Not Emitted."));
        }
        // uint8_t new_gb_buf[BET_MS];
        // hook_param_set(
        //     new_gb_buf,
        //     BET_MS,
        //     BET_C_P,
        //     BET_PS
        //     gb_buf,
        //     BET_MS,
        // );
        DONE("gambit.c: Settlement Successful.");
    }

    uint8_t otxn_state[BINARY_MODEL_SIZE];
    if (state(SBUF(otxn_state), otx_acc, SFS_ACCOUNT) == BINARY_MODEL_SIZE)
    {
        DONE("gambit.c: Cannot update slip after submission.");
    }

    int64_t otxn_param_size = otxn_param(b_model, BINARY_MODEL_SIZE, SLIP_PS, SLIP_S_P);
    TRACEHEX(b_model);

    int64_t amount_xfl = -1;
    otxn_slot(1);
    if (slot_subfield(1, sfAmount, 1) == 1)
    {
        amount_xfl = slot_float(1);
    }
    INT64_TO_BUF(VALUE_OUT, amount_xfl);

    int64_t position = (int64_t)b_model[0];

    int64_t win = 0;
    int64_t newValue = 0;

    // ODD
    uint8_t odd_buf[BET_OS];
    ODD_FROM_BUF(odd_buf, gb_buf);
    uint64_t odd = float_int(*((int64_t*)odd_buf), 0, 1);

    // 1 for yes
    if (position == 1) {
        win = float_multiply(amount_xfl, odd);
    }
    // 2 for no
    if (position == 2) {
        win = float_divide(amount_xfl, odd);
    }
    INT64_TO_BUF(WIN_OUT, win);

    int64_t new_value = amount_xfl;
    int8_t is_sealed = 0;
    {
        // KEYLET: Hook State Dir
        uint8_t hook_state_dir[34];
        util_keylet(SBUF(hook_state_dir), KEYLET_HOOK_STATE_DIR, hook_acc, SFS_ACCOUNT,  SBUF(namespace), 0, 0);

        // SLOT SET:
        if (slot_set(SBUF(hook_state_dir), 2) != 2)
            INT64_TO_BUF(TO_SEAL_OUT, win);
            state_set(SBUF(b_model), otx_acc, SFS_ACCOUNT);
            accept(SBUF("gambit.c: Could not load keylet"), __LINE__);

        // SLOT SUBFIELD: Indexs (array of ledger entry keys)
        if (slot_subfield(2, sfIndexes, 3) != 3)
            accept(SBUF("gambit.c: Could not load sfIndexes"), __LINE__);

        // DA: NO! NO! NO! Calc outside and fail if slip already sealed 
        // (submit the slip you want to seal)
        int64_t arrays_len = slot(SBUF(index_buffer), 3);
        #define INDEX_LENGTH 20
        size_t numIndexes = arrays_len / INDEX_LENGTH;
        for (size_t i = 0; GUARD(10), i < numIndexes; i++) {
            TRACESTR(SBUF("startIndex"));
            size_t startIndex = 1 + i * INDEX_LENGTH;
            
            uint8_t hook_state[34];
            util_keylet(SBUF(hook_state), KEYLET_UNCHECKED, index_buffer + startIndex, 32, 0, 0, 0, 0);
            if (slot_set(SBUF(hook_state), 4) != 4)
                accept(SBUF("gambit.c: Could not load inner keylet"), __LINE__);

            if (slot_subfield(4, sfHookStateData, 5) != 5)
                accept(SBUF("gambit.c: Could not load sfHookStateData"), __LINE__);

            if (slot_subfield(4, sfHookStateKey, 6) != 6)
                accept(SBUF("gambit.c: Could not load sfHookStateData"), __LINE__);

            uint8_t b_model_key[slot_size(6)];
            slot(SBUF(b_model_key), 6);
            
            uint8_t b_model_buf[35];
            slot(SBUF(b_model_buf), 5);

            int64_t position = (int64_t)b_model[0];
            int64_t slip_pos = (int64_t)b_model_buf[0 + 1U];

            if (position == slip_pos) {
                TRACESTR('SKIP WRONG POSITION')
                continue;
            }

            uint8_t to_seal[8];
            to_seal[0] = b_model_buf[18];
            to_seal[1] = b_model_buf[19];
            to_seal[2] = b_model_buf[20];
            to_seal[3] = b_model_buf[21];
            to_seal[4] = b_model_buf[22];
            to_seal[5] = b_model_buf[23];
            to_seal[6] = b_model_buf[24];
            to_seal[7] = b_model_buf[25];

            int64_t to_seal_amt = INT64_FROM_BUF(to_seal);
            if (to_seal > new_value)
            {
                TRACESTR("gambit.c: Skip ToSeal > New Value.");
                continue;
            }
            new_value = new_value - to_seal_amt;
            is_sealed = 1;
            #define UPDATE_SEAL_OUT (b_model_buf + 18U);
            INT64_TO_BUF(UPDATE_SEAL_OUT, 0);
            state_set(b_model_buf + 1U, 34, b_model_key, slot_size(6));
            TRACESTR("gambit.c: UPDATE SLIP.");
        }
        if (is_sealed == 0) {
            new_value = win;
        }
        INT64_TO_BUF(TO_SEAL_OUT, new_value);
        state_set(SBUF(b_model), otx_acc, SFS_ACCOUNT);
        TRACESTR("gambit.c: SET SLIP.");
    }

    TRACESTR("gambit.c: End.");
    accept(SBUF("gambit.c: Success."), __LINE__);
    // unreachable
    return 0;
}