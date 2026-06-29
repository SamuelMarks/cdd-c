extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
/**
 * @file test_safe_crt.h
 * @brief Unit tests for the Safe CRT transformer.
 */

#ifndef TEST_CDD_TRANSFORM_SAFE_CRT_H
#define TEST_CDD_TRANSFORM_SAFE_CRT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <string.h>
#include <stdlib.h>
#include "cdd_cst_transform.h"
#include "classes/parse/cdd_cst_parser.h"
#include "classes/emit/cdd_cst_emit.h"
#include "c_str_span.h"
/* clang-format on */

TEST test_cdd_transform_safe_crt(void) {
  cdd_cst_tree_t *tree = NULL;

  const char *code =
      "#define MY_MACRO_CONST 1024\n"
      "void foo() {\n"
      "  char dest[10];\n"
      "  char dest2[10];\n"
      "  char dest3[10];\n"
      "  wchar_t wdest[10];\n"
      "  enum { MY_CONST = 512 };\n"
      "  char *dyn_buf = malloc(256);\n"
      "  char *c_buf = calloc(10, 2);\n"
      "  char *r_buf = realloc(NULL, 1024);\n"
      "  char *enum_buf = malloc(MY_CONST);\n"
      "  char *mac_buf = malloc(MY_MACRO_CONST);\n"
      "  const int MY_INT_CONST = 128;\n"
      "  char *const_buf = malloc(MY_INT_CONST);\n"
      "  typedef char MyChar;\n"
      "  MyChar t_buf[64];\n"
      "  int idx;\n"
      "  char ch;\n"
      "  strcpy(dyn_buf, \"test\");\n"
      "  strcpy(c_buf, \"test\");\n"
      "  strcpy(r_buf, \"test\");\n"
      "  strcpy(enum_buf, \"test\");\n"
      "  strcpy(mac_buf, \"test\");\n"
      "  strcpy(const_buf, \"test\");\n"
      "  strcpy(t_buf, \"test\");\n"
      "  strcpy(dest + 2, \"test\");\n"
      "  strcpy(&dest[3], \"test\");\n"

      "  int idx;\n"
      "  char ch;\n"
      "  double d = 1.23;\n"
      "  char *p = _gcvt(d, 5, dest);\n"
      "  mbstowcs(wdest, \"abc\", 3);\n"
      "  wcstombs(dest, L\"abc\", 3);\n"
      "  wctomb(dest, L'a');\n"
      "  strcpy(dest, \"abc\");\n"
      "  strncpy(dest, \"def\", 3);\n"
      "  sprintf(dest, \"%d\", 1);\n"
      "  FILE *f;\n"
      "  f = fopen(\"a.txt\", \"r\");\n"
      "  FILE *f2 = fopen(\"b.txt\", \"r\");\n"
      "  strcat(dest, \"x\");\n"
      "  strncat(dest, \"y\", 1);\n"
      "  memcpy(dest, \"z\", 1);\n"
      "  memmove(dest, \"w\", 1);\n"
      "  strcpy(dest, strcpy(dest, \"nested\"));\n"
      "  snprintf(dest, 10, \"%d\", 2);\n"
      "  printf(\"%s\", dest);\n"
      "  fprintf(f, \"%s\", dest);\n"
      "  scanf(\"%s %d %c\", dest, &idx, &ch);\n"
      "  fscanf(f, \"%10s\", dest);\n"
      "  sscanf(dest, \"%[a-z] %% %*d %s\", dest2, dest3);\n"
      "  _itoa(idx, dest, 10);\n"
      "  _mbscpy(dest, \"abc\");\n"
      "  _mbsncpy(dest, \"abc\", 2);\n"
      "  _mbscat(dest, \"abc\");\n"
      "  _mbsncat(dest, \"abc\", 2);\n"
      "  _mbslwr(dest);\n"
      "  _mbsupr(dest);\n"
      "  _mbsset(dest, 'a');\n"
      "  _mbsnset(dest, 'a', 2);\n"
      "  _wcslwr(wdest);\n"
      "  _wcsupr(wdest);\n"
      "  f = freopen(\"a.txt\", \"w\", f);\n"
      "  FILE *f3 = _wfopen(L\"a.txt\", L\"r\");\n"
      "  FILE *f4 = tmpfile();\n"
      "  _splitpath(dest, dest, dest, dest, dest);\n"
      "  _makepath(dest, dest, dest, dest, dest);\n"
      "  _searchenv(\"x\", \"PATH\", dest);\n"
      "  _wsearchenv(L\"x\", L\"PATH\", wdest);\n"
      "  p = getenv(\"PATH\");\n"
      "  wdest[0] = (wchar_t)_wgetenv(L\"PATH\");\n"
      "  _putenv(\"A=B\");\n"
      "  _wputenv(L\"A=B\");\n"
      "  qsort(dest, 10, 1, foo);\n"
      "  bsearch(\"a\", dest, 10, 1, foo);\n"
      "  1 ? strcpy(dest, \"b\") : strcpy(dest, \"c\");\n"
      "  (void)0, strcpy(dest, \"d\");\n"
      "  if (1) strcpy(dest, \"e\");\n"
      "  for (;;) strcpy(dest, \"f\");\n"
      "  idx = (int)strlen(dest);\n"
      "  strerror(1);\n"
      "  _wcserror(1);\n"
      "  strtok(dest, \"a\");\n"
      "  wcstok(wdest, L\"a\");\n"
      "  _mbstok(dest, \"a\");\n"
      "  _ecvt(d, 1, &idx, &idx);\n"
      "  _fcvt(d, 1, &idx, &idx);\n"
      "  /* prefix */ strcpy(dest, /* src */ \"g\" /* suffix */);\n"
      "}\n"
      "#define MY_COPY_MACRO(a, b) strcpy(a, b)\n"
      "void bar() { MY_COPY_MACRO(dest, \"h\"); }\n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_transform_safe_crt(tree, &config);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  printf("=== GENERATED CODE ===\n%s\n=== END ===\n", out);

  ASSERT(strstr(out, "strcpy_s(dest, sizeof(dest), \"abc\");") != NULL);
  ASSERT(strstr(out, "strncpy_s(dest, sizeof(dest), \"def\", _TRUNCATE);") !=
         NULL);
  ASSERT(strstr(out, "strcpy_s(dyn_buf, 256, \"test\");") != NULL);
  ASSERT(strstr(out, "strcpy_s(c_buf, (10) * (2), \"test\");") != NULL);
  ASSERT(strstr(out, "strcpy_s(enum_buf, MY_CONST, \"test\");") != NULL);
  ASSERT(strstr(out, "strcpy_s(mac_buf, MY_MACRO_CONST, \"test\");") != NULL);
  ASSERT(strstr(out, "strcpy_s(const_buf, MY_INT_CONST, \"test\");") != NULL);
  ASSERT(strstr(out, "strcpy_s(t_buf, sizeof(t_buf), \"test\");") != NULL);

  ASSERT(strstr(out, "strcpy_s(r_buf, 1024, \"test\");") != NULL);

  ASSERT(strstr(out, "strcpy_s(dest + 2, (sizeof(dest) - (2)), \"test\");") !=
         NULL);
  ASSERT(strstr(out, "strcpy_s(&dest[3], (sizeof(dest) - (3)), \"test\");") !=
         NULL);

  /* Test string conversions */
  ASSERT(strstr(out, "(_gcvt_s(dest, sizeof(dest), d, 5), dest)") != NULL);
  ASSERT(strstr(out, "(mbstowcs_s(NULL, wdest, (sizeof(wdest)) / "
                     "sizeof(wchar_t), \"abc\", 3), wdest)") != NULL);
  ASSERT(
      strstr(out,
             "(wcstombs_s(NULL, dest, (sizeof(dest)), L\"abc\", 3), dest)") !=
      NULL);
  ASSERT(strstr(out, "(wctomb_s(NULL, dest, sizeof(dest), L'a'), dest)") !=
         NULL);

  /* Test file system conversions */
  ASSERT(strstr(out, "_searchenv_s(\"x\", \"PATH\", dest, sizeof(dest))") !=
         NULL);
  ASSERT(strstr(out, "_wsearchenv_s(L\"x\", L\"PATH\", wdest, sizeof(wdest) "
                     "/ sizeof(wchar_t))") != NULL);
  ASSERT(strstr(out, "_wputenv_s(L\"A\", L\"B\")") != NULL);
  ASSERT(strstr(out, "qsort_s(dest, 10, 1, foo, NULL)") != NULL);
  ASSERT(strstr(out, "bsearch_s(\"a\", dest, 10, 1, foo, NULL)") != NULL);

  ASSERT(strstr(out, "sprintf_s(dest, sizeof(dest), \"%d\", 1);") != NULL);
  ASSERT(strstr(out, "fopen_s(&f") != NULL);
  ASSERT(strstr(out, "FILE *f2") != NULL);
  ASSERT(strstr(out, "fopen_s(&f2, \"b.txt\", \"r\")") != NULL);
  ASSERT(strstr(out, "strcat_s(dest, sizeof(dest), \"x\");") != NULL);
  ASSERT(strstr(out, "strncat_s(dest, sizeof(dest), \"y\", _TRUNCATE);") !=
         NULL);
  ASSERT(strstr(out, "memcpy_s(dest, sizeof(dest), \"z\", 1);") != NULL);
  ASSERT(strstr(out, "memmove_s(dest, sizeof(dest), \"w\", 1);") != NULL);

  /* Test nested evaluator */
  ASSERT(strstr(out, "strcpy_s(dest, sizeof(dest), strcpy_s(dest, "
                     "sizeof(dest), \"nested\"));") != NULL);

  /* Test Section 1.1 formatted I/O additions */
  ASSERT(strstr(out, "snprintf_s(dest, 10, _TRUNCATE, \"%d\", 2);") != NULL);
  ASSERT(strstr(out, "printf_s(\"%s\", dest);") != NULL);

  /* Test Section 1.2 scanf additions */
  ASSERT(strstr(out, "scanf_s(\"%s %d %c\", dest, (unsigned)sizeof(dest), "
                     "&idx, &ch, (unsigned)sizeof(ch));") != NULL);
  ASSERT(strstr(out, "fscanf_s(f, \"%10s\", dest, (unsigned)sizeof(dest));") !=
         NULL);
  ASSERT(strstr(out, "sscanf_s(dest, \"%[a-z] %% %*d %s\", dest2, "
                     "(unsigned)sizeof(dest2), "
                     "dest3, (unsigned)sizeof(dest3));") != NULL);

  /* Test _itoa, _mbscpy, freopen, _wfopen additions */
  ASSERT(strstr(out, "_itoa_s(idx, dest, sizeof(dest), 10);") != NULL);
  ASSERT(strstr(out, "_mbscpy_s(dest, sizeof(dest), \"abc\");") != NULL);
  ASSERT(strstr(out, "_mbsncpy_s(dest, sizeof(dest), \"abc\", _TRUNCATE);") !=
         NULL);
  ASSERT(strstr(out, "_mbscat_s(dest, sizeof(dest), \"abc\");") != NULL);
  ASSERT(strstr(out, "_mbsncat_s(dest, sizeof(dest), \"abc\", _TRUNCATE);") !=
         NULL);
  ASSERT(strstr(out, "_mbslwr_s(dest, sizeof(dest));") != NULL);
  ASSERT(strstr(out, "_mbsupr_s(dest, sizeof(dest));") != NULL);
  ASSERT(strstr(out, "_mbsset_s(dest, sizeof(dest), 'a');") != NULL);
  ASSERT(strstr(out, "_mbsnset_s(dest, sizeof(dest), 'a', _TRUNCATE);") !=
         NULL);
  ASSERT(strstr(out, "_wcslwr_s(wdest, sizeof(wdest));") != NULL);
  ASSERT(strstr(out, "_wcsupr_s(wdest, sizeof(wdest));") != NULL);
  ASSERT(strstr(out, "freopen_s") != NULL);
  ASSERT(strstr(out, "FILE *f3") != NULL);
  ASSERT(strstr(out, "_wfopen_s(&f3, L\"a.txt\", L\"r\")") != NULL);
  ASSERT(strstr(out, "FILE *f4") != NULL);
  ASSERT(strstr(out, "tmpfile_s(&f4)") != NULL);
  ASSERT(strstr(out, "_splitpath_s(dest, dest, sizeof(dest), dest, "
                     "sizeof(dest), dest, sizeof(dest), dest, sizeof(dest))") !=
         NULL);
  ASSERT(
      strstr(out, "_makepath_s(dest, sizeof(dest), dest, dest, dest, dest)") !=
      NULL);

  /* Test strlen addition */
  ASSERT(strstr(out, "strnlen_s(dest, sizeof(dest))") != NULL);
  ASSERT(strstr(out, "strtok_s(dest, \"a\", &__strtokctx)") != NULL);
  ASSERT(strstr(out, "wcstok_s(wdest, L\"a\", &__wcstokctx)") != NULL);
  ASSERT(strstr(out, "_mbstok_s(dest, \"a\", &__mbstokctx)") != NULL);
  ASSERT(
      strstr(out, "((_ecvt_s(__ecvtbuf, 128, d, 1, &idx, &idx), __ecvtbuf))") !=
      NULL);
  ASSERT(
      strstr(out, "((_fcvt_s(__fcvtbuf, 128, d, 1, &idx, &idx), __fcvtbuf))") !=
      NULL);
  ASSERT(strstr(out, "((strerror_s(__errbuf, 94, 1), __errbuf))") != NULL);
  ASSERT(strstr(out, "((_wcserror_s(__wcserrbuf, 94, 1), __wcserrbuf))") !=
         NULL);

  /* Test preservation of inline comments */
  ASSERT(strstr(out,
                "strcpy_s(dest, sizeof(dest), /* src */ \"g\" /* suffix */)") !=
         NULL);

  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_transform_safe_crt_extended_functions(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};
  char *out = NULL;

  const char *code = "void foo(va_list args) {\n"
                     "  char dest[100];\n"
                     "  wchar_t wdest[100];\n"
                     "  int idx;\n"
                     "  long lval;\n"
                     "  unsigned long ulval;\n"
                     "  long long llval;\n"
                     "  unsigned long long ullval;\n"
                     "  vsprintf(dest, \"%s\", args);\n"
                     "  vsnprintf(dest, 10, \"%s\", args);\n"
                     "  _vsnprintf(dest, 10, \"%s\", args);\n"
                     "  wmemcpy(wdest, L\"abc\", 3);\n"
                     "  wmemmove(wdest, L\"abc\", 3);\n"
                     "  wcscpy(wdest, L\"abc\");\n"
                     "  wcsncpy(wdest, L\"abc\", 2);\n"
                     "  wcscat(wdest, L\"abc\");\n"
                     "  wcsncat(wdest, L\"abc\", 2);\n"
                     "  swprintf(wdest, 100, L\"%s\", L\"test\");\n"
                     "  vswprintf(wdest, 100, L\"%s\", args);\n"
                     "  _ltoa(lval, dest, 10);\n"
                     "  _ultoa(ulval, dest, 10);\n"
                     "  _i64toa(llval, dest, 10);\n"
                     "  _ui64toa(ullval, dest, 10);\n"
                     "  _itow(idx, wdest, 10);\n"
                     "  _ltow(lval, wdest, 10);\n"
                     "  _ultow(ulval, wdest, 10);\n"
                     "  _strerror(dest);\n"
                     "  _stricmp(\"a\", \"b\");\n"
                     "  _strnicmp(\"a\", \"b\", 1);\n"
                     "  _strlwr(dest);\n"
                     "  _strupr(dest);\n"
                     "  _strnset(dest, 'a', 2);\n"
                     "  _strset(dest, 'a');\n"
                     "  tmpnam(dest);\n"
                     "  _wsplitpath(wdest, wdest, wdest, wdest, wdest);\n"
                     "  _wmakepath(wdest, wdest, wdest, wdest, wdest);\n"
                     "}\n";

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  ASSERT_EQ(0, cdd_transform_safe_crt(tree, &config));

  cdd_cst_emit(tree, &out);
  cdd_cst_tree_free(tree);

  printf("EXT OUT:\n%s\n", out);
  printf("\n\nEXT OUT:\n%s\n\n", out);
  ASSERT(strstr(out, "vsprintf_s(dest, sizeof(dest), \"%s\", args);") != NULL);
  ASSERT(strstr(out, "vsnprintf_s(dest, 10, _TRUNCATE, \"%s\", args);") !=
         NULL);
  ASSERT(strstr(out, "_vsnprintf_s(dest, 10, _TRUNCATE, \"%s\", args);") !=
         NULL);
  ASSERT(strstr(out, "wmemcpy_s(wdest, sizeof(wdest), L\"abc\", 3);") != NULL);
  ASSERT(strstr(out, "wmemmove_s(wdest, sizeof(wdest), L\"abc\", 3);") != NULL);
  ASSERT(strstr(out, "wcscpy_s(wdest, sizeof(wdest), L\"abc\");") != NULL);
  ASSERT(strstr(out, "wcsncpy_s(wdest, sizeof(wdest), L\"abc\", _TRUNCATE);") !=
         NULL);
  ASSERT(strstr(out, "wcscat_s(wdest, sizeof(wdest), L\"abc\");") != NULL);
  ASSERT(strstr(out, "wcsncat_s(wdest, sizeof(wdest), L\"abc\", _TRUNCATE);") !=
         NULL);
  ASSERT(strstr(out,
                "swprintf_s(wdest, sizeof(wdest), 100, L\"%s\", L\"test\");") !=
         NULL);
  ASSERT(
      strstr(out, "vswprintf_s(wdest, sizeof(wdest), 100, L\"%s\", args);") !=
      NULL);
  ASSERT(strstr(out, "_ltoa_s(lval, dest, sizeof(dest), 10);") != NULL);
  ASSERT(strstr(out, "_ultoa_s(ulval, dest, sizeof(dest), 10);") != NULL);
  ASSERT(strstr(out, "_i64toa_s(llval, dest, sizeof(dest), 10);") != NULL);
  ASSERT(strstr(out, "_ui64toa_s(ullval, dest, sizeof(dest), 10);") != NULL);
  ASSERT(strstr(out, "_itow_s(idx, wdest, sizeof(wdest), 10);") != NULL);
  ASSERT(strstr(out, "_ltow_s(lval, wdest, sizeof(wdest), 10);") != NULL);
  ASSERT(strstr(out, "_ultow_s(ulval, wdest, sizeof(wdest), 10);") != NULL);
  ASSERT(strstr(out, "((_strerror_s(__errbuf, 94, dest), __errbuf))") != NULL);
  ASSERT(strstr(out, "_stricmp(\"a\", \"b\")") != NULL);
  ASSERT(strstr(out, "_strlwr_s(dest, sizeof(dest));") != NULL);
  ASSERT(strstr(out, "_strupr_s(dest, sizeof(dest));") != NULL);
  ASSERT(strstr(out, "_strnset_s(dest, sizeof(dest), 'a', 2);") != NULL);
  ASSERT(strstr(out, "_strset_s(dest, sizeof(dest), 'a');") != NULL);
  ASSERT(strstr(out, "tmpnam_s(dest, sizeof(dest));") != NULL);
  ASSERT(
      strstr(out,
             "_wsplitpath_s(wdest, wdest, sizeof(wdest), wdest, sizeof(wdest), "
             "wdest, sizeof(wdest), wdest, sizeof(wdest));") != NULL);
  ASSERT(
      strstr(
          out,
          "_wmakepath_s(wdest, sizeof(wdest), wdest, wdest, wdest, wdest);") !=
      NULL);

  free(out);
  PASS();
}

