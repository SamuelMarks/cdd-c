/**
 * @file standalone_json.c
 * @brief Zero-dependency UDT JSON parser and emitter code generator
 * implementation.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/code_writer.h"
#include "classes/emit/json.h"
#include "classes/emit/struct.h"
#include "classes/parse/cdd_serde.h"
/* clang-format on */

/** @brief CHECK_IO macro */
#define CHECK_IO(x)                                                            \
  for (; (x) < 0;)                                                             \
  return CDD_C_ERROR_IO

/**
 * @brief Emits self-contained C89 zero-dependency JSON scanner and buffer
 * writer runtime helpers, guarded by `#ifndef
 * CDD_JSON_STANDALONE_HELPERS_DEFINED`.
 *
 * @param[in] fp Output stream.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO on write failure.
 */
static cdd_c_error_t write_standalone_json_helpers(FILE *fp) {
  CHECK_IO(FPRINTF_HOOK(fp, "#ifndef CDD_JSON_STANDALONE_HELPERS_DEFINED\n"
                            "#define CDD_JSON_STANDALONE_HELPERS_DEFINED\n"
                            "struct _cdd_json_buf {\n"
                            "  char *data;\n"
                            "  size_t len;\n"
                            "  size_t cap;\n"
                            "};\n"));
  fprintf(fp,
          "static cdd_c_error_t _cdd_json_buf_append(struct _cdd_json_buf *b, "
          "const char *s, size_t n) {\n"
          "  if (b->len + n + 1 > b->cap) {\n"
          "    size_t ncap = b->cap == 0 ? 128 : b->cap * 2;\n"
          "    char *tmp;\n"
          "    while (ncap < b->len + n + 1) ncap *= 2;\n"
          "    tmp = (char *)realloc(b->data, ncap);\n"
          "    if (!tmp) return CDD_C_ERROR_MEMORY;\n"
          "    b->data = tmp;\n"
          "    b->cap = ncap;\n"
          "  }\n");
  fprintf(fp, "  memcpy(b->data + b->len, s, n);\n"
              "  b->len += n;\n"
              "  b->data[b->len] = '\\0';\n"
              "  return CDD_C_SUCCESS;\n"
              "}\n");
  fprintf(
      fp,
      "static cdd_c_error_t _cdd_json_buf_append_escaped(struct _cdd_json_buf "
      "*b, const char *s) {\n"
      "  size_t i;\n"
      "  cdd_c_error_t rc;\n"
      "  if (!s) return _cdd_json_buf_append(b, \"null\", 4);\n"
      "  rc = _cdd_json_buf_append(b, \"\\\"\", 1);\n"
      "  if (rc != CDD_C_SUCCESS) return rc;\n"
      "  for (i = 0; s[i] != '\\0'; ++i) {\n"
      "    unsigned char c = (unsigned char)s[i];\n"
      "    if (c == '\"') rc = _cdd_json_buf_append(b, \"\\\\\\\"\", 2);\n"
      "    else if (c == '\\\\') rc = _cdd_json_buf_append(b, \"\\\\\\\\\", "
      "2);\n");
  fprintf(
      fp,
      "    else if (c == '\\b') rc = _cdd_json_buf_append(b, \"\\\\b\", 2);\n"
      "    else if (c == '\\f') rc = _cdd_json_buf_append(b, \"\\\\f\", 2);\n"
      "    else if (c == '\\n') rc = _cdd_json_buf_append(b, \"\\\\n\", 2);\n"
      "    else if (c == '\\r') rc = _cdd_json_buf_append(b, \"\\\\r\", 2);\n"
      "    else if (c == '\\t') rc = _cdd_json_buf_append(b, \"\\\\t\", 2);\n");
  fprintf(
      fp,
      "    else if (c < 0x20) {\n"
      "      char uhex[7];\n"
      "#if defined(_MSC_VER)\n"
      "      sprintf_s(uhex, sizeof(uhex), \"\\\\u%%04x\", (unsigned int)c);\n"
      "#else\n"
      "      sprintf(uhex, \"\\\\u%%04x\", (unsigned int)c);\n"
      "#endif\n"
      "      rc = _cdd_json_buf_append(b, uhex, 6);\n");
  fprintf(fp, "    } else {\n"
              "      char ch = (char)c;\n"
              "      rc = _cdd_json_buf_append(b, &ch, 1);\n"
              "    }\n"
              "    if (rc != CDD_C_SUCCESS) return rc;\n"
              "  }\n"
              "  return _cdd_json_buf_append(b, \"\\\"\", 1);\n"
              "}\n"
              "static cdd_c_error_t _cdd_json_skip_ws(const char **p) {\n"
              "  if (!p || !*p) return CDD_C_ERROR_INVALID_ARGUMENT;\n"
              "  while (**p == ' ' || **p == '\\t' || **p == '\\n' || **p == "
              "'\\r') (*p)++;\n"
              "  return CDD_C_SUCCESS;\n"
              "}\n");
  fprintf(
      fp,
      "static cdd_c_error_t _cdd_json_parse_hex4(const char *s, unsigned "
      "int *out) {\n"
      "  unsigned int v = 0;\n"
      "  int i;\n"
      "  for (i = 0; i < 4; ++i) {\n"
      "    char c = s[i];\n"
      "    v <<= 4;\n"
      "    if (c >= '0' && c <= '9') v |= (unsigned int)(c - '0');\n"
      "    else if (c >= 'a' && c <= 'f') v |= (unsigned int)(c - 'a' + 10);\n"
      "    else if (c >= 'A' && c <= 'F') v |= (unsigned int)(c - 'A' + 10);\n"
      "    else return CDD_C_ERROR_PARSE;\n"
      "  }\n"
      "  *out = v;\n"
      "  return CDD_C_SUCCESS;\n"
      "}\n");
  fprintf(fp, "static cdd_c_error_t _cdd_json_unescape_str(const char **p, "
              "char **out) {\n"
              "  const char *start;\n"
              "  size_t max_len, oi = 0;\n"
              "  char *res;\n"
              "  if (**p != '\"') return CDD_C_ERROR_INVALID_ARGUMENT;\n"
              "  (*p)++;\n"
              "  start = *p;\n"
              "  while (**p && **p != '\"') {\n"
              "    if (**p == '\\' && *(*p + 1)) *p += 2;\n"
              "    else (*p)++;\n"
              "  }\n"
              "  if (**p != '\"') return CDD_C_ERROR_INVALID_ARGUMENT;\n"
              "  max_len = (size_t)(*p - start);\n");
  fprintf(fp, "  res = (char *)malloc(max_len + 1);\n"
              "  if (!res) return CDD_C_ERROR_MEMORY;\n"
              "  *p = start;\n"
              "  while (**p && **p != '\"') {\n"
              "    if (**p == '\\') {\n"
              "      (*p)++;\n"
              "      if (**p == '\"' || **p == '\\' || **p == '/') res[oi++] = "
              "*(*p)++;\n"
              "      else if (**p == 'b') { res[oi++] = '\\b'; (*p)++; }\n"
              "      else if (**p == 'f') { res[oi++] = '\\f'; (*p)++; }\n"
              "      else if (**p == 'n') { res[oi++] = '\\n'; (*p)++; }\n");
  fprintf(fp, "      else if (**p == 'r') { res[oi++] = '\\r'; (*p)++; }\n"
              "      else if (**p == 't') { res[oi++] = '\\t'; (*p)++; }\n"
              "      else if (**p == 'u') {\n"
              "        unsigned int cp = 0;\n"
              "        (*p)++;\n"
              "        if (_cdd_json_parse_hex4(*p, &cp) != CDD_C_SUCCESS) { "
              "free(res); return CDD_C_ERROR_PARSE; }\n"
              "        *p += 4;\n");
  fprintf(
      fp,
      "        if (cp >= 0xD800 && cp <= 0xDBFF && (*p)[0] == '\\' && (*p)[1] "
      "== 'u') {\n"
      "          unsigned int low = 0;\n"
      "          if (_cdd_json_parse_hex4(*p + 2, &low) == CDD_C_SUCCESS && "
      "low >= 0xDC00 && low <= 0xDFFF) {\n"
      "            cp = 0x10000 + (((cp - 0xD800) << 10) | (low - 0xDC00));\n"
      "            *p += 6;\n"
      "          }\n"
      "        }\n");
  fprintf(fp, "        if (cp <= 0x7F) {\n"
              "          res[oi++] = (char)cp;\n"
              "        } else if (cp <= 0x7FF) {\n"
              "          res[oi++] = (char)(0xC0 | ((cp >> 6) & 0x1F));\n"
              "          res[oi++] = (char)(0x80 | (cp & 0x3F));\n"
              "        } else if (cp <= 0xFFFF) {\n"
              "          res[oi++] = (char)(0xE0 | ((cp >> 12) & 0x0F));\n"
              "          res[oi++] = (char)(0x80 | ((cp >> 6) & 0x3F));\n"
              "          res[oi++] = (char)(0x80 | (cp & 0x3F));\n");
  fprintf(fp, "        } else {\n"
              "          res[oi++] = (char)(0xF0 | ((cp >> 18) & 0x07));\n"
              "          res[oi++] = (char)(0x80 | ((cp >> 12) & 0x3F));\n"
              "          res[oi++] = (char)(0x80 | ((cp >> 6) & 0x3F));\n"
              "          res[oi++] = (char)(0x80 | (cp & 0x3F));\n"
              "        }\n"
              "      } else res[oi++] = *(*p)++;\n"
              "    } else {\n"
              "      res[oi++] = *(*p)++;\n"
              "    }\n"
              "  }\n"
              "  if (**p == '\"') (*p)++;\n"
              "  res[oi] = '\\0';\n"
              "  *out = res;\n"
              "  return CDD_C_SUCCESS;\n"
              "}\n");
  fprintf(fp, "static cdd_c_error_t _cdd_json_skip_value(const char **p) {\n"
              "  cdd_c_error_t rc = _cdd_json_skip_ws(p);\n"
              "  if (rc != CDD_C_SUCCESS) return rc;\n"
              "  if (**p == '\"') {\n"
              "    (*p)++;\n"
              "    while (**p && **p != '\"') {\n"
              "      if (**p == '\\' && *(*p + 1)) *p += 2;\n"
              "      else (*p)++;\n"
              "    }\n"
              "    if (**p == '\"') (*p)++;\n"
              "  } else if (**p == '{' || **p == '[') {\n"
              "    char open = **p;\n"
              "    char close = (open == '{') ? '}' : ']';\n"
              "    int depth = 1;\n"
              "    (*p)++;\n");
  fprintf(fp, "    while (**p && depth > 0) {\n"
              "      if (**p == '\"') {\n"
              "        (*p)++;\n"
              "        while (**p && **p != '\"') {\n"
              "          if (**p == '\\' && *(*p + 1)) *p += 2;\n"
              "          else (*p)++;\n"
              "        }\n"
              "        if (**p == '\"') (*p)++;\n"
              "      } else {\n"
              "        if (**p == open) depth++;\n"
              "        else if (**p == close) depth--;\n"
              "        (*p)++;\n"
              "      }\n"
              "    }\n");
  fprintf(
      fp,
      "  } else {\n"
      "    while (**p && **p != ',' && **p != '}' && **p != ']' && **p != ' ' "
      "&& **p != '\\t' && **p != '\\n' && **p != '\\r') (*p)++;\n"
      "  }\n"
      "  return CDD_C_SUCCESS;\n"
      "}\n"
      "#endif /* CDD_JSON_STANDALONE_HELPERS_DEFINED */\n\n");
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits array element deserialization statements for a 1D array
 * field in standalone JSON parsing.
 *
 * @param[in] fp Output stream.
 * @param[in] f Field descriptor.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t
write_standalone_array_from_json(FILE *fp, const struct StructField *f) {
  const char *elem = f->ref;
  if (elem[0] == 0) {
    fprintf(
        fp,
        "        if (_cdd_json_skip_value(&p) != CDD_C_SUCCESS) goto err;\n");
    return CDD_C_SUCCESS;
  }

  fprintf(fp,
          "        if (*p == '[') {\n"
          "          p++;\n"
          "          while (*p) {\n"
          "            if (_cdd_json_skip_ws(&p) != CDD_C_SUCCESS) goto err;\n"
          "            if (*p == ']') { p++; break; }\n"
          "            if (*p == ',') { p++; if (_cdd_json_skip_ws(&p) != "
          "CDD_C_SUCCESS) goto err; }\n"
          "            {\n"
          "              size_t _idx = ret->n_%s;\n"
          "              void *_na = realloc((void *)ret->%s, (_idx + 1) * "
          "sizeof(*ret->%s));\n"
          "              if (!_na) goto err;\n"
          "              ret->%s = _na;\n",
          f->name, f->name, f->name, f->name);
  if (strcmp(elem, "integer") == 0) {
    fprintf(fp,
            "              ret->%s[_idx] = (int)strtol(p, (char **)&p, 10);\n",
            f->name);
  } else if (strcmp(elem, "number") == 0) {
    fprintf(fp, "              ret->%s[_idx] = strtod(p, (char **)&p);\n",
            f->name);
  } else if (strcmp(elem, "boolean") == 0) {
    fprintf(fp,
            "              ret->%s[_idx] = (strncmp(p, \"true\", 4) == 0) ? 1 "
            ": 0;\n"
            "              if (_cdd_json_skip_value(&p) != CDD_C_SUCCESS) goto "
            "err;\n",
            f->name);
  } else if (strcmp(elem, "string") == 0) {
    fprintf(fp,
            "              ret->%s[_idx] = NULL;\n"
            "              if (_cdd_json_unescape_str(&p, &ret->%s[_idx]) != "
            "CDD_C_SUCCESS) goto err;\n",
            f->name, f->name);
  } else if (f->is_enum_array) {
    const char *tn = NULL;
    const char *er = f->enum_array_ref[0] ? f->enum_array_ref : elem;
    cdd_c_error_t rc_ref = get_type_from_ref(er, &tn);
    if (rc_ref != CDD_C_SUCCESS)
      return rc_ref;
    fprintf(fp,
            "              {\n"
            "                char *_es = NULL;\n"
            "                if (_cdd_json_unescape_str(&p, &_es) != "
            "CDD_C_SUCCESS) goto err;\n"
            "                if (%s_from_str(_es, &ret->%s[_idx]) != "
            "CDD_C_SUCCESS) { free(_es); goto err; }\n"
            "                free(_es);\n"
            "              }\n",
            tn, f->name);
  } else {
    const char *tn = NULL;
    cdd_c_error_t rc_ref = get_type_from_ref(elem, &tn);
    if (rc_ref != CDD_C_SUCCESS)
      return rc_ref;
    fprintf(fp,
            "              ret->%s[_idx] = NULL;\n"
            "              if (%s_parse_json(p, &ret->%s[_idx]) != "
            "CDD_C_SUCCESS) goto err;\n"
            "              if (_cdd_json_skip_value(&p) != CDD_C_SUCCESS) goto "
            "err;\n",
            f->name, tn, f->name);
  }
  fprintf(fp,
          "              ret->n_%s++;\n"
          "            }\n"
          "          }\n",
          f->name);
  if (f->has_min_items) {
    fprintf(fp, "          if (ret->n_%s < %lu) goto err;\n", f->name,
            (unsigned long)f->min_items);
  }
  if (f->has_max_items) {
    fprintf(fp, "          if (ret->n_%s > %lu) goto err;\n", f->name,
            (unsigned long)f->max_items);
  }
  fprintf(fp,
          "        } else {\n"
          "          if (_cdd_json_skip_value(&p) != CDD_C_SUCCESS) goto err;\n"
          "        }\n");
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits array element serialization statements for a 1D array
 * field in standalone JSON emission.
 *
 * @param[in] fp Output stream.
 * @param[in] f Field descriptor.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t
write_standalone_array_to_json(FILE *fp, const struct StructField *f) {
  const char *elem = f->ref;
  if (elem[0] == 0) {
    CHECK_IO(FPRINTF_HOOK(
        fp, "  _rc = _cdd_json_buf_append(&_b, \"[]\", 2);\n"
            "  if (_rc != CDD_C_SUCCESS) { free(_b.data); return _rc; }\n"));
    return CDD_C_SUCCESS;
  }

  CHECK_IO(FPRINTF_HOOK(
      fp,
      "  { size_t _i;\n"
      "    _rc = _cdd_json_buf_append(&_b, \"[\", 1);\n"
      "    if (_rc != CDD_C_SUCCESS) { free(_b.data); return _rc; }\n"
      "    for (_i = 0; _i < obj->n_%s; ++_i) {\n"
      "      if (_i > 0) { _rc = _cdd_json_buf_append(&_b, \",\", 1); if (_rc "
      "!= CDD_C_SUCCESS) { free(_b.data); return _rc; } }\n",
      f->name));
  if (strcmp(elem, "integer") == 0) {
    CHECK_IO(FPRINTF_HOOK(
        fp,
        "      { char _nb[32];\n"
        "#if defined(_MSC_VER)\n"
        "        sprintf_s(_nb, sizeof(_nb), \"%%d\", obj->%s[_i]);\n"
        "#else\n"
        "        sprintf(_nb, \"%%d\", obj->%s[_i]);\n"
        "#endif\n"
        "        _rc = _cdd_json_buf_append(&_b, _nb, strlen(_nb));\n"
        "        if (_rc != CDD_C_SUCCESS) { free(_b.data); return _rc; }\n"
        "      }\n",
        f->name, f->name));
  } else if (strcmp(elem, "number") == 0) {
    CHECK_IO(FPRINTF_HOOK(
        fp,
        "      { char _nb[64];\n"
        "#if defined(_MSC_VER)\n"
        "        sprintf_s(_nb, sizeof(_nb), \"%%.17g\", obj->%s[_i]);\n"
        "#else\n"
        "        sprintf(_nb, \"%%.17g\", obj->%s[_i]);\n"
        "#endif\n"
        "        _rc = _cdd_json_buf_append(&_b, _nb, strlen(_nb));\n"
        "        if (_rc != CDD_C_SUCCESS) { free(_b.data); return _rc; }\n"
        "      }\n",
        f->name, f->name));
  } else if (strcmp(elem, "boolean") == 0) {
    CHECK_IO(FPRINTF_HOOK(
        fp,
        "      _rc = _cdd_json_buf_append(&_b, obj->%s[_i] ? \"true\" : "
        "\"false\", obj->%s[_i] ? 4 : 5);\n"
        "      if (_rc != CDD_C_SUCCESS) { free(_b.data); return _rc; }\n",
        f->name, f->name));
  } else if (strcmp(elem, "string") == 0) {
    CHECK_IO(FPRINTF_HOOK(
        fp,
        "      _rc = _cdd_json_buf_append_escaped(&_b, obj->%s[_i]);\n"
        "      if (_rc != CDD_C_SUCCESS) { free(_b.data); return _rc; }\n",
        f->name));
  } else if (f->is_enum_array) {
    const char *tn = NULL;
    const char *er = f->enum_array_ref[0] ? f->enum_array_ref : elem;
    cdd_c_error_t rc_ref = get_type_from_ref(er, &tn);
    if (rc_ref != CDD_C_SUCCESS)
      return rc_ref;
    CHECK_IO(FPRINTF_HOOK(
        fp,
        "      { char *_es = NULL;\n"
        "        _rc = %s_to_str(obj->%s[_i], &_es);\n"
        "        if (_rc != CDD_C_SUCCESS) { free(_b.data); return _rc; }\n"
        "        _rc = _cdd_json_buf_append_escaped(&_b, _es);\n"
        "        free(_es);\n"
        "        if (_rc != CDD_C_SUCCESS) { free(_b.data); return _rc; }\n"
        "      }\n",
        tn, f->name));
  } else {
    const char *tn = NULL;
    cdd_c_error_t rc_ref = get_type_from_ref(elem, &tn);
    if (rc_ref != CDD_C_SUCCESS)
      return rc_ref;
    CHECK_IO(FPRINTF_HOOK(
        fp,
        "      if (obj->%s[_i]) {\n"
        "        char *_sub = NULL;\n"
        "        _rc = %s_to_json_standalone(obj->%s[_i], &_sub);\n"
        "        if (_rc != CDD_C_SUCCESS) { free(_b.data); return _rc; }\n"
        "        _rc = _cdd_json_buf_append(&_b, _sub, strlen(_sub));\n"
        "        free(_sub);\n"
        "        if (_rc != CDD_C_SUCCESS) { free(_b.data); return _rc; }\n"
        "      } else {\n"
        "        _rc = _cdd_json_buf_append(&_b, \"null\", 4);\n"
        "        if (_rc != CDD_C_SUCCESS) { free(_b.data); return _rc; }\n"
        "      }\n",
        f->name, tn, f->name));
  }
  CHECK_IO(FPRINTF_HOOK(
      fp, "    }\n"
          "    _rc = _cdd_json_buf_append(&_b, \"]\", 1);\n"
          "    if (_rc != CDD_C_SUCCESS) { free(_b.data); return _rc; }\n"
          "  }\n"));
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits a standalone zero-dependency JSON parser function for a struct.
 *
 * @param[in] fp Output stream.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Fields descriptor.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t
write_struct_from_json_standalone_func(FILE *fp, const char *struct_name,
                                       const struct StructFields *sf) {
  size_t i;
  int has_string = 0;
  int has_primitive = 0;
  cdd_c_error_t rc_h;

  if (!fp || !struct_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < sf->size; ++i) {
    if (sf->fields[i].read_only || sf->fields[i].serde_skip ||
        sf->fields[i].serde_skip_deserializing)
      continue;
    if (strcmp(sf->fields[i].type, "string") == 0 ||
        strcmp(sf->fields[i].type, "enum") == 0) {
      has_string = 1;
    } else if (strcmp(sf->fields[i].type, "integer") == 0 ||
               strcmp(sf->fields[i].type, "number") == 0 ||
               strcmp(sf->fields[i].type, "boolean") == 0 ||
               strcmp(sf->fields[i].type, "object") == 0 ||
               strcmp(sf->fields[i].type, "array") == 0) {
      has_primitive = 1;
    }
  }

  rc_h = write_standalone_json_helpers(fp);
  if (rc_h != CDD_C_SUCCESS)
    return rc_h;

  CHECK_IO(FPRINTF_HOOK(
      fp,
      "/**\n"
      " * @brief Autogenerated zero-dependency JSON parser for %s.\n"
      " * @param[in] json JSON string to parse.\n"
      " * @param[out] out Pointer to store the parsed struct.\n"
      " * @return CDD_C_SUCCESS on success, non-zero on error.\n"
      " */\n"
      "cdd_c_error_t %s_parse_json(const char *json, struct %s **const out) "
      "{\n",
      struct_name, struct_name, struct_name));
  fprintf(fp, "  struct %s *ret;\n", struct_name);
  fprintf(fp, "  const char *p = json;\n");
  fprintf(fp, "  char *key = NULL;\n");
  fprintf(fp, "  char *val = NULL;\n\n");

  for (i = 0; i < sf->size; ++i) {
    if (sf->fields[i].read_only || sf->fields[i].serde_skip ||
        sf->fields[i].serde_skip_deserializing)
      continue;
    if (sf->fields[i].required) {
      fprintf(fp, "  int has_%s = 0;\n", sf->fields[i].name);
    }
  }

  fprintf(fp, "  if (!json || !out) return CDD_C_ERROR_INVALID_ARGUMENT;\n");
  fprintf(fp, "  ret = (struct %s *)calloc(1, sizeof(struct %s));\n",
          struct_name, struct_name);
  fprintf(fp, "  if (!ret) return CDD_C_ERROR_MEMORY;\n\n");

  for (i = 0; i < sf->size; ++i) {
    const struct StructField *f = &sf->fields[i];
    if (f->default_val[0] == 0)
      continue;
    if (strcmp(f->type, "integer") == 0) {
      fprintf(fp, "  ret->%s = %s;\n", f->name, f->default_val);
    } else if (strcmp(f->type, "number") == 0) {
      fprintf(fp, "  ret->%s = %s;\n", f->name, f->default_val);
    } else if (strcmp(f->type, "boolean") == 0) {
      fprintf(fp, "  ret->%s = %d;\n", f->name,
              (strcmp(f->default_val, "true") == 0 ||
               strcmp(f->default_val, "1") == 0)
                  ? 1
                  : 0);
    } else if (strcmp(f->type, "string") == 0 && f->fixed_array_len == 0) {
      char esc_def[128];
      rc_h = cdd_escape_json_c_str(f->default_val, esc_def, sizeof(esc_def));
      fprintf(fp,
              "  ret->%s = strdup(\"%s\");\n"
              "  if (!ret->%s) goto err;\n",
              f->name, esc_def, f->name);
    }
  }

  fprintf(fp, "  if (_cdd_json_skip_ws(&p) != CDD_C_SUCCESS) goto err;\n");
  fprintf(fp, "  if (*p != '{') goto err;\n");
  fprintf(fp, "  p++;\n");
  fprintf(fp, "  while (*p) {\n");
  fprintf(fp, "    if (_cdd_json_skip_ws(&p) != CDD_C_SUCCESS) goto err;\n");
  fprintf(fp, "    if (!*p || *p == '}') { if (*p == '}') p++; break; }\n");
  fprintf(fp, "    if (*p == ',') { p++; if (_cdd_json_skip_ws(&p) != "
              "CDD_C_SUCCESS) goto err; }\n");
  fprintf(fp, "    if (*p != '\"') break;\n");
  fprintf(
      fp,
      "    if (_cdd_json_unescape_str(&p, &key) != CDD_C_SUCCESS) goto err;\n");
  fprintf(fp, "    if (_cdd_json_skip_ws(&p) != CDD_C_SUCCESS) { free(key); "
              "key = NULL; goto err; }\n");
  fprintf(fp, "    if (*p != ':') { free(key); key = NULL; goto err; }\n");
  fprintf(fp, "    p++;\n");
  fprintf(fp, "    if (_cdd_json_skip_ws(&p) != CDD_C_SUCCESS) { free(key); "
              "key = NULL; goto err; }\n\n");

  /* Parse String value */
  fprintf(fp, "    if (*p == '\"') {\n");
  fprintf(fp, "      if (_cdd_json_unescape_str(&p, &val) != CDD_C_SUCCESS) { "
              "free(key); key = NULL; goto err; }\n");

  if (has_string) {
    int string_idx = 0;
    for (i = 0; i < sf->size; ++i) {
      const struct StructField *f = &sf->fields[i];
      char kn[128];
      char esc_kn[128];
      if (f->read_only || f->serde_skip || f->serde_skip_deserializing)
        continue;
      if (strcmp(f->type, "string") == 0 || strcmp(f->type, "enum") == 0) {
        size_t a;
        cdd_c_error_t rc =
            cdd_serde_resolve_field_names(sf, i, NULL, 0, kn, sizeof(kn));
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc_h = cdd_escape_json_c_str(kn, esc_kn, sizeof(esc_kn));
        fprintf(fp, "      %s (strcmp(key, \"%s\") == 0",
                (string_idx == 0) ? "if" : "else if", esc_kn);
        for (a = 0; a < f->n_serde_aliases; ++a) {
          if (f->serde_aliases[a]) {
            char esc_al[128];
            rc_h = cdd_escape_json_c_str(f->serde_aliases[a], esc_al,
                                         sizeof(esc_al));
            fprintf(fp, " || strcmp(key, \"%s\") == 0", esc_al);
          }
        }
        fprintf(fp, ") {\n");
        if (f->has_max_len) {
          fprintf(fp, "        if (strlen(val) > %lu) goto err;\n",
                  (unsigned long)f->max_len);
        }
        if (f->has_min_len) {
          fprintf(fp, "        if (strlen(val) < %lu) goto err;\n",
                  (unsigned long)f->min_len);
        }
        if (strcmp(f->type, "enum") == 0) {
          const char *tn = NULL;
          cdd_c_error_t rc_ref = get_type_from_ref(f->ref, &tn);
          if (rc_ref != CDD_C_SUCCESS)
            return rc_ref;
          fprintf(fp,
                  "        if (%s_from_str(val, &ret->%s) != CDD_C_SUCCESS) "
                  "goto err;\n"
                  "        free(val); val = NULL;\n",
                  tn, f->name);
        } else if (f->fixed_array_len > 0) {
          fprintf(fp,
                  "        strncpy(ret->%s, val, %lu - 1);\n"
                  "        ret->%s[%lu - 1] = '\\0';\n"
                  "        free(val); val = NULL;\n",
                  f->name, (unsigned long)f->fixed_array_len, f->name,
                  (unsigned long)f->fixed_array_len);
        } else {
          fprintf(fp,
                  "        free(ret->%s);\n"
                  "        ret->%s = val;\n"
                  "        val = NULL;\n",
                  f->name, f->name);
        }
        if (f->required) {
          fprintf(fp, "        has_%s = 1;\n", f->name);
        }
        fprintf(fp, "      }\n");
        string_idx++;
      }
    }
  }
  fprintf(fp, "      free(val); val = NULL;\n");

  /* Parse Number / Boolean / Object / Array / Null */
  fprintf(fp, "    } else if (strncmp(p, \"null\", 4) == 0) {\n"
              "      p += 4;\n"
              "    } else {\n");
  if (has_primitive) {
    int primitive_idx = 0;
    for (i = 0; i < sf->size; ++i) {
      const struct StructField *f = &sf->fields[i];
      char kn[128];
      char esc_kn[128];
      size_t a;
      if (f->read_only || f->serde_skip || f->serde_skip_deserializing)
        continue;
      if (strcmp(f->type, "integer") == 0 || strcmp(f->type, "number") == 0 ||
          strcmp(f->type, "boolean") == 0 || strcmp(f->type, "object") == 0 ||
          strcmp(f->type, "array") == 0) {
        cdd_c_error_t rc =
            cdd_serde_resolve_field_names(sf, i, NULL, 0, kn, sizeof(kn));
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc_h = cdd_escape_json_c_str(kn, esc_kn, sizeof(esc_kn));
        fprintf(fp, "      %s (strcmp(key, \"%s\") == 0",
                (primitive_idx == 0) ? "if" : "else if", esc_kn);
        for (a = 0; a < f->n_serde_aliases; ++a) {
          if (f->serde_aliases[a]) {
            char esc_al[128];
            rc_h = cdd_escape_json_c_str(f->serde_aliases[a], esc_al,
                                         sizeof(esc_al));
            fprintf(fp, " || strcmp(key, \"%s\") == 0", esc_al);
          }
        }
        fprintf(fp, ") {\n");
        if (strcmp(f->type, "integer") == 0) {
          fprintf(fp, "        ret->%s = (int)strtol(p, (char **)&p, 10);\n",
                  f->name);
          if (f->has_min) {
            fprintf(fp, "        if ((double)ret->%s %s %g) goto err;\n",
                    f->name, f->exclusive_min ? "<=" : "<", f->min_val);
          }
          if (f->has_max) {
            fprintf(fp, "        if ((double)ret->%s %s %g) goto err;\n",
                    f->name, f->exclusive_max ? ">=" : ">", f->max_val);
          }
        } else if (strcmp(f->type, "number") == 0) {
          fprintf(fp, "        ret->%s = strtod(p, (char **)&p);\n", f->name);
          if (f->has_min) {
            fprintf(fp, "        if (ret->%s %s %g) goto err;\n", f->name,
                    f->exclusive_min ? "<=" : "<", f->min_val);
          }
          if (f->has_max) {
            fprintf(fp, "        if (ret->%s %s %g) goto err;\n", f->name,
                    f->exclusive_max ? ">=" : ">", f->max_val);
          }
        } else if (strcmp(f->type, "boolean") == 0) {
          fprintf(fp,
                  "        ret->%s = (strncmp(p, \"true\", 4) == 0) ? 1 : 0;\n"
                  "        if (_cdd_json_skip_value(&p) != CDD_C_SUCCESS) goto "
                  "err;\n",
                  f->name);
        } else if (strcmp(f->type, "object") == 0) {
          const char *tn = NULL;
          cdd_c_error_t rc_ref = get_type_from_ref(f->ref, &tn);
          if (rc_ref != CDD_C_SUCCESS)
            return rc_ref;
          fprintf(fp,
                  "        if (%s_parse_json(p, &ret->%s) != CDD_C_SUCCESS) "
                  "goto err;\n"
                  "        if (_cdd_json_skip_value(&p) != CDD_C_SUCCESS) goto "
                  "err;\n",
                  tn, f->name);
        } else {
          rc = write_standalone_array_from_json(fp, f);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
        if (f->required) {
          fprintf(fp, "        has_%s = 1;\n", f->name);
        }
        fprintf(fp, "      }\n");
        primitive_idx++;
      }
    }
    fprintf(fp, "      else { if (_cdd_json_skip_value(&p) != CDD_C_SUCCESS) "
                "goto err; }\n");
  } else {
    fprintf(fp,
            "      if (_cdd_json_skip_value(&p) != CDD_C_SUCCESS) goto err;\n");
  }

  fprintf(fp, "    }\n");
  fprintf(fp, "    free(key); key = NULL;\n");
  fprintf(fp, "  }\n\n");

  for (i = 0; i < sf->size; ++i) {
    if (sf->fields[i].read_only || sf->fields[i].serde_skip ||
        sf->fields[i].serde_skip_deserializing)
      continue;
    if (sf->fields[i].required) {
      fprintf(fp, "  if (!has_%s) goto err;\n", sf->fields[i].name);
    }
  }

  fprintf(fp, "  *out = ret;\n");
  fprintf(fp, "  return CDD_C_SUCCESS;\n");

  fprintf(fp, "\nerr:\n");
  fprintf(fp, "  free(key);\n");
  fprintf(fp, "  free(val);\n");
  fprintf(fp, "  %s_cleanup(ret);\n", struct_name);
  fprintf(fp, "  return CDD_C_ERROR_UNKNOWN;\n");
  fprintf(fp, "}\n");
  return rc_h;
}

/**
 * @brief Emits a standalone zero-dependency JSON serializer function for a
 * struct.
 *
 * @param[in] fp Output stream.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Fields descriptor.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t
write_struct_to_json_standalone_func(FILE *fp, const char *struct_name,
                                     const struct StructFields *sf) {
  size_t i;
  cdd_c_error_t rc_h;

  if (!fp || !struct_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc_h = write_standalone_json_helpers(fp);
  if (rc_h != CDD_C_SUCCESS)
    return rc_h;

  CHECK_IO(FPRINTF_HOOK(
      fp,
      "/**\n"
      " * @brief Autogenerated zero-dependency JSON emitter for %s.\n"
      " * @param[in] obj Struct instance to serialize.\n"
      " * @param[out] out_json Heap-allocated JSON string.\n"
      " * @return CDD_C_SUCCESS on success, or error code on failure.\n"
      " */\n"
      "cdd_c_error_t %s_to_json_standalone(const struct %s *obj, char "
      "**out_json) {\n",
      struct_name, struct_name, struct_name));
  CHECK_IO(FPRINTF_HOOK(
      fp, "  struct _cdd_json_buf _b;\n"
          "  int _first = 1;\n"
          "  cdd_c_error_t _rc;\n"
          "  if (!obj || !out_json) return CDD_C_ERROR_INVALID_ARGUMENT;\n"
          "  _b.data = NULL; _b.len = 0; _b.cap = 0;\n"
          "  _rc = _cdd_json_buf_append(&_b, \"{\", 1);\n"
          "  if (_rc != CDD_C_SUCCESS) return _rc;\n"));

  for (i = 0; i < sf->size; ++i) {
    const struct StructField *f = &sf->fields[i];
    char kn[128];
    char esc_kn[128];
    cdd_c_error_t rc;
    if (f->write_only || f->serde_skip || f->serde_skip_serializing)
      continue;
    rc = cdd_serde_resolve_field_names(sf, i, kn, sizeof(kn), NULL, 0);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc_h = cdd_escape_json_c_str(kn, esc_kn, sizeof(esc_kn));

    CHECK_IO(FPRINTF_HOOK(
        fp,
        "  if (!_first) { _rc = _cdd_json_buf_append(&_b, \",\", 1); if (_rc "
        "!= CDD_C_SUCCESS) { free(_b.data); return _rc; } }\n"
        "  _rc = _cdd_json_buf_append(&_b, \"\\\"%s\\\":\", %lu);\n"
        "  if (_rc != CDD_C_SUCCESS) { free(_b.data); return _rc; }\n"
        "  _first = 0;\n",
        esc_kn, (unsigned long)(strlen(esc_kn) + 3)));

    if (strcmp(f->type, "string") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "  _rc = _cdd_json_buf_append_escaped(&_b, obj->%s);\n"
          "  if (_rc != CDD_C_SUCCESS) { free(_b.data); return _rc; }\n",
          f->name));
    } else if (strcmp(f->type, "integer") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "  { char _nb[32];\n"
          "#if defined(_MSC_VER)\n"
          "    sprintf_s(_nb, sizeof(_nb), \"%%d\", obj->%s);\n"
          "#else\n"
          "    sprintf(_nb, \"%%d\", obj->%s);\n"
          "#endif\n"
          "    _rc = _cdd_json_buf_append(&_b, _nb, strlen(_nb));\n"
          "    if (_rc != CDD_C_SUCCESS) { free(_b.data); return _rc; }\n"
          "  }\n",
          f->name, f->name));
    } else if (strcmp(f->type, "number") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "  { char _nb[64];\n"
          "#if defined(_MSC_VER)\n"
          "    sprintf_s(_nb, sizeof(_nb), \"%%.17g\", obj->%s);\n"
          "#else\n"
          "    sprintf(_nb, \"%%.17g\", obj->%s);\n"
          "#endif\n"
          "    _rc = _cdd_json_buf_append(&_b, _nb, strlen(_nb));\n"
          "    if (_rc != CDD_C_SUCCESS) { free(_b.data); return _rc; }\n"
          "  }\n",
          f->name, f->name));
    } else if (strcmp(f->type, "boolean") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "  _rc = _cdd_json_buf_append(&_b, obj->%s ? \"true\" : \"false\", "
          "obj->%s ? 4 : 5);\n"
          "  if (_rc != CDD_C_SUCCESS) { free(_b.data); return _rc; }\n",
          f->name, f->name));
    } else if (strcmp(f->type, "enum") == 0) {
      const char *tn = NULL;
      rc = get_type_from_ref(f->ref, &tn);
      if (rc != CDD_C_SUCCESS)
        return rc;
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "  { char *_es = NULL;\n"
          "    _rc = %s_to_str(obj->%s, &_es);\n"
          "    if (_rc != CDD_C_SUCCESS) { free(_b.data); return _rc; }\n"
          "    _rc = _cdd_json_buf_append_escaped(&_b, _es);\n"
          "    free(_es);\n"
          "    if (_rc != CDD_C_SUCCESS) { free(_b.data); return _rc; }\n"
          "  }\n",
          tn, f->name));
    } else if (strcmp(f->type, "object") == 0) {
      const char *tn = NULL;
      rc = get_type_from_ref(f->ref, &tn);
      if (rc != CDD_C_SUCCESS)
        return rc;
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "  if (obj->%s) {\n"
          "    char *_sub = NULL;\n"
          "    _rc = %s_to_json_standalone(obj->%s, &_sub);\n"
          "    if (_rc != CDD_C_SUCCESS) { free(_b.data); return _rc; }\n"
          "    _rc = _cdd_json_buf_append(&_b, _sub, strlen(_sub));\n"
          "    free(_sub);\n"
          "    if (_rc != CDD_C_SUCCESS) { free(_b.data); return _rc; }\n"
          "  } else {\n"
          "    _rc = _cdd_json_buf_append(&_b, \"null\", 4);\n"
          "    if (_rc != CDD_C_SUCCESS) { free(_b.data); return _rc; }\n"
          "  }\n",
          f->name, tn, f->name));
    } else {
      rc = write_standalone_array_to_json(fp, f);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  CHECK_IO(FPRINTF_HOOK(
      fp, "  _rc = _cdd_json_buf_append(&_b, \"}\", 1);\n"
          "  if (_rc != CDD_C_SUCCESS) { free(_b.data); return _rc; }\n"
          "  *out_json = _b.data;\n"
          "  return CDD_C_SUCCESS;\n"
          "}\n"));
  return rc_h;
}
