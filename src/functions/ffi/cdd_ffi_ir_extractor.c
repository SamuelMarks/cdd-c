/**
 * @file cdd_ffi_ir_extractor.c
 * @brief Core cdd ffi ir extractor definitions and implementation.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_ir_extractor.h"
#include "functions/ffi/cdd_ffi_ir_internal.h"
#include "../../classes/parse/cdd_cst_parser.h"
#include "../../classes/parse/cdd_cst_query.h"
#include "../../classes/parse/inspector.h"
#include "../../functions/parse/fs.h"
#include "../../functions/parse/macro_evaluator.h"
#include "../../functions/parse/preprocessor.h"
#include "c_cdd/format_specifiers.h"
#include "c_cdd/memory.h"
#include "c_cdd_export.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Preserves the first non-success error code across two sequential
 * calls.
 *
 * @param[in] first_rc Primary error code.
 * @param[in] second_rc Secondary/cleanup error code.
 * @return `first_rc` if non-success, otherwise `second_rc`.
 */
static NO_DISCARD cdd_c_error_t cdd_keep_err(cdd_c_error_t first_rc,
                                             cdd_c_error_t second_rc) {
  return (first_rc != CDD_C_SUCCESS) ? first_rc : second_rc;
}

/** @brief Counter to simulate allocation failure in FFI IR extractor tests. */
C_CDD_EXPORT int g_ffi_extractor_alloc_fail = 0;

#ifdef CDD_BUILD_TESTS
/** @brief Flag to simulate class name lookup failure in tests. */
C_CDD_EXPORT int g_cdd_ffi_fail_class_name = 0;
/** @brief Flag to simulate `is_visited` failure in tests. */
C_CDD_EXPORT int g_cdd_ffi_fail_is_visited = 0;
/** @brief Selector to simulate internal extractor step failures in tests. */
C_CDD_EXPORT int g_cdd_ffi_extractor_fail = 0;
/** @brief Flag to simulate CST node lookup failure in tests. */
C_CDD_EXPORT int g_cdd_ffi_fail_cst_node = 0;
/** @brief Flag to simulate CST token lookup failure in tests. */
C_CDD_EXPORT int g_cdd_ffi_fail_token = 0;
#endif

/**
 * @brief Converts a 64-bit signed integer to a string.
 *
 * @param[in] val Value to convert.
 * @param[in,out] buf Output buffer.
 * @param[in] buf_sz Size of output buffer.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
static cdd_c_error_t int64_to_str(int64_t val, char *buf, size_t buf_sz) {
  char temp[64];
  size_t i = 0;
  size_t j = 0;
  int neg = 0;
  uint64_t uval;
  if (buf == NULL || buf_sz == 0) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  if (val < 0) {
    neg = 1;
    uval = (uint64_t) - (val + 1) + 1;
  } else {
    uval = (uint64_t)val;
  }
  if (uval == 0) {
    temp[i++] = '0';
  } else {
    while (uval > 0) {
      temp[i++] = (char)('0' + (uval % 10));
      uval /= 10;
    }
  }
  if (neg) {
    temp[i++] = '-';
  }
  while (i > 0 && j < buf_sz - 1) {
    buf[j++] = temp[--i];
  }
  buf[j] = '\0';
  return CDD_C_SUCCESS;
}

/**
 * @brief Adds a new node to the FFI IR.
 *
 * @param[in,out] ir Pointer to FFI IR structure.
 * @param[in] kind Kind of node to add.
 * @param[in] name Name of the node.
 * @param[out] out_node Optional pointer to receive the created node.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
C_CDD_EXPORT cdd_c_error_t ir_add_node(cdd_ffi_ir_t *ir,
                                       cdd_ffi_node_kind_t kind,
                                       const char *name,
                                       cdd_ffi_ir_node_t **out_node) {
  cdd_ffi_ir_node_t *node;

  if (!ir || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (ir->nodes_count >= ir->nodes_capacity) {
    size_t new_cap = ir->nodes_capacity == 0 ? 16 : ir->nodes_capacity * 2;
    cdd_ffi_ir_node_t *new_nodes = (cdd_ffi_ir_node_t *)CDD_REALLOC(
        ir->nodes, new_cap * sizeof(cdd_ffi_ir_node_t));
    if (!new_nodes)
      return CDD_C_ERROR_MEMORY;
    ir->nodes = new_nodes;
    ir->nodes_capacity = new_cap;
  }

  node = &ir->nodes[ir->nodes_count];
  memset(node, 0, sizeof(cdd_ffi_ir_node_t));
  node->kind = kind;
  node->name = CDD_STRDUP(name);
  if (!node->name)
    return CDD_C_ERROR_MEMORY;
  ir->nodes_count++;

  if (out_node) {
    *out_node = node;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Context for searching a CST node by name.
 */
struct find_node_ctx_t {
  const char *target_name;   /**< Target identifier name to match */
  cdd_cst_node_t **out_node; /**< Output pointer to receive the matched node */
};

/**
 * @brief Visitor to find a class declaration by name.
 * @param[in,out] node The current node.
 * @param[in,out] user_data Pointer to user data.
 * @return 1 to stop traversal, 0 otherwise.
 */
