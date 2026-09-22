/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_ir_extractor.h"
#include "../../classes/parse/cdd_cst_parser.h"
#include "../../classes/parse/cdd_cst_query.h"
#include "../../classes/parse/inspector.h"
#include "../../functions/parse/fs.h"
#include "../../functions/parse/macro_evaluator.h"
#include "../../functions/parse/preprocessor.h"
#include "c_cdd/format_specifiers.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef CDD_BUILD_TESTS
#include "c_cdd_export.h"
#endif
/* clang-format on */

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif
#endif

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_ffi_extractor_alloc_fail = 0;
C_CDD_EXPORT int g_cdd_ffi_fail_class_name = 0;
C_CDD_EXPORT int g_cdd_ffi_fail_is_visited = 0;
C_CDD_EXPORT int g_cdd_ffi_extractor_fail = 0;

static void *cdd_ffi_malloc_impl(size_t sz) {
  if (g_ffi_extractor_alloc_fail && --g_ffi_extractor_alloc_fail == 0)
    return NULL;
  return malloc(sz);
}

static void *cdd_ffi_calloc_impl(size_t n, size_t sz) {
  if (g_ffi_extractor_alloc_fail && --g_ffi_extractor_alloc_fail == 0)
    return NULL;
  return calloc(n, sz);
}

static void *cdd_ffi_realloc_impl(void *ptr, size_t sz) {
  if (g_ffi_extractor_alloc_fail && --g_ffi_extractor_alloc_fail == 0)
    return NULL;
  return realloc(ptr, sz);
}

static char *cdd_ffi_strdup_impl(const char *s) {
  if (!s)
    return NULL;
  if (g_ffi_extractor_alloc_fail && --g_ffi_extractor_alloc_fail == 0)
    return NULL;
  return strdup(s);
}

#define CDD_MALLOC(sz) cdd_ffi_malloc_impl(sz)
#define CDD_CALLOC(n, sz) cdd_ffi_calloc_impl((n), (sz))
#define CDD_REALLOC(ptr, sz) cdd_ffi_realloc_impl((ptr), (sz))
#define CDD_STRDUP(s) cdd_ffi_strdup_impl(s)
#else
#define CDD_MALLOC(sz) malloc(sz)
#define CDD_CALLOC(n, sz) calloc(n, sz)
#define CDD_REALLOC(ptr, sz) realloc(ptr, sz)
#define CDD_STRDUP(s) strdup(s)
#endif

