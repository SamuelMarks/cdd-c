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
/**
 * @brief Macro CDD_FREOPEN.
 */
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
#if defined(_MSC_VER)
#include <io.h>
#else
#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif
#endif
#endif
/* clang-format on */

/**
 * @brief Macro TEMP_OUT_FILE.
 */
#define TEMP_OUT_FILE "to_docs_json_out.json"
/**
 * @brief Macro TEMP_SPEC_FILE.
 */
#define TEMP_SPEC_FILE "test_spec.json"

/**
 * @brief Helper function write_test_spec.
 */
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

/**
 * @brief Unit test test_to_docs_json_basic.
 * @return Greatest test result.
 */
TEST test_to_docs_json_basic(void) {
  char *argv[3];
  int rc = 0;
  int stdout_fd = 0;
  JSON_Value *val = NULL;
  JSON_Object *root_obj = NULL;
  JSON_Object *endpoints_obj = NULL;
  JSON_Object *pet_obj = NULL;
  JSON_Object *get_obj = NULL;
  const char *imports_str = NULL;
  const char *wrapper_start_str = NULL;
  const char *snippet_str = NULL;
  const char *wrapper_end_str = NULL;
  fpos_t pos;

  argv[0] = cdd_test_mut_str("to_docs_json");
  argv[1] = cdd_test_mut_str("-i");
  argv[2] = cdd_test_mut_str(TEMP_SPEC_FILE);
  rc += 0;

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

  get_obj = json_object_get_object(pet_obj, "get");
  ASSERT(get_obj != NULL);

  imports_str = json_object_get_string(get_obj, "imports");
  wrapper_start_str = json_object_get_string(get_obj, "wrapper_start");
  snippet_str = json_object_get_string(get_obj, "snippet");
  wrapper_end_str = json_object_get_string(get_obj, "wrapper_end");

  ASSERT(imports_str != NULL);
  ASSERT(wrapper_start_str != NULL);
  ASSERT(snippet_str != NULL);
  ASSERT(wrapper_end_str != NULL);

  ASSERT(strstr(imports_str, "#include \"generated_client.h\"") != NULL);
  ASSERT(strstr(wrapper_start_str, "int main(void)") != NULL);
  ASSERT(strstr(snippet_str, "api_getPet(&client, &err)") != NULL);
  ASSERT(strstr(wrapper_end_str, "api_cleanup(&client);") != NULL);

  json_value_free(val);

  /* Test parameters, requestBody, webhooks, and directory -o output */
  {
    const char *rich_p1 =
        "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"R\",\"version\":\"1\"},"
        "\"paths\":{\"/pets\":{\"post\":{\"operationId\":\"createPet\","
        "\"parameters\":[{\"name\":\"petId\",\"in\":\"query\",\"required\":true"
        ",\"schema\":{\"type\":\"integer\"}},{\"name\":\"weight\",\"in\":"
        "\"query\",\"schema\":{\"type\":\"number\"}},{\"name\":\"name\","
        "\"in\":\"query\",\"schema\":{\"type\":\"string\"}},{\"name\":\"tags\","
        "\"in\":\"query\",\"schema\":{\"type\":\"array\",\"items\":{\"type\":"
        "\"string\"}}}],";
    const char *rich_p2 =
        "\"requestBody\":{\"content\":{\"application/json\":{\"schema\":"
        "{\"$ref\":\"#/components/schemas/Pet\"}}}},\"responses\":{\"200\":"
        "{\"description\":\"OK\"}}},\"put\":{\"operationId\":\"rawPet\","
        "\"requestBody\":{\"content\":{\"application/json\":{\"schema\":"
        "{\"type\":\"string\"}}}},\"responses\":{\"200\":{\"description\":"
        "\"OK\"}}}}},\"webhooks\":{\"newPet\":{\"post\":{\"operationId\":"
        "\"onNewPet\",\"responses\":{\"200\":{\"description\":\"OK\"}}}}},"
        "\"components\":{\"schemas\":{\"Pet\":{\"type\":\"object\"}}}}";
    char rich_spec[2048];
    cdd_docs_json_config_t cfg;
    JSON_Value *rval = NULL;
    JSON_Object *robj = NULL;
    JSON_Object *ep_obj = NULL;
    JSON_Object *wh_obj = NULL;
    JSON_Object *p_obj = NULL;
    JSON_Object *post_obj = NULL;
    JSON_Object *put_obj = NULL;
    JSON_Object *wh_pet_obj = NULL;
    JSON_Object *wh_post_obj = NULL;
    const char *post_snip = NULL;
    const char *put_snip = NULL;

    rich_spec[0] = '\0';
    CDD_STRCAT(rich_spec, sizeof(rich_spec), rich_p1);
    CDD_STRCAT(rich_spec, sizeof(rich_spec), rich_p2);
    ASSERT_EQ(CDD_C_SUCCESS, fs_write_to_file(TEMP_SPEC_FILE, rich_spec));

    memset(&cfg, 0, sizeof(cfg));
    cfg.input = TEMP_SPEC_FILE;
    cfg.output = ".";
#ifdef CDD_BUILD_TESTS
    {
      extern C_CDD_EXPORT int g_cdd_alloc_fail;
      int step;
      for (step = 1; step <= 40; ++step) {
        g_cdd_alloc_fail = step;
        cdd_generate_docs_json(&cfg);
        g_cdd_alloc_fail = 0;
      }
    }
#endif
    ASSERT_EQ(CDD_C_SUCCESS, cdd_generate_docs_json(&cfg));

    rval = json_parse_file("docs.json");
    ASSERT(rval != NULL);
    robj = json_value_get_object(rval);
    ep_obj = json_object_get_object(robj, "endpoints");
    wh_obj = json_object_get_object(robj, "webhooks");
    ASSERT(ep_obj != NULL);
    ASSERT(wh_obj != NULL);

    p_obj = json_object_get_object(ep_obj, "/pets");
    ASSERT(p_obj != NULL);
    post_obj = json_object_get_object(p_obj, "post");
    put_obj = json_object_get_object(p_obj, "put");
    ASSERT(post_obj != NULL);
    ASSERT(put_obj != NULL);

    post_snip = json_object_get_string(post_obj, "snippet");
    put_snip = json_object_get_string(put_obj, "snippet");
    ASSERT(post_snip != NULL);
    ASSERT(put_snip != NULL);
    ASSERT(strstr(post_snip, "int petId = 1;") != NULL);
    ASSERT(strstr(post_snip, "double weight = 1.0;") != NULL);
    ASSERT(strstr(post_snip, "const char *name = \"example\";") != NULL);
    ASSERT(strstr(post_snip, "const char *tags[] = {\"example\"};") != NULL);
    ASSERT(strstr(post_snip, "struct Pet req_body = {0};") != NULL);
    ASSERT(strstr(post_snip,
                  "api_createPet(&client, petId, weight, name, tags, 1, "
                  "&req_body, &err)") != NULL);
    ASSERT(strstr(put_snip, "const char *req_body = \"{}\";") != NULL);
    ASSERT(strstr(put_snip, "api_rawPet(&client, req_body, &err)") != NULL);

    wh_pet_obj = json_object_get_object(wh_obj, "newPet");
    ASSERT(wh_pet_obj != NULL);
    wh_post_obj = json_object_get_object(wh_pet_obj, "post");
    ASSERT(wh_post_obj != NULL);
    ASSERT(strstr(json_object_get_string(wh_post_obj, "snippet"),
                  "api_onNewPet(&client, &err)") != NULL);

    json_value_free(rval);
    remove("docs.json");
  }

  /* Test Arazzo workflows in cdd_generate_docs_json */
  {
    const char *az_spec_json =
        "{\n"
        "  \"arazzo\": \"1.1.0\",\n"
        "  \"info\": {\"title\": \"Flow\", \"version\": \"1.0.0\"},\n"
        "  \"sourceDescriptions\": [{\"name\": \"pet\", \"url\": "
        "\"openapi.json\", \"type\": \"openapi\"}],\n"
        "  \"workflows\": [\n"
        "    {\n"
        "      \"workflowId\": \"loginFlow\",\n"
        "      \"steps\": [{\"stepId\": \"s1\", \"operationId\": \"login\"}]\n"
        "    }\n"
        "  ]\n"
        "}\n";
    cdd_docs_json_config_t az_cfg;
    JSON_Value *az_val = NULL;
    JSON_Object *az_robj = NULL;
    JSON_Object *wf_obj = NULL;
    JSON_Object *login_obj = NULL;

    ASSERT_EQ(CDD_C_SUCCESS, fs_write_to_file(TEMP_SPEC_FILE, az_spec_json));
    memset(&az_cfg, 0, sizeof(az_cfg));
    az_cfg.input = TEMP_SPEC_FILE;
    az_cfg.output = TEMP_OUT_FILE;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_generate_docs_json(&az_cfg));

    az_val = json_parse_file(TEMP_OUT_FILE);
    ASSERT(az_val != NULL);
    az_robj = json_value_get_object(az_val);
    wf_obj = json_object_get_object(az_robj, "workflows");
    ASSERT(wf_obj != NULL);
    login_obj = json_object_get_object(wf_obj, "loginFlow");
    ASSERT(login_obj != NULL);
    ASSERT(strstr(json_object_get_string(login_obj, "snippet"),
                  "arazzo_run_workflow_loginFlow") != NULL);
    json_value_free(az_val);
  }

  remove(TEMP_SPEC_FILE);
  remove(TEMP_OUT_FILE);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_to_docs_json_no_imports_no_wrapping.
 * @return Greatest test result.
 */
