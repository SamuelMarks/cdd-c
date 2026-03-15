/**
 * @file migration_runner.c
 * @brief Implementation of database migrations logic using libpq.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "functions/parse/migration_runner.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if defined(USE_LIBPQ_LINKED)
#include <libpq-fe.h>
#endif /* USE_LIBPQ_LINKED */

#include "classes/parse/migration.h"
#include "functions/parse/fs.h"
#include "functions/parse/str.h"
/* clang-format on */

#if defined(USE_LIBPQ_LINKED) || defined(USE_LIBPQ_DYNAMIC)

#if defined(USE_LIBPQ_DYNAMIC)
#if defined(_WIN32)
#pragma warning(push)
#pragma warning(disable : 4201 4214)
#include "win_compat_sym.h"
#include <winbase.h>
#include <windef.h>
#pragma warning(pop)
#else
#include <dlfcn.h>
#endif

typedef struct pg_conn PGconn;
typedef struct pg_result PGresult;

#define CONNECTION_OK 0
#define PGRES_COMMAND_OK 1
#define PGRES_TUPLES_OK 2

typedef PGconn *(*PQconnectdb_t)(const char *conninfo);
typedef int (*PQstatus_t)(const PGconn *conn);
typedef char *(*PQerrorMessage_t)(const PGconn *conn);
typedef void (*PQfinish_t)(PGconn *conn);
typedef PGresult *(*PQexec_t)(PGconn *conn, const char *query);
typedef int (*PQresultStatus_t)(const PGresult *res);
typedef int (*PQntuples_t)(const PGresult *res);
typedef char *(*PQgetvalue_t)(const PGresult *res, int tup_num, int field_num);
typedef void (*PQclear_t)(PGresult *res);
typedef PGresult *(*PQexecParams_t)(PGconn *conn, const char *command,
                                    int nParams, const unsigned int *paramTypes,
                                    const char *const *paramValues,
                                    const int *paramLengths,
                                    const int *paramFormats, int resultFormat);

static PQconnectdb_t dyn_PQconnectdb = NULL;
static PQstatus_t dyn_PQstatus = NULL;
static PQerrorMessage_t dyn_PQerrorMessage = NULL;
static PQfinish_t dyn_PQfinish = NULL;
static PQexec_t dyn_PQexec = NULL;
static PQresultStatus_t dyn_PQresultStatus = NULL;
static PQntuples_t dyn_PQntuples = NULL;
static PQgetvalue_t dyn_PQgetvalue = NULL;
static PQclear_t dyn_PQclear = NULL;
static PQexecParams_t dyn_PQexecParams = NULL;

static int load_libpq(void) {
  static int loaded = 0;
  void *handle;
  if (loaded)
    return 1;
#if defined(_WIN32)
  handle = LoadLibraryA("libpq.dll");
#else
  handle = dlopen("libpq.so", RTLD_LAZY);
#endif
  if (!handle)
    return 0;

#if defined(_WIN32)
#define LOAD_SYM(name, type)                                                   \
  dyn_##name = (type)GetProcAddress((HMODULE)handle, #name)
#else
#define LOAD_SYM(name, type) dyn_##name = (type)dlsym(handle, #name)
#endif

  LOAD_SYM(PQconnectdb, PQconnectdb_t);
  LOAD_SYM(PQstatus, PQstatus_t);
  LOAD_SYM(PQerrorMessage, PQerrorMessage_t);
  LOAD_SYM(PQfinish, PQfinish_t);
  LOAD_SYM(PQexec, PQexec_t);
  LOAD_SYM(PQresultStatus, PQresultStatus_t);
  LOAD_SYM(PQntuples, PQntuples_t);
  LOAD_SYM(PQgetvalue, PQgetvalue_t);
  LOAD_SYM(PQclear, PQclear_t);
  LOAD_SYM(PQexecParams, PQexecParams_t);

  if (!dyn_PQconnectdb || !dyn_PQexec || !dyn_PQfinish)
    return 0;

  loaded = 1;
  return 1;
}

