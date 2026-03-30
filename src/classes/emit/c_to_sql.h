/**
 * @file c_to_sql.h
 * @brief C-ORM Schema Generation logic.
 */

#ifndef C_CDD_CODEGEN_C_TO_SQL_H
#define C_CDD_CODEGEN_C_TO_SQL_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include <stdio.h>
#include "c_cdd_export.h"
#include "classes/emit/struct.h"
#include "classes/emit/cdd_c_orm_meta.h"
/* clang-format on */

/**
 * @brief Dialects supported for SQL generation.
 */
typedef enum {
  C_TO_SQL_DIALECT_SQLITE = 0,
  C_TO_SQL_DIALECT_POSTGRESQL = 1,
  C_TO_SQL_DIALECT_MYSQL = 2
} c_to_sql_dialect_t;

/**
 * @brief Differences between two schema versions.
 */
typedef struct {
  cdd_c_prop_meta_t *added_props;
  size_t num_added;
  cdd_c_prop_meta_t *dropped_props;
  size_t num_dropped;
  cdd_c_prop_meta_t *altered_props; /* simplistic: just track name */
  size_t num_altered;
} cdd_c_meta_diff_t;

/**
 * @brief Generate a CREATE TABLE SQL statement from C struct fields.
 *
 * @param fp The file pointer to write to.
 * @param table_name The name of the table to create.
 * @param sf The struct fields representing the table columns.
 * @param dialect The SQL dialect to generate for.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT int write_struct_to_sql_create_table(FILE *fp,
                                                  const char *table_name,
                                                  const struct StructFields *sf,
                                                  c_to_sql_dialect_t dialect);

/**
 * @brief Generate a CREATE TABLE SQL statement from C-ORM metadata.
 *
 * @param meta The C-ORM metadata.
 * @param dialect The SQL dialect to generate for.
 * @param out_sql Pointer to a string that will be allocated and contain the
 * generated SQL.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT int cdd_c_meta_to_sql_create_table(const cdd_c_meta_t *meta,
                                                c_to_sql_dialect_t dialect,
                                                char **out_sql);

/**
 * @brief Compute the difference between an old and new schema.
 *
 * @param old_schema The old C-ORM metadata.
 * @param new_schema The new C-ORM metadata.
 * @param out_diff Pointer to a diff structure to populate.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT int cdd_c_meta_diff(const cdd_c_meta_t *old_schema,
                                 const cdd_c_meta_t *new_schema,
                                 cdd_c_meta_diff_t *out_diff);

/**
 * @brief Generate UP and DOWN migration SQL scripts from a diff.
 *
 * @param table_name The name of the table being migrated.
 * @param diff The diff structure containing changes.
 * @param dialect The SQL dialect to generate for.
 * @param up_sql Pointer to a string that will be allocated and contain the UP
 * migration SQL.
 * @param down_sql Pointer to a string that will be allocated and contain the
 * DOWN migration SQL.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT int cdd_c_meta_diff_to_sql(const char *table_name,
                                        const cdd_c_meta_diff_t *diff,
                                        c_to_sql_dialect_t dialect,
                                        char **up_sql, char **down_sql);

/**
 * @brief Free a diff object.
 *
 * @param diff The diff structure to free.
 */
C_CDD_EXPORT void cdd_c_meta_diff_free(cdd_c_meta_diff_t *diff);

#ifdef __cplusplus
}
#endif

#endif /* C_CDD_CODEGEN_C_TO_SQL_H */
