/**
 * @file test_safe_crt_msvc_unit.c
 * @brief Exhaustive unit and OOM tests for `include/c_cdd/safe_crt_msvc.h`.
 */

/* clang-format off */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** @brief Fault-injection mode for `cdd_test_msvc_dupenv_s`. */
static int g_test_dupenv_mode = 0;
/** @brief Fault-injection mode for `cdd_test_msvc_fopen_s`. */
static int g_test_fopen_mode = 0;

/**
 * @brief Mock `_dupenv_s` implementation for Safe CRT unit testing.
 * @param[out] pbuf Output allocated string buffer.
 * @param[out] psz Output string byte length.
 * @param[in] name Environment variable name.
 * @return 0 on success, or errno code on simulated OOM.
 */
static int cdd_test_msvc_dupenv_s(char **pbuf, size_t *psz, const char *name) {
  size_t nlen;
  if (g_test_dupenv_mode == 1) {
    *pbuf = 0;
    *psz = 0;
    return ENOMEM;
  }
  if (g_test_dupenv_mode == 2) {
    *pbuf = 0;
    *psz = 0;
    return 0;
  }
  nlen = strlen(name);
  *pbuf = (char *)malloc(nlen + 5);
  if (!*pbuf)
    return ENOMEM;
  memcpy(*pbuf, "val_", 4);
  memcpy(*pbuf + 4, name, nlen + 1);
  *psz = nlen + 5;
  return 0;
}

/**
 * @brief Mock `strncpy_s` implementation for Safe CRT unit testing.
 * @param[out] dst Destination buffer.
 * @param[in] cap Destination buffer capacity.
 * @param[in] src Source string.
 */
static void cdd_test_msvc_strncpy_s(char *dst, size_t cap, const char *src) {
  size_t slen = strlen(src);
  if (slen >= cap)
    slen = cap - 1;
  memcpy(dst, src, slen);
  dst[slen] = '\0';
}

/**
 * @brief Mock `fopen_s` implementation for Safe CRT unit testing.
 * @param[out] pf Output `FILE *` pointer.
 * @param[in] path File path.
 * @param[in] mode Access mode.
 * @return 0 on success, or errno code on failure.
 */
static int cdd_test_msvc_fopen_s(FILE **pf, const char *path,
                                 const char *mode) {
  if (g_test_fopen_mode == 1) {
    *pf = 0;
    return ENOENT;
  }
  if (g_test_fopen_mode == 2) {
    *pf = 0;
    return 0;
  }
#if defined(_MSC_VER)
  return (int)fopen_s(pf, path, mode);
#else
  *pf = fopen(path, mode);
  return (*pf != 0) ? 0 : ENOENT;
#endif
}

/** @brief Enables compilation of Safe CRT inline wrappers on non-MSVC hosts. */
#define CDD_TEST_SAFE_CRT_MSVC 1
/** @brief Overrides `_dupenv_s` with deterministic test mock. */
#define CDD_SAFE_CRT_DUPENV_S(pbuf, psz, name)                                 \
  cdd_test_msvc_dupenv_s((pbuf), (psz), (name))
/** @brief Overrides `strncpy_s` with deterministic test mock. */
#define CDD_SAFE_CRT_STRNCPY_S(dst, cap, src)                                  \
  cdd_test_msvc_strncpy_s((dst), (cap), (src))
/** @brief Overrides `free` for Safe CRT test mock buffers. */
#define CDD_SAFE_CRT_FREE(ptr) free((ptr))
/** @brief Overrides `fopen_s` with deterministic test mock. */
#define CDD_SAFE_CRT_FOPEN_S(pf, path, mode)                                   \
  cdd_test_msvc_fopen_s((pf), (path), (mode))

#include "c_cdd/safe_crt_msvc.h"
#include <greatest.h>
/* clang-format on */

/**
 * @brief Unit test covering 100% of branches in `cdd_getenv_out` and
 * `cdd_fopen_out`.
 * @return Greatest test result.
 */
TEST test_safe_crt_msvc_wrappers(void) {
  char sentinel_buf[] = "sentinel";
  char *val = sentinel_buf;
  FILE *fp = 0;
  int i;

  /* 1. NULL name with out_val non-NULL and NULL */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_getenv_out(0, &val));
  ASSERT_EQ((char *)0, val);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_getenv_out(0, 0));

  /* 2. Simulated _dupenv_s OOM failure with out_val non-NULL and NULL */
  g_test_dupenv_mode = 1;
  val = sentinel_buf;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_getenv_out("MY_KEY", &val));
  ASSERT_EQ((char *)0, val);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_getenv_out("MY_KEY", 0));

  /* 3. Unset environment variable (rc == 0, buf == NULL) */
  g_test_dupenv_mode = 2;
  val = sentinel_buf;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_getenv_out("UNSET_KEY", &val));
  ASSERT_EQ((char *)0, val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_getenv_out("UNSET_KEY", 0));

  /* 4. Present environment variable across >8 calls to wrap ring buffer */
  g_test_dupenv_mode = 0;
  for (i = 0; i < 10; i++) {
    ASSERT_EQ(CDD_C_SUCCESS, cdd_getenv_out("RING_VAR", &val));
    ASSERT(val != 0);
    ASSERT_STR_EQ("val_RING_VAR", val);
  }
  ASSERT_EQ(CDD_C_SUCCESS, cdd_getenv_out("RING_VAR_NO_OUT", 0));

  /* 5. cdd_fopen_out invalid arguments */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_fopen_out("a.txt", "r", 0));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_fopen_out(0, "r", &fp));
  ASSERT_EQ((FILE *)0, fp);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_fopen_out("a.txt", 0, &fp));
  ASSERT_EQ((FILE *)0, fp);

  /* 6. cdd_fopen_out IO failures (nonzero rc and zero rc with NULL FILE*) */
  g_test_fopen_mode = 1;
  ASSERT_EQ(CDD_C_ERROR_IO, cdd_fopen_out("missing.txt", "r", &fp));
  ASSERT_EQ((FILE *)0, fp);

  g_test_fopen_mode = 2;
  ASSERT_EQ(CDD_C_ERROR_IO, cdd_fopen_out("null_fp.txt", "r", &fp));
  ASSERT_EQ((FILE *)0, fp);

  /* 7. cdd_fopen_out success */
  g_test_fopen_mode = 0;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_fopen_out("cdd_safe_crt_msvc_tmp.txt", "w+", &fp));
  ASSERT(fp != 0);
  fclose(fp);
  remove("cdd_safe_crt_msvc_tmp.txt");

  PASS();
}

/**
 * @brief Suite for `include/c_cdd/safe_crt_msvc.h`.
 */
SUITE(safe_crt_msvc_suite) { RUN_TEST(test_safe_crt_msvc_wrappers); }
