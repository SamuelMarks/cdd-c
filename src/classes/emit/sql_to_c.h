/**
 * @file sql_to_c.h
 * @brief Emits C structures and array containers from SQL DDL AST.
 */

#ifndef C_CDD_SQL_TO_C_H
#define C_CDD_SQL_TO_C_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "classes/parse/sql.h"
#include <stdio.h>
/* clang-format on */

/**
 * @brief Map a SQL data type to a C type string.
 *
 * @param type The SQL data type.
 * @param is_nullable Whether the field can be NULL (if 1, returns a pointer
 * type for certain types if needed, though usually handled by the caller).
 * @return String representing the C type (e.g., "int32_t", "char*").
 */
C_CDD_EXPORT int sql_type_to_c_type(enum SqlDataType type, char **_out_val);

/**
 * @brief Checks if a given SQL type is passed/stored as a pointer string by
 * default.
 * @param type The SQL data type.
 * @return 1 if string/pointer, 0 if primitive.
 */
C_CDD_EXPORT int sql_type_is_string(enum SqlDataType type);

/**
 * @brief Generate the C header file content for a given SQL table.
 *
 * @param fp File pointer to write the output to.
 * @param table The SQL table AST node.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT int sql_to_c_header_emit(FILE *fp,
                                      const struct sql_table_t *table);

/**
 * @brief Generate the C source file content containing helpers for a given SQL
 * table.
 *
 * @param fp File pointer to write the output to.
 * @param table The SQL table AST node.
 * @param header_name The name of the header file to include (e.g. "users.h").
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT int sql_to_c_source_emit(FILE *fp, const struct sql_table_t *table,
                                      const char *header_name);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_SQL_TO_C_H */