/**
 * @file parson.h
 * @brief Zero-dependency C89 JSON DOM header backed by c_cdd/json_dom.h.
 */

#ifndef CDD_NATIVE_PARSON_H
#define CDD_NATIVE_PARSON_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd/json_dom.h"
/* clang-format on */

/** @brief Retrieves the JSON_Object from a JSON_Value. */
#define json_value_get_object(v)                                               \
  ((v) && ((const JSON_Value *)(v))->type == JSONObject                        \
       ? ((const JSON_Value *)(v))->object_val                                 \
       : (JSON_Object *)0)

/** @brief Retrieves the JSON_Array from a JSON_Value. */
#define json_value_get_array(v)                                                \
  ((v) && ((const JSON_Value *)(v))->type == JSONArray                         \
       ? ((const JSON_Value *)(v))->array_val                                  \
       : (JSON_Array *)0)

/** @brief Retrieves the C string from a JSON_Value. */
#define json_value_get_string(v)                                               \
  ((v) && ((const JSON_Value *)(v))->type == JSONString                        \
       ? (const char *)((const JSON_Value *)(v))->string_val                   \
       : (const char *)0)

/** @brief Retrieves the numeric value from a JSON_Value. */
#define json_value_get_number(v)                                               \
  ((v) && ((const JSON_Value *)(v))->type == JSONNumber                        \
       ? ((const JSON_Value *)(v))->number_val                                 \
       : 0.0)

/** @brief Returns the number of entries in a JSON_Object. */
#define json_object_get_count(o)                                               \
  ((o) ? ((const JSON_Object *)(o))->count : (size_t)0)

/** @brief Returns the key name at index i in a JSON_Object. */
#define json_object_get_name(o, i)                                             \
  ((o) && (size_t)(i) < ((const JSON_Object *)(o))->count                      \
       ? (const char *)((const JSON_Object *)(o))->names[(size_t)(i)]          \
       : (const char *)0)

/** @brief Returns the child JSON_Value at index i in a JSON_Object. */
#define json_object_get_value_at(o, i)                                         \
  ((o) && (size_t)(i) < ((const JSON_Object *)(o))->count                      \
       ? ((const JSON_Object *)(o))->values[(size_t)(i)]                       \
       : (JSON_Value *)0)

/** @brief Returns the owning JSON_Value of a JSON_Object. */
#define json_object_get_wrapping_value(o)                                      \
  ((o) ? ((const JSON_Object *)(o))->wrapping_value : (JSON_Value *)0)

/** @brief Returns the number of elements in a JSON_Array. */
#define json_array_get_count(a)                                                \
  ((a) ? ((const JSON_Array *)(a))->count : (size_t)0)

/** @brief Returns the JSON_Value at index i in a JSON_Array. */
#define json_array_get_value(a, i)                                             \
  ((a) && (size_t)(i) < ((const JSON_Array *)(a))->count                       \
       ? ((const JSON_Array *)(a))->items[(size_t)(i)]                         \
       : (JSON_Value *)0)

/** @brief Returns the string at index i in a JSON_Array. */
#define json_array_get_string(a, i)                                            \
  json_value_get_string(json_array_get_value((a), (i)))

/** @brief Returns the JSON_Object at index i in a JSON_Array. */
#define json_array_get_object(a, i)                                            \
  json_value_get_object(json_array_get_value((a), (i)))

/** @brief Returns the JSON_Array at index i in a JSON_Array. */
#define json_array_get_array(a, i)                                             \
  json_value_get_array(json_array_get_value((a), (i)))

/** @brief Returns the number at index i in a JSON_Array. */
#define json_array_get_number(a, i)                                            \
  json_value_get_number(json_array_get_value((a), (i)))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_NATIVE_PARSON_H */
