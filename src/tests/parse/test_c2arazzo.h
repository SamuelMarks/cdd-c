/**
 * @file test_c2arazzo.h
 * @brief Unit tests for Arazzo docstring parser (`doc_arazzo.c`) and
 * C-to-Arazzo extractor (`c2arazzo.c`).
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_TEST_C2ARAZZO_H
/** @brief Header guard for test_c2arazzo.h */
#define C_CDD_TEST_C2ARAZZO_H

/* clang-format off */
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arazzo/emit/arazzo.h"
#include "arazzo/parse/arazzo.h"
#include "arazzo/parse/c2arazzo.h"
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include "cdd_test_helpers/cdd_helpers.h"
#include "docstrings/parse/doc.h"
#include "docstrings/parse/doc_internal.h"
#include "functions/parse/fs.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef doc_parse_block
#undef doc_parse_block
#endif

/**
 * @brief Tests parsing of all Arazzo Doxygen annotations in `doc_arazzo.c`.
 * @return Test or helper result.
 */
TEST test_doc_arazzo_all_directives(void) {
  struct DocMetadata meta;
  int handled = 0;
  char comment[4096];
  const char *chunks[] = {
      "/**\n"
      " * @arazzo 1.0.0\n"
      " * @arazzo 1.1.0\n"
      " * @self https://example.com/old.json\n"
      " * @self https://example.com/arazzo.json\n"
      " * @infoTitle Pet Workflow\n"
      " * @infoVersion 2.0.0\n"
      " * @infoSummary Summary text\n"
      " * @infoDescription Description text\n",
      " * @sourceDescription petstore ./openapi.json [type:openapi] "
      "[type:openapi]\n"
      " * @sourceDescription events ./asyncapi.json\n"
      " * @sourceDescription\n"
      " * @sourceDescription onlyname\n"
      " * @sourceDescription unclosed ./url [type:openapi\n",
      " * @workflow oldWf [summary:Old] Old desc\n"
      " * @workflow loginWf [summary:First] [summary:Login summary] Login "
      "workflow description\n"
      " * @workflow\n"
      " * @workflow unclosedWf [summary:abc\n"
      " * @workflowSummary Overridden summary\n"
      " * @workflowDependsOn initWf, setupWf\n"
      " * @workflowDependsOn\n",
      " * @workflowInput {\"type\":\"object\"}\n"
      " * @workflowInput {\"type\":\"object\",\"properties\":{\"u\":"
      "{\"type\":\"string\"}}}\n"
      " * @workflowOutput token $steps.s1.outputs.tok\n"
      " * @workflowOutput\n"
      " * @workflowParam apiKey [in:header] [in:query] $inputs.key\n",
      " * @workflowParam refParam [reference:$components.parameters.p1] "
      "[reference:$components.parameters.p2] \"v\"\n"
      " * @workflowParam unclosedParam [in:header val\n"
      " * @workflowParam\n"
      " * @stepDependsOn preStep\n",
      " * @step s1 [operationId:oldOp] [operationId:loginUser] "
      "[operationPath:{$sourceDescriptions.petstore.url}#/paths/~1login/post] "
      "[operationPath:{$sourceDescriptions.petstore.url}#/paths/~1v2/post]\n",
      " * @step s1b [channelPath:/events] [channelPath:/events2] "
      "[workflowId:sub1] [workflowId:sub2] [timeout:5000] "
      "[correlationId:$message.header#/id] "
      "[correlationId:$message.header#/id2] [action:send] [action:receive] "
      "Step 1 desc\n",
      " * @step s2 [operationId:getProfile] [action:send] Step 2\n"
      " * @step s3 [operationPath:{$sourceDescriptions.petstore.url}#/paths] "
      "[unclosed\n"
      " * @step s4 [channelPath:/ch]\n"
      " * @step s5 [workflowId:nestedWf]\n"
      " * @step\n"
      " * @stepDependsOn s1, s2\n",
      " * @stepParam p1 [in:path] $inputs.id\n"
      " * @stepParam p2 [reference:$components.parameters.shared] "
      "\"override\"\n"
      " * @stepRequestBody [contentType:text/plain] "
      "[contentType:application/json] {\"a\":1}\n"
      " * @stepRequestBody [unclosed\n"
      " * @stepRequestBody $inputs.u\n",
      " * @stepReplacement /a $inputs.u\n"
      " * @stepReplacement $.b \"literal\"\n"
      " * @stepReplacement\n"
      " * @stepCriterion [context:$response.body] [context:$statusCode] "
      "[type:simple] [type:regex] [version:1.0] [version:draft] ^200$\n",
      " * @stepCriterion [context:$response.body] [type:jsonpath] "
      "[version:draft-goessner-dispatch-jsonpath-00] $[?(@.ok == true)]\n"
      " * @stepCriterion [type:jsonpath] $.ok\n"
      " * @stepCriterion [type:xpath] [unclosed\n",
      " * @stepOnSuccess done [type:end] [type:goto] [stepId:s1] [stepId:s2] "
      "[workflowId:w1] [workflowId:w2]\n"
      " * @stepOnSuccess refAct [reference:$components.successActions.sa1] "
      "[reference:$components.successActions.sa2] [unclosed\n"
      " * @stepOnSuccess\n",
      " * @stepOnFailure retryAct [type:retry] [retryAfter:1.5] [retryLimit:3] "
      "[stepId:s1] [workflowId:w1]\n"
      " * @stepOnFailure refFail [reference:$components.failureActions.fa1]\n"
      " * @stepOutput outKey $response.body#/id\n",
      " * @arazzoComponent parameters sharedParam {\"name\":\"x\",\"in\":"
      "\"query\",\"value\":\"1\"}\n"
      " * @arazzoComponent\n"
      " * @arazzoComponent onlykind\n"
      " * @arazzocomponent inputs in1 {\"type\":\"string\"}\n"
      " * @sourcedescription sd2 ./oa2.json [type:invalid_type] [other:1]\n"
      " * @workflow w2 [other:1]\n"
      " * @workflowsummary Lowercase summary\n",
      " * @workflowdependson d1 , , d2,\n"
      " * @workflowinput {\"type\":\"object\"}\n"
      " * @workflowoutput ko1 $statusCode\n"
      " * @workflowoutput ko_noval\n"
      " * @workflowparam wp1 [in:invalid_loc] [other:1] plain_val\n"
      " * @workflowparam wp_noval [in:query]\n"
      " * @step s_extra [action:other] [other:1]\n"
      " * @stepdependson sd1\n",
      " * @stepparam sp1 {embedded_$inputs.x}\n"
      " * @steprequestbody [other:1] {\"k\":1}\n"
      " * @stepreplacement /k {embedded_$inputs.x}\n"
      " * @stepreplacement /noval\n"
      " * @stepcriterion [type:xpath] [other:1] /a/b\n"
      " * @stepcriterion [type:other] /a/b\n"
      " * @steponsuccess sa_bad [type:unknown_act] [other:1]\n"
      " * @steponsuccess sa_notype\n",
      " * @steponfailure fa_bad [type:unknown_act] [other:1]\n"
      " * @steponfailure fa_notype\n"
      " * @stepoutput so1 $statusCode\n"
      " * @x-custom-ext {\"enabled\":true}\n"
      " * @x-flag_ext\n"
      " * @xref something\n"
      " * @\n"
      " */",
      NULL};
  size_t idx = 0;

  comment[0] = '\0';
  while (chunks[idx]) {
    CDD_STRCAT(comment, sizeof(comment), chunks[idx]);
    idx++;
  }

  ASSERT_EQ(
      CDD_C_ERROR_INVALID_ARGUMENT,
      doc_parse_arazzo_directive(NULL, comment, comment, &meta, &handled));
  ASSERT_EQ(
      CDD_C_ERROR_INVALID_ARGUMENT,
      doc_parse_arazzo_directive("arazzo", NULL, comment, &meta, &handled));
  ASSERT_EQ(
      CDD_C_ERROR_INVALID_ARGUMENT,
      doc_parse_arazzo_directive("arazzo", comment, NULL, &meta, &handled));
  ASSERT_EQ(
      CDD_C_ERROR_INVALID_ARGUMENT,
      doc_parse_arazzo_directive("arazzo", comment, comment, NULL, &handled));
  ASSERT_EQ(
      CDD_C_ERROR_INVALID_ARGUMENT,
      doc_parse_arazzo_directive("arazzo", comment, comment, &meta, NULL));
  doc_free_arazzo_metadata(NULL);

  /* OOM sweep on doc_parse_block with all Arazzo directives */
  {
    int f;
    for (f = 1; f <= 265; ++f) {
      g_cdd_alloc_fail = f;
      if (doc_metadata_init(&meta) == CDD_C_SUCCESS) {
        doc_parse_block(comment, &meta);
        doc_metadata_free(&meta);
      }
      g_cdd_alloc_fail = 0;
    }
    for (f = 1; f <= 70; ++f) {
      g_cdd_strdup_fail = f;
      if (doc_metadata_init(&meta) == CDD_C_SUCCESS) {
        doc_parse_block(comment, &meta);
        doc_metadata_free(&meta);
      }
      g_cdd_strdup_fail = 0;
    }
  }

  /* Test @stepParam / @stepRequestBody / @stepReplacement / @stepCriterion /
   * @stepOnSuccess / @stepOnFailure / @stepOutput when no prior @step exists */
  {
    const char *implicit_steps[] = {
        "/** @stepParam p1 val */",
        "/** @stepRequestBody {\"a\":1} */",
        "/** @stepReplacement /a 1 */",
        "/** @stepCriterion $statusCode == 200 */",
        "/** @stepOnSuccess done [type:end] */",
        "/** @stepOnFailure err [type:end] */",
        "/** @stepOutput out $statusCode */",
        "/** @x-custom-ext {\"enabled\":true} */",
        "/** @workflow emptyStepsWf\n * @workflowInput not_an_object */",
        "/** @workflow validInputNonObj\n * @workflowInput [1,2,3] */",
        NULL};
    size_t k;
    for (k = 0; implicit_steps[k]; ++k) {
      int f;
      for (f = 1; f <= 6; ++f) {
        g_cdd_alloc_fail = f;
        if (doc_metadata_init(&meta) == CDD_C_SUCCESS) {
          doc_parse_block(implicit_steps[k], &meta);
          doc_metadata_free(&meta);
        }
        g_cdd_alloc_fail = 0;
        g_cdd_strdup_fail = f;
        if (doc_metadata_init(&meta) == CDD_C_SUCCESS) {
          doc_parse_block(implicit_steps[k], &meta);
          doc_metadata_free(&meta);
        }
        g_cdd_strdup_fail = 0;
      }
      ASSERT_EQ(CDD_C_SUCCESS, doc_metadata_init(&meta));
      ASSERT_EQ(CDD_C_SUCCESS, doc_parse_block(implicit_steps[k], &meta));
      {
        struct Arazzo_Spec s2;
        if (arazzo_spec_init(&s2) == CDD_C_SUCCESS) {
          meta.workflow_id = C_CDD_STRDUP("impWf");
          c2arazzo_apply_doc_metadata(&meta, &s2);
          arazzo_spec_free(&s2);
        }
      }
      doc_metadata_free(&meta);
    }
  }

  ASSERT_EQ(CDD_C_SUCCESS, doc_metadata_init(&meta));
  ASSERT_EQ(CDD_C_SUCCESS, doc_parse_block(comment, &meta));

  ASSERT_STR_EQ("1.1.0", meta.arazzo_version);
  ASSERT_STR_EQ("https://example.com/arazzo.json", meta.self_uri);
  ASSERT_EQ(2, meta.n_arazzo_extensions);
  ASSERT_STR_EQ("x-custom-ext", meta.arazzo_extensions[0].key);

  {
    struct Arazzo_Spec spec;
    int f;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2arazzo_apply_doc_metadata(NULL, &spec));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2arazzo_apply_doc_metadata(&meta, NULL));
    for (f = 1; f <= 80; ++f) {
      memset(&spec, 0, sizeof(spec));
      g_cdd_alloc_fail = f;
      if (arazzo_spec_init(&spec) == CDD_C_SUCCESS) {
        c2arazzo_apply_doc_metadata(&meta, &spec);
      }
      g_cdd_alloc_fail = 0;
      arazzo_spec_free(&spec);
    }
    for (f = 1; f <= 115; ++f) {
      memset(&spec, 0, sizeof(spec));
      g_cdd_strdup_fail = f;
      if (arazzo_spec_init(&spec) == CDD_C_SUCCESS) {
        c2arazzo_apply_doc_metadata(&meta, &spec);
      }
      g_cdd_strdup_fail = 0;
      arazzo_spec_free(&spec);
    }
    ASSERT_EQ(CDD_C_SUCCESS, arazzo_spec_init(&spec));
    ASSERT_EQ(CDD_C_SUCCESS, c2arazzo_apply_doc_metadata(&meta, &spec));
    ASSERT_EQ(CDD_C_SUCCESS, c2arazzo_apply_doc_metadata(&meta, &spec));
    arazzo_spec_free(&spec);
  }

  doc_metadata_free(&meta);
  PASS();
}

