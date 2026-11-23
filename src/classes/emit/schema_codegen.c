/*  */
/**
 * @file schema_codegen.c
 * @brief Implementation of C code generation from JSON Schema.
 *
 * Implements a multi-pass approach to support forward declarations and
 * delegates struct/enum generation to specialized modules.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "c_cdd/code_writer.h"
#include "c_cdd/memory.h"
#include "classes/emit/schema_codegen.h"
#include "classes/emit/enum.h"
#include "classes/emit/form.h"
#include "classes/emit/json.h"
#include "classes/emit/jwt.h"
#include "classes/emit/oauth2_error.h"
#include "classes/emit/struct.h"
#include "classes/emit/types.h"
#include "classes/parse/code2schema.h"
#include "functions/emit/codegen.h" /* Facade header */
#include "functions/parse/fs.h"
#include "functions/parse/str.h"
#include "c_cdd/json_dom.h"
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#ifndef strdup
#define strdup _strdup
#endif
#endif
#define PATH_MAX _MAX_PATH
#else
#endif

#ifndef PATH_MAX
/** @brief PATH_MAX definition */
#define PATH_MAX 4096
#endif

/** @brief CHECK_IO definition */
#define CHECK_IO(x)                                                            \
  for (; (x) < 0;)                                                             \
  return CDD_C_ERROR_IO
/** @brief CHECK_RC definition */
#define CHECK_RC(x)                                                            \
  for (;;) {                                                                   \
    cdd_c_error_t err = (x);                                                   \
    if (err != 0)                                                              \
      return err;                                                              \
    break;                                                                     \
  }

/** @brief Check I/O result and close file on error */
#define F_CHECK_IO(x)                                                          \
  for (; (x) < 0;) {                                                           \
    fclose(fp);                                                                \
    return CDD_C_ERROR_IO;                                                     \
  }

/** @brief Check return code and close file on error */
#define F_CHECK_RC_TESTABLE(x)                                                 \
  for (;;) {                                                                   \
    cdd_c_error_t err = (x);                                                   \
    if (err != 0) {                                                            \
      fclose(fp);                                                              \
      return err;                                                              \
    }                                                                          \
    break;                                                                     \
  }
/** @brief Check return code, free struct fields, and close file on error */
#define F_CHECK_RC_TESTABLE_SF(x)                                              \
  for (;;) {                                                                   \
    cdd_c_error_t err = (x);                                                   \
    if (err != 0) {                                                            \
      err = (cdd_c_error_t)((int)err | (int)struct_fields_free(&sf));          \
      fclose(fp);                                                              \
      return err;                                                              \
    }                                                                          \
    break;                                                                     \
  }

/* Write Header Guard Start */
/**
 * @brief Emits the opening `#ifndef` / `#define` header include guard.
 *
 * @param[in] hfile Output header file stream.
 * @param[in] basename Base name used to construct the guard macro.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO on write failure.
 */
static cdd_c_error_t print_header_guard(FILE *hfile, const char *basename) {
  CHECK_IO(FPRINTF_HOOK(hfile, "#ifndef %s_H\n", basename));
  CHECK_IO(FPRINTF_HOOK(hfile, "#define %s_H\n\n", basename));
  return CDD_C_SUCCESS;
}

/* Write Header Guard End */
/**
 * @brief Emits the closing `#endif` header include guard.
 *
 * @param[in] hfile Output header file stream.
 * @param[in] basename Base name used to construct the guard macro.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO on write failure.
 */
static cdd_c_error_t print_header_guard_end(FILE *hfile, const char *basename) {
  CHECK_IO(FPRINTF_HOOK(hfile, "#endif /* !%s_H */\n", basename));
  return CDD_C_SUCCESS;
}

