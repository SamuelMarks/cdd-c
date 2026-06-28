/* clang-format off */
#include "cdd_ffi_ir_extractor.h"
#include "../../classes/parse/inspector.h"
#include "../../classes/parse/cdd_cst_parser.h"
#include "../../classes/parse/cdd_cst_query.h"
#include "../../functions/parse/preprocessor.h"
#include "../../functions/parse/macro_evaluator.h"
#include "../../functions/parse/fs.h"
#include "c_cdd/format_specifiers.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#endif
/* clang-format on */

static enum cdd_c_error ir_add_node(cdd_ffi_ir_t *ir, cdd_ffi_node_kind_t kind,
                                    const char *name,
                                    cdd_ffi_ir_node_t **out_node) {
  cdd_ffi_ir_node_t *node;

  if (ir->nodes_count >= ir->nodes_capacity) {
    size_t new_cap = ir->nodes_capacity == 0 ? 16 : ir->nodes_capacity * 2;
    cdd_ffi_ir_node_t *new_nodes = (cdd_ffi_ir_node_t *)realloc(
        ir->nodes, new_cap * sizeof(cdd_ffi_ir_node_t));
    if (!new_nodes)
      return CDD_C_ERROR_MEMORY;
    ir->nodes = new_nodes;
    ir->nodes_capacity = new_cap;
  }

  node = &ir->nodes[ir->nodes_count++];
  memset(node, 0, sizeof(cdd_ffi_ir_node_t));
  node->kind = kind;
  node->name = strdup(name);
  if (!node->name)
    return CDD_C_ERROR_MEMORY;

  if (out_node) {
    *out_node = node;
  }
  return CDD_C_SUCCESS;
}

static enum cdd_c_error
map_c_type_to_ffi_kind(const char *c_type, cdd_ffi_primitive_kind_t *out_kind);

static enum cdd_c_error parse_template_type(const char *c_type,
                                            cdd_ffi_type_t *out_type) {
  const char *lt = strchr(c_type, '<');
  const char *gt = strrchr(c_type, '>');
  size_t base_len;
  char *base_name;
  char *inner_type_str;
  size_t inner_len;

  if (!lt || !gt || gt < lt) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  base_len = (size_t)(lt - c_type);
  base_name = (char *)malloc(base_len + 1);
  if (!base_name)
    return CDD_C_ERROR_MEMORY;
  strncpy(base_name, c_type, base_len);
  base_name[base_len] = '\0';

  out_type->ref_name = base_name;

  inner_len = (size_t)(gt - lt - 1);
  inner_type_str = (char *)malloc(inner_len + 1);
  if (!inner_type_str)
    return CDD_C_ERROR_MEMORY;
  strncpy(inner_type_str, lt + 1, inner_len);
  inner_type_str[inner_len] = '\0';

  /* For naive implementation, we just assume 1 template arg without commas for
   * now */
  out_type->template_args_count = 1;
  out_type->template_args = (cdd_ffi_type_t *)calloc(1, sizeof(cdd_ffi_type_t));
  if (!out_type->template_args) {
    free(inner_type_str);
    return CDD_C_ERROR_MEMORY;
  }

  {
    enum cdd_c_error rc;
    rc = map_c_type_to_ffi_kind(inner_type_str,
                                &out_type->template_args[0].kind);
    if (rc != CDD_C_SUCCESS) {
      free(inner_type_str);
      return rc;
    }
  }
  if (out_type->template_args[0].kind == CDD_FFI_KIND_STRUCT_REF ||
      out_type->template_args[0].kind == CDD_FFI_KIND_TEMPLATE_STRUCT_REF) {
    if (out_type->template_args[0].kind == CDD_FFI_KIND_TEMPLATE_STRUCT_REF) {
      parse_template_type(inner_type_str, &out_type->template_args[0]);
    } else {
      out_type->template_args[0].ref_name = strdup(inner_type_str);
    }
  }

  free(inner_type_str);
  return CDD_C_SUCCESS;
}

