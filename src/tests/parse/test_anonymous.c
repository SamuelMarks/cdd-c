/**
 * @file test_anonymous.c
 * @brief Integration tests for anonymous structure lifting.
 */

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "classes/parse/code2schema.h"
#include "functions/parse/fs.h"

GREATEST_MAIN_DEFS();

TEST test_lift_anonymous_struct(void) {
  const char *src = "struct Parent {\n"
                    "  int id;\n"
                    "  struct {\n"
                    "    int x;\n"
                    "    int y;\n"
                    "  } coords;\n"
                    "};\n";

  write_to_file("anon.h", src);

  char *argv[] = {"anon.h", "anon.json"};
  ASSERT_EQ(EXIT_SUCCESS, code2schema_main(2, argv));

  /* Check JSON */
  {
    char *content = NULL;
    size_t sz;
    FILE *f = fopen("anon.json", "r");
    ASSERT(f);
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    rewind(f);
    content = malloc(sz + 1);
    fread(content, 1, sz, f);
    content[sz] = 0;
    fclose(f);

    /* We expect a definition for Parent */
    ASSERT(strstr(content, "\"Parent\":"));

    /* We expect a definition for Parent_coords (Lifting) */
    ASSERT(strstr(content, "\"Parent_coords\":"));

    /* We expect Parent_coords to have properties x and y */
    ASSERT(strstr(content, "\"x\":"));
    ASSERT(strstr(content, "\"y\":"));

    /* We expect Parent to have property coords referencing Parent_coords */
    ASSERT(strstr(content, "\"$ref\": \"#/components/schemas/Parent_coords\""));

    free(content);
  }

  remove("anon.h");
  remove("anon.json");
  PASS();
}

SUITE(anonymous_suite) { RUN_TEST(test_lift_anonymous_struct); }

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(anonymous_suite);
  GREATEST_MAIN_END();
}
