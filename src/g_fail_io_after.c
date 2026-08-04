/* clang-format off */
#include "c_cdd_export.h"
/* clang-format on */
C_CDD_EXPORT int g_fail_io_after = -1;
C_CDD_EXPORT int g_io_calls = 0;

int g_crypto_fail_digestfinal = 0;
int g_crypto_fail_digestfinal_len = 0;
int g_crypto_fail_digestinit = 0;
int g_crypto_fail_digestupdate = 0;
int g_crypto_fail_hmac = 0;
int g_crypto_fail_hmac_len = 0;
int g_crypto_fail_mdctx_new = 0;
int g_crypto_fail_sha256 = 0;
