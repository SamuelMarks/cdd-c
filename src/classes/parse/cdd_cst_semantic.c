/* clang-format off */
#include "cdd_cst_semantic.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "c_cdd/memory.h"
#include "c_cdd/log.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_cdd_scope_enter_fails = 0;
C_CDD_EXPORT int g_cdd_scope_leave_fails = 0;
C_CDD_EXPORT int g_cdd_scope_add_fails = 0;
#endif

static cdd_c_error_t extract_identifier(cdd_cst_node_t *node,
                                        const char **out_name) {
  size_t i;
  cdd_c_error_t rc;

  if (node->kind == CDD_CST_IDENTIFIER) {
    for (i = 0; i < node->num_children; i++) {
      if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *tok = node->children[i].val.token;
        if (tok->kind == CDD_TOKEN_IDENTIFIER) {
          char *name;
          name = (char *)C_CDD_MALLOC(tok->length + 1);
          if (!name) {
            return CDD_C_ERROR_MEMORY;
          }
          memcpy(name, tok->start, tok->length);
          name[tok->length] = '\0';
          *out_name = name;
          return CDD_C_SUCCESS;
        }
      }
    }
    return CDD_C_ERROR_NOT_FOUND; /* Loop exhausted */
  }
  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_NODE) {
      rc = extract_identifier(node->children[i].val.node, out_name);
      if (rc == CDD_C_SUCCESS) {
        return CDD_C_SUCCESS;
      }
      if (rc != CDD_C_ERROR_NOT_FOUND) {
        return rc;
      }
    }
  }

  return CDD_C_ERROR_NOT_FOUND;
}

static cdd_c_error_t analyze_node(cdd_cst_scope_env_t *env,
                                  cdd_cst_node_t *node) {
  cdd_c_error_t rc = CDD_C_SUCCESS;
  size_t i;

  if (!node)
    return CDD_C_SUCCESS;

  switch (node->kind) {
  case CDD_CST_BLOCK:
#ifdef CDD_BUILD_TESTS
    if (g_cdd_scope_enter_fails)
      return CDD_C_ERROR_MEMORY;
#endif
    rc = cdd_cst_scope_enter(env, CDD_CST_SCOPE_BLOCK);
    if (rc != CDD_C_SUCCESS)
      return rc;
    break;
  case CDD_CST_FUNCTION_DEFINITION:
#ifdef CDD_BUILD_TESTS
    if (g_cdd_scope_enter_fails)
      return CDD_C_ERROR_MEMORY;
#endif
    rc = cdd_cst_scope_enter(env, CDD_CST_SCOPE_FUNCTION);
    if (rc != CDD_C_SUCCESS)
      return rc;
    break;
  case CDD_CST_NAMESPACE_DECLARATION:
#ifdef CDD_BUILD_TESTS
    if (g_cdd_scope_enter_fails)
      return CDD_C_ERROR_MEMORY;
#endif
    rc = cdd_cst_scope_enter(env, CDD_CST_SCOPE_NAMESPACE);
    if (rc != CDD_C_SUCCESS)
      return rc;
    break;
  case CDD_CST_DECLARATION: {
    const char *name = NULL;
    /* Very simplified extraction of identifier */
    rc = extract_identifier(node, &name);
    if (rc == CDD_C_ERROR_MEMORY)
      return rc;
    if (rc == CDD_C_SUCCESS && name) {
      /* Assuming CDD_CST_SYMBOL_VARIABLE for now */
#ifdef CDD_BUILD_TESTS
      if (g_cdd_scope_add_fails) {
        C_CDD_FREE((void *)name);
        return CDD_C_ERROR_MEMORY;
      }
#endif
      rc = cdd_cst_scope_add_symbol(env, name, CDD_CST_SYMBOL_VARIABLE, node);
      C_CDD_FREE((void *)name);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    break;
  }
  case CDD_CST_TYPE_SPECIFIER: {
    const char *name = NULL;
    rc = extract_identifier(node, &name);
    if (rc == CDD_C_ERROR_MEMORY)
      return rc;
    if (rc == CDD_C_SUCCESS && name) {
      /* Assuming Struct tag for simplicity */
#ifdef CDD_BUILD_TESTS
      if (g_cdd_scope_add_fails) {
        C_CDD_FREE((void *)name);
        return CDD_C_ERROR_MEMORY;
      }
#endif
      rc = cdd_cst_scope_add_symbol(env, name, CDD_CST_SYMBOL_STRUCT_TAG, node);
      C_CDD_FREE((void *)name);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    break;
  }
  default:
    break;
  }

  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_NODE) {
      rc = analyze_node(env, node->children[i].val.node);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  switch (node->kind) {
  case CDD_CST_BLOCK:
  case CDD_CST_FUNCTION_DEFINITION:
  case CDD_CST_NAMESPACE_DECLARATION:
#ifdef CDD_BUILD_TESTS
    if (g_cdd_scope_leave_fails)
      return CDD_C_ERROR_MEMORY;
#endif
    rc = cdd_cst_scope_leave(env);
    if (rc != CDD_C_SUCCESS)
      return rc;
    break;
  default:
    break;
  }

  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_cst_build_semantic_info(cdd_cst_tree_t *tree,
                                          cdd_cst_scope_env_t **out_env) {
  cdd_cst_scope_env_t *env = NULL;
  cdd_c_error_t rc;

  if (!tree || !out_env)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = cdd_cst_scope_env_init(&env);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = analyze_node(env, tree->root);
  if (rc != CDD_C_SUCCESS) {
    cdd_cst_scope_env_free(env);
    return rc;
  }

  *out_env = env;
  return CDD_C_SUCCESS;
}