/**
 * @brief Converts a 64-bit signed integer to a string.
 *
 * @param val Value to convert.
 * @param buf Output buffer.
 * @param buf_sz Size of output buffer.
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
 * @param ir Pointer to FFI IR structure.
 * @param kind Kind of node to add.
 * @param name Name of the node.
 * @param out_node Optional pointer to receive the created node.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
static cdd_c_error_t ir_add_node(cdd_ffi_ir_t *ir, cdd_ffi_node_kind_t kind,
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
 * @brief Maps a C type string to an FFI primitive kind.
 *
 * @param c_type C type string.
 * @param out_kind Destination primitive kind pointer.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
static cdd_c_error_t map_c_type_to_ffi_kind(const char *c_type,
                                            cdd_ffi_primitive_kind_t *out_kind);

/**
 * @brief Parses a C++ template type string into an FFI type representation.
 *
 * @param c_type C++ template type string.
 * @param out_type Destination FFI type pointer.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
static cdd_c_error_t parse_template_type(const char *c_type,
                                         cdd_ffi_type_t *out_type) {
  const char *lt;
  const char *gt;
  size_t base_len;
  char *base_name;
  char *inner_type_str;
  size_t inner_len;
  const char *inner_lt;
  const char *inner_gt;
  cdd_c_error_t rc;

  if (!c_type || !out_type) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  lt = strchr(c_type, '<');
  gt = strrchr(c_type, '>');

  if (!lt || !gt || gt < lt) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  base_len = (size_t)(lt - c_type);
  base_name = (char *)(size_t)CDD_MALLOC(base_len + 1);
  if (!base_name)
    return CDD_C_ERROR_MEMORY;
#if defined(_MSC_VER)
  strncpy_s(base_name, base_len + 1, c_type, base_len);
#else
  strncpy(base_name, c_type, base_len);
#endif
  base_name[base_len] = '\0';

  out_type->ref_name = base_name;

  inner_len = (size_t)(gt - lt - 1);
  inner_type_str = (char *)(size_t)CDD_MALLOC(inner_len + 1);
  if (!inner_type_str) {
    free(base_name);
    out_type->ref_name = NULL;
    return CDD_C_ERROR_MEMORY;
  }
#if defined(_MSC_VER)
  strncpy_s(inner_type_str, inner_len + 1, lt + 1, inner_len);
#else
  strncpy(inner_type_str, lt + 1, inner_len);
#endif
  inner_type_str[inner_len] = '\0';

  out_type->template_args_count = 1;
  out_type->template_args =
      (cdd_ffi_type_t *)CDD_CALLOC(1, sizeof(cdd_ffi_type_t));
  if (!out_type->template_args) {
    free(inner_type_str);
    free(base_name);
    out_type->ref_name = NULL;
    return CDD_C_ERROR_MEMORY;
  }

  rc = map_c_type_to_ffi_kind(inner_type_str, &out_type->template_args[0].kind);
  if (rc != CDD_C_SUCCESS) {
    free(inner_type_str);
    free(base_name);
    return rc;
  }
  if (out_type->template_args[0].kind == CDD_FFI_KIND_STRUCT_REF ||
      out_type->template_args[0].kind == CDD_FFI_KIND_TEMPLATE_STRUCT_REF) {
    inner_lt = strchr(inner_type_str, '<');
    inner_gt = strrchr(inner_type_str, '>');
    if (inner_lt && inner_gt && inner_gt > inner_lt) {
      cdd_c_error_t rc_ex =
          parse_template_type(inner_type_str, &out_type->template_args[0]);
      if (rc_ex != CDD_C_SUCCESS) {
        free(inner_type_str);
        return rc_ex;
      }
    } else {
      out_type->template_args[0].ref_name = CDD_STRDUP(inner_type_str);
      if (!out_type->template_args[0].ref_name) {
        free(inner_type_str);
        return CDD_C_ERROR_MEMORY;
      }
    }
  }

  free(inner_type_str);
  return CDD_C_SUCCESS;
}

static cdd_c_error_t
map_c_type_to_ffi_kind(const char *c_type, cdd_ffi_primitive_kind_t *out_kind) {
  if (!out_kind)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (!c_type) {
    *out_kind = CDD_FFI_KIND_VOID;
    return CDD_C_SUCCESS;
  }

  if (c_type[0] == '\0') {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (strstr(c_type, "std::string") || strstr(c_type, "std_string")) {
    *out_kind = CDD_FFI_KIND_STD_STRING;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "std::vector") || strstr(c_type, "std_vector")) {
    *out_kind = CDD_FFI_KIND_STD_VECTOR;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "std::shared_ptr") || strstr(c_type, "std_shared_ptr")) {
    *out_kind = CDD_FFI_KIND_STD_SHARED_PTR;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "std::unique_ptr") || strstr(c_type, "std_unique_ptr")) {
    *out_kind = CDD_FFI_KIND_STD_UNIQUE_PTR;
    return CDD_C_SUCCESS;
  }

  if (strchr(c_type, '<') && strchr(c_type, '>')) {
    *out_kind = CDD_FFI_KIND_TEMPLATE_STRUCT_REF;
    return CDD_C_SUCCESS;
  }

  if (strstr(c_type, "uint8_t") || strcmp(c_type, "unsigned char") == 0) {
    *out_kind = CDD_FFI_KIND_UINT8;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "int8_t") || strcmp(c_type, "char") == 0) {
    *out_kind = CDD_FFI_KIND_INT8;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "uint16_t") || strcmp(c_type, "unsigned short") == 0) {
    *out_kind = CDD_FFI_KIND_UINT16;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "int16_t") || strcmp(c_type, "short") == 0) {
    *out_kind = CDD_FFI_KIND_INT16;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "uint32_t") || strcmp(c_type, "unsigned int") == 0) {
    *out_kind = CDD_FFI_KIND_UINT32;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "int32_t") || strcmp(c_type, "int") == 0 ||
      strcmp(c_type, "integer") == 0) {
    *out_kind = CDD_FFI_KIND_INT32;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "uint64_t") || strcmp(c_type, "unsigned long long") == 0) {
    *out_kind = CDD_FFI_KIND_UINT64;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "int64_t") || strcmp(c_type, "long long") == 0) {
    *out_kind = CDD_FFI_KIND_INT64;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "float")) {
    *out_kind = CDD_FFI_KIND_FLOAT32;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "double") || strcmp(c_type, "number") == 0) {
    *out_kind = CDD_FFI_KIND_FLOAT64;
    return CDD_C_SUCCESS;
  }
  if (strcmp(c_type, "boolean") == 0 || strstr(c_type, "bool")) {
    *out_kind = CDD_FFI_KIND_BOOL;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "void")) {
    *out_kind = CDD_FFI_KIND_VOID;
    return CDD_C_SUCCESS;
  }

  *out_kind = CDD_FFI_KIND_STRUCT_REF;
  return CDD_C_SUCCESS;
}

C_CDD_EXPORT cdd_c_error_t cdd_ffi_mangle_cpp_name(const char *ns_name,
                                                   const char *class_name,
                                                   const char *method_name,
                                                   char **out_mangled) {
  size_t len = 0;
  char *mangled = NULL;

  if (!method_name || !out_mangled) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (ns_name)
    len += strlen(ns_name) + 1;
  if (class_name)
    len += strlen(class_name) + 1;
  len += strlen(method_name) + 1;

  mangled = (char *)(size_t)CDD_MALLOC(len);
  if (!mangled)
    return CDD_C_ERROR_MEMORY;

#if defined(_MSC_VER)
  if (ns_name && class_name) {
    sprintf_s(mangled, len, "%s_%s_%s", ns_name, class_name, method_name);
  } else if (ns_name) {
    sprintf_s(mangled, len, "%s_%s", ns_name, method_name);
  } else if (class_name) {
    sprintf_s(mangled, len, "%s_%s", class_name, method_name);
  } else {
    strcpy_s(mangled, len, method_name);
  }
#else
  if (ns_name && class_name) {
    sprintf(mangled, "%s_%s_%s", ns_name, class_name, method_name);
  } else if (ns_name) {
    sprintf(mangled, "%s_%s", ns_name, method_name);
  } else if (class_name) {
    sprintf(mangled, "%s_%s", class_name, method_name);
  } else {
    strcpy(mangled, method_name);
  }
#endif

  *out_mangled = mangled;
  return CDD_C_SUCCESS;
}

/**
 * @brief Extracts exported functions, types, and macros from a single file.
 *
 * @param ir Pointer to FFI IR structure.
 * @param filename File name.
 * @param content File contents string.
 * @param config Configuration options.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
static cdd_c_error_t
extract_single_file_exports(cdd_ffi_ir_t *ir, const char *filename,
                            const char *content,
                            const cdd_generate_bindings_config_t *config) {
  cdd_c_error_t rc;
  size_t i;
  struct TypeDefList types;
  struct FuncSigList sigs;
  (void)config;

  if (!ir || !filename || !content)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  memset(&types, 0, sizeof(types));

  /* Extract Types */
  rc = c_inspector_scan_file_types(filename, &types);
  if (rc != CDD_C_SUCCESS) {
    type_def_list_free(&types);
    return rc;
  }

  for (i = 0; i < types.size; i++) {
    cdd_ffi_ir_node_t *node = NULL;
    if (types.items[i].kind == KIND_ENUM) {
      rc = ir_add_node(ir, CDD_FFI_NODE_ENUM, types.items[i].name, &node);
      if (rc != CDD_C_SUCCESS)
        break;
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
      /* Add parsing for base classes from CST */
      {
        cdd_cst_tree_t *tree_base = NULL;
        cdd_cst_query_result_t structs_base = {0};
        size_t si;
        az_span span_base;
        span_base = az_span_create_from_str((char *)(size_t)content);
        if (cdd_cst_parse(span_base, &tree_base) == 0) {
          rc = cdd_cst_find_nodes_by_type(
              tree_base->root, CDD_CST_CLASS_DECLARATION, &structs_base);
#ifdef CDD_BUILD_TESTS
          if (g_cdd_ffi_extractor_fail == 1)
            rc = CDD_C_ERROR_MEMORY;
#endif
          if (rc != CDD_C_SUCCESS) {
            cdd_cst_tree_free(tree_base);
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
                            if (bname) {
                              memcpy(bname, tok->start, tok->length);
                              bname[tok->length] = '\0';
                              node->base_classes[bi].name = bname;
                            }
                          }
                        }
                        bi++;
                      }
                    }
                  }
                  break;
                }
              }
            }
          }
          free(structs_base.nodes);
          cdd_cst_tree_free(tree_base);
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
          free(node->fields);
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
              free(node->fields[j].type.ref_name);
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
  type_def_list_free(&types);

  if (rc != CDD_C_SUCCESS)
    return rc;

  /* Extract Functions */
  memset(&sigs, 0, sizeof(sigs));

  rc = c_inspector_extract_signatures(content, &sigs);
  if (rc != CDD_C_SUCCESS) {
    func_sig_list_free(&sigs);
    return rc;
  }

  for (i = 0; i < sigs.size; i++) {
    cdd_ffi_ir_node_t *node = NULL;
    const char *lparen;
    const char *rparen;
    rc = ir_add_node(ir, CDD_FFI_NODE_FUNCTION, sigs.items[i].name, &node);
    if (rc != CDD_C_SUCCESS)
      break;

    node->return_or_base_type.kind = CDD_FFI_KIND_VOID;
    node->is_variadic = sigs.items[i].is_variadic;
    if (sigs.items[i].doc) {
      node->doc = CDD_STRDUP(sigs.items[i].doc);
      if (strstr(sigs.items[i].doc, "@ffi_release_gil") ||
          strstr(sigs.items[i].doc, "@blocking")) {
        node->requires_gil_release = 1;
      }
    }
    lparen = strchr(sigs.items[i].sig, '(');
    rparen = strrchr(sigs.items[i].sig, ')');
    if (rparen > lparen + 1) {
      char params_str[1024];
      char *p;
      char *ctx = NULL;
      size_t len = (size_t)(rparen - lparen - 1);
      if (len >= sizeof(params_str))
        len = sizeof(params_str) - 1;
#if defined(_MSC_VER)
      strncpy_s(params_str, sizeof(params_str), lparen + 1, len);
#else
      strncpy(params_str, lparen + 1, len);
#endif
      params_str[len] = '\0';

#if defined(_MSC_VER)
      p = strtok_s(params_str, ",", &ctx);
#else
      p = strtok_r(params_str, ",", &ctx);
#endif
      while (p) {
        char *param_trim = p;
        while (*param_trim == ' ')
          param_trim++;
        if (strcmp(param_trim, "void") != 0 && strcmp(param_trim, "...") != 0) {
          cdd_ffi_field_t *new_fields;
          size_t new_cap = node->fields_count + 1;
          new_fields = (cdd_ffi_field_t *)CDD_REALLOC(
              node->fields, new_cap * sizeof(cdd_ffi_field_t));
          if (new_fields) {
            char *last_space = strrchr(param_trim, ' ');
            char *name = last_space ? last_space + 1 : param_trim;
            char *astx = strchr(name, '*');
            if (astx)
              name = astx + 1;

            node->fields = new_fields;
            memset(&node->fields[node->fields_count], 0,
                   sizeof(cdd_ffi_field_t));
            node->fields[node->fields_count].name = CDD_STRDUP(name);
            node->fields[node->fields_count].type.kind = CDD_FFI_KIND_INT32;
            node->fields[node->fields_count].intent = CDD_FFI_INTENT_UNKNOWN;

            if (node->doc) {
              char search_out[128], search_in[128], search_inout[128];
#if defined(_MSC_VER)
              sprintf_s(search_out, sizeof(search_out), "@param[out] %s", name);
              sprintf_s(search_in, sizeof(search_in), "@param[in] %s", name);
              sprintf_s(search_inout, sizeof(search_inout), "@param[in,out] %s",
                        name);
#else
              sprintf(search_out, "@param[out] %s", name);
              sprintf(search_in, "@param[in] %s", name);
              sprintf(search_inout, "@param[in,out] %s", name);
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
                const char *end = strchr(start, ')');
                if (end && (end - start) < 63) {
#if defined(_MSC_VER)
                  strncpy_s(len_buf, sizeof(len_buf), start,
                            (size_t)(end - start));
#else
                  strncpy(len_buf, start, (size_t)(end - start));
#endif
                  node->fields[node->fields_count].intent = CDD_FFI_INTENT_OUT;
                  node->fields[node->fields_count].array_length_ref =
                      CDD_STRDUP(len_buf);
                }
              }
            }
            if (strstr(param_trim, "_Inout_"))
              node->fields[node->fields_count].intent = CDD_FFI_INTENT_INOUT;
            if (strstr(param_trim, "_In_"))
              node->fields[node->fields_count].intent = CDD_FFI_INTENT_IN;

            node->fields_count++;
          }
        }
#if defined(_MSC_VER)
        p = strtok_s(NULL, ",", &ctx);
#else
        p = strtok_r(NULL, ",", &ctx);
#endif
      }
    }
  }
  func_sig_list_free(&sigs);

  if (rc != CDD_C_SUCCESS)
    return rc;

  /* Extract Macros as Constants */
  {
    struct PreprocessorContext pp_ctx;
    rc = pp_context_init(&pp_ctx);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = pp_scan_defines(&pp_ctx, filename);
    if (rc != CDD_C_SUCCESS) {
      pp_context_free(&pp_ctx);
      return rc;
    }
    {
      size_t j;
      for (j = 0; j < pp_ctx.macro_count; j++) {
        if (!pp_ctx.macros[j].is_function_like && pp_ctx.macros[j].value) {
          cdd_ffi_ir_node_t *node = NULL;
          rc =
              ir_add_node(ir, CDD_FFI_NODE_MACRO, pp_ctx.macros[j].name, &node);
          if (rc != CDD_C_SUCCESS)
            break;
          {
            cdd_macro_eval_result_t eval_res;
            node->variants_count = 1;
            node->variants = (cdd_ffi_enum_variant_t *)CDD_CALLOC(
                1, sizeof(cdd_ffi_enum_variant_t));
            if (!node->variants) {
              rc = CDD_C_ERROR_MEMORY;
              break;
            }
            node->variants[0].name = CDD_STRDUP(pp_ctx.macros[j].name);
            node->variants[0].value = CDD_STRDUP(pp_ctx.macros[j].value);
            if (cdd_macro_evaluate(&pp_ctx, pp_ctx.macros[j].value,
                                   &eval_res) == 0) {
              node->inferred_type = (cdd_ffi_macro_type_t)eval_res.type;
              if (eval_res.type == MACRO_EVAL_TYPE_INT) {
                char buf[64];
                rc = int64_to_str(eval_res.int_val, buf, sizeof(buf));
#ifdef CDD_BUILD_TESTS
                if (g_cdd_ffi_extractor_fail == 5)
                  rc = CDD_C_ERROR_UNKNOWN;
#endif
                if (rc != CDD_C_SUCCESS)
                  break;
                node->evaluated_value = CDD_STRDUP(buf);
              } else if (eval_res.type == MACRO_EVAL_TYPE_FLOAT) {
                char buf[64];
#if defined(_MSC_VER)
                sprintf_s(buf, sizeof(buf), "%f", eval_res.float_val);
#else
                sprintf(buf, "%f", eval_res.float_val);
#endif
                node->evaluated_value = CDD_STRDUP(buf);
              } else {
                node->evaluated_value = CDD_STRDUP(eval_res.str_val);
              }
              cdd_macro_eval_result_free(&eval_res);
            } else {
              node->inferred_type = CDD_FFI_MACRO_TYPE_UNKNOWN;
              node->evaluated_value = NULL;
            }
          }
        }
      }
    }
    pp_context_free(&pp_ctx);
  }

  if (rc != CDD_C_SUCCESS)
    return rc;

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

          if (!ir->nodes[k].base_classes[b].name)
            continue;

