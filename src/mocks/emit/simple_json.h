#ifndef SIMPLE_JSON_H
#define SIMPLE_JSON_H

#ifdef __cplusplus
extern "C" {
#else
#if __STDC_VERSION__ >= 199901L
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
#if !defined(_MSC_VER) || _MSC_VER >= 1800
#include <stdbool.h>
#endif
#endif
#endif
#else
#include <c_cdd_stdbool.h>
#endif /* __STDC_VERSION__ >= 199901L */
#endif /* __cplusplus */

#include <stdio.h>

#include <parson.h>

#include "simple_mocks_export.h"
/* clang-format on */

enum SIMPLE_MOCKS_EXPORT Tank { Tank_BIG, Tank_SMALL, Tank_UNKNOWN = -1 };

extern SIMPLE_MOCKS_EXPORT /**
                            * @brief Executes the Tank default operation.
                            */
    int
    Tank_default(enum Tank *out);

extern SIMPLE_MOCKS_EXPORT /**
                            * @brief Executes the Tank to str operation.
                            */
    int
    Tank_to_str(enum Tank, char **);

extern SIMPLE_MOCKS_EXPORT /**
                            * @brief Executes the Tank from str operation.
                            */
    int
    Tank_from_str(const char *, enum Tank *);

struct SIMPLE_MOCKS_EXPORT HazE {
  /** @brief haz */
  struct HazE *haz;
};

extern SIMPLE_MOCKS_EXPORT /**
                            * @brief Executes the FooE cleanup operation.
                            */
    void
    FooE_cleanup(struct FooE *);

extern SIMPLE_MOCKS_EXPORT /**
                            * @brief Executes the FooE default operation.
                            */
    int
    FooE_default(struct FooE **);

extern SIMPLE_MOCKS_EXPORT /**
                            * @brief Executes the FooE deepcopy operation.
                            */
    int
    FooE_deepcopy(const struct FooE *, struct FooE **);

extern SIMPLE_MOCKS_EXPORT /**
                            * @brief Executes the FooE display operation.
                            */
    int
    FooE_display(const struct FooE *, FILE *);

extern SIMPLE_MOCKS_EXPORT /**
                            * @brief Executes the FooE debug operation.
                            */
    int
    FooE_debug(const struct FooE *, FILE *);

extern SIMPLE_MOCKS_EXPORT /**
                            * @brief Executes the FooE eq operation.
                            */
    int
    FooE_eq(const struct FooE *, const struct FooE *);

extern SIMPLE_MOCKS_EXPORT /**
                            * @brief Executes the FooE to json operation.
                            */
    int
    FooE_to_json(const struct FooE *, char **);

extern SIMPLE_MOCKS_EXPORT /**
                            * @brief Executes the FooE from jsonObject
                            * operation.
                            */
    int
    FooE_from_jsonObject(const JSON_Object *, struct FooE **);

extern SIMPLE_MOCKS_EXPORT /**
                            * @brief Executes the FooE from json operation.
                            */
    int
    FooE_from_json(const char *, struct FooE **);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SIMPLE_JSON_H */