#define PQconnectdb dyn_PQconnectdb
#define PQstatus dyn_PQstatus
#define PQerrorMessage dyn_PQerrorMessage
#define PQfinish dyn_PQfinish
#define PQexec dyn_PQexec
#define PQresultStatus dyn_PQresultStatus
#define PQntuples dyn_PQntuples
#define PQgetvalue dyn_PQgetvalue
#define PQclear dyn_PQclear
#define PQexecParams dyn_PQexecParams

#define ENSURE_LIBPQ()                                                         \
  if (!load_libpq()) {                                                         \
    fprintf(                                                                   \
        stderr,                                                                \
        "Error: PostgreSQL dynamic library (libpq) not found at runtime.\n");  \
    return ENOSYS;                                                             \
  }

#else
static int load_libpq(void) { return 1; }
#define ENSURE_LIBPQ()
#endif /* USE_LIBPQ_DYNAMIC */

/**
 * @brief Helper struct for storing a migration file info.
 */
struct MigrationFile {
  char *filepath;
  char *version;
};

/**
 * @brief Helper list for collecting migration files.
 */
struct MigrationList {
  struct MigrationFile *files;
  size_t count;
  size_t capacity;
};

/**
 * @brief Create the schema_migrations table if it doesn't exist.
 *
 * @param[in] conn Active PGconn.
 * @return 0 on success, error code otherwise.
 */
static int ensure_schema_migrations_table(PGconn *conn) {
  const char *query = "CREATE TABLE IF NOT EXISTS schema_migrations ("
                      "version VARCHAR(255) PRIMARY KEY, "
                      "applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
                      ");";
  PGresult *res = PQexec(conn, query);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    fprintf(stderr, "Failed to create schema_migrations table: %s\n",
            PQerrorMessage(conn));
    PQclear(res);
    return EIO;
  }
  PQclear(res);
  return 0;
}

/**
 * @brief Callback to collect migration files via walk_directory.
 *
 * @param[in] path File path.
 * @param[in] user_data Pointer to MigrationList.
 * @return 0 on success, error code otherwise.
 */
static int collect_migration_cb(const char *path, void *user_data) {
  struct MigrationList *list;
  size_t len;
  int rc;
  char *basename;

  list = (struct MigrationList *)user_data;
  basename = NULL;

  if (!path) {
    return 0;
  }

  len = strlen(path);
  if (len < 4 || strcmp(path + len - 4, ".sql") != 0) {
    return 0;
  }

  rc = get_basename(path, &basename);
  if (rc != 0 || !basename) {
    return rc ? rc : ENOMEM;
  }

  /* Extract version: remove .sql extension */
  basename[strlen(basename) - 4] = '\0';

  if (list->count >= list->capacity) {
    size_t new_cap = list->capacity == 0 ? 16 : list->capacity * 2;
    struct MigrationFile *new_arr = (struct MigrationFile *)realloc(
        list->files, new_cap * sizeof(struct MigrationFile));
    if (!new_arr) {
      free(basename);
      return ENOMEM;
    }
    list->files = new_arr;
    list->capacity = new_cap;
  }

  if (c_cdd_strdup(path, &list->files[list->count].filepath) != 0) {
    free(basename);
    return ENOMEM;
  }
  list->files[list->count].version = basename;
  list->count++;

  return 0;
}

/**
 * @brief Compare function for qsort.
 *
 * @param[in] a MigrationFile pointer A.
 * @param[in] b MigrationFile pointer B.
 * @return integer less than, equal to, or greater than zero.
 */
static int compare_migrations(const void *a, const void *b) {
  const struct MigrationFile *ma = (const struct MigrationFile *)a;
  const struct MigrationFile *mb = (const struct MigrationFile *)b;
  return strcmp(ma->version, mb->version);
}

/**
 * @brief Get list of applied migrations from DB.
 *
 * @param[in] conn Active PGconn.
 * @param[out] out_versions Array of strings.
 * @param[out] out_count Number of elements.
 * @return 0 on success, error code otherwise.
 */
