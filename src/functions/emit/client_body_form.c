/**
 * @file client_body_form.c
 * @brief Form URL-encoded request body serialization for client bodies.
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
 * @brief Generates C code for write form urlencoded body.
 */
cdd_c_error_t
client_body_write_form_urlencoded_body(FILE *fp,
                                       const struct OpenAPI_Operation *op,
                                       const struct OpenAPI_Spec *spec) {
  const struct OpenAPI_MediaType *_ast_find_media_type_3;
  struct StructFields *_ast_openapi_spec_find_schema_for_ref_4;
  struct OpenAPI_Encoding *_ast_find_encoding_5;
  struct StructFields *_ast_openapi_spec_find_schema_6;
  const struct OpenAPI_MediaType *mt;
  const struct StructFields *sf;
  size_t i;

  if (!fp || !op || !spec)

    return CDD_C_ERROR_INVALID_ARGUMENT;

  mt = (find_media_type(op->req_body_media_types, op->n_req_body_media_types,
                        "application/x-www-form-urlencoded",
                        &_ast_find_media_type_3),
        _ast_find_media_type_3);

  sf = (openapi_spec_find_schema_for_ref(
            spec, &op->req_body, &_ast_openapi_spec_find_schema_for_ref_4),
        _ast_openapi_spec_find_schema_for_ref_4);
  if (!sf) {
    CHECK_IO(fprintf(
        fp,
        "  /* Warning: Schema %s definition not found, skipping form body */\n",
        op->req_body.ref_name));
    return CDD_C_SUCCESS;
  }

  CHECK_IO(fprintf(fp, "  /* Form URL-Encoded Body Construction */\n"));
  CHECK_IO(fprintf(fp, "  rc = url_query_init(&form_qp);\n"));
  CHECK_IO(fprintf(fp, "  if (rc != CDD_C_SUCCESS) goto cleanup;\n"));

  for (i = 0; i < sf->size; ++i) {
    const struct StructField *f = &sf->fields[i];
    const struct OpenAPI_Encoding *enc =
        (find_encoding(mt, f->name, &_ast_find_encoding_5),
         _ast_find_encoding_5);
    enum OpenAPI_Style style =
        (enc && enc->style_set) ? enc->style : OA_STYLE_FORM;
    int explode = (enc && enc->explode_set) ? enc->explode
                                            : (style == OA_STYLE_FORM ? 1 : 0);
    int allow_reserved =
        (enc && enc->allow_reserved_set) ? enc->allow_reserved : 0;

    if (strcmp(f->type, "array") == 0) {
      const char *items_type = f->ref[0] != '\0' ? f->ref : "string";
      char len_field[80];
      const char *encode_fn = NULL;
      int add_encoded = 0;
      int items_is_object = 0;

      client_body_is_object_ref_type(items_type, &items_is_object);

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      sprintf_s(len_field, sizeof(len_field), "n_%s", f->name);
#else
      sprintf(len_field, "n_%s", f->name);
#endif

      if (allow_reserved) {

        encode_fn = "url_encode_form_allow_reserved";
      }

      if (style == OA_STYLE_FORM && explode && items_is_object) {
        const char *enc_fn = allow_reserved ? "url_encode_form_allow_reserved"
                                            : "url_encode_form";
        CHECK_IO(fprintf(fp, "  {\n    size_t i;\n"));
        CHECK_IO(
            fprintf(fp, "    for(i=0; i < req_body->%s; ++i) {\n", len_field));
        CHECK_IO(
            fprintf(fp, "      if (!req_body->%s[i]) continue;\n", f->name));
        CHECK_IO(fprintf(fp, "      char *item_json = NULL;\n"));
        CHECK_IO(fprintf(fp, "      char *enc = NULL;\n"));
        CHECK_IO(
            fprintf(fp, "      rc = %s_to_json(req_body->%s[i], &item_json);\n",
                    items_type, f->name));
        CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
        CHECK_IO(fprintf(fp, "      enc = %s(item_json);\n", enc_fn));
        CHECK_IO(fprintf(fp, "      free(item_json);\n"));
        CHECK_IO(fprintf(
            fp,
            "      if (!enc) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
        CHECK_IO(fprintf(
            fp, "      rc = url_query_add_encoded(&form_qp, \"%s\", enc);\n",
            f->name));
        CHECK_IO(fprintf(fp, "      free(enc);\n"));
        CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
        CHECK_IO(fprintf(fp, "    }\n  }\n"));
      } else if (style == OA_STYLE_FORM && explode) {
        CHECK_IO(fprintf(fp, "  {\n    size_t i;\n"));
        CHECK_IO(
            fprintf(fp, "    for(i=0; i < req_body->%s; ++i) {\n", len_field));
        if (strcmp(items_type, "string") == 0) {
          if (allow_reserved) {

            CHECK_IO(fprintf(fp,

                             "      char *enc = url_encode_form_allow_reserved"
                             "(req_body->%s[i]);\n",
                             f->name));

            CHECK_IO(fprintf(fp, "      if (!enc) { rc = CDD_C_ERROR_MEMORY; "

                                 "goto cleanup; }\n"));

            CHECK_IO(fprintf(

                fp,
                "      rc = url_query_add_encoded(&form_qp, \"%s\", enc);\n",
                f->name));

            CHECK_IO(fprintf(fp, "      free(enc);\n"));

          } else {
            CHECK_IO(fprintf(fp,
                             "      rc = url_query_add(&form_qp, \"%s\", "
                             "req_body->%s[i]);\n",
                             f->name, f->name));
          }
        } else if (strcmp(items_type, "integer") == 0) {

          CHECK_IO(fprintf(fp, "      char num_buf[32];\n"));
          CHECK_IO(

              fprintf(fp,
                      "      spr"
                      "intf(num_buf, \"%%d\", req_body->%s[i]);\n",
                      f->name));

          CHECK_IO(fprintf(

              fp, "      rc = url_query_add(&form_qp, \"%s\", num_buf);\n",
              f->name));
        } else if (strcmp(items_type, "number") == 0) {

          CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
          CHECK_IO(

              fprintf(fp,
                      "      spr"
                      "intf(num_buf, \"%%g\", req_body->%s[i]);\n",
                      f->name));

          CHECK_IO(fprintf(

              fp, "      rc = url_query_add(&form_qp, \"%s\", num_buf);\n",
              f->name));
        } else if (strcmp(items_type, "boolean") == 0) {

          CHECK_IO(fprintf(fp,

                           "      rc = url_query_add(&form_qp, \"%s\", "
                           "req_body->%s[i] ? \"true\" : \"false\");\n",
                           f->name, f->name));
        } else {
          CHECK_IO(fprintf(
              fp, "      /* Unsupported array item type for %s */\n", f->name));
        }
        CHECK_IO(fprintf(
            fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n    }\n  }\n"));

      } else if (style == OA_STYLE_FORM) {
        add_encoded = 1;
        if (write_joined_form_array(fp, f->name, len_field, items_type, ',',

                                    encode_fn ? encode_fn : "url_encode_form",
                                    add_encoded, items_is_object) != 0)

          return CDD_C_ERROR_IO;
      } else if (style == OA_STYLE_SPACE_DELIMITED) {
        if (write_joined_form_array(fp, f->name, len_field, items_type, ' ',

                                    NULL, 0, items_is_object) != 0)

          return CDD_C_ERROR_IO;
      } else if (style == OA_STYLE_PIPE_DELIMITED) {
        if (write_joined_form_array(fp, f->name, len_field, items_type, '|',

                                    NULL, 0, items_is_object) != 0)

          return CDD_C_ERROR_IO;

      } else {

        CHECK_IO(fprintf(

            fp, "  /* Array style not supported for %s in form body */\n",
            f->name));
      }
      continue;
    }

    if (strcmp(f->type, "string") == 0) {
      CHECK_IO(fprintf(fp, "  if (req_body->%s) {\n", f->name));
      if (allow_reserved) {

        CHECK_IO(fprintf(

            fp,
            "    char *enc = url_encode_form_allow_reserved(req_body->%s);\n",
            f->name));

        CHECK_IO(fprintf(

            fp, "    if (!enc) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));

        CHECK_IO(fprintf(

            fp, "    rc = url_query_add_encoded(&form_qp, \"%s\", enc);\n",
            f->name));

        CHECK_IO(fprintf(fp, "    free(enc);\n"));

      } else {
        CHECK_IO(fprintf(
            fp, "    rc = url_query_add(&form_qp, \"%s\", req_body->%s);\n",
            f->name, f->name));
      }
      CHECK_IO(
          fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n  }\n"));
    } else if (strcmp(f->type, "integer") == 0) {
      CHECK_IO(fprintf(fp, "  {\n    char num_buf[32];\n"));
      CHECK_IO(fprintf(fp,
                       "    spr"
                       "intf(num_buf, \"%%d\", req_body->%s);\n",
                       f->name));
      CHECK_IO(fprintf(
          fp, "    rc = url_query_add(&form_qp, \"%s\", num_buf);\n", f->name));
      CHECK_IO(
          fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n  }\n"));
    } else if (strcmp(f->type, "number") == 0) {

      CHECK_IO(fprintf(fp, "  {\n    char num_buf[64];\n"));
      CHECK_IO(fprintf(fp,
                       "    spr"
                       "intf(num_buf, \"%%g\", req_body->%s);\n",

                       f->name));

      CHECK_IO(fprintf(

          fp, "    rc = url_query_add(&form_qp, \"%s\", num_buf);\n", f->name));

      CHECK_IO(
          fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n  }\n"));

    } else if (strcmp(f->type, "boolean") == 0) {

      CHECK_IO(fprintf(fp,

                       "  rc = url_query_add(&form_qp, \"%s\", req_body->%s ? "
                       "\"true\" : \"false\");\n",
                       f->name, f->name));

      CHECK_IO(fprintf(fp, "  if (rc != CDD_C_SUCCESS) goto cleanup;\n"));

    } else if (strcmp(f->type, "object") == 0) {
      if (f->ref[0] != '\0') {
        int all_prim = 0;
        const struct StructFields *obj_sf =
            (openapi_spec_find_schema(spec, f->ref,
                                      &_ast_openapi_spec_find_schema_6),
             _ast_openapi_spec_find_schema_6);
        const struct OpenAPI_Encoding *obj_enc = enc;
        int style_based =
            obj_enc && (obj_enc->style_set || obj_enc->explode_set ||
                        obj_enc->allow_reserved_set);

        if (obj_sf) {
          struct_fields_all_primitive(obj_sf, &all_prim);
        }

        if (style_based && obj_sf && all_prim && obj_sf->size > 0) {
          enum OpenAPI_Style obj_style =
              obj_enc->style_set ? obj_enc->style : OA_STYLE_FORM;
          int obj_explode = obj_enc->explode_set
                                ? obj_enc->explode
                                : (obj_style == OA_STYLE_FORM ? 1 : 0);
          int obj_allow_reserved =
              obj_enc->allow_reserved_set ? obj_enc->allow_reserved : 0;

          if (obj_style == OA_STYLE_FORM && obj_explode) {
            size_t pf_idx;
            CHECK_IO(fprintf(fp, "  if (req_body->%s) {\n", f->name));
            for (pf_idx = 0; pf_idx < obj_sf->size; ++pf_idx) {
              const struct StructField *pf = &obj_sf->fields[pf_idx];
              if (strcmp(pf->type, "string") == 0) {
                CHECK_IO(fprintf(fp, "    if (req_body->%s->%s) {\n", f->name,
                                 pf->name));
                if (obj_allow_reserved) {

                  CHECK_IO(fprintf(

                      fp,
                      "      char *enc = "
                      "url_encode_form_allow_reserved(req_body->%s->%s);\n",
                      f->name, pf->name));

                  CHECK_IO(fprintf(fp,

                                   "      if (!enc) { rc = CDD_C_ERROR_MEMORY; "
                                   "goto cleanup; }\n"));

                  CHECK_IO(fprintf(fp,

                                   "      rc = url_query_add_encoded(&form_qp, "
                                   "\"%s\", enc);\n",
                                   pf->name));

                  CHECK_IO(fprintf(fp, "      free(enc);\n"));

                } else {
                  CHECK_IO(fprintf(fp,
                                   "      rc = url_query_add(&form_qp, \"%s\", "
                                   "req_body->%s->%s);\n",
                                   pf->name, f->name, pf->name));
                }
                CHECK_IO(fprintf(
                    fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
                CHECK_IO(fprintf(fp, "    }\n"));
              } else if (strcmp(pf->type, "integer") == 0) {
                CHECK_IO(fprintf(fp, "    {\n      char num_buf[32];\n"));
                CHECK_IO(fprintf(fp,
                                 "      spr"
                                 "intf(num_buf, \"%%d\", req_body->%s->%s);\n",
                                 f->name, pf->name));
                CHECK_IO(fprintf(
                    fp,
                    "      rc = url_query_add(&form_qp, \"%s\", num_buf);\n",
                    pf->name));
                CHECK_IO(fprintf(
                    fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
                CHECK_IO(fprintf(fp, "    }\n"));

              } else if (strcmp(pf->type, "number") == 0) {
                CHECK_IO(fprintf(fp, "    {\n      char num_buf[64];\n"));
                CHECK_IO(fprintf(

                    fp,
                    "      spr"
                    "intf(num_buf, \"%%g\", req_body->%s->%s);\n",
                    f->name, pf->name));

                CHECK_IO(fprintf(

                    fp,
                    "      rc = url_query_add(&form_qp, \"%s\", num_buf);\n",
                    pf->name));

                CHECK_IO(fprintf(
                    fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
                CHECK_IO(fprintf(fp, "    }\n"));
              } else {
                CHECK_IO(fprintf(fp,

                                 "    rc = url_query_add(&form_qp, \"%s\", "
                                 "req_body->%s->%s ? \"true\" : \"false\");\n",
                                 pf->name, f->name, pf->name));

                CHECK_IO(fprintf(
                    fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
              }
            }
            CHECK_IO(fprintf(fp, "  }\n"));
          } else if (obj_style == OA_STYLE_FORM) {
            size_t pf_idx;
            CHECK_IO(fprintf(fp, "  if (req_body->%s) {\n", f->name));
            CHECK_IO(fprintf(fp, "    struct OpenAPI_KV kvs[%lu];\n",
                             (unsigned long)obj_sf->size));
            CHECK_IO(fprintf(fp, "    size_t kv_len = 0;\n"));
            for (pf_idx = 0; pf_idx < obj_sf->size; ++pf_idx) {
              const struct StructField *pf = &obj_sf->fields[pf_idx];
              if (strcmp(pf->type, "string") == 0) {
                CHECK_IO(fprintf(fp, "    if (req_body->%s->%s) {\n", f->name,
                                 pf->name));
                CHECK_IO(
                    fprintf(fp, "      kvs[kv_len].key = \"%s\";\n", pf->name));
                CHECK_IO(
                    fprintf(fp, "      kvs[kv_len].type = OA_KV_STRING;\n"));
                CHECK_IO(fprintf(
                    fp, "      kvs[kv_len].value.s = req_body->%s->%s;\n",
                    f->name, pf->name));
                CHECK_IO(fprintf(fp, "      kv_len++;\n    }\n"));
              } else if (strcmp(pf->type, "integer") == 0) {
                CHECK_IO(
                    fprintf(fp, "    kvs[kv_len].key = \"%s\";\n", pf->name));
                CHECK_IO(
                    fprintf(fp, "    kvs[kv_len].type = OA_KV_INTEGER;\n"));
                CHECK_IO(
                    fprintf(fp, "    kvs[kv_len].value.i = req_body->%s->%s;\n",
                            f->name, pf->name));
                CHECK_IO(fprintf(fp, "    kv_len++;\n"));

              } else if (strcmp(pf->type, "number") == 0) {
                CHECK_IO(

                    fprintf(fp, "    kvs[kv_len].key = \"%s\";\n", pf->name));

                CHECK_IO(fprintf(fp, "    kvs[kv_len].type = OA_KV_NUMBER;\n"));
                CHECK_IO(

                    fprintf(fp, "    kvs[kv_len].value.n = req_body->%s->%s;\n",
                            f->name, pf->name));

                CHECK_IO(fprintf(fp, "    kv_len++;\n"));
              } else {
                CHECK_IO(

                    fprintf(fp, "    kvs[kv_len].key = \"%s\";\n", pf->name));

                CHECK_IO(

                    fprintf(fp, "    kvs[kv_len].type = OA_KV_BOOLEAN;\n"));

                CHECK_IO(

                    fprintf(fp, "    kvs[kv_len].value.b = req_body->%s->%s;\n",
                            f->name, pf->name));

                CHECK_IO(fprintf(fp, "    kv_len++;\n"));
              }
            }
            CHECK_IO(fprintf(fp, "    if (kv_len > 0) {\n"));
            CHECK_IO(fprintf(fp,
                             "      char *joined = openapi_kv_join_form(kvs, "
                             "kv_len, \",\", %d);\n",
                             obj_allow_reserved ? 1 : 0));
            CHECK_IO(fprintf(fp, "      if (!joined) { rc = "
                                 "CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
            CHECK_IO(fprintf(
                fp,
                "      rc = url_query_add_encoded(&form_qp, \"%s\", joined);\n",
                f->name));
            CHECK_IO(fprintf(fp, "      free(joined);\n"));
            CHECK_IO(
                fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
            CHECK_IO(fprintf(fp, "    }\n"));
            CHECK_IO(fprintf(fp, "  }\n"));
          } else if (obj_style == OA_STYLE_DEEP_OBJECT && obj_explode) {
            size_t pf_idx;
            CHECK_IO(fprintf(fp, "  if (req_body->%s) {\n", f->name));
            for (pf_idx = 0; pf_idx < obj_sf->size; ++pf_idx) {
              const struct StructField *pf = &obj_sf->fields[pf_idx];
              if (strcmp(pf->type, "string") == 0) {
                CHECK_IO(fprintf(fp, "    if (req_body->%s->%s) {\n", f->name,
                                 pf->name));
                if (obj_allow_reserved) {

                  CHECK_IO(fprintf(

                      fp,
                      "      char *enc = "
                      "url_encode_form_allow_reserved(req_body->%s->%s);\n",
                      f->name, pf->name));

                  CHECK_IO(fprintf(fp,

                                   "      if (!enc) { rc = CDD_C_ERROR_MEMORY; "
                                   "goto cleanup; }\n"));

                  CHECK_IO(fprintf(fp,

                                   "      rc = url_query_add_encoded(&form_qp, "
                                   "\"%s[%s]\", enc);\n",
                                   f->name, pf->name));

                  CHECK_IO(fprintf(fp, "      free(enc);\n"));

                } else {
                  CHECK_IO(fprintf(fp,
                                   "      rc = url_query_add(&form_qp, "
                                   "\"%s[%s]\", req_body->%s->%s);\n",
                                   f->name, pf->name, f->name, pf->name));
                }
                CHECK_IO(fprintf(
                    fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
                CHECK_IO(fprintf(fp, "    }\n"));
              } else if (strcmp(pf->type, "integer") == 0) {
                CHECK_IO(fprintf(fp, "    {\n      char num_buf[32];\n"));
                CHECK_IO(fprintf(fp,
                                 "      spr"
                                 "intf(num_buf, \"%%d\", req_body->%s->%s);\n",
                                 f->name, pf->name));
                CHECK_IO(fprintf(fp,
                                 "      rc = url_query_add(&form_qp, "
                                 "\"%s[%s]\", num_buf);\n",
                                 f->name, pf->name));
                CHECK_IO(fprintf(
                    fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
                CHECK_IO(fprintf(fp, "    }\n"));

              } else if (strcmp(pf->type, "number") == 0) {
                CHECK_IO(fprintf(fp, "    {\n      char num_buf[64];\n"));
                CHECK_IO(fprintf(

                    fp,
                    "      spr"
                    "intf(num_buf, \"%%g\", req_body->%s->%s);\n",
                    f->name, pf->name));

                CHECK_IO(fprintf(fp,

                                 "      rc = url_query_add(&form_qp, "
                                 "\"%s[%s]\", num_buf);\n",
                                 f->name, pf->name));

                CHECK_IO(fprintf(
                    fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
                CHECK_IO(fprintf(fp, "    }\n"));
              } else {
                CHECK_IO(fprintf(fp,

                                 "    rc = url_query_add(&form_qp, \"%s[%s]\", "
                                 "req_body->%s->%s ? \"true\" : \"false\");\n",
                                 f->name, pf->name, f->name, pf->name));

                CHECK_IO(fprintf(
                    fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
              }
            }
            CHECK_IO(fprintf(fp, "  }\n"));

          } else if (obj_style == OA_STYLE_SPACE_DELIMITED ||
                     obj_style == OA_STYLE_PIPE_DELIMITED) {

            size_t pf_idx;

            const char *delim =

                (obj_style == OA_STYLE_SPACE_DELIMITED) ? "%20" : "%7C";

            CHECK_IO(fprintf(fp, "  if (req_body->%s) {\n", f->name));
            CHECK_IO(fprintf(fp, "    struct OpenAPI_KV kvs[%lu];\n",
                             (unsigned long)obj_sf->size));
            CHECK_IO(fprintf(fp, "    size_t kv_len = 0;\n"));
            for (pf_idx = 0; pf_idx < obj_sf->size; ++pf_idx) {
              const struct StructField *pf = &obj_sf->fields[pf_idx];
              if (strcmp(pf->type, "string") == 0) {
                CHECK_IO(fprintf(fp, "    if (req_body->%s->%s) {\n", f->name,

                                 pf->name));

                CHECK_IO(

                    fprintf(fp, "      kvs[kv_len].key = \"%s\";\n", pf->name));

                CHECK_IO(

                    fprintf(fp, "      kvs[kv_len].type = OA_KV_STRING;\n"));

                CHECK_IO(fprintf(

                    fp, "      kvs[kv_len].value.s = req_body->%s->%s;\n",
                    f->name, pf->name));

                CHECK_IO(fprintf(fp, "      kv_len++;\n    }\n"));
              } else if (strcmp(pf->type, "integer") == 0) {
                CHECK_IO(

                    fprintf(fp, "    kvs[kv_len].key = \"%s\";\n", pf->name));

                CHECK_IO(

                    fprintf(fp, "    kvs[kv_len].type = OA_KV_INTEGER;\n"));

                CHECK_IO(

                    fprintf(fp, "    kvs[kv_len].value.i = req_body->%s->%s;\n",
                            f->name, pf->name));

                CHECK_IO(fprintf(fp, "    kv_len++;\n"));
              } else if (strcmp(pf->type, "number") == 0) {
                CHECK_IO(

                    fprintf(fp, "    kvs[kv_len].key = \"%s\";\n", pf->name));

                CHECK_IO(fprintf(fp, "    kvs[kv_len].type = OA_KV_NUMBER;\n"));
                CHECK_IO(

                    fprintf(fp, "    kvs[kv_len].value.n = req_body->%s->%s;\n",
                            f->name, pf->name));

                CHECK_IO(fprintf(fp, "    kv_len++;\n"));
              } else {
                CHECK_IO(

                    fprintf(fp, "    kvs[kv_len].key = \"%s\";\n", pf->name));

                CHECK_IO(

                    fprintf(fp, "    kvs[kv_len].type = OA_KV_BOOLEAN;\n"));

                CHECK_IO(

                    fprintf(fp, "    kvs[kv_len].value.b = req_body->%s->%s;\n",
                            f->name, pf->name));

                CHECK_IO(fprintf(fp, "    kv_len++;\n"));
              }
            }

            CHECK_IO(fprintf(fp, "    if (kv_len > 0) {\n"));
            CHECK_IO(fprintf(fp,

                             "      char *joined = openapi_kv_join_form(kvs, "
                             "kv_len, \"%s\", %d);\n",
                             delim, obj_allow_reserved ? 1 : 0));

            CHECK_IO(fprintf(fp, "      if (!joined) { rc = "

                                 "CDD_C_ERROR_MEMORY; goto cleanup; }\n"));

            CHECK_IO(fprintf(

                fp,
                "      rc = url_query_add_encoded(&form_qp, \"%s\", joined);\n",
                f->name));

            CHECK_IO(fprintf(fp, "      free(joined);\n"));
            CHECK_IO(
                fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
            CHECK_IO(fprintf(fp, "    }\n"));
            CHECK_IO(fprintf(fp, "  }\n"));

          } else {

            CHECK_IO(fprintf(

                fp, "  /* Unsupported object style for %s in form body */\n",
                f->name));
          }
        } else {
          const char *enc_fn = allow_reserved ? "url_encode_form_allow_reserved"
                                              : "url_encode_form";
          CHECK_IO(fprintf(fp, "  if (req_body->%s) {\n", f->name));
          CHECK_IO(fprintf(fp, "    char *obj_json = NULL;\n"));
          CHECK_IO(fprintf(fp, "    char *enc = NULL;\n"));
          CHECK_IO(fprintf(fp,
                           "    rc = %s_to_json(req_body->%s, &obj_json);\n",
                           f->ref, f->name));
          CHECK_IO(fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
          CHECK_IO(fprintf(fp, "    enc = %s(obj_json);\n", enc_fn));
          CHECK_IO(fprintf(fp, "    free(obj_json);\n"));
          CHECK_IO(fprintf(
              fp,
              "    if (!enc) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
          CHECK_IO(fprintf(
              fp, "    rc = url_query_add_encoded(&form_qp, \"%s\", enc);\n",
              f->name));
          CHECK_IO(fprintf(fp, "    free(enc);\n"));
          CHECK_IO(
              fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n  }\n"));
        }
      } else {
        CHECK_IO(fprintf(fp,
                         "  /* Unsupported object field for %s in form body "
                         "(missing ref) */\n",
                         f->name));
      }
    } else {

      CHECK_IO(

          fprintf(fp, "  /* Unsupported form field type for %s */\n", f->name));
    }
  }

  CHECK_IO(fprintf(fp, "  rc = url_query_build_form(&form_qp, &form_body);\n"));
  CHECK_IO(fprintf(fp, "  if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
  CHECK_IO(fprintf(fp, "  req.body = form_body;\n"));
  CHECK_IO(fprintf(fp, "  req.body_len = strlen(form_body);\n"));
  CHECK_IO(fprintf(
      fp, "  http_headers_add(&req.headers, "
          "\"Content-Type\", \"application/x-www-form-urlencoded\");\n\n"));

  return CDD_C_SUCCESS;
}
