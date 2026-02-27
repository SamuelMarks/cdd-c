#ifndef SIMPLE_JSON_H
#define SIMPLE_JSON_H

#ifdef __cplusplus
extern "C" {
#else
#if __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#else
#include <c_cdd_stdbool.h>
#endif /* __STDC_VERSION__ >= 199901L */
#endif /* __cplusplus */

#include <stdio.h>

#include <parson.h>

#include "simple_mocks_export.h"

enum SIMPLE_MOCKS_EXPORT Tank { Tank_BIG, Tank_SMALL, Tank_UNKNOWN = -1 };

extern SIMPLE_MOCKS_EXPORT enum Tank Tank_default(void);

extern SIMPLE_MOCKS_EXPORT int Tank_to_str(enum Tank, char **);

extern SIMPLE_MOCKS_EXPORT int Tank_from_str(const char *, enum Tank *);

struct SIMPLE_MOCKS_EXPORT HazE {
  const char *bzr;
  enum Tank tank;
};

extern SIMPLE_MOCKS_EXPORT void HazE_cleanup(struct HazE *);

extern SIMPLE_MOCKS_EXPORT int HazE_default(struct HazE **);

extern SIMPLE_MOCKS_EXPORT int HazE_deepcopy(const struct HazE *,
                                             struct HazE **);

extern SIMPLE_MOCKS_EXPORT int HazE_display(const struct HazE *, FILE *);

extern SIMPLE_MOCKS_EXPORT int HazE_debug(const struct HazE *, FILE *);

extern SIMPLE_MOCKS_EXPORT bool HazE_eq(const struct HazE *,
                                        const struct HazE *);

extern SIMPLE_MOCKS_EXPORT int HazE_to_json(const struct HazE *, char **);

extern SIMPLE_MOCKS_EXPORT int HazE_from_json(const char *, struct HazE **);

extern SIMPLE_MOCKS_EXPORT int HazE_from_jsonObject(const JSON_Object *,
                                                    struct HazE **);

struct SIMPLE_MOCKS_EXPORT FooE {
  const char *bar;
  int can;
  struct HazE *haz;
};

extern SIMPLE_MOCKS_EXPORT void FooE_cleanup(struct FooE *);

extern SIMPLE_MOCKS_EXPORT int FooE_default(struct FooE **);

extern SIMPLE_MOCKS_EXPORT int FooE_deepcopy(const struct FooE *,
                                             struct FooE **);

extern SIMPLE_MOCKS_EXPORT int FooE_display(const struct FooE *, FILE *);

extern SIMPLE_MOCKS_EXPORT int FooE_debug(const struct FooE *, FILE *);

extern SIMPLE_MOCKS_EXPORT bool FooE_eq(const struct FooE *,
                                        const struct FooE *);

extern SIMPLE_MOCKS_EXPORT int FooE_to_json(const struct FooE *, char **);

extern SIMPLE_MOCKS_EXPORT int FooE_from_jsonObject(const JSON_Object *,
                                                    struct FooE **);

extern SIMPLE_MOCKS_EXPORT int FooE_from_json(const char *, struct FooE **);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SIMPLE_JSON_H */
