/**
 * @file json_dom.h
 * @brief Zero-dependency C89 JSON DOM parser, mutator, and serializer API.
 *
 * Provides a strict C89 in-memory JSON Document Object Model (DOM) where all
 * core functions return `cdd_c_error_t` or `JSON_Status` error enums and pass
 * outputs via pointer parameters.
 */

#ifndef C_CDD_JSON_DOM_H
#define C_CDD_JSON_DOM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stddef.h>
#include <stdio.h>
#if defined(_MSC_VER) && _MSC_VER < 1600
#include "msvc/stdint.h"
#else
#include <stdint.h>
#endif
#include "c_cdd_export.h"
#include "cdd_c_error.h"
/* clang-format on */

#ifndef CDD_INT128_TYPES_DEFINED
/** @brief Guard indicating 128-bit integer structs are defined. */
#define CDD_INT128_TYPES_DEFINED
/**
 * @brief 128-bit unsigned integer structure.
 */
typedef struct __cdd_uint128 {
  uint64_t low;  /**< Lower 64 bits */
  uint64_t high; /**< Upper 64 bits */
} cdd_uint128_t;

/**
 * @brief 128-bit signed integer structure.
 */
typedef struct __cdd_int128 {
  uint64_t low; /**< Lower 64 bits */
  int64_t high; /**< Upper 64 bits (signed) */
} cdd_int128_t;
#endif /* CDD_INT128_TYPES_DEFINED */

/**
 * @brief JSON value type enumeration.
 */
typedef enum json_value_type_t {
  JSONError = -1,
  JSONNull = 1,
  JSONString = 2,
  JSONNumber = 3,
  JSONObject = 4,
  JSONArray = 5,
  JSONBoolean = 6
} JSON_Value_Type;

/**
 * @brief Status enumeration for JSON DOM mutation operations.
 */
typedef enum json_result_t { JSONSuccess = 0, JSONFailure = -1 } JSON_Status;

/**
 * @brief Boolean / presence return enumeration for JSON DOM queries.
 */
typedef enum cdd_json_bool_t {
  CDD_JSON_ERR_BOOL = -1,
  CDD_JSON_FALSE = 0,
  CDD_JSON_TRUE = 1
} cdd_json_bool_t;

/** @brief Forward declaration of JSON_Value. */
typedef struct json_value_t JSON_Value;
/** @brief Forward declaration of JSON_Object. */
typedef struct json_object_t JSON_Object;
/** @brief Forward declaration of JSON_Array. */
typedef struct json_array_t JSON_Array;

/**
 * @brief Custom memory allocation callback signature.
 */
typedef void *(*JSON_Malloc_Function)(size_t);

/**
 * @brief Custom memory deallocation callback signature.
 */
typedef void (*JSON_Free_Function)(void *);

/**
 * @brief Structured diagnostics for JSON parse failures.
 */
struct cdd_json_parse_error {
  /** @brief 1-based line number where the error occurred. */
  size_t line;
  /** @brief 1-based column number where the error occurred. */
  size_t column;
  /** @brief 0-based byte offset from the start of the input string. */
  size_t byte_offset;
};

/**
 * @brief Configurable options for JSON parsing.
 */
struct cdd_json_parse_options {
  /** @brief Non-zero to allow `//` and block comments. */
  int allow_comments;
  /** @brief Non-zero to allow trailing commas in objects and arrays. */
  int allow_trailing_commas;
  /** @brief Non-zero to reject duplicate keys within the same JSON object. */
  int reject_duplicate_keys;
  /** @brief Maximum nesting depth (0 uses default CDD_JSON_MAX_DEPTH). */
  size_t max_depth;
};

/**
 * @brief JSON Object key-value collection preserving insertion order.
 */
struct json_object_t {
  /** @brief Owning JSON_Value wrapper. */
  JSON_Value *wrapping_value;
  /** @brief Array of heap-allocated key strings. */
  char **names;
  /** @brief Array of explicit byte lengths for each key in names. */
  size_t *name_lens;
  /** @brief Array of child JSON_Value pointers. */
  JSON_Value **values;
  /** @brief Number of key-value pairs currently stored. */
  size_t count;
  /** @brief Allocated capacity of names and values arrays. */
  size_t capacity;
  /** @brief Open-addressing hash bucket array storing 1-based entry indices (0
   * = empty). */
  size_t *hash_buckets;
  /** @brief Capacity of hash_buckets (power of 2, or 0 if inactive). */
  size_t hash_cap;
};

/**
 * @brief JSON Array ordered dynamic collection.
 */
struct json_array_t {
  /** @brief Owning JSON_Value wrapper. */
  JSON_Value *wrapping_value;
  /** @brief Array of child JSON_Value pointers. */
  JSON_Value **items;
  /** @brief Number of elements currently stored. */
  size_t count;
  /** @brief Allocated capacity of items array. */
  size_t capacity;
};

/**
 * @brief Tagged union representing any JSON DOM value.
 */
struct json_value_t {
  /** @brief Parent JSON_Value node, if attached. */
  JSON_Value *parent;
  /** @brief Discriminator tag indicating active value kind. */
  JSON_Value_Type type;
  /** @brief Heap string when type == JSONString. */
  char *string_val;
  /** @brief Explicit byte length of string_val when type == JSONString. */
  size_t string_len;
  /** @brief Numeric value when type == JSONNumber. */
  double number_val;
  /** @brief Signed 64-bit integer value when num_kind == 1. */
  int64_t int64_val;
  /** @brief Unsigned 64-bit integer value when num_kind == 2. */
  uint64_t uint64_val;
  /** @brief Signed 128-bit integer value when num_kind == 3. */
  cdd_int128_t int128_val;
  /** @brief Unsigned 128-bit integer value when num_kind == 4. */
  cdd_uint128_t uint128_val;
  /** @brief Exact raw numeric literal string when type == JSONNumber. */
  char *raw_num_str;
  /** @brief Numeric subtype (0 = double, 1 = int64, 2 = uint64, 3 = int128, 4 =
   * uint128). */
  int num_kind;
  /** @brief Boolean value (0 or 1) when type == JSONBoolean. */
  int boolean_val;
  /** @brief Object pointer when type == JSONObject. */
  JSON_Object *object_val;
  /** @brief Array pointer when type == JSONArray. */
  JSON_Array *array_val;
};

/**
 * @brief Configures custom memory allocation and deallocation callbacks.
 *
 * @param[in] malloc_fun Custom malloc callback (or NULL to reset to malloc).
 * @param[in] free_fun Custom free callback (or NULL to reset to free).
 * @return CDD_C_SUCCESS.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_set_allocation_functions(
    JSON_Malloc_Function malloc_fun, JSON_Free_Function free_fun);

/**
 * @brief Configures custom memory allocation and deallocation callbacks.
 *
 * @param[in] malloc_fun Custom malloc callback.
 * @param[in] free_fun Custom free callback.
 * @return JSONSuccess.
 */
extern C_CDD_EXPORT JSON_Status json_set_allocation_functions(
    JSON_Malloc_Function malloc_fun, JSON_Free_Function free_fun);

