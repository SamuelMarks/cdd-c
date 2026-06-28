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
#if !defined(_MSC_VER) || _MSC_VER >= 1800
/* clang-format off */
#if defined(_MSC_VER) && _MSC_VER < 1800
#include "msvc/stdbool.h"
#include "cdd_c_error.h"
#else
#include <stdbool.h>
#endif
#endif
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

extern SIMPLE_MOCKS_EXPORT /**
                            * @brief Executes the Tank default operation.
                            */
    enum cdd_c_error
    Tank_default(enum Tank *out);

extern SIMPLE_MOCKS_EXPORT /**
                            * @brief Executes the Tank to str operation.
                            */
    enum cdd_c_error
    Tank_to_str(enum Tank, char **);

extern SIMPLE_MOCKS_EXPORT /**
                            * @brief Executes the Tank from str operation.
                            */
    enum cdd_c_error
    Tank_from_str(const char *, enum Tank *);

struct SIMPLE_MOCKS_EXPORT HazE {
  /** @brief bzr */
  const char *bzr;
  /** @brief tank */
  enum Tank tank;
};

extern SIMPLE_MOCKS_EXPORT /**
                            * @brief Executes the FooE cleanup operation.
                            */
    void
    FooE_cleanup(struct FooE *);

extern SIMPLE_MOCKS_EXPORT /**
                            * @brief Executes the FooE default operation.
                            */
    enum cdd_c_error
    FooE_default(struct FooE **);

extern SIMPLE_MOCKS_EXPORT /**
                            * @brief Executes the FooE deepcopy operation.
                            */
    enum cdd_c_error
    FooE_deepcopy(const struct FooE *, struct FooE **);

extern SIMPLE_MOCKS_EXPORT /**
                            * @brief Executes the FooE display operation.
                            */
    enum cdd_c_error
    FooE_display(const struct FooE *, FILE *);

extern SIMPLE_MOCKS_EXPORT /**
                            * @brief Executes the FooE debug operation.
                            */
    enum cdd_c_error
    FooE_debug(const struct FooE *, FILE *);

extern SIMPLE_MOCKS_EXPORT /**
                            * @brief Executes the FooE eq operation.
                            */
    enum cdd_c_error
    FooE_eq(const struct FooE *, const struct FooE *);

extern SIMPLE_MOCKS_EXPORT /**
                            * @brief Executes the FooE to json operation.
                            */
    enum cdd_c_error
    FooE_to_json(const struct FooE *, char **);

extern SIMPLE_MOCKS_EXPORT /**
                            * @brief Executes the FooE from jsonObject
                            * operation.
                            */
    enum cdd_c_error
    FooE_from_jsonObject(const JSON_Object *, struct FooE **);

extern SIMPLE_MOCKS_EXPORT /**
                            * @brief Executes the FooE from json operation.
                            */
    enum cdd_c_error
    FooE_from_json(const char *, struct FooE **);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SIMPLE_JSON_H */