TEST test_to_docs_json_no_imports_no_wrapping(void) {
  char *argv[5];
  int rc = 0;
  int stdout_fd = 0;
  JSON_Value *val = NULL;
  JSON_Object *root_obj = NULL;
  JSON_Object *endpoints_obj = NULL;
  JSON_Object *pet_obj = NULL;
  JSON_Object *get_obj = NULL;
  const char *snippet_str = NULL;
  fpos_t pos;

  argv[0] = cdd_test_mut_str("to_docs_json");
  argv[1] = cdd_test_mut_str("--no-imports");
  argv[2] = cdd_test_mut_str("--no-wrapping");
  argv[3] = cdd_test_mut_str("-i");
  argv[4] = cdd_test_mut_str(TEMP_SPEC_FILE);
  rc += 0;

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

  get_obj = json_object_get_object(pet_obj, "get");
  ASSERT(get_obj != NULL);

  ASSERT(json_object_get_string(get_obj, "imports") == NULL);
  ASSERT(json_object_get_string(get_obj, "wrapper_start") == NULL);
  ASSERT(json_object_get_string(get_obj, "wrapper_end") == NULL);

  snippet_str = json_object_get_string(get_obj, "snippet");
  ASSERT(snippet_str != NULL);
  ASSERT(strstr(snippet_str, "api_getPet(&client, &err)") != NULL);

  json_value_free(val);

  remove(TEMP_SPEC_FILE);
  remove(TEMP_OUT_FILE);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Executes the to_docs_json_suite test suite.
 */
SUITE(to_docs_json_suite) {
  RUN_TEST(test_to_docs_json_basic);
  RUN_TEST(test_to_docs_json_no_imports_no_wrapping);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_TEST_TO_DOCS_JSON_H */
