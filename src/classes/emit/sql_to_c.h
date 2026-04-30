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
#include "classes/emit/cdd_c_orm_meta.h"
#include "classes/parse/query_projection.h"
#include "classes/parse/abstract_struct.h"
#include <stdio.h>
/* clang-format on */

/**
 * @brief Map a SQL data type to a C type string.
 *
 * @param type The SQL data type.
 * @param _out_val Whether the field can be NULL (if 1, returns a pointer
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
 * @brief Map a SQL data type to a c_orm_type_t enum string.
 *
 * @param type The SQL data type.
 * @param[out] out_val out_val
 * @return String representing the c_orm_type_t (e.g., "C_ORM_TYPE_INT32").
 */
C_CDD_EXPORT int sql_type_to_c_orm_type(enum SqlDataType type,
                                        const char **out_val);

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

/** @cond DOXYGEN_IGNORE */
C_CDD_EXPORT int
sql_to_c_projection_struct_emit(FILE *fp, const cdd_c_query_projection_t *proj,
                                const char *struct_name,
                                unsigned long long *out_hash);
/** @endcond */
C_CDD_EXPORT int
sql_to_c_projection_free_emit(FILE *fp, const cdd_c_query_projection_t *proj,
                              const char *struct_name);
C_CDD_EXPORT int
sql_to_c_projection_meta_emit(FILE *fp, const cdd_c_query_projection_t *proj,
                              const char *struct_name);
C_CDD_EXPORT int
sql_to_c_projection_hydrate_emit(FILE *fp, const cdd_c_query_projection_t *proj,
                                 const char *struct_name);
/**
 * @brief sql_to_c_projection_dehydrate_emit
 * @param fp fp
 * @param proj proj
 * @param struct_name struct_name
 */
C_CDD_EXPORT int sql_to_c_projection_dehydrate_emit(
    FILE *fp, const cdd_c_query_projection_t *proj, const char *struct_name);
/**
 * @brief sql_to_c_projection_nested_struct_emit
 * @param fp fp
 * @param proj proj
 * @param struct_name struct_name
 */
C_CDD_EXPORT int sql_to_c_projection_nested_struct_emit(
    FILE *fp, const cdd_c_query_projection_t *proj, const char *struct_name);
/**
 * @brief sql_to_c_projection_nested_array_emit
 * @param fp fp
 * @param proj proj
 * @param struct_name struct_name
 * @param array_name array_name
 */
C_CDD_EXPORT int sql_to_c_projection_nested_array_emit(
    FILE *fp, const cdd_c_query_projection_t *proj, const char *struct_name,
    const char *array_name);
/**
 * @brief sql_to_c_projection_dirty_bitmask_emit
 * @param fp fp
 * @param proj proj
 * @param struct_name struct_name
 */
C_CDD_EXPORT int sql_to_c_projection_dirty_bitmask_emit(
    FILE *fp, const cdd_c_query_projection_t *proj, const char *struct_name);
C_CDD_EXPORT int
sql_to_c_projection_union_struct_emit(FILE *fp,
                                      const cdd_c_query_projection_t *projs,
                                      size_t n_projs, const char *struct_name);
/**
 * @brief sql_to_c_projection_polymorphic_struct_emit
 * @param fp fp
 * @param proj proj
 * @param struct_name struct_name
 */
C_CDD_EXPORT int sql_to_c_projection_polymorphic_struct_emit(
    FILE *fp, const cdd_c_query_projection_t *proj, const char *struct_name);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_SQL_TO_C_H */
