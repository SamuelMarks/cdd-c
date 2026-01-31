/**
 * @file test_c_cdd.c
 * @brief Main test runner aggregating all test suites.
 * @author Samuel Marks
 */

#include <greatest.h>

/* Forward declarations or include headers containing suites */
/* These headers are expected to be available in the include path */
#include "test_analysis.h"
#include "test_c_cdd_integration.h"
#include "test_code2schema.h"
#include "test_cst_parser.h"
#include "test_dataclasses.h"
#include "test_fs.h"
#include "test_json_from_and_to.h"
#include "test_parsing.h"
#include "test_project_audit.h"
#include "test_refactor_orchestrator.h"
#include "test_rewriter_body.h"
#include "test_rewriter_sig.h"
#include "test_schema2tests.h"
#include "test_schema_codegen.h"
#include "test_sync_code.h"
#include "test_tokenizer.h"

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

/**
 * @brief Main test executable entry point.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return 0 on success (all tests passed), non-zero otherwise.
 */
int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();

  /* Run all test suites */
  RUN_SUITE(analysis_suite);
  RUN_SUITE(code2schema_suite);
  RUN_SUITE(cst_parser_suite);
  RUN_SUITE(dataclasses_suite);
  RUN_SUITE(fs_suite);
  RUN_SUITE(integration_suite);
  RUN_SUITE(json_from_and_to_suite);
  RUN_SUITE(parsing_suite);
  RUN_SUITE(project_audit_suite);
  RUN_SUITE(refactor_orchestrator_suite);
  RUN_SUITE(rewriter_body_suite);
  RUN_SUITE(rewriter_sig_suite);
  RUN_SUITE(schema2tests_suite);
  RUN_SUITE(schema_codegen_suite);
  RUN_SUITE(sync_code_suite);
  RUN_SUITE(tokenizer_suite);

  GREATEST_MAIN_END();
}
