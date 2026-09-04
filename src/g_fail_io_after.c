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
