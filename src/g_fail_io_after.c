/* clang-format off */
#include "c_cdd_export.h"
/* clang-format on */
C_CDD_EXPORT int g_fail_io_after = -1;
C_CDD_EXPORT int g_io_calls = 0;

C_CDD_EXPORT int g_crypto_fail_digestfinal = 0;
C_CDD_EXPORT int g_crypto_fail_digestfinal_len = 0;
C_CDD_EXPORT int g_crypto_fail_digestinit = 0;
C_CDD_EXPORT int g_crypto_fail_digestupdate = 0;
C_CDD_EXPORT int g_crypto_fail_hmac = 0;
C_CDD_EXPORT int g_crypto_fail_hmac_len = 0;
C_CDD_EXPORT int g_crypto_fail_mdctx_new = 0;
C_CDD_EXPORT int g_crypto_fail_sha256 = 0;

C_CDD_EXPORT int g_cdd_mock_dlopen_success = 0;
C_CDD_EXPORT int g_cdd_alloc_fail = 0;
C_CDD_EXPORT int g_cdd_fprintf_fail = 0;
C_CDD_EXPORT int g_cdd_strdup_fail = 0;
C_CDD_EXPORT int g_cdd_fail_alloc = 0;
C_CDD_EXPORT int g_str_unquote_malloc_fail = 0;
C_CDD_EXPORT int g_cdd_fail_token_matches_string = 0;
C_CDD_EXPORT int g_parse_binary_str_fail = 0;
C_CDD_EXPORT int g_is_basic_type_keyword_fail = 0;
C_CDD_EXPORT int g_cdd_fail_stricmp = 0;
C_CDD_EXPORT int g_cdd_fail_count_returning_allocs = 0;
C_CDD_EXPORT int g_cdd_fail_find_balanced_end = 0;
C_CDD_EXPORT int g_cdd_fail_parsed_sig_init = 0;
C_CDD_EXPORT int g_cdd_fail_args_represent_void = 0;
C_CDD_EXPORT int g_cdd_fail_is_inside_condition = 0;
C_CDD_EXPORT int g_cdd_fail_is_dereference_use = 0;
C_CDD_EXPORT int g_cdd_fail_is_checked = 0;
C_CDD_EXPORT int g_cdd_fail_vla_basic_type = 0;
C_CDD_EXPORT int g_cdd_fail_str_after_last = 0;
C_CDD_EXPORT int g_cdd_fail_find_next_token_idx = 0;
C_CDD_EXPORT int g_cdd_fail_patch_list_sort = 0;
C_CDD_EXPORT int g_cdd_fail_str_starts_with = 0;
C_CDD_EXPORT int g_cdd_fail_skip_qualifiers = 0;
C_CDD_EXPORT int g_cdd_fail_get_basename = 0;
C_CDD_EXPORT int g_cdd_fail_server_apply = 0;
C_CDD_EXPORT int g_cdd_fail_sync_matches = 0;
C_CDD_EXPORT int g_cdd_fail_sync_patch_add = 0;
