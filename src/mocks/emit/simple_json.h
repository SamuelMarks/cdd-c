#ifndef SIMPLE_JSON_H
#define SIMPLE_JSON_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

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
/* clang-format off */
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

#include <parson.h>

#include "simple_mocks_export.h"
/* clang-format on */

enum SIMPLE_MOCKS_EXPORT Tank { Tank_BIG, Tank_SMALL, Tank_UNKNOWN = -1 };

/**
 * @brief Initializes a Tank instance with default values.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error Tank_default(enum Tank *out);

/**
 * @brief Converts a Tank enum value to its string representation.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error Tank_to_str(enum Tank, char **);

/**
 * @brief Parses a Tank enum value from its string representation.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error Tank_from_str(const char *,
                                                          enum Tank *);

struct SIMPLE_MOCKS_EXPORT HazE {
  /** @brief bzr */
  char *bzr;
  /** @brief tank */
  enum Tank tank;
};

struct SIMPLE_MOCKS_EXPORT FooE {
  /** @brief bar */
  char *bar;
  /** @brief can */
  int can;
  /** @brief haz */
  struct HazE *haz;
};

/**
 * @brief Releases memory owned by a FooE instance.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error FooE_cleanup(struct FooE *);

/**
 * @brief Initializes a FooE instance with default values.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error FooE_default(struct FooE **);

/**
 * @brief Performs a deep copy of a FooE instance.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error FooE_deepcopy(const struct FooE *,
                                                          struct FooE **);

/**
 * @brief Formats a human-readable representation of FooE to a stream.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error FooE_display(const struct FooE *,
                                                         FILE *);

/**
 * @brief Formats a debug representation of FooE to a stream.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error FooE_debug(const struct FooE *,
                                                       FILE *);

/**
 * @brief Compares two FooE values for equality.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error FooE_eq(const struct FooE *,
                                                    const struct FooE *);

/**
 * @brief Serializes a FooE instance to a JSON string.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error FooE_to_json(const struct FooE *,
                                                         char **);

/**
 * @brief Performs FooE from jsonObject.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error
FooE_from_jsonObject(const JSON_Object *, struct FooE **);

/**
 * @brief Deserializes a FooE instance from a JSON string.
 */
extern SIMPLE_MOCKS_EXPORT enum cdd_c_error FooE_from_json(const char *,
                                                           struct FooE **);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SIMPLE_JSON_H */
