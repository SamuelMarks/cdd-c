/* clang-format off */
#include "cdd_cst_type_eval.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "c_cdd/log.h"
/* clang-format on */

enum cdd_c_error cdd_cst_eval_primitive_type(const char *type_name,
                                             enum cdd_cst_abi_model_t abi,
                                             cdd_cst_type_info_t *out_info) {
  if (!type_name || !out_info)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (strcmp(type_name, "char") == 0 || strcmp(type_name, "signed char") == 0 ||
      strcmp(type_name, "unsigned char") == 0) {
    out_info->size = 1;
    out_info->alignment = 1;
    return CDD_C_SUCCESS;
  }

  if (strcmp(type_name, "short") == 0 ||
      strcmp(type_name, "unsigned short") == 0) {
    out_info->size = 2;
    out_info->alignment = 2;
    return CDD_C_SUCCESS;
  }

  if (strcmp(type_name, "int") == 0 || strcmp(type_name, "unsigned int") == 0) {
    out_info->size = 4;
    out_info->alignment = 4;
    return CDD_C_SUCCESS;
  }

  if (strcmp(type_name, "long") == 0 ||
      strcmp(type_name, "unsigned long") == 0) {
    if (abi == CDD_CST_ABI_ILP32 || abi == CDD_CST_ABI_LLP64) {
      out_info->size = 4;
      out_info->alignment = 4;
    } else { /* LP64 */
      out_info->size = 8;
      out_info->alignment = 8;
    }
    return CDD_C_SUCCESS;
  }

  if (strcmp(type_name, "long long") == 0 ||
      strcmp(type_name, "unsigned long long") == 0) {
    out_info->size = 8;
    out_info->alignment = 8;
    return CDD_C_SUCCESS;
  }

  if (strcmp(type_name, "float") == 0) {
    out_info->size = 4;
    out_info->alignment = 4;
    return CDD_C_SUCCESS;
  }

  if (strcmp(type_name, "double") == 0) {
    out_info->size = 8;
    out_info->alignment = 8;
    return CDD_C_SUCCESS;
  }

  if (strcmp(type_name, "long double") == 0) {
    if (abi == CDD_CST_ABI_ILP32) {
      out_info->size = 8;
      out_info->alignment = 4;
    } else if (abi == CDD_CST_ABI_LLP64) {
      out_info->size = 8;
      out_info->alignment = 8; /* usually on msvc */
    } else {
      out_info->size = 16;
      out_info->alignment = 16;
    }
    return CDD_C_SUCCESS;
  }

  if (strcmp(type_name, "void *") == 0 || strcmp(type_name, "ptr") == 0) {
    if (abi == CDD_CST_ABI_ILP32) {
      out_info->size = 4;
      out_info->alignment = 4;
    } else {
      out_info->size = 8;
      out_info->alignment = 8;
    }
    return CDD_C_SUCCESS;
  }

  if (strcmp(type_name, "__int128") == 0 ||
      strcmp(type_name, "unsigned __int128") == 0) {
    out_info->size = 16;
    out_info->alignment = 16;
    return CDD_C_SUCCESS;
  }

  return CDD_C_ERROR_NOT_FOUND;
}

static enum cdd_c_error extract_type_name(cdd_cst_node_t *node, char **out_name,
                                          int *is_pointer) {
  size_t i;
  char buf[256];
  size_t buf_len = 0;
  int ptr_count = 0;

  buf[0] = '\0';
  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
      cdd_token_t *tok = node->children[i].val.token;
      if (tok->kind == CDD_TOKEN_STAR) {
        ptr_count++;
      } else if (tok->kind == CDD_TOKEN_IDENTIFIER ||
                 tok->kind == CDD_TOKEN_KEYWORD_INT ||
                 tok->kind == CDD_TOKEN_KEYWORD___INT128) {
        if (buf_len > 0 && buf_len + 1 < sizeof(buf)) {
          buf[buf_len++] = ' ';
          buf[buf_len] = '\0';
        }
        if (buf_len + tok->length < sizeof(buf)) {
          memcpy(buf + buf_len, tok->start, tok->length);
          buf_len += tok->length;
          buf[buf_len] = '\0';
        }
      }
    }
  }

  if (ptr_count > 0) {
    *is_pointer = 1;
  } else {
    *is_pointer = 0;
  }

  if (buf_len > 0) {
    char *ret;
#ifdef CDD_BUILD_TESTS
    extern int g_cdd_cst_alloc_token_fail;
    if (g_cdd_cst_alloc_token_fail == 1)
      ret = NULL;
    else
#endif
      ret = (char *)malloc(buf_len + 1);
    if (!ret) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    memcpy(ret, buf, buf_len + 1);
    *out_name = ret;
    return CDD_C_SUCCESS;
  }

  return CDD_C_ERROR_NOT_FOUND;
}

enum cdd_c_error cdd_cst_eval_sizeof(cdd_cst_scope_env_t *env,
                                     cdd_cst_node_t *type_node,
                                     enum cdd_cst_abi_model_t abi,
                                     size_t *out_size) {
  char *name = NULL;
  int is_pointer = 0;
  int rc;
  cdd_cst_type_info_t info;

  if (!env || !type_node || !out_size)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = extract_type_name(type_node, &name, &is_pointer);
  if (rc == 0) {
    if (is_pointer) {
      rc = cdd_cst_eval_primitive_type("ptr", abi, &info);
    } else {
      rc = cdd_cst_eval_primitive_type(name, abi, &info);
      C_CDD_LOG_DEBUG("TYPE NAME: '%s'\n", name);
    }
    free(name);
    if (rc == 0) {
      *out_size = info.size;
      return CDD_C_SUCCESS;
    }
  }

  /* Fallback / Stub for structs, unions, arrays, etc. */
  *out_size = 0;
  return CDD_C_ERROR_SYSTEM;
}

enum cdd_c_error cdd_cst_eval_alignof(cdd_cst_scope_env_t *env,
                                      cdd_cst_node_t *type_node,
                                      enum cdd_cst_abi_model_t abi,
                                      size_t *out_align) {
  char *name = NULL;
  int is_pointer = 0;
  int rc;
  cdd_cst_type_info_t info;

  if (!env || !type_node || !out_align)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = extract_type_name(type_node, &name, &is_pointer);
  if (rc == 0) {
    if (is_pointer) {
      rc = cdd_cst_eval_primitive_type("ptr", abi, &info);
    } else {
      rc = cdd_cst_eval_primitive_type(name, abi, &info);
      C_CDD_LOG_DEBUG("TYPE NAME: '%s'\n", name);
    }
    free(name);
    if (rc == 0) {
      *out_align = info.alignment;
      return CDD_C_SUCCESS;
    }
  }

  /* Fallback / Stub for structs, unions, arrays, etc. */
  *out_align = 0;
  return CDD_C_ERROR_SYSTEM;
}
