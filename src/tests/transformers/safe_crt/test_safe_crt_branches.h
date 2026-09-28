/**
 * @file test_safe_crt_branches.h
 * @brief Branch and OOM unit tests for the Safe CRT transformer.
 */

#ifndef TEST_CDD_TRANSFORM_SAFE_CRT_BRANCHES_H
#define TEST_CDD_TRANSFORM_SAFE_CRT_BRANCHES_H

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

extern C_CDD_EXPORT int g_safe_crt_malloc_fail;
extern C_CDD_EXPORT int g_cdd_cst_alloc_node_fail;
extern C_CDD_EXPORT int g_cdd_query_err_fail;
extern C_CDD_EXPORT int g_fail_io_after;

TEST test_cdd_transform_safe_crt_oom(void) {
#ifdef CDD_BUILD_TESTS
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_tree_t *tree2 = NULL;

  const char code[] = {
      'v',  'o', 'i', 'd',  ' ', 'f',  '(', ')', ' ',  '{', ' ', 'c',  'h',
      'a',  'r', ' ', 'b',  'u', 'f',  '[', '1', '0',  ']', ';', ' ',  'c',
      'h',  'a', 'r', ' ',  '*', 'p',  ';', ' ', 'd',  'o', 'u', 'b',  'l',
      'e',  ' ', 'd', ';',  ' ', 'w',  'c', 'h', 'a',  'r', '_', 't',  ' ',
      'w',  'b', 'u', 'f',  '[', '1',  '0', ']', ';',  ' ', 'p', ' ',  '=',
      ' ',  'i', 'f', ' ',  '(', '1',  ')', ' ', 's',  't', 'r', 't',  'o',
      'k',  '(', 'b', 'u',  'f', ',',  ' ', '"', 'a',  '"', ')', ';',  ' ',
      'i',  'f', ' ', '(',  '1', ')',  ' ', 'p', ' ',  '=', ' ', 'w',  'c',
      's',  't', 'o', 'k',  '(', 'w',  'b', 'u', 'f',  ',', ' ', 'L',  '"',
      'a',  '"', ')', ';',  ' ', 'i',  'f', ' ', '(',  '1', ')', ' ',  '_',
      'm',  'b', 's', 't',  'o', 'k',  '(', 'b', 'u',  'f', ',', ' ',  '"',
      'a',  '"', ')', ';',  ' ', 'i',  'f', ' ', '(',  '1', ')', ' ',  's',
      't',  'r', 'e', 'r',  'r', 'o',  'r', '(', '1',  ')', ';', ' ',  'i',
      'f',  ' ', '(', '1',  ')', ' ',  '_', 'w', 'c',  's', 'e', 'r',  'r',
      'o',  'r', '(', '1',  ')', ';',  ' ', 'i', 'f',  ' ', '(', '1',  ')',
      ' ',  '_', 'e', 'c',  'v', 't',  '(', 'd', ',',  ' ', '1', ',',  ' ',
      '0',  ',', ' ', '0',  ')', ';',  ' ', 'i', 'f',  ' ', '(', '1',  ')',
      ' ',  '_', 'f', 'c',  'v', 't',  '(', 'd', ',',  ' ', '1', ',',  ' ',
      '0',  ',', ' ', '0',  ')', ';',  ' ', 'i', 'f',  ' ', '(', '1',  ')',
      ' ',  'c', 't', 'i',  'm', 'e',  '(', 'N', 'U',  'L', 'L', ')',  ';',
      ' ',  'i', 'f', ' ',  '(', '1',  ')', ' ', 'g',  'e', 't', 'e',  'n',
      'v',  '(', '"', 'A',  '"', ')',  ';', ' ', 'F',  'I', 'L', 'E',  ' ',
      '*',  'f', ';', '\n', '#', 'i',  'f', ' ', 'd',  'e', 'f', 'i',  'n',
      'e',  'd', ' ', '(',  '_', 'M',  'S', 'C', '_',  'V', 'E', 'R',  ')',
      '\n', ' ', ' ', 'i',  'f', ' ',  '(', 'f', 'o',  'p', 'e', 'n',  '_',
      's',  '(', '&', 'f',  ',', ' ',  '"', 'A', '"',  ',', ' ', '"',  'B',
      '"',  ')', ' ', '!',  '=', ' ',  '0', ')', ' ',  'f', ' ', '=',  ' ',
      'N',  'U', 'L', 'L',  ';', '\n', '#', 'e', 'l',  's', 'e', '\n', ' ',
      ' ',  'f', ' ', '=',  ' ', 'f',  'o', 'p', 'e',  'n', '(', '"',  'A',
      '"',  ',', ' ', '"',  'B', '"',  ')', ';', '\n', '#', 'e', 'n',  'd',
      'i',  'f', ' ', '\n', 'i', 'f',  ' ', '(', '1',  ')', ' ', '_',  'w',
      'g',  'e', 't', 'e',  'n', 'v',  '(', 'L', '"',  'A', '"', ')',  ';',
      ' ',  's', 't', 'r',  'c', 'p',  'y', '(', 'b',  'u', 'f', ',',  ' ',
      '"',  'a', 'b', 'c',  '"', ')',  ';', ' ', '}',  '\0'};
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));

  {
    int i;
    for (i = 1; i <= 200; i++) {
      cdd_cst_tree_free(tree);
      tree = NULL;
      ASSERT_EQ(
          0, cdd_cst_parse(
                 az_span_create_from_str((char *)(size_t)(size_t)code), &tree));
      g_safe_crt_malloc_fail = i;
      cdd_transform_safe_crt(tree, &config);
      g_safe_crt_malloc_fail = 0;
      cdd_cst_tree_free(tree);
      tree = NULL;
    }
    for (i = 1; i <= 20; i++) {
      cdd_cst_tree_free(tree);
      tree = NULL;
      ASSERT_EQ(
          0, cdd_cst_parse(
                 az_span_create_from_str((char *)(size_t)(size_t)code), &tree));
      g_cdd_cst_alloc_node_fail = i;
      cdd_transform_safe_crt(tree, &config);
      g_cdd_cst_alloc_node_fail = 0;
      cdd_cst_tree_free(tree);
      tree = NULL;
    }
  }

  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  g_cdd_cst_alloc_node_fail = 3;
  cdd_transform_safe_crt(tree, &config);
  g_cdd_cst_alloc_node_fail = 0;
  cdd_cst_tree_free(tree);
  tree = NULL;

  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  g_cdd_query_err_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_transform_safe_crt(tree, &config));
  g_cdd_query_err_fail = 0;
  cdd_cst_tree_free(tree);
  tree = NULL;

  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  g_safe_crt_malloc_fail = 2;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 0;
  cdd_cst_tree_free(tree);
  tree = NULL;
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  g_safe_crt_malloc_fail = 15;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 14;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 13;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 12;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 11;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 10;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 9;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 8;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 7;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 0;
  cdd_cst_tree_free(tree);
  tree = NULL;

  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  g_safe_crt_malloc_fail = 3;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 0;
  cdd_cst_tree_free(tree);
  tree = NULL;

  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  g_safe_crt_malloc_fail = 4;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 0;
  cdd_cst_tree_free(tree);
  tree = NULL;

  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  g_safe_crt_malloc_fail = 5;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 0;
  cdd_cst_tree_free(tree);
  tree = NULL;

  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree2));
  cdd_transform_safe_crt(tree2, &config);
  cdd_cst_tree_free(tree2);

  {
    const char *code9 =
        "void edge9() { char buf[256]; strcpy(malloc(10), \"a\"); "
        "str"
        "cpy(calloc(1, 10), \"a\"); strcpy(realloc(NULL, 10), \"a\"); }";
    cdd_cst_tree_t *tree9 = NULL;
    ASSERT_EQ(
        0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code9),
                         &tree9));
    ASSERT_EQ(0, cdd_transform_safe_crt(tree9, &config));
    cdd_cst_tree_free(tree9);

    {
      const char *code10 = "void edge10() { scanf(\"%s\", NULL); }";
      cdd_cst_tree_t *tree10 = NULL;
      ASSERT_EQ(0, cdd_cst_parse(
                       az_span_create_from_str((char *)(size_t)(size_t)code10),
                       &tree10));
      ASSERT_EQ(0, cdd_transform_safe_crt(tree10, &config));
      cdd_cst_tree_free(tree10);

      {
        const char *code11 = "void edge11() { scanf(\"%s\", 0); }";
        cdd_cst_tree_t *tree11 = NULL;
        ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(
                                       (char *)(size_t)(size_t)code11),
                                   &tree11));
        ASSERT_EQ(0, cdd_transform_safe_crt(tree11, &config));
        cdd_cst_tree_free(tree11);

        {
          const char code12[] = {
              118, 111, 105, 100, 32,  101, 100, 103, 101, 49,  50,  40,  41,
              32,  123, 32,  99,  104, 97,  114, 32,  98,  117, 102, 91,  49,
              48,  93,  59,  32,  115, 99,  97,  110, 102, 40,  34,  37,  115,
              32,  37,  115, 32,  37,  115, 32,  37,  115, 32,  37,  115, 32,
              37,  115, 32,  37,  115, 32,  37,  115, 32,  37,  115, 32,  37,
              115, 32,  37,  115, 32,  37,  115, 32,  37,  115, 32,  37,  115,
              32,  37,  115, 32,  37,  115, 32,  37,  115, 32,  37,  115, 32,
              37,  115, 32,  37,  115, 32,  37,  115, 32,  37,  115, 32,  37,
              115, 32,  37,  115, 32,  37,  115, 32,  37,  115, 32,  37,  115,
              32,  37,  115, 32,  37,  115, 32,  37,  115, 32,  37,  115, 32,
              37,  115, 32,  37,  115, 32,  37,  115, 34,  44,  32,  98,  117,
              102, 44,  32,  98,  117, 102, 44,  32,  98,  117, 102, 44,  32,
              98,  117, 102, 44,  32,  98,  117, 102, 44,  32,  98,  117, 102,
              44,  32,  98,  117, 102, 44,  32,  98,  117, 102, 44,  32,  98,
              117, 102, 44,  32,  98,  117, 102, 44,  32,  98,  117, 102, 44,
              32,  98,  117, 102, 44,  32,  98,  117, 102, 44,  32,  98,  117,
              102, 44,  32,  98,  117, 102, 44,  32,  98,  117, 102, 44,  32,
              98,  117, 102, 44,  32,  98,  117, 102, 44,  32,  98,  117, 102,
              44,  32,  98,  117, 102, 44,  32,  98,  117, 102, 44,  32,  98,
              117, 102, 44,  32,  98,  117, 102, 44,  32,  98,  117, 102, 44,
              32,  98,  117, 102, 44,  32,  98,  117, 102, 44,  32,  98,  117,
              102, 44,  32,  98,  117, 102, 44,  32,  98,  117, 102, 44,  32,
              98,  117, 102, 44,  32,  98,  117, 102, 44,  32,  98,  117, 102,
              44,  32,  98,  117, 102, 44,  32,  98,  117, 102, 41,  59,  32,
              125, 0};
          cdd_cst_tree_t *tree12 = NULL;
          ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(
                                         (char *)(size_t)(size_t)code12),
                                     &tree12));
          ASSERT_EQ(0, cdd_transform_safe_crt(tree12, &config));
          cdd_cst_tree_free(tree12);