static int get_applied_migrations(PGconn *conn, char ***out_versions,
                                  size_t *out_count) {
  PGresult *res;
  int rows;
  int i;
  char **versions;

  *out_versions = NULL;
  *out_count = 0;

  res = PQexec(conn,
               "SELECT version FROM schema_migrations ORDER BY version ASC");
  if (PQresultStatus(res) != PGRES_TUPLES_OK) {
    fprintf(stderr, "Failed to fetch applied migrations: %s\n",
            PQerrorMessage(conn));
    PQclear(res);
    return EIO;
  }

  rows = PQntuples(res);
  if (rows == 0) {
    PQclear(res);
    return 0;
  }

  versions = (char **)malloc((size_t)rows * sizeof(char *));
  if (!versions) {
    PQclear(res);
    return ENOMEM;
  }

  for (i = 0; i < rows; i++) {
    char *val = PQgetvalue(res, i, 0);
    if (c_cdd_strdup(val, &versions[i]) != 0) {
      int j;
      for (j = 0; j < i; j++) {
        free(versions[j]);
      }
      free(versions);
      PQclear(res);
      return ENOMEM;
    }
  }

  *out_versions = versions;
  *out_count = (size_t)rows;
  PQclear(res);

  return 0;
}

/**
 * @brief Helper to check if a version is in applied list.
 *
 * @param[in] version Version string.
 * @param[in] applied_versions Array of strings.
 * @param[in] applied_count Count of strings.
 * @return 1 if found, 0 otherwise.
 */
static int is_applied(const char *version, char **applied_versions,
                      size_t applied_count) {
  size_t i;
  for (i = 0; i < applied_count; i++) {
    if (strcmp(version, applied_versions[i]) == 0) {
      return 1;
    }
  }
  return 0;
}

/**
 * @brief Autogenerated docstring
 */
int apply_migration(const char *filepath) {
  struct MigrationStatements stmts;
  PGconn *conn;
  PGresult *res;
  const char *db_url;
  int rc;

  ENSURE_LIBPQ();

  db_url = getenv("DATABASE_URL");
  if (!db_url) {
    fprintf(stderr, "Error: DATABASE_URL environment variable is not set.\n");
    return EINVAL;
  }

  rc = parse_migration_file(filepath, &stmts);
  if (rc != 0) {
    fprintf(stderr, "Error: failed to parse migration file '%s'\n", filepath);
    return rc;
  }

  if (!stmts.up_statement) {
    migration_statements_free(&stmts);
    return 0; /* Nothing to do */
  }

  conn = PQconnectdb(db_url);
  if (PQstatus(conn) != CONNECTION_OK) {
    fprintf(stderr, "Connection to database failed: %s\n",
            PQerrorMessage(conn));
    PQfinish(conn);
    migration_statements_free(&stmts);
    return ECONNREFUSED;
  }

  res = PQexec(conn, stmts.up_statement);
  if (PQresultStatus(res) != PGRES_COMMAND_OK &&
      PQresultStatus(res) != PGRES_TUPLES_OK) {
    fprintf(stderr, "Migration UP failed: %s\n", PQerrorMessage(conn));
    PQclear(res);
    PQfinish(conn);
    migration_statements_free(&stmts);
    return EIO;
  }

  PQclear(res);
  PQfinish(conn);
  migration_statements_free(&stmts);

  return 0;
}

/**
 * @brief Autogenerated docstring
 */
int rollback_migration(const char *filepath) {
  struct MigrationStatements stmts;
  PGconn *conn;
  PGresult *res;
  const char *db_url;
  int rc;

  db_url = getenv("DATABASE_URL");
  if (!db_url) {
    fprintf(stderr, "Error: DATABASE_URL environment variable is not set.\n");
    return EINVAL;
  }

  rc = parse_migration_file(filepath, &stmts);
  if (rc != 0) {
    fprintf(stderr, "Error: failed to parse migration file '%s'\n", filepath);
    return rc;
  }

  if (!stmts.down_statement) {
    migration_statements_free(&stmts);
    return 0; /* Nothing to do */
  }

  conn = PQconnectdb(db_url);
  if (PQstatus(conn) != CONNECTION_OK) {
    fprintf(stderr, "Connection to database failed: %s\n",
            PQerrorMessage(conn));
    PQfinish(conn);
    migration_statements_free(&stmts);
    return ECONNREFUSED;
  }

  res = PQexec(conn, stmts.down_statement);
  if (PQresultStatus(res) != PGRES_COMMAND_OK &&
      PQresultStatus(res) != PGRES_TUPLES_OK) {
    fprintf(stderr, "Migration DOWN failed: %s\n", PQerrorMessage(conn));
    PQclear(res);
    PQfinish(conn);
    migration_statements_free(&stmts);
    return EIO;
  }

  PQclear(res);
  PQfinish(conn);
  migration_statements_free(&stmts);

  return 0;
}

