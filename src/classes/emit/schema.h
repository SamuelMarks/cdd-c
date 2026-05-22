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
#include <stdio.h>
/* clang-format on */

/**
 * @brief Schema type wrapper.
 */
struct SchemaType {
  char *name;
  char *type;
  char *ref;
};

/**
 * @brief Represents schema validation constraints.
 */
struct SchemaConstraints {
  char **required;
  size_t required_count;
  size_t required_capacity;
  int has_additional_properties;
  struct SchemaType *additional_properties;
};

/**
 * @brief Initializes schema constraints.
 * @param sc Pointer to constraints struct.
 * @return 0 on success, error code otherwise.
 */
extern C_CDD_EXPORT int schema_constraints_init(struct SchemaConstraints *sc);

/**
 * @brief Adds a required field to schema constraints.
 * @param sc Pointer to constraints struct.
 * @param field Field name.
 * @return 0 on success, error code otherwise.
 */
extern C_CDD_EXPORT int
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
