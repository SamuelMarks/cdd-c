/**
 * @file test_diff_generator.h
 * @brief Unit tests for Diff Generator.
 */

#ifndef TEST_DIFF_GENERATOR_H
#define TEST_DIFF_GENERATOR_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/diff_generator.h"
#include "functions/parse/tokenizer.h"

TEST test_generate_diff_basic(void) {
  const char *src =
      ""
"int main() {\n"
      "  return 0;\n"
      "}\n";

  struct TokenList *tokens = NULL;
  struct PatchList patches;
  char *diff = NULL;
  int rc;

  az_span span = az_span_create((uint8_t *)src, strlen(src));
  rc = tokenize(span, &tokens);
  ASSERT_EQ(0, rc);

  patch_list_init(&patches);
  
  /* Create a dummy patch replacing '0' with '1' */
  {
    size_t i;
    for (i = 0; i < tokens->size; ++i) {
      if (tokens->tokens[i].kind == TOKEN_NUMBER_LITERAL) {
        if (patches.size < 1) {
          patches.patches = malloc(sizeof(struct Patch));
          patches.capacity = 1;
        }
        patches.patches[0].start_token_idx = i;
        patches.patches[0].end_token_idx = i + 1;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
        patches.patches[0].text = _strdup("1");
#else
        patches.patches[0].text = strdup("1");
#endif
        patches.size = 1;
        break;
      }
    }
  }

  rc = patch_list_generate_diff(tokens, &patches, "main.c", &diff);
  ASSERT_EQ(0, rc);
  ASSERT(diff != NULL);

  ASSERT(strstr(diff, "--- a/main.c\n") != NULL);
  ASSERT(strstr(diff, "+++ b/main.c\n") != NULL);
  ASSERT(strstr(diff, "-0\n") != NULL);
  ASSERT(strstr(diff, "+1\n") != NULL);

  free(diff);
  patch_list_free(&patches);
  free_token_list(tokens);
  PASS();
}

SUITE(diff_generator_suite) {
  RUN_TEST(test_generate_diff_basic);
}

#endif /* TEST_DIFF_GENERATOR_H */