#endif
          g_fail_io_after = -1;
          PASS();
        }
      }
    }
  }
}

TEST test_cdd_transform_safe_crt_needs_buffers(void) {
  cdd_transform_config_t config;
  cdd_cst_tree_t *tree = NULL;
  const char *snippets[] = {
      "void test_err_noindent(void){\nchar *msg = strerror(1);\n}\n",
      "void test_err(void) {\n  char *msg = strerror(1);\n}\n",
      "void test_wcserr(void) {\n  wchar_t *msg = _wcserror(1);\n}\n",
      "void test_strtok(void) {\n  char buf[32];\n  char *tok = strtok(buf, "
      "\",\");\n}\n",
      "void test_wcstok(void) {\n  wchar_t buf[32];\n  wchar_t *tok = "
      "wcstok(buf, L\",\");\n}\n",
      "void test_mbstok(void) {\n  unsigned char buf[32];\n  unsigned char "
      "*tok = _mbstok(buf, (unsigned char *)\",\");\n}\n",
      "void test_ecvt(void) {\n  int dec, sign;\n  char *res = _ecvt(3.14, 2, "
      "&dec, &sign);\n}\n",
      "void test_fcvt(void) {\n  int dec, sign;\n  char *res = _fcvt(3.14, 2, "
      "&dec, &sign);\n}\n",
      "void test_getenv(void) {\n  char *val = getenv(\"PATH\");\n}\n",
      "void test_wgetenv(void) {\n  wchar_t *val = _wgetenv(L\"PATH\");\n}\n",
      "void test_putenv(void) {\n  _putenv(\"KEY=VAL\");\n}\n",
      "void test_wputenv(void) {\n  _wputenv(L\"KEY=VAL\");\n}\n",
      "void test_qsort(void) {\n  int arr[4];\n  qsort(arr, 4, sizeof(int), "
      "NULL);\n}\n"};
  size_t i;
  memset(&config, 0, sizeof(config));

  for (i = 0; i < sizeof(snippets) / sizeof(snippets[0]); ++i) {
    cdd_c_error_t rc_t;
    ASSERT_EQ(
        0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)snippets[i]),
                         &tree));
    rc_t = cdd_transform_safe_crt(tree, &config);
    ASSERT_EQ(0, rc_t);
    cdd_cst_tree_free(tree);
    tree = NULL;
  }
  PASS();
}

