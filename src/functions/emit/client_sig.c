/**
 * @file client_sig.c
 * @brief Implementation of Client Signature Generation.
 *
 * Updated to support Grouped naming convention (Resource_Prefix_OpId).
 * Appends standard `struct ApiError **api_error` argument to all operations.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/safe_crt.h"
#include "functions/emit/client_sig.h"
#include "functions/emit/client_sig_media.h"
#include "functions/emit/client_sig_param.h"
#include "functions/emit/client_sig_types.h"
#include "functions/parse/str.h"
#include "win_compat_sym.h"
/* clang-format on */

/** @brief CHECK_IO definition */
#ifdef CDD_BUILD_TESTS
extern int g_fail_io_after;
extern int g_io_calls;
extern C_CDD_EXPORT int g_cdd_fail_response_is_binary_success;
extern C_CDD_EXPORT int g_cdd_fail_get_success_schema;
extern C_CDD_EXPORT int g_cdd_fail_get_success_response;

static int test_cdd_fprintf_hook(FILE *stream, const char *format, ...)
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((format(printf, 2, 3)))
#endif
    ;
static int test_cdd_fprintf_hook(FILE *stream, const char *format, ...) {
  int ret;
  va_list args;
  if (g_fail_io_after >= 0 && ++g_io_calls > g_fail_io_after) {
    return -1;
  }
  va_start(args, format);
  ret = vfprintf(stream, format, args);
  va_end(args);
  return ret;
}
#define fprintf test_cdd_fprintf_hook
#endif

#define CHECK_IO(x)                                                            \
  do {                                                                         \
    if ((x) < 0)                                                               \
      return CDD_C_ERROR_IO;                                                   \
  } while (0)

/**
 * @brief Selects the primary success response from an operation.
 *
 * @param[in] op Operation definition.
 * @param[out] _out_val Pointer to receive selected response.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if _out_val is
 * NULL.
 */
