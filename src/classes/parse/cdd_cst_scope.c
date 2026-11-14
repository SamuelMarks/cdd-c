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

/*
 * @brief Initializes a new CST lexical scope environment with a root file
 * scope.
 *
 * @param[out] out_env Output pointer receiving the newly allocated scope
 * environment.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_scope_env_init(cdd_cst_scope_env_t **out_env) {
  cdd_cst_scope_env_t *env;
  cdd_cst_scope_t *global;

  if (!out_env)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  env = (cdd_cst_scope_env_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_scope_env_t));
  if (!env) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM");
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
 * @brief Frees a linked list of struct/union member layout entries.
 *
 * @param[in] field Head of the field info linked list to free.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t free_fields(cdd_cst_field_info_t *field) {
  while (field) {
    cdd_cst_field_info_t *next = field->next;
    C_CDD_FREE(field->name);
    C_CDD_FREE(field->type_str);
    C_CDD_FREE(field);
    field = next;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Frees a linked list of scope symbols.
 *
 * @param[in] sym Head of the symbol linked list to free.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t free_symbols(cdd_cst_symbol_t *sym) {
  cdd_c_error_t rc = CDD_C_SUCCESS;
  while (sym) {
    cdd_cst_symbol_t *next = sym->next;
    rc = (cdd_c_error_t)((int)rc | (int)free_fields(sym->fields));
    C_CDD_FREE(sym->type_str);
    C_CDD_FREE(sym->name);
    C_CDD_FREE(sym);
    sym = next;
  }
  return rc;
}

/**
 * @brief Recursively frees a lexical scope and all its child scopes and
 * symbols.
 *
 * @param[in] scope Scope to free.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t free_scope(cdd_cst_scope_t *scope) {
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (!scope)
    return CDD_C_SUCCESS;
  rc = (cdd_c_error_t)((int)rc | (int)free_symbols(scope->symbols));
  for (i = 0; i < scope->num_children; i++) {
    rc = (cdd_c_error_t)((int)rc | (int)free_scope(scope->children[i]));
  }
  if (scope->children)
    C_CDD_FREE(scope->children);
  C_CDD_FREE(scope);
  return rc;
}

/*
 * @brief Frees a CST lexical scope environment and all scopes within it.
 *
 * @param[in,out] env Pointer to the scope environment to free.
 * @return CDD_C_SUCCESS.
 */
cdd_c_error_t cdd_cst_scope_env_free(cdd_cst_scope_env_t *env) {
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (!env)
    return CDD_C_SUCCESS;
  if (env->bound_tree && env->bound_tree->active_sem_env == env) {
    env->bound_tree->active_sem_env = NULL;
  }
  rc = (cdd_c_error_t)((int)rc | (int)free_scope(env->global_scope));
  C_CDD_FREE(env);
  return rc;
}

/*
 * @brief Enters a new nested lexical scope of the specified kind.
 *
 * @param[in,out] env Active scope environment.
 * @param[in] kind Kind of nested lexical scope to enter.
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
    C_CDD_LOG_DEBUG("ENOMEM: OOM");
    return CDD_C_ERROR_MEMORY;
  }

  new_scope->kind = kind;
  new_scope->parent = parent;

  if (parent->num_children >= parent->capacity) {
    size_t new_cap = parent->capacity == 0 ? 4 : parent->capacity * 2;
    cdd_cst_scope_t **new_arr = (cdd_cst_scope_t **)C_CDD_REALLOC(
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

/*
 * @brief Leaves the current lexical scope and returns to its parent scope.
 *
 * @param[in,out] env Active scope environment.
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
    C_CDD_LOG_DEBUG("ENOMEM: OOM");
    return CDD_C_ERROR_MEMORY;
  }
  memcpy(d, s, len + 1);
  *out_s = d;
  return CDD_C_SUCCESS;
}

/*
 * @brief Adds a symbol entry to the current lexical scope.
 *
 * @param[in,out] env Active scope environment.
 * @param[in] name Symbol identifier name.
 * @param[in] kind Symbol namespace/kind.
 * @param[in] decl_node CST declaration node associated with the symbol.
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
    C_CDD_LOG_DEBUG("ENOMEM: OOM");
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

/*
 * @brief Checks whether a symbol kind belongs to the C tag namespace
 * (`struct`, `union`, or `enum`).
 *
 * @param[in] kind Symbol kind to test.
 * @param[out] out_is_tag Output pointer receiving 1 if tag symbol, 0 otherwise.
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

/*
 * @brief Attaches a canonical C type string to a symbol entry.
 *
 * @param[in,out] sym Target symbol entry.
 * @param[in] type_str Null-terminated C type string.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_symbol_set_type(cdd_cst_symbol_t *sym,
                                      const char *type_str) {
  char *dup = NULL;
  cdd_c_error_t rc;
  if (!sym || !type_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_strdup(type_str, &dup);
  if (rc != CDD_C_SUCCESS)
    return rc;
  C_CDD_FREE(sym->type_str);
  sym->type_str = dup;
  return CDD_C_SUCCESS;
}

/*
 * @brief Appends a struct or union member layout entry to a tag symbol.
 *
 * @param[in,out] tag_sym Target struct or union tag symbol.
 * @param[in] field_name Member identifier (empty string if unnamed).
 * @param[in] type_str Canonical C type string of the member.
 * @param[in] array_len Constant array element count (0 if scalar/pointer).
 * @param[in] is_bitfield Non-zero if declared as a bit-field.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_symbol_add_field(cdd_cst_symbol_t *tag_sym,
                                       const char *field_name,
                                       const char *type_str, size_t array_len,
                                       int is_bitfield) {
  cdd_cst_field_info_t *fi;
  cdd_cst_field_info_t *tail;
  cdd_c_error_t rc;

  if (!tag_sym || !field_name || !type_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  fi = (cdd_cst_field_info_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_field_info_t));
  if (!fi)
    return CDD_C_ERROR_MEMORY;

  rc = cdd_strdup(field_name, &fi->name);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(fi);
    return rc;
  }
  rc = cdd_strdup(type_str, &fi->type_str);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(fi->name);
    C_CDD_FREE(fi);
    return rc;
  }
  fi->array_len = array_len;
  fi->is_bitfield = is_bitfield;

  if (!tag_sym->fields) {
    tag_sym->fields = fi;
  } else {
    tail = tag_sym->fields;
    while (tail->next) {
      tail = tail->next;
    }
    tail->next = fi;
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Looks up a symbol starting from an explicit lexical scope outward.
 *
 * @param[in] scope Starting lexical scope.
 * @param[in] name Symbol identifier name to look up.
 * @param[in] kind Symbol namespace/kind to match.
 * @param[out] out_symbol Output pointer receiving the resolved symbol entry.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_NOT_FOUND if not found, or
 * error code.
 */
