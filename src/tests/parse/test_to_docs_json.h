#if defined(_MSC_VER)
static FILE *cdd_freopen_helper_to_docs(const char *p, const char *m, FILE *s) {
  FILE *f = NULL;
  freopen_s(&f, p, m, s);
  return f;
}
#undef CDD_FREOPEN
#define CDD_FREOPEN cdd_freopen_helper_to_docs
#define dup _dup
#define dup2 _dup2
#define close _close
#define fileno _fileno
#else
#define CDD_FREOPEN freopen
#endif
/**
 */

/**
 * @file test_to_docs_json.h
 * @brief Tests for the to_docs_json CLI functionality.
 */

#ifndef C_CDD_TEST_TO_DOCS_JSON_H
#define C_CDD_TEST_TO_DOCS_JSON_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "functions/parse/fs.h"
#include "greatest.h"
#include "routes/parse/cli.h"
#include <parson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
/* clang-format on */

#define TEMP_OUT_FILE "to_docs_json_out.json"
#define TEMP_SPEC_FILE "test_spec.json"

static void write_test_spec(void) {
  const char spec[] = {
      123, 10,  32,  32,  34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  32,
      34,  51,  46,  50,  46,  48,  34,  44,  10,  32,  32,  34,  105, 110, 102,
      111, 34,  58,  32,  123, 32,  34,  116, 105, 116, 108, 101, 34,  58,  32,
      34,  84,  101, 115, 116, 32,  65,  80,  73,  34,  44,  32,  34,  118, 101,
      114, 115, 105, 111, 110, 34,  58,  32,  34,  49,  46,  48,  46,  48,  34,
      32,  125, 44,  10,  32,  32,  34,  112, 97,  116, 104, 115, 34,  58,  32,
      123, 10,  32,  32,  32,  32,  34,  47,  112, 101, 116, 34,  58,  32,  123,
      10,  32,  32,  32,  32,  32,  32,  34,  103, 101, 116, 34,  58,  32,  123,
      10,  32,  32,  32,  32,  32,  32,  32,  32,  34,  111, 112, 101, 114, 97,
      116, 105, 111, 110, 73,  100, 34,  58,  32,  34,  103, 101, 116, 80,  101,
      116, 34,  44,  10,  32,  32,  32,  32,  32,  32,  32,  32,  34,  114, 101,
      115, 112, 111, 110, 115, 101, 115, 34,  58,  32,  123, 10,  32,  32,  32,
      32,  32,  32,  32,  32,  32,  32,  34,  50,  48,  48,  34,  58,  32,  123,
      32,  34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111, 110, 34,  58,
      32,  34,  79,  75,  34,  32,  125, 10,  32,  32,  32,  32,  32,  32,  32,
      32,  125, 10,  32,  32,  32,  32,  32,  32,  125, 10,  32,  32,  32,  32,
      125, 10,  32,  32,  125, 10,  125, 0};
  FILE *fp = NULL;
#if defined(_MSC_VER)
  if (fopen_s(&fp, TEMP_SPEC_FILE, "w") != 0)
    fp = NULL;
#else
#if defined(_MSC_VER)
  if (fopen_s(&fp, TEMP_SPEC_FILE, "w") != 0)
    fp = NULL;
#else
  fp = fopen(TEMP_SPEC_FILE, "w");
#endif
#endif
  if (fp) {
    fputs(spec, fp);
    if (fp)
      fclose(fp);
  }
}

