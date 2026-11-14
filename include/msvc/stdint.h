/**
 * @file stdint.h
 * @brief MSVC stdint port.
 */
#ifndef C_CDD_MSVC_STDINT_H
#define C_CDD_MSVC_STDINT_H

/* clang-format off */
/* No includes required for MSVC < 1600 fixed-width integer port */
/* clang-format on */

#if defined(_MSC_VER) && _MSC_VER < 1600

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @brief 8-bit signed integer */
typedef signed __int8 int8_t;
/** @brief 16-bit signed integer */
typedef signed __int16 int16_t;
/** @brief 32-bit signed integer */
typedef signed __int32 int32_t;
/** @brief 64-bit signed integer */
typedef signed __int64 int64_t;
/** @brief 8-bit unsigned integer */
typedef unsigned __int8 uint8_t;
/** @brief 16-bit unsigned integer */
typedef unsigned __int16 uint16_t;
/** @brief 32-bit unsigned integer */
typedef unsigned __int32 uint32_t;
/** @brief 64-bit unsigned integer */
typedef unsigned __int64 uint64_t;

#ifndef _INTPTR_T_DEFINED
/** @brief Guard indicating intptr_t is defined */
#define _INTPTR_T_DEFINED
#ifdef _WIN64
/** @brief Signed pointer-sized integer */
typedef signed __int64 intptr_t;
#else
/** @brief Signed pointer-sized integer */
typedef int intptr_t;
#endif
#endif

#ifndef _UINTPTR_T_DEFINED
/** @brief Guard indicating uintptr_t is defined */
#define _UINTPTR_T_DEFINED
#ifdef _WIN64
/** @brief Unsigned pointer-sized integer */
typedef unsigned __int64 uintptr_t;
#else
/** @brief Unsigned pointer-sized integer */
typedef unsigned int uintptr_t;
#endif
#endif

#ifndef INT8_MIN
/** @brief Minimum 8-bit signed integer */
#define INT8_MIN (-127 - 1)
/** @brief Maximum 8-bit signed integer */
#define INT8_MAX 127
/** @brief Maximum 8-bit unsigned integer */
#define UINT8_MAX 0xffU
/** @brief Minimum 16-bit signed integer */
#define INT16_MIN (-32767 - 1)
/** @brief Maximum 16-bit signed integer */
#define INT16_MAX 32767
/** @brief Maximum 16-bit unsigned integer */
#define UINT16_MAX 0xffffU
/** @brief Minimum 32-bit signed integer */
#define INT32_MIN (-2147483647 - 1)
/** @brief Maximum 32-bit signed integer */
#define INT32_MAX 2147483647
/** @brief Maximum 32-bit unsigned integer */
#define UINT32_MAX 0xffffffffU
/** @brief Minimum 64-bit signed integer */
#define INT64_MIN (-9223372036854775807i64 - 1)
/** @brief Maximum 64-bit signed integer */
#define INT64_MAX 9223372036854775807i64
/** @brief Maximum 64-bit unsigned integer */
#define UINT64_MAX 0xffffffffffffffffui64
#endif

#ifndef SIZE_MAX
/** @brief Maximum value of size_t */
#define SIZE_MAX ((size_t) - 1)
#endif

#ifndef INT64_C
/** @brief 64-bit signed integer constant macro */
#define INT64_C(x) (x##i64)
/** @brief 64-bit unsigned integer constant macro */
#define UINT64_C(x) (x##ui64)
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

#endif /* !C_CDD_MSVC_STDINT_H */
