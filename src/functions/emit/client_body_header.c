/**
 * @file client_body_header.c
 * @brief Header parameter serialization for client bodies.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "functions/emit/client_body_internal.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern int g_fail_io_after;
extern int g_io_calls;
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

#ifdef CDD_BUILD_TESTS
static int test_cdd_fputs_hook(const char *s, FILE *stream) {
  if (g_fail_io_after >= 0 && ++g_io_calls > g_fail_io_after)
    return -1;
  return fputs(s, stream);
}
#define fputs test_cdd_fputs_hook
#endif

/**
 * @brief Generates C code for write header param logic.
 */
cdd_c_error_t write_header_param_logic(FILE *fp,
                                       const struct OpenAPI_Operation *op) {
  size_t i;
  int is_json = 0;
  cdd_c_error_t rc_mt;

  if (!fp || !op)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < op->n_parameters; ++i) {
    if (op->parameters[i].in == OA_PARAM_IN_HEADER) {
      const struct OpenAPI_Parameter *p = &op->parameters[i];
      CHECK_IO(fprintf(fp, "  /* Header Parameter: %s */\n", p->name));
      is_json = 0;
      if (p->content_type) {
        media_type_is_json(p->content_type, &is_json);
      }
      if (p->content_type && is_json) {
        if (p->is_array) {

          const char *item_type =
              p->items_type ? p->items_type : p->schema.inline_type;
          int is_prim = 0;
          if (item_type) {
            rc_mt = is_primitive_type(item_type, &is_prim);
            if (rc_mt != CDD_C_SUCCESS)
              return rc_mt;
          }
          if (item_type && is_prim) {
            CHECK_IO(fprintf(

                fp, "  /* Header JSON array parameter (primitive): %s */\n",
                p->name));

            CHECK_IO(

                fprintf(fp, "  if (%s && %s_len > 0) {\n", p->name, p->name));

            CHECK_IO(fprintf(fp, "    JSON_Value *hdr_val = NULL;\n"));
            CHECK_IO(fprintf(fp, "    JSON_Array *hdr_arr = NULL;\n"));
            CHECK_IO(fprintf(fp, "    char *hdr_json = NULL;\n"));
            CHECK_IO(fprintf(fp, "    size_t i;\n"));
            CHECK_IO(fprintf(fp, "    hdr_val = json_value_init_array();\n"));
            CHECK_IO(fprintf(fp, "    if (!hdr_val) { rc = CDD_C_ERROR_MEMORY; "

                                 "goto cleanup; }\n"));

            CHECK_IO(

                fprintf(fp, "    hdr_arr = json_value_get_array(hdr_val);\n"));

            CHECK_IO(

                fprintf(fp, "    if (!hdr_arr) { rc = "
                            "CDD_C_ERROR_INVALID_ARGUMENT; goto cleanup; }\n"));

            CHECK_IO(

                fprintf(fp, "    for (i = 0; i < %s_len; ++i) {\n", p->name));

            if (strcmp(item_type, "string") == 0) {
              CHECK_IO(fprintf(fp, "      if (!%s[i]) {\n", p->name));
              CHECK_IO(fprintf(

                  fp,
                  "        if (json_array_append_null(hdr_arr) != JSONSuccess) "
                  "{ rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));

              CHECK_IO(fprintf(fp, "      } else {\n"));
              CHECK_IO(fprintf(

                  fp,
                  "        if (json_array_append_string(hdr_arr, %s[i]) != "
                  "JSONSuccess) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n",
                  p->name));

              CHECK_IO(fprintf(fp, "      }\n"));
            } else if (strcmp(item_type, "integer") == 0) {
              CHECK_IO(fprintf(

                  fp,
                  "      if (json_array_append_number(hdr_arr, (double)%s[i]) "
                  "!= JSONSuccess) { rc = CDD_C_ERROR_MEMORY; goto cleanup; "
                  "}\n",
                  p->name));

            } else if (strcmp(item_type, "number") == 0) {
              CHECK_IO(fprintf(

                  fp,
                  "      if (json_array_append_number(hdr_arr, %s[i]) != "
                  "JSONSuccess) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n",
                  p->name));

            } else {
              CHECK_IO(fprintf(

                  fp,
                  "      if (json_array_append_boolean(hdr_arr, %s[i] ? 1 : 0) "
                  "!= JSONSuccess) { rc = CDD_C_ERROR_MEMORY; goto cleanup; "
                  "}\n",
                  p->name));
            }

            CHECK_IO(fprintf(fp, "    }\n"));
            CHECK_IO(fprintf(

                fp, "    hdr_json = json_serialize_to_string(hdr_val);\n"));

            CHECK_IO(fprintf(fp, "    json_value_free(hdr_val);\n"));
            CHECK_IO(fprintf(fp, "    if (!hdr_json) { rc = "

                                 "CDD_C_ERROR_MEMORY; goto cleanup; }\n"));

            CHECK_IO(fprintf(

                fp,
                "    rc = http_headers_add(&req.headers, \"%s\", hdr_json);\n",
                p->name));

            CHECK_IO(

                fprintf(fp, "    json_free_serialized_string(hdr_json);\n"));

            CHECK_IO(
                fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
            CHECK_IO(fprintf(fp, "  }\n"));
          } else if (item_type && strcmp(item_type, "object") != 0) {
            CHECK_IO(fprintf(

                fp, "  /* Header JSON array parameter (object refs): %s */\n",
                p->name));

            CHECK_IO(

                fprintf(fp, "  if (%s && %s_len > 0) {\n", p->name, p->name));

            CHECK_IO(fprintf(fp, "    JSON_Value *hdr_val = NULL;\n"));
            CHECK_IO(fprintf(fp, "    JSON_Array *hdr_arr = NULL;\n"));
            CHECK_IO(fprintf(fp, "    char *hdr_json = NULL;\n"));
            CHECK_IO(fprintf(fp, "    size_t i;\n"));
            CHECK_IO(fprintf(fp, "    hdr_val = json_value_init_array();\n"));
            CHECK_IO(fprintf(fp, "    if (!hdr_val) { rc = CDD_C_ERROR_MEMORY; "

                                 "goto cleanup; }\n"));

            CHECK_IO(

                fprintf(fp, "    hdr_arr = json_value_get_array(hdr_val);\n"));

            CHECK_IO(

                fprintf(fp, "    if (!hdr_arr) { rc = "
                            "CDD_C_ERROR_INVALID_ARGUMENT; goto cleanup; }\n"));

            CHECK_IO(

                fprintf(fp, "    for (i = 0; i < %s_len; ++i) {\n", p->name));

            CHECK_IO(fprintf(fp, "      char *item_json = NULL;\n"));
            CHECK_IO(fprintf(fp, "      JSON_Value *item_val = NULL;\n"));
            CHECK_IO(fprintf(fp, "      if (!%s[i]) {\n", p->name));
            CHECK_IO(fprintf(

                fp,
                "        if (json_array_append_null(hdr_arr) != JSONSuccess) "
                "{ rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));

            CHECK_IO(fprintf(fp, "        continue;\n"));
            CHECK_IO(fprintf(fp, "      }\n"));
            CHECK_IO(fprintf(fp, "      rc = %s_to_json(%s[i], &item_json);\n",

                             item_type, p->name));

            CHECK_IO(
                fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
            CHECK_IO(fprintf(

                fp, "      item_val = json_parse_string(item_json);\n"));

            CHECK_IO(fprintf(fp, "      free(item_json);\n"));
            CHECK_IO(

                fprintf(fp, "      if (!item_val) { rc = "
                            "CDD_C_ERROR_INVALID_ARGUMENT; goto cleanup; }\n"));

            CHECK_IO(fprintf(

                fp, "      if (json_array_append_value(hdr_arr, item_val) != "
                    "JSONSuccess) { json_value_free(item_val); rc = "
                    "CDD_C_ERROR_MEMORY; goto "
                    "cleanup; }\n"));

            CHECK_IO(fprintf(fp, "    }\n"));
            CHECK_IO(fprintf(

                fp, "    hdr_json = json_serialize_to_string(hdr_val);\n"));

            CHECK_IO(fprintf(fp, "    json_value_free(hdr_val);\n"));
            CHECK_IO(fprintf(fp, "    if (!hdr_json) { rc = "

                                 "CDD_C_ERROR_MEMORY; goto cleanup; }\n"));

            CHECK_IO(fprintf(

                fp,
                "    rc = http_headers_add(&req.headers, \"%s\", hdr_json);\n",
                p->name));

            CHECK_IO(

                fprintf(fp, "    json_free_serialized_string(hdr_json);\n"));

            CHECK_IO(
                fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
            CHECK_IO(fprintf(fp, "  }\n"));

          } else {

            CHECK_IO(fprintf(

                fp, "  /* Unsupported JSON header array parameter for %s */\n",
                p->name));
          }
        } else {
          const char *ref_name = p->schema.ref_name;
          int is_prim = 0;
          if (p->type) {
            rc_mt = is_primitive_type(p->type, &is_prim);
            if (rc_mt != CDD_C_SUCCESS)
              return rc_mt;
          }
          if (!ref_name && p->type && !is_prim &&
              strcmp(p->type, "object") != 0 && strcmp(p->type, "array") != 0)
            ref_name = p->type;

          if (ref_name) {
            CHECK_IO(fprintf(fp, "  if (%s) {\n", p->name));
            CHECK_IO(fprintf(fp, "    char *hdr_json = NULL;\n"));
            CHECK_IO(fprintf(fp, "    rc = %s_to_json(%s, &hdr_json);\n",
                             ref_name, p->name));
            CHECK_IO(
                fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
            CHECK_IO(fprintf(
                fp,
                "    rc = http_headers_add(&req.headers, \"%s\", hdr_json);\n",
                p->name));
            CHECK_IO(fprintf(fp, "    free(hdr_json);\n"));
            CHECK_IO(
                fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
            CHECK_IO(fprintf(fp, "  }\n"));

          } else if (p->type && strcmp(p->type, "object") == 0) {
            CHECK_IO(

                fprintf(fp, "  if (%s && %s_len > 0) {\n", p->name, p->name));

            CHECK_IO(fprintf(fp, "    JSON_Value *hdr_val = NULL;\n"));
            CHECK_IO(fprintf(fp, "    JSON_Object *hdr_obj = NULL;\n"));
            CHECK_IO(fprintf(fp, "    char *hdr_json = NULL;\n"));
            CHECK_IO(fprintf(fp, "    size_t i;\n"));
            CHECK_IO(fprintf(fp, "    hdr_val = json_value_init_object();\n"));
            CHECK_IO(fprintf(fp, "    if (!hdr_val) { rc = CDD_C_ERROR_MEMORY; "

                                 "goto cleanup; }\n"));

            CHECK_IO(

                fprintf(fp, "    hdr_obj = json_value_get_object(hdr_val);\n"));

            CHECK_IO(fprintf(fp, "    if (!hdr_obj) { "

                                 "json_value_free(hdr_val); rc = "
                                 "CDD_C_ERROR_INVALID_ARGUMENT; goto "
                                 "cleanup; }\n"));

            CHECK_IO(

                fprintf(fp, "    for (i = 0; i < %s_len; ++i) {\n", p->name));

            CHECK_IO(fprintf(

                fp, "      const struct OpenAPI_KV *kv = &%s[i];\n", p->name));

            CHECK_IO(fprintf(fp, "      const char *kv_key = kv->key;\n"));
            CHECK_IO(fprintf(fp, "      if (!kv_key) continue;\n"));
            CHECK_IO(fprintf(fp, "      switch (kv->type) {\n"));
            CHECK_IO(

                fprintf(fp, "      case OA_KV_STRING:\n"
                            "        if (kv->value.s) {\n"
                            "          json_object_set_string(hdr_obj, kv_key, "
                            "kv->value.s);\n"
                            "        } else {\n"
                            "          json_object_set_null(hdr_obj, kv_key);\n"
                            "        }\n"
                            "        break;\n"));

            CHECK_IO(fprintf(fp,

                             "      case OA_KV_INTEGER:\n"
                             "        json_object_set_number(hdr_obj, kv_key, "
                             "(double)kv->value.i);\n"
                             "        break;\n"));

            CHECK_IO(fprintf(fp,

                             "      case OA_KV_NUMBER:\n"
                             "        json_object_set_number(hdr_obj, kv_key, "
                             "kv->value.n);\n"
                             "        break;\n"));

            CHECK_IO(fprintf(fp,

                             "      case OA_KV_BOOLEAN:\n"
                             "        json_object_set_boolean(hdr_obj, kv_key, "
                             "kv->value.b ? 1 : 0);\n"
                             "        break;\n"));

            CHECK_IO(fprintf(fp,

                             "      default:\n"
                             "        json_object_set_null(hdr_obj, kv_key);\n"
                             "        break;\n"));

            CHECK_IO(fprintf(fp, "      }\n"));
            CHECK_IO(fprintf(fp, "    }\n"));
            CHECK_IO(fprintf(

                fp, "    hdr_json = json_serialize_to_string(hdr_val);\n"));

            CHECK_IO(fprintf(fp, "    json_value_free(hdr_val);\n"));
            CHECK_IO(fprintf(fp, "    if (!hdr_json) { rc = "

                                 "CDD_C_ERROR_MEMORY; goto cleanup; }\n"));

            CHECK_IO(fprintf(

                fp,
                "    rc = http_headers_add(&req.headers, \"%s\", hdr_json);\n",
                p->name));

            CHECK_IO(

                fprintf(fp, "    json_free_serialized_string(hdr_json);\n"));

            CHECK_IO(
                fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
            CHECK_IO(fprintf(fp, "  }\n"));

          } else {

            const char *prim = p->type ? p->type : p->schema.inline_type;
            int prim_is_p = 0;
            if (prim) {
              rc_mt = is_primitive_type(prim, &prim_is_p);
              if (rc_mt != CDD_C_SUCCESS)
                return rc_mt;
            }
            if (prim && prim_is_p) {
              CHECK_IO(

                  fprintf(fp, "  /* Header JSON parameter (primitive): %s */\n",
                          p->name));

              if (strcmp(prim, "string") == 0) {
                CHECK_IO(fprintf(fp, "  if (%s) {\n", p->name));

              } else {

                CHECK_IO(fprintf(fp, "  {\n"));
              }

              CHECK_IO(fprintf(fp, "    JSON_Value *hdr_val = NULL;\n"));
              CHECK_IO(fprintf(fp, "    char *hdr_json = NULL;\n"));
              if (strcmp(prim, "string") == 0) {
                CHECK_IO(fprintf(fp,

                                 "    hdr_val = json_value_init_string(%s);\n",
                                 p->name));

              } else if (strcmp(prim, "integer") == 0) {
                CHECK_IO(fprintf(

                    fp, "    hdr_val = json_value_init_number((double)%s);\n",
                    p->name));

              } else if (strcmp(prim, "number") == 0) {
                CHECK_IO(fprintf(fp,

                                 "    hdr_val = json_value_init_number(%s);\n",
                                 p->name));

              } else {
                CHECK_IO(fprintf(

                    fp, "    hdr_val = json_value_init_boolean(%s ? 1 : 0);\n",
                    p->name));
              }

              CHECK_IO(fprintf(fp, "    if (!hdr_val) { rc = "

                                   "CDD_C_ERROR_MEMORY; goto cleanup; }\n"));

              CHECK_IO(fprintf(

                  fp, "    hdr_json = json_serialize_to_string(hdr_val);\n"));

              CHECK_IO(fprintf(fp, "    json_value_free(hdr_val);\n"));
              CHECK_IO(fprintf(fp, "    if (!hdr_json) { rc = "

                                   "CDD_C_ERROR_MEMORY; goto cleanup; }\n"));

              CHECK_IO(fprintf(fp,

                               "    rc = http_headers_add(&req.headers, "
                               "\"%s\", hdr_json);\n",
                               p->name));

              CHECK_IO(

                  fprintf(fp, "    json_free_serialized_string(hdr_json);\n"));

              CHECK_IO(
                  fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
              if (strcmp(prim, "string") == 0) {
                CHECK_IO(fprintf(fp, "  }\n"));

              } else {

                CHECK_IO(fprintf(fp, "  }\n"));
              }
            } else {

              CHECK_IO(fprintf(

                  fp, "  /* Unsupported JSON header parameter for %s */\n",
                  p->name));
            }
          }
        }
        continue;
      }
      if (p->is_array) {
        const char *item_type = p->items_type ? p->items_type : "string";
        CHECK_IO(fprintf(fp, "  {\n    size_t i;\n"));
        CHECK_IO(fprintf(fp, "    char *joined = NULL;\n"));
        CHECK_IO(fprintf(fp, "    size_t joined_len = 0;\n"));
        CHECK_IO(fprintf(fp, "    for(i=0; i < %s_len; ++i) {\n", p->name));
        if (strcmp(item_type, "integer") == 0) {
          CHECK_IO(fprintf(fp, "      const char *raw;\n"));
          CHECK_IO(fprintf(fp, "      char num_buf[32];\n"));
          CHECK_IO(fprintf(fp,
                           "      spr"
                           "intf(num_buf, \"%%d\", %s[i]);\n",
                           p->name));
          CHECK_IO(fprintf(fp, "      raw = num_buf;\n"));

        } else if (strcmp(item_type, "number") == 0) {
          CHECK_IO(fprintf(fp, "      const char *raw;\n"));
          CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
          CHECK_IO(fprintf(fp,
                           "      spr"
                           "intf(num_buf, \"%%g\", %s[i]);\n",

                           p->name));

          CHECK_IO(fprintf(fp, "      raw = num_buf;\n"));
        } else if (strcmp(item_type, "boolean") == 0) {
          CHECK_IO(fprintf(fp, "      const char *raw;\n"));
          CHECK_IO(fprintf(fp, "      raw = %s[i] ? \"true\" : \"false\";\n",

                           p->name));
        } else {

          CHECK_IO(fprintf(fp, "      const char *raw;\n"));
          CHECK_IO(fprintf(fp, "      raw = %s[i];\n", p->name));
        }
        CHECK_IO(fprintf(fp, "      if (raw) {\n"));
        CHECK_IO(fprintf(fp, "        size_t val_len = strlen(raw);\n"));
        CHECK_IO(fprintf(
            fp,
            "        size_t extra = val_len + (joined_len > 0 ? 1 : 0);\n"));
        CHECK_IO(fprintf(fp,
                         "        char *tmp = (char *)(size_t)realloc(joined, "
                         "joined_len + extra + 1);\n"));
        CHECK_IO(fprintf(
            fp,
            "        if (!tmp) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
        CHECK_IO(fprintf(fp, "        joined = tmp;\n"));
        CHECK_IO(fprintf(
            fp, "        if (joined_len > 0) joined[joined_len++] = ',';\n"));
        CHECK_IO(fprintf(
            fp, "        memcpy(joined + joined_len, raw, val_len);\n"));
        CHECK_IO(fprintf(fp, "        joined_len += val_len;\n"));
        CHECK_IO(fprintf(fp, "        joined[joined_len] = '\\0';\n"));
        CHECK_IO(fprintf(fp, "      }\n"));
        CHECK_IO(fprintf(fp, "    }\n"));
        CHECK_IO(fprintf(fp, "    if (joined) {\n"));
        CHECK_IO(fprintf(
            fp, "      rc = http_headers_add(&req.headers, \"%s\", joined);\n",
            p->name));
        CHECK_IO(fprintf(fp, "      free(joined);\n"));
        CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
        CHECK_IO(fprintf(fp, "    }\n"));
        CHECK_IO(fprintf(fp, "  }\n"));
      } else if (strcmp(p->type, "object") == 0) {
        int explode = p->explode_set ? p->explode : 0;
        CHECK_IO(fprintf(fp, "  {\n    size_t i;\n"));
        CHECK_IO(fprintf(fp, "    char *joined = NULL;\n"));
        CHECK_IO(fprintf(fp, "    size_t joined_len = 0;\n"));
        CHECK_IO(fprintf(fp, "    int first = 1;\n"));
        CHECK_IO(fprintf(fp, "    for(i=0; i < %s_len; ++i) {\n", p->name));
        CHECK_IO(fprintf(fp, "      const struct OpenAPI_KV *kv = &%s[i];\n",
                         p->name));
        CHECK_IO(fprintf(fp, "      const char *kv_key = kv->key;\n"));
        CHECK_IO(fprintf(fp, "      const char *kv_raw = NULL;\n"));
        CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
        CHECK_IO(fprintf(fp, "      switch (kv->type) {\n"));
        CHECK_IO(fprintf(fp, "      case OA_KV_STRING:\n"));
        CHECK_IO(
            fprintf(fp, "        kv_raw = kv->value.s;\n        break;\n"));
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
        CHECK_IO(
            fprintf(fp, "      case OA_KV_BOOLEAN:\n"
                        "        kv_raw = kv->value.b ? \"true\" : \"false\";\n"
                        "        break;\n"));
        CHECK_IO(fprintf(fp, "      default:\n"
                             "        kv_raw = NULL;\n"
                             "        break;\n"));
        CHECK_IO(fprintf(fp, "      }\n"));
        CHECK_IO(fprintf(fp, "      if (!kv_key || !kv_raw) continue;\n"));
        CHECK_IO(fprintf(fp, "      {\n"));
        if (explode) {
          CHECK_IO(fputs(
              "        size_t key_len = strlen(kv_key);\n"
              "        size_t val_len = strlen(kv_raw);\n"
              "        size_t extra = key_len + val_len + 1 + "
              "(first ? 0 : 1);\n"
              "        char *tmp = (char *)(size_t)realloc(joined, "
              "joined_len + extra + 1);\n"
              "        if (!tmp) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"
              "        joined = tmp;\n",
              fp));
          CHECK_IO(
              fputs("        if (!first) joined[joined_len++] = ',';\n"
                    "        memcpy(joined + joined_len, kv_key, key_len);\n"
                    "        joined_len += key_len;\n"
                    "        joined[joined_len++] = '=';\n"
                    "        memcpy(joined + joined_len, kv_raw, val_len);\n"
                    "        joined_len += val_len;\n"
                    "        joined[joined_len] = '\\0';\n",
                    fp));
        } else {

          CHECK_IO(fputs(

              "        size_t key_len = strlen(kv_key);\n"
              "        size_t val_len = strlen(kv_raw);\n"
              "        size_t extra = key_len + val_len + 1 + "
              "(first ? 0 : 1) + 1;\n"
              "        char *tmp = (char *)(size_t)realloc(joined, "
              "joined_len + extra + 1);\n"
              "        if (!tmp) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"
              "        joined = tmp;\n",
              fp));

          CHECK_IO(

              fputs("        if (!first) joined[joined_len++] = ',';\n"
                    "        memcpy(joined + joined_len, kv_key, key_len);\n"
                    "        joined_len += key_len;\n"
                    "        joined[joined_len++] = ',';\n"
                    "        memcpy(joined + joined_len, kv_raw, val_len);\n"
                    "        joined_len += val_len;\n"
                    "        joined[joined_len] = '\\0';\n",
                    fp));
        }
        CHECK_IO(fprintf(fp, "      }\n"));
        CHECK_IO(fprintf(fp, "      first = 0;\n"));
        CHECK_IO(fprintf(fp, "    }\n"));
        CHECK_IO(fprintf(fp, "    if (joined) {\n"));
        CHECK_IO(fprintf(
            fp, "      rc = http_headers_add(&req.headers, \"%s\", joined);\n",
            p->name));
        CHECK_IO(fprintf(fp, "      free(joined);\n"));
        CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
        CHECK_IO(fprintf(fp, "    }\n"));
        CHECK_IO(fprintf(fp, "  }\n"));
      } else if (strcmp(p->type, "string") == 0) {

        CHECK_IO(fprintf(fp, "  if (%s) {\n", p->name));
        CHECK_IO(fprintf(

            fp, "    rc = http_headers_add(&req.headers, \"%s\", %s);\n",
            p->name, p->name));

        CHECK_IO(fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
        CHECK_IO(fprintf(fp, "  }\n"));

      } else if (strcmp(p->type, "integer") == 0) {

        CHECK_IO(fprintf(fp, "  {\n    char num_buf[32];\n"));
        CHECK_IO(fprintf(fp,
                         "    spr"
                         "intf(num_buf, \"%%d\", %s);\n",
                         p->name));
        CHECK_IO(fprintf(

            fp, "    rc = http_headers_add(&req.headers, \"%s\", num_buf);\n",
            p->name));

        CHECK_IO(
            fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n  }\n"));

      } else if (strcmp(p->type, "number") == 0) {
        CHECK_IO(fprintf(fp, "  {\n    char num_buf[64];\n"));
        CHECK_IO(fprintf(fp,
                         "    spr"
                         "intf(num_buf, \"%%g\", %s);\n",
                         p->name));
        CHECK_IO(fprintf(
            fp, "    rc = http_headers_add(&req.headers, \"%s\", num_buf);\n",
            p->name));
        CHECK_IO(
            fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n  }\n"));

      } else if (strcmp(p->type, "boolean") == 0) {
        CHECK_IO(fprintf(fp,

                         "  rc = http_headers_add(&req.headers, \"%s\", %s ? "
                         "\"true\" : \"false\");\n",
                         p->name, p->name));

        CHECK_IO(fprintf(fp, "  if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
      }
    }
  }
  return CDD_C_SUCCESS;
}