/**
 * @brief Autogenerated docstring
 */
int run_pending_migrations(const char *migrations_dir) {
  PGconn *conn;
  const char *db_url;
  int rc;
  struct MigrationList list;
  char **applied_versions;
  size_t applied_count;
  size_t i;

  db_url = getenv("DATABASE_URL");
  if (!db_url) {
    fprintf(stderr, "Error: DATABASE_URL environment variable is not set.\n");
    return EINVAL;
  }

  conn = PQconnectdb(db_url);
  if (PQstatus(conn) != CONNECTION_OK) {
    fprintf(stderr, "Connection to database failed: %s\n",
            PQerrorMessage(conn));
    PQfinish(conn);
    return ECONNREFUSED;
  }

  rc = ensure_schema_migrations_table(conn);
  if (rc != 0) {
    PQfinish(conn);
    return rc;
  }

  rc = get_applied_migrations(conn, &applied_versions, &applied_count);
  if (rc != 0) {
    PQfinish(conn);
    return rc;
  }

  list.files = NULL;
  list.count = 0;
  list.capacity = 0;

  rc = walk_directory(migrations_dir, collect_migration_cb, &list);
  if (rc != 0) {
    fprintf(stderr, "Error reading migrations directory.\n");
    for (i = 0; i < applied_count; i++) {
      free(applied_versions[i]);
    }
    if (applied_versions) {
      free(applied_versions);
    }
    PQfinish(conn);
    return rc;
  }

  if (list.count > 0) {
    qsort(list.files, list.count, sizeof(struct MigrationFile),
          compare_migrations);
  }

  for (i = 0; i < list.count; i++) {
    if (!is_applied(list.files[i].version, applied_versions, applied_count)) {
      struct MigrationStatements stmts;
      PGresult *res;
      const char *paramValues[1];

      printf("Applying migration: %s\n", list.files[i].version);

      rc = parse_migration_file(list.files[i].filepath, &stmts);
      if (rc != 0) {
        fprintf(stderr, "Failed to parse migration file '%s'\n",
                list.files[i].filepath);
        break;
      }

      if (stmts.up_statement) {
        res = PQexec(conn, stmts.up_statement);
        if (PQresultStatus(res) != PGRES_COMMAND_OK &&
            PQresultStatus(res) != PGRES_TUPLES_OK) {
          fprintf(stderr, "Migration UP failed: %s\n", PQerrorMessage(conn));
          PQclear(res);
          migration_statements_free(&stmts);
          rc = EIO;
          break;
        }
        PQclear(res);
      }
      migration_statements_free(&stmts);

      paramValues[0] = list.files[i].version;
      res = PQexecParams(conn,
                         "INSERT INTO schema_migrations (version) VALUES ($1);",
                         1, NULL, paramValues, NULL, NULL, 0);
      if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Failed to record migration: %s\n",
                PQerrorMessage(conn));
        PQclear(res);
        rc = EIO;
        break;
      }
      PQclear(res);
    }
  }

  /* Cleanup */
  for (i = 0; i < applied_count; i++) {
    free(applied_versions[i]);
  }
  if (applied_versions) {
    free(applied_versions);
  }

  for (i = 0; i < list.count; i++) {
    free(list.files[i].filepath);
    free(list.files[i].version);
  }
  if (list.files) {
    free(list.files);
  }

  PQfinish(conn);
  return rc;
}

/**
 * @brief Autogenerated docstring
 */
