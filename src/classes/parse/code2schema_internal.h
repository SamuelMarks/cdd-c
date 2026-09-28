/**
 * @file code2schema_internal.h
 * @brief Internal declarations and utilities for code2schema modules.
 * @author Samuel Marks
 */

#ifndef CODE2SCHEMA_INTERNAL_H
#define CODE2SCHEMA_INTERNAL_H

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../win_compat_sym.h"

#include <parson.h>

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif
#endif

#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include "classes/emit/struct.h"
#include "classes/parse/code2schema.h"
#include "classes/parse/mapping.h"
#include "classes/parse/numeric.h"
#include "functions/emit/codegen.h"
#include "functions/parse/str.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT volatile int g_c2s_helper_fail;
extern C_CDD_EXPORT volatile int g_cdd_fail_c2s_collect_schema_extras;
extern C_CDD_EXPORT volatile int g_cdd_fail_json_set_value;
extern C_CDD_EXPORT int g_cdd_fail_json_serialize;
#endif

/** @brief MAX_LINE_LENGTH definition */
#define MAX_LINE_LENGTH 1024

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CODE2SCHEMA_INTERNAL_H */
