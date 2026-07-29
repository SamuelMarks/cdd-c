## `src/classes/parse/cdd_cst_builder.c`
- [x] `cdd_cst_bld_block_comment`
  - Line 723: Error variable 'pool_rc' not immediately checked
  - Line 729: Error variable 'rc' not immediately checked
- [x] `get_last_token`
  - Line 771: Enum return value discarded or not assigned in inline condition

## `src/classes/parse/cdd_cst_parser.c`
- [x] `cdd_cst_parse`
  - Line 1319: Enum return value discarded or not assigned in inline condition
- [x] `parse_block`
  - Line 193: Enum return value discarded or not assigned in inline condition
- [x] `parse_declaration_or_statement`
  - Line 446: Enum return value discarded or not assigned in inline condition
  - Line 603: Enum return value discarded or not assigned in inline condition
  - Line 617: Enum return value discarded or not assigned in inline condition
  - Line 631: Enum return value discarded or not assigned in inline condition
  - Line 677: Enum return value discarded or not assigned in inline condition
  - Line 748: Enum return value discarded or not assigned in inline condition
  - Line 800: Enum return value discarded or not assigned in inline condition
  - Line 806: Enum return value discarded or not assigned in inline condition
  - Line 883: Enum return value discarded or not assigned in inline condition
  - Line 950: Enum return value discarded or not assigned in inline condition
  - Line 970: Enum return value discarded or not assigned in inline condition
  - Line 1108: Enum return value discarded or not assigned in inline condition
  - Line 1182: Enum return value discarded or not assigned in inline condition
  - Line 1196: Enum return value discarded or not assigned in inline condition

## `src/classes/parse/cdd_cst_semantic.c`
- [x] `analyze_node`
  - Line 94: Error variable 'rc' not returned
  - Line 113: Error variable 'rc' not returned

## `src/classes/parse/cdd_cst_type_eval.c`
- [x] `cdd_cst_eval_alignof`
  - Line 201: Error variable 'rc' mutated before return
  - Line 201: Error variable 'rc' mutated before return
  - Line 201: Error variable 'rc' not returned
  - Line 204: Error variable 'rc' not immediately checked
  - Line 206: Error variable 'rc' not immediately checked
- [x] `cdd_cst_eval_sizeof`
  - Line 169: Error variable 'rc' mutated before return
  - Line 169: Error variable 'rc' mutated before return
  - Line 169: Error variable 'rc' not returned
  - Line 172: Error variable 'rc' not immediately checked
  - Line 174: Error variable 'rc' not immediately checked

## `src/classes/parse/cdd_lexer.c`
- [x] `cdd_lexer_tokenize`
  - Line 221: Error variable 'rc' not returned
  - Line 263: Enum return value discarded or not assigned
  - Line 297: Error variable 'rc' not returned

## `src/functions/ffi/cdd_ffi_ir_extractor.c`
- [x] `cdd_ffi_ir_extract_exports`
  - Line 1023: Error variable 'rc' mutated before return
  - Line 1023: Error variable 'rc' not returned
- [x] `include_visitor`
  - Line 883: Error variable 'ext_rc' not immediately checked

## `src/gen_group_ns_test.c`
- [x] `Foo_Pet_api_test_op`
  - Line 200: Error variable 'api_rc' not returned

## `src/gen_ns_only_test.c`
- [x] `Bar_api_test_op`
  - Line 200: Error variable 'api_rc' not returned

## `src/gen_op_params.c`
- [x] `api_test_op`
  - Line 224: Error variable 'api_rc' not returned

## `src/gen_path_override.c`
- [x] `api_test_op`
  - Line 200: Error variable 'api_rc' not returned

## `src/gen_path_params.c`
- [x] `api_test_op`
  - Line 206: Error variable 'api_rc' not returned

## `src/gen_querystring_param.c`
- [x] `api_test_op`
  - Line 227: Error variable 'api_rc' not returned

## `src/routes/parse/cli_cst.c`
- [x] `cli_cst_transformer_main`
  - Line 179: Error variable 'p_rc' not returned

## `src/tests/cdd_test_helpers/mock_server.c`
- [x] `mock_server_destroy`
  - Line 281: Error variable 'rc' not returned

## `src/tests/mocks/emit/simple_json.c`
- [x] `FooE_from_jsonObject`
  - Line 617: Enum return value discarded or not assigned
- [x] `HazE_to_json`
  - Line 330: Error variable 'tank_rc' not returned
