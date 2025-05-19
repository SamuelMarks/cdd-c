#ifndef LIB_EXPORT_H
#define LIB_EXPORT_H

#ifdef LIB_STATIC_DEFINE
#define LIB_EXPORT
#define LIB_NO_EXPORT
#else
#ifndef LIB_EXPORT
#ifdef LIB_EXPORTS
/* We are building this library */
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MSYS__) ||          \
    defined(__CYGWIN__)
#define LIB_EXPORT __declspec(dllexport)
#elif defined(__SUNPRO_C)
#define LIB_EXPORT __global
#else
#define LIB_EXPORT __attribute__((visibility("default")))
#endif /* defined(_MSC_VER) || defined(__MINGW32__) || defined(__MSYS__) ||    \
          defined(__CYGWIN__) */
#else
/* We are using this library */
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MSYS__) ||          \
    defined(__CYGWIN__)
#define LIB_EXPORT __declspec(dllimport)
#elif defined(__SUNPRO_C)
#define LIB_EXPORT __global
#else
#define LIB_EXPORT __attribute__((visibility("default")))
#endif /* defined(_MSC_VER) || defined(__MINGW32__) || defined(__MSYS__) ||    \
          defined(__CYGWIN__) */
#endif /* LIB_EXPORTS */
#endif /* ! LIB_EXPORT */

#ifndef LIB_NO_EXPORT
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MSYS__) ||          \
    defined(__CYGWIN__) || defined(__SUNPRO_C)
#define LIB_NO_EXPORT
#else
#define LIB_NO_EXPORT __attribute__((visibility("hidden")))
#endif /* defined(_MSC_VER) || defined(__MINGW32__) || defined(__MSYS__) ||    \
          defined(__CYGWIN__) || defined(__SUNPRO_C) */
#endif /* ! LIB_NO_EXPORT */
#endif /* LIB_STATIC_DEFINE */

#ifndef LIB_DEPRECATED
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MSYS__) ||          \
    defined(__CYGWIN__)
#define LIB_DEPRECATED __declspec(deprecated)
#elif defined(__SUNPRO_C)
#define LIB_DEPRECATED
#else
#define LIB_DEPRECATED __attribute__((__deprecated__))
#endif /* defined(_MSC_VER) || defined(__MINGW32__) || defined(__MSYS__) ||    \
          defined(__CYGWIN__) */
#endif /* ! LIB_DEPRECATED */

#ifndef LIB_DEPRECATED_EXPORT
#define LIB_DEPRECATED_EXPORT LIB_EXPORT LIB_DEPRECATED
#endif /* ! LIB_DEPRECATED_EXPORT */

#ifndef LIB_DEPRECATED_NO_EXPORT
#define LIB_DEPRECATED_NO_EXPORT LIB_NO_EXPORT LIB_DEPRECATED
#endif /* ! LIB_DEPRECATED_NO_EXPORT */

#if 0 /* DEFINE_NO_DEPRECATED */
#ifndef LIB_NO_DEPRECATED
#define LIB_NO_DEPRECATED
#endif /* ! LIB_NO_DEPRECATED */
#endif /* 0 */

#endif /* !LIB_EXPORT_H */
