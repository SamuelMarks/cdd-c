/**
 * @file cdd_cst_scope.c
 * @brief Concrete Syntax Tree (CST) scope implementation and utilities.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_cst_scope.h"
#include "c_cdd_export.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/log.h"
#include "c_cdd/memory.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_scope_alloc_fail;
/** @brief Mock allocation failure counter for CST scope tests */
C_CDD_EXPORT int g_cdd_scope_alloc_fail = 0;
#endif

/**
 * @brief Initializes a new CST lexical scope environment with a root file
 * scope.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_scope_env_init(cdd_cst_scope_env_t **out_env) {
  cdd_cst_scope_env_t *env;
  cdd_cst_scope_t *global;

  if (!out_env)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  env = (cdd_cst_scope_env_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_scope_env_t));
  if (!env) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  global = (cdd_cst_scope_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_scope_t));
  if (!global) {
    C_CDD_FREE(env);
    return CDD_C_ERROR_MEMORY;
  }
  global->kind = CDD_CST_SCOPE_FILE;

  env->global_scope = global;
  env->current_scope = global;

  *out_env = env;
  return CDD_C_SUCCESS;
}

/**
 * @brief Frees a linked list of scope symbols.
 *
 * @param[in] sym Head of the symbol linked list to free.
 */
static void free_symbols(cdd_cst_symbol_t *sym) {
  while (sym) {
    cdd_cst_symbol_t *next = sym->next;
    C_CDD_FREE(sym->name);
    C_CDD_FREE(sym);
    sym = next;
  }
}

/**
 * @brief Recursively frees a lexical scope and all its child scopes and
 * symbols.
 *
 * @param[in] scope Scope to free.
 */
static void free_scope(cdd_cst_scope_t *scope) {
  size_t i;
  if (!scope)
    return;
  free_symbols(scope->symbols);
  for (i = 0; i < scope->num_children; i++) {
    free_scope(scope->children[i]);
  }
  if (scope->children)
    C_CDD_FREE(scope->children);
  C_CDD_FREE(scope);
}

/**
 * @brief Frees a CST lexical scope environment and all scopes within it.
 */
void cdd_cst_scope_env_free(cdd_cst_scope_env_t *env) {
  if (!env)
    return;
  free_scope(env->global_scope);
  C_CDD_FREE(env);
}

/**
 * @brief Enters a new nested lexical scope of the specified kind.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_scope_enter(cdd_cst_scope_env_t *env,
                                  enum cdd_cst_scope_kind_t kind) {
  cdd_cst_scope_t *new_scope;
  cdd_cst_scope_t *parent;

  if (!env || !env->current_scope)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  parent = env->current_scope;

  new_scope = (cdd_cst_scope_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_scope_t));
  if (!new_scope) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  new_scope->kind = kind;
  new_scope->parent = parent;

  if (parent->num_children >= parent->capacity) {
    size_t new_cap = parent->capacity == 0 ? 4 : parent->capacity * 2;
    cdd_cst_scope_t **new_arr;
    new_arr = (cdd_cst_scope_t **)C_CDD_REALLOC(
        parent->children, new_cap * sizeof(cdd_cst_scope_t *));
    if (!new_arr) {
      C_CDD_FREE(new_scope);
      return CDD_C_ERROR_MEMORY;
    }
    parent->children = new_arr;
    parent->capacity = new_cap;
  }

  parent->children[parent->num_children++] = new_scope;
  env->current_scope = new_scope;

  return CDD_C_SUCCESS;
}

/**
 * @brief Leaves the current lexical scope and returns to its parent scope.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if at root.
 */
cdd_c_error_t cdd_cst_scope_leave(cdd_cst_scope_env_t *env) {
  if (!env || !env->current_scope || !env->current_scope->parent)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  env->current_scope = env->current_scope->parent;
  return CDD_C_SUCCESS;
}

/**
 * @brief Duplicates a null-terminated string using `C_CDD_MALLOC`.
 *
 * @param[in] s Source string to duplicate.
 * @param[out] out_s Pointer to receive the duplicated string.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY on OOM.
 */
static cdd_c_error_t cdd_strdup(const char *s, char **out_s) {
  size_t len;
  char *d;
  len = strlen(s);
  d = (char *)C_CDD_MALLOC(len + 1);
  if (!d) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  memcpy(d, s, len + 1);
  *out_s = d;
  return CDD_C_SUCCESS;
}

/**
 * @brief Adds a symbol entry to the current lexical scope.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_scope_add_symbol(cdd_cst_scope_env_t *env,
                                       const char *name,
                                       enum cdd_cst_symbol_kind_t kind,
                                       cdd_cst_node_t *decl_node) {
  cdd_cst_symbol_t *sym;

  if (!env || !env->current_scope || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  sym = (cdd_cst_symbol_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_symbol_t));
  if (!sym) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  if (cdd_strdup(name, &sym->name) != 0)
    sym->name = NULL;
  if (!sym->name) {
    C_CDD_FREE(sym);
    return CDD_C_ERROR_MEMORY;
  }
  sym->kind = kind;
  sym->decl_node = decl_node;

  sym->next = env->current_scope->symbols;
  env->current_scope->symbols = sym;

  return CDD_C_SUCCESS;
}

/**
 * @brief Checks whether a symbol kind belongs to the C tag namespace
 * (`struct`, `union`, or `enum`).
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
cdd_c_error_t cdd_cst_symbol_is_tag(enum cdd_cst_symbol_kind_t kind,
                                    int *out_is_tag) {
  if (!out_is_tag || (int)kind < 0 || kind > CDD_CST_SYMBOL_FUNCTION)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_tag =
      (kind == CDD_CST_SYMBOL_STRUCT_TAG || kind == CDD_CST_SYMBOL_UNION_TAG ||
       kind == CDD_CST_SYMBOL_ENUM_TAG);
  return CDD_C_SUCCESS;
}

/**
 * @brief Looks up a symbol by name and namespace starting from the current
 * scope outward.
 *
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_NOT_FOUND if not found, or
 * error code.
 */
cdd_c_error_t cdd_cst_scope_lookup_symbol(cdd_cst_scope_env_t *env,
                                          const char *name,
                                          enum cdd_cst_symbol_kind_t kind,
                                          cdd_cst_symbol_t **out_symbol) {
  cdd_cst_scope_t *curr;
  int is_tag_lookup = 0;
  cdd_c_error_t rc;

  if (!env || !name || !out_symbol)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = cdd_cst_symbol_is_tag(kind, &is_tag_lookup);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }

  curr = env->current_scope;
  while (curr) {
    cdd_cst_symbol_t *sym = curr->symbols;
    while (sym) {
      if (strcmp(sym->name, name) == 0) {
        /* C has 4 namespaces: tags, labels, members, and ordinary identifiers
         */
        /* For this, we just differentiate between tags and everything else for
         * now */
        int sym_is_tag = 0;
        rc = cdd_cst_symbol_is_tag(sym->kind, &sym_is_tag);
        if (rc != CDD_C_SUCCESS) {
          return rc;
        }
        if (is_tag_lookup == sym_is_tag) {
          *out_symbol = sym;
          return CDD_C_SUCCESS;
        }
      }
      sym = sym->next;
    }
    curr = curr->parent;
  }

  return CDD_C_ERROR_NOT_FOUND;
}
