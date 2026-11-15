/**
 * @file test_crt_bridge_unit.c
 * @brief Exhaustive unit and fault-injection tests for `cdd_crt_bridge.h`.
 */

/* clang-format off */
/** @brief Enables compilation of CRT bridge wrappers on non-MSVC hosts. */
#define CDD_TEST_CRT_BRIDGE 1
/** @brief Suppresses CRT macro redirections while testing wrapper functions. */
#define CDD_CRT_BRIDGE_NO_MACROS 1
#include "cdd_test_helpers/cdd_crt_bridge.h"
#include <greatest.h>
/* clang-format on */

/**
 * @brief Helper invoking `test_crt_vasprintf` with variadic arguments.
 * @param[out] out_str Output string pointer.
 * @param[in] fmt Printf format string.
 * @return Characters formatted or -1 on error.
 */
#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 2, 3)))
#endif
static int
call_test_crt_vasprintf(char **out_str, const char *fmt, ...) {
  va_list ap;
  int rc;
  va_start(ap, fmt);
  rc = test_crt_vasprintf(out_str, fmt, ap);
  va_end(ap);
  return rc;
}

/**
 * @brief Unit test exercising 100% of functions, lines, and branches in
 * `src/tests/cdd_test_helpers/cdd_crt_bridge.h`.
 * @return Greatest test result.
 */
