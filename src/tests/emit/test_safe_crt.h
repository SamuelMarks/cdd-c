/**
 * @file test_safe_crt.h
 * @brief Unit tests for Safe CRT transformations.
 */

#ifndef TEST_SAFE_CRT_H
#define TEST_SAFE_CRT_H

/* clang-format off */
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/safe_crt.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

TEST test_safe_crt_strcpy(void) {
  const char *src = ""
                    "void foo() {\n"
                    "  char dest[100];\n"
                    "  const char *src = \"hello\";\n"
                    "  strcpy(dest, src);\n"
                    "}\n";

  struct TokenList *tokens = NULL;
  struct CstNodeList *nodes = NULL;
  struct SafeCrtPatchList patches;
  int rc;

  nodes = calloc(1, sizeof(struct CstNodeList));

  az_span span = az_span_create((uint8_t *)src, strlen(src));
  rc = tokenize(span, &tokens);
  ASSERT_EQ(0, rc);

  rc = parse_tokens(tokens, nodes);
  ASSERT_EQ(0, rc);

  safe_crt_patch_list_init(&patches);
  rc = cst_generate_safe_crt_patches(nodes, tokens, &patches);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, patches.size);
  if (strstr(patches.patches[0].replacement_text,
             "strcpy_s(dest, sizeof(dest),  src)") == NULL) {
    printf("DEBUG: replacement_text=\n%s\n",
           patches.patches[0].replacement_text);
    FAILm("Missing expected text");
  }

  safe_crt_patch_list_free(&patches);
  free_cst_node_list(nodes);
  free(nodes);
  free_token_list(tokens);
  PASS();
}

SUITE(safe_crt_suite) { RUN_TEST(test_safe_crt_strcpy); }

#endif /* TEST_SAFE_CRT_H */
