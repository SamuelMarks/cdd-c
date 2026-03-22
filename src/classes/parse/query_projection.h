/**
 * @file query_projection.h
 * @brief CDD_C query projection AST representation
 *
 * Provides cdd_c_query_projection_t and related structs.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CLASSES_PARSE_QUERY_PROJECTION_H
#define C_CDD_CLASSES_PARSE_QUERY_PROJECTION_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stddef.h>
#include "c_cdd_export.h"
#include "classes/parse/sql.h"
/* clang-format on */

/**
 * @brief Represents the target mapping kind.
 */
typedef enum CddCMappingKind {
  CDD_C_MAPPING_KIND_SPECIFIC = 0,
  CDD_C_MAPPING_KIND_ABSTRACT
} cdd_c_mapping_kind_t;

/**
 * @brief Represents mapping metadata for a projection.
 */
typedef struct CddCMappingMetadata {
  char *target_name;
  cdd_c_mapping_kind_t kind;
} cdd_c_mapping_metadata_t;

/**
 * @brief Represents a single field/column in a SQL SELECT projection.
 */
typedef struct CddCQueryProjectionField {
  char *name;            /**< Column name or alias */
  char *original_name;   /**< Original column name if aliased */
  enum SqlDataType type; /**< Inferred type */
  int is_aggregate;      /**< 1 if aggregate (e.g. COUNT), 0 otherwise */
  size_t length; /**< Length bound for varchars/arrays (0 if unbounded) */
  int is_array;  /**< 1 if field is an array, 0 otherwise */
  int is_secure; /**< 1 if SECURE_FIELD tagged, zeroing out memory on free */
} cdd_c_query_projection_field_t;

/**
 * @brief Represents a complete SQL SELECT projection output.
 */
typedef struct CddCQueryProjection {
  cdd_c_query_projection_field_t *fields;
  size_t n_fields;
  size_t capacity;
  char *source_table;                    /**< Primary FROM table */
  cdd_c_mapping_metadata_t mapping_meta; /**< Mapping metadata for codegen */
} cdd_c_query_projection_t;

extern C_CDD_EXPORT /**
                     * @brief Initialize a query projection.
                     * @param proj The projection to initialize.
                     * @return 0 on success, non-zero on error.
                     */
    int
    cdd_c_query_projection_init(cdd_c_query_projection_t *proj);

extern C_CDD_EXPORT /**
                     * @brief Add a field to a query projection.
                     * @param proj The projection.
                     * @param field The field to add.
                     * @return 0 on success, non-zero on error.
                     */
    int
    cdd_c_query_projection_add_field(
        cdd_c_query_projection_t *proj,
        const cdd_c_query_projection_field_t *field);

extern C_CDD_EXPORT /**
                     * @brief Free a query projection.
                     * @param proj The projection to free.
                     * @return 0 on success, non-zero on error.
                     */
    int
    cdd_c_query_projection_free(cdd_c_query_projection_t *proj);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CLASSES_PARSE_QUERY_PROJECTION_H */