int rollback_last_migration(const char *migrations_dir) {
  PGconn *conn;
  const char *db_url;
  int rc;
  PGresult *res;
  char *last_version;
  struct MigrationList list;
  size_t i;
  char *target_filepath;

  db_url = getenv("DATABASE_URL");
  if (!db_url) {
    fprintf(stderr, "Error: DATABASE_URL environment variable is not set.\n");
    return EINVAL;
  }

  conn = PQconnectdb(db_url);
  if (PQstatus(conn) != CONNECTION_OK) {
    fprintf(stderr, "Connection to database failed: %s\n",
            PQerrorMessage(conn));
    PQfinish(conn);
    return ECONNREFUSED;
  }

  rc = ensure_schema_migrations_table(conn);
  if (rc != 0) {
    PQfinish(conn);
    return rc;
  }

  res = PQexec(
      conn,
      "SELECT version FROM schema_migrations ORDER BY version DESC LIMIT 1");
  if (PQresultStatus(res) != PGRES_TUPLES_OK) {
    fprintf(stderr, "Failed to fetch last migration: %s\n",
            PQerrorMessage(conn));
    PQclear(res);
    PQfinish(conn);
    return EIO;
  }

  if (PQntuples(res) == 0) {
    printf("No migrations to rollback.\n");
    PQclear(res);
    PQfinish(conn);
    return 0;
  }

  last_version = NULL;
  rc = c_cdd_strdup(PQgetvalue(res, 0, 0), &last_version);
  PQclear(res);

  if (rc != 0) {
    PQfinish(conn);
    return rc;
  }

  list.files = NULL;
  list.count = 0;
  list.capacity = 0;

  rc = walk_directory(migrations_dir, collect_migration_cb, &list);
  if (rc != 0) {
    fprintf(stderr, "Error reading migrations directory.\n");
    free(last_version);
    PQfinish(conn);
    return rc;
  }

  target_filepath = NULL;
  for (i = 0; i < list.count; i++) {
    if (strcmp(list.files[i].version, last_version) == 0) {
      rc = c_cdd_strdup(list.files[i].filepath, &target_filepath);
      break;
    }
  }

  for (i = 0; i < list.count; i++) {
    free(list.files[i].filepath);
    free(list.files[i].version);
  }
  if (list.files) {
    free(list.files);
  }

  if (!target_filepath || rc != 0) {
    fprintf(stderr,
            "Error: Migration file for version '%s' not found in directory.\n",
            last_version);
    free(last_version);
    if (target_filepath) {
      free(target_filepath);
    }
    PQfinish(conn);
    return ENOENT;
  }

  printf("Rolling back migration: %s\n", last_version);

  {
    struct MigrationStatements stmts;
    rc = parse_migration_file(target_filepath, &stmts);
    if (rc != 0) {
      fprintf(stderr, "Failed to parse migration file '%s'\n", target_filepath);
      free(target_filepath);
      free(last_version);
      PQfinish(conn);
      return rc;
    }

    if (stmts.down_statement) {
      res = PQexec(conn, stmts.down_statement);
      if (PQresultStatus(res) != PGRES_COMMAND_OK &&
          PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Migration DOWN failed: %s\n", PQerrorMessage(conn));
        PQclear(res);
        migration_statements_free(&stmts);
        free(target_filepath);
        free(last_version);
        PQfinish(conn);
        return EIO;
      }
      PQclear(res);
    }
    migration_statements_free(&stmts);
  }

  {
    const char *paramValues[1];
    paramValues[0] = last_version;
    res =
        PQexecParams(conn, "DELETE FROM schema_migrations WHERE version = $1;",
                     1, NULL, paramValues, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
      fprintf(stderr, "Failed to delete migration record: %s\n",
              PQerrorMessage(conn));
      PQclear(res);
      free(target_filepath);
      free(last_version);
      PQfinish(conn);
      return EIO;
    }
    PQclear(res);
  }

  free(target_filepath);
  free(last_version);
  PQfinish(conn);

  return 0;
}
#include <time.h>

/**
 * @brief Autogenerated docstring
 */