/**
 * @brief Parses a UTF-8 JSON string into a newly allocated JSON_Value tree
 * with optional structured error diagnostics.
 *
 * @param[in] str Null-terminated JSON text.
 * @param[out] out Output pointer receiving the root JSON_Value.
 * @param[out] err_out Optional output structure receiving line/column/offset on
 * error.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_parse_string_ex(
    const char *str, JSON_Value **out, struct cdd_json_parse_error *err_out);

/**
 * @brief Parses a UTF-8 JSON string into a newly allocated JSON_Value tree.
 *
 * @param[in] str Null-terminated JSON text.
 * @param[out] out Output pointer receiving the root JSON_Value.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_parse_string(const char *str,
                                                        JSON_Value **out);

/**
 * @brief Parses a JSON string after stripping C/C++ line and block comments.
 *
 * @param[in] str Null-terminated JSON with comments.
 * @param[out] out Output pointer receiving the root JSON_Value.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_parse_string_with_comments(const char *str, JSON_Value **out);

/**
 * @brief Parses a JSON buffer of explicit byte length into a JSON_Value tree.
 *
 * @param[in] buf Buffer containing UTF-8 JSON bytes.
 * @param[in] len Number of bytes in buf.
 * @param[out] out Output pointer receiving the root JSON_Value.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_parse_buffer(const char *buf,
                                                        size_t len,
                                                        JSON_Value **out);

/**
 * @brief Parses a UTF-8 JSON string using configurable parser options.
 *
 * @param[in] str Null-terminated JSON text.
 * @param[in] options Optional parser configuration (NULL uses strict RFC 8259
 * defaults).
 * @param[out] out Output pointer receiving the root JSON_Value.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_parse_string_with_options(
    const char *str, const struct cdd_json_parse_options *options,
    JSON_Value **out);

/**
 * @brief Reads a file and parses its JSON content into a JSON_Value tree.
 *
 * @param[in] filename Path to the JSON file.
 * @param[out] out Output pointer receiving the root JSON_Value.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_parse_file(const char *filename,
                                                      JSON_Value **out);

/**
 * @brief Allocates and initializes a new JSONObject value.
 *
 * @param[out] out Output pointer receiving the new JSON_Value.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY on OOM.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_value_init_object(JSON_Value **out);

/**
 * @brief Allocates and initializes a new JSONArray value.
 *
 * @param[out] out Output pointer receiving the new JSON_Value.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY on OOM.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_value_init_array(JSON_Value **out);

/**
 * @brief Allocates and initializes a new JSONString value with explicit byte
 * length (preserving embedded NUL bytes).
 *
 * @param[in] str String bytes to copy.
 * @param[in] len Number of bytes to copy.
 * @param[out] out Output pointer receiving the new JSON_Value.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_value_init_string_with_len(
    const char *str, size_t len, JSON_Value **out);

/**
 * @brief Allocates and initializes a new JSONString value.
 *
 * @param[in] str Null-terminated string to copy.
 * @param[out] out Output pointer receiving the new JSON_Value.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_value_init_string(const char *str,
                                                             JSON_Value **out);

/**
 * @brief Retrieves the explicit byte length of a JSONString value.
 *
 * @param[in] val JSON_Value of type JSONString.
 * @param[out] out_len Output pointer receiving the byte length.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_value_get_string_len(const JSON_Value *val, size_t *out_len);

/**
 * @brief Allocates and initializes a new JSONNumber value.
 *
 * @param[in] number Numeric value.
 * @param[out] out Output pointer receiving the new JSON_Value.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_value_init_number(double number,
                                                             JSON_Value **out);

/**
 * @brief Allocates and initializes a new JSONNumber value from a signed 64-bit
 * integer without precision loss.
 *
 * @param[in] val Signed 64-bit integer value.
 * @param[out] out Output pointer receiving the new JSON_Value.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_value_init_int64(int64_t val,
                                                            JSON_Value **out);

/**
 * @brief Allocates and initializes a new JSONNumber value from an unsigned
 * 64-bit integer without precision loss.
 *
 * @param[in] val Unsigned 64-bit integer value.
 * @param[out] out Output pointer receiving the new JSON_Value.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_value_init_uint64(uint64_t val,
                                                             JSON_Value **out);

/**
 * @brief Retrieves a signed 64-bit integer from a JSONNumber value.
 *
 * @param[in] val JSONNumber value to inspect.
 * @param[out] out Output pointer receiving the signed 64-bit integer.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_value_get_int64(const JSON_Value *val, int64_t *out);

/**
 * @brief Retrieves an unsigned 64-bit integer from a JSONNumber value.
 *
 * @param[in] val JSONNumber value to inspect.
 * @param[out] out Output pointer receiving the unsigned 64-bit integer.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_value_get_uint64(const JSON_Value *val, uint64_t *out);

/**
 * @brief Allocates and initializes a new JSONNumber value from a signed 128-bit
 * integer without precision loss.
 *
 * @param[in] val Signed 128-bit integer value.
 * @param[out] out Output pointer receiving the new JSON_Value.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_value_init_int128(cdd_int128_t val,
                                                             JSON_Value **out);

/**
 * @brief Allocates and initializes a new JSONNumber value from an unsigned
 * 128-bit integer without precision loss.
 *
 * @param[in] val Unsigned 128-bit integer value.
 * @param[out] out Output pointer receiving the new JSON_Value.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_value_init_uint128(cdd_uint128_t val,
                                                              JSON_Value **out);

/**
 * @brief Retrieves a signed 128-bit integer from a JSONNumber value.
 *
 * @param[in] val JSONNumber value to inspect.
 * @param[out] out Output pointer receiving the signed 128-bit integer.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_value_get_int128(const JSON_Value *val, cdd_int128_t *out);

/**
 * @brief Retrieves an unsigned 128-bit integer from a JSONNumber value.
 *
 * @param[in] val JSONNumber value to inspect.
 * @param[out] out Output pointer receiving the unsigned 128-bit integer.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_value_get_uint128(const JSON_Value *val, cdd_uint128_t *out);

/**
 * @brief Allocates and initializes a new JSONBoolean value.
 *
 * @param[in] boolean Boolean value (0 or non-zero).
 * @param[out] out Output pointer receiving the new JSON_Value.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_value_init_boolean(int boolean,
                                                              JSON_Value **out);

/**
 * @brief Allocates and initializes a new JSONNull value.
 *
 * @param[out] out Output pointer receiving the new JSON_Value.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY on OOM.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_value_init_null(JSON_Value **out);

/**
 * @brief Deep-copies a JSON_Value tree.
 *
 * @param[in] val Source JSON_Value to clone.
 * @param[out] out Output pointer receiving the cloned JSON_Value.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_value_deep_copy(const JSON_Value *val, JSON_Value **out);

/**
 * @brief Frees a JSON_Value tree and all owned children.
 *
 * @param[in] val Root value to free (safe if NULL).
 * @return CDD_C_SUCCESS.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_value_free(JSON_Value *val);

/**
 * @brief Frees a JSON_Value tree and all owned children.
 *
 * @param[in] val Root value to free (safe if NULL).
 * @return CDD_C_SUCCESS.
 */
extern C_CDD_EXPORT cdd_c_error_t json_value_free(JSON_Value *val);

