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