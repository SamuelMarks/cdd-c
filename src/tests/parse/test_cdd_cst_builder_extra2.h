/* clang-format off */
#include "c_cdd_export.h"
/* clang-format on */
extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
TEST test_cdd_cst_builder_all_errors(void) {
  cdd_cst_builder_t b;
  b.error_state = CDD_C_ERROR_INVALID_ARGUMENT;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_bld_token(NULL, CDD_TOKEN_IDENTIFIER, "test"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_space(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_space(&b));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_newline(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_newline(&b));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_indent(NULL, 1));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_indent(&b, 1));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_ident(NULL, "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_ident(&b, "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_string(NULL, "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_string(&b, "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_int(NULL, 1));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_int(&b, 1));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_punct(NULL, "{"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_punct(&b, "{"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_include(NULL, "a", 0));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_include(&b, "a", 0));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_ifndef(NULL, "A"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_ifndef(&b, "A"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_define(NULL, "A", "B"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_define(&b, "A", "B"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_endif(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_endif(&b));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_pragma_once(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_pragma_once(&b));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_ifdef_cplusplus(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_ifdef_cplusplus(&b));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_endif_cplusplus(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_endif_cplusplus(&b));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_struct_decl(NULL, "A"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_struct_decl(&b, "A"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_enum_decl(NULL, "A"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_enum_decl(&b, "A"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_typedef(NULL, "A", "B"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_typedef(&b, "A", "B"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_bld_keyword(NULL, CDD_TOKEN_KEYWORD_INT));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_bld_keyword(&b, CDD_TOKEN_KEYWORD_INT));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_bld_decl_start(NULL, "int", "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_bld_decl_start(&b, "int", "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_decl_end(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_decl_end(&b));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_bld_func_decl_start(NULL, "int", "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_bld_func_decl_start(&b, "int", "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_func_decl_end(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_func_decl_end(&b));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_comment(NULL, "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_comment(&b, "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_macro(NULL, "A"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_macro(&b, "A"));

  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *root = NULL;
  cdd_cst_builder_t valid_b;
  tree = (cdd_cst_tree_t *)calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  cdd_cst_builder_init(&valid_b, tree, root);

  /* punct edge cases */
  cdd_cst_bld_punct(&valid_b, "[");
  cdd_cst_bld_punct(&valid_b, "]");
  cdd_cst_bld_punct(&valid_b, ".");
  cdd_cst_bld_punct(&valid_b, "->");
  cdd_cst_bld_punct(&valid_b, "+");
  cdd_cst_bld_punct(&valid_b, "-");
  cdd_cst_bld_punct(&valid_b, "*");
  cdd_cst_bld_punct(&valid_b, "/");
  cdd_cst_bld_punct(&valid_b, "=");
  cdd_cst_bld_punct(&valid_b, "==");
  cdd_cst_bld_punct(&valid_b, "!=");
  cdd_cst_bld_punct(&valid_b, "!");
  cdd_cst_bld_punct(&valid_b, "~");
  cdd_cst_bld_punct(&valid_b, "^");
  cdd_cst_bld_punct(&valid_b, "&&");
  cdd_cst_bld_punct(&valid_b, "||");
  cdd_cst_bld_punct(&valid_b, "?");
  cdd_cst_bld_punct(&valid_b, ":");

  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;

  PASS();
}