/**
 * @brief Frees a heap string returned by JSON serialization.
 *
 * @param[in] str String to free (safe if NULL).
 * @return CDD_C_SUCCESS.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_free_serialized_string(char *str);

/**
 * @brief Frees a heap string returned by JSON serialization.
 *
 * @param[in] str String to free (safe if NULL).
 * @return CDD_C_SUCCESS.
 */
extern C_CDD_EXPORT cdd_c_error_t json_free_serialized_string(char *str);

/**
 * @brief Retrieves the type tag of a JSON_Value via out-parameter.
 *
 * @param[in] val Value to inspect.
 * @param[out] out_type Output pointer receiving JSON_Value_Type.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_value_get_type(const JSON_Value *val, JSON_Value_Type *out_type);

/**
 * @brief Returns the type tag of a JSON_Value.
 *
 * @param[in] val Value to inspect.
 * @return JSON_Value_Type tag, or JSONError if val is NULL.
 */
extern C_CDD_EXPORT JSON_Value_Type json_value_get_type(const JSON_Value *val);

/**
 * @brief Retrieves the boolean value of a JSON_Value via out-parameter.
 *
 * @param[in] val Value to inspect.
 * @param[out] out_bool Output pointer receiving 0 or 1.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_value_get_boolean(const JSON_Value *val, int *out_bool);

/**
 * @brief Returns the boolean state of a JSON_Value.
 *
 * @param[in] val Value to inspect.
 * @return CDD_JSON_TRUE (1), CDD_JSON_FALSE (0), or CDD_JSON_ERR_BOOL (-1).
 */
extern C_CDD_EXPORT cdd_json_bool_t
json_value_get_boolean(const JSON_Value *val);

/**
 * @brief Retrieves the numeric value of a JSON_Value via out-parameter.
 *
 * @param[in] val Value to inspect.
 * @param[out] out_num Output pointer receiving the double value.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_value_get_number(const JSON_Value *val, double *out_num);

/**
 * @brief Retrieves the JSON_Object from a JSON_Value via out-parameter.
 *
 * @param[in] val Value to inspect.
 * @param[out] out_obj Output pointer receiving the JSON_Object.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_value_get_object(const JSON_Value *val, JSON_Object **out_obj);

/**
 * @brief Retrieves the JSON_Array from a JSON_Value via out-parameter.
 *
 * @param[in] val Value to inspect.
 * @param[out] out_arr Output pointer receiving the JSON_Array.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_value_get_array(const JSON_Value *val, JSON_Array **out_arr);

/**
 * @brief Retrieves the C string from a JSON_Value via out-parameter.
 *
 * @param[in] val Value to inspect.
 * @param[out] out_str Output pointer receiving the string.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_value_get_string(const JSON_Value *val, const char **out_str);

/**
 * @brief Checks deep structural equality of two JSON_Value trees.
 *
 * @param[in] a First JSON_Value.
 * @param[in] b Second JSON_Value.
 * @param[out] out_eq Output pointer receiving 1 if equal, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if out_eq
 * is NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_value_equals(const JSON_Value *a,
                                                        const JSON_Value *b,
                                                        int *out_eq);

/**
 * @brief Checks deep structural equality of two JSON_Value trees.
 *
 * @param[in] a First JSON_Value.
 * @param[in] b Second JSON_Value.
 * @return CDD_JSON_TRUE if equal, CDD_JSON_FALSE otherwise.
 */
extern C_CDD_EXPORT cdd_json_bool_t json_value_equals(const JSON_Value *a,
                                                      const JSON_Value *b);

/**
 * @brief Validates a JSON_Value tree against a structural template schema.
 *
 * @param[in] schema Template JSON_Value defining expected types/keys.
 * @param[in] value Target JSON_Value to validate.
 * @return CDD_C_SUCCESS if valid, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_validate(const JSON_Value *schema,
                                                    const JSON_Value *value);

/**
 * @brief Validates a JSON_Value tree against a structural template schema.
 *
 * @param[in] schema Template JSON_Value defining expected types/keys.
 * @param[in] value Target JSON_Value to validate.
 * @return JSONSuccess if valid, or JSONFailure.
 */
extern C_CDD_EXPORT JSON_Status json_validate(const JSON_Value *schema,
                                              const JSON_Value *value);

/**
 * @brief Looks up a child value by key in a JSON_Object.
 *
 * @param[in] obj Object to search.
 * @param[in] name Key name.
 * @param[out] out Output pointer receiving the child JSON_Value.
 * @return CDD_C_SUCCESS if found, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_get_value(
    const JSON_Object *obj, const char *name, JSON_Value **out);

/**
 * @brief Looks up a child string by key in a JSON_Object.
 *
 * @param[in] obj Object to search.
 * @param[in] name Key name.
 * @param[out] out Output pointer receiving the string.
 * @return CDD_C_SUCCESS if found, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_get_string(
    const JSON_Object *obj, const char *name, const char **out);

/**
 * @brief Looks up a child object by key in a JSON_Object.
 *
 * @param[in] obj Object to search.
 * @param[in] name Key name.
 * @param[out] out Output pointer receiving the child JSON_Object.
 * @return CDD_C_SUCCESS if found, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_get_object(
    const JSON_Object *obj, const char *name, JSON_Object **out);

/**
 * @brief Looks up a child array by key in a JSON_Object.
 *
 * @param[in] obj Object to search.
 * @param[in] name Key name.
 * @param[out] out Output pointer receiving the child JSON_Array.
 * @return CDD_C_SUCCESS if found, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_get_array(
    const JSON_Object *obj, const char *name, JSON_Array **out);

/**
 * @brief Looks up a child boolean by key in a JSON_Object.
 *
 * @param[in] obj Object to search.
 * @param[in] name Key name.
 * @param[out] out_bool Output pointer receiving 0 or 1.
 * @return CDD_C_SUCCESS if found, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_get_boolean(
    const JSON_Object *obj, const char *name, int *out_bool);

/**
 * @brief Looks up a child number by key in a JSON_Object.
 *
 * @param[in] obj Object to search.
 * @param[in] name Key name.
 * @param[out] out_num Output pointer receiving the numeric value.
 * @return CDD_C_SUCCESS if found, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_get_number(
    const JSON_Object *obj, const char *name, double *out_num);

/**
 * @brief Looks up a signed 64-bit integer by key in a JSON_Object.
 *
 * @param[in] obj Object to search.
 * @param[in] name Key name.
 * @param[out] out Output pointer receiving the signed 64-bit integer.
 * @return CDD_C_SUCCESS if found, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_get_int64(
    const JSON_Object *obj, const char *name, int64_t *out);

/**
 * @brief Looks up an unsigned 64-bit integer by key in a JSON_Object.
 *
 * @param[in] obj Object to search.
 * @param[in] name Key name.
 * @param[out] out Output pointer receiving the unsigned 64-bit integer.
 * @return CDD_C_SUCCESS if found, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_get_uint64(
    const JSON_Object *obj, const char *name, uint64_t *out);

/**
 * @brief Looks up a signed 128-bit integer by key in a JSON_Object.
 *
 * @param[in] obj Object to search.
 * @param[in] name Key name.
 * @param[out] out Output pointer receiving the signed 128-bit integer.
 * @return CDD_C_SUCCESS if found, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_get_int128(
    const JSON_Object *obj, const char *name, cdd_int128_t *out);

/**
 * @brief Looks up an unsigned 128-bit integer by key in a JSON_Object.
 *
 * @param[in] obj Object to search.
 * @param[in] name Key name.
 * @param[out] out Output pointer receiving the unsigned 128-bit integer.
 * @return CDD_C_SUCCESS if found, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_get_uint128(
    const JSON_Object *obj, const char *name, cdd_uint128_t *out);

/**
 * @brief Retrieves the number of key-value pairs in a JSON_Object.
 *
 * @param[in] obj Object to inspect.
 * @param[out] out_count Output pointer receiving entry count.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_object_get_count(const JSON_Object *obj, size_t *out_count);

/**
 * @brief Retrieves the key name at index idx in a JSON_Object.
 *
 * @param[in] obj Object to inspect.
 * @param[in] idx Zero-based entry index.
 * @param[out] out_name Output pointer receiving key name.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_get_name(
    const JSON_Object *obj, size_t idx, const char **out_name);

/**
 * @brief Retrieves the value at index idx in a JSON_Object.
 *
 * @param[in] obj Object to inspect.
 * @param[in] idx Zero-based entry index.
 * @param[out] out_val Output pointer receiving child JSON_Value.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_get_value_at(
    const JSON_Object *obj, size_t idx, JSON_Value **out_val);

/**
 * @brief Looks up a dot-separated path in a JSON_Object.
 *
 * @param[in] obj Root object to search.
 * @param[in] name Dot-separated path.
 * @param[out] out Output pointer receiving the target JSON_Value.
 * @return CDD_C_SUCCESS if found, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_dotget_value(
    const JSON_Object *obj, const char *name, JSON_Value **out);

/**
 * @brief Checks if a dot-separated path exists in a JSON_Object.
 *
 * @param[in] obj Root object to search.
 * @param[in] name Dot-separated path.
 * @param[out] out_has Output pointer receiving 1 if present, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_dothas_value(
    const JSON_Object *obj, const char *name, int *out_has);

/**
 * @brief Checks if a dot-separated path exists in a JSON_Object.
 *
 * @param[in] obj Root object to search.
 * @param[in] name Dot-separated path.
 * @return CDD_JSON_TRUE if present, CDD_JSON_FALSE otherwise.
 */