/**
 * @brief Resolves the C identifier name for a JSON Schema definition.
 *
 * @param[in] raw_name Raw schema key in the JSON Schema document.
 * @param[in] s Schema JSON_Object (inspected for `x-cdd-c-name`).
 * @param[out] out_name Buffer (capacity >= 64) receiving the resolved C
 * identifier.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t resolve_schema_c_name(const char *raw_name,
                                           const JSON_Object *s,
                                           char *out_name) {
  const char *c_name_ext = NULL;
  char *san_name = NULL;
  int needs_san = 0;
  cdd_c_error_t rc;
  cdd_c_error_t rc_opt;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif

  rc_opt = cdd_json_object_get_string(s, "x-cdd-c-name", &c_name_ext);
  if (c_name_ext && c_name_ext[0] != '\0') {
    CDD_STRCPY(out_name, 64, c_name_ext);
    return (cdd_c_error_t)(CDD_C_SUCCESS + (int)rc_opt * 0);
  }
  rc = c2s_needs_identifier_sanitization(raw_name, &needs_san);
  if (needs_san) {
    cdd_c_error_t rc_san = sanitize_identifier(raw_name, &san_name);
    if (rc_san != CDD_C_SUCCESS)
      return rc_san;
    CDD_STRCPY(out_name, 64, san_name);
    C_CDD_FREE(san_name);
    return rc;
  }

  CDD_STRCPY(out_name, 64, raw_name);
  return CDD_C_SUCCESS;
}

/**
 * @brief Extracts a schema entry at index `idx` from `schemas_obj` and checks
 * whether it describes an object/struct schema (`"type": "object"` or has
 * `"properties"`).
 *
 * @param[in] schemas_obj Schemas JSON object map.
 * @param[in] idx Zero-based schema index.
 * @param[out] out_name Schema key name.
 * @param[out] out_schema Schema JSON object (or NULL if entry is not an
 * object).
 * @param[out] out_is_obj Set to 1 if `out_schema` represents a struct/object
 * schema, 0 otherwise.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t schema_entry_at(const JSON_Object *schemas_obj, size_t idx,
                                     const char **out_name,
                                     JSON_Object **out_schema,
                                     int *out_is_obj) {
  JSON_Value *val_at = schemas_obj->values[idx];
  JSON_Object *s = (val_at->type == JSONObject) ? val_at->object_val : NULL;
  const char *type = NULL;
  JSON_Object *props = NULL;
  cdd_c_error_t rc_t;
  cdd_c_error_t rc_p;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_name = schemas_obj->names[idx];
  *out_schema = s;
  rc_t = cdd_json_object_get_string(s, "type", &type);
  if (rc_t == CDD_C_SUCCESS && strcmp(type, "object") == 0) {
    *out_is_obj = 1;
    return rc_t;
  }
  rc_p = cdd_json_object_get_object(s, "properties", &props);
  if (rc_p == CDD_C_SUCCESS) {
    *out_is_obj = 1;
    return rc_p;
  }
  *out_is_obj = 0;
  return CDD_C_SUCCESS;
}
/**
 * @brief Generates a C header file (`<prefix>.h`) for the given JSON Schemas.
 *
 * @param[in] prefix Output path prefix (without `.h` extension).
 * @param[in] basename Base filename used for include guards.
 * @param[in] schemas_obj Parson `JSON_Object` mapping schema names to schemas.
 * @param[in] config Optional code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t generate_header(const char *prefix,
                                           const char *basename,
                                           JSON_Object *schemas_obj,
                                           const struct CodegenConfig *config) {
  char fname[256];
  FILE *fp;
  size_t i, count = 0;
  cdd_c_error_t rc_fn = CDD_C_SUCCESS;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  sprintf_s(fname, sizeof(fname), "%s.h", prefix);
#else
  CDD_SNPRINTF(fname, sizeof(fname), "%s.h", prefix);
#endif
#if defined(_MSC_VER)
  if (fopen_s(&fp, fname, "w") != 0)
    fp = NULL;
#else
#if defined(_MSC_VER)
  fopen_s(&fp, fname, "w");
#else
#if defined(_MSC_VER)
  if (fopen_s(&fp, fname, "w") != 0)
    fp = NULL;
#else
  fp = fopen(fname, "w");
#endif
#endif
#endif
  if (!fp)
    return CDD_C_ERROR_SYSTEM;

  F_CHECK_RC_TESTABLE(print_header_guard(fp, basename));
  F_CHECK_IO(FPRINTF_HOOK(fp, "#include <stdlib.h>\n#include <cdd_c_error.h>\n"
                              "#include \"lib_export.h\"\n\n"
                              "#if defined(_MSC_VER) && _MSC_VER < 1600\n"
                              "typedef signed __int8 int8_t;\n"
                              "typedef unsigned __int8 uint8_t;\n"
                              "typedef signed __int16 int16_t;\n"
                              "typedef unsigned __int16 uint16_t;\n"));
  F_CHECK_IO(FPRINTF_HOOK(fp, "typedef signed __int32 int32_t;\n"
                              "typedef unsigned __int32 uint32_t;\n"
                              "typedef signed __int64 int64_t;\n"
                              "typedef unsigned __int64 uint64_t;\n"
                              "#else\n"
                              "#if defined(_MSC_VER) && _MSC_VER < 1600\n"
                              "#include \"c_cdd/msvc/stdint.h\"\n"
                              "#else\n"
                              "#include <stdint.h>\n"
                              "#endif\n"
                              "#endif\n\n"));
  F_CHECK_IO(FPRINTF_HOOK(fp,
                          "#if defined(_MSC_VER) && _MSC_VER < 1800\n"
                          "#if !defined(__cplusplus)\n"
                          "#ifndef bool\n"
                          "#define bool unsigned char\n"
                          "#endif\n"
                          "#ifndef true\n"
                          "#define true 1\n"
                          "#endif\n"
                          "#ifndef false\n"
                          "#define false 0\n"
                          "#endif\n"
                          "#endif\n"
                          "#else\n"
                          "#if defined(_MSC_VER) && _MSC_VER < 1800\n"
                          "#include \"c_cdd/msvc/stdbool.h\"\n"
                          "#else\n"
                          "#include <stdbool.h>\n"
                          "#endif\n"
                          "#endif\n\n"
                          "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n"));

  /* Pass 1: Forward Decls */
  F_CHECK_RC_TESTABLE(cdd_json_object_get_count(schemas_obj, &count));
  for (i = 0; i < count; i++) {
    const char *raw_name = NULL;
    JSON_Object *s = NULL;
    struct StructFields sf;
    int is_obj_hint = 0;
    int is_object_schema = 0;
    char c_name[64];
    cdd_c_error_t rc_step;

    F_CHECK_RC_TESTABLE(
        schema_entry_at(schemas_obj, i, &raw_name, &s, &is_obj_hint));

    rc_step = resolve_schema_c_name(raw_name, s, c_name);
    if (rc_step != CDD_C_SUCCESS) {
      fclose(fp);
      return cdd_c_keep_err(CDD_C_ERROR_MEMORY, rc_step);
    }

    rc_step = struct_fields_init(&sf);
    if (rc_step != CDD_C_SUCCESS) {
      fclose(fp);
      return cdd_c_keep_err(CDD_C_ERROR_MEMORY, rc_step);
    }
    rc_step =
        json_object_to_struct_fields_ex_codegen(s, &sf, schemas_obj, raw_name);
    if (rc_step != CDD_C_SUCCESS) {
      fclose(fp);
      return cdd_c_keep_err(CDD_C_ERROR_MEMORY,
                            cdd_c_keep_err(rc_step, struct_fields_free(&sf)));
    }

    is_object_schema = is_obj_hint || sf.size > 0;

    if (sf.is_union || is_object_schema) {
      F_CHECK_RC_TESTABLE_SF(write_forward_decl(fp, c_name));
    }
    rc_fn = struct_fields_free(&sf);
  }
  fprintf(fp, "\n");

  /* Pass 2: Definitions */
  count = schemas_obj->count;
  for (i = 0; i < count; i++) {
    const char *raw_name = NULL;
    JSON_Object *s = NULL;
    struct StructFields sf;
    int is_obj_hint = 0;
    int is_object_schema = 0;
    char c_name[64];
    cdd_c_error_t rc_step;

    F_CHECK_RC_TESTABLE(
        schema_entry_at(schemas_obj, i, &raw_name, &s, &is_obj_hint));

    rc_step = resolve_schema_c_name(raw_name, s, c_name);
    if (rc_step != CDD_C_SUCCESS) {
      fclose(fp);
      return cdd_c_keep_err(CDD_C_ERROR_MEMORY, rc_step);
    }

    rc_step = struct_fields_init(&sf);
    if (rc_step != CDD_C_SUCCESS) {
      fclose(fp);
      return cdd_c_keep_err(CDD_C_ERROR_MEMORY, rc_step);
    }
    rc_step =
        json_object_to_struct_fields_ex_codegen(s, &sf, schemas_obj, raw_name);
    if (rc_step != CDD_C_SUCCESS) {
      fclose(fp);
      return cdd_c_keep_err(CDD_C_ERROR_MEMORY,
                            cdd_c_keep_err(rc_step, struct_fields_free(&sf)));
    }

    if (strcmp(c_name, raw_name) != 0 && sf.serde_rename[0] == '\0') {
      CDD_STRCPY(sf.serde_rename, sizeof(sf.serde_rename), raw_name);
    }

    is_object_schema = is_obj_hint || sf.size > 0;

    if (sf.is_enum) {
      F_CHECK_RC_TESTABLE_SF(write_enum_declaration_h(fp, c_name, &sf, config));
    } else if (sf.is_union) {
      F_CHECK_RC_TESTABLE_SF(
          write_union_declaration_h(fp, c_name, &sf, config));
    } else if (is_object_schema) {
      F_CHECK_RC_TESTABLE_SF(
          write_struct_declaration_h(fp, c_name, &sf, config));
    }

    rc_fn = (cdd_c_error_t)((int)rc_fn | (int)struct_fields_free(&sf));
  }

  F_CHECK_IO(FPRINTF_HOOK(fp, "#ifdef __cplusplus\n}\n#endif\n"));
  F_CHECK_RC_TESTABLE(print_header_guard_end(fp, basename));
  fclose(fp);
  return rc_fn;
}

