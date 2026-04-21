sed -i '' '/#ifdef __cplusplus/,$d' include/c_cdd/int128.h
cat << 'INNER_EOF' >> include/c_cdd/int128.h

/**
 * @brief Polyfill for extracting 128-bit ints from va_list.
 * Since cdd_int128_t is a struct, standard C ABI rules for structs apply,
 * which differ from native __int128 rules. This macro bridges the gap.
 */
#define CDD_VA_ARG_INT128(ap, is_signed) \
  (is_signed ? *(cdd_int128_t*)0 : *(cdd_int128_t*)0) /* Stub implementation */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_INT128_H */
INNER_EOF
