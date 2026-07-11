/**
 * @file schema.c
 * @brief Implementation of schema constraint handling.
 */

/* clang-format off */
#include "c_cdd_export.h"
#include "classes/emit/schema.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "functions/parse/str.h"
#include "c_cdd/log.h"
/* clang-format on */

enum cdd_c_error schema_constraints_init(struct SchemaConstraints *sc) {
  if (!sc)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  sc->required = NULL;
  sc->required_count = 0;
  sc->required_capacity = 0;
  sc->has_additional_properties = 0;
  sc->additional_properties = NULL;
  return CDD_C_SUCCESS;
}

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_schema_strdup_fail = 0;
C_CDD_EXPORT int g_schema_realloc_fail = 0;
#endif

enum cdd_c_error schema_constraints_add_required(struct SchemaConstraints *sc,
                                                 const char *field) {
  if (!sc || !field)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (sc->required_count >= sc->required_capacity) {
    size_t new_cap = sc->required_capacity == 0 ? 8 : sc->required_capacity * 2;
    char **new_req;
    if (new_cap < sc->required_capacity ||
        new_cap > ((size_t)-1) / (2 * sizeof(char *))) {
      return CDD_C_ERROR_MEMORY;
    }
#ifdef CDD_BUILD_TESTS
    if (g_schema_realloc_fail) {
      new_req = NULL;
    } else {
#endif
      new_req = (char **)realloc(sc->required, new_cap * sizeof(char *));
#ifdef CDD_BUILD_TESTS
    }
#endif
    if (!new_req) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    sc->required = new_req;
    sc->required_capacity = new_cap;
  }
#ifdef CDD_BUILD_TESTS
  if (g_schema_strdup_fail) {
    sc->required[sc->required_count] = NULL;
  } else {
#endif
    c_cdd_strdup(field, &sc->required[sc->required_count]);
#ifdef CDD_BUILD_TESTS
  }
#endif
  if (!sc->required[sc->required_count])
    return CDD_C_ERROR_MEMORY;

  sc->required_count++;
  return CDD_C_SUCCESS;
}

void schema_constraints_free(struct SchemaConstraints *sc) {
  size_t i;
  if (!sc)
    return;
  if (sc->required) {
    for (i = 0; i < sc->required_count; i++) {
      if (sc->required[i])
        free(sc->required[i]);
    }
    free(sc->required);
    sc->required = NULL;
  }
  sc->required_count = 0;
  sc->required_capacity = 0;

  if (sc->additional_properties) {
    if (sc->additional_properties->name)
      free(sc->additional_properties->name);
    if (sc->additional_properties->type)
      free(sc->additional_properties->type);
    if (sc->additional_properties->ref)
      free(sc->additional_properties->ref);
    free(sc->additional_properties);
    sc->additional_properties = NULL;
  }
  sc->has_additional_properties = 0;
}
