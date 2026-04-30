/* clang-format off */
#include "cdd_cst_scope.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "c_cdd/log.h"
/* clang-format on */

int cdd_cst_scope_env_init(cdd_cst_scope_env_t **out_env) {
  cdd_cst_scope_env_t *env;
  cdd_cst_scope_t *global;

  if (!out_env)
    return EINVAL;

  env = (cdd_cst_scope_env_t *)calloc(1, sizeof(cdd_cst_scope_env_t));
  if (!env) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM in %s\n", __func__);
    return ENOMEM;
  }

  global = (cdd_cst_scope_t *)calloc(1, sizeof(cdd_cst_scope_t));
  if (!global) {
    free(env);
    return ENOMEM;
  }
  global->kind = CDD_CST_SCOPE_FILE;

  env->global_scope = global;
  env->current_scope = global;

  *out_env = env;
  return 0;
}

static void free_symbols(cdd_cst_symbol_t *sym) {
  while (sym) {
    cdd_cst_symbol_t *next = sym->next;
    if (sym->name)
      free((void *)sym->name);
    free(sym);
    sym = next;
  }
}

static void free_scope(cdd_cst_scope_t *scope) {
  size_t i;
  if (!scope)
    return;
  free_symbols(scope->symbols);
  for (i = 0; i < scope->num_children; i++) {
    free_scope(scope->children[i]);
  }
  if (scope->children)
    free(scope->children);
  free(scope);
}

void cdd_cst_scope_env_free(cdd_cst_scope_env_t *env) {
  if (!env)
    return;
  free_scope(env->global_scope);
  free(env);
}

int cdd_cst_scope_enter(cdd_cst_scope_env_t *env,
                        enum cdd_cst_scope_kind_t kind) {
  cdd_cst_scope_t *new_scope;
  cdd_cst_scope_t *parent;

  if (!env || !env->current_scope)
    return EINVAL;

  parent = env->current_scope;

  new_scope = (cdd_cst_scope_t *)calloc(1, sizeof(cdd_cst_scope_t));
  if (!new_scope) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM in %s\n", __func__);
    return ENOMEM;
  }

  new_scope->kind = kind;
  new_scope->parent = parent;

  if (parent->num_children >= parent->capacity) {
    size_t new_cap = parent->capacity == 0 ? 4 : parent->capacity * 2;
    cdd_cst_scope_t **new_arr = (cdd_cst_scope_t **)realloc(
        parent->children, new_cap * sizeof(cdd_cst_scope_t *));
    if (!new_arr) {
      free(new_scope);
      return ENOMEM;
    }
    parent->children = new_arr;
    parent->capacity = new_cap;
  }

  parent->children[parent->num_children++] = new_scope;
  env->current_scope = new_scope;

  return 0;
}

int cdd_cst_scope_leave(cdd_cst_scope_env_t *env) {
  if (!env || !env->current_scope || !env->current_scope->parent)
    return EINVAL;

  env->current_scope = env->current_scope->parent;
  return 0;
}

static int cdd_strdup(const char *s, char **out_s) {
  size_t len;
  char *d;
  if (!s || !out_s)
    return EINVAL;
  len = strlen(s);
  d = (char *)malloc(len + 1);
  if (!d) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM in %s\n", __func__);
    return ENOMEM;
  }
  memcpy(d, s, len + 1);
  *out_s = d;
  return 0;
}

int cdd_cst_scope_add_symbol(cdd_cst_scope_env_t *env, const char *name,
                             enum cdd_cst_symbol_kind_t kind,
                             cdd_cst_node_t *decl_node) {
  cdd_cst_symbol_t *sym;

  if (!env || !env->current_scope || !name)
    return EINVAL;

  sym = (cdd_cst_symbol_t *)calloc(1, sizeof(cdd_cst_symbol_t));
  if (!sym) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM in %s\n", __func__);
    return ENOMEM;
  }

  if (cdd_strdup(name, (char **)&sym->name) != 0)
    sym->name = NULL;
  if (!sym->name) {
    free(sym);
    return ENOMEM;
  }
  sym->kind = kind;
  sym->decl_node = decl_node;

  sym->next = env->current_scope->symbols;
  env->current_scope->symbols = sym;

  return 0;
}

int cdd_cst_symbol_is_tag(enum cdd_cst_symbol_kind_t kind) {
  return kind == CDD_CST_SYMBOL_STRUCT_TAG ||
         kind == CDD_CST_SYMBOL_UNION_TAG || kind == CDD_CST_SYMBOL_ENUM_TAG;
}

int cdd_cst_scope_lookup_symbol(cdd_cst_scope_env_t *env, const char *name,
                                enum cdd_cst_symbol_kind_t kind,
                                cdd_cst_symbol_t **out_symbol) {
  cdd_cst_scope_t *curr;
  int is_tag_lookup = cdd_cst_symbol_is_tag(kind);

  if (!env || !name || !out_symbol)
    return EINVAL;

  curr = env->current_scope;
  while (curr) {
    cdd_cst_symbol_t *sym = curr->symbols;
    while (sym) {
      if (strcmp(sym->name, name) == 0) {
        /* C has 4 namespaces: tags, labels, members, and ordinary identifiers
         */
        /* For this, we just differentiate between tags and everything else for
         * now */
        int sym_is_tag = cdd_cst_symbol_is_tag(sym->kind);
        if (is_tag_lookup == sym_is_tag) {
          *out_symbol = sym;
          return 0;
        }
      }
      sym = sym->next;
    }
    curr = curr->parent;
  }

  return ENOENT;
}
