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
#include "classes/parse/query_projection.h"
#include "classes/emit/c_orm_meta.h"
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
 * @brief Map a SQL data type to a c_orm_type_t enum string.
 *
 * @param type The SQL data type.
 * @return String representing the c_orm_type_t (e.g., "C_ORM_TYPE_INT32").
 */
C_CDD_EXPORT const char *sql_type_to_c_orm_type(enum SqlDataType type);

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

/**
 * @brief Generate a specific C struct for a given query projection.
 *
 * @param fp File pointer to write output to.
 * @param proj The query projection AST node.
 * @param struct_name The name of the struct to generate.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT int
sql_to_c_projection_struct_emit(FILE *fp, const cdd_c_query_projection_t *proj,
                                const char *struct_name,
                                unsigned long long *out_hash);

/**
 * @brief Generate metadata variables and definitions for a projection struct.
 *
 * @param fp File pointer to write output to.
 * @param proj The query projection AST node.
 * @param struct_name The name of the struct.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT int
sql_to_c_projection_meta_emit(FILE *fp, const cdd_c_query_projection_t *proj,
                              const char *struct_name);

/**
 * @brief Generate the deep free function for a projection struct.
 *
 * @param fp File pointer to write output to.
 * @param proj The query projection AST node.
 * @param struct_name The name of the struct.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT int
sql_to_c_projection_free_emit(FILE *fp, const cdd_c_query_projection_t *proj,
                              const char *struct_name);

/**
 * @brief Generate the hydration function for a projection struct (SQL row ->
 * struct).
 *
 * @param fp File pointer to write output to.
 * @param proj The query projection AST node.
 * @param struct_name The name of the struct.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT int
sql_to_c_projection_hydrate_emit(FILE *fp, const cdd_c_query_projection_t *proj,
                                 const char *struct_name);

/**
 * @brief Generate the dehydration function for a projection struct (struct ->
 * SQL parameters).
 *
 * @param fp File pointer to write output to.
 * @param proj The query projection AST node.
 * @param struct_name The name of the struct.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT int sql_to_c_projection_dehydrate_emit(
    FILE *fp, const cdd_c_query_projection_t *proj, const char *struct_name);

/**
 * @brief Generate a 1-to-1 nested C struct for a given query projection
 * representing a JOIN.
 *
 * @param fp File pointer to write output to.
 * @param proj The query projection AST node.
 * @param struct_name The name of the nested struct.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT int sql_to_c_projection_nested_struct_emit(
    FILE *fp, const cdd_c_query_projection_t *proj, const char *struct_name);

/**
 * @brief Generate a 1-to-Many nested C array struct for a given query
 * projection representing a JOIN.
 *
 * @param fp File pointer to write output to.
 * @param proj The query projection AST node.
 * @param struct_name The name of the nested struct.
 * @param array_name The name of the array type wrapper.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT int sql_to_c_projection_nested_array_emit(
    FILE *fp, const cdd_c_query_projection_t *proj, const char *struct_name,
    const char *array_name);

/**
 * @brief Generate a bitmask struct to track dirtied fields for a given
 * projection.
 *
 * @param fp File pointer to write output to.
 * @param proj The query projection AST node.
 * @param struct_name The name of the projection struct to generate the mask
 * for.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT int sql_to_c_projection_dirty_bitmask_emit(
    FILE *fp, const cdd_c_query_projection_t *proj, const char *struct_name);

/**
 * @brief Generate a variant struct for a SQL UNION projection.
 *
 * @param fp File pointer to write output to.
 * @param projs Array of query projection AST nodes representing union branches.
 * @param n_projs Number of projection branches.
 * @param struct_name The name of the union struct.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT int
sql_to_c_projection_union_struct_emit(FILE *fp,
                                      const cdd_c_query_projection_t *projs,
                                      size_t n_projs, const char *struct_name);

/**
 * @brief Generate a polymorphic struct projection output.
 *
 * @param fp File pointer to write output to.
 * @param proj The query projection AST node.
 * @param struct_name The name of the struct.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT int sql_to_c_projection_polymorphic_struct_emit(
    FILE *fp, const cdd_c_query_projection_t *proj, const char *struct_name);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_SQL_TO_C_H */