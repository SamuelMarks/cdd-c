/**
 * @file client_body_payload.c
 * @brief Response decoding and payload serialization code generation for client
 * bodies.
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
 * @brief Generates C code for write text plain success.
 */
cdd_c_error_t write_text_plain_success(FILE *fp) {
  if (!fp)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  CHECK_IO(fprintf(fp, "      if (res->body && out) {\n"));
  CHECK_IO(fprintf(fp, "        size_t body_len = res->body_len;\n"));
  CHECK_IO(fprintf(
      fp, "        char *tmp = (char *)(size_t)malloc(body_len + 1);\n"));
  CHECK_IO(fprintf(fp, "        if (!tmp) { rc = CDD_C_ERROR_MEMORY; }\n"));
  CHECK_IO(fprintf(fp, "        else {\n"));
  CHECK_IO(fprintf(fp, "          memcpy(tmp, res->body, body_len);\n"));
  CHECK_IO(fprintf(fp, "          tmp[body_len] = '\\0';\n"));
  CHECK_IO(fprintf(fp, "          *out = tmp;\n"));
  CHECK_IO(fprintf(fp, "        }\n"));
  CHECK_IO(fprintf(fp, "      }\n"));
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write binary success.
 */
cdd_c_error_t write_binary_success(FILE *fp) {
  if (!fp)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  CHECK_IO(fprintf(fp, "      if (out && out_len) {\n"));
  CHECK_IO(fprintf(fp, "        if (!res->body || res->body_len == 0) {\n"));
  CHECK_IO(fprintf(fp, "          *out = NULL;\n"));
  CHECK_IO(fprintf(fp, "          *out_len = 0;\n"));
  CHECK_IO(fprintf(fp, "        } else {\n"));
  CHECK_IO(fprintf(fp, "          unsigned char *tmp = "
                       "(unsigned char *)malloc(res->body_len);\n"));
  CHECK_IO(fprintf(fp, "          if (!tmp) { rc = CDD_C_ERROR_MEMORY; }\n"));
  CHECK_IO(fprintf(fp,
                   "          else { memcpy(tmp, res->body, res->body_len); "
                   "*out = tmp; *out_len = res->body_len; }\n"));
  CHECK_IO(fprintf(fp, "        }\n"));
  CHECK_IO(fprintf(fp, "      }\n"));
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write inline json parse.
 */
cdd_c_error_t write_inline_json_parse(FILE *fp,
                                      const struct OpenAPI_SchemaRef *schema) {
  const char *type;
  if (!fp || !schema || !schema->inline_type)

    return CDD_C_ERROR_INVALID_ARGUMENT;

  type = schema->inline_type;

  if (schema->is_array) {
    CHECK_IO(fprintf(fp, "      if (res->body && out && out_len) {\n"));
    CHECK_IO(fprintf(fp, "        JSON_Value *val = json_parse_string((const "
                         "char*)res->body);\n"));
    CHECK_IO(fprintf(fp, "        JSON_Array *arr = NULL;\n"));
    CHECK_IO(fprintf(fp, "        size_t count = 0;\n"));
    CHECK_IO(fprintf(
        fp, "        if (!val) { rc = CDD_C_ERROR_INVALID_ARGUMENT; }\n"));
    CHECK_IO(fprintf(fp, "        if (rc == CDD_C_SUCCESS) {\n"));
    CHECK_IO(fprintf(fp, "          arr = json_value_get_array(val);\n"));
    CHECK_IO(fprintf(
        fp, "          if (!arr) rc = CDD_C_ERROR_INVALID_ARGUMENT;\n"));
    CHECK_IO(fprintf(fp, "        }\n"));
    CHECK_IO(fprintf(fp, "        if (rc == CDD_C_SUCCESS) {\n"));
    CHECK_IO(fprintf(fp, "          count = json_array_get_count(arr);\n"));
    CHECK_IO(fprintf(fp, "          *out_len = count;\n"));
    CHECK_IO(fprintf(fp, "          if (count == 0) {\n"));
    CHECK_IO(fprintf(fp, "            *out = NULL;\n"));
    CHECK_IO(fprintf(fp, "          } else {\n"));
    if (strcmp(type, "string") == 0) {
      CHECK_IO(fprintf(fp, "            char **tmp = (char **)calloc(count, "
                           "sizeof(char *));\n"));
      CHECK_IO(fprintf(fp, "            size_t i;\n"));
      CHECK_IO(
          fprintf(fp, "            if (!tmp) { rc = CDD_C_ERROR_MEMORY; }\n"));
      CHECK_IO(fprintf(fp, "            if (rc == CDD_C_SUCCESS) {\n"));
      CHECK_IO(fprintf(fp, "              for (i = 0; i < count; ++i) {\n"));
      CHECK_IO(fprintf(fp, "                const char *s = "
                           "json_array_get_string(arr, i);\n"));
      CHECK_IO(fprintf(fp, "                if (!s) { rc = "
                           "CDD_C_ERROR_INVALID_ARGUMENT; break; }\n"));
      CHECK_IO(fprintf(fp, "                tmp[i] = strdup(s);\n"));
      CHECK_IO(fprintf(
          fp, "                if (!tmp[i]) { rc = CDD_C_ERROR_MEMORY; break; "
              "}\n"));
      CHECK_IO(fprintf(fp, "              }\n"));
      CHECK_IO(fprintf(fp, "            }\n"));
      CHECK_IO(fprintf(fp, "            if (rc == CDD_C_SUCCESS) {\n"));
      CHECK_IO(fprintf(fp, "              *out = tmp;\n"));
      CHECK_IO(fprintf(fp, "            } else if (tmp) {\n"));
      CHECK_IO(fprintf(fp, "              size_t k;\n"));
      CHECK_IO(fprintf(fp, "              for (k = 0; k < count; ++k) "
                           "free(tmp[k]);\n"));
      CHECK_IO(fprintf(fp, "              free(tmp);\n"));
      CHECK_IO(fprintf(fp, "            }\n"));
    } else if (strcmp(type, "integer") == 0) {
      CHECK_IO(fprintf(fp, "            int *tmp = (int *)calloc(count, "
                           "sizeof(int));\n"));
      CHECK_IO(fprintf(fp, "            size_t i;\n"));
      CHECK_IO(
          fprintf(fp, "            if (!tmp) { rc = CDD_C_ERROR_MEMORY; }\n"));
      CHECK_IO(fprintf(fp, "            if (rc == CDD_C_SUCCESS) {\n"));
      CHECK_IO(fprintf(fp, "              for (i = 0; i < count; ++i) {\n"));
      CHECK_IO(fprintf(
          fp, "                if "
              "(json_array_get_value(arr, i) && "
              "json_value_get_type(json_array_get_value(arr, i)) != "
              "JSONNumber) { rc = CDD_C_ERROR_INVALID_ARGUMENT; break; }\n"));
      CHECK_IO(fprintf(fp,
                       "                tmp[i] = (int)json_array_get_number("
                       "arr, i);\n"));
      CHECK_IO(fprintf(fp, "              }\n"));
      CHECK_IO(fprintf(fp, "            }\n"));
      CHECK_IO(fprintf(fp, "            if (rc == CDD_C_SUCCESS) *out = tmp; "
                           "else free(tmp);\n"));
    } else if (strcmp(type, "number") == 0) {
      CHECK_IO(fprintf(fp, "            double *tmp = (double *)calloc(count, "
                           "sizeof(double));\n"));
      CHECK_IO(fprintf(fp, "            size_t i;\n"));
      CHECK_IO(
          fprintf(fp, "            if (!tmp) { rc = CDD_C_ERROR_MEMORY; }\n"));
      CHECK_IO(fprintf(fp, "            if (rc == CDD_C_SUCCESS) {\n"));
      CHECK_IO(fprintf(fp, "              for (i = 0; i < count; ++i) {\n"));
      CHECK_IO(fprintf(
          fp, "                if "
              "(json_array_get_value(arr, i) && "
              "json_value_get_type(json_array_get_value(arr, i)) != "
              "JSONNumber) { rc = CDD_C_ERROR_INVALID_ARGUMENT; break; }\n"));
      CHECK_IO(fprintf(fp,
                       "                tmp[i] = json_array_get_number(arr, "
                       "i);\n"));
      CHECK_IO(fprintf(fp, "              }\n"));
      CHECK_IO(fprintf(fp, "            }\n"));
      CHECK_IO(fprintf(fp, "            if (rc == CDD_C_SUCCESS) *out = tmp; "
                           "else free(tmp);\n"));
    } else if (strcmp(type, "boolean") == 0) {
      CHECK_IO(fprintf(fp, "            int *tmp = (int *)calloc(count, "
                           "sizeof(int));\n"));
      CHECK_IO(fprintf(fp, "            size_t i;\n"));
      CHECK_IO(
          fprintf(fp, "            if (!tmp) { rc = CDD_C_ERROR_MEMORY; }\n"));
      CHECK_IO(fprintf(fp, "            if (rc == CDD_C_SUCCESS) {\n"));
      CHECK_IO(fprintf(fp, "              for (i = 0; i < count; ++i) {\n"));
      CHECK_IO(fprintf(
          fp, "                if "
              "(json_array_get_value(arr, i) && "
              "json_value_get_type(json_array_get_value(arr, i)) != "
              "JSONBoolean) { rc = CDD_C_ERROR_INVALID_ARGUMENT; break; }\n"));
      CHECK_IO(fprintf(fp,
                       "                tmp[i] = json_array_get_boolean(arr, "
                       "i) ? 1 : 0;\n"));
      CHECK_IO(fprintf(fp, "              }\n"));
      CHECK_IO(fprintf(fp, "            }\n"));
      CHECK_IO(fprintf(fp, "            if (rc == CDD_C_SUCCESS) *out = tmp; "
                           "else free(tmp);\n"));
    } else {
      CHECK_IO(fprintf(fp, "            rc = CDD_C_ERROR_INVALID_ARGUMENT;\n"));
    }
    CHECK_IO(fprintf(fp, "          }\n"));
    CHECK_IO(fprintf(fp, "        }\n"));
    CHECK_IO(fprintf(fp, "        if (val) json_value_free(val);\n"));
    CHECK_IO(fprintf(fp, "      }\n"));
  } else {
    CHECK_IO(fprintf(fp, "      if (res->body && out) {\n"));
    CHECK_IO(fprintf(fp, "        JSON_Value *val = json_parse_string((const "
                         "char*)res->body);\n"));
    CHECK_IO(fprintf(
        fp, "        if (!val) { rc = CDD_C_ERROR_INVALID_ARGUMENT; }\n"));
    CHECK_IO(fprintf(fp, "        if (rc == CDD_C_SUCCESS) {\n"));
    if (strcmp(type, "string") == 0) {
      CHECK_IO(fprintf(
          fp, "          const char *s = json_value_get_string(val);\n"));
      CHECK_IO(fprintf(
          fp, "          if (!s) { rc = CDD_C_ERROR_INVALID_ARGUMENT; }\n"));
      CHECK_IO(fprintf(fp, "          if (rc == CDD_C_SUCCESS) {\n"));
      CHECK_IO(fprintf(fp, "            *out = strdup(s);\n"));
      CHECK_IO(
          fprintf(fp, "            if (!*out) rc = CDD_C_ERROR_MEMORY;\n"));
      CHECK_IO(fprintf(fp, "          }\n"));
    } else if (strcmp(type, "integer") == 0) {
      CHECK_IO(fprintf(fp,
                       "          if (json_value_get_type(val) != JSONNumber) "
                       "{ rc = CDD_C_ERROR_INVALID_ARGUMENT; }\n"));
      CHECK_IO(fprintf(fp, "          if (rc == CDD_C_SUCCESS) *out = "
                           "(int)json_value_get_number(val);\n"));
    } else if (strcmp(type, "number") == 0) {

      CHECK_IO(fprintf(fp,

                       "          if (json_value_get_type(val) != JSONNumber) "
                       "{ rc = CDD_C_ERROR_INVALID_ARGUMENT; }\n"));

      CHECK_IO(fprintf(fp, "          if (rc == CDD_C_SUCCESS) *out = "

                           "json_value_get_number(val);\n"));
    } else if (strcmp(type, "boolean") == 0) {
      CHECK_IO(fprintf(fp,
                       "          if (json_value_get_type(val) != JSONBoolean) "
                       "{ rc = CDD_C_ERROR_INVALID_ARGUMENT; }\n"));
      CHECK_IO(fprintf(fp, "          if (rc == CDD_C_SUCCESS) *out = "
                           "json_value_get_boolean(val) ? 1 : 0;\n"));
    } else {

      CHECK_IO(fprintf(fp, "          rc = CDD_C_ERROR_INVALID_ARGUMENT;\n"));
    }
    CHECK_IO(fprintf(fp, "        }\n"));
    CHECK_IO(fprintf(fp, "        if (val) json_value_free(val);\n"));
    CHECK_IO(fprintf(fp, "      }\n"));
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write joined form array.
 */
cdd_c_error_t write_joined_form_array(FILE *fp, const char *field,
                                      const char *len_field,
                                      const char *items_type, char delim,
                                      const char *encode_fn, int add_encoded,
                                      int items_is_object) {

  const int do_encode = (encode_fn && encode_fn[0] != '\0');

  if (!fp || !field || !len_field || !items_type)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  CHECK_IO(fprintf(fp, "  {\n"));
  CHECK_IO(fprintf(fp, "    size_t i;\n"));
  CHECK_IO(fprintf(fp, "    char *joined = NULL;\n"));
  CHECK_IO(fprintf(fp, "    size_t joined_len = 0;\n"));
  CHECK_IO(fprintf(fp, "    for(i=0; i < req_body->%s; ++i) {\n", len_field));

  if (items_is_object) {
    CHECK_IO(fprintf(fp, "      char *raw = NULL;\n"));
    CHECK_IO(fprintf(fp, "      if (!req_body->%s[i]) continue;\n", field));
    CHECK_IO(fprintf(fp, "      rc = %s_to_json(req_body->%s[i], &raw);\n",

                     items_type, field));

    CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
    if (do_encode) {
      CHECK_IO(fprintf(fp, "      char *enc = %s(raw);\n", encode_fn));
      CHECK_IO(fprintf(fp, "      size_t val_len;\n"));
      CHECK_IO(fprintf(fp, "      if (!enc) { free(raw); rc = "

                           "CDD_C_ERROR_MEMORY; goto cleanup; }\n"));

      CHECK_IO(fprintf(fp, "      val_len = strlen(enc);\n"));
      CHECK_IO(fprintf(

          fp,
          "      {\n"
          "        size_t extra = val_len + (i > 0 ? 1 : 0);\n"
          "        char *tmp = (char *)(size_t)realloc(joined, joined_len "
          "+ extra + 1);\n"
          "        if (!tmp) { free(raw); free(enc); rc = CDD_C_ERROR_MEMORY; "
          "goto cleanup; }\n"
          "        joined = tmp;\n"
          "        if (i > 0) joined[joined_len++] = '%c';\n"
          "        memcpy(joined + joined_len, enc, val_len);\n"
          "        joined_len += val_len;\n"
          "        joined[joined_len] = '\\0';\n"
          "      }\n",
          delim));

      CHECK_IO(fprintf(fp, "      free(enc);\n"));
      CHECK_IO(fprintf(fp, "      free(raw);\n"));

    } else {

      CHECK_IO(fprintf(fp, "      size_t val_len = strlen(raw);\n"));
      CHECK_IO(fprintf(

          fp,
          "      {\n"
          "        size_t extra = val_len + (i > 0 ? 1 : 0);\n"
          "        char *tmp = (char *)(size_t)realloc(joined, joined_len + "
          "extra + "
          "1);\n"
          "        if (!tmp) { free(raw); rc = CDD_C_ERROR_MEMORY; goto "
          "cleanup; }\n"
          "        joined = tmp;\n"
          "        if (i > 0) joined[joined_len++] = '%c';\n"
          "        memcpy(joined + joined_len, raw, val_len);\n"
          "        joined_len += val_len;\n"
          "        joined[joined_len] = '\\0';\n"
          "      }\n",
          delim));

      CHECK_IO(fprintf(fp, "      free(raw);\n"));
    }

  } else if (strcmp(items_type, "integer") == 0) {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      char num_buf[32];\n"));
    CHECK_IO(fprintf(fp,
                     "      spr"
                     "intf(num_buf, \"%%d\", req_body->%s[i]);\n",

                     field));

    CHECK_IO(fprintf(fp, "      raw = num_buf;\n"));
    if (do_encode) {
      CHECK_IO(fprintf(fp, "      char *enc = %s(raw);\n", encode_fn));
      CHECK_IO(fprintf(fp, "      size_t val_len;\n"));
      CHECK_IO(fprintf(

          fp, "      if (!enc) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));

      CHECK_IO(fprintf(fp, "      val_len = strlen(enc);\n"));
      CHECK_IO(fprintf(

          fp,
          "      {\n"
          "        size_t extra = val_len + (i > 0 ? 1 : 0);\n"
          "        char *tmp = (char *)(size_t)realloc(joined, joined_len + "
          "extra + "
          "1);\n"
          "        if (!tmp) { free(enc); rc = CDD_C_ERROR_MEMORY; goto "
          "cleanup; }\n"
          "        joined = tmp;\n"
          "        if (i > 0) joined[joined_len++] = '%c';\n"
          "        memcpy(joined + joined_len, enc, val_len);\n"
          "        joined_len += val_len;\n"
          "        joined[joined_len] = '\\0';\n"
          "      }\n",
          delim));

      CHECK_IO(fprintf(fp, "      free(enc);\n"));

    } else {

      CHECK_IO(fprintf(fp, "      size_t val_len = strlen(raw);\n"));
      CHECK_IO(fprintf(

          fp,
          "      {\n"
          "        size_t extra = val_len + (i > 0 ? 1 : 0);\n"
          "        char *tmp = (char *)(size_t)realloc(joined, joined_len "
          "+ extra + 1);\n"
          "        if (!tmp) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"
          "        joined = tmp;\n"
          "        if (i > 0) joined[joined_len++] = '%c';\n"
          "        memcpy(joined + joined_len, raw, val_len);\n"
          "        joined_len += val_len;\n"
          "        joined[joined_len] = '\\0';\n"
          "      }\n",
          delim));
    }

  } else if (strcmp(items_type, "number") == 0) {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
    CHECK_IO(fprintf(fp,
                     "      spr"
                     "intf(num_buf, \"%%g\", req_body->%s[i]);\n",

                     field));

    CHECK_IO(fprintf(fp, "      raw = num_buf;\n"));
    if (do_encode) {
      CHECK_IO(fprintf(fp, "      char *enc = %s(raw);\n", encode_fn));
      CHECK_IO(fprintf(fp, "      size_t val_len;\n"));
      CHECK_IO(fprintf(

          fp, "      if (!enc) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));

      CHECK_IO(fprintf(fp, "      val_len = strlen(enc);\n"));
      CHECK_IO(fprintf(

          fp,
          "      {\n"
          "        size_t extra = val_len + (i > 0 ? 1 : 0);\n"
          "        char *tmp = (char *)(size_t)realloc(joined, joined_len + "
          "extra + "
          "1);\n"
          "        if (!tmp) { free(enc); rc = CDD_C_ERROR_MEMORY; goto "
          "cleanup; }\n"
          "        joined = tmp;\n"
          "        if (i > 0) joined[joined_len++] = '%c';\n"
          "        memcpy(joined + joined_len, enc, val_len);\n"
          "        joined_len += val_len;\n"
          "        joined[joined_len] = '\\0';\n"
          "      }\n",
          delim));

      CHECK_IO(fprintf(fp, "      free(enc);\n"));

    } else {

      CHECK_IO(fprintf(fp, "      size_t val_len = strlen(raw);\n"));
      CHECK_IO(fprintf(

          fp,
          "      {\n"
          "        size_t extra = val_len + (i > 0 ? 1 : 0);\n"
          "        char *tmp = (char *)(size_t)realloc(joined, joined_len "
          "+ extra + 1);\n"
          "        if (!tmp) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"
          "        joined = tmp;\n"
          "        if (i > 0) joined[joined_len++] = '%c';\n"
          "        memcpy(joined + joined_len, raw, val_len);\n"
          "        joined_len += val_len;\n"
          "        joined[joined_len] = '\\0';\n"
          "      }\n",
          delim));
    }

  } else if (strcmp(items_type, "boolean") == 0) {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(

        fp, "      raw = req_body->%s[i] ? \"true\" : \"false\";\n", field));

    if (do_encode) {
      CHECK_IO(fprintf(fp, "      char *enc = %s(raw);\n", encode_fn));
      CHECK_IO(fprintf(fp, "      size_t val_len;\n"));
      CHECK_IO(fprintf(

          fp, "      if (!enc) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));

      CHECK_IO(fprintf(fp, "      val_len = strlen(enc);\n"));
      CHECK_IO(fprintf(

          fp,
          "      {\n"
          "        size_t extra = val_len + (i > 0 ? 1 : 0);\n"
          "        char *tmp = (char *)(size_t)realloc(joined, joined_len + "
          "extra + "
          "1);\n"
          "        if (!tmp) { free(enc); rc = CDD_C_ERROR_MEMORY; goto "
          "cleanup; }\n"
          "        joined = tmp;\n"
          "        if (i > 0) joined[joined_len++] = '%c';\n"
          "        memcpy(joined + joined_len, enc, val_len);\n"
          "        joined_len += val_len;\n"
          "        joined[joined_len] = '\\0';\n"
          "      }\n",
          delim));

      CHECK_IO(fprintf(fp, "      free(enc);\n"));

    } else {

      CHECK_IO(fprintf(fp, "      size_t val_len = strlen(raw);\n"));
      CHECK_IO(fprintf(

          fp,
          "      {\n"
          "        size_t extra = val_len + (i > 0 ? 1 : 0);\n"
          "        char *tmp = (char *)(size_t)realloc(joined, joined_len "
          "+ extra + 1);\n"
          "        if (!tmp) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"
          "        joined = tmp;\n"
          "        if (i > 0) joined[joined_len++] = '%c';\n"
          "        memcpy(joined + joined_len, raw, val_len);\n"
          "        joined_len += val_len;\n"
          "        joined[joined_len] = '\\0';\n"
          "      }\n",
          delim));
    }
  } else {

    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      raw = req_body->%s[i];\n", field));
    if (do_encode) {
      CHECK_IO(fprintf(fp, "      char *enc = %s(raw);\n", encode_fn));
      CHECK_IO(fprintf(fp, "      size_t val_len;\n"));
      CHECK_IO(fprintf(

          fp, "      if (!enc) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));

      CHECK_IO(fprintf(fp, "      val_len = strlen(enc);\n"));
      CHECK_IO(fprintf(

          fp,
          "      {\n"
          "        size_t extra = val_len + (i > 0 ? 1 : 0);\n"
          "        char *tmp = (char *)(size_t)realloc(joined, joined_len + "
          "extra + "
          "1);\n"
          "        if (!tmp) { free(enc); rc = CDD_C_ERROR_MEMORY; goto "
          "cleanup; }\n"
          "        joined = tmp;\n"
          "        if (i > 0) joined[joined_len++] = '%c';\n"
          "        memcpy(joined + joined_len, enc, val_len);\n"
          "        joined_len += val_len;\n"
          "        joined[joined_len] = '\\0';\n"
          "      }\n",
          delim));

      CHECK_IO(fprintf(fp, "      free(enc);\n"));

    } else {

      CHECK_IO(fprintf(fp, "      size_t val_len = strlen(raw);\n"));
      CHECK_IO(fprintf(

          fp,
          "      {\n"
          "        size_t extra = val_len + (i > 0 ? 1 : 0);\n"
          "        char *tmp = (char *)(size_t)realloc(joined, joined_len "
          "+ extra + 1);\n"
          "        if (!tmp) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"
          "        joined = tmp;\n"
          "        if (i > 0) joined[joined_len++] = '%c';\n"
          "        memcpy(joined + joined_len, raw, val_len);\n"
          "        joined_len += val_len;\n"
          "        joined[joined_len] = '\\0';\n"
          "      }\n",
          delim));
    }
  }

  CHECK_IO(fprintf(fp, "    }\n"));
  CHECK_IO(fprintf(fp, "    if (joined) {\n"));
  if (add_encoded) {
    CHECK_IO(fprintf(

        fp, "      rc = url_query_add_encoded(&form_qp, \"%s\", joined);\n",
        field));
  } else {

    CHECK_IO(fprintf(

        fp, "      rc = url_query_add(&form_qp, \"%s\", joined);\n", field));
  }

  CHECK_IO(fprintf(fp, "      free(joined);\n"));
  CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
  CHECK_IO(fprintf(fp, "    }\n"));
  CHECK_IO(fprintf(fp, "  }\n"));

  return CDD_C_SUCCESS;
}