/**
 * @brief Generates a C source file (`<prefix>.c`) for the given JSON Schemas.
 *
 * @param[in] prefix Output path prefix (without `.c` extension).
 * @param[in] basename Base filename used to `#include "<basename>.h"`.
 * @param[in] schemas_obj Parson `JSON_Object` mapping schema names to schemas.
 * @param[in] config Optional code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t generate_source(const char *prefix,
                                           const char *basename,
                                           JSON_Object *schemas_obj,
                                           const struct CodegenConfig *config) {
  char fname[256];
  FILE *fp;
  size_t i;
  struct CodegenJsonConfig json_cfg;
  struct CodegenStructConfig struct_cfg;
  struct CodegenTypesConfig types_cfg;
  struct CodegenEnumConfig enum_cfg;

  if (config) {
    json_cfg.guard_macro = config->json_guard;
    struct_cfg.guard_macro = config->utils_guard;
    types_cfg.json_guard = config->json_guard;
    types_cfg.utils_guard = config->utils_guard;
    enum_cfg.guard_macro = config->enum_guard;
  } else {
    json_cfg.guard_macro = NULL;
    struct_cfg.guard_macro = NULL;
    types_cfg.json_guard = NULL;
    types_cfg.utils_guard = NULL;
    enum_cfg.guard_macro = NULL;
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  sprintf_s(fname, sizeof(fname), "%s.c", prefix);
#else
  CDD_SNPRINTF(fname, sizeof(fname), "%s.c", prefix);
#endif
#if defined(_MSC_VER)
  if (fopen_s(&fp, fname, "w") != 0)
    fp = NULL;
#else
#if defined(_MSC_VER)
  fopen_s(&fp, fname, "w");
#else
#if defined(_MSC_VER)
  if (fopen_s(&fp, fname, "w") != 0)
    fp = NULL;
#else
  fp = fopen(fname, "w");
#endif
#endif
#endif
  if (!fp)
    return CDD_C_ERROR_SYSTEM;

  F_CHECK_IO(
      FPRINTF_HOOK(fp,
                   "#include <errno.h>\n#include <stdio.h>\n#include "
                   "<stdlib.h>\n#include "
                   "<string.h>\n#include <parson.h>\n#include "
                   "<c89stringutils_string_extras.h>\n#include \"%s.h\"\n\n",
                   basename));
  F_CHECK_IO(FPRINTF_HOOK(fp,
                          "#ifndef CDD_JSON_DOM_H\n"
                          "#define cdd_json_parse_string(str, out) (*(out) = "
                          "json_parse_string(str), (*(out) ? CDD_C_SUCCESS : "
                          "CDD_C_ERROR_INVALID_ARGUMENT))\n"
                          "#define cdd_json_value_get_object(val, out) (*(out) "
                          "= json_value_get_object(val), CDD_C_SUCCESS)\n"
                          "#define cdd_json_value_get_array(val, out) (*(out) "
                          "= json_value_get_array(val), CDD_C_SUCCESS)\n"
                          "#define cdd_json_value_free(val) "
                          "(json_value_free(val), CDD_C_SUCCESS)\n"));
  F_CHECK_IO(FPRINTF_HOOK(
      fp, "#define cdd_json_object_get_count(obj, out) (*(out) = "
          "json_object_get_count(obj), CDD_C_SUCCESS)\n"
          "#define cdd_json_object_get_name(obj, i, out) (*(out) = "
          "json_object_get_name(obj, i), CDD_C_SUCCESS)\n"
          "#define cdd_json_object_has_value(obj, k, out) (*(out) = "
          "json_object_has_value(obj, k), CDD_C_SUCCESS)\n"
          "#define cdd_json_object_get_string(obj, k, out) (*(out) = "
          "json_object_get_string(obj, k), CDD_C_SUCCESS)\n"));
  F_CHECK_IO(FPRINTF_HOOK(
      fp, "#define cdd_json_object_get_number(obj, k, out) (*(out) = "
          "json_object_get_number(obj, k), CDD_C_SUCCESS)\n"
          "#define cdd_json_object_get_boolean(obj, k, out) (*(out) = "
          "json_object_get_boolean(obj, k), CDD_C_SUCCESS)\n"
          "#define cdd_json_object_get_object(obj, k, out) (*(out) = "
          "json_object_get_object(obj, k), CDD_C_SUCCESS)\n"
          "#define cdd_json_object_get_array(obj, k, out) (*(out) = "
          "json_object_get_array(obj, k), CDD_C_SUCCESS)\n"));
  F_CHECK_IO(
      FPRINTF_HOOK(fp, "#define cdd_json_array_get_count(arr, out) (*(out) = "
                       "json_array_get_count(arr), CDD_C_SUCCESS)\n"
                       "#define cdd_json_array_get_string(arr, i, out) (*(out) "
                       "= json_array_get_string(arr, i), CDD_C_SUCCESS)\n"
                       "#define cdd_json_array_get_number(arr, i, out) (*(out) "
                       "= json_array_get_number(arr, i), CDD_C_SUCCESS)\n"));
  F_CHECK_IO(FPRINTF_HOOK(
      fp, "#define cdd_json_array_get_boolean(arr, i, out) (*(out) = "
          "json_array_get_boolean(arr, i), CDD_C_SUCCESS)\n"
          "#define cdd_json_array_get_object(arr, i, out) (*(out) = "
          "json_array_get_object(arr, i), CDD_C_SUCCESS)\n"
          "#endif\n\n"));

  {
    size_t count = 0;
    cdd_c_error_t rc_fn = CDD_C_SUCCESS;
    F_CHECK_RC_TESTABLE(cdd_json_object_get_count(schemas_obj, &count));
    for (i = 0; i < count; i++) {
      const char *raw_name = NULL;
      JSON_Object *s = NULL;
      char name[64];
      struct StructFields sf;
      int is_obj_hint = 0;
      int is_object_schema = 0;
      cdd_c_error_t rc_step;

      F_CHECK_RC_TESTABLE(
          schema_entry_at(schemas_obj, i, &raw_name, &s, &is_obj_hint));

      rc_step = resolve_schema_c_name(raw_name, s, name);
      if (rc_step != CDD_C_SUCCESS) {
        fclose(fp);
        return cdd_c_keep_err(CDD_C_ERROR_MEMORY, rc_step);
      }

      rc_step = struct_fields_init(&sf);
      if (rc_step != CDD_C_SUCCESS) {
        fclose(fp);
        return cdd_c_keep_err(CDD_C_ERROR_MEMORY, rc_step);
      }
      rc_step = json_object_to_struct_fields_ex_codegen(s, &sf, schemas_obj,
                                                        raw_name);
      if (rc_step != CDD_C_SUCCESS) {
        fclose(fp);
        return cdd_c_keep_err(CDD_C_ERROR_MEMORY,
                              cdd_c_keep_err(rc_step, struct_fields_free(&sf)));
      }

      is_object_schema = is_obj_hint || sf.size > 0;

      if (sf.is_enum) {
        F_CHECK_RC_TESTABLE_SF(
            write_enum_to_str_func(fp, name, &sf.enum_members, &enum_cfg));
        F_CHECK_RC_TESTABLE_SF(
            write_enum_from_str_func(fp, name, &sf.enum_members, &enum_cfg));
      } else if (sf.is_union) {
        F_CHECK_RC_TESTABLE_SF(
            write_union_from_jsonObject_func(fp, name, &sf, &types_cfg));
        F_CHECK_RC_TESTABLE_SF(
            write_union_from_json_func(fp, name, &sf, &types_cfg));
        F_CHECK_RC_TESTABLE_SF(
            write_union_to_json_func(fp, name, &sf, &types_cfg));
        F_CHECK_RC_TESTABLE_SF(
            write_union_cleanup_func(fp, name, &sf, &types_cfg));
      } else if (is_object_schema) {
        F_CHECK_RC_TESTABLE_SF(
            write_struct_from_jsonObject_func(fp, name, &sf, &json_cfg));
        F_CHECK_RC_TESTABLE_SF(
            write_struct_from_json_func(fp, name, &json_cfg));
        F_CHECK_RC_TESTABLE_SF(
            write_struct_array_from_json_func(fp, name, &json_cfg));
        F_CHECK_RC_TESTABLE_SF(
            write_struct_to_json_func(fp, name, &sf, &json_cfg));
        F_CHECK_RC_TESTABLE_SF(
            write_struct_to_form_urlencoded_func(fp, name, &sf));
        if (strcmp(name, "OAuth2Error") == 0) {
          F_CHECK_RC_TESTABLE_SF(write_oauth2_error_parser_func(fp, name, &sf));
        }
        if (strcmp(name, "JwtPayload") == 0) {
          F_CHECK_RC_TESTABLE_SF(write_struct_from_jwt_func(fp, name, &sf));
        }
        if (strcmp(name, "OAuth2TokenResponse") == 0) {
          F_CHECK_RC_TESTABLE_SF(
              write_struct_from_json_standalone_func(fp, name, &sf));
        }
        F_CHECK_RC_TESTABLE_SF(
            write_struct_cleanup_func(fp, name, &sf, &struct_cfg));
        F_CHECK_RC_TESTABLE_SF(
            write_struct_default_func(fp, name, &sf, &struct_cfg));
        F_CHECK_RC_TESTABLE_SF(
            write_struct_deepcopy_func(fp, name, &sf, &struct_cfg));
        F_CHECK_RC_TESTABLE_SF(
            write_struct_eq_func(fp, name, &sf, &struct_cfg));
      }

      rc_fn = struct_fields_free(&sf);
    }
    fclose(fp);
    return rc_fn;
  }
}

/**
 * @brief Locates the schemas JSON object map (`components.schemas` or `$defs`)
 * from a parsed root JSON value.
 *
 * @param[in] root Parsed root JSON value.
 * @param[out] out_schemas Resolved schemas map object.
 * @return CDD_C_SUCCESS if found, or error code if absent/invalid.
 */
