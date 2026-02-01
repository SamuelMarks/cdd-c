/**
 * @file test_c_cdd.c
 * @brief Main test runner.
 */

#include <stdlib.h>
#include <time.h>

#include <greatest.h>

#include "test_analysis.h"
#include "test_c_cdd_integration.h"
#include "test_c_inspector_types.h"
#include "test_code2schema.h"
#include "test_codegen_defaults.h"
#include "test_codegen_eq.h"
#include "test_codegen_root_arrays.h"
#include "test_codegen_validation.h"
#include "test_cst_parser.h"
#include "test_dataclasses.h"
#include "test_declarator_parser.h" /* <--- Added */
#include "test_flexible_array.h"
#include "test_fs.h"
#include "test_initializer_parser.h"
#include "test_json_from_and_to.h"
#include "test_numeric_parser.h"
#include "test_parsing.h"
#include "test_pragma.h"
#include "test_preprocessor.h"
#include "test_preprocessor_macros.h"
#include "test_project_audit.h"
#include "test_refactor_orchestrator.h"
#include "test_rewriter_body.h"
#include "test_rewriter_sig.h"
#include "test_schema2tests.h"
#include "test_schema_codegen.h"
#include "test_sync_code.h"
#include "test_tokenizer.h"
#include "test_tokenizer_trigraphs.h"

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  srand((unsigned int)time(NULL));

  RUN_SUITE(analysis_suite);
  RUN_SUITE(code2schema_suite);
  RUN_SUITE(c_inspector_types_suite);
  RUN_SUITE(codegen_defaults_suite);
  RUN_SUITE(codegen_eq_suite);
  RUN_SUITE(codegen_validation_suite);
  RUN_SUITE(root_array_suite);
  RUN_SUITE(cst_parser_suite);
  RUN_SUITE(dataclasses_suite);
  RUN_SUITE(declarator_parser_suite); /* <--- Added */
  RUN_SUITE(flexible_array_suite);
  RUN_SUITE(fs_suite);
  RUN_SUITE(initializer_parser_suite);
  RUN_SUITE(integration_suite);
  RUN_SUITE(json_from_and_to_suite);
  RUN_SUITE(numeric_parser_suite);
  RUN_SUITE(parsing_suite);
  RUN_SUITE(pragma_suite);
  RUN_SUITE(preprocessor_suite);
  RUN_SUITE(preprocessor_macros_suite);
  RUN_SUITE(project_audit_suite);
  RUN_SUITE(refactor_orchestrator_suite);
  RUN_SUITE(rewriter_body_suite);
  RUN_SUITE(rewriter_sig_suite);
  RUN_SUITE(schema2tests_suite);
  RUN_SUITE(schema_codegen_suite);
  RUN_SUITE(sync_code_suite);
  RUN_SUITE(tokenizer_suite);
  RUN_SUITE(tokenizer_trigraphs_suite);

  GREATEST_MAIN_END();
}
