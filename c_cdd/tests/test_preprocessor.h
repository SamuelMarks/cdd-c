#ifndef TEST_PREPROCESSOR_H
#define TEST_PREPROCESSOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <greatest.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "fs.h"
#include "preprocessor.h"
#include "tokenizer.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#define PATH_SEP_CHAR '\\'
#else
#define PATH_SEP_CHAR '/'
#endif

/* Helpers */
struct TestPPCtx {
  int count;
  char last_found[256];
};

static int mock_cb(const char *path, void *user_data) {
  struct TestPPCtx *ctx = (struct TestPPCtx *)user_data;
  ctx->count++;
  strncpy(ctx->last_found, path, 255);
  ctx->last_found[255] = '\0';
  return 0;
}

/* --- Expression Evaluator Tests --- */

static long eval(const char *expr, struct PreprocessorContext *ctx) {
  struct TokenList *tl = NULL;
  long res = 0;
  int rc;
  if (tokenize(az_span_create_from_str((char *)expr), &tl) != 0)
    return -999;
  rc = pp_eval_expression(tl, 0, tl->size, ctx, &res);
  free_token_list(tl);
  if (rc != 0)
    return -999;
  return res;
}

TEST test_pp_eval_arithmetic(void) {
  ASSERT_EQ(2, eval("1 + 1", NULL));
  ASSERT_EQ(7, eval("1 + 2 * 3", NULL));   /* Precedence */
  ASSERT_EQ(9, eval("(1 + 2) * 3", NULL)); /* Parens */
  ASSERT_EQ(1, eval("5 / 5", NULL));
  ASSERT_EQ(0, eval("1 - 1", NULL));
  ASSERT_EQ(-1, eval("1 - 2", NULL));
  ASSERT_EQ(1, eval("5 % 2", NULL));
  PASS();
}

TEST test_pp_eval_logical(void) {
  ASSERT_EQ(1, eval("1 && 1", NULL));
  ASSERT_EQ(0, eval("1 && 0", NULL));
  ASSERT_EQ(1, eval("1 || 0", NULL));
  ASSERT_EQ(0, eval("0 || 0", NULL));
  ASSERT_EQ(1, eval("!0", NULL));
  ASSERT_EQ(0, eval("!1", NULL));
  PASS();
}

TEST test_pp_eval_comparison(void) {
  ASSERT_EQ(1, eval("1 == 1", NULL));
  ASSERT_EQ(0, eval("1 == 2", NULL));
  ASSERT_EQ(1, eval("1 != 2", NULL));
  ASSERT_EQ(1, eval("2 > 1", NULL));
  ASSERT_EQ(0, eval("1 > 2", NULL));
  ASSERT_EQ(1, eval("1 <= 1", NULL));
  PASS();
}

TEST test_pp_eval_defined(void) {
  struct PreprocessorContext ctx;
  pp_context_init(&ctx);
  pp_add_macro(&ctx, "FOO", NULL);

  ASSERT_EQ(1, eval("defined FOO", &ctx));
  ASSERT_EQ(1, eval("defined(FOO)", &ctx));
  ASSERT_EQ(0, eval("defined BAR", &ctx));
  ASSERT_EQ(0, eval("defined(BAR)", &ctx));
  ASSERT_EQ(1, eval("!defined BAR", &ctx));

  pp_context_free(&ctx);
  PASS();
}

TEST test_pp_eval_macros_as_values(void) {
  struct PreprocessorContext ctx;
  pp_context_init(&ctx);
  pp_add_macro(&ctx, "ONE", "1");
  pp_add_macro(&ctx, "TWO", "2");

  ASSERT_EQ(3, eval("ONE + TWO", &ctx));
  ASSERT_EQ(1, eval("ONE == 1", &ctx));
  /* Undefined identifier evaluates to 0 */
  ASSERT_EQ(0, eval("UNKNOWN", &ctx));
  /* Recursive macro value */
  /* If MACRO expands to text, simplified evaluator tries strtol on text. */
  /* If text is "0x10", strtol handles hex. */
  pp_add_macro(&ctx, "HEX", "0x10");
  ASSERT_EQ(16, eval("HEX", &ctx));

  pp_context_free(&ctx);
  PASS();
}

/* --- Conditional Inclusion Tests --- */

