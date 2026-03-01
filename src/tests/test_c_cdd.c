/**
 * @file test_c_cdd.c
 * @brief Main test runner.
 */

#include <stdlib.h>
#include <time.h>

#include <greatest.h>

#include "emit/test_codegen_build.h"
#include "emit/test_codegen_client_body.h"
#include "emit/test_codegen_client_sig.h"
#include "emit/test_codegen_defaults.h"
#include "emit/test_codegen_eq.h"
#include "emit/test_codegen_json.h"
#include "emit/test_codegen_make.h"
#include "emit/test_codegen_root_arrays.h"
#include "emit/test_codegen_sdk_tests.h"
#include "emit/test_codegen_security.h"
#include "emit/test_codegen_struct.h"
#include "emit/test_codegen_types.h"
#include "emit/test_codegen_url.h"
#include "emit/test_codegen_validation.h"
#include "emit/test_generate_build_system.h"
#include "emit/test_http_curl.h"
#include "emit/test_http_types.h"
#include "emit/test_http_winhttp.h"
#include "emit/test_http_wininet.h"
#include "emit/test_openapi_client_gen.h"
#include "emit/test_rewriter_body.h"
#include "emit/test_rewriter_sig.h"
#include "emit/test_schema2tests.h"
#include "emit/test_schema_codegen.h"
#include "emit/test_sync_code.h"
#include "emit/test_text_patcher.h"
#include "emit/test_url_utils.h"
#include "parse/test_analysis.h"
#include "parse/test_c_cdd_integration.h"
#include "parse/test_c_inspector_types.h"
#include "parse/test_code2schema.h"
#include "parse/test_crypto.h"
#include "parse/test_cst_parser.h"
#include "parse/test_dataclasses.h"
#include "parse/test_declarator_parser.h"
#include "parse/test_flexible_array.h"
#include "parse/test_fs.h"
#include "parse/test_initializer_parser.h"
#include "parse/test_integration_server.h"
#include "parse/test_json_from_and_to.h"
#include "parse/test_numeric_parser.h"
#include "parse/test_openapi_loader.h"
#include "parse/test_parsing.h"
#include "parse/test_pragma.h"
#include "parse/test_preprocessor.h"
#include "parse/test_preprocessor_macros.h"
#include "parse/test_project_audit.h"
#include "parse/test_refactor_api_sync.h"
#include "parse/test_refactor_orchestrator.h"
#include "parse/test_schema_constraints.h"
#include "parse/test_schema_enum_required.h"
#include "parse/test_simple_json.h"
#include "parse/test_str_utils.h"
#include "parse/test_tokenizer.h"
#include "parse/test_tokenizer_trigraphs.h"
#include "parse/test_transport_factory.h"

/* New Suites */
#include "emit/test_aggregator.h"
#include "emit/test_openapi_writer.h"
#include "parse/test_c2openapi_op.h"
#include "parse/test_c2openapi_schema.h"
#include "parse/test_c_mapping.h"
#include "parse/test_doc_parser.h"
#include "parse/test_integration_c2openapi.h"
#include "parse/test_to_docs_json.h"

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  srand((unsigned int)time(NULL));

  RUN_SUITE(analysis_suite);
  RUN_SUITE(code2schema_suite);
  RUN_SUITE(c_inspector_types_suite);
  RUN_SUITE(codegen_build_suite);
  RUN_SUITE(client_body_suite);
  RUN_SUITE(client_sig_suite);
  RUN_SUITE(codegen_defaults_suite);
  RUN_SUITE(codegen_eq_suite);
  RUN_SUITE(codegen_json_suite);
  RUN_SUITE(codegen_make_suite);
  RUN_SUITE(codegen_security_suite);
  RUN_SUITE(codegen_struct_suite);
  RUN_SUITE(codegen_types_suite);
  RUN_SUITE(codegen_url_suite);
  RUN_SUITE(codegen_validation_suite);
  RUN_SUITE(root_array_suite);
  RUN_SUITE(codegen_sdk_tests_suite);
  RUN_SUITE(cst_parser_suite);
  RUN_SUITE(crypto_suite);
  RUN_SUITE(dataclasses_suite);
  RUN_SUITE(declarator_parser_suite);
  RUN_SUITE(flexible_array_suite);
  RUN_SUITE(fs_suite);
  RUN_SUITE(generate_build_system_suite);
  RUN_SUITE(http_types_suite);
  RUN_SUITE(http_curl_suite);
  RUN_SUITE(http_winhttp_suite);
  RUN_SUITE(http_wininet_suite);
  RUN_SUITE(initializer_parser_suite);
  RUN_SUITE(integration_suite);
  RUN_SUITE(integration_server_suite);
  RUN_SUITE(json_from_and_to_suite);
  RUN_SUITE(numeric_parser_suite);
  RUN_SUITE(openapi_client_gen_suite);
  RUN_SUITE(openapi_loader_suite);
  RUN_SUITE(parsing_suite);
  RUN_SUITE(pragma_suite);
  RUN_SUITE(preprocessor_suite);
  RUN_SUITE(preprocessor_macros_suite);
  RUN_SUITE(project_audit_suite);
  RUN_SUITE(api_sync_suite);
  RUN_SUITE(refactor_orchestrator_suite);
  RUN_SUITE(rewriter_body_suite);
  RUN_SUITE(rewriter_sig_suite);
  RUN_SUITE(schema2tests_suite);
  RUN_SUITE(schema_codegen_suite);
  RUN_SUITE(schema_constraints_suite);
  RUN_SUITE(schema_enum_required_suite);
  RUN_SUITE(simple_mocks_suite);
  RUN_SUITE(str_utils_suite);
  RUN_SUITE(sync_code_suite);
  RUN_SUITE(text_patcher_suite);
  RUN_SUITE(tokenizer_suite);
  RUN_SUITE(tokenizer_trigraphs_suite);
  RUN_SUITE(transport_factory_suite);
  RUN_SUITE(url_utils_suite);

  /* New Runners */
  RUN_SUITE(openapi_writer_suite);
  RUN_SUITE(doc_parser_suite);
  RUN_SUITE(c_mapping_suite);
  RUN_SUITE(c2openapi_op_suite);
  RUN_SUITE(aggregator_suite);
  RUN_SUITE(c2openapi_schema_suite);
  RUN_SUITE(integration_c2openapi_suite);
  RUN_SUITE(to_docs_json_suite);

  GREATEST_MAIN_END();
}
