/**
 * @file client_body_multipart.c
 * @brief Multipart form data request body serialization for client bodies.
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

/**
 * @brief Generates C code for write multipart part headers.
 */
cdd_c_error_t
client_body_write_multipart_part_headers(FILE *fp,
                                         const struct OpenAPI_Encoding *enc) {
  size_t h;
  if (!fp)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!enc || !enc->headers || enc->n_headers == 0 || !enc->name)
    return CDD_C_SUCCESS;

  for (h = 0; h < enc->n_headers; ++h) {
    const struct OpenAPI_Header *hdr = &enc->headers[h];
    const char *hdr_type = hdr->type ? hdr->type : "string";
    int hdr_is_array = hdr->is_array || (strcmp(hdr_type, "array") == 0);
    char param_name[256];
    char joined_name[300];
    char joined_len_name[300];
    char idx_name[300];
    char first_name[300];
    int explode = hdr->explode_set ? hdr->explode : 0;
    int is_ct = 0;

    if (!hdr->name)
      continue;

    client_body_header_name_is_content_type(hdr->name, &is_ct);
    if (is_ct)
      continue;

    client_body_multipart_header_param_name(param_name, sizeof(param_name),
                                            enc->name, hdr->name);

    CDD_SNPRINTF(joined_name, sizeof(joined_name), "%s_joined", param_name);
    CDD_SNPRINTF(joined_len_name, sizeof(joined_len_name), "%s_joined_len",
                 param_name);
    CDD_SNPRINTF(idx_name, sizeof(idx_name), "%s_i", param_name);
    CDD_SNPRINTF(first_name, sizeof(first_name), "%s_first", param_name);

    if (hdr_is_array) {
      const char *item_type = hdr->items_type ? hdr->items_type : "string";
      CHECK_IO(fprintf(fp, "      {\n"));
      CHECK_IO(fprintf(fp, "        size_t %s;\n", idx_name));
      CHECK_IO(fprintf(fp, "        char *%s = NULL;\n", joined_name));
      CHECK_IO(fprintf(fp, "        size_t %s = 0;\n", joined_len_name));
      CHECK_IO(fprintf(fp, "        for (%s = 0; %s < %s_len; ++%s) {\n",
                       idx_name, idx_name, param_name, idx_name));
      CHECK_IO(fprintf(fp, "          const char *raw = NULL;\n"));
      CHECK_IO(fprintf(fp, "          char num_buf[64];\n"));
      if (strcmp(item_type, "integer") == 0) {
        CHECK_IO(fprintf(fp,
                         "          spr"
                         "intf(num_buf, \"%%d\", %s[%s]);\n",
                         param_name, idx_name));
        CHECK_IO(fprintf(fp, "          raw = num_buf;\n"));

      } else if (strcmp(item_type, "number") == 0) {
        CHECK_IO(fprintf(fp,
                         "          spr"
                         "intf(num_buf, \"%%g\", %s[%s]);\n",

                         param_name, idx_name));

        CHECK_IO(fprintf(fp, "          raw = num_buf;\n"));
      } else if (strcmp(item_type, "boolean") == 0) {
        CHECK_IO(fprintf(fp, "          raw = %s[%s] ? \"true\" : \"false\";\n",

                         param_name, idx_name));
      } else {

        CHECK_IO(

            fprintf(fp, "          raw = %s[%s];\n", param_name, idx_name));
      }
      CHECK_IO(fprintf(fp, "          if (raw) {\n"));
      CHECK_IO(fprintf(fp, "            size_t val_len = strlen(raw);\n"));
      CHECK_IO(fprintf(fp,
                       "            size_t extra = val_len + "
                       "(%s > 0 ? 1 : 0);\n",
                       joined_len_name));
      CHECK_IO(
          fprintf(fp,
                  "            char *tmp = (char *)(size_t)realloc(%s, %s + "
                  "extra + 1);\n",
                  joined_name, joined_len_name));
      CHECK_IO(fprintf(fp, "            if (!tmp) { rc = CDD_C_ERROR_MEMORY; "
                           "goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "            %s = tmp;\n", joined_name));
      CHECK_IO(fprintf(fp, "            if (%s > 0) %s[%s++] = ',';\n",
                       joined_len_name, joined_name, joined_len_name));
      CHECK_IO(fprintf(fp, "            memcpy(%s + %s, raw, val_len);\n",
                       joined_name, joined_len_name));
      CHECK_IO(fprintf(fp, "            %s += val_len;\n", joined_len_name));
      CHECK_IO(fprintf(fp, "            %s[%s] = '\\0';\n", joined_name,
                       joined_len_name));
      CHECK_IO(fprintf(fp, "          }\n"));
      CHECK_IO(fprintf(fp, "        }\n"));
      CHECK_IO(fprintf(fp, "        if (%s) {\n", joined_name));
      CHECK_IO(fprintf(fp,
                       "          rc = http_request_add_part_header_last(&req, "
                       "\"%s\", %s);\n",
                       hdr->name, joined_name));
      CHECK_IO(fprintf(fp, "          free(%s);\n", joined_name));
      CHECK_IO(
          fprintf(fp, "          if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
      CHECK_IO(fprintf(fp, "        }\n"));
      CHECK_IO(fprintf(fp, "      }\n"));
    } else if (strcmp(hdr_type, "object") == 0) {

      CHECK_IO(fprintf(fp, "      {\n"));
      CHECK_IO(fprintf(fp, "        size_t %s;\n", idx_name));
      CHECK_IO(fprintf(fp, "        char *%s = NULL;\n", joined_name));
      CHECK_IO(fprintf(fp, "        size_t %s = 0;\n", joined_len_name));
      CHECK_IO(fprintf(fp, "        int %s = 1;\n", first_name));
      CHECK_IO(fprintf(fp, "        for (%s = 0; %s < %s_len; ++%s) {\n",

                       idx_name, idx_name, param_name, idx_name));

      CHECK_IO(fprintf(fp, "          const struct OpenAPI_KV *kv = &%s[%s];\n",

                       param_name, idx_name));

      CHECK_IO(fprintf(fp, "          const char *kv_key = kv->key;\n"));
      CHECK_IO(fprintf(fp, "          const char *kv_raw = NULL;\n"));
      CHECK_IO(fprintf(fp, "          char num_buf[64];\n"));
      CHECK_IO(fprintf(fp, "          switch (kv->type) {\n"));
      CHECK_IO(fprintf(fp, "          case OA_KV_STRING:\n"));
      CHECK_IO(fprintf(

          fp, "            kv_raw = kv->value.s;\n            break;\n"));

      CHECK_IO(fprintf(fp,

                       "          case OA_KV_INTEGER:\n"
                       "            spr"
                       "intf(num_buf, \"%%d\", kv->value.i);\n"
                       "            kv_raw = num_buf;\n"
                       "            break;\n"));

      CHECK_IO(fprintf(fp,

                       "          case OA_KV_NUMBER:\n"
                       "            spr"
                       "intf(num_buf, \"%%g\", kv->value.n);\n"
                       "            kv_raw = num_buf;\n"
                       "            break;\n"));

      CHECK_IO(fprintf(

          fp, "          case OA_KV_BOOLEAN:\n"
              "            kv_raw = kv->value.b ? \"true\" : \"false\";\n"
              "            break;\n"));

      CHECK_IO(fprintf(fp, "          default:\n"

                           "            kv_raw = NULL;\n"
                           "            break;\n"));

      CHECK_IO(fprintf(fp, "          }\n"));
      CHECK_IO(fprintf(fp, "          if (!kv_key || !kv_raw) continue;\n"));
      CHECK_IO(fprintf(fp, "          {\n"));
      if (explode) {
        CHECK_IO(fprintf(

            fp,
            "            size_t key_len = strlen(kv_key);\n"
            "            size_t val_len = strlen(kv_raw);\n"
            "            size_t extra = key_len + val_len + 1 + (%s ? 0 : 1);\n"
            "            char *tmp = (char *)(size_t)realloc(%s, %s + extra + "
            "1);\n"
            "            if (!tmp) { rc = CDD_C_ERROR_MEMORY; goto cleanup; "
            "}\n",
            first_name, joined_name, joined_len_name));

        CHECK_IO(fprintf(fp,

                         "            %s = tmp;\n"
                         "            if (!%s) %s[%s++] = ',';\n"
                         "            memcpy(%s + %s, kv_key, key_len);\n"
                         "            %s += key_len;\n"
                         "            %s[%s++] = '=';\n"
                         "            memcpy(%s + %s, kv_raw, val_len);\n"
                         "            %s += val_len;\n"
                         "            %s[%s] = '\\0';\n",
                         joined_name, first_name, joined_name, joined_len_name,
                         joined_name, joined_len_name, joined_len_name,
                         joined_name, joined_len_name, joined_name,
                         joined_len_name, joined_len_name, joined_name,
                         joined_len_name));
      } else {

        CHECK_IO(fprintf(

            fp,
            "            size_t key_len = strlen(kv_key);\n"
            "            size_t val_len = strlen(kv_raw);\n"
            "            size_t extra = key_len + val_len + 1 + (%s ? 0 : 1) + "
            "1;\n"
            "            char *tmp = (char *)(size_t)realloc(%s, %s + extra + "
            "1);\n"
            "            if (!tmp) { rc = CDD_C_ERROR_MEMORY; goto cleanup; "
            "}\n",
            first_name, joined_name, joined_len_name));

        CHECK_IO(fprintf(fp,

                         "            %s = tmp;\n"
                         "            if (!%s) %s[%s++] = ',';\n"
                         "            memcpy(%s + %s, kv_key, key_len);\n"
                         "            %s += key_len;\n"
                         "            %s[%s++] = ',';\n"
                         "            memcpy(%s + %s, kv_raw, val_len);\n"
                         "            %s += val_len;\n"
                         "            %s[%s] = '\\0';\n",
                         joined_name, first_name, joined_name, joined_len_name,
                         joined_name, joined_len_name, joined_len_name,
                         joined_name, joined_len_name, joined_name,
                         joined_len_name, joined_len_name, joined_name,
                         joined_len_name));
      }

      CHECK_IO(fprintf(fp, "          }\n"));
      CHECK_IO(fprintf(fp, "          %s = 0;\n", first_name));
      CHECK_IO(fprintf(fp, "        }\n"));
      CHECK_IO(fprintf(fp, "        if (%s) {\n", joined_name));
      CHECK_IO(fprintf(fp,

                       "          rc = http_request_add_part_header_last(&req, "
                       "\"%s\", %s);\n",
                       hdr->name, joined_name));

      CHECK_IO(fprintf(fp, "          free(%s);\n", joined_name));
      CHECK_IO(
          fprintf(fp, "          if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
      CHECK_IO(fprintf(fp, "        }\n"));
      CHECK_IO(fprintf(fp, "      }\n"));

    } else if (strcmp(hdr_type, "integer") == 0) {
      CHECK_IO(fprintf(fp, "      {\n        char num_buf[32];\n"));
      CHECK_IO(

          fprintf(fp,
                  "        spr"
                  "intf(num_buf, \"%%d\", %s);\n",
                  param_name));

      CHECK_IO(fprintf(fp,

                       "        rc = http_request_add_part_header_last(&req, "
                       "\"%s\", num_buf);\n",
                       hdr->name));

      CHECK_IO(fprintf(fp, "        if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
      CHECK_IO(fprintf(fp, "      }\n"));
    } else if (strcmp(hdr_type, "number") == 0) {
      CHECK_IO(fprintf(fp, "      {\n        char num_buf[64];\n"));
      CHECK_IO(

          fprintf(fp,
                  "        spr"
                  "intf(num_buf, \"%%g\", %s);\n",
                  param_name));

      CHECK_IO(fprintf(fp,

                       "        rc = http_request_add_part_header_last(&req, "
                       "\"%s\", num_buf);\n",
                       hdr->name));

      CHECK_IO(fprintf(fp, "        if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
      CHECK_IO(fprintf(fp, "      }\n"));
    } else if (strcmp(hdr_type, "boolean") == 0) {
      CHECK_IO(fprintf(fp,

                       "      rc = http_request_add_part_header_last(&req, "
                       "\"%s\", %s ? \"true\" : \"false\");\n",
                       hdr->name, param_name));

      CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));

    } else {

      CHECK_IO(fprintf(fp, "      if (%s) {\n", param_name));
      CHECK_IO(fprintf(fp,

                       "        rc = http_request_add_part_header_last(&req, "
                       "\"%s\", %s);\n",
                       hdr->name, param_name));

      CHECK_IO(fprintf(fp, "        if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
      CHECK_IO(fprintf(fp, "      }\n"));
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write multipart body.
 */
cdd_c_error_t
client_body_write_multipart_body(FILE *fp, const struct OpenAPI_Operation *op,
                                 const struct OpenAPI_Spec *spec) {
  struct StructFields *_ast_openapi_spec_find_schema_for_ref_7;
  const struct OpenAPI_MediaType *_ast_find_media_type_8;
  struct OpenAPI_Encoding *_ast_find_encoding_9;
  const char *_ast_first_content_type_entry_10 = NULL;
  const char *_ast_first_content_type_entry_11 = NULL;
  const char *_ast_first_content_type_entry_12 = NULL;
  const char *_ast_first_content_type_entry_13 = NULL;
  const char *_ast_first_content_type_entry_14 = NULL;
  const char *_ast_first_content_type_entry_15 = NULL;
  const struct StructFields *sf;
  const struct OpenAPI_MediaType *mt;
  size_t i;

  if (!fp || !op || !spec)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  sf = (openapi_spec_find_schema_for_ref(
            spec, &op->req_body, &_ast_openapi_spec_find_schema_for_ref_7),
        _ast_openapi_spec_find_schema_for_ref_7);
  if (!sf) {
    CHECK_IO(fprintf(
        fp,
        "  /* Warning: Schema %s definition not found, skipping multipart */\n",
        op->req_body.ref_name));
    return CDD_C_SUCCESS;
  }

  CHECK_IO(fprintf(fp, "  /* Multipart Body Construction */\n"));
  mt = (find_media_type(op->req_body_media_types, op->n_req_body_media_types,
                        "multipart/form-data", &_ast_find_media_type_8),
        _ast_find_media_type_8);
  for (i = 0; i < sf->size; ++i) {
    const struct StructField *f = &sf->fields[i];
    const struct OpenAPI_Encoding *enc =
        (mt != NULL) ? (find_encoding(mt, f->name, &_ast_find_encoding_9),
                        _ast_find_encoding_9)
                     : NULL;
    if (strcmp(f->type, "array") == 0) {
      const char *items_type = f->ref[0] != '\0' ? f->ref : "string";
      int items_is_object = 0;
      const char *content_type =
          (enc && enc->content_type) ? enc->content_type : NULL;
      const char *final_ct = content_type;
      char ct_buf[512];
      char ct_clean[256];
      const char *ct_arg = "NULL";
      char len_field[80];

      client_body_is_object_ref_type(items_type, &items_is_object);

      if (!final_ct && items_is_object)
        final_ct = "application/json";
      if (final_ct && final_ct[0] != '\0') {
        final_ct =
            (first_content_type_entry(final_ct, ct_clean, sizeof(ct_clean),
                                      &_ast_first_content_type_entry_10),
             _ast_first_content_type_entry_10);
        CDD_SNPRINTF(ct_buf, sizeof(ct_buf), "\"%s\"", final_ct);
        ct_arg = ct_buf;
      }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      sprintf_s(len_field, sizeof(len_field), "n_%s", f->name);
#else
      CDD_SNPRINTF(len_field, sizeof(len_field), "n_%s", f->name);
#endif

      CHECK_IO(fprintf(fp, "  if (req_body->%s) {\n", f->name));
      CHECK_IO(fprintf(fp, "    size_t i;\n"));
      if (items_is_object) {
        CHECK_IO(fprintf(fp, "    for (i = 0; i < req_body->%s; ++i) {\n",
                         len_field));
        CHECK_IO(fprintf(fp, "      char *part_json = NULL;\n"));
        CHECK_IO(
            fprintf(fp, "      if (!req_body->%s[i]) continue;\n", f->name));
        CHECK_IO(
            fprintf(fp, "      rc = %s_to_json(req_body->%s[i], &part_json);\n",
                    items_type, f->name));
        CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
        CHECK_IO(fprintf(fp,
                         "      rc = http_request_add_part(&req, \"%s\", "
                         "NULL, %s, part_json, strlen(part_json));\n",
                         f->name, ct_arg));
        CHECK_IO(fprintf(fp, "      free(part_json);\n"));
        CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
        if (write_multipart_part_headers(fp, enc) != 0)

          return CDD_C_ERROR_IO;

        CHECK_IO(fprintf(fp, "    }\n"));
      } else if (strcmp(items_type, "string") == 0) {
        CHECK_IO(fprintf(fp, "    for (i = 0; i < req_body->%s; ++i) {\n",
                         len_field));
        CHECK_IO(
            fprintf(fp, "      const char *val = req_body->%s[i];\n", f->name));
        CHECK_IO(fprintf(fp, "      if (!val) continue;\n"));
        CHECK_IO(fprintf(fp,
                         "      rc = http_request_add_part(&req, \"%s\", "
                         "NULL, %s, val, strlen(val));\n",
                         f->name, ct_arg));
        CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
        if (write_multipart_part_headers(fp, enc) != 0)

          return CDD_C_ERROR_IO;

        CHECK_IO(fprintf(fp, "    }\n"));
      } else if (strcmp(items_type, "integer") == 0) {
        CHECK_IO(fprintf(fp, "    for (i = 0; i < req_body->%s; ++i) {\n",
                         len_field));
        CHECK_IO(fprintf(fp, "      char num_buf[32];\n"));
        CHECK_IO(fprintf(fp,
                         "      spr"
                         "intf(num_buf, \"%%d\", "
                         "req_body->%s[i]);\n",
                         f->name));
        CHECK_IO(fprintf(fp,
                         "      rc = http_request_add_part(&req, \"%s\", "
                         "NULL, %s, num_buf, strlen(num_buf));\n",
                         f->name, ct_arg));
        CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
        if (write_multipart_part_headers(fp, enc) != 0)

          return CDD_C_ERROR_IO;

        CHECK_IO(fprintf(fp, "    }\n"));
      } else if (strcmp(items_type, "number") == 0) {

        CHECK_IO(fprintf(fp, "    for (i = 0; i < req_body->%s; ++i) {\n",

                         len_field));

        CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
        CHECK_IO(fprintf(fp,

                         "      spr"
                         "intf(num_buf, \"%%g\", "
                         "req_body->%s[i]);\n",
                         f->name));

        CHECK_IO(fprintf(fp,

                         "      rc = http_request_add_part(&req, \"%s\", "
                         "NULL, %s, num_buf, strlen(num_buf));\n",
                         f->name, ct_arg));

        CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
        if (write_multipart_part_headers(fp, enc) != 0)
          return CDD_C_ERROR_IO;
        CHECK_IO(fprintf(fp, "    }\n"));

      } else if (strcmp(items_type, "boolean") == 0) {

        CHECK_IO(fprintf(fp, "    for (i = 0; i < req_body->%s; ++i) {\n",

                         len_field));

        CHECK_IO(fprintf(fp,

                         "      const char *val = req_body->%s[i] ? "
                         "\"true\" : \"false\";\n",
                         f->name));

        CHECK_IO(fprintf(fp,

                         "      rc = http_request_add_part(&req, \"%s\", "
                         "NULL, %s, val, strlen(val));\n",
                         f->name, ct_arg));

        CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
        if (write_multipart_part_headers(fp, enc) != 0)
          return CDD_C_ERROR_IO;
        CHECK_IO(fprintf(fp, "    }\n"));

      } else {
        CHECK_IO(fprintf(
            fp, "    /* Unsupported array item type for %s in multipart */\n",
            f->name));
      }
      CHECK_IO(fprintf(fp, "  }\n"));
    } else if (strcmp(f->type, "string") == 0) {
      const char *content_type =
          (enc && enc->content_type) ? enc->content_type : NULL;
      char ct_buf[512];
      char ct_clean[256];
      const char *ct_arg = "NULL";
      if (content_type && content_type[0] != '\0') {
        content_type =
            (first_content_type_entry(content_type, ct_clean, sizeof(ct_clean),
                                      &_ast_first_content_type_entry_11),
             _ast_first_content_type_entry_11);
        CDD_SNPRINTF(ct_buf, sizeof(ct_buf), "\"%s\"", content_type);
        ct_arg = ct_buf;
      }
      CHECK_IO(fprintf(fp, "    if (req_body->%s) {\n", f->name));
      CHECK_IO(fprintf(fp,
                       "      rc = http_request_add_part(&req, \"%s\", NULL, "
                       "%s, req_body->%s, strlen(req_body->%s));\n",
                       f->name, ct_arg, f->name, f->name));
      CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
      if (write_multipart_part_headers(fp, enc) != 0)

        return CDD_C_ERROR_IO;

      CHECK_IO(fprintf(fp, "    }\n"));
    } else if (strcmp(f->type, "integer") == 0) {
      const char *content_type =
          (enc && enc->content_type) ? enc->content_type : NULL;
      char ct_buf[512];
      char ct_clean[256];
      const char *ct_arg = "NULL";
      if (content_type && content_type[0] != '\0') {

        content_type =
            (first_content_type_entry(content_type, ct_clean, sizeof(ct_clean),

                                      &_ast_first_content_type_entry_12),
             _ast_first_content_type_entry_12);

        CDD_SNPRINTF(ct_buf, sizeof(ct_buf), "\"%s\"", content_type);
        ct_arg = ct_buf;
      }
      CHECK_IO(fprintf(fp, "    {\n      char num_buf[32];\n"));
      CHECK_IO(fprintf(fp,
                       "      spr"
                       "intf(num_buf, \"%%d\", req_body->%s);\n",
                       f->name));
      CHECK_IO(fprintf(fp,
                       "      rc = http_request_add_part(&req, \"%s\", NULL, "
                       "%s, num_buf, strlen(num_buf));\n",
                       f->name, ct_arg));
      CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
      if (write_multipart_part_headers(fp, enc) != 0)

        return CDD_C_ERROR_IO;

      CHECK_IO(fprintf(fp, "    }\n"));
    } else if (strcmp(f->type, "number") == 0) {
      const char *content_type =
          (enc && enc->content_type) ? enc->content_type : NULL;
      char ct_buf[512];
      char ct_clean[256];
      const char *ct_arg = "NULL";
      if (content_type && content_type[0] != '\0') {

        content_type =
            (first_content_type_entry(content_type, ct_clean, sizeof(ct_clean),

                                      &_ast_first_content_type_entry_13),
             _ast_first_content_type_entry_13);

        CDD_SNPRINTF(ct_buf, sizeof(ct_buf), "\"%s\"", content_type);
        ct_arg = ct_buf;
      }
      CHECK_IO(fprintf(fp, "    {\n      char num_buf[64];\n"));
      CHECK_IO(fprintf(fp,
                       "      spr"
                       "intf(num_buf, \"%%g\", req_body->%s);\n",
                       f->name));
      CHECK_IO(fprintf(fp,
                       "      rc = http_request_add_part(&req, \"%s\", NULL, "
                       "%s, num_buf, strlen(num_buf));\n",
                       f->name, ct_arg));
      CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
      if (write_multipart_part_headers(fp, enc) != 0)

        return CDD_C_ERROR_IO;

      CHECK_IO(fprintf(fp, "    }\n"));
    } else if (strcmp(f->type, "boolean") == 0) {
      const char *content_type =
          (enc && enc->content_type) ? enc->content_type : NULL;
      char ct_buf[512];
      char ct_clean[256];
      const char *ct_arg = "NULL";
      if (content_type && content_type[0] != '\0') {

        content_type =
            (first_content_type_entry(content_type, ct_clean, sizeof(ct_clean),

                                      &_ast_first_content_type_entry_14),
             _ast_first_content_type_entry_14);

        CDD_SNPRINTF(ct_buf, sizeof(ct_buf), "\"%s\"", content_type);
        ct_arg = ct_buf;
      }
      CHECK_IO(fprintf(fp,
                       "    {\n      const char *val = req_body->%s ? "
                       "\"true\" : \"false\";\n",
                       f->name));
      CHECK_IO(fprintf(fp,
                       "      rc = http_request_add_part(&req, \"%s\", NULL, "
                       "%s, val, strlen(val));\n",
                       f->name, ct_arg));
      CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
      if (write_multipart_part_headers(fp, enc) != 0)

        return CDD_C_ERROR_IO;

      CHECK_IO(fprintf(fp, "    }\n"));
    } else if (strcmp(f->type, "object") == 0) {
      const char *content_type =
          (enc && enc->content_type) ? enc->content_type : NULL;
      const char *final_ct = content_type;
      char ct_buf[512];
      char ct_clean[256];
      const char *ct_arg = "NULL";
      if (!final_ct)
        final_ct = "application/json";
      if (final_ct[0] != '\0') {
        final_ct =
            (first_content_type_entry(final_ct, ct_clean, sizeof(ct_clean),
                                      &_ast_first_content_type_entry_15),
             _ast_first_content_type_entry_15);
        CDD_SNPRINTF(ct_buf, sizeof(ct_buf), "\"%s\"", final_ct);
        ct_arg = ct_buf;
      }
      if (f->ref[0] != '\0') {
        CHECK_IO(fprintf(fp, "    if (req_body->%s) {\n", f->name));
        CHECK_IO(fprintf(fp, "      char *part_json = NULL;\n"));
        CHECK_IO(fprintf(fp,
                         "      rc = %s_to_json(req_body->%s, &part_json);\n",
                         f->ref, f->name));
        CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
        CHECK_IO(fprintf(fp,
                         "      rc = http_request_add_part(&req, \"%s\", "
                         "NULL, %s, part_json, strlen(part_json));\n",
                         f->name, ct_arg));
        CHECK_IO(fprintf(fp, "      free(part_json);\n"));
        CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
        if (write_multipart_part_headers(fp, enc) != 0)

          return CDD_C_ERROR_IO;

        CHECK_IO(fprintf(fp, "    }\n"));
      } else {
        CHECK_IO(fprintf(fp,
                         "    /* Unsupported object field for %s in multipart "
                         "(missing ref) */\n",
                         f->name));
      }
    }
  }
  CHECK_IO(fprintf(fp, "  rc = http_request_flatten_parts(&req);\n"));
  CHECK_IO(fprintf(fp, "  if (rc != CDD_C_SUCCESS) goto cleanup;\n\n"));
  return CDD_C_SUCCESS;
}