extern C_CDD_EXPORT cdd_json_bool_t
json_object_dothas_value(const JSON_Object *obj, const char *name);

/**
 * @brief Returns the boolean value for a key in a JSON_Object.
 *
 * @param[in] obj Object to search.
 * @param[in] name Key name.
 * @return CDD_JSON_TRUE (1), CDD_JSON_FALSE (0), or CDD_JSON_ERR_BOOL (-1).
 */
extern C_CDD_EXPORT cdd_json_bool_t
json_object_get_boolean(const JSON_Object *obj, const char *name);

/**
 * @brief Checks if a key exists in a JSON_Object via out-parameter.
 *
 * @param[in] obj Object to search.
 * @param[in] name Key name.
 * @param[out] out_has Output pointer receiving 1 if present, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_has_value(
    const JSON_Object *obj, const char *name, int *out_has);

/**
 * @brief Checks if a key exists in a JSON_Object.
 *
 * @param[in] obj Object to search.
 * @param[in] name Key name.
 * @return CDD_JSON_TRUE (1) if present, CDD_JSON_FALSE (0) otherwise.
 */
extern C_CDD_EXPORT cdd_json_bool_t
json_object_has_value(const JSON_Object *obj, const char *name);

/**
 * @brief Checks if a key with a specific value type exists in a JSON_Object
 * via out-parameter.
 *
 * @param[in] obj Object to search.
 * @param[in] name Key name.
 * @param[in] type Expected JSON_Value_Type.
 * @param[out] out_has Output pointer receiving 1 if present with matching type,
 * 0 otherwise.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_object_has_value_of_type(const JSON_Object *obj, const char *name,
                                  JSON_Value_Type type, int *out_has);

/**
 * @brief Checks if a key with a specific value type exists in a JSON_Object.
 *
 * @param[in] obj Object to search.
 * @param[in] name Key name.
 * @param[in] type Expected JSON_Value_Type.
 * @return CDD_JSON_TRUE (1) if present with matching type, CDD_JSON_FALSE (0)
 * otherwise.
 */
extern C_CDD_EXPORT cdd_json_bool_t json_object_has_value_of_type(
    const JSON_Object *obj, const char *name, JSON_Value_Type type);

/**
 * @brief Retrieves the number of elements in a JSON_Array.
 *
 * @param[in] arr Array to inspect.
 * @param[out] out_count Output pointer receiving element count.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_array_get_count(const JSON_Array *arr, size_t *out_count);

/**
 * @brief Retrieves the element at index idx in a JSON_Array.
 *
 * @param[in] arr Array to inspect.
 * @param[in] idx Zero-based element index.
 * @param[out] out_val Output pointer receiving the element JSON_Value.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_array_get_value(
    const JSON_Array *arr, size_t idx, JSON_Value **out_val);

/**
 * @brief Retrieves the string at index idx in a JSON_Array.
 *
 * @param[in] arr Array to inspect.
 * @param[in] idx Zero-based element index.
 * @param[out] out_str Output pointer receiving the string.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_array_get_string(
    const JSON_Array *arr, size_t idx, const char **out_str);

/**
 * @brief Retrieves the object at index idx in a JSON_Array.
 *
 * @param[in] arr Array to inspect.
 * @param[in] idx Zero-based element index.
 * @param[out] out_obj Output pointer receiving the JSON_Object.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_array_get_object(
    const JSON_Array *arr, size_t idx, JSON_Object **out_obj);

/**
 * @brief Retrieves the array at index idx in a JSON_Array.
 *
 * @param[in] arr Array to inspect.
 * @param[in] idx Zero-based element index.
 * @param[out] out_arr Output pointer receiving the JSON_Array.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_array_get_array(
    const JSON_Array *arr, size_t idx, JSON_Array **out_arr);

/**
 * @brief Retrieves the number at index idx in a JSON_Array.
 *
 * @param[in] arr Array to inspect.
 * @param[in] idx Zero-based element index.
 * @param[out] out_num Output pointer receiving the numeric value.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_array_get_number(const JSON_Array *arr, size_t idx, double *out_num);

/**
 * @brief Retrieves the boolean value at index idx in a JSON_Array.
 *
 * @param[in] arr Array to inspect.
 * @param[in] idx Zero-based element index.
 * @param[out] out_bool Output pointer receiving 0 or 1.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_array_get_boolean(const JSON_Array *arr, size_t idx, int *out_bool);

/**
 * @brief Returns the boolean value at an index in a JSON_Array.
 *
 * @param[in] arr Array to inspect.
 * @param[in] idx Zero-based element index.
 * @return CDD_JSON_TRUE (1), CDD_JSON_FALSE (0), or CDD_JSON_ERR_BOOL (-1).
 */
extern C_CDD_EXPORT cdd_json_bool_t
json_array_get_boolean(const JSON_Array *arr, size_t idx);

