/**
 * @file db_loader.c
 * @brief Dynamic loader checking for DB clients
 */

/* clang-format off */
#include "functions/parse/db_loader.h"
#include <stddef.h>

#if defined(_WIN32)
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4201 4214)
#endif
#include "win_compat_sym.h"
#include <windef.h>
#include <winbase.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
#else
#include <dlfcn.h>
#endif
/* clang-format on */

/**
 * @brief Internal function to check if a dynamic library can be loaded.
 *
 * @param win_name Name of the DLL on Windows.
 * @param posix_name Name of the Shared Object on POSIX.
 * @param[out] out_avail 1 if available, 0 otherwise.
 * @return int 0 on success, error code otherwise.
 */
static int check_lib(const char *win_name, const char *posix_name,
                     int *out_avail) {
  (void)win_name;
  (void)posix_name;
  if (!out_avail)
    return 22; /* EINVAL */
  *out_avail = 0;
#if defined(_WIN32)
  {
    HMODULE h = LoadLibraryA(win_name);
    if (h) {
      FreeLibrary(h);
      *out_avail = 1;
      return 0;
    }
  }
#else
  {
    void *h = dlopen(posix_name, RTLD_LAZY);
    if (h) {
      dlclose(h);
      *out_avail = 1;
      return 0;
    }
  }
#endif
  return 0;
}

int check_libpq_available(int *out_avail) {
  return check_lib("libpq.dll", "libpq.so", out_avail);
}

int check_sqlite3_available(int *out_avail) {
  return check_lib("sqlite3.dll", "libsqlite3.so", out_avail);
}

int check_mysql_available(int *out_avail) {
  return check_lib("libmysql.dll", "libmysqlclient.so", out_avail);
}