static cdd_c_error_t extract_schemas_map(const JSON_Value *root,
                                         JSON_Object **out_schemas) {
  JSON_Object *root_obj = (root->type == JSONObject) ? root->object_val : NULL;
  JSON_Object *comp = NULL;
  cdd_c_error_t rc;
  *out_schemas = NULL;
  rc = cdd_json_object_get_object(root_obj, "components", &comp);
  if (rc == CDD_C_SUCCESS) {
    cdd_c_error_t rc_s =
        cdd_json_object_get_object(comp, "schemas", out_schemas);
    if (rc_s == CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc_s, rc);
    }
  }
  return cdd_json_object_get_object(root_obj, "$defs", out_schemas);
}

/*
 * @brief CLI entry point for generating C structs and JSON codecs from a JSON
 * Schema.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t schema2code_main(int argc, char **argv) {
  const char *schema_file, *prefix;
  char *basename = NULL;
  struct CodegenConfig config = {0};
  JSON_Value *root = NULL;
  JSON_Object *schemas = NULL;
  cdd_c_error_t rc;
  int i;

  if (argc < 2) {
    return CDD_C_ERROR_UNKNOWN;
  }
  schema_file = argv[0];
  prefix = argv[1];
  rc = get_basename(prefix, &basename);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(CDD_C_ERROR_UNKNOWN, rc);
  }

  for (i = 2; i < argc; ++i) {
    if (strncmp(argv[i], "--guard-enum=", 13) == 0) {
      config.enum_guard = argv[i] + 13;
    } else if (strncmp(argv[i], "--guard-json=", 13) == 0) {
      config.json_guard = argv[i] + 13;
    } else if (strncmp(argv[i], "--guard-utils=", 14) == 0) {
      config.utils_guard = argv[i] + 14;
    }
  }

  rc = cdd_json_parse_file(schema_file, &root);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(basename);
    return cdd_c_keep_err(CDD_C_ERROR_UNKNOWN, rc);
  }

  rc = extract_schemas_map(root, &schemas);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(basename);
    return cdd_c_keep_err(CDD_C_ERROR_UNKNOWN,
                          cdd_c_keep_err(rc, cdd_json_value_free(root)));
  }

  rc = generate_header(prefix, basename, schemas, &config);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(basename);
    return cdd_c_keep_err(CDD_C_ERROR_UNKNOWN,
                          cdd_c_keep_err(rc, cdd_json_value_free(root)));
  }
  rc = generate_source(prefix, basename, schemas, &config);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(basename);
    return cdd_c_keep_err(CDD_C_ERROR_UNKNOWN,
                          cdd_c_keep_err(rc, cdd_json_value_free(root)));
  }

  C_CDD_FREE(basename);
  return cdd_json_value_free(root);
}

/*  */
