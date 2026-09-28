/**
 * @file client_body_cookie.c
 * @brief Cookie parameter serialization for client bodies.
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
 * @brief Generates C code for write cookie param logic.
 */
cdd_c_error_t
client_body_write_cookie_param_logic(FILE *fp,
                                     const struct OpenAPI_Operation *op) {
  size_t i;

  if (!fp || !op)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  CHECK_IO(fprintf(fp, "  /* Cookie Parameters */\n"));

  for (i = 0; i < op->n_parameters; ++i) {
    if (op->parameters[i].in != OA_PARAM_IN_COOKIE)

      continue;

    {
      const struct OpenAPI_Parameter *p = &op->parameters[i];
      const char *item_type = p->items_type ? p->items_type : "string";
      enum OpenAPI_Style style =
          (p->style == OA_STYLE_UNKNOWN) ? OA_STYLE_FORM : p->style;
      int explode =
          p->explode_set
              ? p->explode
              : ((style == OA_STYLE_FORM || style == OA_STYLE_COOKIE) ? 1 : 0);
      int allow_reserved = p->allow_reserved_set ? p->allow_reserved : 0;
      const char *encode_fn =
          (style == OA_STYLE_FORM)
              ? (allow_reserved ? "url_encode_allow_reserved" : "url_encode")
              : NULL;

      CHECK_IO(fprintf(fp, "  /* Cookie Parameter: %s */\n", p->name));

      if (p->type && strcmp(p->type, "object") == 0 && !p->is_array) {
        if (explode) {
          CHECK_IO(
              fprintf(fp, "  if (%s && %s_len > 0) {\n", p->name, p->name));
          CHECK_IO(fprintf(fp, "    size_t i;\n"));
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
          CHECK_IO(fprintf(fp, "      case OA_KV_BOOLEAN:\n"
                               "        kv_raw = kv->value.b ? \"true\" : "
                               "\"false\";\n"
                               "        break;\n"));
          CHECK_IO(fprintf(fp,
                           "      default:\n        kv_raw = NULL;\n        "
                           "break;\n"));
          CHECK_IO(fprintf(fp, "      }\n"));
          CHECK_IO(fprintf(fp, "      if (!kv_key || !kv_raw) continue;\n"));
          if (encode_fn) {
            CHECK_IO(fprintf(fp, "      {\n"));
            CHECK_IO(fprintf(fp, "        char *key_enc = %s(kv_key);\n",
                             encode_fn));
            CHECK_IO(fprintf(fp, "        char *val_enc = NULL;\n"));
            CHECK_IO(fprintf(
                fp, "        if (!key_enc) { rc = CDD_C_ERROR_MEMORY; goto "
                    "cleanup; }\n"));
            CHECK_IO(fprintf(fp, "        if (kv->type == OA_KV_STRING) {\n"));
            CHECK_IO(
                fprintf(fp, "          val_enc = %s(kv_raw);\n", encode_fn));
            CHECK_IO(fprintf(fp, "          if (!val_enc) { free(key_enc); rc "
                                 "= CDD_C_ERROR_MEMORY; goto "
                                 "cleanup; }\n"));
            CHECK_IO(fprintf(fp, "        }\n"));
            CHECK_IO(fprintf(fp, "        {\n"));
            CHECK_IO(fprintf(fp, "          const char *out_key = key_enc;\n"));
            CHECK_IO(fprintf(fp, "          const char *out_val = "
                                 "val_enc ? val_enc : kv_raw;\n"));
            CHECK_IO(
                fprintf(fp, "          size_t name_len = strlen(out_key);\n"));
            CHECK_IO(
                fprintf(fp, "          size_t val_len = strlen(out_val);\n"));
            CHECK_IO(fprintf(fp,
                             "          size_t extra = name_len + 1 + val_len "
                             "+ (cookie_len ? 2 : 0);\n"));
            CHECK_IO(fprintf(
                fp, "          char *tmp = (char *)(size_t)realloc(cookie_str, "
                    "cookie_len + extra + 1);\n"));
            CHECK_IO(fprintf(
                fp,
                "          if (!tmp) { free(key_enc); if (val_enc) "
                "free(val_enc); rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
            CHECK_IO(fprintf(fp, "          cookie_str = tmp;\n"));
            CHECK_IO(fprintf(fp, "          if (cookie_len) { "
                                 "cookie_str[cookie_len++] = ';'; "
                                 "cookie_str[cookie_len++] = ' '; }\n"));
            CHECK_IO(fprintf(fp, "          memcpy(cookie_str + cookie_len, "
                                 "out_key, name_len);\n"));
            CHECK_IO(fprintf(fp, "          cookie_len += name_len;\n"));
            CHECK_IO(
                fprintf(fp, "          cookie_str[cookie_len++] = '=';\n"));
            CHECK_IO(fprintf(fp, "          memcpy(cookie_str + cookie_len, "
                                 "out_val, val_len);\n"));
            CHECK_IO(fprintf(fp, "          cookie_len += val_len;\n"));
            CHECK_IO(
                fprintf(fp, "          cookie_str[cookie_len] = '\\0';\n"));
            CHECK_IO(fprintf(fp, "        }\n"));
            CHECK_IO(fprintf(fp, "        free(key_enc);\n"));
            CHECK_IO(fprintf(fp, "        if (val_enc) free(val_enc);\n"));
            CHECK_IO(fprintf(fp, "      }\n"));
          } else {

            CHECK_IO(fprintf(fp, "      {\n"));
            CHECK_IO(

                fprintf(fp, "        size_t name_len = strlen(kv_key);\n"));

            CHECK_IO(fprintf(fp, "        size_t val_len = strlen(kv_raw);\n"));
            CHECK_IO(fprintf(fp,

                             "        size_t extra = name_len + 1 + val_len + "
                             "(cookie_len ? 2 : 0);\n"));

            CHECK_IO(fprintf(
                fp,

                "        char *tmp = (char *)(size_t)realloc(cookie_str, "
                "cookie_len + extra + 1);\n"));

            CHECK_IO(fprintf(fp, "        if (!tmp) { rc = CDD_C_ERROR_MEMORY; "

                                 "goto cleanup; }\n"));

            CHECK_IO(fprintf(fp, "        cookie_str = tmp;\n"));
            CHECK_IO(fprintf(

                fp, "        if (cookie_len) { cookie_str[cookie_len++] "
                    "= ';'; cookie_str[cookie_len++] = ' '; }\n"));

            CHECK_IO(fprintf(fp,

                             "        memcpy(cookie_str + cookie_len, kv_key, "
                             "name_len);\n"));

            CHECK_IO(fprintf(fp, "        cookie_len += name_len;\n"));
            CHECK_IO(fprintf(fp, "        cookie_str[cookie_len++] = '=';\n"));
            CHECK_IO(fprintf(fp,

                             "        memcpy(cookie_str + cookie_len, kv_raw, "
                             "val_len);\n"));

            CHECK_IO(fprintf(fp, "        cookie_len += val_len;\n"));
            CHECK_IO(fprintf(fp, "        cookie_str[cookie_len] = '\\0';\n"));
            CHECK_IO(fprintf(fp, "      }\n"));
          }
          CHECK_IO(fprintf(fp, "    }\n  }\n"));
        } else {

          CHECK_IO(

              fprintf(fp, "  if (%s && %s_len > 0) {\n", p->name, p->name));

          CHECK_IO(fprintf(fp, "    size_t i;\n"));
          CHECK_IO(fprintf(fp, "    char *joined = NULL;\n"));
          CHECK_IO(fprintf(fp, "    size_t joined_len = 0;\n"));
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

          CHECK_IO(fprintf(fp,

                           "      case OA_KV_INTEGER:\n"
                           "        spr"
                           "intf(num_buf, \"%%d\", kv->value.i);\n"
                           "        kv_raw = num_buf;\n"
                           "        break;\n"));

          CHECK_IO(fprintf(fp,

                           "      case OA_KV_NUMBER:\n"
                           "        spr"
                           "intf(num_buf, \"%%g\", kv->value.n);\n"
                           "        kv_raw = num_buf;\n"
                           "        break;\n"));

          CHECK_IO(fprintf(fp, "      case OA_KV_BOOLEAN:\n"

                               "        kv_raw = kv->value.b ? \"true\" : "
                               "\"false\";\n"
                               "        break;\n"));

          CHECK_IO(fprintf(fp,

                           "      default:\n        kv_raw = NULL;\n        "
                           "break;\n"));

          CHECK_IO(fprintf(fp, "      }\n"));
          CHECK_IO(fprintf(fp, "      if (!kv_key || !kv_raw) continue;\n"));
          if (encode_fn) {
            CHECK_IO(fprintf(fp, "      {\n"));
            CHECK_IO(fprintf(fp, "        char *key_enc = %s(kv_key);\n",

                             encode_fn));

            CHECK_IO(fprintf(fp, "        char *val_enc = NULL;\n"));
            CHECK_IO(fprintf(

                fp, "        if (!key_enc) { rc = CDD_C_ERROR_MEMORY; goto "
                    "cleanup; }\n"));

            CHECK_IO(fprintf(fp, "        if (kv->type == OA_KV_STRING) {\n"));
            CHECK_IO(

                fprintf(fp, "          val_enc = %s(kv_raw);\n", encode_fn));

            CHECK_IO(fprintf(fp, "          if (!val_enc) { free(key_enc); rc "

                                 "= CDD_C_ERROR_MEMORY; goto "
                                 "cleanup; }\n"));

            CHECK_IO(fprintf(fp, "        }\n"));
            CHECK_IO(fprintf(fp, "        {\n"));
            CHECK_IO(fprintf(fp, "          const char *out_key = key_enc;\n"));
            CHECK_IO(fprintf(fp, "          const char *out_val = "

                                 "val_enc ? val_enc : kv_raw;\n"));

            CHECK_IO(

                fprintf(fp, "          size_t key_len = strlen(out_key);\n"));

            CHECK_IO(

                fprintf(fp, "          size_t val_len = strlen(out_val);\n"));

            CHECK_IO(fprintf(fp,

                             "          size_t extra = key_len + val_len + 2 + "
                             "(joined_len ? 1 : 0);\n"));

            CHECK_IO(
                fprintf(fp,

                        "          char *tmp = (char *)(size_t)realloc(joined, "
                        "joined_len + extra + 1);\n"));

            CHECK_IO(fprintf(fp, "          if (!tmp) { free(key_enc); if "

                                 "(val_enc) free(val_enc); "
                                 "rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));

            CHECK_IO(fprintf(fp, "          joined = tmp;\n"));
            CHECK_IO(fprintf(fp,

                             "          if (joined_len) joined[joined_len++] = "
                             "',';\n"));

            CHECK_IO(fprintf(fp,

                             "          memcpy(joined + joined_len, out_key, "
                             "key_len);\n"));

            CHECK_IO(fprintf(fp, "          joined_len += key_len;\n"));
            CHECK_IO(fprintf(fp, "          joined[joined_len++] = ',';\n"));
            CHECK_IO(fprintf(fp,

                             "          memcpy(joined + joined_len, out_val, "
                             "val_len);\n"));

            CHECK_IO(fprintf(fp, "          joined_len += val_len;\n"));
            CHECK_IO(fprintf(fp, "          joined[joined_len] = '\\0';\n"));
            CHECK_IO(fprintf(fp, "        }\n"));
            CHECK_IO(fprintf(fp, "        free(key_enc);\n"));
            CHECK_IO(fprintf(fp, "        if (val_enc) free(val_enc);\n"));
            CHECK_IO(fprintf(fp, "      }\n"));

          } else {

            CHECK_IO(fprintf(fp, "      {\n"));
            CHECK_IO(fprintf(fp, "        size_t key_len = strlen(kv_key);\n"));
            CHECK_IO(fprintf(fp, "        size_t val_len = strlen(kv_raw);\n"));
            CHECK_IO(fprintf(fp,

                             "        size_t extra = key_len + val_len + 2 + "
                             "(joined_len ? 1 : 0);\n"));

            CHECK_IO(fprintf(
                fp, "        char *tmp = (char *)(size_t)realloc(joined, "

                    "joined_len + extra + 1);\n"));

            CHECK_IO(fprintf(fp, "        if (!tmp) { rc = CDD_C_ERROR_MEMORY; "

                                 "goto cleanup; }\n"));

            CHECK_IO(fprintf(fp, "        joined = tmp;\n"));
            CHECK_IO(fprintf(fp,

                             "        if (joined_len) joined[joined_len++] = "
                             "',';\n"));

            CHECK_IO(fprintf(fp, "        memcpy(joined + joined_len, kv_key, "

                                 "key_len);\n"));

            CHECK_IO(fprintf(fp, "        joined_len += key_len;\n"));
            CHECK_IO(fprintf(fp, "        joined[joined_len++] = ',';\n"));
            CHECK_IO(fprintf(fp, "        memcpy(joined + joined_len, kv_raw, "

                                 "val_len);\n"));

            CHECK_IO(fprintf(fp, "        joined_len += val_len;\n"));
            CHECK_IO(fprintf(fp, "        joined[joined_len] = '\\0';\n"));
            CHECK_IO(fprintf(fp, "      }\n"));
          }

          CHECK_IO(fprintf(fp, "    }\n"));
          CHECK_IO(fprintf(fp, "    if (joined) {\n"));
          CHECK_IO(fprintf(fp, "      size_t name_len = strlen(\"%s\");\n",

                           p->name));

          CHECK_IO(fprintf(fp, "      size_t val_len = strlen(joined);\n"));
          CHECK_IO(fprintf(fp, "      size_t extra = name_len + 1 + val_len + "

                               "(cookie_len ? 2 : 0);\n"));

          CHECK_IO(fprintf(
              fp, "      char *tmp = (char *)(size_t)realloc(cookie_str, "

                  "cookie_len + extra + 1);\n"));

          CHECK_IO(fprintf(

              fp,
              "      if (!tmp) { free(joined); rc = CDD_C_ERROR_MEMORY; goto "
              "cleanup; }\n"));

          CHECK_IO(fprintf(fp, "      cookie_str = tmp;\n"));
          CHECK_IO(fprintf(fp,

                           "      if (cookie_len) { cookie_str[cookie_len++] = "
                           "';'; cookie_str[cookie_len++] = ' '; }\n"));

          CHECK_IO(fprintf(fp,

                           "      memcpy(cookie_str + cookie_len, \"%s\", "
                           "name_len);\n",
                           p->name));

          CHECK_IO(fprintf(fp, "      cookie_len += name_len;\n"));
          CHECK_IO(fprintf(fp, "      cookie_str[cookie_len++] = '=';\n"));
          CHECK_IO(fprintf(fp, "      memcpy(cookie_str + cookie_len, joined, "

                               "val_len);\n"));

          CHECK_IO(fprintf(fp, "      cookie_len += val_len;\n"));
          CHECK_IO(fprintf(fp, "      cookie_str[cookie_len] = '\\0';\n"));
          CHECK_IO(fprintf(fp, "      free(joined);\n"));
          CHECK_IO(fprintf(fp, "    }\n  }\n"));
        }
        continue;
      }

      if (p->is_array) {
        if (explode) {
          CHECK_IO(fprintf(fp, "  {\n    size_t i;\n"));
          CHECK_IO(fprintf(fp, "    for(i=0; i < %s_len; ++i) {\n", p->name));
          if (encode_fn) {
            CHECK_IO(fprintf(fp, "      char *cookie_enc = NULL;\n"));
          }
          if (strcmp(item_type, "integer") == 0) {

            CHECK_IO(fprintf(fp, "      const char *cookie_val;\n"));
            CHECK_IO(fprintf(fp, "      char num_buf[32];\n"));
            CHECK_IO(fprintf(fp,
                             "      spr"
                             "intf(num_buf, \"%%d\", %s[i]);\n",

                             p->name));

            CHECK_IO(fprintf(fp, "      cookie_val = num_buf;\n"));

          } else if (strcmp(item_type, "number") == 0) {
            CHECK_IO(fprintf(fp, "      const char *cookie_val;\n"));
            CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
            CHECK_IO(fprintf(fp,
                             "      spr"
                             "intf(num_buf, \"%%g\", %s[i]);\n",
                             p->name));
            CHECK_IO(fprintf(fp, "      cookie_val = num_buf;\n"));

          } else if (strcmp(item_type, "boolean") == 0) {
            CHECK_IO(fprintf(fp, "      const char *cookie_val;\n"));
            CHECK_IO(fprintf(

                fp, "      cookie_val = %s[i] ? \"true\" : \"false\";\n",
                p->name));
          } else {

            CHECK_IO(fprintf(fp, "      const char *cookie_val;\n"));
            if (encode_fn) {
              CHECK_IO(fprintf(fp, "      cookie_enc = %s(%s[i]);\n", encode_fn,

                               p->name));

              CHECK_IO(fprintf(fp, "      if (!cookie_enc) { rc = "

                                   "CDD_C_ERROR_MEMORY; goto cleanup; }\n"));

              CHECK_IO(fprintf(fp, "      cookie_val = cookie_enc;\n"));

            } else {

              CHECK_IO(fprintf(fp, "      cookie_val = %s[i];\n", p->name));
            }
          }
          CHECK_IO(fprintf(fp, "      if (cookie_val) {\n"));
          CHECK_IO(fprintf(fp, "        size_t name_len = strlen(\"%s\");\n",
                           p->name));
          CHECK_IO(
              fprintf(fp, "        size_t val_len = strlen(cookie_val);\n"));
          CHECK_IO(fprintf(fp,
                           "        size_t extra = name_len + 1 + val_len + "
                           "(cookie_len ? 2 : 0);\n"));
          CHECK_IO(fprintf(
              fp, "        char *tmp = (char *)(size_t)realloc(cookie_str, "
                  "cookie_len + extra + 1);\n"));
          CHECK_IO(fprintf(fp, "        if (!tmp) { rc = CDD_C_ERROR_MEMORY; "
                               "goto cleanup; }\n"));
          CHECK_IO(fprintf(fp, "        cookie_str = tmp;\n"));
          CHECK_IO(fprintf(
              fp, "        if (cookie_len) { cookie_str[cookie_len++] = "
                  "';'; cookie_str[cookie_len++] = ' '; }\n"));
          CHECK_IO(fprintf(fp,
                           "        memcpy(cookie_str + cookie_len, \"%s\", "
                           "name_len);\n",
                           p->name));
          CHECK_IO(fprintf(fp, "        cookie_len += name_len;\n"));
          CHECK_IO(fprintf(fp, "        cookie_str[cookie_len++] = '=';\n"));
          CHECK_IO(
              fprintf(fp, "        memcpy(cookie_str + cookie_len, cookie_val, "
                          "val_len);\n"));
          CHECK_IO(fprintf(fp, "        cookie_len += val_len;\n"));
          CHECK_IO(fprintf(fp, "        cookie_str[cookie_len] = '\\0';\n"));
          CHECK_IO(fprintf(fp, "      }\n"));
          if (encode_fn) {
            CHECK_IO(fprintf(fp, "      if (cookie_enc) free(cookie_enc);\n"));
          }
          CHECK_IO(fprintf(fp, "    }\n  }\n"));
        } else {
          CHECK_IO(fprintf(fp, "  {\n    size_t i;\n"));
          CHECK_IO(fprintf(fp, "    char *joined = NULL;\n"));
          CHECK_IO(fprintf(fp, "    size_t joined_len = 0;\n"));
          CHECK_IO(fprintf(fp, "    for(i=0; i < %s_len; ++i) {\n", p->name));
          if (encode_fn) {
            CHECK_IO(fprintf(fp, "      char *raw_enc = NULL;\n"));
          }
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
            if (encode_fn) {
              CHECK_IO(fprintf(fp, "      raw_enc = %s(%s[i]);\n", encode_fn,
                               p->name));
              CHECK_IO(fprintf(fp, "      if (!raw_enc) { rc = "
                                   "CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
              CHECK_IO(fprintf(fp, "      raw = raw_enc;\n"));
            } else {

              CHECK_IO(fprintf(fp, "      raw = %s[i];\n", p->name));
            }
          }
          CHECK_IO(fprintf(fp, "      if (raw) {\n"));
          CHECK_IO(fprintf(fp, "        size_t val_len = strlen(raw);\n"));
          CHECK_IO(fprintf(
              fp,
              "        size_t extra = val_len + (joined_len > 0 ? 1 : 0);\n"));
          CHECK_IO(
              fprintf(fp, "        char *tmp = (char *)(size_t)realloc(joined, "
                          "joined_len + extra + 1);\n"));
          CHECK_IO(fprintf(fp, "        if (!tmp) { rc = CDD_C_ERROR_MEMORY; "
                               "goto cleanup; }\n"));
          CHECK_IO(fprintf(fp, "        joined = tmp;\n"));
          CHECK_IO(fprintf(
              fp, "        if (joined_len > 0) joined[joined_len++] = ',';\n"));
          CHECK_IO(fprintf(
              fp, "        memcpy(joined + joined_len, raw, val_len);\n"));
          CHECK_IO(fprintf(fp, "        joined_len += val_len;\n"));
          CHECK_IO(fprintf(fp, "        joined[joined_len] = '\\0';\n"));
          CHECK_IO(fprintf(fp, "      }\n"));
          if (encode_fn) {
            CHECK_IO(fprintf(fp, "      if (raw_enc) free(raw_enc);\n"));
          }
          CHECK_IO(fprintf(fp, "    }\n"));
          CHECK_IO(fprintf(fp, "    if (joined) {\n"));
          CHECK_IO(fprintf(fp, "      size_t name_len = strlen(\"%s\");\n",
                           p->name));
          CHECK_IO(fprintf(fp, "      size_t val_len = strlen(joined);\n"));
          CHECK_IO(fprintf(fp, "      size_t extra = name_len + 1 + val_len + "
                               "(cookie_len ? 2 : 0);\n"));
          CHECK_IO(fprintf(
              fp, "      char *tmp = (char *)(size_t)realloc(cookie_str, "
                  "cookie_len + extra + 1);\n"));
          CHECK_IO(fprintf(
              fp,
              "      if (!tmp) { free(joined); rc = CDD_C_ERROR_MEMORY; goto "
              "cleanup; }\n"));
          CHECK_IO(fprintf(fp, "      cookie_str = tmp;\n"));
          CHECK_IO(fprintf(fp,
                           "      if (cookie_len) { cookie_str[cookie_len++] = "
                           "';'; cookie_str[cookie_len++] = ' '; }\n"));
          CHECK_IO(fprintf(fp,
                           "      memcpy(cookie_str + cookie_len, \"%s\", "
                           "name_len);\n",
                           p->name));
          CHECK_IO(fprintf(fp, "      cookie_len += name_len;\n"));
          CHECK_IO(fprintf(fp, "      cookie_str[cookie_len++] = '=';\n"));
          CHECK_IO(fprintf(fp, "      memcpy(cookie_str + cookie_len, joined, "
                               "val_len);\n"));
          CHECK_IO(fprintf(fp, "      cookie_len += val_len;\n"));
          CHECK_IO(fprintf(fp, "      cookie_str[cookie_len] = '\\0';\n"));
          CHECK_IO(fprintf(fp, "      free(joined);\n"));
          CHECK_IO(fprintf(fp, "    }\n  }\n"));
        }
      } else if (strcmp(p->type, "string") == 0) {
        if (encode_fn) {
          CHECK_IO(fprintf(fp, "  if (%s) {\n", p->name));
          CHECK_IO(fprintf(fp, "    char *cookie_val = %s(%s);\n", encode_fn,
                           p->name));
          CHECK_IO(fprintf(fp, "    if (!cookie_val) { rc = "
                               "CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
          CHECK_IO(
              fprintf(fp, "    size_t name_len = strlen(\"%s\");\n", p->name));
          CHECK_IO(fprintf(fp, "    size_t val_len = strlen(cookie_val);\n"));
          CHECK_IO(fprintf(fp, "    size_t extra = name_len + 1 + val_len + "
                               "(cookie_len ? 2 : 0);\n"));
          CHECK_IO(
              fprintf(fp, "    char *tmp = (char *)(size_t)realloc(cookie_str, "
                          "cookie_len + extra + 1);\n"));
          CHECK_IO(fprintf(
              fp,
              "    if (!tmp) { free(cookie_val); rc = CDD_C_ERROR_MEMORY; goto "
              "cleanup; }\n"));
          CHECK_IO(fprintf(fp, "    cookie_str = tmp;\n"));
          CHECK_IO(fprintf(fp,
                           "    if (cookie_len) { cookie_str[cookie_len++] = "
                           "';'; cookie_str[cookie_len++] = ' '; }\n"));
          CHECK_IO(fprintf(
              fp, "    memcpy(cookie_str + cookie_len, \"%s\", name_len);\n",
              p->name));
          CHECK_IO(fprintf(fp, "    cookie_len += name_len;\n"));
          CHECK_IO(fprintf(fp, "    cookie_str[cookie_len++] = '=';\n"));
          CHECK_IO(fprintf(fp,
                           "    memcpy(cookie_str + cookie_len, cookie_val, "
                           "val_len);\n"));
          CHECK_IO(fprintf(fp, "    cookie_len += val_len;\n"));
          CHECK_IO(fprintf(fp, "    cookie_str[cookie_len] = '\\0';\n"));
          CHECK_IO(fprintf(fp, "    free(cookie_val);\n"));
          CHECK_IO(fprintf(fp, "  }\n"));
        } else {

          CHECK_IO(fprintf(fp, "  if (%s) {\n", p->name));
          CHECK_IO(fprintf(fp, "    const char *cookie_val = %s;\n", p->name));
          CHECK_IO(

              fprintf(fp, "    size_t name_len = strlen(\"%s\");\n", p->name));

          CHECK_IO(fprintf(fp, "    size_t val_len = strlen(cookie_val);\n"));
          CHECK_IO(fprintf(fp, "    size_t extra = name_len + 1 + val_len + "

                               "(cookie_len ? 2 : 0);\n"));

          CHECK_IO(
              fprintf(fp, "    char *tmp = (char *)(size_t)realloc(cookie_str, "

                          "cookie_len + extra + 1);\n"));

          CHECK_IO(fprintf(

              fp,
              "    if (!tmp) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));

          CHECK_IO(fprintf(fp, "    cookie_str = tmp;\n"));
          CHECK_IO(fprintf(fp,

                           "    if (cookie_len) { cookie_str[cookie_len++] = "
                           "';'; cookie_str[cookie_len++] = ' '; }\n"));

          CHECK_IO(fprintf(

              fp, "    memcpy(cookie_str + cookie_len, \"%s\", name_len);\n",
              p->name));

          CHECK_IO(fprintf(fp, "    cookie_len += name_len;\n"));
          CHECK_IO(fprintf(fp, "    cookie_str[cookie_len++] = '=';\n"));
          CHECK_IO(fprintf(fp,

                           "    memcpy(cookie_str + cookie_len, cookie_val, "
                           "val_len);\n"));

          CHECK_IO(fprintf(fp, "    cookie_len += val_len;\n"));
          CHECK_IO(fprintf(fp, "    cookie_str[cookie_len] = '\\0';\n"));
          CHECK_IO(fprintf(fp, "  }\n"));
        }

      } else if (strcmp(p->type, "integer") == 0) {
        CHECK_IO(fprintf(fp, "  {\n    char num_buf[32];\n"));
        CHECK_IO(fprintf(fp,
                         "    spr"
                         "intf(num_buf, \"%%d\", %s);\n",
                         p->name));
        CHECK_IO(

            fprintf(fp, "    size_t name_len = strlen(\"%s\");\n", p->name));

        CHECK_IO(fprintf(fp, "    size_t val_len = strlen(num_buf);\n"));
        CHECK_IO(fprintf(fp, "    size_t extra = name_len + 1 + val_len + "

                             "(cookie_len ? 2 : 0);\n"));

        CHECK_IO(fprintf(fp,
                         "    char *tmp = (char *)(size_t)realloc(cookie_str, "

                         "cookie_len + extra + 1);\n"));

        CHECK_IO(fprintf(

            fp, "    if (!tmp) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));

        CHECK_IO(fprintf(fp, "    cookie_str = tmp;\n"));
        CHECK_IO(fprintf(fp, "    if (cookie_len) { cookie_str[cookie_len++] = "

                             "';'; cookie_str[cookie_len++] = ' '; }\n"));

        CHECK_IO(fprintf(

            fp, "    memcpy(cookie_str + cookie_len, \"%s\", name_len);\n",
            p->name));

        CHECK_IO(fprintf(fp, "    cookie_len += name_len;\n"));
        CHECK_IO(fprintf(fp, "    cookie_str[cookie_len++] = '=';\n"));
        CHECK_IO(fprintf(

            fp, "    memcpy(cookie_str + cookie_len, num_buf, val_len);\n"));

        CHECK_IO(fprintf(fp, "    cookie_len += val_len;\n"));
        CHECK_IO(fprintf(fp, "    cookie_str[cookie_len] = '\\0';\n"));
        CHECK_IO(fprintf(fp, "  }\n"));
      } else if (strcmp(p->type, "number") == 0) {
        CHECK_IO(fprintf(fp, "  {\n    char num_buf[64];\n"));
        CHECK_IO(fprintf(fp,
                         "    spr"
                         "intf(num_buf, \"%%g\", %s);\n",
                         p->name));
        CHECK_IO(

            fprintf(fp, "    size_t name_len = strlen(\"%s\");\n", p->name));

        CHECK_IO(fprintf(fp, "    size_t val_len = strlen(num_buf);\n"));
        CHECK_IO(fprintf(fp, "    size_t extra = name_len + 1 + val_len + "

                             "(cookie_len ? 2 : 0);\n"));

        CHECK_IO(fprintf(fp,
                         "    char *tmp = (char *)(size_t)realloc(cookie_str, "

                         "cookie_len + extra + 1);\n"));

        CHECK_IO(fprintf(

            fp, "    if (!tmp) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));

        CHECK_IO(fprintf(fp, "    cookie_str = tmp;\n"));
        CHECK_IO(fprintf(fp, "    if (cookie_len) { cookie_str[cookie_len++] = "

                             "';'; cookie_str[cookie_len++] = ' '; }\n"));

        CHECK_IO(fprintf(

            fp, "    memcpy(cookie_str + cookie_len, \"%s\", name_len);\n",
            p->name));

        CHECK_IO(fprintf(fp, "    cookie_len += name_len;\n"));
        CHECK_IO(fprintf(fp, "    cookie_str[cookie_len++] = '=';\n"));
        CHECK_IO(fprintf(

            fp, "    memcpy(cookie_str + cookie_len, num_buf, val_len);\n"));

        CHECK_IO(fprintf(fp, "    cookie_len += val_len;\n"));
        CHECK_IO(fprintf(fp, "    cookie_str[cookie_len] = '\\0';\n"));
        CHECK_IO(fprintf(fp, "  }\n"));
      } else if (strcmp(p->type, "boolean") == 0) {
        CHECK_IO(fprintf(

            fp,
            "  {\n    const char *cookie_val = %s ? \"true\" : \"false\";\n",
            p->name));

        CHECK_IO(

            fprintf(fp, "    size_t name_len = strlen(\"%s\");\n", p->name));

        CHECK_IO(fprintf(fp, "    size_t val_len = strlen(cookie_val);\n"));
        CHECK_IO(fprintf(fp, "    size_t extra = name_len + 1 + val_len + "

                             "(cookie_len ? 2 : 0);\n"));

        CHECK_IO(fprintf(fp,
                         "    char *tmp = (char *)(size_t)realloc(cookie_str, "

                         "cookie_len + extra + 1);\n"));

        CHECK_IO(fprintf(

            fp, "    if (!tmp) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));

        CHECK_IO(fprintf(fp, "    cookie_str = tmp;\n"));
        CHECK_IO(fprintf(fp, "    if (cookie_len) { cookie_str[cookie_len++] = "

                             "';'; cookie_str[cookie_len++] = ' '; }\n"));

        CHECK_IO(fprintf(

            fp, "    memcpy(cookie_str + cookie_len, \"%s\", name_len);\n",
            p->name));

        CHECK_IO(fprintf(fp, "    cookie_len += name_len;\n"));
        CHECK_IO(fprintf(fp, "    cookie_str[cookie_len++] = '=';\n"));
        CHECK_IO(fprintf(

            fp, "    memcpy(cookie_str + cookie_len, cookie_val, val_len);\n"));

        CHECK_IO(fprintf(fp, "    cookie_len += val_len;\n"));
        CHECK_IO(fprintf(fp, "    cookie_str[cookie_len] = '\\0';\n"));
        CHECK_IO(fprintf(fp, "  }\n"));
      }
    }
  }

  CHECK_IO(fprintf(fp, "  if (cookie_str) {\n"));
  CHECK_IO(fprintf(
      fp,
      "    rc = http_headers_add(&req.headers, \"Cookie\", cookie_str);\n"));
  CHECK_IO(fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
  CHECK_IO(fprintf(fp, "  }\n"));

  return CDD_C_SUCCESS;
}