static cdd_c_error_t
get_success_response(const struct OpenAPI_Operation *op,
                     const struct OpenAPI_Response **_out_val) {
  const struct OpenAPI_Response *default_resp = NULL;
  size_t i;
  if (!_out_val) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_get_success_response) {
    g_cdd_fail_get_success_response = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  *_out_val = NULL;
  if (!op) {
    return CDD_C_SUCCESS;
  }

  for (i = 0; i < op->n_responses; ++i) {
    const struct OpenAPI_Response *resp = &op->responses[i];
    const char *c = resp->code;
    if (!c) {
      continue;
    }
    if (strcmp(c, "default") == 0) {
      default_resp = resp;
      continue;
    }
    if (strlen(c) == 3 && c[0] == '2' && c[1] == 'X' && c[2] == 'X') {
      *_out_val = resp;
      return CDD_C_SUCCESS;
    }
    if (c[0] == '2') {
      *_out_val = resp;
      return CDD_C_SUCCESS;
    }
  }
  *_out_val = default_resp;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if the success response of an operation is binary.
 *
 * @param[in] op Operation definition.
 * @param[out] out Pointer to receive 1 if binary success, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t
response_is_binary_success(const struct OpenAPI_Operation *op, int *out) {
  const struct OpenAPI_Response *resp = NULL;
  cdd_c_error_t rc;
  if (!out) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_response_is_binary_success) {
    g_cdd_fail_response_is_binary_success = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  *out = 0;
  if (!op) {
    return CDD_C_SUCCESS;
  }
  rc = get_success_response(op, &resp);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  if (!resp || !resp->content_type) {
    return CDD_C_SUCCESS;
  }
  return media_type_is_binary(resp->content_type, out);
}

/**
 * @brief Selects the success schema reference from an operation.
 *
 * @param[in] op Operation definition.
 * @param[out] _out_val Pointer to receive selected schema reference pointer.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t
get_success_schema(const struct OpenAPI_Operation *op,
                   const struct OpenAPI_SchemaRef **_out_val) {
  const struct OpenAPI_Response *default_resp = NULL;
  size_t i;
  cdd_c_error_t rc;

  if (!_out_val) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_get_success_schema) {
    g_cdd_fail_get_success_schema = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  *_out_val = NULL;
  if (!op) {
    return CDD_C_SUCCESS;
  }

  for (i = 0; i < op->n_responses; ++i) {
    const char *c = op->responses[i].code;
    int has_inline = 0;
    if (!c) {
      continue;
    }
    if (strcmp(c, "default") == 0) {
      default_resp = &op->responses[i];
      continue;
    }
    rc = schema_has_inline(&op->responses[i].schema, &has_inline);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }
    if (strlen(c) == 3 && c[0] == '2' && c[1] == 'X' && c[2] == 'X') {
      if (op->responses[i].schema.ref_name || has_inline ||
          op->responses[i].schema.is_array) {
        *_out_val = &op->responses[i].schema;
        return CDD_C_SUCCESS;
      }
      continue;
    }
    if (c[0] == '2') {
      if (op->responses[i].schema.ref_name || has_inline ||
          op->responses[i].schema.is_array) {
        *_out_val = &op->responses[i].schema;
        return CDD_C_SUCCESS;
      }
    }
  }

  if (default_resp) {
    int has_inline = 0;
    rc = schema_has_inline(&default_resp->schema, &has_inline);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }
    if (default_resp->schema.ref_name || has_inline ||
        default_resp->schema.is_array) {
      *_out_val = &default_resp->schema;
      return CDD_C_SUCCESS;
    }
  }

  *_out_val = &op->req_body;
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for codegen client write signature.
 */
cdd_c_error_t
codegen_client_write_signature(FILE *fp, const struct OpenAPI_Operation *op,
                               const struct CodegenSigConfig *config) {
  const char *ctx_type;
  const char *prefix;
  const char *func_name;
  const char *group;
  const struct OpenAPI_SchemaRef *success_schema = NULL;
  int success_is_binary = 0;
  size_t i;
  cdd_c_error_t rc;

  if (!fp || !op) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  ctx_type =
      (config && config->ctx_type) ? config->ctx_type : "struct HttpClient *";
  prefix = (config && config->prefix) ? config->prefix : "";
  group = (config && config->group_name) ? config->group_name : NULL;
  func_name = op->operation_id ? op->operation_id : "unnamed_op";

  /* Construct function name: [Group_][Prefix][OpName] */
  CHECK_IO(fprintf(fp, "int "));
  if (group && *group) {
    CHECK_IO(fprintf(fp, "%s_", group));
  }
  CHECK_IO(fprintf(fp, "%s%s(%sctx", prefix, func_name, ctx_type));

  /* 1. Parameters */
  for (i = 0; i < op->n_parameters; ++i) {
    const struct OpenAPI_Parameter *p = &op->parameters[i];
    if (p->in == OA_PARAM_IN_QUERYSTRING) {
      const char *qs_json_item = NULL;
      const char *qs_json_obj = NULL;
      const char *qs_json_prim = NULL;
      const char *qs_raw = NULL;
      int is_form_obj = 0;
      int is_json_ref_val = 0;

      rc = sig_querystring_param_json_array_item_type(p, &qs_json_item);
      if (rc != CDD_C_SUCCESS) {
        return rc;
      }
      rc = sig_querystring_param_json_array_item_ref(p, &qs_json_obj);
      if (rc != CDD_C_SUCCESS) {
        return rc;
      }
      rc = sig_querystring_param_json_primitive_type(p, &qs_json_prim);
      if (rc != CDD_C_SUCCESS) {
        return rc;
      }
      rc = sig_querystring_param_raw_primitive_type(p, &qs_raw);
      if (rc != CDD_C_SUCCESS) {
        return rc;
      }
      rc = sig_querystring_param_is_form_object(p, &is_form_obj);
      if (rc != CDD_C_SUCCESS) {
        return rc;
      }
      rc = sig_querystring_param_is_json_ref(p, &is_json_ref_val);
      if (rc != CDD_C_SUCCESS) {
        return rc;
      }

      if (is_form_obj) {
        CHECK_IO(fprintf(fp, ", const struct OpenAPI_KV *%s, size_t %s_len",
                         p->name, p->name));
      } else if (is_json_ref_val) {
        CHECK_IO(
            fprintf(fp, ", const struct %s *%s", p->schema.ref_name, p->name));
      } else if (qs_json_obj) {
        CHECK_IO(fprintf(fp, ", const struct %s **%s, size_t %s_len",
                         qs_json_obj, p->name, p->name));
      } else if (qs_json_item) {
        const char *c_type = NULL;
        rc = map_array_item_type(qs_json_item, &c_type);
        if (rc != CDD_C_SUCCESS) {
          return rc;
        }
        CHECK_IO(
            fprintf(fp, ", %s%s, size_t %s_len", c_type, p->name, p->name));
      } else if (qs_json_prim) {
        const char *c_type = NULL;
        rc = map_type_to_c_arg(qs_json_prim, &c_type);
        if (rc != CDD_C_SUCCESS) {
          return rc;
        }
        CHECK_IO(fprintf(fp, ", %s%s", c_type, p->name));
      } else if (qs_raw) {
        const char *c_type = NULL;
        rc = map_type_to_c_arg(qs_raw, &c_type);
        if (rc != CDD_C_SUCCESS) {
          return rc;
        }
        CHECK_IO(fprintf(fp, ", %s%s", c_type, p->name));
      } else {
        CHECK_IO(fprintf(fp, ", const char *%s", p->name));
      }
      continue;
    }
    {
      int is_p_json = 0;
      if (p->content_type) {
        rc = media_type_is_json(p->content_type, &is_p_json);
        if (rc != CDD_C_SUCCESS) {
          return rc;
        }
      }
      if (is_p_json) {
        const char *ref_name = p->schema.ref_name;
        int is_prim = 0;
        if (p->type) {
          rc = is_primitive_type(p->type, &is_prim);
          if (rc != CDD_C_SUCCESS) {
            return rc;
          }
        }
        if (!ref_name && p->type && !is_prim &&
            strcmp(p->type, "object") != 0 && strcmp(p->type, "array") != 0) {
          ref_name = p->type;
        }
        if (p->is_array) {
          const char *item_type =
              p->items_type ? p->items_type : p->schema.inline_type;
          int item_is_prim = 0;
          if (item_type) {
            rc = is_primitive_type(item_type, &item_is_prim);
            if (rc != CDD_C_SUCCESS) {
              return rc;
            }
          }
          if (item_type && item_is_prim) {
            const char *c_type = NULL;
            rc = map_array_item_type(item_type, &c_type);
            if (rc != CDD_C_SUCCESS) {
              return rc;
            }
            CHECK_IO(
                fprintf(fp, ", %s%s, size_t %s_len", c_type, p->name, p->name));
          } else if (item_type && strcmp(item_type, "object") != 0) {
            CHECK_IO(fprintf(fp, ", const struct %s **%s, size_t %s_len",
                             item_type, p->name, p->name));
          } else {
            CHECK_IO(fprintf(fp, ", const void *%s, size_t %s_len", p->name,
                             p->name));
          }
        } else if (ref_name) {
          CHECK_IO(fprintf(fp, ", const struct %s *%s", ref_name, p->name));
        } else if (p->type && strcmp(p->type, "object") == 0) {
          CHECK_IO(fprintf(fp, ", const struct OpenAPI_KV *%s, size_t %s_len",
                           p->name, p->name));
        } else {
          const char *prim = p->type ? p->type : p->schema.inline_type;
          const char *c_type = NULL;
          rc = map_type_to_c_arg(prim ? prim : "string", &c_type);
          if (rc != CDD_C_SUCCESS) {
            return rc;
          }
          CHECK_IO(fprintf(fp, ", %s%s", c_type, p->name));
        }
        continue;
      }
    }
    {
      int is_obj_kv = 0;
      rc = param_is_object_kv(p, &is_obj_kv);
      if (rc != CDD_C_SUCCESS) {
        return rc;
      }
      if (is_obj_kv) {
        CHECK_IO(fprintf(fp, ", const struct OpenAPI_KV *%s, size_t %s_len",
                         p->name, p->name));
      } else if (p->is_array) {
        const char *c_type = NULL;
        rc = map_array_item_type(p->items_type, &c_type);
        if (rc != CDD_C_SUCCESS) {
          return rc;
        }
        CHECK_IO(
            fprintf(fp, ", %s%s, size_t %s_len", c_type, p->name, p->name));
      } else {
        const char *c_type = NULL;
        rc = map_type_to_c_arg(p->type, &c_type);
        if (rc != CDD_C_SUCCESS) {
          return rc;
        }
        CHECK_IO(fprintf(fp, ", %s%s", c_type, p->name));
      }
    }
  }

  /* 2. Request Body */
  if (op->req_body.content_type) {
    int is_bin = 0;
    int is_mp = 0;
    int is_mp_form = 0;
    int is_txt = 0;

    rc = media_type_is_binary(op->req_body.content_type, &is_bin);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }
    rc = media_type_is_multipart(op->req_body.content_type, &is_mp);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }
    rc = media_type_is_multipart_form(op->req_body.content_type, &is_mp_form);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }
    rc = media_type_is_textual(op->req_body.content_type, &is_txt);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }

    if (is_bin || (is_mp && !is_mp_form)) {
      CHECK_IO(fprintf(fp, ", const unsigned char *body, size_t body_len"));
    } else if (is_txt) {
      CHECK_IO(fprintf(fp, ", const char *req_body"));
    } else if (op->req_body.ref_name) {
      if (op->req_body.is_array) {
        if (strcmp(op->req_body.ref_name, "string") == 0) {
          CHECK_IO(fprintf(fp, ", const char **body, size_t body_len"));
        } else if (strcmp(op->req_body.ref_name, "integer") == 0) {
          CHECK_IO(fprintf(fp, ", const int *body, size_t body_len"));
        } else {
          CHECK_IO(fprintf(fp, ", struct %s **body, size_t body_len",
                           op->req_body.ref_name));
        }
      } else {
        CHECK_IO(
            fprintf(fp, ", const struct %s *req_body", op->req_body.ref_name));
      }
    } else if (op->req_body.inline_type) {
      if (op->req_body.is_array) {
        const char *item_type = op->req_body.inline_type;
        const char *c_type = NULL;
        rc = map_array_item_type(item_type, &c_type);
        if (rc != CDD_C_SUCCESS) {
          return rc;
        }
        CHECK_IO(fprintf(fp, ", %sbody, size_t body_len", c_type));
      } else {
        const char *c_type = NULL;
        rc = map_type_to_c_arg(op->req_body.inline_type, &c_type);
        if (rc != CDD_C_SUCCESS) {
          return rc;
        }
        CHECK_IO(fprintf(fp, ", %sreq_body", c_type));
      }
    }
  }

  /* 2b. Multipart per-part encoding headers */
  if (op->req_body.content_type) {
    int is_mp_form = 0;
    rc = media_type_is_multipart_form(op->req_body.content_type, &is_mp_form);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }
    if (is_mp_form) {
      const struct OpenAPI_MediaType *mt = NULL;
      rc = find_media_type(op->req_body_media_types, op->n_req_body_media_types,
                           "multipart/form-data", &mt);
      if (rc != CDD_C_SUCCESS) {
        return rc;
      }
      if (mt && mt->encoding && mt->n_encoding > 0) {
        size_t e;
        for (e = 0; e < mt->n_encoding; ++e) {
          const struct OpenAPI_Encoding *enc = &mt->encoding[e];
          size_t h;
          if (!enc->name || !enc->headers || enc->n_headers == 0) {
            continue;
          }
          for (h = 0; h < enc->n_headers; ++h) {
            const struct OpenAPI_Header *hdr = &enc->headers[h];
            const char *hdr_type = hdr->type ? hdr->type : "string";
            int hdr_is_array =
                hdr->is_array || (strcmp(hdr_type, "array") == 0);
            char param_name[256];
            int is_ct = 0;
            if (!hdr->name) {
              continue;
            }
            rc = header_name_is_content_type(hdr->name, &is_ct);
            if (rc != CDD_C_SUCCESS) {
              return rc;
            }
            if (is_ct) {
              continue;
            }
            rc = multipart_header_param_name(param_name, sizeof(param_name),
                                             enc->name, hdr->name);
            if (rc != CDD_C_SUCCESS) {
              return rc;
            }
            if (hdr_is_array) {
              const char *item_type =
                  hdr->items_type ? hdr->items_type : "string";
              const char *c_type = NULL;
              rc = map_array_item_type(item_type, &c_type);
              if (rc != CDD_C_SUCCESS) {
                return rc;
              }
              CHECK_IO(fprintf(fp, ", %s%s, size_t %s_len", c_type, param_name,
                               param_name));
            } else if (strcmp(hdr_type, "object") == 0) {
              CHECK_IO(fprintf(fp,
                               ", const struct OpenAPI_KV *%s, size_t %s_len",
                               param_name, param_name));
            } else {
              const char *c_type = NULL;
              rc = map_type_to_c_arg(hdr_type, &c_type);
              if (rc != CDD_C_SUCCESS) {
                return rc;
              }
              CHECK_IO(fprintf(fp, ", %s%s", c_type, param_name));
            }
          }
        }
      }
    }
  }

  /* 3. Success Output */
  rc = response_is_binary_success(op, &success_is_binary);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  rc = get_success_schema(op, &success_schema);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }

  if (success_is_binary) {
    CHECK_IO(fprintf(fp, ", unsigned char **out, size_t *out_len"));
  } else {
    int schema_inline = 0;
    rc = schema_has_inline(success_schema, &schema_inline);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }

    if (success_schema->ref_name || schema_inline || success_schema->is_array) {
      if (success_schema->is_array) {
        if (success_schema->ref_name) {
          if (strcmp(success_schema->ref_name, "string") == 0) {
            CHECK_IO(fprintf(fp, ", char ***out, size_t *out_len"));
          } else if (strcmp(success_schema->ref_name, "integer") == 0) {
            CHECK_IO(fprintf(fp, ", int **out, size_t *out_len"));
          } else {
            CHECK_IO(fprintf(fp, ", struct %s ***out, size_t *out_len",
                             success_schema->ref_name));
          }
        } else {
          const char *out_type = NULL;
          rc = map_array_item_type_out(success_schema->inline_type, &out_type);
          if (rc != CDD_C_SUCCESS) {
            return rc;
          }
          CHECK_IO(fprintf(fp, ", %sout, size_t *out_len", out_type));
        }
      } else if (success_schema->ref_name) {
        CHECK_IO(fprintf(fp, ", struct %s **out", success_schema->ref_name));
      } else {
        const char *out_type = NULL;
        rc = map_type_to_c_out(success_schema->inline_type, &out_type);
        if (rc != CDD_C_SUCCESS) {
          return rc;
        }
        CHECK_IO(fprintf(fp, ", %sout", out_type));
      }
    }
  }

  /* 4. Global Error Output */
  CHECK_IO(fprintf(fp, ", struct ApiError **api_error"));
  CHECK_IO(fprintf(fp, ")"));

  if (config && config->include_semicolon) {
    CHECK_IO(fprintf(fp, ";\n"));
  } else {
    CHECK_IO(fprintf(fp, " {\n"));
  }

  return CDD_C_SUCCESS;
}

#ifdef CDD_BUILD_TESTS
/**
 * @brief Test helper to get success response.
 */
cdd_c_error_t
cdd_test_sig_get_success_response(const struct OpenAPI_Operation *op,
                                  const struct OpenAPI_Response **out_val) {
  return get_success_response(op, out_val);
}

/**
 * @brief Test helper to check if response is binary success.
 */
cdd_c_error_t
cdd_test_sig_response_is_binary_success(const struct OpenAPI_Operation *op,
                                        int *out) {
  return response_is_binary_success(op, out);
}

/**
 * @brief Test helper to get success schema.
 */
cdd_c_error_t
cdd_test_sig_get_success_schema(const struct OpenAPI_Operation *op,
                                const struct OpenAPI_SchemaRef **out_val) {
  return get_success_schema(op, out_val);
}
#endif
