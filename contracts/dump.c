uint8_t nav_buffer[8];
uint8_t nav_key[3] = {'n', 'a', 'v'};
uint8_t ns_hash[32];
if (util_sha512h(ns_hash, 32, nav_key, 3) < 0)
    rollback("Could not generate Namespace Hash", 23, 1);

if (state_foreign(SBUF(nav_buffer), SBUF(HOOK_ACC), SBUF(ns_hash), SBUF(HOOK_ACC)) != SFS_ACCOUNT)
{
    DONE("mutualfund: No current NAV");
}

uint64_t price_xfl = FLIP_ENDIAN_64(INT64_FROM_BUF(nav_buffer));


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