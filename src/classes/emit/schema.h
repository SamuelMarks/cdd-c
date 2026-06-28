/**
 * @file schema.h
 * @brief Schema constraint handling logic for code generation.
 */

#ifndef C_CDD_CODEGEN_SCHEMA_H
#define C_CDD_CODEGEN_SCHEMA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <stdio.h>
/* clang-format on */

/**
 * @brief Schema type wrapper.
 */
struct SchemaType {
  /** @brief name */
  char *name;
  /** @brief type */
  char *type;
  /** @brief ref */
  char *ref;
};

/**
 * @brief Represents schema validation constraints.
 */
struct SchemaConstraints {
  /** @brief required */
  char **required;
  /** @brief required_count */
  size_t required_count;
  /** @brief required_capacity */
  size_t required_capacity;
  /** @brief has_additional_properties */
  int has_additional_properties;
  /** @brief additional_properties */
  struct SchemaType *additional_properties;
};

/**
 * @brief Initializes schema constraints.
 * @param sc Pointer to constraints struct.
 * @return 0 on success, error code otherwise.
 */
extern C_CDD_EXPORT enum cdd_c_error
schema_constraints_init(struct SchemaConstraints *sc);

/**
 * @brief Adds a required field to schema constraints.
 * @param sc Pointer to constraints struct.
 * @param field Field name.
 * @return 0 on success, error code otherwise.
 */
extern C_CDD_EXPORT enum cdd_c_error
schema_constraints_add_required(struct SchemaConstraints *sc,
                                const char *field);

/**
 * @brief Cleans up schema constraints.
 * @param sc Pointer to constraints struct.
 */
extern C_CDD_EXPORT void
schema_constraints_cleanup(struct SchemaConstraints *sc);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CODEGEN_SCHEMA_H */
