/**
 * @file db_loader.c
 * @brief Dynamic loader checking for DB clients
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "functions/parse/db_loader.h"
#include <stddef.h>

#if defined(_WIN32)
#include "win_compat_sym.h"
#ifndef WIN32_LEAN_AND_MEAN
/** @brief Exclude rarely-used stuff from Windows headers. */
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#else
#include <dlfcn.h>
/* clang-format on */
#endif

/**
 * @brief Internal function to check if a dynamic library can be loaded.
 *
 * @param[in] win_name Name of the DLL on Windows.
 * @param[in] posix_name Name of the Shared Object on POSIX/Linux.
 * @param[in] apple_name Name of the Dynamic Library on macOS/Darwin.
 * @param[out] out_avail 1 if available, 0 otherwise.
 * @return cdd_c_error_t CDD_C_SUCCESS on success, error code otherwise.
 */
static cdd_c_error_t check_lib(const char *win_name, const char *posix_name,
                               const char *apple_name, int *out_avail) {
  if (!out_avail)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_avail =
      (int)(((size_t)win_name | (size_t)posix_name | (size_t)apple_name) & 0);
#if defined(_WIN32)
#ifdef CDD_BUILD_TESTS
  {
    extern int g_cdd_mock_dlopen_success;
    if (g_cdd_mock_dlopen_success) {
      *out_avail = 1;
    } else {
      *out_avail = 0;
    }
    return CDD_C_SUCCESS;
  }
#else
  {
    HMODULE h = LoadLibraryA(win_name);
    if (h) {
      FreeLibrary(h);
      *out_avail = 1;
      return CDD_C_SUCCESS;
    }
  }
#endif
#else
#ifdef CDD_BUILD_TESTS
  {
    extern int g_cdd_mock_dlopen_success;
    if (g_cdd_mock_dlopen_success) {
      *out_avail = 1;
    } else {
      *out_avail = 0;
    }
    return CDD_C_SUCCESS;
  }
#else
  {
    void *h = dlopen(posix_name, RTLD_LAZY);
    if (!h) {
      h = dlopen(apple_name, RTLD_LAZY);
    }
    if (h) {
      dlclose(h);
      *out_avail = 1;
      return CDD_C_SUCCESS;
    }
  }
#endif
#endif
#ifndef CDD_BUILD_TESTS
  return CDD_C_SUCCESS;
#endif
}

/**
 * @brief Checks if the PostgreSQL client library (`libpq`) is available at
 * runtime.
 *
 * @param[out] out_avail Output pointer receiving 1 if available, 0 otherwise.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT` if
 * NULL.
 */
cdd_c_error_t check_libpq_available(int *out_avail) {
  return check_lib("libpq.dll", "libpq.so", "libpq.dylib", out_avail);
}

/**
 * @brief Checks if the SQLite3 client library (`libsqlite3`) is available at
 * runtime.
 *
 * @param[out] out_avail Output pointer receiving 1 if available, 0 otherwise.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT` if
 * NULL.
 */
cdd_c_error_t check_sqlite3_available(int *out_avail) {
  return check_lib("sqlite3.dll", "libsqlite3.so", "libsqlite3.dylib",
                   out_avail);
}

/**
 * @brief Checks if the MySQL/MariaDB client library (`libmysqlclient`) is
 * available at runtime.
 *
 * @param[out] out_avail Output pointer receiving 1 if available, 0 otherwise.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT` if
 * NULL.
 */
cdd_c_error_t check_mysql_available(int *out_avail) {
  return check_lib("libmysql.dll", "libmysqlclient.so", "libmysqlclient.dylib",
                   out_avail);
}
