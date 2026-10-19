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

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

#endif /* !C_CDD_MSVC_STDINT_H */
