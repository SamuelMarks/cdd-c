#ifndef TEST_CDD_TRANSFORM_SAFE_CRT_H
#define TEST_CDD_TRANSFORM_SAFE_CRT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
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
      "void foo() {\n"
      "  char dest[10];\n"
      "  char dest2[10];\n"
      "  char dest3[10];\n"
      "  wchar_t wdest[10];\n"
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
      "  sscanf(dest, \"%[a-z] %s\", dest2, dest3);\n"
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
      "  /* prefix */ strcpy(dest, /* src */ \"g\" /* suffix */);\n"
      "}\n"
      "#define MY_COPY_MACRO(a, b) strcpy(a, b)\n"
      "void bar() { MY_COPY_MACRO(dest, \"h\"); }\n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2, 0};

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
  ASSERT(
      strstr(out, "(_dupenv_s(&__getenv_ptr, NULL, \"PATH\"), __getenv_ptr)") !=
      NULL);
  ASSERT(
      strstr(out,
             "(_wdupenv_s(&__wgetenv_ptr, NULL, L\"PATH\"), __wgetenv_ptr)") !=
      NULL);
  ASSERT(strstr(out, "_putenv_s(\"A\", \"B\")") != NULL);
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
  ASSERT(strstr(out, "fprintf_s(f, \"%s\", dest);") != NULL);

  /* Test Section 1.2 scanf additions */
  ASSERT(strstr(out, "scanf_s(\"%s %d %c\", dest, (unsigned)sizeof(dest), "
                     "&idx, &ch, (unsigned)sizeof(ch));") != NULL);
  ASSERT(strstr(out, "fscanf_s(f, \"%10s\", dest, (unsigned)sizeof(dest));") !=
         NULL);
  ASSERT(strstr(out,
                "sscanf_s(dest, \"%[a-z] %s\", dest2, (unsigned)sizeof(dest2), "
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

  /* Test preservation of inline comments */
  ASSERT(strstr(out,
                "strcpy_s(dest, sizeof(dest), /* src */ \"g\" /* suffix */)") !=
         NULL);

  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_cdd_transform_safe_crt_edge_cases(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "void edge_cases() {\n"
                     "  char buf[256];\n"
                     "  int dummy = 1;\n"
                     "  if (dummy) strcpy(buf, \"abc\"); else strcpy(buf, \"def\");\n"
                     "  switch (dummy) { case 1: strcpy(buf, \"a\"); }\n"
                     "  for (; dummy; dummy--) strcpy(buf, \"x\");\n"
                     "  while (dummy) strcpy(buf, \"y\");\n"
                     "  do strcpy(buf, \"z\"); while (0);\n"
                     "  { strcpy(buf, \"foo\"); }\n"
                     "  printf(\"hello\");\n" /* not transformed without args */
                     "}\n";
  cdd_transform_config_t config = {0, 2, 0};

  ASSERT_EQ(EINVAL, cdd_transform_safe_crt(NULL, &config));

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  ASSERT_EQ(0, cdd_transform_safe_crt(tree, &config));

  cdd_cst_tree_free(tree);
  PASS();
}

SUITE(transformer_safe_crt_suite) { 
  RUN_TEST(test_cdd_transform_safe_crt); 
  RUN_TEST(test_cdd_transform_safe_crt_edge_cases);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_TRANSFORM_SAFE_CRT_H */