/**
 * @brief Sets or replaces a key-value pair in a JSON_Object.
 *
 * @param[in] obj Target object.
 * @param[in] name Key name.
 * @param[in] val Child value (ownership transferred on success).
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_set_value(JSON_Object *obj,
                                                            const char *name,
                                                            JSON_Value *val);

/**
 * @brief Sets or replaces a key-value pair in a JSON_Object.
 *
 * @param[in] obj Target object.
 * @param[in] name Key name.
 * @param[in] val Child value (ownership transferred on success).
 * @return JSONSuccess or JSONFailure.
 */
extern C_CDD_EXPORT JSON_Status json_object_set_value(JSON_Object *obj,
                                                      const char *name,
                                                      JSON_Value *val);

/**
 * @brief Sets a string property in a JSON_Object.
 *
 * @param[in] obj Target object.
 * @param[in] name Key name.
 * @param[in] str String value to copy.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_set_string(JSON_Object *obj,
                                                             const char *name,
                                                             const char *str);

/**
 * @brief Sets a string property in a JSON_Object.
 *
 * @param[in] obj Target object.
 * @param[in] name Key name.
 * @param[in] str String value to copy.
 * @return JSONSuccess or JSONFailure.
 */
extern C_CDD_EXPORT JSON_Status json_object_set_string(JSON_Object *obj,
                                                       const char *name,
                                                       const char *str);

/**
 * @brief Sets a numeric property in a JSON_Object.
 *
 * @param[in] obj Target object.
 * @param[in] name Key name.
 * @param[in] number Numeric value.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_set_number(JSON_Object *obj,
                                                             const char *name,
                                                             double number);

/**
 * @brief Sets a signed 64-bit integer property in a JSON_Object.
 *
 * @param[in] obj Target object.
 * @param[in] name Key name.
 * @param[in] val Signed 64-bit integer value.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_set_int64(JSON_Object *obj,
                                                            const char *name,
                                                            int64_t val);

/**
 * @brief Sets an unsigned 64-bit integer property in a JSON_Object.
 *
 * @param[in] obj Target object.
 * @param[in] name Key name.
 * @param[in] val Unsigned 64-bit integer value.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_set_uint64(JSON_Object *obj,
                                                             const char *name,
                                                             uint64_t val);

/**
 * @brief Sets a signed 128-bit integer property in a JSON_Object.
 *
 * @param[in] obj Target object.
 * @param[in] name Key name.
 * @param[in] val Signed 128-bit integer value.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_set_int128(JSON_Object *obj,
                                                             const char *name,
                                                             cdd_int128_t val);

/**
 * @brief Sets an unsigned 128-bit integer property in a JSON_Object.
 *
 * @param[in] obj Target object.
 * @param[in] name Key name.
 * @param[in] val Unsigned 128-bit integer value.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_set_uint128(
    JSON_Object *obj, const char *name, cdd_uint128_t val);

/**
 * @brief Retrieves the signed 64-bit integer at index idx in a JSON_Array.
 *
 * @param[in] arr Array to inspect.
 * @param[in] idx Zero-based element index.
 * @param[out] out Output pointer receiving the signed 64-bit integer.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_array_get_int64(const JSON_Array *arr, size_t idx, int64_t *out);

/**
 * @brief Retrieves the unsigned 64-bit integer at index idx in a JSON_Array.
 *
 * @param[in] arr Array to inspect.
 * @param[in] idx Zero-based element index.
 * @param[out] out Output pointer receiving the unsigned 64-bit integer.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_array_get_uint64(const JSON_Array *arr, size_t idx, uint64_t *out);

/**
 * @brief Retrieves the signed 128-bit integer at index idx in a JSON_Array.
 *
 * @param[in] arr Array to inspect.
 * @param[in] idx Zero-based element index.
 * @param[out] out Output pointer receiving the signed 128-bit integer.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_array_get_int128(const JSON_Array *arr, size_t idx, cdd_int128_t *out);

/**
 * @brief Retrieves the unsigned 128-bit integer at index idx in a JSON_Array.
 *
 * @param[in] arr Array to inspect.
 * @param[in] idx Zero-based element index.
 * @param[out] out Output pointer receiving the unsigned 128-bit integer.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_array_get_uint128(
    const JSON_Array *arr, size_t idx, cdd_uint128_t *out);

/**
 * @brief Appends a signed 64-bit integer to a JSON_Array.
 *
 * @param[in] arr Target array.
 * @param[in] val Signed 64-bit integer value to append.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_array_append_int64(JSON_Array *arr,
                                                              int64_t val);

/**
 * @brief Appends an unsigned 64-bit integer to a JSON_Array.
 *
 * @param[in] arr Target array.
 * @param[in] val Unsigned 64-bit integer value to append.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_array_append_uint64(JSON_Array *arr,
                                                               uint64_t val);

/**
 * @brief Appends a signed 128-bit integer to a JSON_Array.
 *
 * @param[in] arr Target array.
 * @param[in] val Signed 128-bit integer value to append.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_array_append_int128(JSON_Array *arr, cdd_int128_t val);

/**
 * @brief Appends an unsigned 128-bit integer to a JSON_Array.
 *
 * @param[in] arr Target array.
 * @param[in] val Unsigned 128-bit integer value to append.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_array_append_uint128(JSON_Array *arr, cdd_uint128_t val);

/**
 * @brief Sets a numeric property in a JSON_Object.
 *
 * @param[in] obj Target object.
 * @param[in] name Key name.
 * @param[in] number Numeric value.
 * @return JSONSuccess or JSONFailure.
 */
extern C_CDD_EXPORT JSON_Status json_object_set_number(JSON_Object *obj,
                                                       const char *name,
                                                       double number);

/**
 * @brief Sets a boolean property in a JSON_Object.
 *
 * @param[in] obj Target object.
 * @param[in] name Key name.
 * @param[in] boolean Boolean value.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_set_boolean(JSON_Object *obj,
                                                              const char *name,
                                                              int boolean);

/**
 * @brief Sets a boolean property in a JSON_Object.
 *
 * @param[in] obj Target object.
 * @param[in] name Key name.
 * @param[in] boolean Boolean value.
 * @return JSONSuccess or JSONFailure.
 */
extern C_CDD_EXPORT JSON_Status json_object_set_boolean(JSON_Object *obj,
                                                        const char *name,
                                                        int boolean);

/**
 * @brief Sets a null property in a JSON_Object.
 *
 * @param[in] obj Target object.
 * @param[in] name Key name.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_set_null(JSON_Object *obj,
                                                           const char *name);

/**
 * @brief Sets a null property in a JSON_Object.
 *
 * @param[in] obj Target object.
 * @param[in] name Key name.
 * @return JSONSuccess or JSONFailure.
 */
extern C_CDD_EXPORT JSON_Status json_object_set_null(JSON_Object *obj,
                                                     const char *name);