static enum cdd_c_error
map_c_type_to_ffi_kind(const char *c_type, cdd_ffi_primitive_kind_t *out_kind) {
  if (!out_kind)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (!c_type) {
    *out_kind = CDD_FFI_KIND_VOID;
    return CDD_C_SUCCESS;
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

  if (strstr(c_type, "int8_t") || strcmp(c_type, "char") == 0) {
    *out_kind = CDD_FFI_KIND_INT8;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "uint8_t") || strcmp(c_type, "unsigned char") == 0) {
    *out_kind = CDD_FFI_KIND_UINT8;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "int16_t") || strcmp(c_type, "short") == 0) {
    *out_kind = CDD_FFI_KIND_INT16;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "uint16_t") || strcmp(c_type, "unsigned short") == 0) {
    *out_kind = CDD_FFI_KIND_UINT16;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "int32_t") || strcmp(c_type, "int") == 0 ||
      strcmp(c_type, "integer") == 0) {
    *out_kind = CDD_FFI_KIND_INT32;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "uint32_t") || strcmp(c_type, "unsigned int") == 0) {
    *out_kind = CDD_FFI_KIND_UINT32;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "int64_t") || strcmp(c_type, "long long") == 0) {
    *out_kind = CDD_FFI_KIND_INT64;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "uint64_t") || strcmp(c_type, "unsigned long long") == 0) {
    *out_kind = CDD_FFI_KIND_UINT64;
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
  if (strstr(c_type, "bool") || strcmp(c_type, "boolean") == 0) {
    *out_kind = CDD_FFI_KIND_BOOL;
    return CDD_C_SUCCESS;
  }
  if (strstr(c_type, "void")) {
    *out_kind = CDD_FFI_KIND_VOID;
    return CDD_C_SUCCESS;
  }

  /* Fallback: if we don't know it, we assume it's a struct reference for now in
   * the naive mapper */
  *out_kind = CDD_FFI_KIND_STRUCT_REF;
  return CDD_C_SUCCESS;
}

C_CDD_EXPORT enum cdd_c_error cdd_ffi_mangle_cpp_name(const char *ns_name,
                                                      const char *class_name,
                                                      const char *method_name,
                                                      char **out_mangled) {
  size_t len = 0;
  char *mangled = NULL;

  if (!method_name || !out_mangled) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (ns_name)
    len += strlen(ns_name) + 1; /* +1 for '_' */
  if (class_name)
    len += strlen(class_name) + 1;
  len += strlen(method_name) + 1; /* +1 for '\0' */

  mangled = (char *)malloc(len);
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
  return 0;
}

