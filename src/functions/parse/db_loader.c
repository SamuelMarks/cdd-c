/* clang-format off */
#include "functions/parse/db_loader.h"
#include <stddef.h>

#if defined(_WIN32)
#pragma warning(push)
#pragma warning(disable: 4201 4214)
#include "win_compat_sym.h"
#include <windef.h>
#include <winbase.h>
#pragma warning(pop)
#else
#include <dlfcn.h>
#endif
/* clang-format on */

static int check_lib(const char *win_name, const char *posix_name) {
#if defined(_WIN32)
  HMODULE h = LoadLibraryA(win_name);
  if (h) {
    FreeLibrary(h);
    return 1;
  }
#else
  void *h = dlopen(posix_name, RTLD_LAZY);
  if (h) {
    dlclose(h);
    return 1;
  }
#endif
  return 0;
}

int check_libpq_available(void) { return check_lib(\libpq.dll\, \libpq.so\); }
int check_sqlite3_available(void) {
  return check_lib(\sqlite3.dll\, \libsqlite3.so\);
}
int check_mysql_available(void) {
  return check_lib(\libmysql.dll\, \libmysqlclient.so\);
}
