/**
 * @file url_query.c
 * @brief Implementation of URL query parameters code generation.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "functions/parse/str.h"
#include "routes/emit/url.h"
#include "win_compat_sym.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
#include <stdarg.h>
static int test_cdd_fprintf_hook(FILE *stream, const char *format, ...)
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((format(printf, 2, 3)))
#endif
    ;
static int test_cdd_fprintf_hook(FILE *stream, const char *format, ...) {
  int ret;
  va_list args;
  if (g_fail_io_after >= 0 && ++g_io_calls > g_fail_io_after)
    return -1;
  va_start(args, format);
  ret = vfprintf(stream, format, args);
  va_end(args);
  return ret;
}
#define fprintf test_cdd_fprintf_hook
#endif

/** @brief CHECK_IO definition */
#define CHECK_IO(x)                                                            \
  for (; (x) < 0;)                                                             \
  return CDD_C_ERROR_IO

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif
#endif

/**
 * @brief Generates C code to append query parameters to a constructed URL.
 */
cdd_c_error_t codegen_url_write_query_params(FILE *fp,
                                             const struct OpenAPI_Operation *op,
                                             int qp_tracking) {
  const char *_ast_querystring_param_json_array_item_type_6 = NULL;
  const char *_ast_querystring_param_json_array_item_ref_7 = NULL;
  const char *_ast_querystring_param_json_primitive_type_8 = NULL;
  const char *_ast_querystring_param_raw_primitive_type_9 = NULL;
  size_t i;
  int has_query = 0;
  const struct OpenAPI_Parameter *querystring_param = NULL;

  if (!fp || !op)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < op->n_parameters; ++i) {
    if (op->parameters[i].in == OA_PARAM_IN_QUERYSTRING) {
      querystring_param = &op->parameters[i];
      break;
    }
  }

  if (querystring_param) {
    const char *qs_name =
        querystring_param->name ? querystring_param->name : "querystring";
    const char *qs_json_item =
        (querystring_param_json_array_item_type(
             querystring_param, &_ast_querystring_param_json_array_item_type_6),
         _ast_querystring_param_json_array_item_type_6);
    const char *qs_json_obj =
        (querystring_param_json_array_item_ref(
             querystring_param, &_ast_querystring_param_json_array_item_ref_7),
         _ast_querystring_param_json_array_item_ref_7);
    const char *qs_json_prim =
        (querystring_param_json_primitive_type(
             querystring_param, &_ast_querystring_param_json_primitive_type_8),
         _ast_querystring_param_json_primitive_type_8);
    if (querystring_param_is_form_object(querystring_param)) {
      CHECK_IO(fprintf(fp, "  /* Querystring Parameter (form object): %s */\n",
                       qs_name));
      CHECK_IO(fprintf(fp, "  if (%s && %s_len > 0) {\n", qs_name, qs_name));
      CHECK_IO(fprintf(fp, "    size_t i;\n"));
      CHECK_IO(fprintf(fp, "    char *qs_form_body = NULL;\n"));
      CHECK_IO(fprintf(fp, "    rc = url_query_init(&qp);\n"));
      CHECK_IO(fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
      CHECK_IO(fprintf(fp, "    for(i=0; i < %s_len; ++i) {\n", qs_name));
      CHECK_IO(fprintf(fp, "      const struct OpenAPI_KV *kv = &%s[i];\n",
                       qs_name));
      CHECK_IO(fprintf(fp, "      const char *kv_key = kv->key;\n"));
      CHECK_IO(fprintf(fp, "      const char *kv_raw = NULL;\n"));
      CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
      CHECK_IO(fprintf(fp, "      switch (kv->type) {\n"));
      CHECK_IO(fprintf(fp, "      case OA_KV_STRING:\n"));
      CHECK_IO(fprintf(fp, "        kv_raw = kv->value.s;\n        break;\n"));
      CHECK_IO(fprintf(fp, "      case OA_KV_INTEGER:\n"
                           "        spr"
                           "intf(num_buf, \"%%d\", kv->value.i);\n"
                           "        kv_raw = num_buf;\n"
                           "        break;\n"));
      CHECK_IO(fprintf(fp, "      case OA_KV_NUMBER:\n"
                           "        spr"
                           "intf(num_buf, \"%%g\", kv->value.n);\n"
                           "        kv_raw = num_buf;\n"
                           "        break;\n"));
      CHECK_IO(fprintf(fp,
                       "      case OA_KV_BOOLEAN:\n"
                       "        kv_raw = kv->value.b ? \"true\" : \"false\";\n"
                       "        break;\n"));
      CHECK_IO(fprintf(
          fp, "      default:\n        kv_raw = NULL;\n        break;\n"));
      CHECK_IO(fprintf(fp, "      }\n"));
      CHECK_IO(fprintf(fp, "      if (!kv_key || !kv_raw) continue;\n"));
      CHECK_IO(fprintf(fp, "      rc = url_query_add(&qp, kv_key, kv_raw);\n"));
      CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
      CHECK_IO(fprintf(fp, "    }\n"));
      CHECK_IO(fprintf(fp, "    rc = url_query_build_form(&qp, "
                           "&qs_form_body);\n"));
      CHECK_IO(fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
      CHECK_IO(fprintf(fp, "    if (qs_form_body && qs_form_body[0] != "
                           "'\\0') {\n"));
      CHECK_IO(fprintf(
          fp, "      if (asprintf(&query_str, \"?%%s\", "
              "qs_form_body) == -1) { rc = CDD_C_ERROR_MEMORY; goto cleanup; "
              "}\n"));
      CHECK_IO(fprintf(fp, "    } else {\n"));
      CHECK_IO(fprintf(fp, "      query_str = strdup(\"\");\n"));
      CHECK_IO(fprintf(
          fp, "      if (!query_str) { rc = CDD_C_ERROR_MEMORY; goto cleanup; "
              "}\n"));
      CHECK_IO(fprintf(fp, "    }\n"));
      CHECK_IO(fprintf(fp, "    free(qs_form_body);\n"));
      CHECK_IO(fprintf(fp, "  } else {\n"));
      CHECK_IO(fprintf(fp, "    query_str = strdup(\"\");\n"));
      CHECK_IO(fprintf(
          fp,
          "    if (!query_str) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "  }\n\n"));
      return CDD_C_SUCCESS;
    }
    if (qs_json_obj) {
      CHECK_IO(fprintf(
          fp, "  /* Querystring Parameter (json array objects): %s */\n",
          qs_name));
      CHECK_IO(fprintf(fp, "  if (%s && %s_len > 0) {\n", qs_name, qs_name));
      CHECK_IO(fprintf(fp, "    JSON_Value *qs_val = NULL;\n"));
      CHECK_IO(fprintf(fp, "    JSON_Array *qs_arr = NULL;\n"));
      CHECK_IO(fprintf(fp, "    char *qs_json = NULL;\n"));
      CHECK_IO(fprintf(fp, "    char *qs_enc = NULL;\n"));
      CHECK_IO(fprintf(fp, "    size_t i;\n"));
      CHECK_IO(fprintf(fp, "    qs_val = json_value_init_array();\n"));
      CHECK_IO(fprintf(
          fp, "    if (!qs_val) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    qs_arr = json_value_get_array(qs_val);\n"));
      CHECK_IO(fprintf(fp, "    if (!qs_arr) { rc = "
                           "CDD_C_ERROR_INVALID_ARGUMENT; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    for (i = 0; i < %s_len; ++i) {\n", qs_name));
      CHECK_IO(fprintf(fp, "      char *item_json = NULL;\n"));
      CHECK_IO(fprintf(fp, "      JSON_Value *item_val = NULL;\n"));
      CHECK_IO(fprintf(fp, "      if (!%s[i]) {\n", qs_name));
      CHECK_IO(fprintf(
          fp, "        if (json_array_append_null(qs_arr) != "
              "JSONSuccess) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "        continue;\n"));
      CHECK_IO(fprintf(fp, "      }\n"));
      CHECK_IO(fprintf(fp, "      rc = %s_to_json(%s[i], &item_json);\n",
                       qs_json_obj, qs_name));
      CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
      CHECK_IO(fprintf(fp, "      item_val = json_parse_string(item_json);\n"));
      CHECK_IO(fprintf(fp, "      free(item_json);\n"));
      CHECK_IO(fprintf(fp, "      if (!item_val) { rc = "
                           "CDD_C_ERROR_INVALID_ARGUMENT; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp,
                       "      if (json_array_append_value(qs_arr, item_val) "
                       "!= JSONSuccess) { json_value_free(item_val); rc = "
                       "ENOMEM; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    }\n"));
      CHECK_IO(
          fprintf(fp, "    qs_json = json_serialize_to_string(qs_val);\n"));
      CHECK_IO(fprintf(fp, "    json_value_free(qs_val);\n"));
      CHECK_IO(fprintf(
          fp,
          "    if (!qs_json) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    url_encode(qs_json, &qs_enc);\n"));
      CHECK_IO(fprintf(fp, "    json_free_serialized_string(qs_json);\n"));
      CHECK_IO(fprintf(
          fp, "    if (!qs_enc) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
      CHECK_IO(fprintf(
          fp, "    if (asprintf(&query_str, \"?%%s\", qs_enc) == -1) "
              "{ rc = CDD_C_ERROR_MEMORY; free(qs_enc); goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    free(qs_enc);\n"));
      CHECK_IO(fprintf(fp, "  } else {\n"));
      CHECK_IO(fprintf(fp, "    query_str = strdup(\"\");\n"));
      CHECK_IO(fprintf(
          fp,
          "    if (!query_str) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "  }\n\n"));
      return CDD_C_SUCCESS;
    }
    if (qs_json_item) {
      CHECK_IO(fprintf(fp, "  /* Querystring Parameter (json array): %s */\n",
                       qs_name));
      CHECK_IO(fprintf(fp, "  if (%s && %s_len > 0) {\n", qs_name, qs_name));
      CHECK_IO(fprintf(fp, "    JSON_Value *qs_val = NULL;\n"));
      CHECK_IO(fprintf(fp, "    JSON_Array *qs_arr = NULL;\n"));
      CHECK_IO(fprintf(fp, "    char *qs_json = NULL;\n"));
      CHECK_IO(fprintf(fp, "    char *qs_enc = NULL;\n"));
      CHECK_IO(fprintf(fp, "    size_t i;\n"));
      CHECK_IO(fprintf(fp, "    qs_val = json_value_init_array();\n"));
      CHECK_IO(fprintf(
          fp, "    if (!qs_val) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    qs_arr = json_value_get_array(qs_val);\n"));
      CHECK_IO(fprintf(fp, "    if (!qs_arr) { rc = "
                           "CDD_C_ERROR_INVALID_ARGUMENT; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    for (i = 0; i < %s_len; ++i) {\n", qs_name));
      if (strcmp(qs_json_item, "string") == 0) {
        CHECK_IO(fprintf(fp, "      if (!%s[i]) {\n", qs_name));
        CHECK_IO(fprintf(
            fp, "        if (json_array_append_null(qs_arr) != "
                "JSONSuccess) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
        CHECK_IO(fprintf(fp, "      } else {\n"));
        CHECK_IO(fprintf(
            fp,
            "        if (json_array_append_string(qs_arr, %s[i]) "
            "!= JSONSuccess) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n",
            qs_name));
        CHECK_IO(fprintf(fp, "      }\n"));
      } else if (strcmp(qs_json_item, "integer") == 0) {
        CHECK_IO(fprintf(
            fp,
            "      if (json_array_append_number(qs_arr, (double)%s[i]) "
            "!= JSONSuccess) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n",
            qs_name));
      } else if (strcmp(qs_json_item, "number") == 0) {
        CHECK_IO(fprintf(
            fp,
            "      if (json_array_append_number(qs_arr, %s[i]) "
            "!= JSONSuccess) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n",
            qs_name));
      } else {
        CHECK_IO(fprintf(
            fp,
            "      if (json_array_append_boolean(qs_arr, %s[i] ? 1 : 0) "
            "!= JSONSuccess) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n",
            qs_name));
      }
      CHECK_IO(fprintf(fp, "    }\n"));
      CHECK_IO(
          fprintf(fp, "    qs_json = json_serialize_to_string(qs_val);\n"));
      CHECK_IO(fprintf(fp, "    json_value_free(qs_val);\n"));
      CHECK_IO(fprintf(
          fp,
          "    if (!qs_json) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    url_encode(qs_json, &qs_enc);\n"));
      CHECK_IO(fprintf(fp, "    json_free_serialized_string(qs_json);\n"));
      CHECK_IO(fprintf(
          fp, "    if (!qs_enc) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
      CHECK_IO(fprintf(
          fp, "    if (asprintf(&query_str, \"?%%s\", qs_enc) == -1) "
              "{ rc = CDD_C_ERROR_MEMORY; free(qs_enc); goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    free(qs_enc);\n"));
      CHECK_IO(fprintf(fp, "  } else {\n"));
      CHECK_IO(fprintf(fp, "    query_str = strdup(\"\");\n"));
      CHECK_IO(fprintf(
          fp,
          "    if (!query_str) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "  }\n\n"));
      return CDD_C_SUCCESS;
    }
    if (qs_json_prim) {
      CHECK_IO(fprintf(
          fp, "  /* Querystring Parameter (json primitive): %s */\n", qs_name));
      if (strcmp(qs_json_prim, "string") == 0) {
        CHECK_IO(fprintf(fp, "  if (%s) {\n", qs_name));
      } else {
        CHECK_IO(fprintf(fp, "  {\n"));
      }
      CHECK_IO(fprintf(fp, "    JSON_Value *qs_val = NULL;\n"));
      CHECK_IO(fprintf(fp, "    char *qs_json = NULL;\n"));
      CHECK_IO(fprintf(fp, "    char *qs_enc = NULL;\n"));
      if (strcmp(qs_json_prim, "string") == 0) {
        CHECK_IO(
            fprintf(fp, "    qs_val = json_value_init_string(%s);\n", qs_name));
      } else if (strcmp(qs_json_prim, "integer") == 0) {
        CHECK_IO(fprintf(
            fp, "    qs_val = json_value_init_number((double)%s);\n", qs_name));
      } else if (strcmp(qs_json_prim, "number") == 0) {
        CHECK_IO(
            fprintf(fp, "    qs_val = json_value_init_number(%s);\n", qs_name));
      } else {
        CHECK_IO(fprintf(fp,
                         "    qs_val = json_value_init_boolean(%s ? 1 : 0);\n",
                         qs_name));
      }
      CHECK_IO(fprintf(
          fp, "    if (!qs_val) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
      CHECK_IO(
          fprintf(fp, "    qs_json = json_serialize_to_string(qs_val);\n"));
      CHECK_IO(fprintf(fp, "    json_value_free(qs_val);\n"));
      CHECK_IO(fprintf(
          fp,
          "    if (!qs_json) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    url_encode(qs_json, &qs_enc);\n"));
      CHECK_IO(fprintf(fp, "    json_free_serialized_string(qs_json);\n"));
      CHECK_IO(fprintf(
          fp, "    if (!qs_enc) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
      CHECK_IO(fprintf(
          fp, "    if (asprintf(&query_str, \"?%%s\", qs_enc) == -1) "
              "{ rc = CDD_C_ERROR_MEMORY; free(qs_enc); goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    free(qs_enc);\n"));
      if (strcmp(qs_json_prim, "string") == 0) {
        CHECK_IO(fprintf(fp, "  } else {\n"));
        CHECK_IO(fprintf(fp, "    query_str = strdup(\"\");\n"));
        CHECK_IO(fprintf(fp, "    if (!query_str) { rc = CDD_C_ERROR_MEMORY; "
                             "goto cleanup; }\n"));
        CHECK_IO(fprintf(fp, "  }\n\n"));
      } else {
        CHECK_IO(fprintf(fp, "  }\n\n"));
      }
      return CDD_C_SUCCESS;
    }
    if (querystring_param_is_json_ref(querystring_param)) {
      CHECK_IO(
          fprintf(fp, "  /* Querystring Parameter (json): %s */\n", qs_name));
      CHECK_IO(fprintf(fp, "  if (%s) {\n", qs_name));
      CHECK_IO(fprintf(fp, "    char *qs_json = NULL;\n"));
      CHECK_IO(fprintf(fp, "    char *qs_enc = NULL;\n"));
      CHECK_IO(fprintf(fp, "    rc = %s_to_json(%s, &qs_json);\n",
                       querystring_param->schema.ref_name, qs_name));
      CHECK_IO(fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
      CHECK_IO(fprintf(fp, "    url_encode(qs_json, &qs_enc);\n"));
      CHECK_IO(fprintf(fp, "    free(qs_json);\n"));
      CHECK_IO(fprintf(
          fp, "    if (!qs_enc) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
      CHECK_IO(fprintf(
          fp, "    if (asprintf(&query_str, \"?%%s\", qs_enc) == -1) "
              "{ rc = CDD_C_ERROR_MEMORY; free(qs_enc); goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    free(qs_enc);\n"));
      CHECK_IO(fprintf(fp, "  } else {\n"));
      CHECK_IO(fprintf(fp, "    query_str = strdup(\"\");\n"));
      CHECK_IO(fprintf(
          fp,
          "    if (!query_str) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "  }\n\n"));
      return CDD_C_SUCCESS;
    }
    {
      const char *qs_raw =
          (querystring_param_raw_primitive_type(
               querystring_param, &_ast_querystring_param_raw_primitive_type_9),
           _ast_querystring_param_raw_primitive_type_9);
      if (qs_raw) {
        CHECK_IO(
            fprintf(fp, "  /* Querystring Parameter (raw): %s */\n", qs_name));
        if (strcmp(qs_raw, "string") == 0) {
          CHECK_IO(fprintf(fp, "  if (%s) {\n", qs_name));
          CHECK_IO(
              fprintf(fp, "    char *qs_enc = NULL; url_encode(%s, &qs_enc);\n",
                      qs_name));
          CHECK_IO(fprintf(
              fp,
              "    if (!qs_enc) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
          CHECK_IO(fprintf(
              fp,
              "    if (asprintf(&query_str, \"?%%s\", qs_enc) "
              "== -1) { rc = CDD_C_ERROR_MEMORY; free(qs_enc); goto cleanup; "
              "}\n"));
          CHECK_IO(fprintf(fp, "    free(qs_enc);\n"));
          CHECK_IO(fprintf(fp, "  } else {\n"));
          CHECK_IO(fprintf(fp, "    query_str = strdup(\"\");\n"));
          CHECK_IO(fprintf(fp, "    if (!query_str) { rc = CDD_C_ERROR_MEMORY; "
                               "goto cleanup; }\n"));
          CHECK_IO(fprintf(fp, "  }\n\n"));
        } else if (strcmp(qs_raw, "integer") == 0) {
          CHECK_IO(fprintf(fp, "  {\n    char num_buf[32];\n"));
          CHECK_IO(fprintf(fp, "    char *qs_enc = NULL;\n"));
          CHECK_IO(fprintf(fp,
                           "    spr"
                           "intf(num_buf, \"%%d\", %s);\n",
                           qs_name));
          CHECK_IO(fprintf(fp, "    url_encode(num_buf, &qs_enc);\n"));
          CHECK_IO(fprintf(
              fp,
              "    if (!qs_enc) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
          CHECK_IO(fprintf(
              fp,
              "    if (asprintf(&query_str, \"?%%s\", qs_enc) "
              "== -1) { rc = CDD_C_ERROR_MEMORY; free(qs_enc); goto cleanup; "
              "}\n"));
          CHECK_IO(fprintf(fp, "    free(qs_enc);\n"));
          CHECK_IO(fprintf(fp, "  }\n\n"));
        } else if (strcmp(qs_raw, "number") == 0) {
          CHECK_IO(fprintf(fp, "  {\n    char num_buf[64];\n"));
          CHECK_IO(fprintf(fp, "    char *qs_enc = NULL;\n"));
          CHECK_IO(fprintf(fp,
                           "    spr"
                           "intf(num_buf, \"%%g\", %s);\n",
                           qs_name));
          CHECK_IO(fprintf(fp, "    url_encode(num_buf, &qs_enc);\n"));
          CHECK_IO(fprintf(
              fp,
              "    if (!qs_enc) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
          CHECK_IO(fprintf(
              fp,
              "    if (asprintf(&query_str, \"?%%s\", qs_enc) "
              "== -1) { rc = CDD_C_ERROR_MEMORY; free(qs_enc); goto cleanup; "
              "}\n"));
          CHECK_IO(fprintf(fp, "    free(qs_enc);\n"));
          CHECK_IO(fprintf(fp, "  }\n\n"));
        } else {
          CHECK_IO(fprintf(fp, "  {\n"));
          CHECK_IO(fprintf(
              fp, "    const char *raw_val = %s ? \"true\" : \"false\";\n",
              qs_name));
          CHECK_IO(fprintf(
              fp, "    char *qs_enc = NULL; url_encode(raw_val, &qs_enc);\n"));
          CHECK_IO(fprintf(
              fp,
              "    if (!qs_enc) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
          CHECK_IO(fprintf(
              fp,
              "    if (asprintf(&query_str, \"?%%s\", qs_enc) "
              "== -1) { rc = CDD_C_ERROR_MEMORY; free(qs_enc); goto cleanup; "
              "}\n"));
          CHECK_IO(fprintf(fp, "    free(qs_enc);\n"));
          CHECK_IO(fprintf(fp, "  }\n\n"));
        }
        return CDD_C_SUCCESS;
      }
    }
    CHECK_IO(fprintf(fp, "  rc = url_query_init(&qp);\n"));
    CHECK_IO(fprintf(fp, "  if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
    CHECK_IO(fprintf(fp, "  /* Querystring Parameter: %s */\n", qs_name));
    CHECK_IO(fprintf(fp, "  if (%s && %s[0] != '\\0') {\n", qs_name, qs_name));
    CHECK_IO(fprintf(fp, "    if (%s[0] == '?') {\n", qs_name));
    CHECK_IO(fprintf(fp, "      query_str = strdup(%s);\n", qs_name));
    CHECK_IO(fprintf(fp, "      if (!query_str) goto cleanup;\n"));
    CHECK_IO(fprintf(fp, "    } else {\n"));
    CHECK_IO(fprintf(fp,
                     "      if (asprintf(&query_str, \"?%%s\", %s) == -1) "
                     "{ rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n",
                     qs_name));
    CHECK_IO(fprintf(fp, "    }\n"));
    CHECK_IO(fprintf(fp, "  } else {\n"));
    CHECK_IO(fprintf(fp, "    query_str = strdup(\"\");\n"));
    CHECK_IO(fprintf(fp, "    if (!query_str) goto cleanup;\n"));
    CHECK_IO(fprintf(fp, "  }\n\n"));
    return CDD_C_SUCCESS;
  }

  for (i = 0; i < op->n_parameters; ++i) {
    if (op->parameters[i].in == OA_PARAM_IN_QUERY) {
      const struct OpenAPI_Parameter *p = &op->parameters[i];
      enum OpenAPI_Style style =
          (p->style == OA_STYLE_UNKNOWN) ? OA_STYLE_FORM : p->style;
      int explode =
          p->explode_set ? p->explode : (style == OA_STYLE_FORM ? 1 : 0);

      if (!has_query) {
        if (qp_tracking) {
          CHECK_IO(fprintf(fp, "  if (!qp_initialized) {\n"));
          CHECK_IO(fprintf(fp, "    rc = url_query_init(&qp);\n"));
          CHECK_IO(fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
          CHECK_IO(fprintf(fp, "    qp_initialized = 1;\n"));
          CHECK_IO(fprintf(fp, "  }\n"));
        } else {
          CHECK_IO(fprintf(fp, "  rc = url_query_init(&qp);\n"));
          CHECK_IO(fprintf(fp, "  if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
        }
        has_query = 1;
      }

      CHECK_IO(fprintf(fp, "  /* Query Parameter: %s */\n", p->name));

      if (p->content_type && media_type_is_json_url(p->content_type)) {
        cdd_c_error_t rc2 = write_query_json_param(fp, p);
        if (rc2 != 0)
          return rc2;
        continue;
      }

      if (param_is_object_kv_url(p)) {
        cdd_c_error_t rc2 = write_query_object_param(fp, p);
        if (rc2 != 0)
          return rc2;
        continue;
      }

      if (p->is_array) {
        if (style == OA_STYLE_FORM) {
          if (explode) {
            /* === form + explode=true === */
            CHECK_IO(fprintf(fp, "  {\n    size_t i;\n"));
            CHECK_IO(fprintf(fp, "    for(i=0; i < %s_len; ++i) {\n", p->name));

            if (p->items_type) {
              if (strcmp(p->items_type, "string") == 0) {
                if (p->allow_reserved_set && p->allow_reserved) {
                  CHECK_IO(fprintf(fp,
                                   "      char *enc = NULL; "
                                   "url_encode_allow_reserved(%s[i], &enc);\n",
                                   p->name));
                  CHECK_IO(fprintf(fp,
                                   "      if (!enc) { rc = "
                                   "CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
                  CHECK_IO(fprintf(
                      fp,
                      "      rc = url_query_add_encoded(&qp, \"%s\", enc);\n",
                      p->name));
                  CHECK_IO(fprintf(fp, "      free(enc);\n"));
                } else {
                  CHECK_IO(fprintf(
                      fp, "      rc = url_query_add(&qp, \"%s\", %s[i]);\n",
                      p->name, p->name));
                }
              } else if (strcmp(p->items_type, "integer") == 0) {
                CHECK_IO(fprintf(fp, "      char num_buf[32];\n"));
                CHECK_IO(fprintf(fp,
                                 "      spr"
                                 "intf(num_buf, \"%%d\", %s[i]);\n",
                                 p->name));
                CHECK_IO(fprintf(
                    fp, "      rc = url_query_add(&qp, \"%s\", num_buf);\n",
                    p->name));
              } else if (strcmp(p->items_type, "number") == 0) {
                CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
                CHECK_IO(fprintf(fp,
                                 "      spr"
                                 "intf(num_buf, \"%%g\", %s[i]);\n",
                                 p->name));
                CHECK_IO(fprintf(
                    fp, "      rc = url_query_add(&qp, \"%s\", num_buf);\n",
                    p->name));
              } else if (strcmp(p->items_type, "boolean") == 0) {
                CHECK_IO(
                    fprintf(fp,
                            "      rc = url_query_add(&qp, \"%s\", %s[i] ? "
                            "\"true\" : \"false\");\n",
                            p->name, p->name));
              }
            }
            CHECK_IO(fprintf(
                fp,
                "      if (rc != CDD_C_SUCCESS) goto cleanup;\n    }\n  }\n"));
          } else {
            /* === form + explode=false (CSV) === */
            {
              const char *encode_fn =
                  (p->allow_reserved_set && p->allow_reserved)
                      ? "url_encode_allow_reserved"
                      : "url_encode";
              cdd_c_error_t rc2 =
                  write_joined_query_array(fp, p, ',', encode_fn, 1);
              if (rc2 != 0)
                return rc2;
            }
          }
        } else if (style == OA_STYLE_SPACE_DELIMITED) {
          /* === spaceDelimited (explode n/a) === */
          if (p->allow_reserved_set && p->allow_reserved) {
            cdd_c_error_t rc2 = write_joined_query_array_encoded_delim(
                fp, p, "%20", "url_encode_allow_reserved");
            if (rc2 != 0)
              return rc2;
          } else {
            cdd_c_error_t rc2 = write_joined_query_array(fp, p, ' ', NULL, 0);
            if (rc2 != 0)
              return rc2;
          }
        } else if (style == OA_STYLE_PIPE_DELIMITED) {
          /* === pipeDelimited (explode n/a) === */
          if (p->allow_reserved_set && p->allow_reserved) {
            cdd_c_error_t rc2 = write_joined_query_array_encoded_delim(
                fp, p, "%7C", "url_encode_allow_reserved");
            if (rc2 != 0)
              return rc2;
          } else {
            cdd_c_error_t rc2 = write_joined_query_array(fp, p, '|', NULL, 0);
            if (rc2 != 0)
              return rc2;
          }
        } else if (explode) {
          /* === fallback explode=true === */
          CHECK_IO(fprintf(fp, "  {\n    size_t i;\n"));
          CHECK_IO(fprintf(fp, "    for(i=0; i < %s_len; ++i) {\n", p->name));
          if (p->items_type) {
            if (strcmp(p->items_type, "string") == 0) {
              if (p->allow_reserved_set && p->allow_reserved) {
                CHECK_IO(fprintf(fp,
                                 "      char *enc = NULL; "
                                 "url_encode_allow_reserved(%s[i], &enc);\n",
                                 p->name));
                CHECK_IO(fprintf(fp,
                                 "      if (!enc) { rc = CDD_C_ERROR_MEMORY; "
                                 "goto cleanup; }\n"));
                CHECK_IO(fprintf(
                    fp, "      rc = url_query_add_encoded(&qp, \"%s\", enc);\n",
                    p->name));
                CHECK_IO(fprintf(fp, "      free(enc);\n"));
              } else {
                CHECK_IO(fprintf(
                    fp, "      rc = url_query_add(&qp, \"%s\", %s[i]);\n",
                    p->name, p->name));
              }
            } else if (strcmp(p->items_type, "integer") == 0) {
              CHECK_IO(fprintf(fp, "      char num_buf[32];\n"));
              CHECK_IO(fprintf(fp,
                               "      spr"
                               "intf(num_buf, \"%%d\", %s[i]);\n",
                               p->name));
              CHECK_IO(fprintf(
                  fp, "      rc = url_query_add(&qp, \"%s\", num_buf);\n",
                  p->name));
            } else if (strcmp(p->items_type, "number") == 0) {
              CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
              CHECK_IO(fprintf(fp,
                               "      spr"
                               "intf(num_buf, \"%%g\", %s[i]);\n",
                               p->name));
              CHECK_IO(fprintf(
                  fp, "      rc = url_query_add(&qp, \"%s\", num_buf);\n",
                  p->name));
            } else if (strcmp(p->items_type, "boolean") == 0) {
              CHECK_IO(fprintf(fp,
                               "      rc = url_query_add(&qp, \"%s\", %s[i] ? "
                               "\"true\" : \"false\");\n",
                               p->name, p->name));
            }
          }
          CHECK_IO(fprintf(
              fp,
              "      if (rc != CDD_C_SUCCESS) goto cleanup;\n    }\n  }\n"));
        } else {
          CHECK_IO(fprintf(fp, "  /* Array style not yet supported for %s */\n",
                           p->name));
        }
      } else {
        /* === Scalar === */
        if (strcmp(p->type, "string") == 0) {
          CHECK_IO(fprintf(fp, "  if (%s) {\n", p->name));
          if (p->allow_reserved_set && p->allow_reserved) {
            CHECK_IO(fprintf(
                fp,
                "    char *enc = NULL; url_encode_allow_reserved(%s, &enc);\n",
                p->name));
            CHECK_IO(fprintf(
                fp,
                "    if (!enc) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
            CHECK_IO(fprintf(
                fp, "    rc = url_query_add_encoded(&qp, \"%s\", enc);\n",
                p->name));
            CHECK_IO(fprintf(fp, "    free(enc);\n"));
          } else {
            CHECK_IO(fprintf(fp, "    rc = url_query_add(&qp, \"%s\", %s);\n",
                             p->name, p->name));
          }
          CHECK_IO(
              fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n  }\n"));
        } else if (strcmp(p->type, "integer") == 0) {
          CHECK_IO(fprintf(fp, "  {\n    char num_buf[32];\n"));
          CHECK_IO(fprintf(fp,
                           "    spr"
                           "intf(num_buf, \"%%d\", %s);\n",
                           p->name));
          CHECK_IO(fprintf(
              fp, "    rc = url_query_add(&qp, \"%s\", num_buf);\n", p->name));
          CHECK_IO(
              fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n  }\n"));
        } else if (strcmp(p->type, "number") == 0) {
          CHECK_IO(fprintf(fp, "  {\n    char num_buf[64];\n"));
          CHECK_IO(fprintf(fp,
                           "    spr"
                           "intf(num_buf, \"%%g\", %s);\n",
                           p->name));
          CHECK_IO(fprintf(
              fp, "    rc = url_query_add(&qp, \"%s\", num_buf);\n", p->name));
          CHECK_IO(
              fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n  }\n"));
        } else {
          CHECK_IO(fprintf(fp,
                           "  rc = url_query_add(&qp, \"%s\", %s ? \"true\" : "
                           "\"false\");\n",
                           p->name, p->name));
          CHECK_IO(fprintf(fp, "  if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
        }
      }
    }
  }

  if (has_query) {
    CHECK_IO(fprintf(fp, "  rc = url_query_build(&qp, &query_str);\n"));
    CHECK_IO(fprintf(fp, "  if (rc != CDD_C_SUCCESS) goto cleanup;\n\n"));
  } else if (qp_tracking) {
    CHECK_IO(fprintf(fp, "  if (qp_initialized) {\n"));
    CHECK_IO(fprintf(fp, "    rc = url_query_build(&qp, &query_str);\n"));
    CHECK_IO(fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
    CHECK_IO(fprintf(fp, "  }\n\n"));
  }

  return CDD_C_SUCCESS;
}
