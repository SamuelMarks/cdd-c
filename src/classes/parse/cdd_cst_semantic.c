/* clang-format off */
#include "cdd_cst_semantic.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
/* clang-format on */

static int extract_identifier(cdd_cst_node_t *node, const char **out_name) {
  size_t i;
  if (!node)
    return ENOENT;

  if (node->kind == CDD_CST_IDENTIFIER) {
    for (i = 0; i < node->num_children; i++) {
      if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *tok = node->children[i].val.token;
        if (tok->kind == CDD_TOKEN_IDENTIFIER) {
          char *name = (char *)malloc(tok->length + 1);
          if (!name)
            return ENOMEM;
          memcpy(name, tok->start, tok->length);
          name[tok->length] = '\0';
          *out_name = name;
          return 0;
        }
      }
    }
  }

  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_NODE) {
      if (extract_identifier(node->children[i].val.node, out_name) == 0) {
        return 0;
      }
    }
  }

  return ENOENT;
}

static int analyze_node(cdd_cst_scope_env_t *env, cdd_cst_node_t *node) {
  int rc = 0;
  size_t i;

  if (!node)
    return 0;

  switch (node->kind) {
  case CDD_CST_BLOCK:
    rc = cdd_cst_scope_enter(env, CDD_CST_SCOPE_BLOCK);
    if (rc != 0)
      return rc;
    break;
  case CDD_CST_FUNCTION_DEFINITION:
    rc = cdd_cst_scope_enter(env, CDD_CST_SCOPE_FUNCTION);
    if (rc != 0)
      return rc;
    break;
  case CDD_CST_DECLARATION: {
    const char *name = NULL;
    /* Very simplified extraction of identifier */
    rc = extract_identifier(node, &name);
    if (rc == 0 && name) {
      /* Assuming CDD_CST_SYMBOL_VARIABLE for now */
      cdd_cst_scope_add_symbol(env, name, CDD_CST_SYMBOL_VARIABLE, node);
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
    rc = cdd_cst_scope_leave(env);
    if (rc != 0)
      return rc;
    break;
  default:
    break;
  }

  return 0;
}

int cdd_cst_build_semantic_info(cdd_cst_tree_t *tree,
                                cdd_cst_scope_env_t **out_env) {
  int rc;
  cdd_cst_scope_env_t *env;

  if (!tree || !out_env)
    return EINVAL;

  rc = cdd_cst_scope_env_init(&env);
  if (rc != 0)
    return rc;

  rc = analyze_node(env, tree->root);
  if (rc != 0) {
    cdd_cst_scope_env_free(env);
    return rc;
  }

  *out_env = env;
  return 0;
}
