/**
 * @file simple_json.h
 * @brief Unit tests and mocks in simple_json.h.
 */
#ifndef SIMPLE_JSON_H
#define SIMPLE_JSON_H

/* clang-format off */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L &&                \
    (!defined(_MSC_VER) || _MSC_VER >= 1800)
#include <stdbool.h>
#else
#include <c_cdd_stdbool.h>
#endif
#include "cdd_c_error.h"
#include <stdio.h>
#include "c_cdd/json_dom.h"
#include "simple_mocks_export.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @brief Test definition Tank. */
enum Tank { Tank_BIG, Tank_SMALL, Tank_UNKNOWN = -1 };

extern SIMPLE_MOCKS_EXPORT cdd_c_error_t Tank_default(enum Tank *out);

extern SIMPLE_MOCKS_EXPORT cdd_c_error_t Tank_to_str(enum Tank tank,
                                                     char **str);

extern SIMPLE_MOCKS_EXPORT cdd_c_error_t Tank_from_str(const char *str,
                                                       enum Tank *val);

/** \brief mock */
struct HazE {
  char *bzr;
  enum Tank tank;
};

extern SIMPLE_MOCKS_EXPORT cdd_c_error_t HazE_cleanup(struct HazE *);

extern SIMPLE_MOCKS_EXPORT cdd_c_error_t HazE_default(struct HazE **);

extern SIMPLE_MOCKS_EXPORT cdd_c_error_t HazE_deepcopy(const struct HazE *,
                                                       struct HazE **);

extern SIMPLE_MOCKS_EXPORT cdd_c_error_t HazE_display(const struct HazE *,
                                                      FILE *);

extern SIMPLE_MOCKS_EXPORT cdd_c_error_t HazE_debug(const struct HazE *,
                                                    FILE *);

extern SIMPLE_MOCKS_EXPORT cdd_c_error_t HazE_eq(const struct HazE *,
                                                 const struct HazE *);

extern SIMPLE_MOCKS_EXPORT cdd_c_error_t HazE_to_json(const struct HazE *,
                                                      char **);

extern SIMPLE_MOCKS_EXPORT cdd_c_error_t HazE_from_json(const char *,
                                                        struct HazE **);

extern SIMPLE_MOCKS_EXPORT cdd_c_error_t
HazE_from_jsonObject(const JSON_Object *, struct HazE **);

/** @brief Test definition FooE. */
struct FooE {
  char *bar;
  int can;
  struct HazE *haz;
};

extern SIMPLE_MOCKS_EXPORT cdd_c_error_t FooE_cleanup(struct FooE *);

extern SIMPLE_MOCKS_EXPORT cdd_c_error_t FooE_default(struct FooE **);

extern SIMPLE_MOCKS_EXPORT cdd_c_error_t FooE_deepcopy(const struct FooE *,
                                                       struct FooE **);

extern SIMPLE_MOCKS_EXPORT cdd_c_error_t FooE_display(const struct FooE *,
                                                      FILE *);

extern SIMPLE_MOCKS_EXPORT cdd_c_error_t FooE_debug(const struct FooE *,
                                                    FILE *);

extern SIMPLE_MOCKS_EXPORT cdd_c_error_t FooE_eq(const struct FooE *,
                                                 const struct FooE *);

extern SIMPLE_MOCKS_EXPORT cdd_c_error_t FooE_to_json(const struct FooE *,
                                                      char **);

extern SIMPLE_MOCKS_EXPORT cdd_c_error_t
FooE_from_jsonObject(const JSON_Object *, struct FooE **);

extern SIMPLE_MOCKS_EXPORT cdd_c_error_t FooE_from_json(const char *,
                                                        struct FooE **);

/**
 * @brief Helper function run_mocks_test.
 * @return Result status or value.
 */
extern SIMPLE_MOCKS_EXPORT cdd_c_error_t run_mocks_test(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SIMPLE_JSON_H */