/**
 * @brief Sets a value along a dot-separated path in a JSON_Object, creating
 * intermediate objects as needed.
 *
 * @param[in] obj Target root object.
 * @param[in] name Dot-separated path.
 * @param[in] val Value to set.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_dotset_value(JSON_Object *obj,
                                                               const char *name,
                                                               JSON_Value *val);

/**
 * @brief Sets a value along a dot-separated path in a JSON_Object, creating
 * intermediate objects as needed.
 *
 * @param[in] obj Target root object.
 * @param[in] name Dot-separated path.
 * @param[in] val Value to set.
 * @return JSONSuccess or JSONFailure.
 */
extern C_CDD_EXPORT JSON_Status json_object_dotset_value(JSON_Object *obj,
                                                         const char *name,
                                                         JSON_Value *val);

/**
 * @brief Sets a string value along a dot-separated path in a JSON_Object.
 *
 * @param[in] obj Target root object.
 * @param[in] name Dot-separated path.
 * @param[in] str String value to copy.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_dotset_string(
    JSON_Object *obj, const char *name, const char *str);

/**
 * @brief Sets a string value along a dot-separated path in a JSON_Object.
 *
 * @param[in] obj Target root object.
 * @param[in] name Dot-separated path.
 * @param[in] str String value to copy.
 * @return JSONSuccess or JSONFailure.
 */
extern C_CDD_EXPORT JSON_Status json_object_dotset_string(JSON_Object *obj,
                                                          const char *name,
                                                          const char *str);

/**
 * @brief Sets a numeric value along a dot-separated path in a JSON_Object.
 *
 * @param[in] obj Target root object.
 * @param[in] name Dot-separated path.
 * @param[in] number Numeric value.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_dotset_number(
    JSON_Object *obj, const char *name, double number);

/**
 * @brief Sets a numeric value along a dot-separated path in a JSON_Object.
 *
 * @param[in] obj Target root object.
 * @param[in] name Dot-separated path.
 * @param[in] number Numeric value.
 * @return JSONSuccess or JSONFailure.
 */
extern C_CDD_EXPORT JSON_Status json_object_dotset_number(JSON_Object *obj,
                                                          const char *name,
                                                          double number);

/**
 * @brief Sets a boolean value along a dot-separated path in a JSON_Object.
 *
 * @param[in] obj Target root object.
 * @param[in] name Dot-separated path.
 * @param[in] boolean Boolean value.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_object_dotset_boolean(JSON_Object *obj, const char *name, int boolean);

/**
 * @brief Sets a boolean value along a dot-separated path in a JSON_Object.
 *
 * @param[in] obj Target root object.
 * @param[in] name Dot-separated path.
 * @param[in] boolean Boolean value.
 * @return JSONSuccess or JSONFailure.
 */
extern C_CDD_EXPORT JSON_Status json_object_dotset_boolean(JSON_Object *obj,
                                                           const char *name,
                                                           int boolean);

/**
 * @brief Sets a null value along a dot-separated path in a JSON_Object.
 *
 * @param[in] obj Target root object.
 * @param[in] name Dot-separated path.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_dotset_null(JSON_Object *obj,
                                                              const char *name);

/**
 * @brief Sets a null value along a dot-separated path in a JSON_Object.
 *
 * @param[in] obj Target root object.
 * @param[in] name Dot-separated path.
 * @return JSONSuccess or JSONFailure.
 */
extern C_CDD_EXPORT JSON_Status json_object_dotset_null(JSON_Object *obj,
                                                        const char *name);

/**
 * @brief Removes and frees a key-value pair from a JSON_Object.
 *
 * @param[in] obj Target object.
 * @param[in] name Key to remove.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_remove(JSON_Object *obj,
                                                         const char *name);

/**
 * @brief Removes and frees a key-value pair from a JSON_Object.
 *
 * @param[in] obj Target object.
 * @param[in] name Key to remove.
 * @return JSONSuccess or JSONFailure.
 */
extern C_CDD_EXPORT JSON_Status json_object_remove(JSON_Object *obj,
                                                   const char *name);

/**
 * @brief Removes and frees a dot-separated key path from a JSON_Object.
 *
 * @param[in] obj Target root object.
 * @param[in] name Dot-separated path to remove.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_dotremove(JSON_Object *obj,
                                                            const char *name);

/**
 * @brief Removes and frees a dot-separated key path from a JSON_Object.
 *
 * @param[in] obj Target root object.
 * @param[in] name Dot-separated path to remove.
 * @return JSONSuccess or JSONFailure.
 */
extern C_CDD_EXPORT JSON_Status json_object_dotremove(JSON_Object *obj,
                                                      const char *name);

/**
 * @brief Removes and frees all key-value pairs in a JSON_Object.
 *
 * @param[in] obj Target object.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_object_clear(JSON_Object *obj);

/**
 * @brief Removes and frees all key-value pairs in a JSON_Object.
 *
 * @param[in] obj Target object.
 * @return JSONSuccess or JSONFailure.
 */
extern C_CDD_EXPORT JSON_Status json_object_clear(JSON_Object *obj);

/**
 * @brief Appends a JSON_Value to a JSON_Array.
 *
 * @param[in] arr Target array.
 * @param[in] val Value to append (ownership transferred on success).
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_array_append_value(JSON_Array *arr,
                                                              JSON_Value *val);

/**
 * @brief Appends a JSON_Value to a JSON_Array.
 *
 * @param[in] arr Target array.
 * @param[in] val Value to append (ownership transferred on success).
 * @return JSONSuccess or JSONFailure.
 */
extern C_CDD_EXPORT JSON_Status json_array_append_value(JSON_Array *arr,
                                                        JSON_Value *val);

/**
 * @brief Appends a string to a JSON_Array.
 *
 * @param[in] arr Target array.
 * @param[in] str String value to copy and append.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_array_append_string(JSON_Array *arr,
                                                               const char *str);

/**
 * @brief Appends a string to a JSON_Array.
 *
 * @param[in] arr Target array.
 * @param[in] str String value to copy and append.
 * @return JSONSuccess or JSONFailure.
 */
extern C_CDD_EXPORT JSON_Status json_array_append_string(JSON_Array *arr,
                                                         const char *str);

/**
 * @brief Appends a number to a JSON_Array.
 *
 * @param[in] arr Target array.
 * @param[in] number Numeric value to append.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_array_append_number(JSON_Array *arr,
                                                               double number);

/**
 * @brief Appends a number to a JSON_Array.
 *
 * @param[in] arr Target array.
 * @param[in] number Numeric value to append.
 * @return JSONSuccess or JSONFailure.
 */
extern C_CDD_EXPORT JSON_Status json_array_append_number(JSON_Array *arr,
                                                         double number);

/**
 * @brief Appends a boolean to a JSON_Array.
 *
 * @param[in] arr Target array.
 * @param[in] boolean Boolean value to append.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_array_append_boolean(JSON_Array *arr,
                                                                int boolean);

/**
 * @brief Appends a boolean to a JSON_Array.
 *
 * @param[in] arr Target array.
 * @param[in] boolean Boolean value to append.
 * @return JSONSuccess or JSONFailure.
 */
extern C_CDD_EXPORT JSON_Status json_array_append_boolean(JSON_Array *arr,
                                                          int boolean);

/**
 * @brief Appends a null value to a JSON_Array.
 *
 * @param[in] arr Target array.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_array_append_null(JSON_Array *arr);

/**
 * @brief Appends a null value to a JSON_Array.
 *
 * @param[in] arr Target array.
 * @return JSONSuccess or JSONFailure.
 */
