/**
 * @file simple_json.h
 * @brief Unit tests and mocks in simple_json.h.
 */
#ifndef SIMPLE_JSON_H
#define SIMPLE_JSON_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#if defined(_MSC_VER) && _MSC_VER < 1800
#if !defined(__cplusplus)
#ifndef bool
#define bool unsigned char
#endif
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif
#endif
#else
#if defined(_MSC_VER) && _MSC_VER < 1800
#ifndef __cplusplus
typedef unsigned char bool;
#define true 1
#define false 0
#endif
#else
#if defined(_MSC_VER) && _MSC_VER < 1800
#include "msvc/stdbool.h"
#else
#if defined(_MSC_VER) && _MSC_VER < 1800
#include "c_cdd/msvc/stdbool.h"
#else
#include <stdbool.h>
#endif
#endif
#include "cdd_c_error.h"
#endif
#endif
#else
#include <c_cdd_stdbool.h>
#endif /* __STDC_VERSION__ >= 199901L */

#include <stdio.h>

#include "c_cdd/json_dom.h"

#include "simple_mocks_export.h"
/* clang-format on */

/** @brief Test definition Tank. */
enum Tank {
  /** @brief Big tank. */
  Tank_BIG,
  /** @brief Small tank. */
  Tank_SMALL,
  /** @brief Unknown tank. */
  Tank_UNKNOWN = -1
};

/**
 * @brief Initializes a Tank instance with default values.
 * @param[out] out Parameter out.
 * @return Result status or value.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error Tank_default(enum Tank *out);

/**
 * @brief Converts a Tank enum value to its string representation.
 * @param[in] tank Parameter tank.
 * @param[out] str Parameter str.
 * @return Result status or value.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error Tank_to_str(enum Tank tank,
                                                        char **str);

/**
 * @brief Parses a Tank enum value from its string representation.
 * @param[in] str Parameter str.
 * @param[out] val Parameter val.
 * @return Result status or value.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error Tank_from_str(const char *str,
                                                          enum Tank *val);

/** @brief Test definition HazE. */
struct HazE {
  /** @brief bzr */
  char *bzr;
  /** @brief tank */
  enum Tank tank;
};

/**
 * @brief Releases memory owned by a HazE instance.
 * @param[in,out] haz_e Parameter haz_e.
 * @return Result status or value.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error HazE_cleanup(struct HazE *haz_e);

/**
 * @brief Initializes a HazE instance with default values.
 * @param[out] haz_e Parameter haz_e.
 * @return Result status or value.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error HazE_default(struct HazE **haz_e);

/**
 * @brief Performs a deep copy of a HazE instance.
 * @param[in] haz_e_original Parameter haz_e_original.
 * @param[out] haz_e_dest Parameter haz_e_dest.
 * @return Result status or value.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error
HazE_deepcopy(const struct HazE *haz_e_original, struct HazE **haz_e_dest);

/**
 * @brief Formats a human-readable representation of HazE to a stream.
 * @param[in] haz_e Parameter haz_e.
 * @param[in,out] fh Parameter fh.
 * @return Result status or value.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error
HazE_display(const struct HazE *haz_e, FILE *fh);

/**
 * @brief Formats a debug representation of HazE to a stream.
 * @param[in] haz_e Parameter haz_e.
 * @param[in,out] fh Parameter fh.
 * @return Result status or value.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error HazE_debug(const struct HazE *haz_e,
                                                       FILE *fh);

/**
 * @brief Compares two HazE values for equality.
 * @param[in] haz_e0 Parameter haz_e0.
 * @param[in] haz_e1 Parameter haz_e1.
 * @return Result status or value.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error HazE_eq(const struct HazE *haz_e0,
                                                    const struct HazE *haz_e1);

/**
 * @brief Serializes a HazE instance to a JSON string.
 * @param[in] haz_e Parameter haz_e.
 * @param[out] json Parameter json.
 * @return Result status or value.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error
HazE_to_json(const struct HazE *haz_e, char **json);

/**
 * @brief Deserializes a HazE instance from a JSON_Object.
 * @param[in] jsonObject Parameter jsonObject.
 * @param[out] haz_e Parameter haz_e.
 * @return Result status or value.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error
HazE_from_jsonObject(const JSON_Object *jsonObject, struct HazE **haz_e);

/**
 * @brief Deserializes a HazE instance from a JSON string.
 * @param[in] json Parameter json.
 * @param[out] haz_e Parameter haz_e.
 * @return Result status or value.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error HazE_from_json(const char *json,
                                                           struct HazE **haz_e);

/** @brief Test definition FooE. */
struct FooE {
  /** @brief bar */
  char *bar;
  /** @brief can */
  int can;
  /** @brief haz */
  struct HazE *haz;
};

/**
 * @brief Releases memory owned by a FooE instance.
 * @param[in,out] foo_e Parameter foo_e.
 * @return Result status or value.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error FooE_cleanup(struct FooE *foo_e);

/**
 * @brief Initializes a FooE instance with default values.
 * @param[out] foo_e Parameter foo_e.
 * @return Result status or value.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error FooE_default(struct FooE **foo_e);

/**
 * @brief Performs a deep copy of a FooE instance.
 * @param[in] foo_e_original Parameter foo_e_original.
 * @param[out] foo_e_dest Parameter foo_e_dest.
 * @return Result status or value.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error
FooE_deepcopy(const struct FooE *foo_e_original, struct FooE **foo_e_dest);

/**
 * @brief Formats a human-readable representation of FooE to a stream.
 * @param[in] foo_e Parameter foo_e.
 * @param[in,out] fh Parameter fh.
 * @return Result status or value.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error
FooE_display(const struct FooE *foo_e, FILE *fh);

/**
 * @brief Formats a debug representation of FooE to a stream.
 * @param[in] foo_e Parameter foo_e.
 * @param[in,out] fh Parameter fh.
 * @return Result status or value.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error FooE_debug(const struct FooE *foo_e,
                                                       FILE *fh);

/**
 * @brief Compares two FooE values for equality.
 * @param[in] foo_e0 Parameter foo_e0.
 * @param[in] foo_e1 Parameter foo_e1.
 * @return Result status or value.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error FooE_eq(const struct FooE *foo_e0,
                                                    const struct FooE *foo_e1);

/**
 * @brief Serializes a FooE instance to a JSON string.
 * @param[in] foo_e Parameter foo_e.
 * @param[out] json Parameter json.
 * @return Result status or value.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error
FooE_to_json(const struct FooE *foo_e, char **json);

/**
 * @brief Performs FooE from jsonObject.
 * @param[in] jsonObject Parameter jsonObject.
 * @param[out] foo_e Parameter foo_e.
 * @return Result status or value.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error
FooE_from_jsonObject(const JSON_Object *jsonObject, struct FooE **foo_e);

/**
 * @brief Deserializes a FooE instance from a JSON string.
 * @param[in] json Parameter json.
 * @param[out] foo_e Parameter foo_e.
 * @return Result status or value.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error FooE_from_json(const char *json,
                                                           struct FooE **foo_e);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SIMPLE_JSON_H */
