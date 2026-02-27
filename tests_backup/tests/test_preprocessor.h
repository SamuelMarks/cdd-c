#ifndef TEST_PREPROCESSOR_H
#define TEST_PREPROCESSOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <greatest.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "functions/parse_fs.h"
#include "functions/parse_preprocessor.h"
#include "functions/parse_tokenizer.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#define PATH_SEP_CHAR '\\'
#else
#define PATH_SEP_CHAR '/'
#endif

/* Helpers */
struct TestPPCtx {
  int count;
  char last_found[256];
  struct EmbedParams last_params; /* captured */
};

/* Updated mock callback for new signature */
static int mock_cb(const struct IncludeInfo *info, void *user_data) {
  struct TestPPCtx *ctx = (struct TestPPCtx *)user_data;
  ctx->count++;
  strncpy(ctx->last_found, info->resolved_path, 255);
  ctx->last_found[255] = '\0';

  /* Copy embed params for verification */
  if (info->kind == PP_DIR_EMBED) {
    ctx->last_params.limit = info->params.limit;

    if (ctx->last_params.prefix)
      free(ctx->last_params.prefix);
    if (info->params.prefix)
      ctx->last_params.prefix = strdup(info->params.prefix);
    else
      ctx->last_params.prefix = NULL;

    if (ctx->last_params.suffix)
      free(ctx->last_params.suffix);
    if (info->params.suffix)
      ctx->last_params.suffix = strdup(info->params.suffix);
    else
      ctx->last_params.suffix = NULL;
  }
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
  PASS();
  pp_context_free(&ctx);
}

TEST test_pp_eval_macros_as_values(void) {
  struct PreprocessorContext ctx;
  pp_context_init(&ctx);
  pp_add_macro(&ctx, "ONE", "1");
  pp_add_macro(&ctx, "TWO", "2");

  ASSERT_EQ(3, eval("ONE + TWO", &ctx));

  pp_context_free(&ctx);
  PASS();
}

/* --- Introspection Tests --- */

TEST test_pp_has_include(void) {
  struct PreprocessorContext ctx;
  char *tmp = NULL;
  char *root = NULL;
  char *h = NULL;

  tempdir(&tmp);
  asprintf(&root, "%s%cpp_inc_%d", tmp, PATH_SEP_CHAR, rand());
  makedir(root);
  asprintf(&h, "%s%cheading.h", root, PATH_SEP_CHAR);
  write_to_file(h, "/* contents */");

  pp_context_init(&ctx);
  pp_add_search_path(&ctx, root);

  ASSERT_EQ(1, eval("__has_include(\"heading.h\")", &ctx));
  ASSERT_EQ(0, eval("__has_include(\"missing.h\")", &ctx));

  pp_context_free(&ctx);
  remove(h);
  rmdir(root);
  free(h);
  free(root);
  free(tmp);
  PASS();
}

TEST test_pp_embed_params_parsing(void) {
  struct PreprocessorContext ctx;
  struct TestPPCtx tctx;
  char *tmp = NULL, *root = NULL, *src = NULL, *dat = NULL;

  memset(&tctx, 0, sizeof(tctx));
  tctx.last_params.limit = -1;

  tempdir(&tmp);
  asprintf(&root, "%s%cpp_embp_%d", tmp, PATH_SEP_CHAR, rand());
  makedir(root);
  asprintf(&src, "%s%cmain.c", root, PATH_SEP_CHAR);
  asprintf(&dat, "%s%cdata.bin", root, PATH_SEP_CHAR);
  write_to_file(dat, "123");

  write_to_file(
      src, "#embed \"data.bin\" limit(10) prefix(0x00, ) suffix( ,0xFF)\n");

  pp_context_init(&ctx);
  pp_add_search_path(&ctx, root);

  ASSERT_EQ(0, pp_scan_includes(src, &ctx, mock_cb, &tctx));

  ASSERT_EQ(1, tctx.count);
  ASSERT_EQ(10, tctx.last_params.limit);
  /* Includes space after comma due to reconstruct_path behavior */
  ASSERT_STR_EQ("0x00, ", tctx.last_params.prefix);
  ASSERT_STR_EQ(" ,0xFF", tctx.last_params.suffix);

  if (tctx.last_params.prefix)
    free(tctx.last_params.prefix);
  if (tctx.last_params.suffix)
    free(tctx.last_params.suffix);

  pp_context_free(&ctx);
  remove(src);
  remove(dat);
  rmdir(root);
  free(src);
  free(dat);
  free(root);
  free(tmp);
  PASS();
}