extern C_CDD_EXPORT JSON_Status json_array_append_null(JSON_Array *arr);

/**
 * @brief Replaces an element at index i in a JSON_Array with a new value.
 *
 * @param[in] arr Target array.
 * @param[in] i Zero-based index.
 * @param[in] val Replacement value.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_array_replace_value(JSON_Array *arr,
                                                               size_t i,
                                                               JSON_Value *val);

/**
 * @brief Replaces an element at index i in a JSON_Array with a new value.
 *
 * @param[in] arr Target array.
 * @param[in] i Zero-based index.
 * @param[in] val Replacement value.
 * @return JSONSuccess or JSONFailure.
 */
extern C_CDD_EXPORT JSON_Status json_array_replace_value(JSON_Array *arr,
                                                         size_t i,
                                                         JSON_Value *val);

/**
 * @brief Replaces an element at index i in a JSON_Array with a string.
 *
 * @param[in] arr Target array.
 * @param[in] i Zero-based index.
 * @param[in] str Replacement string.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_array_replace_string(JSON_Array *arr, size_t i, const char *str);

/**
 * @brief Replaces an element at index i in a JSON_Array with a string.
 *
 * @param[in] arr Target array.
 * @param[in] i Zero-based index.
 * @param[in] str Replacement string.
 * @return JSONSuccess or JSONFailure.
 */
extern C_CDD_EXPORT JSON_Status json_array_replace_string(JSON_Array *arr,
                                                          size_t i,
                                                          const char *str);

/**
 * @brief Replaces an element at index i in a JSON_Array with a number.
 *
 * @param[in] arr Target array.
 * @param[in] i Zero-based index.
 * @param[in] number Replacement number.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_array_replace_number(JSON_Array *arr,
                                                                size_t i,
                                                                double number);

/**
 * @brief Replaces an element at index i in a JSON_Array with a number.
 *
 * @param[in] arr Target array.
 * @param[in] i Zero-based index.
 * @param[in] number Replacement number.
 * @return JSONSuccess or JSONFailure.
 */
extern C_CDD_EXPORT JSON_Status json_array_replace_number(JSON_Array *arr,
                                                          size_t i,
                                                          double number);

/**
 * @brief Replaces an element at index i in a JSON_Array with a boolean.
 *
 * @param[in] arr Target array.
 * @param[in] i Zero-based index.
 * @param[in] boolean Replacement boolean.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_array_replace_boolean(JSON_Array *arr, size_t i, int boolean);

/**
 * @brief Replaces an element at index i in a JSON_Array with a boolean.
 *
 * @param[in] arr Target array.
 * @param[in] i Zero-based index.
 * @param[in] boolean Replacement boolean.
 * @return JSONSuccess or JSONFailure.
 */
extern C_CDD_EXPORT JSON_Status json_array_replace_boolean(JSON_Array *arr,
                                                           size_t i,
                                                           int boolean);

/**
 * @brief Replaces an element at index i in a JSON_Array with null.
 *
 * @param[in] arr Target array.
 * @param[in] i Zero-based index.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_array_replace_null(JSON_Array *arr,
                                                              size_t i);

/**
 * @brief Replaces an element at index i in a JSON_Array with null.
 *
 * @param[in] arr Target array.
 * @param[in] i Zero-based index.
 * @return JSONSuccess or JSONFailure.
 */
extern C_CDD_EXPORT JSON_Status json_array_replace_null(JSON_Array *arr,
                                                        size_t i);

/**
 * @brief Inserts a value at index idx in a JSON_Array, shifting subsequent
 * elements to the right.
 *
 * @param[in,out] arr Target array.
 * @param[in] idx Zero-based insertion index (`0 <= idx <= count`).
 * @param[in] val Value to insert (ownership transferred on success).
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_array_insert_value(JSON_Array *arr,
                                                              size_t idx,
                                                              JSON_Value *val);

/**
 * @brief Removes and frees the element at index idx in a JSON_Array.
 *
 * @param[in] arr Target array.
 * @param[in] idx Zero-based index to remove.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_array_remove(JSON_Array *arr,
                                                        size_t idx);

/**
 * @brief Removes and frees the element at index idx in a JSON_Array.
 *
 * @param[in] arr Target array.
 * @param[in] idx Zero-based index to remove.
 * @return JSONSuccess or JSONFailure.
 */
extern C_CDD_EXPORT JSON_Status json_array_remove(JSON_Array *arr, size_t idx);

/**
 * @brief Removes and frees all elements in a JSON_Array.
 *
 * @param[in] arr Target array.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_array_clear(JSON_Array *arr);

/**
 * @brief Removes and frees all elements in a JSON_Array.
 *
 * @param[in] arr Target array.
 * @return JSONSuccess or JSONFailure.
 */
extern C_CDD_EXPORT JSON_Status json_array_clear(JSON_Array *arr);

/**
 * @brief Serializes a JSON_Value tree to a compact heap-allocated string.
 *
 * @param[in] val Root value to serialize.
 * @param[out] out Output pointer receiving the serialized string.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_serialize_to_string(const JSON_Value *val, char **out);

/**
 * @brief Serializes a JSON_Value tree to a pretty-printed heap-allocated
 * string.
 *
 * @param[in] val Root value to serialize.
 * @param[out] out Output pointer receiving the serialized string.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_serialize_to_string_pretty(const JSON_Value *val, char **out);

/**
 * @brief Serializes a JSON_Value tree to a canonical (RFC 8785 JCS) compact
 * heap-allocated string with lexicographically sorted object keys.
 *
 * @param[in] val Root value to serialize.
 * @param[out] out Output pointer receiving the canonical serialized string.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_serialize_to_string_canonical(const JSON_Value *val, char **out);

/**
 * @brief Computes the required buffer size (including NUL terminator) for
 * serializing a JSON_Value.
 *
 * @param[in] val Root value to measure.
 * @param[in] pretty Non-zero for pretty-printed size, 0 for compact.
 * @param[out] out_size Output pointer receiving required buffer size in bytes.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_serialization_size(
    const JSON_Value *val, int pretty, size_t *out_size);

/**
 * @brief Serializes a JSON_Value into a caller-provided buffer.
 *
 * @param[in] val Root value to serialize.
 * @param[out] buf Destination character buffer.
 * @param[in] buf_size Capacity of buf in bytes.
 * @param[in] pretty Non-zero for pretty-printing, 0 for compact.
 * @return CDD_C_SUCCESS on success, or error code if buffer is too small.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_serialize_to_buffer(
    const JSON_Value *val, char *buf, size_t buf_size, int pretty);

/**
 * @brief Serializes a JSON_Value tree directly to a `FILE *` stream without
 * allocating a secondary full-document copy.
 *
 * @param[in] val Root value to serialize.
 * @param[in,out] fp Open destination file stream.
 * @param[in] pretty Non-zero for pretty-printing, 0 for compact.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_serialize_to_fp(const JSON_Value *val, FILE *fp, int pretty);

/**
 * @brief Serializes a JSON_Value tree to a pretty-printed file.
 *
 * @param[in] val Root value to serialize.
 * @param[in] filename Destination file path.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_serialize_to_file_pretty(const JSON_Value *val, const char *filename);

/**
 * @brief Serializes a JSON_Value tree to a pretty-printed file.
 *
 * @param[in] val Root value to serialize.
 * @param[in] filename Destination file path.
 * @return JSONSuccess or JSONFailure.
 */
