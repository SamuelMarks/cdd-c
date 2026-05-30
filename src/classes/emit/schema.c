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

int schema_constraints_init(struct SchemaConstraints *sc) {
  if (!sc)
    return EINVAL;
  sc->required = NULL;
  sc->required_count = 0;
  sc->required_capacity = 0;
  sc->has_additional_properties = 0;
  sc->additional_properties = NULL;
  return 0;
}

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_schema_strdup_fail = 0;
#endif

int schema_constraints_add_required(struct SchemaConstraints *sc,
                                    const char *field) {
  if (!sc || !field)
    return EINVAL;

  if (sc->required_count >= sc->required_capacity) {
    size_t new_cap = sc->required_capacity == 0 ? 8 : sc->required_capacity * 2;
    char **new_req = (char **)realloc(sc->required, new_cap * sizeof(char *));
    if (!new_req) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return ENOMEM;
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
    return ENOMEM;

  sc->required_count++;
  return 0;
}

void schema_constraints_cleanup(struct SchemaConstraints *sc) {
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