TEST test_cdd_transform_safe_crt_more_cases(void) {
  cdd_transform_config_t config;
  cdd_cst_tree_t *tree = NULL;
  const char *snippets[] = {
      "void test_bare_fopen(void) {\n  fopen(\"a\", \"r\");\n}\n",
      "void test_splitpath_null(void) {\n  char drive[3];\n  char dir[256];\n  "
      "char fname[256];\n  char ext[256];\n  _splitpath(\"path\", drive, dir, "
      "NULL, 0);\n}\n",
      "void test_putenv_var(void) {\n  char *var = \"KEY=VAL\";\n  "
      "_putenv(var);\n}\n",
      "void test_decl_fopen(void) {\n  FILE *f = fopen(\"a\", \"r\");\n}\n",
      "void test_decl_fopen_ptr(void) {\n  FILE **fp;\n  *fp = fopen(\"a\", "
      "\"r\");\n}\n",
      "void test_struct_fopen(void) {\n  struct S { FILE *f; } s;\n  s.f = "
      "fopen(\"a\", \"r\");\n}\n",
      "void test_scanf_brackets(void) {\n  char buf[32];\n  sscanf(\"str\", "
      "\"%*10[a-z] %*s %10[^]a] %10[]a] %10[a-z] %*c %10C %10S\", buf, buf, "
      "buf, buf, buf);\n}\n",
      "void test_inferred_trivia(void) {\n  char dest[10];\n  /*c1*/ "
      "strcpy(/*c2*/ dest /*c3*/, \"test\");\n}\n",
      "void test_equal_trivia(void) {\n  FILE *f;\n  f /*c1*/ = /*c2*/ "
      "fopen(\"a\", \"r\");\n}\n",
      "void test_already_safe(void) {\n  /*CDD_SAFE_CRT*/ strcpy(dest, "
      "\"a\");\n}\n",
      "void test_scanf_no_args(void) {\n  scanf();\n}\n",
      "void test_sscanf_no_args(void) {\n  sscanf(s);\n}\n",

      "void test_assign_dot_fopen(void) {\n  struct S { FILE *f; } s;\n  s.f = "
      "fopen(\"a\", \"r\");\n}\n",
      "void test_assign_star_fopen(void) {\n  FILE *f; FILE **fp = &f;\n  *fp "
      "= fopen(\"a\", \"r\");\n}\n",

      "void test_semicolon_in_call(void) {\n  foo(a; b);\n}\n",
      "void test_call_comma_stop(void) {\n  (a, b);\n}\n",

      "void test_synthesized(void) {\n\n#if defined(_MSC_VER)\n  int x = "
      "1;\n#endif\n}\n",

      "void test_bare_w_fopen(void) {\n  _wfopen(\"a\", \"r\");\n}\n",
      "void test_bare_freopen(void) {\n  FILE *f = NULL;\n  freopen(\"a\", "
      "\"r\", f);\n}\n",
      "void test_bare_tmpfile(void) {\n  tmpfile();\n}\n",
      "void test_arrow_fopen(void) {\n  struct S { FILE *f; } *s;\n  s->f = "
      "fopen(\"a\", \"r\");\n}\n",
      "void test_bracket_fopen(void) {\n  FILE *arr[2];\n  arr[0] = "
      "fopen(\"a\", \"r\");\n}\n",
      "void test_arrow_tmpfile(void) {\n  struct S { FILE *f; } *s;\n  s->f = "
      "tmpfile();\n}\n",
      "void test_bracket_tmpfile(void) {\n  FILE *arr[2];\n  arr[0] = "
      "tmpfile();\n}\n",
      "void test_nospace_fopen(void) {\n  FILE *f; f=fopen(\"a\",\"r\");\n}\n",
      "void test_zero_args(void) {\n"
      "  strcpy();\n  strncpy();\n  snprintf();\n  printf();\n  memcpy();\n"
      "  _itoa();\n  gets();\n  _splitpath();\n  _makepath();\n  _gcvt();\n"
      "  mbstowcs();\n  wctomb();\n  _searchenv();\n  getenv();\n  _putenv();\n"
      "  qsort();\n  strtok();\n}\n",
      "void test_extra_funcs1(void) {\n"
      "  char buf[32]; wchar_t wbuf[32]; va_list va; FILE *f = NULL;\n"
      "  _snprintf(buf, 32, \"%s\", \"a\");\n  _vsnprintf(buf, 32, \"%s\", "
      "va);\n"
      "  vprintf(\"%s\", va);\n  vfprintf(f, \"%s\", va);\n"
      "  vfscanf(f, \"%s\", buf);\n  vsscanf(\"s\", \"%s\", buf);\n"
      "  wcscpy(wbuf, L\"a\");\n  wcscat(wbuf, L\"b\");\n"
      "  swprintf(wbuf, 32, L\"%s\", L\"a\");\n  vswprintf(wbuf, 32, L\"%s\", "
      "va);\n"
      "}\n",
      "void test_sscanf_variants(void) {\n"
      "  char s2[16]; char s3[16]; char *fmt = \"%s\"; int d;\n"
      "  sscanf(\"str\", \"%d %% %s\", &d, s2);\n"
      "  sscanf(\"str\", \"%*d %s\", s2);\n"
      "  sscanf(\"str\", \"%*\", s2);\n"
      "  sscanf(\"str\", \"%\", s2);\n"
      "  sscanf(\"str\", fmt, s2, s3, &d);\n"
      "}\n",
      "void test_extra_funcs2(void) {\n"
      "  char buf[32]; wchar_t wbuf[32]; unsigned char mbuf[32]; char s[5];\n"
      "  _mbscpy(mbuf, \"a\");\n  _mbscat(mbuf, \"b\");\n"
      "  _strnset(buf, 'a', 5);\n  _strset(buf, 'a');\n  _mbsset(mbuf, 'a');\n"
      "  _strlwr(buf);\n  _strupr(buf);\n  _mbslwr(mbuf);\n  _mbsupr(mbuf);\n"
      "  _wcslwr(wbuf);\n  _wcsupr(wbuf);\n  tmpnam(buf);\n  strlen(s);\n"
      "  wcsncpy(wbuf, L\"a\", 5);\n  wcsncat(wbuf, L\"b\", 5);\n"
      "}\n",
      "void test_extra_funcs3(void) {\n"
      "  char buf[32]; wchar_t wbuf[32]; unsigned char mbuf[32];\n"
      "  _mbsncpy(mbuf, \"a\", 5);\n  _mbsncat(mbuf, \"b\", 5);\n  "
      "_mbsnset(mbuf, 'a', 5);\n"
      "  memmove(buf, buf+1, 5);\n  wmemcpy(wbuf, wbuf+1, 5);\n  "
      "wmemmove(wbuf, wbuf+1, 5);\n"
      "  _ltoa(1, buf, 10);\n  _ultoa(1, buf, 10);\n  _i64toa(1, buf, 10);\n  "
      "_ui64toa(1, buf, 10);\n"
      "}\n",
      "void test_extra_funcs4(void) {\n"
      "  char buf[32]; wchar_t wbuf[32];\n"
      "  _itow(1, wbuf, 10);\n  _ltow(1, wbuf, 10);\n  _ultow(1, wbuf, 10);\n"
      "  _wmakepath(wbuf, L\"c\", L\"dir\", L\"f\", L\"ext\");\n  "
      "wcstombs(buf, wbuf, 32);\n"
      "  _wsearchenv(L\"f\", L\"PATH\", wbuf);\n  _strerror(\"msg\");\n"
      "}\n",
      "void test_putenv_no_eq(void) {\n  _putenv(\"NOEQUALS\");\n  "
      "_putenv(\"=VAL\");\n  _wputenv(L\"KEY=VAL\");\n}\n",
      "void test_long_comment(void) {\n  /* a comment with length >= 16 */\n  "
      "char dest[10];\n  strcpy(dest, \"val\");\n}\n",
      "void test_unclosed_scanset(void) {\n  char buf[32];\n  sscanf(\"str\", "
      "\"%[abc\", buf);\n  sscanf(\"str\", \"%[^abc]\", buf);\n}\n",
      "void test_infer_patterns(void) {\n"
      "  char buf[32];\n"
      "  char *p1 = malloc(64);\n"
      "  char *p2 = calloc(10, 4);\n"
      "  char *p3 = realloc(p1, 128);\n"
      "  strcpy(&buf[2], \"a\");\n"
      "  strcpy(buf + 2, \"b\");\n"
      "  strcpy(p1, \"c\");\n"
      "  strcpy(p2, \"d\");\n"
      "  strcpy(p3, \"e\");\n"
      "}\n",
      "void test_pool_expand(void){\n"
      "char b[1];\n"
      "strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");"
      "strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");"
      "strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");"
      "strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");"
      "strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");"
      "strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");"
      "strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");"
      "strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");"
      "strcpy(b,\"\");strcpy(b,\"\");\n}\n"

  };
  size_t i;
  memset(&config, 0, sizeof(config));

  for (i = 0; i < sizeof(snippets) / sizeof(snippets[0]); ++i) {
    int p_rc = cdd_cst_parse(
        az_span_create_from_str((char *)(size_t)snippets[i]), &tree);
    int t_rc = 0;
    if (p_rc == 0) {
      t_rc = cdd_transform_safe_crt(tree, &config);
      cdd_cst_tree_free(tree);
      tree = NULL;
    }
    ASSERT_EQ(0, p_rc);
    ASSERT_EQ(0, t_rc);
  }
  PASS();
}

SUITE(transformer_safe_crt_branches_suite) {
  RUN_TEST(test_cdd_transform_safe_crt_oom);
  RUN_TEST(test_cdd_transform_safe_crt_needs_buffers);
  RUN_TEST(test_cdd_transform_safe_crt_more_cases);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_TRANSFORM_SAFE_CRT_BRANCHES_H */
