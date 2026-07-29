## `src/classes/parse/cdd_cst_builder.c`
- [x] `cdd_cst_bld_block_close`
  - Line 407: Error variable 'rc' not immediately checked
- [x] `cdd_cst_bld_block_comment`
  - Line 663: Enum return value discarded or not assigned
  - Line 698: Error variable 'rc' not immediately checked
- [x] `cdd_cst_bld_block_open`
  - Line 385: Error variable 'rc' not returned
- [x] `cdd_cst_bld_else`
  - Line 297: Error variable 'rc' not immediately checked
- [x] `cdd_cst_bld_endif`
  - Line 311: Error variable 'rc' not immediately checked
- [x] `cdd_cst_bld_extern_c_close`
  - Line 371: Error variable 'rc' not immediately checked
- [x] `cdd_cst_bld_extern_c_open`
  - Line 349: Error variable 'rc' not immediately checked
- [x] `cdd_cst_bld_ifdef`
  - Line 283: Error variable 'rc' not immediately checked
- [x] `cdd_cst_bld_ifndef`
  - Line 260: Error variable 'rc' not immediately checked
- [x] `cdd_cst_bld_include`
  - Line 217: Error variable 'rc' not immediately checked
  - Line 232: Error variable 'rc' not immediately checked
  - Line 237: Error variable 'rc' mutated before return
  - Line 237: Error variable 'rc' not returned
- [x] `cdd_cst_quote`
  - Line 562: Error variable 'rc' not returned
  - Line 570: Error variable 'rc' not immediately checked
  - Line 573: Error variable 'rc' not immediately checked
  - Line 592: Error variable 'rc' not immediately checked
- [x] `cdd_cst_replace_node_preserve_trivia`
  - Line 837: Enum return value discarded or not assigned
- [x] `cdd_cst_transfer_trivia`
  - Line 794: Enum return value discarded or not assigned
  - Line 795: Enum return value discarded or not assigned
  - Line 797: Enum return value discarded or not assigned
  - Line 798: Enum return value discarded or not assigned
- [x] `get_last_token`
  - Line 740: Enum return value discarded or not assigned in inline condition

## `src/classes/parse/cdd_cst_parser.c`
- [x] `cdd_cst_parse`
  - Line 1319: Enum return value discarded or not assigned in inline condition
- [x] `parse_block`
  - Line 193: Enum return value discarded or not assigned in inline condition
- [x] `parse_declaration_or_statement`
  - Line 446: Enum return value discarded or not assigned in inline condition
  - Line 602: Enum return value discarded or not assigned in inline condition
  - Line 616: Enum return value discarded or not assigned in inline condition
  - Line 630: Enum return value discarded or not assigned in inline condition
  - Line 676: Enum return value discarded or not assigned in inline condition
  - Line 747: Enum return value discarded or not assigned in inline condition
  - Line 799: Enum return value discarded or not assigned in inline condition
  - Line 805: Enum return value discarded or not assigned in inline condition
  - Line 882: Enum return value discarded or not assigned in inline condition
  - Line 949: Enum return value discarded or not assigned in inline condition
  - Line 969: Enum return value discarded or not assigned in inline condition
  - Line 1107: Enum return value discarded or not assigned in inline condition
  - Line 1182: Enum return value discarded or not assigned in inline condition
  - Line 1196: Enum return value discarded or not assigned in inline condition

## `src/classes/parse/cdd_cst_semantic.c`
- [x] `analyze_node`
  - Line 94: Error variable 'rc' not returned
  - Line 113: Error variable 'rc' not returned

## `src/classes/parse/cdd_cst_type_eval.c`
- [x] `cdd_cst_eval_alignof`
  - Line 211: Error variable 'rc' not immediately checked
  - Line 213: Error variable 'rc' not immediately checked
- [x] `cdd_cst_eval_sizeof`
  - Line 176: Error variable 'rc' not immediately checked
  - Line 178: Error variable 'rc' not immediately checked

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