/* --- Conditional Inclusion Tests --- */

TEST test_pp_ifdef_skip(void) {
  char *tmp = NULL, *root = NULL, *main_c = NULL, *header = NULL;
  struct PreprocessorContext ctx;
  struct TestPPCtx tctx = {0, "", {-1, NULL, NULL, NULL}};
  int rc;

  tempdir(&tmp);
  asprintf(&root, "%s%cpp_cond_%d", tmp, PATH_SEP_CHAR, rand());
  makedir(root);
  asprintf(&main_c, "%s%cmain.c", root, PATH_SEP_CHAR);
  asprintf(&header, "%s%cheader.h", root, PATH_SEP_CHAR);

  write_to_file(header, "//");
  write_to_file(main_c,
                "#define FOO\n#ifdef FOO\n#include \"header.h\"\n#endif\n");

  pp_context_init(&ctx);
  pp_add_macro(&ctx, "FOO", NULL);

  rc = pp_scan_includes(main_c, &ctx, mock_cb, &tctx);

  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, tctx.count);

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
  char *tmp = NULL, *root = NULL, *main_c = NULL, *h1 = NULL, *h2 = NULL;
  struct PreprocessorContext ctx;
  struct TestPPCtx tctx = {0, "", {-1, NULL, NULL, NULL}};
  int rc;

  tempdir(&tmp);
  asprintf(&root, "%s%cpp_else_%d", tmp, PATH_SEP_CHAR, rand());
  makedir(root);
  asprintf(&main_c, "%s%cmain.c", root, PATH_SEP_CHAR);
  asprintf(&h1, "%s%ch1.h", root, PATH_SEP_CHAR);
  asprintf(&h2, "%s%ch2.h", root, PATH_SEP_CHAR);

  write_to_file(h1, "");
  write_to_file(h2, "");
  write_to_file(main_c,
                "#if 0\n#include \"h1.h\"\n#else\n#include \"h2.h\"\n#endif\n");

  pp_context_init(&ctx);
  rc = pp_scan_includes(main_c, &ctx, mock_cb, &tctx);

  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, tctx.count);
  /* The last found path includes the full path, verify substring */
  ASSERT(strstr(tctx.last_found, "h2.h") != NULL);

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
  char *tmp = NULL, *root = NULL, *main_c = NULL, *h1 = NULL;
  struct PreprocessorContext ctx;
  struct TestPPCtx tctx = {0, "", {-1, NULL, NULL, NULL}};
  int rc;

  tempdir(&tmp);
  asprintf(&root, "%s%cpp_nest_%d", tmp, PATH_SEP_CHAR, rand());
  makedir(root);
  asprintf(&main_c, "%s%cmain.c", root, PATH_SEP_CHAR);
  asprintf(&h1, "%s%ch1.h", root, PATH_SEP_CHAR);

  write_to_file(h1, "");
  write_to_file(main_c, "#if 1\n#if 0\n#include \"h1.h\"\n#elif 1\n#include "
                        "\"h1.h\"\n#endif\n#endif\n");

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
  RUN_TEST(test_pp_eval_arithmetic);
  RUN_TEST(test_pp_eval_logical);
  RUN_TEST(test_pp_eval_comparison);
  RUN_TEST(test_pp_eval_defined);
  RUN_TEST(test_pp_eval_macros_as_values);

  RUN_TEST(test_pp_has_include);
  RUN_TEST(test_pp_embed_params_parsing);

  RUN_TEST(test_pp_ifdef_skip);
  RUN_TEST(test_pp_if_else);
  RUN_TEST(test_pp_nested_if);
}

#endif /* TEST_PREPROCESSOR_H */