extern C_CDD_EXPORT JSON_Status
json_serialize_to_file_pretty(const JSON_Value *val, const char *filename);

/**
 * @brief Parses a single newline-delimited JSON (NDJSON / JSON Lines) record,
 * ignoring optional trailing `\r` or `\n`.
 *
 * @param[in] line Null-terminated NDJSON line.
 * @param[out] out Output pointer receiving the parsed `JSON_Value *`.
 * @return CDD_C_SUCCESS on success, or error code on empty/invalid line.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_ndjson_parse_line(const char *line,
                                                             JSON_Value **out);

/**
 * @brief Parses a multi-line NDJSON (JSON Lines) document into a `JSONArray`
 * value, skipping blank lines.
 *
 * @param[in] ndjson_str Null-terminated multi-line NDJSON string.
 * @param[out] out_array Output pointer receiving the `JSONArray` value.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_ndjson_parse_string(const char *ndjson_str, JSON_Value **out_array);

/**
 * @brief Serializes a `JSON_Array` into a newline-delimited JSON (NDJSON)
 * string where each element is a compact JSON line terminated by `\n`.
 *
 * @param[in] arr Source JSON_Array to serialize.
 * @param[out] out_ndjson Output pointer receiving the heap-allocated NDJSON
 * string.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_json_ndjson_serialize_array(const JSON_Array *arr, char **out_ndjson);

/* --- RFC 6901 JSON Pointer, RFC 7396 Merge Patch & RFC 6902 JSON Patch --- */

/**
 * @brief Unescapes a single RFC 6901 JSON Pointer reference token (`~1` -> `/`,
 * `~0` -> `~`).
 *
 * @param[in] in Token substring.
 * @param[in] len Byte length of token substring.
 * @param[out] out_token Newly allocated unescaped token string.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_pointer_unescape(const char *in,
                                                            size_t len,
                                                            char **out_token);

/**
 * @brief Escapes a single raw string token into an RFC 6901 JSON Pointer
 * reference token (`~` -> `~0`, `/` -> `~1`).
 *
 * @param[in] token Raw unescaped token string.
 * @param[out] out_buf Destination buffer for escaped token.
 * @param[in] buf_sz Capacity of out_buf in bytes.
 * @return CDD_C_SUCCESS on success, or error code if buffer is too small.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_pointer_escape(const char *token,
                                                          char *out_buf,
                                                          size_t buf_sz);

/**
 * @brief Evaluates an RFC 6901 JSON Pointer against a root JSON_Value.
 *
 * @param[in] root Root JSON_Value document.
 * @param[in] pointer JSON Pointer string (e.g. `""`, `"/a/b/0"`, `"#/a/b"`).
 * @param[out] out_val Output pointer receiving the resolved JSON_Value.
 * @return CDD_C_SUCCESS if found, CDD_C_ERROR_NOT_FOUND if missing, or error
 * code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_pointer_get(
    const JSON_Value *root, const char *pointer, const JSON_Value **out_val);

/**
 * @brief Sets or inserts a JSON_Value at an RFC 6901 JSON Pointer path within
 * a mutable root JSON_Value document.
 *
 * @param[in,out] root Mutable root JSON_Value container.
 * @param[in] pointer Non-empty RFC 6901 JSON Pointer path (e.g. `"/a/b"`).
 * @param[in] val Value to set (ownership transferred on success).
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_pointer_set(JSON_Value *root,
                                                       const char *pointer,
                                                       JSON_Value *val);

/**
 * @brief Removes and frees the value at an RFC 6901 JSON Pointer path within
 * a mutable root JSON_Value document.
 *
 * @param[in,out] root Mutable root JSON_Value container.
 * @param[in] pointer Non-empty RFC 6901 JSON Pointer path (e.g. `"/a/b"`).
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_pointer_remove(JSON_Value *root,
                                                          const char *pointer);

/**
 * @brief Evaluates an RFC 9535 JSONPath expression against a root JSON_Value.
 *
 * Supports dot selectors (`$.a.b`), wildcard selectors (`$.*`, `$[*]`),
 * bracketed string keys (`$['a']`, `$["a"]`), non-negative and negative array
 * indices (`$[0]`, `$[-1]`), array slice selectors (`[start:end:step]`), and
 * basic filter selectors (`[?(@.key == val)]`).
 *
 * @param[in] root Root JSON_Value document.
 * @param[in] jsonpath RFC 9535 JSONPath query string starting with `$`.
 * @param[out] out_val Output pointer receiving the matched JSON_Value.
 * @return CDD_C_SUCCESS on match, CDD_C_ERROR_NOT_FOUND if not matched,
 * CDD_C_ERROR_NOT_SUPPORTED for unsupported filter syntax, or
 * CDD_C_ERROR_INVALID_ARGUMENT if malformed.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_jsonpath_get(
    const JSON_Value *root, const char *jsonpath, const JSON_Value **out_val);

/**
 * @brief Evaluates an RFC 9535 JSONPath expression against a root JSON_Value
 * and returns a newly allocated `JSONArray` containing deep copies of all
 * matched nodes (nodelist).
 *
 * Supports dot selectors (`$.a.b`), recursive descent (`$..a`, `$..*`),
 * wildcard selectors (`$.*`, `$[*]`), bracketed string keys (`$['a']`,
 * `$["a"]`), union selectors (`$['a','b']`, `$[0,2]`), non-negative and
 * negative array indices (`$[0]`, `$[-1]`), array slice selectors
 * (`[start:end:step]`), and filter selectors (`[?(@.key == val)]`).
 *
 * @param[in] root Root JSON_Value document.
 * @param[in] jsonpath RFC 9535 JSONPath query string starting with `$`.
 * @param[out] out_array Output pointer receiving the newly allocated JSONArray
 * value.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_jsonpath_query(
    const JSON_Value *root, const char *jsonpath, JSON_Value **out_array);

/**
 * @brief Applies an RFC 7396 JSON Merge Patch to a target JSON_Value, producing
 * a newly allocated patched JSON_Value.
 *
 * @param[in] target Original JSON_Value (may be NULL).
 * @param[in] patch Merge patch JSON_Value.
 * @param[out] out Output pointer receiving the newly allocated patched tree.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_merge_patch(const JSON_Value *target,
                                                       const JSON_Value *patch,
                                                       JSON_Value **out);

/**
 * @brief Applies an RFC 6902 JSON Patch array of operations (`add`, `remove`,
 * `replace`, `move`, `copy`, `test`) to a document, producing a newly allocated
 * patched JSON_Value.
 *
 * @param[in] doc Original document JSON_Value.
 * @param[in] patch_arr JSON_Value of type JSONArray containing patch ops.
 * @param[out] out Output pointer receiving the newly allocated patched tree.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_json_apply_patch(
    const JSON_Value *doc, const JSON_Value *patch_arr, JSON_Value **out);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_JSON_DOM_H */
