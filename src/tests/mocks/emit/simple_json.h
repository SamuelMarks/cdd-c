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
/* clang-format off */
#if defined(_MSC_VER) && _MSC_VER < 1800
#include "msvc/stdbool.h"
#else
#include <stdbool.h>
#endif
#include "cdd_c_error.h"
/* clang-format on */
#endif
#endif
#else
#include <c_cdd_stdbool.h>
#endif /* __STDC_VERSION__ >= 199901L */

#include <stdio.h>

#include <parson.h>

#include "simple_mocks_export.h"
/* clang-format on */

enum Tank { Tank_BIG, Tank_SMALL, Tank_UNKNOWN = -1 };

extern SIMPLE_MOCKS_EXPORT enum cdd_c_error Tank_default(enum Tank *out);

extern SIMPLE_MOCKS_EXPORT enum cdd_c_error Tank_to_str(enum Tank, char **);

extern SIMPLE_MOCKS_EXPORT enum cdd_c_error Tank_from_str(const char *,
                                                          enum Tank *);

/** \brief mock */
struct HazE {
  const char *bzr;
  enum Tank tank;
};

extern SIMPLE_MOCKS_EXPORT enum cdd_c_error HazE_cleanup(struct HazE *);

extern SIMPLE_MOCKS_EXPORT enum cdd_c_error HazE_default(struct HazE **);

extern SIMPLE_MOCKS_EXPORT enum cdd_c_error HazE_deepcopy(const struct HazE *,
                                                          struct HazE **);

extern SIMPLE_MOCKS_EXPORT enum cdd_c_error HazE_display(const struct HazE *,
                                                         FILE *);

extern SIMPLE_MOCKS_EXPORT enum cdd_c_error HazE_debug(const struct HazE *,
                                                       FILE *);

extern SIMPLE_MOCKS_EXPORT enum cdd_c_error HazE_eq(const struct HazE *,
                                                    const struct HazE *);

extern SIMPLE_MOCKS_EXPORT enum cdd_c_error HazE_to_json(const struct HazE *,
                                                         char **);

extern SIMPLE_MOCKS_EXPORT enum cdd_c_error HazE_from_json(const char *,
                                                           struct HazE **);

extern SIMPLE_MOCKS_EXPORT enum cdd_c_error
HazE_from_jsonObject(const JSON_Object *, struct HazE **);

struct FooE {
  const char *bar;
  int can;
  struct HazE *haz;
};

extern SIMPLE_MOCKS_EXPORT enum cdd_c_error FooE_cleanup(struct FooE *);

extern SIMPLE_MOCKS_EXPORT enum cdd_c_error FooE_default(struct FooE **);

extern SIMPLE_MOCKS_EXPORT enum cdd_c_error FooE_deepcopy(const struct FooE *,
                                                          struct FooE **);

extern SIMPLE_MOCKS_EXPORT enum cdd_c_error FooE_display(const struct FooE *,
                                                         FILE *);

extern SIMPLE_MOCKS_EXPORT enum cdd_c_error FooE_debug(const struct FooE *,
                                                       FILE *);

extern SIMPLE_MOCKS_EXPORT enum cdd_c_error FooE_eq(const struct FooE *,
                                                    const struct FooE *);

extern SIMPLE_MOCKS_EXPORT enum cdd_c_error FooE_to_json(const struct FooE *,
                                                         char **);

extern SIMPLE_MOCKS_EXPORT enum cdd_c_error
FooE_from_jsonObject(const JSON_Object *, struct FooE **);

extern SIMPLE_MOCKS_EXPORT enum cdd_c_error FooE_from_json(const char *,
                                                           struct FooE **);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SIMPLE_JSON_H */