int create_migration_file(const char *migrations_dir, const char *name) {
  char *filepath;
  char *safe_name;
  time_t t;
  struct tm *tm_info;
  char timestamp[20];
  const char *template_str;
  int rc;
  size_t filepath_len;
  int i;

  if (!migrations_dir || !name) {
    return EINVAL;
  }

  t = time(NULL);
  tm_info = localtime(&t);
  if (!tm_info) {
    return EINVAL;
  }
  strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S", tm_info);

  if (c_cdd_strdup(name, &safe_name) != 0) {
    return ENOMEM;
  }

  for (i = 0; safe_name[i]; i++) {
    if (safe_name[i] == ' ' || safe_name[i] == '-' || safe_name[i] == '.') {
      safe_name[i] = '_';
    }
  }

  filepath_len =
      strlen(migrations_dir) + strlen(timestamp) + strlen(safe_name) + 7;
  filepath = (char *)malloc(filepath_len);
  if (!filepath) {
    free(safe_name);
    return ENOMEM;
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  sprintf_s(filepath, filepath_len, "%s%c%s_%s.sql", migrations_dir, PATH_SEP_C,
            timestamp, safe_name);
#else
  sprintf(filepath, "%s%c%s_%s.sql", migrations_dir, PATH_SEP_C, timestamp,
          safe_name);
#endif

  template_str = "-- UP\n"
                 "-- Add up migration statements here\n\n"
                 "-- DOWN\n"
                 "-- Add down migration statements here\n";

  rc = fs_write_to_file(filepath, template_str);

  if (rc == 0) {
    printf("Created migration file: %s\n", filepath);
  } else {
    fprintf(stderr, "Failed to create migration file: %s\n", filepath);
  }

  free(safe_name);
  free(filepath);
  return rc;
}

/**
 * @brief Autogenerated docstring
 */
int reset_database(const char *migrations_dir) {
  PGconn *conn;
  const char *db_url;
  PGresult *res;
  int rc;

  db_url = getenv("DATABASE_URL");
  if (!db_url) {
    fprintf(stderr, "Error: DATABASE_URL environment variable is not set.\n");
    return EINVAL;
  }

  conn = PQconnectdb(db_url);
  if (PQstatus(conn) != CONNECTION_OK) {
    fprintf(stderr, "Connection to database failed: %s\n",
            PQerrorMessage(conn));
    PQfinish(conn);
    return ECONNREFUSED;
  }

  res = PQexec(conn, "DROP SCHEMA public CASCADE; CREATE SCHEMA public;");
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    fprintf(stderr, "Failed to reset database schema: %s\n",
            PQerrorMessage(conn));
    PQclear(res);
    PQfinish(conn);
    return EIO;
  }
  PQclear(res);

  PQfinish(conn);

  printf("Database schema reset successfully.\n");

  rc = run_pending_migrations(migrations_dir);
  return rc;
}

/**
 * @brief Autogenerated docstring
 */