static cdd_c_error_t find_class_visitor(cdd_cst_node_t *node, void *user_data) {
  struct find_node_ctx_t *ctx = (struct find_node_ctx_t *)user_data;
  const char *target_name = ctx->target_name;
  cdd_cst_node_t **out_node = ctx->out_node;

  if (!*out_node && node->kind == CDD_CST_CLASS_DECLARATION) {
    size_t i;
    for (i = 0; i < node->num_children; i++) {
      if (node->children[i].val.token->kind == CDD_TOKEN_IDENTIFIER) {
        cdd_token_t *tok = node->children[i].val.token;
        if (tok->length == strlen(target_name) &&
            strncmp((const char *)tok->start, target_name, tok->length) == 0) {
          *out_node = node;
          break;
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Finds a class declaration in a CST tree by its name.
 * @param[in] tree The CST tree.
 * @param[in] name The name to search for.
 * @param[out] out_node The matching node, or NULL.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t find_class_declaration_by_name(cdd_cst_tree_t *tree,
                                                    const char *name,
                                                    cdd_cst_node_t **out_node) {
  struct find_node_ctx_t ctx;
  *out_node = NULL;
  ctx.target_name = name;
  ctx.out_node = out_node;
  return cdd_cst_traverse_preorder(tree->root, find_class_visitor, &ctx);
}

/**
 * @brief Visitor to find a function definition by name.
 * @param[in,out] node The current node.
 * @param[in,out] user_data Pointer to user data.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t find_func_visitor(cdd_cst_node_t *node, void *user_data) {
  struct find_node_ctx_t *ctx = (struct find_node_ctx_t *)user_data;
  const char *target_name = ctx->target_name;
  cdd_cst_node_t **out_node = ctx->out_node;

  if (!*out_node && node->kind == CDD_CST_FUNCTION_DEFINITION) {
    size_t i;
    for (i = 0; i < node->num_children; i++) {
      if (node->children[i].val.token->kind == CDD_TOKEN_IDENTIFIER) {
        cdd_token_t *tok = node->children[i].val.token;
        if (tok->length == strlen(target_name) &&
            strncmp((const char *)tok->start, target_name, tok->length) == 0) {
          *out_node = node;
          break;
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Finds a function definition in a CST tree by its name.
 * @param[in] tree The CST tree.
 * @param[in] name The name to search for.
 * @param[out] out_node The matching node, or NULL.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t find_func_definition_by_name(cdd_cst_tree_t *tree,
                                                  const char *name,
                                                  cdd_cst_node_t **out_node) {
  struct find_node_ctx_t ctx;
  *out_node = NULL;
  ctx.target_name = name;
  ctx.out_node = out_node;
  return cdd_cst_traverse_preorder(tree->root, find_func_visitor, &ctx);
}

/**
 * @brief Gets the first token of a node.
 * @param[in] node The node.
 * @param[out] out_tok The first token, or NULL.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t get_first_token(cdd_cst_node_t *node,
                                     cdd_token_t **out_tok) {
  *out_tok = node->children[0].val.token;
  return CDD_C_SUCCESS;
}

/**
 * @brief Gets the last token of a node.
 * @param[in] node The node.
 * @param[out] out_tok The last token, or NULL.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t get_last_token(cdd_cst_node_t *node,
                                    cdd_token_t **out_tok) {
  for (;;) {
    size_t last_idx = node->num_children - 1;
    if (node->children[last_idx].kind == CDD_CST_CHILD_TOKEN) {
      *out_tok = node->children[last_idx].val.token;
      return CDD_C_SUCCESS;
    }
    node = node->children[last_idx].val.node;
  }
}
/**
 * @brief Copies a CST trivia chain to an FFI trivia chain.
 * @param[in] in_trivia The input trivia chain.
 * @param[out] out_trivia Output parameter for the copied chain.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
static cdd_c_error_t copy_trivia_chain(const cdd_trivia_t *in_trivia,
                                       cdd_ffi_trivia_t **out_trivia) {
  cdd_ffi_trivia_t *head = NULL;
  cdd_ffi_trivia_t *tail = NULL;
  const cdd_trivia_t *curr = in_trivia;

  *out_trivia = NULL;

  while (curr) {
    cdd_ffi_trivia_t *new_node =
        (cdd_ffi_trivia_t *)CDD_CALLOC(1, sizeof(cdd_ffi_trivia_t));
    if (!new_node) {
      cdd_ffi_trivia_t *tmp;
      tmp = head;
      while (tmp) {
        cdd_ffi_trivia_t *nxt = tmp->next;
        C_CDD_FREE(tmp->text);
        C_CDD_FREE(tmp);
        tmp = nxt;
      }
      return CDD_C_ERROR_MEMORY;
    }

    if (curr->kind == TRIVIA_WHITESPACE || curr->kind == TRIVIA_NEWLINE) {
      new_node->kind = CDD_FFI_TRIVIA_WHITESPACE;
    } else if (curr->kind == TRIVIA_LINE_COMMENT) {
      new_node->kind = CDD_FFI_TRIVIA_COMMENT_LINE;
    } else {
      new_node->kind = CDD_FFI_TRIVIA_COMMENT_BLOCK;
    }

    new_node->text = (char *)CDD_CALLOC(1, curr->length + 1);
    if (!new_node->text) {
      cdd_ffi_trivia_t *tmp;
      C_CDD_FREE(new_node);
      tmp = head;
      while (tmp) {
        cdd_ffi_trivia_t *nxt = tmp->next;
        C_CDD_FREE(tmp->text);
        C_CDD_FREE(tmp);
        tmp = nxt;
      }
      return CDD_C_ERROR_MEMORY;
    }
    memcpy(new_node->text, curr->start, curr->length);
    new_node->text[curr->length] = '\0';
    new_node->next = NULL;

    if (!head) {
      head = new_node;
      tail = new_node;
    } else {
      tail->next = new_node;
      tail = new_node;
    }
    curr = curr->next;
  }

  *out_trivia = head;
  return CDD_C_SUCCESS;
}

/**
 * @brief Deep-copies a `cdd_ffi_type_t` value for trampoline callback metadata.
 *
 * @param[in] src Source type descriptor.
 * @param[out] dst Destination type descriptor (zeroed on entry).
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY on OOM.
 */
static NO_DISCARD cdd_c_error_t
copy_ffi_type_shallow_sig(const cdd_ffi_type_t *src, cdd_ffi_type_t *dst) {
  memset(dst, 0, sizeof(*dst));
  dst->kind = src->kind;
  dst->is_const = src->is_const;
  dst->pointer_depth = src->pointer_depth;
  dst->array_size = src->array_size;
  if (src->ref_name) {
    dst->ref_name = CDD_STRDUP(src->ref_name);
    if (!dst->ref_name)
      return CDD_C_ERROR_MEMORY;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses a simple C type slice (e.g., `const char *`, `struct Foo *`,
 * `int32_t`) into a `cdd_ffi_type_t`.
 *
 * @param[in] raw Start of C type text.
 * @param[in] len Byte length of `raw`.
 * @param[out] out_type Destination FFI type structure.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY on OOM.
 */
static NO_DISCARD cdd_c_error_t
parse_c_simple_type_str(const char *raw, size_t len, cdd_ffi_type_t *out_type) {
  char buf[256];
  char *p;
  size_t end;
  int ptr_depth;
  int is_const;
  cdd_c_error_t rc;

  memset(out_type, 0, sizeof(*out_type));
  if (len >= sizeof(buf))
    len = sizeof(buf) - 1;
  memcpy(buf, raw, len);
  buf[len] = '\0';

  p = buf;
  while (memchr(" \t\r\n", (unsigned char)*p, 4) != NULL)
    p++;
  end = strlen(p);
  while (end > 0 && memchr(" \t\r\n", (unsigned char)p[end - 1], 4) != NULL) {
    p[--end] = '\0';
  }

  is_const = 0;
  if (strncmp(p, "const ", 6) == 0) {
    is_const = 1;
    p += 6;
    while (memchr(" \t", (unsigned char)*p, 2) != NULL)
      p++;
  }

  end = strlen(p);
  ptr_depth = 0;
  while (end > 0 && memchr("* \t", (unsigned char)p[end - 1], 3) != NULL) {
    if (p[end - 1] == '*')
      ptr_depth++;
    p[--end] = '\0';
  }

  if (end >= 6 && strcmp(p + end - 6, " const") == 0) {
    is_const = 1;
    end -= 6;
    p[end] = '\0';
    while (memchr(" \t", (unsigned char)p[end - 1], 2) != NULL)
      p[--end] = '\0';
  }

  if (strncmp(p, "struct ", 7) == 0) {
    p += 7;
  } else if (strncmp(p, "union ", 6) == 0) {
    p += 6;
  } else if (strncmp(p, "enum ", 5) == 0) {
    p += 5;
    while (memchr(" \t", (unsigned char)*p, 2) != NULL)
      p++;
    out_type->kind = CDD_FFI_KIND_ENUM_REF;
    out_type->is_const = is_const;
    out_type->pointer_depth = ptr_depth;
    out_type->ref_name = CDD_STRDUP(p);
    return out_type->ref_name ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
  }
  while (memchr(" \t", (unsigned char)*p, 2) != NULL)
    p++;

  if (strcmp(p, "void") == 0 && ptr_depth == 1 && !is_const) {
    out_type->kind = CDD_FFI_KIND_OPAQUE_PTR;
    out_type->pointer_depth = 0;
    return CDD_C_SUCCESS;
  }

  rc = map_c_type_to_ffi_kind(p, &out_type->kind);
  out_type->is_const = is_const;
  out_type->pointer_depth = ptr_depth;
  if (out_type->kind == CDD_FFI_KIND_STRUCT_REF) {
    out_type->ref_name = CDD_STRDUP(p);
    if (!out_type->ref_name)
      return CDD_C_ERROR_MEMORY;
  }
  return rc;
}

/**
 * @brief Splits a single trimmed C parameter declaration into type and optional
 * name.
 *
 * @param[in] param_str Trimmed parameter slice (`param_len > 0`).
 * @param[in] param_len Length of `param_str`.
 * @param[out] out_field Destination parameter field structure.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY on OOM.
 */
static NO_DISCARD cdd_c_error_t parse_c_param_item(const char *param_str,
                                                   size_t param_len,
                                                   cdd_ffi_field_t *out_field) {
  size_t id_start;

  memset(out_field, 0, sizeof(*out_field));
  id_start = param_len;
  while (id_start > 0 && (isalnum((unsigned char)param_str[id_start - 1]) ||
                          param_str[id_start - 1] == '_')) {
    id_start--;
  }

  if (id_start > 0 && id_start < param_len) {
    size_t name_len = param_len - id_start;
    char kw_probe[32];
    int is_type_kw = 0;

    if (name_len < sizeof(kw_probe) - 2) {
      kw_probe[0] = ' ';
      memcpy(kw_probe + 1, param_str + id_start, name_len);
      kw_probe[name_len + 1] = ' ';
      kw_probe[name_len + 2] = '\0';
      if (strstr(" int long short char void bool float double signed unsigned "
                 "const ",
                 kw_probe) != NULL) {
        is_type_kw = 1;
      }
    }
    if (!is_type_kw) {
      out_field->name = (char *)CDD_CALLOC(name_len + 1, 1);
      if (!out_field->name)
        return CDD_C_ERROR_MEMORY;
      memcpy(out_field->name, param_str + id_start, name_len);
      return parse_c_simple_type_str(param_str, id_start, &out_field->type);
    }
  }

  return parse_c_simple_type_str(param_str, param_len, &out_field->type);
}

/**
 * @brief Parses a C function-pointer declaration `<ret> (*<name>)(<params>)`
 * into a `CDD_FFI_KIND_FUNCTION_PTR` type with signature metadata.
 *
 * @param[in] decl Null-terminated declaration text (without trailing `;`).
 * @param[out] out_name Non-NULL pointer receiving duplicated identifier name
 * (or NULL if unnamed).
 * @param[out] out_type Destination FFI type descriptor.
 * @return CDD_C_SUCCESS if matched and parsed, CDD_C_ERROR_NOT_FOUND if `decl`
 * is not a function-pointer declaration, or CDD_C_ERROR_MEMORY on OOM.
 */
static NO_DISCARD cdd_c_error_t parse_c_func_ptr_decl(
    const char *decl, char **out_name, cdd_ffi_type_t *out_type) {
  const char *open1;
  const char *star;
  const char *close1;
  const char *open2;
  const char *close2;
  const char *id_s;
  const char *id_e;
  cdd_c_error_t rc;

  *out_name = NULL;
  memset(out_type, 0, sizeof(*out_type));

  open1 = strchr(decl, '(');
  if (!open1)
    return CDD_C_ERROR_NOT_FOUND;
  star = open1 + 1;
  while (memchr(" \t", (unsigned char)*star, 2) != NULL)
    star++;
  if (*star != '*')
    return CDD_C_ERROR_NOT_FOUND;
  id_s = star + 1;
  while (memchr(" \t*", (unsigned char)*id_s, 3) != NULL)
    id_s++;
  close1 = strchr(id_s, ')');
  if (!close1)
    return CDD_C_ERROR_NOT_FOUND;
  id_e = close1;
  while (id_e > id_s && memchr(" \t", (unsigned char)id_e[-1], 2) != NULL)
    id_e--;

  open2 = close1 + 1;
  while (memchr(" \t", (unsigned char)*open2, 2) != NULL)
    open2++;
  if (*open2 != '(')
    return CDD_C_ERROR_NOT_FOUND;
  close2 = strrchr(open2, ')');
  if (!close2)
    return CDD_C_ERROR_NOT_FOUND;

  out_type->kind = CDD_FFI_KIND_FUNCTION_PTR;
  out_type->fn_return_type =
      (cdd_ffi_type_t *)CDD_CALLOC(1, sizeof(cdd_ffi_type_t));
  if (!out_type->fn_return_type)
    return CDD_C_ERROR_MEMORY;

  rc = parse_c_simple_type_str(decl, (size_t)(open1 - decl),
                               out_type->fn_return_type);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (id_e > id_s) {
    size_t nlen = (size_t)(id_e - id_s);
    *out_name = (char *)CDD_CALLOC(nlen + 1, 1);
    if (!*out_name)
      return CDD_C_ERROR_MEMORY;
    memcpy(*out_name, id_s, nlen);
  }

  {
    const char *cur = open2 + 1;
    while (cur < close2) {
      const char *seg_end;
      int depth;
      size_t seg_len;
      const char *trim_s;
      const char *trim_e;

      depth = 0;
      seg_end = cur;
      while (seg_end < close2) {
        if (*seg_end == '(')
          depth++;
        else if (*seg_end == ')')
          depth--;
        else if (*seg_end == ',' && depth == 0)
          break;
        seg_end++;
      }

      trim_s = cur;
      trim_e = seg_end;
      while (trim_s < trim_e &&
             memchr(" \t\r\n", (unsigned char)*trim_s, 4) != NULL)
        trim_s++;
      while (trim_e > trim_s &&
             memchr(" \t\r\n", (unsigned char)trim_e[-1], 4) != NULL)
        trim_e--;
      seg_len = (size_t)(trim_e - trim_s);

      if (seg_len == 3 && strncmp(trim_s, "...", 3) == 0) {
        out_type->fn_is_variadic = 1;
      } else if (seg_len > 0 &&
                 !(seg_len == 4 && strncmp(trim_s, "void", 4) == 0)) {
        cdd_ffi_field_t *new_params;
        new_params = (cdd_ffi_field_t *)CDD_REALLOC(
            out_type->fn_params,
            (out_type->fn_params_count + 1) * sizeof(cdd_ffi_field_t));
        if (!new_params)
          return CDD_C_ERROR_MEMORY;
        out_type->fn_params = new_params;
        rc = parse_c_param_item(
            trim_s, seg_len, &out_type->fn_params[out_type->fn_params_count]);
        if (rc != CDD_C_SUCCESS) {
          out_type->fn_params_count++;
          return rc;
        }
        out_type->fn_params_count++;
      }

      cur = (seg_end < close2) ? (seg_end + 1) : close2;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Populates callback signature metadata on a C++ `_Trampoline` vtable
 * field (`void *ctx` plus optional virtual method parameters).
 *
 * @param[in,out] field Target trampoline callback field.
 * @param[in] vm Optional virtual method descriptor (NULL for AddRef/Release).
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY on OOM.
 */
static NO_DISCARD cdd_c_error_t populate_trampoline_cb_sig(
    cdd_ffi_field_t *field, const cdd_ffi_virtual_method_t *vm) {
  size_t total_params;
  size_t p_idx;
  cdd_c_error_t rc;

  field->type.kind = CDD_FFI_KIND_FUNCTION_PTR;
  field->type.fn_return_type =
      (cdd_ffi_type_t *)CDD_CALLOC(1, sizeof(cdd_ffi_type_t));
  if (!field->type.fn_return_type)
    return CDD_C_ERROR_MEMORY;

  if (vm) {
    rc =
        copy_ffi_type_shallow_sig(&vm->return_type, field->type.fn_return_type);
    if (rc != CDD_C_SUCCESS)
      return rc;
    total_params = vm->args_count + 1;
  } else {
    field->type.fn_return_type->kind = CDD_FFI_KIND_VOID;
    total_params = 1;
  }

  field->type.fn_params =
      (cdd_ffi_field_t *)CDD_CALLOC(total_params, sizeof(cdd_ffi_field_t));
  if (!field->type.fn_params)
    return CDD_C_ERROR_MEMORY;
  field->type.fn_params_count = total_params;

  field->type.fn_params[0].name = CDD_STRDUP("ctx");
  if (!field->type.fn_params[0].name)
    return CDD_C_ERROR_MEMORY;
  field->type.fn_params[0].type.kind = CDD_FFI_KIND_OPAQUE_PTR;

  if (vm) {
    for (p_idx = 0; p_idx < vm->args_count; p_idx++) {
      if (vm->args[p_idx].name) {
        field->type.fn_params[p_idx + 1].name =
            CDD_STRDUP(vm->args[p_idx].name);
        if (!field->type.fn_params[p_idx + 1].name)
          return CDD_C_ERROR_MEMORY;
      }
      rc = copy_ffi_type_shallow_sig(&vm->args[p_idx].type,
                                     &field->type.fn_params[p_idx + 1].type);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Scans raw header text to extract `typedef Ret (*Name)(Args);`
 * function-pointer typedefs, struct callback field signatures, and packed
 * struct attributes.
 *
 * @param[in,out] ir Target FFI IR structure.
 * @param[in] content Raw C/C++ header content.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY on OOM.
 */
static NO_DISCARD cdd_c_error_t
extract_func_ptr_typedefs_and_packed(cdd_ffi_ir_t *ir, const char *content) {
  int global_pack_1;
  const char *p;
  size_t k;

  global_pack_1 = (strstr(content, "#pragma pack(1)") != NULL ||
                   strstr(content, "#pragma pack(push,") != NULL)
                      ? 1
                      : 0;

  if (global_pack_1) {
    for (k = 0; k < ir->nodes_count; k++) {
      if (ir->nodes[k].kind == CDD_FFI_NODE_STRUCT ||
          ir->nodes[k].kind == CDD_FFI_NODE_UNION) {
        ir->nodes[k].is_packed = 1;
      }
    }
  }

  for (k = 0; k < ir->nodes_count; k++) {
    if ((ir->nodes[k].kind == CDD_FFI_NODE_STRUCT ||
         ir->nodes[k].kind == CDD_FFI_NODE_UNION) &&
        ir->nodes[k].name) {
      char pattern[256];
      const char *found;
#if defined(_MSC_VER)
      sprintf_s(pattern, sizeof(pattern), "struct %s", ir->nodes[k].name);
#else
      CDD_SNPRINTF(pattern, sizeof(pattern), "struct %s", ir->nodes[k].name);
#endif
      found = strstr(content, pattern);
      if (found) {
        const char *lbrace = strchr(found, '{');
        const char *rbrace = lbrace ? strchr(lbrace, '}') : NULL;
        if (rbrace) {
          const char *semi = strchr(rbrace, ';');
          if (semi) {
            size_t span = (size_t)(semi - found);
            if (span < 4096) {
              const char *attr = strstr(found, "packed");
              if (attr && attr < semi) {
                ir->nodes[k].is_packed = 1;
              }
            }
          }
          {
            const char *stmt = lbrace + 1;
            while (stmt < rbrace) {
              const char *stmt_end = strchr(stmt, ';');
              if (!stmt_end || stmt_end > rbrace)
                break;
              if (memchr(stmt, '(', (size_t)(stmt_end - stmt))) {
                char decl_buf[512];
                size_t dlen = (size_t)(stmt_end - stmt);
                if (dlen < sizeof(decl_buf)) {
                  char *cb_name = NULL;
                  cdd_ffi_type_t cb_type;
                  cdd_c_error_t fp_rc;
                  memcpy(decl_buf, stmt, dlen);
                  decl_buf[dlen] = '\0';
                  fp_rc = parse_c_func_ptr_decl(decl_buf, &cb_name, &cb_type);
                  if (fp_rc == CDD_C_ERROR_MEMORY) {
                    C_CDD_FREE(cb_name);
                    return CDD_C_ERROR_MEMORY;
                  }
                  if (fp_rc == CDD_C_SUCCESS && cb_name) {
                    size_t f_idx;
                    int matched = 0;
                    for (f_idx = 0; f_idx < ir->nodes[k].fields_count;
                         f_idx++) {
                      if (ir->nodes[k].fields[f_idx].name &&
                          strcmp(ir->nodes[k].fields[f_idx].name, cb_name) ==
                              0) {
                        if (ir->nodes[k].fields[f_idx].type.ref_name) {
                          C_CDD_FREE(ir->nodes[k].fields[f_idx].type.ref_name);
                        }
                        ir->nodes[k].fields[f_idx].type = cb_type;
                        C_CDD_FREE(cb_name);
                        matched = 1;
                        break;
                      }
                    }
                    if (!matched) {
                      cdd_ffi_field_t *nf = (cdd_ffi_field_t *)CDD_REALLOC(
                          ir->nodes[k].fields, (ir->nodes[k].fields_count + 1) *
                                                   sizeof(cdd_ffi_field_t));
                      if (!nf) {
                        C_CDD_FREE(cb_name);
                        return CDD_C_ERROR_MEMORY;
                      }
                      ir->nodes[k].fields = nf;
                      memset(&ir->nodes[k].fields[ir->nodes[k].fields_count], 0,
                             sizeof(cdd_ffi_field_t));
                      ir->nodes[k].fields[ir->nodes[k].fields_count].name =
                          cb_name;
                      ir->nodes[k].fields[ir->nodes[k].fields_count].type =
                          cb_type;
                      ir->nodes[k].fields_count++;
                    }
                  } else {
                    C_CDD_FREE(cb_name);
                  }
                }
              }
              stmt = stmt_end + 1;
            }
          }
        }
      }
    }
  }

  p = content;
  while ((p = strstr(p, "typedef ")) != NULL) {
    if (p == content || memchr("\n\r \t;", (unsigned char)p[-1], 5) != NULL) {
      const char *semi = strchr(p + 8, ';');
      const char *brace = strchr(p + 8, '{');
      if (semi && (!brace || brace > semi)) {
        size_t dlen = (size_t)(semi - (p + 8));
        if (dlen < 512) {
          char decl_buf[512];
          char *td_name = NULL;
          cdd_ffi_type_t td_type;
          cdd_c_error_t fp_rc;
          memcpy(decl_buf, p + 8, dlen);
          decl_buf[dlen] = '\0';
          fp_rc = parse_c_func_ptr_decl(decl_buf, &td_name, &td_type);
          if (fp_rc == CDD_C_ERROR_MEMORY) {
            C_CDD_FREE(td_name);
            return CDD_C_ERROR_MEMORY;
          }
          if (fp_rc == CDD_C_SUCCESS && td_name) {
            int exists = 0;
            for (k = 0; k < ir->nodes_count; k++) {
              if (ir->nodes[k].name &&
                  strcmp(ir->nodes[k].name, td_name) == 0) {
                exists = 1;
                break;
              }
            }
            if (!exists) {
              cdd_ffi_ir_node_t *td_node = NULL;
              cdd_c_error_t add_rc =
                  ir_add_node(ir, CDD_FFI_NODE_TYPEDEF, td_name, &td_node);
              C_CDD_FREE(td_name);
              if (add_rc != CDD_C_SUCCESS)
                return add_rc;
              td_node->return_or_base_type = td_type;
            } else {
              C_CDD_FREE(td_name);
              C_CDD_FREE(td_type.fn_return_type->ref_name);
              C_CDD_FREE(td_type.fn_return_type);
              if (td_type.fn_params) {
                size_t pi;
                for (pi = 0; pi < td_type.fn_params_count; pi++) {
                  C_CDD_FREE(td_type.fn_params[pi].name);
                  C_CDD_FREE(td_type.fn_params[pi].type.ref_name);
                }
                C_CDD_FREE(td_type.fn_params);
              }
            }
          } else {
            C_CDD_FREE(td_name);
          }
        }
      }
    }
    p += 8;
  }

  /* Scan top-level C function prototypes/definitions containing callback
   * parameters (`(*name)(...)`) skipped by `c_inspector_extract_signatures` */
  {
    const char *line = content;
    int brace_depth = 0;
    while (*line != '\0') {
      const char *eol = strchr(line, '\n');
      size_t llen = eol ? (size_t)(eol - line) : strlen(line);
      if (brace_depth == 0 && llen > 0 && llen < 512) {
        const char *ls = line;
        size_t rem = llen;
        while (rem > 0 && memchr(" \t", (unsigned char)*ls, 2) != NULL) {
          ls++;
          rem--;
        }
        if (rem > 0 && memchr("#/*}", (unsigned char)*ls, 4) == NULL &&
            strncmp(ls, "typedef ", 8) != 0 && strncmp(ls, "struct ", 7) != 0 &&
            strncmp(ls, "union ", 6) != 0 && strncmp(ls, "enum ", 5) != 0 &&
            strncmp(ls, "class ", 6) != 0 && strncmp(ls, "template", 8) != 0 &&
            strstr(ls, "(*") != NULL) {
          const char *open_p = (const char *)memchr(ls, '(', rem);
          const char *close_p = NULL;
          if (open_p && open_p > ls) {
            size_t si;
            for (si = rem; si > 0; si--) {
              if (ls[si - 1] == ')') {
                close_p = ls + (si - 1);
                break;
              }
            }
          }
          if (open_p && close_p && close_p > open_p) {
            const char *id_e = open_p;
            const char *id_s;
            while (memchr(" \t", (unsigned char)id_e[-1], 2) != NULL)
              id_e--;
            id_s = id_e;
            while (id_s > ls &&
                   (isalnum((unsigned char)id_s[-1]) || id_s[-1] == '_'))
              id_s--;
            if (id_e > id_s && id_s > ls) {
              char fn_nm[128];
              size_t fn_len = (size_t)(id_e - id_s);
              if (fn_len < sizeof(fn_nm) &&
                  !(fn_len == 13 && strncmp(id_s, "__attribute__", 13) == 0)) {
                int already = 0;
                memcpy(fn_nm, id_s, fn_len);
                fn_nm[fn_len] = '\0';
                for (k = 0; k < ir->nodes_count; k++) {
                  if (ir->nodes[k].name &&
                      strcmp(ir->nodes[k].name, fn_nm) == 0) {
                    already = 1;
                    break;
                  }
                }
                if (!already) {
                  cdd_ffi_ir_node_t *fn_node = NULL;
                  cdd_c_error_t add_rc =
                      ir_add_node(ir, CDD_FFI_NODE_FUNCTION, fn_nm, &fn_node);
                  if (add_rc != CDD_C_SUCCESS)
                    return add_rc;
                  add_rc = parse_c_simple_type_str(
                      ls, (size_t)(id_s - ls), &fn_node->return_or_base_type);
                  if (add_rc != CDD_C_SUCCESS)
                    return add_rc;
                  {
                    char p_buf[512];
                    size_t p_len = (size_t)(close_p - (open_p + 1));
                    char *cb_nm = NULL;
                    cdd_ffi_type_t cb_t;
                    cdd_c_error_t fp_rc;
                    memcpy(p_buf, open_p + 1, p_len);
                    p_buf[p_len] = '\0';
                    fp_rc = parse_c_func_ptr_decl(p_buf, &cb_nm, &cb_t);
                    if (fp_rc == CDD_C_ERROR_MEMORY) {
                      C_CDD_FREE(cb_nm);
                      return CDD_C_ERROR_MEMORY;
                    }
                    if (fp_rc == CDD_C_SUCCESS) {
                      fn_node->fields = (cdd_ffi_field_t *)CDD_CALLOC(
                          1, sizeof(cdd_ffi_field_t));
                      if (!fn_node->fields) {
                        C_CDD_FREE(cb_nm);
                        return CDD_C_ERROR_MEMORY;
                      }
                      fn_node->fields[0].name =
                          cb_nm ? cb_nm : CDD_STRDUP("cb");
                      fn_node->fields[0].type = cb_t;
                      fn_node->fields_count = 1;
                      if (!fn_node->fields[0].name)
                        return CDD_C_ERROR_MEMORY;
                    } else {
                      C_CDD_FREE(cb_nm);
                    }
                  }
                }
              }
            }
          }
        }
      }
      {
        size_t bi;
        for (bi = 0; bi < llen; bi++) {
          if (line[bi] == '{')
            brace_depth++;
          else if (line[bi] == '}' && brace_depth > 0)
            brace_depth--;
        }
      }
      if (!eol)
        break;
      line = eol + 1;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Extracts types, functions, macros, base classes, and virtual method
 * trampolines from a single header file into the FFI IR.
 *
 * @param[in,out] ir Target FFI IR structure to populate.
 * @param[in] filename Path to the header file being analyzed.
 * @param[in] content Raw source text of the header file.
 * @param[in] config Optional bindings generation configuration.
 * @param[in] tree_full Optional parsed CST tree for trivia attachment.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static cdd_c_error_t extract_single_file_exports_impl(
    cdd_ffi_ir_t *ir, const char *filename, const char *content,
    const cdd_generate_bindings_config_t *config, cdd_cst_tree_t *tree_full) {
  cdd_c_error_t rc;
  size_t i;
  struct TypeDefList types;
  struct FuncSigList sigs;
  if (config) {
    /* Reserved for configuration-specific single-file extraction flags */
  }

  memset(&types, 0, sizeof(types));

  /* Extract Types */
  rc = c_inspector_scan_file_types(filename, &types);
  if (rc != CDD_C_SUCCESS) {
    rc = cdd_keep_err(rc, type_def_list_free(&types));
    return rc;
  }

  for (i = 0; i < types.size; i++) {
    cdd_ffi_ir_node_t *node = NULL;
    if (types.items[i].kind == KIND_ENUM) {
      rc = ir_add_node(ir, CDD_FFI_NODE_ENUM, types.items[i].name, &node);
      if (rc != CDD_C_SUCCESS)
        break;
      if (tree_full) {
        /* CST parser currently does not structure enums as declarations we can
         * look up by name. */
      }
      if (types.items[i].details.enum_members->size > 0) {
        size_t j;
        struct EnumMembers *em = types.items[i].details.enum_members;
        node->variants_count = em->size;
        node->variants = (cdd_ffi_enum_variant_t *)CDD_CALLOC(
            em->size, sizeof(cdd_ffi_enum_variant_t));
        if (!node->variants) {
          rc = CDD_C_ERROR_MEMORY;
          break;
        }
        for (j = 0; j < em->size; j++) {
          node->variants[j].name = CDD_STRDUP(em->members[j]);
          node->variants[j].value = CDD_STRDUP(em->members[j]);
          if (!node->variants[j].name || !node->variants[j].value) {
            rc = CDD_C_ERROR_MEMORY;
            break;
          }
        }
        if (rc != CDD_C_SUCCESS)
          break;
      }
    } else {
      rc = ir_add_node(ir, CDD_FFI_NODE_STRUCT, types.items[i].name, &node);
      if (rc != CDD_C_SUCCESS)
        break;
      if (tree_full) {
        cdd_cst_node_t *cst_node = NULL;
        rc = find_class_declaration_by_name(tree_full, types.items[i].name,
                                            &cst_node);
#ifdef CDD_BUILD_TESTS
        if (g_cdd_ffi_fail_cst_node)
          cst_node = NULL;
#endif
        if (cst_node) {
          cdd_token_t *first = NULL;
          cdd_token_t *last = NULL;
          rc = get_first_token(cst_node, &first);
          rc = get_last_token(cst_node, &last);
          if (first->leading_trivia) {
            rc =
                copy_trivia_chain(first->leading_trivia, &node->leading_trivia);
            if (rc != CDD_C_SUCCESS)
              break;
          }
          if (last->trailing_trivia) {
            rc = copy_trivia_chain(last->trailing_trivia,
                                   &node->trailing_trivia);
            if (rc != CDD_C_SUCCESS)
              break;
          }
        }
      }
      /* Add parsing for base classes from CST */
      {
        cdd_cst_tree_t *tree_base = NULL;
        cdd_cst_query_result_t structs_base = {0};
        size_t si;
        az_span span_base;
        span_base = az_span_create_from_str(content);
        rc = cdd_cst_parse(span_base, &tree_base);
        if (rc != CDD_C_SUCCESS) {
          rc = CDD_C_SUCCESS;
        } else {
          rc = cdd_cst_find_nodes_by_type(
              tree_base->root, CDD_CST_CLASS_DECLARATION, &structs_base);
#ifdef CDD_BUILD_TESTS
          if (g_cdd_ffi_extractor_fail == 1)
            rc = CDD_C_ERROR_MEMORY;
#endif
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(structs_base.nodes);
            rc = cdd_keep_err(rc, cdd_cst_tree_free(tree_base));
            break;
          }
          for (si = 0; si < structs_base.size; si++) {
            cdd_cst_node_t *s_node = structs_base.nodes[si];
            cdd_token_t *name_tok = NULL;
            rc = get_class_name(s_node, &name_tok);
#ifdef CDD_BUILD_TESTS
            if (g_cdd_ffi_extractor_fail == 2)
              rc = CDD_C_ERROR_UNKNOWN;
#endif
            if (rc != CDD_C_SUCCESS) {
              break;
            }
#ifdef CDD_BUILD_TESTS
            if (g_cdd_ffi_fail_class_name)
              name_tok = NULL;
#endif
            if (name_tok && name_tok->length == strlen(types.items[i].name) &&
                strncmp((const char *)name_tok->start, types.items[i].name,
                        name_tok->length) == 0) {
              /* Find CDD_CST_BASE_CLASS_LIST */
              size_t c_idx;
              for (c_idx = 0; c_idx < s_node->num_children; c_idx++) {
                if (s_node->children[c_idx].kind == CDD_CST_CHILD_NODE &&
                    s_node->children[c_idx].val.node->kind ==
                        CDD_CST_BASE_CLASS_LIST) {
                  cdd_cst_node_t *base_list = s_node->children[c_idx].val.node;
                  size_t b_idx;
                  node->base_classes_count = 0;
                  for (b_idx = 0; b_idx < base_list->num_children; b_idx++) {
                    if (base_list->children[b_idx].kind == CDD_CST_CHILD_NODE) {
                      node->base_classes_count++;
                    }
                  }
                  node->base_classes = (cdd_ffi_base_class_t *)CDD_CALLOC(
                      node->base_classes_count, sizeof(cdd_ffi_base_class_t));
                  if (!node->base_classes) {
                    node->base_classes_count = 0;
                    rc = CDD_C_ERROR_MEMORY;
                    break;
                  }
                  {
                    size_t bi = 0;
                    for (b_idx = 0; b_idx < base_list->num_children; b_idx++) {
                      if (base_list->children[b_idx].kind ==
                          CDD_CST_CHILD_NODE) {
                        cdd_cst_node_t *base_spec =
                            base_list->children[b_idx].val.node;
                        size_t s_idx;
                        for (s_idx = 0; s_idx < base_spec->num_children;
                             s_idx++) {
                          cdd_token_t *tok =
                              base_spec->children[s_idx].val.token;
                          if (tok->kind == CDD_TOKEN_KEYWORD_VIRTUAL) {
                            node->base_classes[bi].is_virtual = 1;
                          } else if (tok->kind == CDD_TOKEN_IDENTIFIER) {
                            char *bname = (char *)CDD_CALLOC(tok->length + 1,
                                                             sizeof(char));
                            if (!bname) {
                              rc = CDD_C_ERROR_MEMORY;
                              break;
                            }
                            memcpy(bname, tok->start, tok->length);
                            bname[tok->length] = '\0';
                            node->base_classes[bi].name = bname;
                          }
                        }
                        if (rc != CDD_C_SUCCESS)
                          break;
                        bi++;
                      }
                    }
                  }
                  break;
                }
              }
              if (rc != CDD_C_SUCCESS)
                break;
            }
          }
          C_CDD_FREE(structs_base.nodes);
          rc = cdd_keep_err(rc, cdd_cst_tree_free(tree_base));
        }
      }
      if (rc != CDD_C_SUCCESS)
        break;

      if (types.items[i].details.struct_fields->size > 0) {
        size_t j;
        struct StructFields *sf = types.items[i].details.struct_fields;
        node->fields_count = sf->size;
        node->fields =
            (cdd_ffi_field_t *)CDD_CALLOC(sf->size, sizeof(cdd_ffi_field_t));
#ifdef CDD_BUILD_TESTS
        if (g_cdd_ffi_extractor_fail == 3) {
          C_CDD_FREE(node->fields);
          node->fields = NULL;
        }
#endif
        if (!node->fields) {
          rc = CDD_C_ERROR_MEMORY;
          break;
        }
        for (j = 0; j < sf->size; j++) {
          const char *target_c_type;
          node->fields[j].name = CDD_STRDUP(sf->fields[j].name);
          if (!node->fields[j].name) {
            rc = CDD_C_ERROR_MEMORY;
            break;
          }
          target_c_type = sf->fields[j].ref[0] != '\0' ? sf->fields[j].ref
                                                       : sf->fields[j].type;
          rc =
              map_c_type_to_ffi_kind(target_c_type, &node->fields[j].type.kind);
#ifdef CDD_BUILD_TESTS
          if (g_cdd_ffi_extractor_fail == 6)
            rc = CDD_C_ERROR_UNKNOWN;
#endif
          if (rc != CDD_C_SUCCESS)
            break;
          if (node->fields[j].type.kind == CDD_FFI_KIND_STRUCT_REF) {
            node->fields[j].type.ref_name = CDD_STRDUP(target_c_type);
#ifdef CDD_BUILD_TESTS
            if (g_cdd_ffi_extractor_fail == 4) {
              C_CDD_FREE(node->fields[j].type.ref_name);
              node->fields[j].type.ref_name = NULL;
            }
#endif
            if (!node->fields[j].type.ref_name) {
              rc = CDD_C_ERROR_MEMORY;
              break;
            }
          } else {
            switch (node->fields[j].type.kind) {
            case CDD_FFI_KIND_TEMPLATE_STRUCT_REF:
            case CDD_FFI_KIND_STD_VECTOR:
            case CDD_FFI_KIND_STD_SHARED_PTR:
            case CDD_FFI_KIND_STD_UNIQUE_PTR:
              rc = parse_template_type(target_c_type, &node->fields[j].type);
              break;
            default:
              break;
            }
            if (rc != CDD_C_SUCCESS) {
              break;
            }
          }
        }
        if (rc != CDD_C_SUCCESS)
          break;
      }
    }
  }
  rc = cdd_keep_err(rc, type_def_list_free(&types));

  if (rc != CDD_C_SUCCESS)
    return rc;

  /* Extract Functions */
  memset(&sigs, 0, sizeof(sigs));

  rc = c_inspector_extract_signatures(content, &sigs);
  if (rc != CDD_C_SUCCESS) {
    rc = cdd_keep_err(rc, func_sig_list_free(&sigs));
    return rc;
  }

  for (i = 0; i < sigs.size; i++) {
    cdd_ffi_ir_node_t *node = NULL;
    const char *lparen;
    const char *rparen;
    rc = ir_add_node(ir, CDD_FFI_NODE_FUNCTION, sigs.items[i].name, &node);
    if (rc != CDD_C_SUCCESS)
      break;
    if (tree_full) {
      cdd_cst_node_t *cst_node = NULL;
      rc = find_func_definition_by_name(tree_full, sigs.items[i].name,
                                        &cst_node);
#ifdef CDD_BUILD_TESTS
      if (g_cdd_ffi_fail_cst_node)
        cst_node = NULL;
#endif
      if (cst_node) {
        cdd_token_t *first = NULL;
        cdd_token_t *last = NULL;
        rc = get_first_token(cst_node, &first);
        rc = get_last_token(cst_node, &last);
#ifdef CDD_BUILD_TESTS
        if (g_cdd_ffi_fail_token) {
          first = NULL;
          last = NULL;
        }
#endif
        if (first && first->leading_trivia) {
          rc = copy_trivia_chain(first->leading_trivia, &node->leading_trivia);
          if (rc != CDD_C_SUCCESS)
            break;
        }
        if (last && last->trailing_trivia) {
          rc = copy_trivia_chain(last->trailing_trivia, &node->trailing_trivia);
          if (rc != CDD_C_SUCCESS)
            break;
        }
      }
    }

    node->return_or_base_type.kind = CDD_FFI_KIND_VOID;
    node->is_variadic = sigs.items[i].is_variadic;
    if (sigs.items[i].doc) {
      node->doc = CDD_STRDUP(sigs.items[i].doc);
      if (!node->doc) {
        rc = CDD_C_ERROR_MEMORY;
        break;
      }
      if (strstr(sigs.items[i].doc, "@ffi_release_gil") ||
          strstr(sigs.items[i].doc, "@blocking")) {
        node->requires_gil_release = 1;
      }
    }
    lparen = strchr(sigs.items[i].sig, '(');
    rparen = strrchr(sigs.items[i].sig, ')');
    if (rparen > lparen + 1) {
      char params_str[1024];
      char *seg_start;
      size_t len = (size_t)(rparen - lparen - 1);
      if (len >= sizeof(params_str))
        len = sizeof(params_str) - 1;
#if defined(_MSC_VER)
      strncpy_s(params_str, sizeof(params_str), lparen + 1, len);
#else
      strncpy(params_str, lparen + 1, len);
#endif
      params_str[len] = '\0';

      seg_start = params_str;
      for (;;) {
        char *seg_end = seg_start;
        char *param_trim;
        int pdepth = 0;
        char saved_ch;

        while (*seg_end != '\0') {
          if (*seg_end == '(')
            pdepth++;
          else if (*seg_end == ')')
            pdepth--;
          else if (*seg_end == ',' && pdepth == 0)
            break;
          seg_end++;
        }
        saved_ch = *seg_end;
        *seg_end = '\0';

        param_trim = seg_start;
        while (memchr(" \t", (unsigned char)*param_trim, 2) != NULL)
          param_trim++;
        {
          size_t plen = strlen(param_trim);
          while (plen > 0 &&
                 memchr(" \t", (unsigned char)param_trim[plen - 1], 2) != NULL)
            param_trim[--plen] = '\0';
        }

        if (param_trim[0] != '\0' && strcmp(param_trim, "void") != 0 &&
            strcmp(param_trim, "...") != 0) {
          cdd_ffi_field_t *new_fields;
          size_t new_cap = node->fields_count + 1;
          new_fields = (cdd_ffi_field_t *)CDD_REALLOC(
              node->fields, new_cap * sizeof(cdd_ffi_field_t));
          if (!new_fields) {
            rc = CDD_C_ERROR_MEMORY;
            break;
          }
          node->fields = new_fields;
          memset(&node->fields[node->fields_count], 0, sizeof(cdd_ffi_field_t));

          {
            char *last_space = strrchr(param_trim, ' ');
            char *name = last_space ? last_space + 1 : param_trim;
            char *astx = strchr(name, '*');
            if (astx)
              name = astx + 1;

            node->fields[node->fields_count].name = CDD_STRDUP(name);
            if (!node->fields[node->fields_count].name) {
              node->fields_count++;
              rc = CDD_C_ERROR_MEMORY;
              break;
            }
            node->fields[node->fields_count].type.kind = CDD_FFI_KIND_INT32;
            node->fields[node->fields_count].intent = CDD_FFI_INTENT_UNKNOWN;

            if (node->doc) {
              char search_out[128], search_in[128], search_inout[128];
#if defined(_MSC_VER)
              sprintf_s(search_out, sizeof(search_out), "@param[out] %.100s",
                        name);
              sprintf_s(search_in, sizeof(search_in), "@param[in] %.100s",
                        name);
              sprintf_s(search_inout, sizeof(search_inout),
                        "@param[in,out] %.100s", name);
#else
              CDD_SNPRINTF(search_out, sizeof(search_out), "@param[out] %.100s",
                           name);
              CDD_SNPRINTF(search_in, sizeof(search_in), "@param[in] %.100s",
                           name);
              CDD_SNPRINTF(search_inout, sizeof(search_inout),
                           "@param[in,out] %.100s", name);
#endif
              if (strstr(node->doc, search_inout)) {
                node->fields[node->fields_count].intent = CDD_FFI_INTENT_INOUT;
              } else if (strstr(node->doc, search_out)) {
                node->fields[node->fields_count].intent = CDD_FFI_INTENT_OUT;
              } else if (strstr(node->doc, search_in)) {
                node->fields[node->fields_count].intent = CDD_FFI_INTENT_IN;
              }
            }
            if (strstr(param_trim, "_Out_")) {
              node->fields[node->fields_count].intent = CDD_FFI_INTENT_OUT;
            }
            {
              const char *writes = strstr(param_trim, "_Out_writes_(");
              if (writes) {
                char len_buf[64] = {0};
                const char *start = writes + 13;
                size_t wlen = strcspn(start, ")") & 63u;
#if defined(_MSC_VER)
                strncpy_s(len_buf, sizeof(len_buf), start, wlen);
#else
                strncpy(len_buf, start, wlen);
#endif
                node->fields[node->fields_count].intent = CDD_FFI_INTENT_OUT;
                node->fields[node->fields_count].array_length_ref =
                    CDD_STRDUP(len_buf);
              }
            }
            if (strstr(param_trim, "_Inout_"))
              node->fields[node->fields_count].intent = CDD_FFI_INTENT_INOUT;
            if (strstr(param_trim, "_In_"))
              node->fields[node->fields_count].intent = CDD_FFI_INTENT_IN;

            node->fields_count++;
          }
        }

        if (saved_ch == '\0')
          break;
        seg_start = seg_end + 1;
      }
    }
  }
  {
    cdd_c_error_t rc_sigs = func_sig_list_free(&sigs);
    if (rc == CDD_C_SUCCESS)
      rc = rc_sigs;
  }
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = extract_func_ptr_typedefs_and_packed(ir, content);
  if (rc != CDD_C_SUCCESS)
    return rc;

  /* Extract Macros as Constants */
  {
    struct PreprocessorContext pp_ctx;
    rc = pp_context_init(&pp_ctx);
    rc = pp_scan_defines(&pp_ctx, filename);
    {
      size_t j;
      for (j = 0; j < pp_ctx.macro_count; j++) {
        if (!pp_ctx.macros[j].is_function_like && pp_ctx.macros[j].value) {
          cdd_ffi_ir_node_t *node = NULL;
          rc =
              ir_add_node(ir, CDD_FFI_NODE_MACRO, pp_ctx.macros[j].name, &node);
          if (rc != CDD_C_SUCCESS) {
            rc = cdd_keep_err(rc, pp_context_free(&pp_ctx));
            return rc;
          }
          {
            cdd_macro_eval_result_t eval_res;
            cdd_c_error_t eval_rc;
            memset(&eval_res, 0, sizeof(eval_res));
            node->variants_count = 1;
            node->variants = (cdd_ffi_enum_variant_t *)CDD_CALLOC(
                1, sizeof(cdd_ffi_enum_variant_t));
            if (!node->variants) {
              node->variants_count = 0;
              return cdd_keep_err(CDD_C_ERROR_MEMORY, pp_context_free(&pp_ctx));
            }
            node->variants[0].name = CDD_STRDUP(pp_ctx.macros[j].name);
            node->variants[0].value = CDD_STRDUP(pp_ctx.macros[j].value);
            eval_rc =
                cdd_macro_evaluate(&pp_ctx, pp_ctx.macros[j].value, &eval_res);
            if (eval_rc == CDD_C_SUCCESS) {
              node->inferred_type = (cdd_ffi_macro_type_t)eval_res.type;
              if (eval_res.type == MACRO_EVAL_TYPE_INT) {
                char buf[64];
                rc = cdd_keep_err(
                    rc, int64_to_str(eval_res.int_val, buf, sizeof(buf)));
                node->evaluated_value = CDD_STRDUP(buf);
              } else if (eval_res.type == MACRO_EVAL_TYPE_FLOAT) {
                char buf[64];
#if defined(_MSC_VER)
                sprintf_s(buf, sizeof(buf), "%f", eval_res.float_val);
#else
                CDD_SNPRINTF(buf, sizeof(buf), "%f", eval_res.float_val);
#endif
                rc = cdd_ffi_do_strdup(buf, &node->evaluated_value);
              } else {
                rc =
                    cdd_ffi_do_strdup(eval_res.str_val, &node->evaluated_value);
              }
            } else {
              node->inferred_type = CDD_FFI_MACRO_TYPE_UNKNOWN;
              node->evaluated_value = NULL;
            }
            rc = cdd_macro_eval_result_free(&eval_res);
          }
        }
      }
    }
    rc = pp_context_free(&pp_ctx);
  }

  /* Inject casting helpers for multiple inheritance */
  {
    size_t orig_count = ir->nodes_count;
    size_t k;
    for (k = 0; k < orig_count; k++) {
      if (ir->nodes[k].kind == CDD_FFI_NODE_STRUCT &&
          ir->nodes[k].base_classes_count > 0) {
        size_t b;
        for (b = 0; b < ir->nodes[k].base_classes_count; b++) {
          cdd_ffi_ir_node_t *upcast_node = NULL;
          cdd_ffi_ir_node_t *downcast_node = NULL;
          char up_name[256];
          char down_name[256];

#if defined(_MSC_VER)
          sprintf_s(up_name, sizeof(up_name), "%s_upcast_to_%s",
                    ir->nodes[k].name, ir->nodes[k].base_classes[b].name);
          sprintf_s(down_name, sizeof(down_name), "%s_downcast_to_%s",
                    ir->nodes[k].base_classes[b].name, ir->nodes[k].name);
#else
          CDD_SNPRINTF(up_name, sizeof(up_name), "%s_upcast_to_%s",
                       ir->nodes[k].name, ir->nodes[k].base_classes[b].name);
          CDD_SNPRINTF(down_name, sizeof(down_name), "%s_downcast_to_%s",
                       ir->nodes[k].base_classes[b].name, ir->nodes[k].name);
#endif

          rc = ir_add_node(ir, CDD_FFI_NODE_FUNCTION, up_name, &upcast_node);
          if (rc != CDD_C_SUCCESS)
            return rc;
          upcast_node->return_or_base_type.kind = CDD_FFI_KIND_STRUCT_REF;
          upcast_node->return_or_base_type.ref_name =
              CDD_STRDUP(ir->nodes[k].base_classes[b].name);
          upcast_node->fields_count = 1;
          upcast_node->fields =
              (cdd_ffi_field_t *)CDD_CALLOC(1, sizeof(cdd_ffi_field_t));
          if (upcast_node->fields) {
            upcast_node->fields[0].name = CDD_STRDUP("ptr");
            upcast_node->fields[0].type.kind = CDD_FFI_KIND_STRUCT_REF;
            upcast_node->fields[0].type.ref_name =
                CDD_STRDUP(ir->nodes[k].name);
          }

          rc =
              ir_add_node(ir, CDD_FFI_NODE_FUNCTION, down_name, &downcast_node);
          if (rc != CDD_C_SUCCESS)
            return rc;
          downcast_node->return_or_base_type.kind = CDD_FFI_KIND_STRUCT_REF;
          downcast_node->return_or_base_type.ref_name =
              CDD_STRDUP(ir->nodes[k].name);
          downcast_node->fields_count = 1;
          downcast_node->fields =
              (cdd_ffi_field_t *)CDD_CALLOC(1, sizeof(cdd_ffi_field_t));
          if (downcast_node->fields) {
            downcast_node->fields[0].name = CDD_STRDUP("ptr");
            downcast_node->fields[0].type.kind = CDD_FFI_KIND_STRUCT_REF;
            downcast_node->fields[0].type.ref_name =
                CDD_STRDUP(ir->nodes[k].base_classes[b].name);
          }
        }
      }

      /* Trampoline generation for virtual methods */
      if (ir->nodes[k].kind == CDD_FFI_NODE_STRUCT &&
          ir->nodes[k].virtual_methods_count > 0) {
        cdd_ffi_ir_node_t *trampoline_node = NULL;
        char tramp_name[256];
        size_t vm_count = ir->nodes[k].virtual_methods_count;
        size_t m;

#if defined(_MSC_VER)
        sprintf_s(tramp_name, sizeof(tramp_name), "%s_Trampoline",
                  ir->nodes[k].name);
#else
        CDD_SNPRINTF(tramp_name, sizeof(tramp_name), "%s_Trampoline",
                     ir->nodes[k].name);
#endif

        rc = ir_add_node(ir, CDD_FFI_NODE_STRUCT, tramp_name, &trampoline_node);
        if (rc != CDD_C_SUCCESS)
          return rc;

        trampoline_node->fields_count = vm_count + 3;
        trampoline_node->fields = (cdd_ffi_field_t *)CDD_CALLOC(
            trampoline_node->fields_count, sizeof(cdd_ffi_field_t));
        if (!trampoline_node->fields) {
          trampoline_node->fields_count = 0;
          return CDD_C_ERROR_MEMORY;
        }

        trampoline_node->fields[0].name = CDD_STRDUP("target_lang_ctx");
        trampoline_node->fields[0].type.kind = CDD_FFI_KIND_OPAQUE_PTR;

        trampoline_node->fields[1].name = CDD_STRDUP("cb_AddRef");
        rc = populate_trampoline_cb_sig(&trampoline_node->fields[1], NULL);
        if (rc != CDD_C_SUCCESS)
          return rc;

        trampoline_node->fields[2].name = CDD_STRDUP("cb_Release");
        rc = populate_trampoline_cb_sig(&trampoline_node->fields[2], NULL);
        if (rc != CDD_C_SUCCESS)
          return rc;

        for (m = 0; m < vm_count; m++) {
          char cb_name[256];
#if defined(_MSC_VER)
          sprintf_s(cb_name, sizeof(cb_name), "cb_%s",
                    ir->nodes[k].virtual_methods[m].name);
#else
          CDD_SNPRINTF(cb_name, sizeof(cb_name), "cb_%s",
                       ir->nodes[k].virtual_methods[m].name);
#endif
          trampoline_node->fields[m + 3].name = CDD_STRDUP(cb_name);
          rc = populate_trampoline_cb_sig(&trampoline_node->fields[m + 3],
                                          &ir->nodes[k].virtual_methods[m]);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      }
    }
  }

  return rc;
}

/**
 * @brief Parses `content` into a CST tree and extracts single-file FFI exports
 * into `ir`.
 *
 * @param[in,out] ir Target FFI IR structure to populate.
 * @param[in] filename Path to the header file being analyzed.
 * @param[in] content Raw source text of the header file.
 * @param[in] config Optional bindings generation configuration.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static cdd_c_error_t
extract_single_file_exports(cdd_ffi_ir_t *ir, const char *filename,
                            const char *content,
                            const cdd_generate_bindings_config_t *config) {
  cdd_c_error_t rc;
  cdd_cst_tree_t *tree_full = NULL;
  az_span span_full;

  if (!ir || !filename || !content)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  span_full = az_span_create_from_str(content);
  if (cdd_cst_parse(span_full, &tree_full) != 0) {
    tree_full = NULL;
  }

  rc = extract_single_file_exports_impl(ir, filename, content, config,
                                        tree_full);

  if (tree_full) {
    rc = cdd_keep_err(rc, cdd_cst_tree_free(tree_full));
  }

  return rc;
}

/**
 * @brief Context for merging included header files into FFI IR.
 */
struct IncludeMergeCtx {
  cdd_ffi_ir_t *ir; /**< FFI IR being built */
  const cdd_generate_bindings_config_t
      *config;                        /**< Binding generation config */
  char **visited;                     /**< Array of visited file paths */
  size_t visited_count;               /**< Number of visited paths */
  size_t visited_capacity;            /**< Capacity of visited array */
  cdd_c_error_t err;                  /**< Error code accumulated */
  struct PreprocessorContext *pp_ctx; /**< Preprocessor context */
};

/**
 * @brief Checks if a path has been visited.
 *
 * @param[in,out] ctx Pointer to merge context.
 * @param[in] path File path.
 * @param[out] out_visited Pointer to receive 1 if visited, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
static cdd_c_error_t is_visited(struct IncludeMergeCtx *ctx, const char *path,
                                int *out_visited) {
  size_t k;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_ffi_fail_is_visited)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!ctx || !path || !out_visited)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_visited = 0;
  for (k = 0; k < ctx->visited_count; k++) {
    if (strcmp(ctx->visited[k], path) == 0) {
      *out_visited = 1;
      break;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Marks a path as visited.
 *
 * @param[in,out] ctx Pointer to merge context.
 * @param[in] path File path.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
static cdd_c_error_t add_visited(struct IncludeMergeCtx *ctx,
                                 const char *path) {
  if (!ctx || !path)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (ctx->visited_count >= ctx->visited_capacity) {
    size_t new_cap =
        ctx->visited_capacity == 0 ? 16 : ctx->visited_capacity * 2;
    char **new_visited =
        (char **)CDD_REALLOC(ctx->visited, new_cap * sizeof(char *));
    if (!new_visited)
      return CDD_C_ERROR_MEMORY;
    ctx->visited = new_visited;
    ctx->visited_capacity = new_cap;
  }
  ctx->visited[ctx->visited_count] = CDD_STRDUP(path);
  if (!ctx->visited[ctx->visited_count])
    return CDD_C_ERROR_MEMORY;
  ctx->visited_count++;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t include_visitor(const struct IncludeInfo *info,
                                     void *user_data);

/**
 * @brief Recursively extracts exports from an included header file.
 *
 * @param[in] filename File name.
 * @param[in] content File contents string.
 * @param[in,out] ctx Pointer to merge context.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
static cdd_c_error_t extract_exports_recursive(const char *filename,
                                               const char *content,
                                               struct IncludeMergeCtx *ctx) {
  cdd_c_error_t rc;
  int visited = 0;
  rc = is_visited(ctx, filename, &visited);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (visited)
    return CDD_C_SUCCESS;
  rc = add_visited(ctx, filename);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = extract_single_file_exports(ctx->ir, filename, content, ctx->config);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (ctx->config && ctx->config->recursive_includes) {
    rc = pp_scan_includes(filename, ctx->pp_ctx, include_visitor, ctx);
  }
  return rc;
}

/**
 * @brief Include visitor callback for recursive header parsing.
 *
 * @param[in] info Include directive information.
 * @param[in,out] user_data Pointer to merge context.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
static cdd_c_error_t include_visitor(const struct IncludeInfo *info,
                                     void *user_data) {
  struct IncludeMergeCtx *ctx;
  if (!info || !user_data)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  ctx = (struct IncludeMergeCtx *)user_data;
  if (ctx->err != CDD_C_SUCCESS)
    return ctx->err;
  if (info->kind == PP_DIR_INCLUDE && info->resolved_path) {
    int visited = 0;
    cdd_c_error_t rc_v = is_visited(ctx, info->resolved_path, &visited);
    if (rc_v != CDD_C_SUCCESS) {
      ctx->err = rc_v;
      return rc_v;
    }
    if (!visited) {
      char *content = NULL;
      size_t sz = 0;
      cdd_c_error_t rc = read_to_file(info->resolved_path, "r", &content, &sz);
      if (rc == CDD_C_SUCCESS) {
        cdd_c_error_t ext_rc =
            extract_exports_recursive(info->resolved_path, content, ctx);
        C_CDD_FREE(content);
        if (ext_rc != CDD_C_SUCCESS) {
          ctx->err = ext_rc;
          return ext_rc;
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Extracts all exported C/C++ types, functions, and macros from
 * `filename` (and optionally its recursive includes) into a newly allocated
 * `cdd_ffi_ir_t`.
 *
 * @param[in] filename Header filename being analyzed.
 * @param[in] content Raw C/C++ header source text.
 * @param[in] config Optional binding generation configuration parameters.
 * @param[out] out_ir Output pointer receiving the newly allocated FFI IR
 * structure.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t
cdd_ffi_ir_extract_exports(const char *filename, const char *content,
                           const cdd_generate_bindings_config_t *config,
                           cdd_ffi_ir_t **out_ir) {
  cdd_ffi_ir_t *ir;
  struct IncludeMergeCtx ctx;
  struct PreprocessorContext pp_ctx;
  size_t k;
  cdd_c_error_t rc;

  if (!filename || !content || !out_ir) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  ir = (cdd_ffi_ir_t *)CDD_CALLOC(1, sizeof(cdd_ffi_ir_t));
  if (!ir) {
    return CDD_C_ERROR_MEMORY;
  }

  memset(&ctx, 0, sizeof(ctx));
  memset(&pp_ctx, 0, sizeof(pp_ctx));
  rc = pp_context_init(&pp_ctx);

  ctx.ir = ir;
  ctx.config = config;
  ctx.pp_ctx = &pp_ctx;

  rc = extract_exports_recursive(filename, content, &ctx);
  if (rc == CDD_C_SUCCESS)
    rc = instantiate_templates(ir);

  for (k = 0; k < ctx.visited_count; k++) {
    C_CDD_FREE(ctx.visited[k]);
  }
  C_CDD_FREE(ctx.visited);
  rc = cdd_keep_err(rc, pp_context_free(&pp_ctx));

  if (rc != CDD_C_SUCCESS) {
    rc = cdd_keep_err(rc, cdd_ffi_ir_free(ir));
    C_CDD_FREE(ir);
    *out_ir = NULL;
    return rc;
  }

  *out_ir = ir;
  return CDD_C_SUCCESS;
}

#ifdef CDD_BUILD_TESTS
/**
 * @brief Test wrapper for `int64_to_str`.
 *
 * @param[in] val 64-bit integer value to format.
 * @param[in,out] buf Destination character buffer.
 * @param[in] buf_sz Capacity of `buf` in bytes.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_int64_to_str_test(int64_t val, char *buf,
                                                     size_t buf_sz) {
  return int64_to_str(val, buf, buf_sz);
}

/**
 * @brief Test wrapper for `extract_single_file_exports`.
 *
 * @param[in,out] ir Target FFI IR structure.
 * @param[in] filename Header filename.
 * @param[in] content Raw source text.
 * @param[in] config Optional generation configuration.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_extract_single_file_exports_test(
    cdd_ffi_ir_t *ir, const char *filename, const char *content,
    const cdd_generate_bindings_config_t *config) {
  return extract_single_file_exports(ir, filename, content, config);
}

/**
 * @brief Test wrapper for `is_visited`.
 *
 * @param[in,out] ctx Pointer to `struct IncludeMergeCtx`.
 * @param[in] path File path to check.
 * @param[out] out_visited Receives 1 if visited, 0 otherwise.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_is_visited_test(void *ctx, const char *path,
                                                   int *out_visited) {
  return is_visited((struct IncludeMergeCtx *)ctx, path, out_visited);
}

/**
 * @brief Test wrapper for `add_visited`.
 *
 * @param[in,out] ctx Pointer to `struct IncludeMergeCtx`.
 * @param[in] path File path to mark as visited.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_add_visited_test(void *ctx,
                                                    const char *path) {
  return add_visited((struct IncludeMergeCtx *)ctx, path);
}

/**
 * @brief Test wrapper for `include_visitor`.
 *
 * @param[in] info Pointer to `struct IncludeInfo`.
 * @param[in,out] user_data Pointer to `struct IncludeMergeCtx`.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_include_visitor_test(const void *info,
                                                        void *user_data) {
  return include_visitor((const struct IncludeInfo *)info, user_data);
}

/**
 * @brief Test wrapper for `extract_exports_recursive`.
 *
 * @param[in] filename Header filename.
 * @param[in] content Raw source text.
 * @param[in,out] ctx Pointer to `struct IncludeMergeCtx`.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_extract_exports_recursive_test(
    const char *filename, const char *content, void *ctx) {
  return extract_exports_recursive(filename, content,
                                   (struct IncludeMergeCtx *)ctx);
}

/**
 * @brief Test wrapper for `ir_add_node`.
 *
 * @param[in,out] ir Target FFI IR structure.
 * @param[in] kind Kind of node to add.
 * @param[in] name Node identifier.
 * @param[out] out_node Optional pointer to receive the added node.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_ffi_ir_add_node_test(cdd_ffi_ir_t *ir, cdd_ffi_node_kind_t kind,
                         const char *name, cdd_ffi_ir_node_t **out_node) {
  return ir_add_node(ir, kind, name, out_node);
}

/**
 * @brief Test wrapper for `cdd_ffi_malloc_impl`.
 *
 * @param[in] sz Allocation size in bytes.
 * @param[out] out_p Output pointer receiving the allocated memory.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_malloc_test(size_t sz, void **out_p) {
  if (!out_p)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_p = CDD_MALLOC(sz);
  return *out_p ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}

/**
 * @brief Test wrapper for `cdd_ffi_calloc_impl`.
 *
 * @param[in] n Number of elements.
 * @param[in] sz Size of each element in bytes.
 * @param[out] out_p Output pointer receiving the allocated memory.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_calloc_test(size_t n, size_t sz,
                                               void **out_p) {
  if (!out_p)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_p = CDD_CALLOC(n, sz);
  return *out_p ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}

/**
 * @brief Test wrapper for `cdd_ffi_realloc_impl`.
 *
 * @param[in,out] p Existing allocation pointer.
 * @param[in] sz New allocation size in bytes.
 * @param[out] out_p Output pointer receiving the reallocated memory.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_realloc_test(void *p, size_t sz,
                                                void **out_p) {
  if (!out_p)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_p = CDD_REALLOC(p, sz);
  return *out_p ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}

/**
 * @brief Test wrapper for `cdd_ffi_strdup_impl`.
 *
 * @param[in] s Source null-terminated string.
 * @param[out] out_s Output pointer receiving the duplicated string.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_strdup_test(const char *s, char **out_s) {
  if (!out_s)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_s = CDD_STRDUP(s);
  return *out_s ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}
#endif
