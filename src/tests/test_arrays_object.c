/**
 * @file test_arrays_object.c
 * @brief Unit tests for Object Arrays generation and parsing.
 *
 * Verifies that the code generator correctly handles arrays of nested objects,
 * including memory allocation, recursive parsing, and cleanup.
 *
 * @author Samuel Marks
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <greatest.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "functions/emit_codegen.h"

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

TEST test_generated_obj_array_logic(void) {
  /*
     Verify the generator output for a structure:
     struct Container {
       struct Item **items;
       size_t n_items;
     };
  */

  struct StructFields sf;
  char *output_buf = NULL;
  size_t output_len;
  FILE *tmp;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "items", "array", "Item", NULL, NULL);

  tmp = tmpfile();
  ASSERT(tmp);

  write_struct_from_jsonObject_func(tmp, "Container", &sf, NULL);

  fseek(tmp, 0, SEEK_END);
  output_len = ftell(tmp);
  rewind(tmp);

  output_buf = malloc(output_len + 1);
  fread(output_buf, 1, output_len, tmp);
  output_buf[output_len] = 0;

  /* Verify malloc logic for array of pointers */
  /* Expect: ret->items = malloc(ret->n_items * sizeof(struct Item*)); */
  ASSERT(strstr(output_buf, "sizeof(struct Item*)"));

  /* Verify loop for object parsing */
  /* Expect: rc = Item_from_jsonObject(..., &ret->items[i]); */
  ASSERT(strstr(output_buf,
                "rc = Item_from_jsonObject(obj_item, &ret->items[i]);"));

  /* Verify cleanup on error */
  ASSERT(strstr(output_buf, "Container_cleanup(ret);"));

  free(output_buf);
  fclose(tmp);
  struct_fields_free(&sf);
  PASS();
}

TEST test_code2schema_obj_array_detection(void) {
  /*
   * Verify that code2schema correctly collapses:
   *   struct Item **items;
   *   size_t n_items;
   * into "items": { "type": "array", "items": { "$ref": "#/.../Item" } }
   */
  const char *header =
      "struct Item { int id; };\n"
      "struct Container { struct Item **items; size_t n_items; };";
  const char *json_out_file = "test_obj_array_detect.json";
  FILE *f;
  char *json_content;
  size_t len;

  write_to_file("test_obj_array.h", header);

  {
/* code2schema_main calls parse_header_file */
#include "classes/parse_code2schema.h"
    char *argv[] = {"test_obj_array.h", (char *)json_out_file};
    ASSERT_EQ(EXIT_SUCCESS, code2schema_main(2, argv));
  }

  /* Read JSON output */
  f = fopen(json_out_file, "r");
  ASSERT(f);
  fseek(f, 0, SEEK_END);
  len = ftell(f);
  rewind(f);
  json_content = malloc(len + 1);
  fread(json_content, 1, len, f);
  json_content[len] = 0;
  fclose(f);

  /* Validate Schema Structure */
  ASSERT(strstr(json_content, "\"items\":"));
  ASSERT(strstr(json_content, "\"type\": \"array\""));
  ASSERT(strstr(json_content, "\"$ref\": \"#/components/schemas/Item\""));

  /* Verify n_items is NOT in the properties list explicitly */
  ASSERT(strstr(json_content, "\"n_items\"") == NULL);

  free(json_content);
  remove("test_obj_array.h");
  remove(json_out_file);
  PASS();
}

TEST test_cleanup_generation(void) {
  /* Verify deep cleanup generation logic */
  struct StructFields sf;
  char *output_buf = NULL;
  size_t output_len;
  FILE *tmp;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "items", "array", "Item", NULL, NULL);

  tmp = tmpfile();
  ASSERT(tmp);

  write_struct_cleanup_func(tmp, "Container", &sf, NULL);

  fseek(tmp, 0, SEEK_END);
  output_len = ftell(tmp);
  rewind(tmp);

  output_buf = malloc(output_len + 1);
  fread(output_buf, 1, output_len, tmp);
  output_buf[output_len] = 0;

  /* Check loop */
  ASSERT(strstr(output_buf, "for (i = 0; i < obj->n_items; ++i)"));
  /* Check item cleanup */
  ASSERT(strstr(output_buf, "Item_cleanup(obj->items[i]);"));

  free(output_buf);
  fclose(tmp);
  struct_fields_free(&sf);
  PASS();
}

SUITE(arrays_object_suite) {
  RUN_TEST(test_generated_obj_array_logic);
  RUN_TEST(test_code2schema_obj_array_detection);
  RUN_TEST(test_cleanup_generation);
}

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(arrays_object_suite);
  GREATEST_MAIN_END();
}