TEST test_to_docs_json_basic(void) {
  char *argv[] = {(char *)(size_t)(size_t) "to_docs_json",
                  (char *)(size_t)(size_t) "-i",
                  (char *)(size_t)(size_t)TEMP_SPEC_FILE};
  int rc;
  int stdout_fd = 0;
  JSON_Value *val = NULL;
  JSON_Object *root_obj = NULL;
  JSON_Object *endpoints_obj = NULL;
  JSON_Object *pet_obj = NULL;
  const char *code_str = NULL;
  fpos_t pos;
  (void)rc;

  memset(&pos, 0, sizeof(pos));

  write_test_spec();

  /* Redirect stdout to capture output */
  fflush(stdout);
  fgetpos(stdout, &pos);
  stdout_fd = dup(fileno(stdout));
  {
    FILE *f_tmp = NULL;
#if defined(_MSC_VER)
    if (fopen_s(&f_tmp, TEMP_OUT_FILE, "w") == 0) {
      dup2(fileno(f_tmp), fileno(stdout));
      fclose(f_tmp);
    }
#else
    f_tmp = fopen(TEMP_OUT_FILE, "w");
    if (f_tmp) {
      dup2(fileno(f_tmp), fileno(stdout));
      fclose(f_tmp);
    }
#endif
  }

  rc = to_docs_json_cli_main(3, argv);

  /* Restore stdout */
  fflush(stdout);
  dup2(stdout_fd, fileno(stdout));
  close(stdout_fd);
  clearerr(stdout);
  fsetpos(stdout, &pos);

  ASSERT_EQ(0, rc);

  val = json_parse_file(TEMP_OUT_FILE);
  ASSERT(val != NULL);

  root_obj = json_value_get_object(val);
  ASSERT(root_obj != NULL);

  endpoints_obj = json_object_get_object(root_obj, "endpoints");
  ASSERT(endpoints_obj != NULL);

  pet_obj = json_object_get_object(endpoints_obj, "/pet");
  ASSERT(pet_obj != NULL);

  code_str = json_object_get_string(pet_obj, "get");
  ASSERT(code_str != NULL);

  ASSERT(strstr(code_str, "#include \"generated_client.h\"") != NULL);
  ASSERT(strstr(code_str, "int main(void)") != NULL);
  ASSERT(strstr(code_str, "api_getPet") != NULL);

  json_value_free(val);

  remove(TEMP_SPEC_FILE);
  remove(TEMP_OUT_FILE);
  g_fail_io_after = -1;
  PASS();
}

TEST test_to_docs_json_no_imports_no_wrapping(void) {
  char *argv[] = {(char *)(size_t)(size_t) "to_docs_json",
                  (char *)(size_t)(size_t) "--no-imports",
                  (char *)(size_t)(size_t) "--no-wrapping",
                  (char *)(size_t)(size_t) "-i",
                  (char *)(size_t)(size_t)TEMP_SPEC_FILE};
  int rc;
  int stdout_fd = 0;
  JSON_Value *val = NULL;
  JSON_Object *root_obj = NULL;
  JSON_Object *endpoints_obj = NULL;
  JSON_Object *pet_obj = NULL;
  const char *code_str = NULL;
  fpos_t pos;
  (void)rc;

  memset(&pos, 0, sizeof(pos));

  write_test_spec();

  /* Redirect stdout */
  fflush(stdout);
  fgetpos(stdout, &pos);
  stdout_fd = dup(fileno(stdout));
  {
    FILE *f_tmp = NULL;
#if defined(_MSC_VER)
    if (fopen_s(&f_tmp, TEMP_OUT_FILE, "w") == 0) {
      dup2(fileno(f_tmp), fileno(stdout));
      fclose(f_tmp);
    }
#else
    f_tmp = fopen(TEMP_OUT_FILE, "w");
    if (f_tmp) {
      dup2(fileno(f_tmp), fileno(stdout));
      fclose(f_tmp);
    }
#endif
  }

  rc = to_docs_json_cli_main(5, argv);

  /* Restore stdout */
  fflush(stdout);
  dup2(stdout_fd, fileno(stdout));
  close(stdout_fd);
  clearerr(stdout);
  fsetpos(stdout, &pos);

  ASSERT_EQ(0, rc);

  val = json_parse_file(TEMP_OUT_FILE);
  ASSERT(val != NULL);

  root_obj = json_value_get_object(val);
  ASSERT(root_obj != NULL);

  endpoints_obj = json_object_get_object(root_obj, "endpoints");
  ASSERT(endpoints_obj != NULL);

  pet_obj = json_object_get_object(endpoints_obj, "/pet");
  ASSERT(pet_obj != NULL);

  code_str = json_object_get_string(pet_obj, "get");
  ASSERT(code_str != NULL);

  ASSERT(strstr(code_str, "#include") == NULL);
  ASSERT(strstr(code_str, "int main(void)") == NULL);
  ASSERT(strstr(code_str, "api_getPet") != NULL);

  json_value_free(val);

  remove(TEMP_SPEC_FILE);
  remove(TEMP_OUT_FILE);
  g_fail_io_after = -1;
  PASS();
}

SUITE(to_docs_json_suite) {
  RUN_TEST(test_to_docs_json_basic);
  RUN_TEST(test_to_docs_json_no_imports_no_wrapping);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_TEST_TO_DOCS_JSON_H */
