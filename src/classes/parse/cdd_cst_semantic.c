/* clang-format off */
#include "c_cdd/memory.h"
#include "cdd_cst_semantic.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "c_cdd/log.h"
/* clang-format on */
#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_cdd_semantic_oom_extract = 0;
C_CDD_EXPORT int g_cdd_semantic_oom_scope = 0;
C_CDD_EXPORT int g_cdd_semantic_oom_scope2 = 0;
#endif

static enum cdd_c_error extract_identifier(cdd_cst_node_t *node,
                                           const char **out_name) {
  size_t i;

  if (node && node->kind == CDD_CST_IDENTIFIER) {
    for (i = 0; i < node->num_children; i++) {
      if (node->children[i].kind == CDD_CST_CHILD_TOKEN &&
          node->children[i].val.token->kind == CDD_TOKEN_IDENTIFIER) {
        cdd_token_t *tok = node->children[i].val.token;
        char *name = NULL;
#ifdef CDD_BUILD_TESTS
        if (g_cdd_semantic_oom_extract == 5) {
          name = NULL;
        } else {
#endif
          name = (char *)C_CDD_MALLOC(tok->length + 1);
#ifdef CDD_BUILD_TESTS
        }
#endif
        if (!name) {
          C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
          return CDD_C_ERROR_MEMORY;
        }
        memcpy(name, tok->start, tok->length);
        name[tok->length] = '\0';
        *out_name = name;
        return CDD_C_SUCCESS;
      }
    }
  }

  if (node) {
    for (i = 0; i < node->num_children; i++) {
      if (node->children[i].kind == CDD_CST_CHILD_NODE &&
          extract_identifier(node->children[i].val.node, out_name) == 0) {
        return CDD_C_SUCCESS;
      }
    }
  }

  return CDD_C_ERROR_NOT_FOUND;
}

static enum cdd_c_error analyze_node(cdd_cst_scope_env_t *env,
                                     cdd_cst_node_t *node) {
  int rc = 0;
  size_t i;

  if (!node)
    return CDD_C_SUCCESS;

  switch (node->kind) {
  case CDD_CST_BLOCK:
    rc = cdd_cst_scope_enter(env, CDD_CST_SCOPE_BLOCK);
#ifdef CDD_BUILD_TESTS
    if (g_cdd_semantic_oom_scope)
      rc = CDD_C_ERROR_MEMORY;
#endif
    if (rc != 0)
      return rc;
    break;
  case CDD_CST_FUNCTION_DEFINITION:
    rc = cdd_cst_scope_enter(env, CDD_CST_SCOPE_FUNCTION);
#ifdef CDD_BUILD_TESTS
    if (g_cdd_semantic_oom_scope2)
      rc = CDD_C_ERROR_MEMORY;
#endif
    if (rc != 0)
      return rc;
    break;
  case CDD_CST_NAMESPACE_DECLARATION:
    rc = cdd_cst_scope_enter(env, CDD_CST_SCOPE_NAMESPACE);
#ifdef CDD_BUILD_TESTS
    if (g_cdd_semantic_oom_scope2 == 2) /* borrow oom variable */
      rc = CDD_C_ERROR_MEMORY;
#endif
    if (rc != 0)
      return rc;
    break;
  case CDD_CST_DECLARATION: {
    const char *name = NULL;
    /* Very simplified extraction of identifier */
    {
      enum cdd_c_error ext_rc = extract_identifier(node, &name);
      if (ext_rc != CDD_C_SUCCESS) {
        return ext_rc;
      } else {
        enum cdd_c_error add_rc;
        /* Assuming CDD_CST_SYMBOL_VARIABLE for now */
#ifdef CDD_BUILD_TESTS
        if (g_cdd_semantic_oom_extract == 1)
          add_rc = CDD_C_ERROR_MEMORY;
        else
#endif
          add_rc = cdd_cst_scope_add_symbol(env, name, CDD_CST_SYMBOL_VARIABLE,
                                            node);
        C_CDD_FREE((void *)name);
        if (add_rc != CDD_C_SUCCESS)
          return add_rc;
      }
    }
    rc = 0; /* ignore extraction errors */
    break;
  }
  case CDD_CST_TYPE_SPECIFIER: {
    const char *name = NULL;
    {
      enum cdd_c_error ext_rc = extract_identifier(node, &name);
      if (ext_rc != CDD_C_SUCCESS) {
        return ext_rc;
      } else {
        /* Assuming Struct tag for simplicity */
        enum cdd_c_error add_rc = cdd_cst_scope_add_symbol(
            env, name, CDD_CST_SYMBOL_STRUCT_TAG, node);
        C_CDD_FREE((void *)name);
        if (add_rc != CDD_C_SUCCESS)
          return add_rc;
      }
    }
    rc = 0; /* ignore extraction errors */
    break;
  }
  default:
    break;
  }

  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_NODE) {
      rc = analyze_node(env, node->children[i].val.node);
      if (rc != 0)
        return rc;
    }
  }

  switch (node->kind) {
  case CDD_CST_BLOCK:
  case CDD_CST_FUNCTION_DEFINITION:
  case CDD_CST_NAMESPACE_DECLARATION:
    rc = cdd_cst_scope_leave(env);
#ifdef CDD_BUILD_TESTS
    if (g_cdd_semantic_oom_extract == 2)
      rc = CDD_C_ERROR_MEMORY;
#endif
    if (rc != 0)
      return rc;
    break;
  default:
    break;
  }

  return CDD_C_SUCCESS;
}

enum cdd_c_error cdd_cst_build_semantic_info(cdd_cst_tree_t *tree,
                                             cdd_cst_scope_env_t **out_env) {
  int rc;
  cdd_cst_scope_env_t *env = NULL;

  if (!tree || !out_env)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = cdd_cst_scope_env_init(&env);
#ifdef CDD_BUILD_TESTS
  if (g_cdd_semantic_oom_extract == 3)
    rc = CDD_C_ERROR_MEMORY;
#endif
  if (rc != 0) {
    if (env)
      cdd_cst_scope_env_free(env);
    return rc;
  }

  rc = analyze_node(env, tree->root);
#ifdef CDD_BUILD_TESTS
  if (g_cdd_semantic_oom_extract == 4)
    rc = CDD_C_ERROR_MEMORY;
#endif
  if (rc != 0) {
    cdd_cst_scope_env_free(env);
    return rc;
  }

  *out_env = env;
  return CDD_C_SUCCESS;
}
