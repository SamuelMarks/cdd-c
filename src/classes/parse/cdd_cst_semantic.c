/* clang-format off */
#include "cdd_cst_semantic.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "c_cdd/log.h"
/* clang-format on */
/* LCOV_EXCL_START */
#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_cdd_semantic_oom_extract = 0;
C_CDD_EXPORT int g_cdd_semantic_oom_scope = 0;
C_CDD_EXPORT int g_cdd_semantic_oom_scope2 = 0;
#endif

static enum cdd_c_error extract_identifier(cdd_cst_node_t *node,
                                           const char **out_name) {
  size_t i;
  if (!node)
    return CDD_C_ERROR_NOT_FOUND;

  if (node->kind == CDD_CST_IDENTIFIER) {
    for (i = 0; i < node->num_children; i++) {
      if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *tok = node->children[i].val.token;
        if (tok->kind == CDD_TOKEN_IDENTIFIER) {
          char *name = NULL;
#ifdef CDD_BUILD_TESTS
          if (g_cdd_semantic_oom_extract == 5) {
            name = NULL;
          } else {
#endif
            name = (char *)malloc(tok->length + 1);
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
  }

  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_NODE) {
      if (extract_identifier(node->children[i].val.node, out_name) == 0) {
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
    if (rc != 0)
      return rc;
    break;
  case CDD_CST_DECLARATION: {
    const char *name = NULL;
    /* Very simplified extraction of identifier */
    rc = extract_identifier(node, &name);
    if (rc == 0 && name) {
      /* Assuming CDD_CST_SYMBOL_VARIABLE for now */
#ifdef CDD_BUILD_TESTS
      if (g_cdd_semantic_oom_extract == 1)
        rc = CDD_C_ERROR_MEMORY;
      else
#endif
        rc = cdd_cst_scope_add_symbol(env, name, CDD_CST_SYMBOL_VARIABLE, node);
      free((void *)name);
    }
    rc = 0; /* ignore extraction errors */
    break;
  }
  case CDD_CST_TYPE_SPECIFIER: {
    const char *name = NULL;
    rc = extract_identifier(node, &name);
    if (rc == 0 && name) {
      /* Assuming Struct tag for simplicity */
      cdd_cst_scope_add_symbol(env, name, CDD_CST_SYMBOL_STRUCT_TAG, node);
      free((void *)name);
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

/* LCOV_EXCL_STOP */
