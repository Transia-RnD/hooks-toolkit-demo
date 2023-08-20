#include <stdint.h>
#include "hookapi.h"

// Global 3mm_govern.c

// Proposal 3mm_govern.c

// Voting 3mm_govern.c

// expiration: UInt64
// required: UInt8
// orderType: UInt8
// value: XFL
// currency: Currency
// issuer: XRPAddress

// #define PREVIOUS_MEMBER_PRESENT(previous_member, n) uint8_t previous_member[32]; \
// int previous_present = (state(previous_member + 12, 20, n, 1) == 20);

// #define ADJUST_MEMBER_COUNT(previous_present, member_count, zero) if (previous_present) \
//         member_count--; \
//     else \
//         member_count++; \
//     ASSERT(member_count > 0); \
//     ASSERT(state_set(&member_count, 1, SBUF(zero)) == 1);

// #define GARBAGE_COLLECT_VOTES(previous_present, previous_member) if (previous_present) \
// { \
//     previous_member[0] = 'V'; \
//     for (int i = 1; GUARD(32), i < 32; ++i) \
//     { \
//         previous_member[1] = i < 2 ? 'R' : i < 12 ? 'H' : 'S'; \
//         previous_member[2] = i < 2 ? i : i < 12 ? i - 2 : i - 12; \
//         uint8_t vote_key[32]; \
//         if (state(SBUF(vote_key), SBUF(previous_member)) == 32) \
//         { \
//             uint8_t vote_count = 0; \
//             vote_key[0] = 'C'; \
//             vote_key[1] = previous_member[1]; \
//             vote_key[2] = previous_member[2]; \
//             if (state(&vote_count, 1, SBUF(vote_key)) == 1) \
//             { \
//                 if (vote_count <= 1) \
//                 { \
//                     ASSERT(state_set(0,0, SBUF(vote_key)) == 0); \
//                 } \
//                 else \
//                 { \
//                     vote_count--; \
//                     ASSERT(state_set(&vote_count, 1, SBUF(vote_key)) == 1); \
//                 } \
//             } \
//             ASSERT(state_set(0,0, SBUF(previous_member)) == 0); \
//         } \
//     } \
// }

// #define ADD_NEW_MEMBER(topic_data, n) if (!topic_data_zero) \
// { \
//     ASSERT(state_set(topic_data, 20, n, 1) == 20); \
//     ASSERT(state_set(n, 1, SBUF(topic_data)) == 20); \
// }