static enum cdd_c_error
extract_single_file_exports(cdd_ffi_ir_t *ir, const char *filename,
                            const char *content,
                            const cdd_generate_bindings_config_t *config) {
  int rc = 0;
  size_t i;
  struct TypeDefList types;
  struct FuncSigList sigs;

  (void)config;

  rc = type_def_list_init(&types);
  if (rc == 0) {
    /* Extract Types */
    rc = c_inspector_scan_file_types(filename, &types);
    if (rc == 0) {
      for (i = 0; i < types.size; i++) {
        cdd_ffi_ir_node_t *node = NULL;
        if (types.items[i].kind == KIND_ENUM) {
          rc = ir_add_node(ir, CDD_FFI_NODE_ENUM, types.items[i].name, &node);
          if (rc == 0 && types.items[i].details.enum_members &&
              types.items[i].details.enum_members->size > 0) {
            size_t j;
            struct EnumMembers *em = types.items[i].details.enum_members;
            node->variants_count = em->size;
            node->variants = (cdd_ffi_enum_variant_t *)calloc(
                em->size, sizeof(cdd_ffi_enum_variant_t));
            if (!node->variants) {
              rc = CDD_C_ERROR_MEMORY;
              break;
            }
            for (j = 0; j < em->size; j++) {
              node->variants[j].name = strdup(em->members[j]);
              node->variants[j].value =
                  strdup(em->members[j]); /* Naive value */
              if (!node->variants[j].name || !node->variants[j].value) {
                rc = CDD_C_ERROR_MEMORY;
                break;
              }
            }
          }
          if (rc != 0)
            break;
        } else if (types.items[i].kind == KIND_STRUCT) {
          rc = ir_add_node(ir, CDD_FFI_NODE_STRUCT, types.items[i].name, &node);
          if (rc == 0) {
            /* Add parsing for base classes from CST */
            cdd_cst_tree_t *tree_base = NULL;
            cdd_cst_query_result_t structs_base = {0};
            az_span span_base;
            span_base = az_span_create_from_str((char *)content);
            if (cdd_cst_parse(span_base, &tree_base) == 0) {
              if (cdd_cst_find_nodes_by_type(tree_base->root,
                                             CDD_CST_CLASS_DECLARATION,
                                             &structs_base) == 0) {
                size_t si;
                for (si = 0; si < structs_base.size; si++) {
                  cdd_cst_node_t *s_node = structs_base.nodes[si];
                  /* Verify struct name matches */
                  cdd_token_t *name_tok = NULL;
                  size_t c_idx;
                  for (c_idx = 0; c_idx < s_node->num_children; c_idx++) {
                    if (s_node->children[c_idx].kind == CDD_CST_CHILD_TOKEN &&
                        s_node->children[c_idx].val.token->kind ==
                            CDD_TOKEN_IDENTIFIER) {
                      name_tok = s_node->children[c_idx].val.token;
                      break;
                    }
                  }
                  if (name_tok &&
                      name_tok->length == strlen(types.items[i].name) &&
                      strncmp((const char *)name_tok->start,
                              types.items[i].name, name_tok->length) == 0) {

                    /* Find CDD_CST_BASE_CLASS_LIST */
                    for (c_idx = 0; c_idx < s_node->num_children; c_idx++) {
                      if (s_node->children[c_idx].kind == CDD_CST_CHILD_NODE &&
                          s_node->children[c_idx].val.node->kind ==
                              CDD_CST_BASE_CLASS_LIST) {
                        cdd_cst_node_t *base_list =
                            s_node->children[c_idx].val.node;
                        size_t b_idx;
                        node->base_classes_count = 0;
                        for (b_idx = 0; b_idx < base_list->num_children;
                             b_idx++) {
                          if (base_list->children[b_idx].kind ==
                                  CDD_CST_CHILD_NODE &&
                              base_list->children[b_idx].val.node->kind ==
                                  CDD_CST_BASE_CLASS_SPECIFIER) {
                            node->base_classes_count++;
                          }
                        }
                        if (node->base_classes_count > 0) {
                          node->base_classes = (cdd_ffi_base_class_t *)calloc(
                              node->base_classes_count,
                              sizeof(cdd_ffi_base_class_t));
                          if (node->base_classes) {
                            size_t bi = 0;
                            for (b_idx = 0; b_idx < base_list->num_children;
                                 b_idx++) {
                              if (base_list->children[b_idx].kind ==
                                      CDD_CST_CHILD_NODE &&
                                  base_list->children[b_idx].val.node->kind ==
                                      CDD_CST_BASE_CLASS_SPECIFIER) {
                                cdd_cst_node_t *base_spec =
                                    base_list->children[b_idx].val.node;
                                size_t s_idx;
                                for (s_idx = 0; s_idx < base_spec->num_children;
                                     s_idx++) {
                                  if (base_spec->children[s_idx].kind ==
                                      CDD_CST_CHILD_TOKEN) {
                                    cdd_token_t *tok_val;
                                    tok_val =
                                        base_spec->children[s_idx].val.token;
                                    if (tok_val->kind ==
                                        CDD_TOKEN_KEYWORD_VIRTUAL) {
                                      node->base_classes[bi].is_virtual = 1;
                                    } else if (tok_val->kind ==
                                               CDD_TOKEN_IDENTIFIER) {
                                      size_t tok_len;
                                      tok_len = tok_val->length;
                                      node->base_classes[bi].name =
                                          (char *)malloc(tok_len + 1);
                                      if (node->base_classes[bi].name) {
                                        memcpy(node->base_classes[bi].name,
                                               (const char *)tok_val->start,
                                               tok_len);
                                        node->base_classes[bi].name[tok_len] =
                                            '\0';
                                      }
                                    }
                                  }
                                }
                                bi++;
                              }
                            }
                          }
                        }
                        break; /* Found base list */
                      }
                    }
                    break; /* Found matching struct */
                  }
                }
                if (structs_base.nodes)
                  free(structs_base.nodes);
              }
              cdd_cst_tree_free(tree_base);
            }
          }
          if (rc == 0 && types.items[i].details.struct_fields &&
              types.items[i].details.struct_fields->size > 0) {
            size_t j;
            struct StructFields *sf = types.items[i].details.struct_fields;
            node->fields_count = sf->size;
            node->fields =
                (cdd_ffi_field_t *)calloc(sf->size, sizeof(cdd_ffi_field_t));
            if (!node->fields) {
              rc = CDD_C_ERROR_MEMORY;
              break;
            }
            for (j = 0; j < sf->size; j++) {
              const char *target_c_type;
              node->fields[j].name = strdup(sf->fields[j].name);
              if (!node->fields[j].name) {
                rc = CDD_C_ERROR_MEMORY;
                break;
              }
              target_c_type = sf->fields[j].ref[0] != '\0' ? sf->fields[j].ref
                                                           : sf->fields[j].type;
              {
                enum cdd_c_error m_rc;
                m_rc = map_c_type_to_ffi_kind(target_c_type,
                                              &node->fields[j].type.kind);
                if (m_rc != CDD_C_SUCCESS) {
                  rc = m_rc;
                  break;
                }
              }
              /* If we defaulted to struct ref but it doesn't have ref_name,
               * let's use the raw type as ref_name */
              if (node->fields[j].type.kind == CDD_FFI_KIND_STRUCT_REF) {
                node->fields[j].type.ref_name = strdup(target_c_type);
                if (!node->fields[j].type.ref_name) {
                  rc = CDD_C_ERROR_MEMORY;
                  break;
                }
              } else if (node->fields[j].type.kind ==
                             CDD_FFI_KIND_TEMPLATE_STRUCT_REF ||
                         node->fields[j].type.kind == CDD_FFI_KIND_STD_VECTOR ||
                         node->fields[j].type.kind ==
                             CDD_FFI_KIND_STD_SHARED_PTR ||
                         node->fields[j].type.kind ==
                             CDD_FFI_KIND_STD_UNIQUE_PTR) {
                parse_template_type(target_c_type, &node->fields[j].type);
              }
            }
          }
          if (rc != 0)
            break;
        }
      }
    }
    type_def_list_free(&types);
  }

  if (rc == 0) {
    rc = func_sig_list_init(&sigs);
    if (rc == 0) {
      /* Extract Functions */
      rc = c_inspector_extract_signatures(content, &sigs);
      if (rc == 0) {
        for (i = 0; i < sigs.size; i++) {
          cdd_ffi_ir_node_t *node = NULL;
          rc =
              ir_add_node(ir, CDD_FFI_NODE_FUNCTION, sigs.items[i].name, &node);
          if (rc != 0)
            break;
          if (node) {
            /* Since inspector currently only extracts 'sig' and 'name', we just
             * map return_type to VOID for now. We also extract parameters here
             * naively from the sig string for intents. */
            node->return_or_base_type.kind = CDD_FFI_KIND_VOID;
            node->is_variadic = sigs.items[i].is_variadic;
            if (sigs.items[i].doc) {
              node->doc = strdup(sigs.items[i].doc);
              if (strstr(sigs.items[i].doc, "@ffi_release_gil") ||
                  strstr(sigs.items[i].doc, "@blocking")) {
                node->requires_gil_release = 1;
              }
            }
            if (sigs.items[i].sig) {
              /* Very naive parameter extraction just to populate fields and
               * intent */
              const char *lparen = strchr(sigs.items[i].sig, '(');
              const char *rparen = strrchr(sigs.items[i].sig, ')');
              if (lparen && rparen && rparen > lparen + 1) {
                char params_str[1024];
                char *p;
                char *ctx = NULL;
                size_t len = (size_t)(rparen - lparen - 1);
                if (len >= sizeof(params_str))
                  len = sizeof(params_str) - 1;
                strncpy(params_str, lparen + 1, len);
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
                  if (strcmp(param_trim, "void") != 0 &&
                      strcmp(param_trim, "...") != 0) {
                    cdd_ffi_field_t *new_fields;
                    size_t new_cap = node->fields_count + 1;
                    new_fields = (cdd_ffi_field_t *)realloc(
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
                      node->fields[node->fields_count].name = strdup(name);
                      node->fields[node->fields_count].type.kind =
                          CDD_FFI_KIND_INT32; /* default naive */
                      node->fields[node->fields_count].intent =
                          CDD_FFI_INTENT_UNKNOWN;

                      /* Parse intent from doc string or SAL */
                      if (node->doc) {
                        char search_out[128], search_in[128], search_inout[128];
                        sprintf(search_out, "@param[out] %s", name);
                        sprintf(search_in, "@param[in] %s", name);
                        sprintf(search_inout, "@param[in,out] %s", name);
                        if (strstr(node->doc, search_inout)) {
                          node->fields[node->fields_count].intent =
                              CDD_FFI_INTENT_INOUT;
                        } else if (strstr(node->doc, search_out)) {
                          node->fields[node->fields_count].intent =
                              CDD_FFI_INTENT_OUT;
                        } else if (strstr(node->doc, search_in)) {
                          node->fields[node->fields_count].intent =
                              CDD_FFI_INTENT_IN;
                        }
                      }
                      if (strstr(param_trim, "_Out_")) {
                        node->fields[node->fields_count].intent =
                            CDD_FFI_INTENT_OUT;
                      }
                      {
                        const char *writes =
                            strstr(param_trim, "_Out_writes_(");
                        if (writes) {
                          char len_buf[64] = {0};
                          const char *start = writes + 13;
                          const char *end = strchr(start, ')');
                          if (end && (end - start) < 63) {
                            strncpy(len_buf, start, end - start);
                            node->fields[node->fields_count].intent =
                                CDD_FFI_INTENT_OUT;
                            node->fields[node->fields_count].array_length_ref =
                                strdup(len_buf);
                          }
                        }
                      }
                      if (strstr(param_trim, "_Inout_"))
                        node->fields[node->fields_count].intent =
                            CDD_FFI_INTENT_INOUT;
                      if (strstr(param_trim, "_In_"))
                        node->fields[node->fields_count].intent =
                            CDD_FFI_INTENT_IN;

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
          }
        }
      }
      func_sig_list_free(&sigs);
    }
  }

  if (rc == 0) {
    /* Extract Macros as Constants */
    struct PreprocessorContext pp_ctx;
    if (pp_context_init(&pp_ctx) == 0) {
      if (pp_scan_defines(&pp_ctx, filename) == 0) {
        size_t j;
        for (j = 0; j < pp_ctx.macro_count; j++) {
          /* Only export object-like macros that have a value */
          if (!pp_ctx.macros[j].is_function_like && pp_ctx.macros[j].value &&
              strlen(pp_ctx.macros[j].value) > 0) {
            cdd_ffi_ir_node_t *node = NULL;
            rc = ir_add_node(ir, CDD_FFI_NODE_MACRO, pp_ctx.macros[j].name,
                             &node);
            if (rc == 0 && node) {
              cdd_macro_eval_result_t eval_res;
              node->variants_count = 1;
              node->variants = (cdd_ffi_enum_variant_t *)calloc(
                  1, sizeof(cdd_ffi_enum_variant_t));
              if (node->variants) {
                node->variants[0].name = strdup(pp_ctx.macros[j].name);
                node->variants[0].value = strdup(pp_ctx.macros[j].value);
              }
              if (cdd_macro_evaluate(&pp_ctx, pp_ctx.macros[j].value,
                                     &eval_res) == 0) {
                node->inferred_type = (cdd_ffi_macro_type_t)eval_res.type;
                if (eval_res.type == MACRO_EVAL_TYPE_INT) {
                  char buf[64];
#if defined(_MSC_VER)
                  sprintf_s(buf, sizeof(buf), "%" CDD_PRId64, eval_res.int_val);
#else
                  sprintf(buf, "%" CDD_PRId64, eval_res.int_val);
#endif
                  node->evaluated_value = strdup(buf);
                } else if (eval_res.type == MACRO_EVAL_TYPE_FLOAT) {
                  char buf[64];
#if defined(_MSC_VER)
                  sprintf_s(buf, sizeof(buf), "%f", eval_res.float_val);
#else
                  sprintf(buf, "%f", eval_res.float_val);
#endif
                  node->evaluated_value = strdup(buf);
                } else if (eval_res.type == MACRO_EVAL_TYPE_STRING) {
                  node->evaluated_value =
                      strdup(eval_res.str_val ? eval_res.str_val : "");
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
  }

  if (rc == 0) {
    /* Inject casting helpers for multiple inheritance */
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
          int is_virtual_cast = 0;

          if (ir->nodes[k].base_classes[b].is_virtual) {
            /* Diamond Inheritance Resolution:
               Virtual bases use dynamic table lookups in C++.
               The native pointer adjustment wrapper generated here ensures
               the C++ compiler emits the correct vtable lookup dynamic_cast
               rather than doing naive static pointer math. */
            is_virtual_cast = 1;
          }
          (void)is_virtual_cast;

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
          if (rc == 0 && upcast_node) {
            upcast_node->return_or_base_type.kind = CDD_FFI_KIND_STRUCT_REF;
            upcast_node->return_or_base_type.ref_name =
                strdup(ir->nodes[k].base_classes[b].name);
            upcast_node->fields_count = 1;
            upcast_node->fields =
                (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
            if (upcast_node->fields) {
              upcast_node->fields[0].name = strdup("ptr");
              upcast_node->fields[0].type.kind = CDD_FFI_KIND_STRUCT_REF;
              upcast_node->fields[0].type.ref_name = strdup(ir->nodes[k].name);
            }
          }

          rc =
              ir_add_node(ir, CDD_FFI_NODE_FUNCTION, down_name, &downcast_node);
          if (rc == 0 && downcast_node) {
            downcast_node->return_or_base_type.kind = CDD_FFI_KIND_STRUCT_REF;
            downcast_node->return_or_base_type.ref_name =
                strdup(ir->nodes[k].name);
            downcast_node->fields_count = 1;
            downcast_node->fields =
                (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
            if (downcast_node->fields) {
              downcast_node->fields[0].name = strdup("ptr");
              downcast_node->fields[0].type.kind = CDD_FFI_KIND_STRUCT_REF;
              downcast_node->fields[0].type.ref_name =
                  strdup(ir->nodes[k].base_classes[b].name);
            }
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
        if (rc == 0 && trampoline_node) {
          /* Add fields for target_lang_ctx, AddRef, Release, and one per
           * virtual method */
          trampoline_node->fields_count =
              ir->nodes[k].virtual_methods_count + 3;
          trampoline_node->fields = (cdd_ffi_field_t *)calloc(
              trampoline_node->fields_count, sizeof(cdd_ffi_field_t));
          if (trampoline_node->fields) {
            trampoline_node->fields[0].name = strdup("target_lang_ctx");
            trampoline_node->fields[0].type.kind = CDD_FFI_KIND_OPAQUE_PTR;

            trampoline_node->fields[1].name = strdup("cb_AddRef");
            trampoline_node->fields[1].type.kind = CDD_FFI_KIND_FUNCTION_PTR;

            trampoline_node->fields[2].name = strdup("cb_Release");
            trampoline_node->fields[2].type.kind = CDD_FFI_KIND_FUNCTION_PTR;

            for (m = 0; m < ir->nodes[k].virtual_methods_count; m++) {
              char cb_name[256];
#if defined(_MSC_VER)
              sprintf_s(cb_name, sizeof(cb_name), "cb_%s",
                        ir->nodes[k].virtual_methods[m].name);
#else
              sprintf(cb_name, "cb_%s", ir->nodes[k].virtual_methods[m].name);
#endif
              trampoline_node->fields[m + 3].name = strdup(cb_name);
              trampoline_node->fields[m + 3].type.kind =
                  CDD_FFI_KIND_FUNCTION_PTR;
            }
          }
        }
      }
    }
  }

  return rc;
}

struct IncludeMergeCtx {
  cdd_ffi_ir_t *ir;
  const cdd_generate_bindings_config_t *config;
  char **visited;
  size_t visited_count;
  size_t visited_capacity;
  int err;
  struct PreprocessorContext *pp_ctx;
};

static enum cdd_c_error is_visited(struct IncludeMergeCtx *ctx,
                                   const char *path) {
  size_t k;
  for (k = 0; k < ctx->visited_count; k++) {
    if (strcmp(ctx->visited[k], path) == 0)
      return CDD_C_ERROR_UNKNOWN;
  }
  return CDD_C_SUCCESS;
}

static enum cdd_c_error add_visited(struct IncludeMergeCtx *ctx,
                                    const char *path) {
  if (ctx->visited_count >= ctx->visited_capacity) {
    size_t new_cap =
        ctx->visited_capacity == 0 ? 16 : ctx->visited_capacity * 2;
    char **new_visited =
        (char **)realloc(ctx->visited, new_cap * sizeof(char *));
    if (!new_visited)
      return CDD_C_ERROR_MEMORY;
    ctx->visited = new_visited;
    ctx->visited_capacity = new_cap;
  }
  ctx->visited[ctx->visited_count++] = strdup(path);
  return CDD_C_SUCCESS;
}

static enum cdd_c_error include_visitor(const struct IncludeInfo *info,
                                        void *user_data);

static enum cdd_c_error extract_exports_recursive(const char *filename,
                                                  const char *content,
                                                  struct IncludeMergeCtx *ctx) {
  int rc;
  if (is_visited(ctx, filename))
    return CDD_C_SUCCESS;
  rc = add_visited(ctx, filename);
  if (rc != 0)
    return rc;

  rc = extract_single_file_exports(ctx->ir, filename, content, ctx->config);
  if (rc != 0)
    return rc;

  if (ctx->config && ctx->config->recursive_includes) {
    rc = pp_scan_includes(filename, ctx->pp_ctx, include_visitor, ctx);
  }
  return rc;
}

static enum cdd_c_error include_visitor(const struct IncludeInfo *info,
                                        void *user_data) {
  struct IncludeMergeCtx *ctx = (struct IncludeMergeCtx *)user_data;
  if (ctx->err != 0)
    return ctx->err;
  if (info->kind == PP_DIR_INCLUDE && info->resolved_path) {
    if (!is_visited(ctx, info->resolved_path)) {
      char *content = NULL;
      size_t sz = 0;
      int rc = read_to_file(info->resolved_path, "r", &content, &sz);
      if (rc == 0 && content) {
        ctx->err = extract_exports_recursive(info->resolved_path, content, ctx);
        free(content);
      }
    }
  }
  return CDD_C_SUCCESS;
}

static enum cdd_c_error instantiate_templates(cdd_ffi_ir_t *ir) {
  size_t i, j;
  size_t initial_count = ir->nodes_count;
  for (i = 0; i < initial_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT ||
        node->kind == CDD_FFI_NODE_FUNCTION) {
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
            const char *node_name = ir->nodes[k].name;
            if (!node_name)
              continue;
            if (strncmp(node_name, "struct ", 7) == 0)
              node_name += 7;
            else if (strncmp(node_name, "class ", 6) == 0)
              node_name += 6;

            if (strcmp(node_name, base_name) == 0) {
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
                }
              }
#if defined(_MSC_VER)
              sprintf_s(inst_name, sizeof(inst_name), "%s_%s", base_name,
                        arg_type_name);
#else
              sprintf(inst_name, "%s_%s", base_name, arg_type_name);
#endif

              for (m = 0; m < ir->nodes_count; m++) {
                if (ir->nodes[m].name &&
                    strcmp(ir->nodes[m].name, inst_name) == 0) {
                  already_inst = 1;
                  break;
                }
              }

              if (!already_inst) {
                ir_add_node(ir, CDD_FFI_NODE_STRUCT, inst_name, &new_node);
                base_struct = &ir->nodes[k];

                new_node->fields_count = base_struct->fields_count;
                new_node->fields = (cdd_ffi_field_t *)calloc(
                    new_node->fields_count, sizeof(cdd_ffi_field_t));
                if (new_node->fields) {
                  size_t f;
                  for (f = 0; f < new_node->fields_count; f++) {
                    new_node->fields[f].name =
                        strdup(base_struct->fields[f].name);
                    if (base_struct->fields[f].type.ref_name &&
                        strlen(base_struct->fields[f].type.ref_name) == 1) {
                      new_node->fields[f].type.kind =
                          ir->nodes[i].fields[j].type.template_args[0].kind;
                    } else {
                      new_node->fields[f].type.kind =
                          base_struct->fields[f].type.kind;
                      if (base_struct->fields[f].type.ref_name)
                        new_node->fields[f].type.ref_name =
                            strdup(base_struct->fields[f].type.ref_name);
                    }
                  }
                }
              }

              node = &ir->nodes[i];
              node->fields[j].type.kind = CDD_FFI_KIND_STRUCT_REF;
              free(node->fields[j].type.ref_name);
              node->fields[j].type.ref_name = strdup(inst_name);
              break;
            }
          }
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}

enum cdd_c_error
cdd_ffi_ir_extract_exports(const char *filename, const char *content,
                           const cdd_generate_bindings_config_t *config,
                           cdd_ffi_ir_t **out_ir) {
  cdd_ffi_ir_t *ir;
  struct IncludeMergeCtx ctx;
  struct PreprocessorContext pp_ctx;
  size_t k;
  int rc;

  if (!filename || !content || !out_ir) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  ir = (cdd_ffi_ir_t *)calloc(1, sizeof(cdd_ffi_ir_t));
  if (!ir) {
    return CDD_C_ERROR_MEMORY;
  }

  memset(&ctx, 0, sizeof(ctx));
  memset(&pp_ctx, 0, sizeof(pp_ctx));
  if (pp_context_init(&pp_ctx) != 0) {
    free(ir);
    return CDD_C_ERROR_MEMORY;
  }

  ctx.ir = ir;
  ctx.config = config;
  ctx.pp_ctx = &pp_ctx;

  rc = extract_exports_recursive(filename, content, &ctx);
  if (rc == 0 && ctx.err != 0) {
    rc = ctx.err;
  }

  if (rc == 0) {
    rc = instantiate_templates(ir);
  }

  for (k = 0; k < ctx.visited_count; k++) {
    free(ctx.visited[k]);
  }
  if (ctx.visited)
    free(ctx.visited);
  pp_context_free(&pp_ctx);

  if (rc != 0) {
    cdd_ffi_ir_free(ir);
    free(ir);
    return rc;
  }

  *out_ir = ir;
  return CDD_C_SUCCESS;
}