TEST test_pp_ifdef_skip(void) {
  char *tmp = NULL;
  char *root = NULL;
  char *main_c = NULL;
  char *header = NULL;
  struct PreprocessorContext ctx;
  struct TestPPCtx tctx = {0, ""};
  int rc;

  /* Setup:
     #define FOO
     #ifdef FOO
       #include "header.h"
     #endif
     #ifdef BAR
       #include "header.h" <-- Should be skipped
     #endif
  */
  tempdir(&tmp);
  asprintf(&root, "%s%cpp_cond_%d", tmp, PATH_SEP_CHAR, rand());
  makedir(root);
  asprintf(&main_c, "%s%cmain.c", root, PATH_SEP_CHAR);
  asprintf(&header, "%s%cheader.h", root, PATH_SEP_CHAR);

  write_to_file(header, "//");
  write_to_file(main_c, "#define FOO\n"
                        "#ifdef FOO\n"
                        "#include \"header.h\"\n"
                        "#endif\n"
                        "#ifdef BAR\n"
                        "#include \"header.h\"\n"
                        "#endif\n");

  pp_context_init(&ctx);
  /* Pre-define FOO in context to simulate #define scanner working or -D */
  /* NB: scan_includes currently assumes scan_defines was called OR context
     pre-seeded. It doesn't auto-update context from #defines inside the file
     during scan (simplistic behavior). Let's seed it. */
  pp_add_macro(&ctx, "FOO", NULL);

  rc = pp_scan_includes(main_c, &ctx, mock_cb, &tctx);

  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, tctx.count); /* One inclusion */

  pp_context_free(&ctx);
  remove(header);
  remove(main_c);
  rmdir(root);
  free(header);
  free(main_c);
  free(root);
  free(tmp);
  PASS();
}

TEST test_pp_if_else(void) {
  char *tmp = NULL;
  char *root = NULL;
  char *main_c = NULL;
  char *h1 = NULL, *h2 = NULL;
  struct PreprocessorContext ctx;
  struct TestPPCtx tctx = {0, ""};
  int rc;

  /* Setup:
     #if 0
       #include "h1.h"
     #else
       #include "h2.h"
     #endif
  */
  tempdir(&tmp);
  asprintf(&root, "%s%cpp_else_%d", tmp, PATH_SEP_CHAR, rand());
  makedir(root);
  asprintf(&main_c, "%s%cmain.c", root, PATH_SEP_CHAR);
  asprintf(&h1, "%s%ch1.h", root, PATH_SEP_CHAR);
  asprintf(&h2, "%s%ch2.h", root, PATH_SEP_CHAR);

  write_to_file(h1, "");
  write_to_file(h2, "");
  write_to_file(main_c, "#if 0\n"
                        "#include \"h1.h\"\n"
                        "#else\n"
                        "#include \"h2.h\"\n"
                        "#endif\n");

  pp_context_init(&ctx);
  rc = pp_scan_includes(main_c, &ctx, mock_cb, &tctx);

  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, tctx.count);
  ASSERT_STR_EQ(h2, tctx.last_found);

  pp_context_free(&ctx);
  remove(h1);
  remove(h2);
  remove(main_c);
  rmdir(root);
  free(h1);
  free(h2);
  free(main_c);
  free(root);
  free(tmp);
  PASS();
}

TEST test_pp_nested_if(void) {
  char *tmp = NULL;
  char *root = NULL;
  char *main_c = NULL;
  char *h1 = NULL;
  struct PreprocessorContext ctx;
  struct TestPPCtx tctx = {0, ""};
  int rc;

  tempdir(&tmp);
  asprintf(&root, "%s%cpp_nest_%d", tmp, PATH_SEP_CHAR, rand());
  makedir(root);
  asprintf(&main_c, "%s%cmain.c", root, PATH_SEP_CHAR);
  asprintf(&h1, "%s%ch1.h", root, PATH_SEP_CHAR);

  write_to_file(h1, "");
  /* Nested logic:
     #if 1
       #if 0
         #include "h1.h" -> skip
       #elif 1
         #include "h1.h" -> take
       #endif
     #endif
  */
  write_to_file(main_c, "#if 1\n"
                        "#if 0\n"
                        "#include \"h1.h\"\n"
                        "#elif 1\n"
                        "#include \"h1.h\"\n"
                        "#endif\n"
                        "#endif\n");

  pp_context_init(&ctx);
  rc = pp_scan_includes(main_c, &ctx, mock_cb, &tctx);

  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, tctx.count);

  pp_context_free(&ctx);
  remove(h1);
  remove(main_c);
  rmdir(root);
  free(h1);
  free(main_c);
  free(root);
  free(tmp);
  PASS();
}

SUITE(preprocessor_suite) {
  /* Expression Evaluator */
  RUN_TEST(test_pp_eval_arithmetic);
  RUN_TEST(test_pp_eval_logical);
  RUN_TEST(test_pp_eval_comparison);
  RUN_TEST(test_pp_eval_defined);
  RUN_TEST(test_pp_eval_macros_as_values);

  /* Conditional Inclusion */
  RUN_TEST(test_pp_ifdef_skip);
  RUN_TEST(test_pp_if_else);
  RUN_TEST(test_pp_nested_if);
}

#endif /* TEST_PREPROCESSOR_H */