TEST test_cdd_transform_safe_crt_edge_cases(void) {
  cdd_cst_tree_t *tree = NULL;

  const char *code =
      "void edge_cases() {\n"
      "  char buf[256];\n"
      "\n#if defined(_MSC_VER)\n"

      "  int dummy = 1;\n"
      "  if (dummy) strcpy(buf, \"abc\"); else strcpy(buf, \"def\");\n"
      "  switch (dummy) { case 1: strcpy(buf, \"a\"); }\n"
      "  for (; dummy; dummy--) strcpy(buf, \"x\");\n"
      "  while (dummy) strcpy(buf, \"y\");\n"
      "  do strcpy(buf, \"z\"); while (0);\n"
      "  { strcpy(buf, \"foo\"); }\n"
      "  fscanf(stdout, \"123\", &dummy, &dummy);\n"

      "  fscanf(stdout, \"123\", &dummy);\n"

      "  scanf();\n"
      "  fscanf(stdout);\n"
      "  printf();\n"
      "  scanf(\"123\", &dummy, &dummy);\n"

      "  printf(\"hello\");\n" /* not transformed without args */
      "}\n";
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_transform_safe_crt(NULL, &config));

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  ASSERT_EQ(0, cdd_transform_safe_crt(tree, &config));

  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_safe_crt_malloc_fail;