int dump_schema(const char *out_filepath) {
  const char *db_url;
  char *cmd;
  size_t cmd_len;
  int rc;

  if (!out_filepath) {
    return EINVAL;
  }

  db_url = getenv("DATABASE_URL");
  if (!db_url) {
    fprintf(stderr, "Error: DATABASE_URL environment variable is not set.\n");
    return EINVAL;
  }

  /* Note: Assumes pg_dump is in PATH */
  cmd_len = strlen("pg_dump -s -O -x \"\" > \"\"") + strlen(db_url) +
            strlen(out_filepath) + 1;
  cmd = (char *)malloc(cmd_len);
  if (!cmd) {
    return ENOMEM;
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  sprintf_s(cmd, cmd_len, "pg_dump -s -O -x \"%s\" > \"%s\"", db_url,
            out_filepath);
#else
  sprintf(cmd, "pg_dump -s -O -x \"%s\" > \"%s\"", db_url, out_filepath);
#endif

  rc = system(cmd);
  if (rc == 0) {
    printf("Schema dumped successfully to %s\n", out_filepath);
  } else {
    fprintf(stderr, "Failed to dump schema (exit code %d).\n", rc);
    fprintf(stderr, "Ensure 'pg_dump' is installed and in your PATH.\n");
  }

  free(cmd);
  return rc != 0 ? EIO : 0;
}
/**
 * @brief Autogenerated docstring
 */
int setup_test_database(const char *db_name, const char *migrations_dir) {
  PGconn *conn;
  const char *db_url;
  char *create_db_query;
  size_t query_len;
  PGresult *res;

  if (!db_name || !migrations_dir) {
    return EINVAL;
  }

  db_url = getenv("DATABASE_URL");
  if (!db_url) {
    fprintf(stderr, "Error: DATABASE_URL environment variable is not set.\n");
    return EINVAL;
  }

  conn = PQconnectdb(db_url);
  if (PQstatus(conn) != CONNECTION_OK) {
    fprintf(stderr, "Connection to administrative database failed: %s\n",
            PQerrorMessage(conn));
    PQfinish(conn);
    return ECONNREFUSED;
  }

  query_len = strlen("CREATE DATABASE \"\"") + strlen(db_name) + 1;
  create_db_query = (char *)malloc(query_len);
  if (!create_db_query) {
    PQfinish(conn);
    return ENOMEM;
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  sprintf_s(create_db_query, query_len, "CREATE DATABASE \"%s\"", db_name);
#else
  sprintf(create_db_query, "CREATE DATABASE \"%s\"", db_name);
#endif

  res = PQexec(conn, create_db_query);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    /* It's OK if it already exists */
    if (strstr(PQerrorMessage(conn), "already exists") == NULL) {
      fprintf(stderr, "Failed to create test database: %s\n",
              PQerrorMessage(conn));
      PQclear(res);
      free(create_db_query);
      PQfinish(conn);
      return EIO;
    }
  }
  PQclear(res);
  free(create_db_query);
  PQfinish(conn);

  printf("Test database '%s' created (if it didn't exist).\n", db_name);
  return 0;
}

/**
 * @brief Autogenerated docstring
 */
int seed_database(const char *seed_filepath) {
  char *file_data;
  size_t file_size;
  PGconn *conn;
  const char *db_url;
  PGresult *res;
  int rc;

  if (!seed_filepath) {
    return EINVAL;
  }

  db_url = getenv("DATABASE_URL");
  if (!db_url) {
    fprintf(stderr, "Error: DATABASE_URL environment variable is not set.\n");
    return EINVAL;
  }

  rc = read_to_file(seed_filepath, "rb", &file_data, &file_size);
  if (rc != 0) {
    fprintf(stderr, "Error reading seed file '%s'\n", seed_filepath);
    return rc;
  }

  if (file_size == 0 || !file_data) {
    if (file_data) {
      free(file_data);
    }
    return 0;
  }

  conn = PQconnectdb(db_url);
  if (PQstatus(conn) != CONNECTION_OK) {
    fprintf(stderr, "Connection to database failed: %s\n",
            PQerrorMessage(conn));
    free(file_data);
    PQfinish(conn);
    return ECONNREFUSED;
  }

  res = PQexec(conn, file_data);
  if (PQresultStatus(res) != PGRES_COMMAND_OK &&
      PQresultStatus(res) != PGRES_TUPLES_OK) {
    fprintf(stderr, "Seeding failed: %s\n", PQerrorMessage(conn));
    PQclear(res);
    free(file_data);
    PQfinish(conn);
    return EIO;
  }

  PQclear(res);
  free(file_data);
  PQfinish(conn);

  printf("Database seeded successfully from '%s'.\n", seed_filepath);
  return 0;
}
#else  /* USE_LIBPQ */
int apply_migration(const char *filepath) {
  (void)filepath;
  return ENOSYS;
}
int rollback_migration(const char *filepath) {
  (void)filepath;
  return ENOSYS;
}
int run_pending_migrations(const char *migrations_dir) {
  (void)migrations_dir;
  return ENOSYS;
}
int rollback_last_migration(const char *migrations_dir) {
  (void)migrations_dir;
  return ENOSYS;
}
int create_migration_file(const char *migrations_dir, const char *name) {
  (void)migrations_dir;
  (void)name;
  return ENOSYS;
}
int reset_database(const char *migrations_dir) {
  (void)migrations_dir;
  return ENOSYS;
}
int dump_schema(const char *out_filepath) {
  (void)out_filepath;
  return ENOSYS;
}
int setup_test_database(const char *db_name, const char *migrations_dir) {
  (void)db_name;
  (void)migrations_dir;
  return ENOSYS;
}
int seed_database(const char *seed_filepath) {
  (void)seed_filepath;
  return ENOSYS;
}
#endif /* USE_LIBPQ */