#if defined(_MSC_VER)
          sprintf_s(up_name, sizeof(up_name), "%s_upcast_to_%s",
                    ir->nodes[k].name, ir->nodes[k].base_classes[b].name);
          sprintf_s(down_name, sizeof(down_name), "%s_downcast_to_%s",
                    ir->nodes[k].base_classes[b].name, ir->nodes[k].name);
#else
          sprintf(up_name, "%s_upcast_to_%s", ir->nodes[k].name,
                  ir->nodes[k].base_classes[b].name);
          sprintf(down_name, "%s_downcast_to_%s",
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
        size_t m;

#if defined(_MSC_VER)
        sprintf_s(tramp_name, sizeof(tramp_name), "%s_Trampoline",
                  ir->nodes[k].name);
#else
        sprintf(tramp_name, "%s_Trampoline", ir->nodes[k].name);
#endif

        rc = ir_add_node(ir, CDD_FFI_NODE_STRUCT, tramp_name, &trampoline_node);
        if (rc != CDD_C_SUCCESS)
          return rc;

        trampoline_node->fields_count = ir->nodes[k].virtual_methods_count + 3;
        trampoline_node->fields = (cdd_ffi_field_t *)CDD_CALLOC(
            trampoline_node->fields_count, sizeof(cdd_ffi_field_t));
        if (!trampoline_node->fields) {
          return CDD_C_ERROR_MEMORY;
        }

        trampoline_node->fields[0].name = CDD_STRDUP("target_lang_ctx");
        trampoline_node->fields[0].type.kind = CDD_FFI_KIND_OPAQUE_PTR;

        trampoline_node->fields[1].name = CDD_STRDUP("cb_AddRef");
        trampoline_node->fields[1].type.kind = CDD_FFI_KIND_FUNCTION_PTR;

        trampoline_node->fields[2].name = CDD_STRDUP("cb_Release");
        trampoline_node->fields[2].type.kind = CDD_FFI_KIND_FUNCTION_PTR;

        for (m = 0; m < ir->nodes[k].virtual_methods_count; m++) {
          char cb_name[256];
#if defined(_MSC_VER)
          sprintf_s(cb_name, sizeof(cb_name), "cb_%s",
                    ir->nodes[k].virtual_methods[m].name);
#else
          sprintf(cb_name, "cb_%s", ir->nodes[k].virtual_methods[m].name);
#endif
          trampoline_node->fields[m + 3].name = CDD_STRDUP(cb_name);
          trampoline_node->fields[m + 3].type.kind = CDD_FFI_KIND_FUNCTION_PTR;
        }
      }
    }
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
 * @param ctx Pointer to merge context.
 * @param path File path.
 * @param out_visited Pointer to receive 1 if visited, 0 otherwise.
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
 * @param ctx Pointer to merge context.
 * @param path File path.
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
 * @param filename File name.
 * @param content File contents string.
 * @param ctx Pointer to merge context.
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
 * @param info Include directive information.
 * @param user_data Pointer to merge context.
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
        free(content);
        if (ext_rc != CDD_C_SUCCESS) {
          ctx->err = ext_rc;
          return ext_rc;
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Instantiates templated types discovered in the FFI IR.
 *
 * @param ir Pointer to FFI IR structure.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
static cdd_c_error_t instantiate_templates(cdd_ffi_ir_t *ir) {
  size_t i, j;
  size_t initial_count;
  if (!ir)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  initial_count = ir->nodes_count;
  for (i = 0; i < initial_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if ((node->kind == CDD_FFI_NODE_STRUCT ||
         node->kind == CDD_FFI_NODE_FUNCTION) &&
        node->fields) {
      for (j = 0; j < node->fields_count; j++) {
        if (node->fields[j].type.kind == CDD_FFI_KIND_TEMPLATE_STRUCT_REF) {
          size_t k;
          const char *base_name = node->fields[j].type.ref_name;
          if (!base_name)
            continue;
          if (strncmp(base_name, "struct ", 7) == 0)
            base_name += 7;
          else if (strncmp(base_name, "class ", 6) == 0)
            base_name += 6;

          for (k = 0; k < initial_count; k++) {
            if (strcmp(ir->nodes[k].name, base_name) == 0) {
              char inst_name[256];
              const char *arg_type_name = "unknown";
              size_t m;
              int already_inst = 0;
              cdd_ffi_ir_node_t *new_node = NULL;
              cdd_ffi_ir_node_t *base_struct;

              if (node->fields[j].type.template_args_count > 0) {
                if (node->fields[j].type.template_args[0].kind ==
                    CDD_FFI_KIND_INT32) {
                  arg_type_name = "int";
                } else if (node->fields[j].type.template_args[0].kind ==
                           CDD_FFI_KIND_FLOAT64) {
                  arg_type_name = "double";
                } else if (node->fields[j].type.template_args[0].kind ==
                           CDD_FFI_KIND_FLOAT32) {
                  arg_type_name = "float";
                } else if (node->fields[j].type.template_args[0].ref_name) {
                  arg_type_name =
                      node->fields[j].type.template_args[0].ref_name;
                }
              }
#if defined(_MSC_VER)
              sprintf_s(inst_name, sizeof(inst_name), "%s_%s", base_name,
                        arg_type_name);
#else
              sprintf(inst_name, "%s_%s", base_name, arg_type_name);
#endif

              for (m = 0; m < ir->nodes_count; m++) {
                if (strcmp(ir->nodes[m].name, inst_name) == 0) {
                  already_inst = 1;
                  break;
                }
              }

              if (!already_inst) {
                cdd_c_error_t rc_ex =
                    ir_add_node(ir, CDD_FFI_NODE_STRUCT, inst_name, &new_node);
                if (rc_ex != CDD_C_SUCCESS)
                  return rc_ex;

                base_struct = &ir->nodes[k];

                new_node->fields_count = base_struct->fields_count;
                if (new_node->fields_count > 0) {
                  new_node->fields = (cdd_ffi_field_t *)CDD_CALLOC(
                      new_node->fields_count, sizeof(cdd_ffi_field_t));
                  if (!new_node->fields) {
                    return CDD_C_ERROR_MEMORY;
                  }
                  {
                    size_t f;
                    for (f = 0; f < new_node->fields_count; f++) {
                      new_node->fields[f].name =
                          CDD_STRDUP(base_struct->fields[f].name);
                      if (base_struct->fields[f].type.ref_name &&
                          strlen(base_struct->fields[f].type.ref_name) == 1) {
                        new_node->fields[f].type.kind =
                            ir->nodes[i].fields[j].type.template_args[0].kind;
                      } else {
                        new_node->fields[f].type.kind =
                            base_struct->fields[f].type.kind;
                        if (base_struct->fields[f].type.ref_name)
                          new_node->fields[f].type.ref_name =
                              CDD_STRDUP(base_struct->fields[f].type.ref_name);
                      }
                    }
                  }
                }
              }

              node = &ir->nodes[i];
              node->fields[j].type.kind = CDD_FFI_KIND_STRUCT_REF;
              free(node->fields[j].type.ref_name);
              node->fields[j].type.ref_name = CDD_STRDUP(inst_name);
              break;
            }
          }
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}

cdd_c_error_t
cdd_ffi_ir_extract_exports(const char *filename, const char *content,
                           const cdd_generate_bindings_config_t *config,
                           cdd_ffi_ir_t **out_ir) {
  cdd_ffi_ir_t *ir;
  struct IncludeMergeCtx ctx;
  struct PreprocessorContext pp_ctx;
  size_t k;
  cdd_c_error_t rc;
  (void)config;

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
  if (rc != CDD_C_SUCCESS) {
    cdd_ffi_ir_free(ir);
    return rc;
  }

  ctx.ir = ir;
  ctx.config = config;
  ctx.pp_ctx = &pp_ctx;

  rc = extract_exports_recursive(filename, content, &ctx);
  if (rc == CDD_C_SUCCESS)
    rc = instantiate_templates(ir);

  for (k = 0; k < ctx.visited_count; k++) {
    free(ctx.visited[k]);
  }
  free(ctx.visited);
  pp_context_free(&pp_ctx);

  if (rc != CDD_C_SUCCESS) {
    cdd_ffi_ir_free(ir);
    free(ir);
    *out_ir = NULL;
    return rc;
  }

  *out_ir = ir;
  return CDD_C_SUCCESS;
}

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT cdd_c_error_t cdd_ffi_int64_to_str_test(int64_t val, char *buf,
                                                     size_t buf_sz) {
  return int64_to_str(val, buf, buf_sz);
}

C_CDD_EXPORT cdd_c_error_t cdd_ffi_map_c_type_to_ffi_kind_test(
    const char *c_type, cdd_ffi_primitive_kind_t *out_kind) {
  return map_c_type_to_ffi_kind(c_type, out_kind);
}

C_CDD_EXPORT cdd_c_error_t
cdd_ffi_parse_template_type_test(const char *c_type, cdd_ffi_type_t *out_type) {
  return parse_template_type(c_type, out_type);
}

C_CDD_EXPORT cdd_c_error_t cdd_ffi_extract_single_file_exports_test(
    cdd_ffi_ir_t *ir, const char *filename, const char *content,
    const cdd_generate_bindings_config_t *config) {
  return extract_single_file_exports(ir, filename, content, config);
}

C_CDD_EXPORT cdd_c_error_t cdd_ffi_is_visited_test(void *ctx, const char *path,
                                                   int *out_visited) {
  return is_visited((struct IncludeMergeCtx *)ctx, path, out_visited);
}

C_CDD_EXPORT cdd_c_error_t cdd_ffi_add_visited_test(void *ctx,
                                                    const char *path) {
  return add_visited((struct IncludeMergeCtx *)ctx, path);
}

C_CDD_EXPORT cdd_c_error_t cdd_ffi_include_visitor_test(const void *info,
                                                        void *user_data) {
  return include_visitor((const struct IncludeInfo *)info, user_data);
}

C_CDD_EXPORT cdd_c_error_t cdd_ffi_extract_exports_recursive_test(
    const char *filename, const char *content, void *ctx) {
  return extract_exports_recursive(filename, content,
                                   (struct IncludeMergeCtx *)ctx);
}

C_CDD_EXPORT cdd_c_error_t
cdd_ffi_instantiate_templates_test(cdd_ffi_ir_t *ir) {
  return instantiate_templates(ir);
}

C_CDD_EXPORT cdd_c_error_t
cdd_ffi_ir_add_node_test(cdd_ffi_ir_t *ir, cdd_ffi_node_kind_t kind,
                         const char *name, cdd_ffi_ir_node_t **out_node) {
  return ir_add_node(ir, kind, name, out_node);
}

C_CDD_EXPORT cdd_c_error_t cdd_ffi_malloc_test(size_t sz, void **out_p) {
  if (!out_p)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_p = cdd_ffi_malloc_impl(sz);
  return *out_p ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}

C_CDD_EXPORT cdd_c_error_t cdd_ffi_calloc_test(size_t n, size_t sz,
                                               void **out_p) {
  if (!out_p)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_p = cdd_ffi_calloc_impl(n, sz);
  return *out_p ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}

C_CDD_EXPORT cdd_c_error_t cdd_ffi_realloc_test(void *p, size_t sz,
                                                void **out_p) {
  if (!out_p)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_p = cdd_ffi_realloc_impl(p, sz);
  return *out_p ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}

C_CDD_EXPORT cdd_c_error_t cdd_ffi_strdup_test(const char *s, char **out_s) {
  if (!out_s)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_s = cdd_ffi_strdup_impl(s);
  return *out_s ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}
#endif
