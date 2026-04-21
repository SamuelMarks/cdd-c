}
else if (tok->length == 13 && memcmp(tok->start, "__builtin_clz", 13) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_clz";
  tok->length = 15;
}
else if (tok->length == 14 && memcmp(tok->start, "__builtin_clzl", 14) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_clzl";
  tok->length = 16;
}
else if (tok->length == 15 && memcmp(tok->start, "__builtin_clzll", 15) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_clzll";
  tok->length = 17;
}
else if (tok->length == 13 && memcmp(tok->start, "__builtin_ctz", 13) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_ctz";
  tok->length = 15;
}
else if (tok->length == 14 && memcmp(tok->start, "__builtin_ctzl", 14) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_ctzl";
  tok->length = 16;
}
else if (tok->length == 15 && memcmp(tok->start, "__builtin_ctzll", 15) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_ctzll";
  tok->length = 17;
}
else if (tok->length == 18 &&
         memcmp(tok->start, "__builtin_popcount", 18) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_popcount";
  tok->length = 20;
}
else if (tok->length == 19 &&
         memcmp(tok->start, "__builtin_popcountl", 19) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_popcountl";
  tok->length = 21;
}
else if (tok->length == 20 &&
         memcmp(tok->start, "__builtin_popcountll", 20) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_popcountll";
  tok->length = 22;
}
else if (tok->length == 13 && memcmp(tok->start, "__builtin_ffs", 13) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_ffs";
  tok->length = 15;
}
else if (tok->length == 14 && memcmp(tok->start, "__builtin_ffsl", 14) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_ffsl";
  tok->length = 16;
}
else if (tok->length == 15 && memcmp(tok->start, "__builtin_ffsll", 15) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_ffsll";
  tok->length = 17;
}
else if (tok->length == 16 && memcmp(tok->start, "__builtin_parity", 16) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_parity";
  tok->length = 18;
}
else if (tok->length == 17 &&
         memcmp(tok->start, "__builtin_parityl", 17) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_parityl";
  tok->length = 19;
}
else if (tok->length == 18 &&
         memcmp(tok->start, "__builtin_parityll", 18) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_parityll";
  tok->length = 20;
}
else if (tok->length == 17 &&
         memcmp(tok->start, "__builtin_bswap16", 17) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_bswap16";
  tok->length = 19;
}
else if (tok->length == 17 &&
         memcmp(tok->start, "__builtin_bswap32", 17) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_bswap32";
  tok->length = 19;
}
else if (tok->length == 17 &&
         memcmp(tok->start, "__builtin_bswap64", 17) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_bswap64";
  tok->length = 19;
}
else if (tok->length == 15 && memcmp(tok->start, "__builtin_clrsb", 15) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_clrsb";
  tok->length = 17;
}
else if (tok->length == 16 && memcmp(tok->start, "__builtin_clrsbl", 16) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_clrsbl";
  tok->length = 18;
}
else if (tok->length == 17 &&
         memcmp(tok->start, "__builtin_clrsbll", 17) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_clrsbll";
  tok->length = 19;
}
else if (tok->length == 16 && memcmp(tok->start, "__builtin_alloca", 16) == 0) {
  tok->start = (const uint8_t *)"alloca";
  tok->length = 6;
}
else if (tok->length == 27 &&
         memcmp(tok->start, "__builtin_alloca_with_align", 27) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_alloca_with_align";
  tok->length = 29;
}
else if (tok->length == 16 && memcmp(tok->start, "__builtin_memcpy", 16) == 0) {
  tok->start = (const uint8_t *)"memcpy";
  tok->length = 6;
}
else if (tok->length == 16 && memcmp(tok->start, "__builtin_memset", 16) == 0) {
  tok->start = (const uint8_t *)"memset";
  tok->length = 6;
}
else if (tok->length == 16 && memcmp(tok->start, "__builtin_memcmp", 16) == 0) {
  tok->start = (const uint8_t *)"memcmp";
  tok->length = 6;
}
else if (tok->length == 16 && memcmp(tok->start, "__builtin_strcpy", 16) == 0) {
  tok->start = (const uint8_t *)"strcpy";
  tok->length = 6;
}
else if (tok->length == 16 && memcmp(tok->start, "__builtin_strlen", 16) == 0) {
  tok->start = (const uint8_t *)"strlen";
  tok->length = 6;
}
else if (tok->length == 17 &&
         memcmp(tok->start, "__builtin_strncpy", 17) == 0) {
  tok->start = (const uint8_t *)"strncpy";
  tok->length = 7;
}
else if (tok->length == 16 && memcmp(tok->start, "__builtin_strcat", 16) == 0) {
  tok->start = (const uint8_t *)"strcat";
  tok->length = 6;
}
else if (tok->length == 17 &&
         memcmp(tok->start, "__builtin_strncat", 17) == 0) {
  tok->start = (const uint8_t *)"strncat";
  tok->length = 7;
}
else if (tok->length == 16 && memcmp(tok->start, "__builtin_strchr", 16) == 0) {
  tok->start = (const uint8_t *)"strchr";
  tok->length = 6;
}
else if (tok->length == 17 &&
         memcmp(tok->start, "__builtin_strrchr", 17) == 0) {
  tok->start = (const uint8_t *)"strrchr";
  tok->length = 7;
}
else if (tok->length == 16 && memcmp(tok->start, "__builtin_strcmp", 16) == 0) {
  tok->start = (const uint8_t *)"strcmp";
  tok->length = 6;
}
else if (tok->length == 17 &&
         memcmp(tok->start, "__builtin_strncmp", 17) == 0) {
  tok->start = (const uint8_t *)"strncmp";
  tok->length = 7;
}
else if (tok->length == 16 && memcmp(tok->start, "__builtin_strstr", 16) == 0) {
  tok->start = (const uint8_t *)"strstr";
  tok->length = 6;
}
else if (tok->length == 17 &&
         memcmp(tok->start, "__builtin_strpbrk", 17) == 0) {
  tok->start = (const uint8_t *)"strpbrk";
  tok->length = 7;
}
else if (tok->length == 16 && memcmp(tok->start, "__builtin_strspn", 16) == 0) {
  tok->start = (const uint8_t *)"strspn";
  tok->length = 6;
}
else if (tok->length == 17 &&
         memcmp(tok->start, "__builtin_strcspn", 17) == 0) {
  tok->start = (const uint8_t *)"strcspn";
  tok->length = 7;
}
else if (tok->length == 16 && memcmp(tok->start, "__builtin_memchr", 16) == 0) {
  tok->start = (const uint8_t *)"memchr";
  tok->length = 6;
}
else if (tok->length == 22 &&
         memcmp(tok->start, "__builtin___memcpy_chk", 22) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_memcpy_chk";
  tok->length = 22;
}
else if (tok->length == 22 &&
         memcmp(tok->start, "__builtin___memset_chk", 22) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_memset_chk";
  tok->length = 22;
}
else if (tok->length == 23 &&
         memcmp(tok->start, "__builtin___memmove_chk", 23) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_memmove_chk";
  tok->length = 23;
}
else if (tok->length == 22 &&
         memcmp(tok->start, "__builtin___stpcpy_chk", 22) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_stpcpy_chk";
  tok->length = 22;
}
else if (tok->length == 22 &&
         memcmp(tok->start, "__builtin___strcat_chk", 22) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_strcat_chk";
  tok->length = 22;
}
else if (tok->length == 22 &&
         memcmp(tok->start, "__builtin___strcpy_chk", 22) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_strcpy_chk";
  tok->length = 22;
}
else if (tok->length == 23 &&
         memcmp(tok->start, "__builtin___strncat_chk", 23) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_strncat_chk";
  tok->length = 23;
}
else if (tok->length == 23 &&
         memcmp(tok->start, "__builtin___strncpy_chk", 23) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_strncpy_chk";
  tok->length = 23;
}
else if (tok->length == 21 &&
         memcmp(tok->start, "__builtin_object_size", 21) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_object_size";
  tok->length = 23;
}
else if (tok->length == 29 &&
         memcmp(tok->start, "__builtin_dynamic_object_size", 29) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_dynamic_object_size";
  tok->length = 31;
}
else if (tok->length == 22 &&
         memcmp(tok->start, "__builtin_add_overflow", 22) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_add_overflow";
  tok->length = 24;
}
else if (tok->length == 22 &&
         memcmp(tok->start, "__builtin_sub_overflow", 22) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_sub_overflow";
  tok->length = 24;
}
else if (tok->length == 22 &&
         memcmp(tok->start, "__builtin_mul_overflow", 22) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_mul_overflow";
  tok->length = 24;
}
else if (tok->length == 18 &&
         memcmp(tok->start, "__builtin_offsetof", 18) == 0) {
  tok->start = (const uint8_t *)"offsetof";
  tok->length = 8;
}
else if (tok->length == 23 &&
         memcmp(tok->start, "__builtin_frame_address", 23) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_frame_address";
  tok->length = 25;
}
else if (tok->length == 24 &&
         memcmp(tok->start, "__builtin_return_address", 24) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_return_address";
  tok->length = 26;
}
else if (tok->length == 14 && memcmp(tok->start, "__builtin_trap", 14) == 0) {
  tok->start = (const uint8_t *)"abort";
  tok->length = 5;
}
else if (tok->length == 16 && memcmp(tok->start, "__builtin_setjmp", 16) == 0) {
  tok->start = (const uint8_t *)"setjmp";
  tok->length = 6;
}
else if (tok->length == 17 &&
         memcmp(tok->start, "__builtin_longjmp", 17) == 0) {
  tok->start = (const uint8_t *)"longjmp";
  tok->length = 7;
}
else if (tok->length == 21 &&
         memcmp(tok->start, "__builtin_va_arg_pack", 21) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_va_arg_pack";
  tok->length = 23;
}
else if (tok->length == 25 &&
         memcmp(tok->start, "__builtin_va_arg_pack_len", 25) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_va_arg_pack_len";
  tok->length = 27;
}
else if (tok->length == 20 &&
         memcmp(tok->start, "__builtin_apply_args", 20) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_apply_args";
  tok->length = 22;
}
else if (tok->length == 15 && memcmp(tok->start, "__builtin_apply", 15) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_apply";
  tok->length = 17;
}
else if (tok->length == 16 && memcmp(tok->start, "__builtin_return", 16) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_return";
  tok->length = 18;
}
else if (tok->length == 20 &&
         memcmp(tok->start, "__sync_fetch_and_add", 20) == 0) {
  tok->start = (const uint8_t *)"cdd_sync_fetch_and_add";
  tok->length = 22;
}
else if (tok->length == 20 &&
         memcmp(tok->start, "__sync_sub_and_fetch", 20) == 0) {
  tok->start = (const uint8_t *)"cdd_sync_sub_and_fetch";
  tok->length = 22;
}
else if (tok->length == 28 &&
         memcmp(tok->start, "__sync_bool_compare_and_swap", 28) == 0) {
  tok->start = (const uint8_t *)"cdd_sync_bool_compare_and_swap";
  tok->length = 30;
}
else if (tok->length == 15 && memcmp(tok->start, "__atomic_load_n", 15) == 0) {
  tok->start = (const uint8_t *)"cdd_atomic_load_n";
  tok->length = 17;
}
else if (tok->length == 16 && memcmp(tok->start, "__atomic_store_n", 16) == 0) {
  tok->start = (const uint8_t *)"cdd_atomic_store_n";
  tok->length = 18;
}
else if (tok->length == 19 &&
         memcmp(tok->start, "__atomic_exchange_n", 19) == 0) {
  tok->start = (const uint8_t *)"cdd_atomic_exchange_n";
  tok->length = 21;
}
else if (tok->length == 27 &&
         memcmp(tok->start, "__atomic_compare_exchange_n", 27) == 0) {
  tok->start = (const uint8_t *)"cdd_atomic_compare_exchange_n";
  tok->length = 29;
}
else if (tok->length == 21 &&
         memcmp(tok->start, "__atomic_thread_fence", 21) == 0) {
  tok->start = (const uint8_t *)"cdd_atomic_thread_fence";
  tok->length = 23;
}
else if (tok->length == 21 &&
         memcmp(tok->start, "__atomic_signal_fence", 21) == 0) {
  tok->start = (const uint8_t *)"cdd_atomic_signal_fence";
  tok->length = 23;