int64_t hook(uint32_t r)
{
    // HookOn: Invoke
    if (otxn_type() != ttINVOKE)  // ttINVOKE only
        DONE("3mm_govern.c: Passing non-Invoke txn. HookOn should be changed to avoid this.");

    // ACCOUNT: Transaction
    uint8_t otxn_accid[32];
    otxn_field(otxn_accid + 12, 20, sfAccount);

    TRACEHEX(otxn_accid);

    // ACCOUNT: Hook
    uint8_t hook_accid[32];
    hook_account(hook_accid + 12, 20);

    int64_t member_count = state(0, 0, "MC", 2);
    if (DEBUG)
        TRACEVAR(member_count);

    // initial execution, setup hook
    if (BUFFER_EQUAL_20(hook_accid + 12, otxn_accid + 12) && member_count == DOESNT_EXIST)
    {
        // uint64_t irr, ird;
        // gather hook parameters
        uint8_t imc;
        TRACEVAR(imc);
        if (hook_param(SVAR(imc), "IMC", 3) < 0)
            NOPE("3mm_govern.c: Initial Member Count Parameter missing (IMC).");
        
        TRACEVAR(imc);
        
        if (imc == 0)
            NOPE("3mm_govern.c: Initial Member Count must be > 0.");

        if (imc > SEAT_COUNT)
            NOPE("3mm_govern.c: Initial Member Count must be <= Seat Count (20).");

        // if (hook_param(SVAR(irr), "IRR", 3) < 0)
        //     NOPE("3mm_govern.c: Initial Reward Rate Parameter missing (IRR).");

        // if (hook_param(SVAR(ird), "IRD", 3) < 0)
        //     NOPE("3mm_govern.c: Initial Reward Delay Parameter miss (IRD).");
        
        // if (ird == 0)
        //     NOPE("3mm_govern.c: Initial Reward Delay must be > 0.");
        
        // // set reward rate
        // ASSERT(state_set(SVAR(irr), "RR", 2));

        // // set reward delay
        // ASSERT(state_set(SVAR(ird), "RD", 2));

        // set member count
        ASSERT(state_set(SBUF(imc), "MC", 2));

        member_count = imc;
        TRACEVAR(member_count);

        for (uint8_t i = 0; GUARD(SEAT_COUNT), i < member_count; ++i)
        {
            uint8_t member_acc[20];
            uint8_t member_pkey[3] = {'I', 'S', i};
            if (hook_param(SBUF(member_acc), member_pkey, 3) != 20)
                NOPE("3mm_govern.c: One or more initial member account ID's is missing");

                                                            // 0... X where X is member id started from 1
                                                            // maps to the member's account ID
            trace(SBUF("3mm_govern.c: Member: "), SBUF(member_acc), 1);
            // reverse key
            ASSERT(state_set(SBUF(member_acc), SVAR(i)) == 20);
            // 0, 0... ACCOUNT ID maps to member_id (as above)

            // forward key
            ASSERT(state_set(SVAR(i), SBUF(member_acc)) == 1);
        }

        DONE("3mm_govern.c: Setup completed successfully.");
    }

    TRACEHEX(hook_accid);
    TRACEHEX(otxn_accid);

    if (BUFFER_EQUAL_20(hook_accid + 12, otxn_accid + 12))
        NOPE("3mm_govern.c: outgoing tx on `Account`.");

    int64_t member_id = state(0, 0, otxn_accid + 12, 20);
    if (member_id < 0)
        NOPE("3mm_govern.c: You are not currently a governance member at this table.");


    // { 'S|H|R', '\0 + topicid' }
    uint8_t topic[2];
    int64_t result = otxn_param(SBUF(topic), "T", 1);
    uint8_t t = topic[0];   // topic type
    uint8_t n = topic[1];   // number (seats)

    if (result != 2 || (
                t != 'S' &&      // topic type: seat (L1)
                t != 'H'))       // topic type: reward
        NOPE("3mm_govern.c: Valid TOPIC must be specified as otxn parameter.");

    if (t == 'S' && n > (SEAT_COUNT - 1))
        NOPE("3mm_govern.c: Valid seat topics are 0 through 19.");

    uint8_t topic_data[32];
    uint8_t topic_size = t == 'H' ? 32 : t == 'S' ? 20 : 0;

    uint8_t padding = 32 - topic_size;

    result = otxn_param(topic_data + padding, topic_size, "V", 1);
    if (result != topic_size)
        NOPE("3mm_govern.c: Missing or incorrect size of VOTE data for TOPIC type.");


    // reuse otxn_accid to create vote key
    otxn_accid[0] = 'V';
    otxn_accid[1] = t;
    otxn_accid[2] = n;

    // get their previous vote if any on this topic
    uint8_t previous_topic_data[32];
    int64_t previous_topic_size =
        state(previous_topic_data + padding, topic_size, SBUF(otxn_accid));

    // check if the vote they're making has already been cast before,
    // if it is identical to their existing vote for this topic then just end with tesSUCCESS
    if (previous_topic_size == topic_size && BUFFER_EQUAL_32(previous_topic_data, topic_data))
        DONE("3mm_govern.c: Your vote is already cast this way for this topic.");

    // execution to here means the vote is different
    // we might have to decrement the old voting if they voted previously
    // and we will have to increment the new voting

    // write vote to their voting key
    ASSERT(state_set(topic_data, topic_size, SBUF(otxn_accid)) == topic_size);
    
    // decrement old vote counter for this option
    if (previous_topic_size > 0)
    {
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
        uint64_t saved_data = *((uint64_t*)topic_data);
        topic_data[0] = 'C';
        topic_data[1] = t; 
        topic_data[2] = n;

        state(&votes, 1, SBUF(topic_data));
        votes++;
        ASSERT(state_set(&votes, 1, SBUF(topic_data)));

        // restore the saved bytes
        *((uint64_t*)topic_data) = saved_data;
    }


    // set this flag if the topic data is all zeros
    uint8_t zero[32];
    int topic_data_zero = BUFFER_EQUAL_32(topic_data, zero); 

    if (DEBUG)
    {
        TRACEVAR(topic_data_zero);
        TRACEVAR(votes);
        TRACEVAR(member_count);
        trace(SBUF("topic"), topic, 2, 1);
    }
    

    int64_t q80 = member_count * 0.8;
    TRACEVAR(q80);

    if (votes <
        t == 'S'
            ? q80                      // 80% threshold for membership/seat voting
            : member_count)            // 100% threshold for all other voting

        DONE("3mm_govern.c: Vote recorded. Not yet enough votes to action.");
    

    // ACCOUNT: Transaction
    accept(SBUF("3mm_govern.c: Finished"), __LINE__);
}

// You can then use these macros in your code as follows:

// ```cpp
// uint8_t previous_member[32];
// int n = 1;
// int member_count = 10;
// uint8_t zero[1] = {0};
// uint8_t topic_data[20] = {0};

// PREVIOUS_MEMBER_PRESENT(previous_member, n)

// if (previous_present && !topic_data_zero)
// {
//     // pass
// }
// else
// {
//     ADJUST_MEMBER_COUNT(previous_present, member_count, zero)
// }

// GARBAGE_COLLECT_VOTES(previous_present, previous_member)

// ADD_NEW_MEMBER(topic_data, n)

// DONE_MESSAGE
// ```

// Please note that macros can make your code harder to read and debug, so use them sparingly and only when necessary.