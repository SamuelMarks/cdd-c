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
  ASSERT_EQ(EINVAL, cst_print_tokens_exact(NULL, stdout));
  ASSERT_EQ(EINVAL, cst_print_tokens_exact(tokens, NULL));

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  rc = fopen_s(&f, "test_cst_print.txt", "wb+");
  if (rc != 0)
    FAILm("Could not open file");
#else
  f = fopen("test_cst_print.txt", "wb+");
  if (!f)
    FAILm("Could not open file");
#endif

  rc = cst_print_tokens_exact(tokens, f);
  ASSERT_EQ(0, rc);

  {
    FILE *readonly_f = fopen("test_cst_print.txt", "r");
    if (readonly_f) {
      ASSERT_EQ(EIO, cst_print_tokens_exact(tokens, readonly_f));
      fclose(readonly_f);
    }
  }

  fseek(f, 0, SEEK_SET);
  fread(buffer, 1, sizeof(buffer) - 1, f);
  fclose(f);
  remove("test_cst_print.txt");

  ASSERT_STR_EQ(src, buffer);

  free_token_list(tokens);
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