extern C_CDD_EXPORT int g_cdd_cst_alloc_node_fail;
#endif

TEST test_cdd_transform_safe_crt_oom(void) {
#ifdef CDD_BUILD_TESTS
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_tree_t *tree2 = NULL;

  const char *code =
      "void f() { char buf[10]; char *p; double d; wchar_t wbuf[10]; p = "
      "strtok(buf, \"a\"); p = wcstok(wbuf, L\"a\"); _mbstok(buf, \"a\"); "
      "strerror(1); _wcserror(1); _ecvt(d, 1, 0, 0); _fcvt(d, 1, 0, 0); "
      "ctime(NULL); getenv(\"A\"); _wgetenv(L\"A\"); strcpy(buf, \"abc\"); }";
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));

  g_safe_crt_malloc_fail = 1;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 0;
  cdd_cst_tree_free(tree);
  tree = NULL;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  g_cdd_cst_alloc_node_fail = 1;
  cdd_transform_safe_crt(tree, &config);
  g_cdd_cst_alloc_node_fail = 0;
  cdd_cst_tree_free(tree);
  tree = NULL;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  g_safe_crt_malloc_fail = 2;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 0;
  cdd_cst_tree_free(tree);
  tree = NULL;
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  g_safe_crt_malloc_fail = 6;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 0;
  cdd_cst_tree_free(tree);
  tree = NULL;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  g_safe_crt_malloc_fail = 3;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 0;
  cdd_cst_tree_free(tree);
  tree = NULL;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  g_safe_crt_malloc_fail = 4;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 0;
  cdd_cst_tree_free(tree);
  tree = NULL;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  g_safe_crt_malloc_fail = 5;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 0;
  cdd_cst_tree_free(tree);
  tree = NULL;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree2));
  cdd_transform_safe_crt(tree2, &config);
  cdd_cst_tree_free(tree2);

#endif
  g_fail_io_after = -1;
  PASS();
}

SUITE(transformer_safe_crt_suite) {
  RUN_TEST(test_cdd_transform_safe_crt);

  RUN_TEST(test_cdd_transform_safe_crt_extended_functions);

  RUN_TEST(test_cdd_transform_safe_crt_edge_cases);
  RUN_TEST(test_cdd_transform_safe_crt_oom);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_TRANSFORM_SAFE_CRT_H */