/**
 * @brief Tests `c2arazzo_parse_source`, `c2arazzo_parse_file`,
 * `c2arazzo_parse_dir`, and `c2arazzo_cli_main`.
 * @return Test or helper result.
 */
TEST test_c2arazzo_source_and_cli(void) {
  struct Arazzo_Spec spec;
  char src_path[512];
  char out_path[512];
  const char *c_code = "/**\n"
                       " * @workflow annotatedWf Annotated workflow\n"
                       " * @step stepA [operationId:opA] First step\n"
                       " */\n"
                       "int api_workflow_annotatedWf(void) {\n"
                       "  return 0;\n"
                       "}\n"
                       "int api_workflow_inferredSteps(void) {\n"
                       "  int x = 0;\n"
                       "  int normal_long_var = 0;\n"
                       "  int step_nolabel = 1;\n"
                       "step_login:\n"
                       "  goto step_fetch;\n"
                       "step_fetch:\n"
                       "  return x + normal_long_var + step_nolabel;\n"
                       "}\n"
                       "int api_workflow_singleStep(void) {\n"
                       "  return 0;\n"
                       "}\n"
                       "int normal_helper_func(int x) {\n"
                       "  return x + 1;\n"
                       "}\n";

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c2arazzo_parse_source(NULL, &spec));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c2arazzo_parse_source(c_code, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c2arazzo_parse_file(NULL, &spec));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c2arazzo_parse_file("nonexistent.c", NULL));
  ASSERT_NEQ(CDD_C_SUCCESS,
             c2arazzo_parse_file("nonexistent_c2arazzo_file.c", &spec));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c2arazzo_parse_dir(NULL, &spec));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c2arazzo_parse_dir(".", NULL));
  memset(&spec, 0, sizeof(spec));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, c2arazzo_parse_dir(".", &spec));
  g_cdd_alloc_fail = 0;
  memset(&spec, 0, sizeof(spec));
  ASSERT_NEQ(CDD_C_SUCCESS,
             c2arazzo_parse_dir("nonexistent_c2arazzo_dir", &spec));
  arazzo_spec_free(&spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c2arazzo_cli_main(0, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c2arazzo_cli_main(1, NULL));

  /* Empty C source defaults */
  {
    int f;
    for (f = 1; f <= 20; ++f) {
      memset(&spec, 0, sizeof(spec));
      g_cdd_alloc_fail = f;
      c2arazzo_parse_source("int x = 1;\n", &spec);
      g_cdd_alloc_fail = 0;
      arazzo_spec_free(&spec);

      memset(&spec, 0, sizeof(spec));
      g_cdd_strdup_fail = f;
      c2arazzo_parse_source("int x = 1;\n", &spec);
      g_cdd_strdup_fail = 0;
      arazzo_spec_free(&spec);
    }
    memset(&spec, 0, sizeof(spec));
    ASSERT_EQ(CDD_C_SUCCESS, c2arazzo_parse_source("int x = 1;\n", &spec));
    ASSERT_EQ(1, spec.n_workflows);
    arazzo_spec_free(&spec);
  }

  /* OOM sweep on c2arazzo_parse_source */
  {
    int f;
    for (f = 1; f <= 45; ++f) {
      memset(&spec, 0, sizeof(spec));
      g_cdd_alloc_fail = f;
      c2arazzo_parse_source(c_code, &spec);
      g_cdd_alloc_fail = 0;
      arazzo_spec_free(&spec);
    }
    for (f = 1; f <= 30; ++f) {
      memset(&spec, 0, sizeof(spec));
      g_cdd_strdup_fail = f;
      c2arazzo_parse_source(c_code, &spec);
      g_cdd_strdup_fail = 0;
      arazzo_spec_free(&spec);
    }
  }

  memset(&spec, 0, sizeof(spec));
  ASSERT_EQ(CDD_C_SUCCESS, c2arazzo_parse_source(c_code, &spec));
  ASSERT_EQ(3, spec.n_workflows);
  arazzo_spec_free(&spec);

  ASSERT_EQ(CDD_C_SUCCESS, makedirs("test_build_dir/c2arazzo_walk"));
  CDD_STRCPY(src_path, sizeof(src_path),
             "test_build_dir/c2arazzo_walk/c2arazzo_in.c");
  CDD_STRCPY(out_path, sizeof(out_path),
             "test_build_dir/c2arazzo_walk/c2arazzo_out.json");
  ASSERT_EQ(CDD_C_SUCCESS, fs_write_to_file(src_path, c_code));
  ASSERT_EQ(CDD_C_SUCCESS,
            fs_write_to_file("test_build_dir/c2arazzo_walk/readme.txt", "hi"));

  memset(&spec, 0, sizeof(spec));
  ASSERT_EQ(CDD_C_SUCCESS, c2arazzo_parse_dir(src_path, &spec));
  ASSERT_EQ(3, spec.n_workflows);
  arazzo_spec_free(&spec);

  memset(&spec, 0, sizeof(spec));
  ASSERT_EQ(CDD_C_SUCCESS,
            c2arazzo_parse_dir("test_build_dir/c2arazzo_walk", &spec));
  ASSERT_EQ(3, spec.n_workflows);
  arazzo_spec_free(&spec);

  {
    char *argv1[8];
    char *argv1_long[8];
    char *argv2[6];
    char *argv_def_out[4];
    char *argv_bad1[3];
    char *argv_bad2[3];
    char *argv_bad3[3];
    char *argv_bad4[2];
    int f;

    argv1[0] = C_CDD_STR_LIT("c2arazzo");
    argv1[1] = C_CDD_STR_LIT("-i");
    argv1[2] = src_path;
    argv1[3] = C_CDD_STR_LIT("-o");
    argv1[4] = out_path;
    argv1[5] = C_CDD_STR_LIT("--self");
    argv1[6] = C_CDD_STR_LIT("https://example.com/a.json");
    argv1[7] = NULL;

    argv1_long[0] = C_CDD_STR_LIT("c2arazzo");
    argv1_long[1] = C_CDD_STR_LIT("--input");
    argv1_long[2] = src_path;
    argv1_long[3] = C_CDD_STR_LIT("--output");
    argv1_long[4] = out_path;
    argv1_long[5] = C_CDD_STR_LIT("-s");
    argv1_long[6] = C_CDD_STR_LIT("https://example.com/b.json");
    argv1_long[7] = NULL;

    argv2[0] = C_CDD_STR_LIT("c2arazzo");
    argv2[1] = src_path;
    argv2[2] = out_path;
    argv2[3] = C_CDD_STR_LIT("extra_pos");
    argv2[4] = C_CDD_STR_LIT("-u");
    argv2[5] = NULL;

    argv_def_out[0] = C_CDD_STR_LIT("c2arazzo");
    argv_def_out[1] = C_CDD_STR_LIT("-i");
    argv_def_out[2] = src_path;
    argv_def_out[3] = NULL;

    argv_bad1[0] = C_CDD_STR_LIT("c2arazzo");
    argv_bad1[1] = C_CDD_STR_LIT("-i");
    argv_bad1[2] = NULL;

    argv_bad2[0] = C_CDD_STR_LIT("c2arazzo");
    argv_bad2[1] = C_CDD_STR_LIT("-o");
    argv_bad2[2] = NULL;

    argv_bad3[0] = C_CDD_STR_LIT("c2arazzo");
    argv_bad3[1] = C_CDD_STR_LIT("--self");
    argv_bad3[2] = NULL;

    argv_bad4[0] = C_CDD_STR_LIT("c2arazzo");
    argv_bad4[1] = NULL;

    ASSERT_EQ(CDD_C_SUCCESS, c2arazzo_cli_main(7, argv1));
    ASSERT_EQ(CDD_C_SUCCESS, c2arazzo_cli_main(7, argv1_long));
    ASSERT_EQ(CDD_C_SUCCESS, c2arazzo_cli_main(5, argv2));
    ASSERT_EQ(CDD_C_SUCCESS, c2arazzo_cli_main(3, argv_def_out));
    remove("arazzo.json");
    ASSERT_EQ(CDD_C_SUCCESS, fs_write_to_file(src_path, "int x = 1;\n"));
    for (f = 1; f <= 25; ++f) {
      g_cdd_alloc_fail = f;
      c2arazzo_cli_main(7, argv1);
      g_cdd_alloc_fail = 0;
    }
    for (f = 1; f <= 25; ++f) {
      g_cdd_strdup_fail = f;
      c2arazzo_cli_main(7, argv1);
      g_cdd_strdup_fail = 0;
    }
    ASSERT_EQ(CDD_C_SUCCESS, fs_write_to_file(src_path, c_code));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c2arazzo_cli_main(2, argv_bad1));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c2arazzo_cli_main(2, argv_bad2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c2arazzo_cli_main(2, argv_bad3));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c2arazzo_cli_main(1, argv_bad4));
  }

  remove(src_path);
  remove(out_path);
  remove("test_build_dir/c2arazzo_walk/readme.txt");
  PASS();
}

/**
 * @brief Suite for C-to-Arazzo extraction tests.
 */
SUITE(c2arazzo_suite) {
  RUN_TEST(test_doc_arazzo_all_directives);
  RUN_TEST(test_c2arazzo_source_and_cli);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_TEST_C2ARAZZO_H */