cdd_c_error_t cdd_cst_scope_lookup_in_scope(const cdd_cst_scope_t *scope,
                                            const char *name,
                                            enum cdd_cst_symbol_kind_t kind,
                                            cdd_cst_symbol_t **out_symbol) {
  const cdd_cst_scope_t *curr;
  int is_tag_lookup = 0;
  cdd_c_error_t rc;

  if (!name || !out_symbol)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = cdd_cst_symbol_is_tag(kind, &is_tag_lookup);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }

  curr = scope;
  while (curr) {
    cdd_cst_symbol_t *sym = curr->symbols;
    while (sym) {
      if (strcmp(sym->name, name) == 0) {
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

/*
 * @brief Looks up a symbol by name and namespace starting from the current
 * scope outward.
 *
 * @param[in] env Active scope environment.
 * @param[in] name Symbol identifier name to look up.
 * @param[in] kind Symbol namespace/kind to match.
 * @param[out] out_symbol Output pointer receiving the resolved symbol entry.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_NOT_FOUND if not found, or
 * error code.
 */
cdd_c_error_t cdd_cst_scope_lookup_symbol(cdd_cst_scope_env_t *env,
                                          const char *name,
                                          enum cdd_cst_symbol_kind_t kind,
                                          cdd_cst_symbol_t **out_symbol) {
  if (!env)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  return cdd_cst_scope_lookup_in_scope(env->current_scope, name, kind,
                                       out_symbol);
}

/**
 * @brief Recursively searches a scope subtree for a scope bound to `target`.
 *
 * @param[in] scope Current non-null scope subtree root.
 * @param[in] target CST node to match against `scope->ast_node`.
 * @param[out] out_scope Output pointer receiving the matching scope if found.
 * @return CDD_C_SUCCESS if found, or CDD_C_ERROR_NOT_FOUND.
 */
static cdd_c_error_t find_scope_by_ast_node(cdd_cst_scope_t *scope,
                                            const cdd_cst_node_t *target,
                                            cdd_cst_scope_t **out_scope) {
  size_t i;
  if (scope->ast_node == target) {
    *out_scope = scope;
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < scope->num_children; i++) {
    if (find_scope_by_ast_node(scope->children[i], target, out_scope) ==
        CDD_C_SUCCESS) {
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_ERROR_NOT_FOUND;
}

/*
 * @brief Finds the innermost lexical scope associated with a CST node or its
 * enclosing block/function.
 *
 * @param[in] env Active scope environment.
 * @param[in] node CST node to locate scope for (NULL returns global scope).
 * @param[out] out_scope Output pointer receiving the resolved scope.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_scope_find_for_node(const cdd_cst_scope_env_t *env,
                                          const cdd_cst_node_t *node,
                                          cdd_cst_scope_t **out_scope) {
  const cdd_cst_node_t *cur;
  if (!env || !env->global_scope || !out_scope)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  cur = node;
  while (cur) {
    cdd_cst_scope_t *found = NULL;
    if (find_scope_by_ast_node(env->global_scope, cur, &found) ==
        CDD_C_SUCCESS) {
      *out_scope = found;
      return CDD_C_SUCCESS;
    }
    cur = cur->parent;
  }

  *out_scope = env->global_scope;
  return CDD_C_SUCCESS;
}

/**
 * @brief Recursively updates `ast_node` pointers matching `old_node` in
 * `scope`.
 *
 * @param[in,out] scope Non-null scope subtree root.
 * @param[in] old_node Previous CST node pointer.
 * @param[in] new_node Replacement CST node pointer.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t remap_scope_ast_node_rec(cdd_cst_scope_t *scope,
                                              const cdd_cst_node_t *old_node,
                                              cdd_cst_node_t *new_node) {
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (scope->ast_node == old_node) {
    scope->ast_node = new_node;
  }
  for (i = 0; i < scope->num_children; i++) {
    rc = (cdd_c_error_t)((int)rc | (int)remap_scope_ast_node_rec(
                                       scope->children[i], old_node, new_node));
  }
  return rc;
}

/*
 * @brief Updates any `scope->ast_node` pointers matching `old_node` to
 * `new_node` when a CST node is spliced/replaced.
 */
cdd_c_error_t cdd_cst_scope_remap_ast_node(cdd_cst_scope_env_t *env,
                                           const cdd_cst_node_t *old_node,
                                           cdd_cst_node_t *new_node) {
  if (!env || !env->global_scope || !old_node)
    return CDD_C_SUCCESS;
  return remap_scope_ast_node_rec(env->global_scope, old_node, new_node);
}