TEST test_crt_bridge_header_all_branches(void) {
  void *p = NULL;
  char *s = NULL;
  FILE *fp = NULL;
  FILE *fp2 = NULL;
  fpos_t fpos;
  char rbuf[32];
  int fd = -1;
  int dup_fd = -1;
  int dup_target = -1;

  g_crt_bridge_fail = 0;

  /* 1. test_crt_resolve_stream: NULL, stdin, stdout, stderr, custom, and fail
   */
  ASSERT_EQ((FILE *)NULL, test_crt_resolve_stream(NULL));
  ASSERT(test_crt_resolve_stream(stdin) != NULL);
  ASSERT(test_crt_resolve_stream(stdout) != NULL);
  ASSERT(test_crt_resolve_stream(stderr) != NULL);

  g_crt_bridge_fail = 2;
  ASSERT(test_crt_resolve_stream(stdin) != NULL);
  ASSERT_EQ(stdin, test_crt_resolve_stream(stdin));
  g_crt_bridge_fail = 1;
  ASSERT_EQ(stdout, test_crt_resolve_stream(stdout));
  g_crt_bridge_fail = 1;
  ASSERT_EQ(stderr, test_crt_resolve_stream(stderr));
  g_crt_bridge_fail = 0;

  /* 2. Heap wrappers: malloc, calloc, realloc, strdup + OOM */
  p = test_crt_malloc(16);
  ASSERT(p != NULL);
  p = test_crt_realloc(p, 32);
  ASSERT(p != NULL);
  c_cdd_crt_free(p);

  p = test_crt_calloc(4, 4);
  ASSERT(p != NULL);
  c_cdd_crt_free(p);

  s = test_crt_strdup("bridge_ok");
  ASSERT(s != NULL);
  ASSERT_STR_EQ("bridge_ok", s);
  c_cdd_crt_free(s);

  g_crt_bridge_fail = 1;
  ASSERT_EQ(NULL, test_crt_malloc(16));
  g_crt_bridge_fail = 1;
  ASSERT_EQ(NULL, test_crt_calloc(4, 4));
  g_crt_bridge_fail = 1;
  ASSERT_EQ(NULL, test_crt_realloc(NULL, 16));
  g_crt_bridge_fail = 1;
  ASSERT_EQ(NULL, test_crt_strdup("oom"));
  g_crt_bridge_fail = 0;

  /* 3. fopen / fopen_s / freopen / freopen_s across success, err!=0, err==0 */
  ASSERT_EQ((FILE *)NULL, test_crt_fopen(NULL, "r"));
  g_crt_bridge_fail = 2;
  fp = test_crt_fopen("cdd_crt_hdr_tmp.txt", "w+");
  ASSERT(fp != NULL);
  ASSERT_EQ((FILE *)NULL, test_crt_fopen("cdd_crt_hdr_tmp.txt", "r"));
  ASSERT_EQ(CDD_C_ERROR_IO,
            (g_crt_bridge_fail = 1,
             c_cdd_crt_fopen("cdd_crt_hdr_tmp.txt", "r", &fp2, NULL)));
  ASSERT_EQ(0, test_crt_fclose(fp));

  ASSERT_EQ(EINVAL, test_crt_fopen_s(NULL, "cdd_crt_hdr_tmp.txt", "r"));
  ASSERT_NEQ(0, test_crt_fopen_s(&fp, NULL, "r"));
  g_crt_bridge_fail = 1;
  ASSERT_EQ(EINVAL, test_crt_fopen_s(&fp, "cdd_crt_hdr_tmp.txt", "r"));
  ASSERT_EQ(0, test_crt_fopen_s(&fp, "cdd_crt_hdr_tmp.txt", "w+"));
  ASSERT(fp != NULL);

  ASSERT_EQ((FILE *)NULL, test_crt_freopen(NULL, "r", fp));
  g_crt_bridge_fail = 2;
  fp = test_crt_freopen("cdd_crt_hdr_tmp2.txt", "w+", fp);
  ASSERT(fp != NULL);
  /* Next call hits --g_crt_bridge_fail == 0 (err == 0) without closing fp */
  ASSERT_EQ((FILE *)NULL, test_crt_freopen("cdd_crt_hdr_tmp2.txt", "w+", fp));
  ASSERT_EQ(CDD_C_ERROR_IO,
            (g_crt_bridge_fail = 1,
             c_cdd_crt_freopen("cdd_crt_hdr_tmp2.txt", "w+", fp, &fp2, NULL)));

  ASSERT_EQ(EINVAL, test_crt_freopen_s(NULL, "cdd_crt_hdr_tmp2.txt", "w+", fp));
  ASSERT_NEQ(0, test_crt_freopen_s(&fp2, NULL, "w+", fp));
  g_crt_bridge_fail = 1;
  ASSERT_EQ(EINVAL, test_crt_freopen_s(&fp2, "cdd_crt_hdr_tmp2.txt", "w+", fp));
  ASSERT_EQ(0, test_crt_freopen_s(&fp2, "cdd_crt_hdr_tmp.txt", "w+", fp));
  fp = fp2;
  ASSERT(fp != NULL);

  /* 4. Stream operations: fwrite, fputs, fprintf, fflush, fseek, ftell, rewind,
   *    fgetpos, fsetpos, fread, fgets, clearerr, fclose */
  ASSERT_EQ(3u, test_crt_fwrite("abc", 1, 3, fp));
  ASSERT_EQ(0u, test_crt_fwrite("abc", 1, 3, NULL));
  ASSERT(test_crt_fputs("def\n", fp) >= 0);
  ASSERT_EQ(EOF, test_crt_fputs("def\n", NULL));
  ASSERT(test_crt_fprintf(fp, "%d\n", 42) > 0);
  ASSERT(test_crt_fprintf(NULL, "%d\n", 42) < 0);

  ASSERT_EQ(0, test_crt_fflush(NULL));
  ASSERT_EQ(0, test_crt_fflush(stdout));
  ASSERT_EQ(0, test_crt_fflush(stderr));
  ASSERT_EQ(0, test_crt_fflush(fp));
  g_crt_bridge_fail = 1;
  ASSERT_EQ(EOF, test_crt_fflush(fp));

  ASSERT_EQ(0, test_crt_fseek(fp, 0L, SEEK_SET));
  ASSERT_NEQ(0, test_crt_fseek(NULL, 0L, SEEK_SET));
  ASSERT_EQ(0L, test_crt_ftell(fp));
  ASSERT_EQ(-1L, test_crt_ftell(NULL));

  test_crt_rewind(fp);
  test_crt_rewind(NULL);

  ASSERT_EQ(0, test_crt_fgetpos(fp, &fpos));
  ASSERT_NEQ(0, test_crt_fgetpos(NULL, &fpos));
  ASSERT_EQ(0, test_crt_fsetpos(fp, &fpos));
  ASSERT_NEQ(0, test_crt_fsetpos(NULL, &fpos));

  memset(rbuf, 0, sizeof(rbuf));
  ASSERT_EQ(3u, test_crt_fread(rbuf, 1, 3, fp));
  ASSERT_EQ(0u, test_crt_fread(rbuf, 1, 3, NULL));

  memset(rbuf, 0, sizeof(rbuf));
  ASSERT(test_crt_fgets(rbuf, (int)sizeof(rbuf), fp) != NULL);
  ASSERT_EQ(NULL, test_crt_fgets(rbuf, (int)sizeof(rbuf), NULL));

  test_crt_clearerr(stdin);
  test_crt_clearerr(stdout);
  test_crt_clearerr(stderr);
  test_crt_clearerr(fp);
  test_crt_clearerr(NULL);

  /* 5. Low-level fd & putenv wrappers */
  fd = test_crt_fileno(fp);
  ASSERT(fd >= 0);
  ASSERT_EQ(-1, test_crt_fileno(NULL));

  dup_fd = test_crt_dup(fd);
  ASSERT(dup_fd >= 0);
  ASSERT_EQ(-1, test_crt_dup(-1));

  dup_target = test_crt_dup(fd);
  ASSERT(dup_target >= 0);
  ASSERT(test_crt_dup2(dup_fd, dup_target) >= 0);
  ASSERT_EQ(-1, test_crt_dup2(-1, -1));

  ASSERT_EQ(0, test_crt_close(dup_fd));
  ASSERT_EQ(0, test_crt_close(dup_target));
  ASSERT_EQ(-1, test_crt_close(-1));

  ASSERT_EQ(0, test_crt_fclose(fp));
  ASSERT_EQ(EOF, test_crt_fclose(NULL));

  ASSERT_EQ(0, test_crt_putenv("CDD_CRT_HDR_ENV=1"));
  ASSERT_EQ(-1, test_crt_putenv(""));

  /* 6. vasprintf, asprintf, jasprintf */
  s = NULL;
  ASSERT(call_test_crt_vasprintf(&s, "v_%d", 7) >= 0);
  ASSERT_STR_EQ("v_7", s);
  c_cdd_crt_free(s);
  ASSERT_EQ(-1, call_test_crt_vasprintf(NULL, "v_%d", 7));

  s = NULL;
  ASSERT(test_crt_asprintf(&s, "a_%d", 9) >= 0);
  ASSERT_STR_EQ("a_9", s);
  c_cdd_crt_free(s);
  ASSERT_EQ(-1, test_crt_asprintf(NULL, "a_%d", 9));

  s = NULL;
  ASSERT(test_crt_jasprintf(&s, "hello") >= 0);
  ASSERT(test_crt_jasprintf(&s, "_world") >= 0);
  ASSERT_STR_EQ("hello_world", s);
  c_cdd_crt_free(s);

  g_crt_bridge_fail = 1;
  s = NULL;
  ASSERT_EQ(-1, test_crt_jasprintf(&s, "oom"));
  ASSERT_EQ(-1, test_crt_jasprintf(NULL, "bad_unto"));

  remove("cdd_crt_hdr_tmp.txt");
  remove("cdd_crt_hdr_tmp2.txt");
  g_crt_bridge_fail = 0;
  PASS();
}

/**
 * @brief Suite for `src/tests/cdd_test_helpers/cdd_crt_bridge.h`.
 */
SUITE(crt_bridge_header_suite) {
  RUN_TEST(test_crt_bridge_header_all_branches);
}
