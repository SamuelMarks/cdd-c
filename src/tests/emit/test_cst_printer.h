/**
 * @file test_cst_printer.h
 * @brief Unit tests for CST non-destructive printing.
 */

#ifndef TEST_CST_PRINTER_H
#define TEST_CST_PRINTER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/cst_printer.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

/**
 * @brief Tests exact CST printing.
 * @return TEST
 */
TEST test_cst_print_exact(void) {
  const char *src = "/* comment */\n"
                    "int main(void) {\n"
                    "  int a = 1; \\\n"
                    "  int b = 2;\n"
                    "  printf(\"hello\\n\");\n"
                    "}\n";

  struct TokenList *tokens = NULL;
  char buffer[1024] = {0};
  FILE *f;
  int rc;

  az_span span = az_span_create((uint8_t *)src, strlen(src));
  rc = tokenize(span, &tokens);
  ASSERT_EQ(0, rc);

  /* Invalid args */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cst_print_tokens_exact(NULL, stdout));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cst_print_tokens_exact(tokens, NULL));

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  rc = fopen_s(&f, "test_cst_print.txt", "wb+");
  if (rc != 0)
    FAILm("Could not open file");
#else
  f = fopen("test_cst_print.txt", "wb+");
  if (!f)
    /* LCOV_EXCL_START */
    FAILm("Could not open file");
/* LCOV_EXCL_STOP */
#endif

  rc = cst_print_tokens_exact(tokens, f);
  ASSERT_EQ(0, rc);

  {
    FILE *readonly_f = tmpfile();
    if (readonly_f) {
      g_fail_io_after = 0;
      g_io_calls = 0;
      ASSERT_EQ(CDD_C_ERROR_IO, cst_print_tokens_exact(tokens, readonly_f));

      /* Reset and test failing on the 2nd IO call to cover the branch */
      g_fail_io_after = 1;
      g_io_calls = 0;
      ASSERT_EQ(CDD_C_ERROR_IO, cst_print_tokens_exact(tokens, readonly_f));

      fclose(readonly_f);
    }
  }

  fseek(f, 0, SEEK_SET);
  fread(buffer, 1, sizeof(buffer) - 1, f);
  fclose(f);
  remove("test_cst_print.txt");

  ASSERT_STR_EQ(src, buffer);

  free_token_list(tokens);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Suite for CST printing
 */
SUITE(cst_printer_suite) { RUN_TEST(test_cst_print_exact); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CST_PRINTER_H */
