/* clang-format off */
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#else
#include <errno.h>
#endif

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) && !defined(strdup)
#define strdup _strdup
#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) */

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(__WINDOWS__)
#ifndef strdup
char *strdup(const char *s);
#endif
#endif

#include <parson.h>

#include <c89stringutils_string_extras.h>

#include "simple_json.h"

/* clang-format on */

/** \brief func */
/**
 * @brief Executes the quote or null operation.
 */
static enum cdd_c_error quote_or_null(const char *s, char **s1) {
  if (s == NULL) {
    *s1 = strdup("(null)");
    if (*s1 == NULL)
      return CDD_C_ERROR_MEMORY;
    return CDD_C_SUCCESS;
  }
  {
    const size_t n = strlen(s);
    size_t i;
    *s1 = malloc((n + 3) * sizeof(char));
    if (*s1 == NULL)
      return CDD_C_ERROR_MEMORY;
    (*s1)[0] = '"';
    for (i = 0; i < n; ++i)
      (*s1)[i + 1] = s[i];
    (*s1)[n + 1] = '"';
    (*s1)[n + 2] = '\0';
  }
  return CDD_C_SUCCESS;
}

/** \brief func */
/**
 * @brief Executes the c str eq operation.
 */
static enum cdd_c_error c_str_eq(const char *s0, const char *s1) {
  return ((s0 == NULL && s1 == NULL) ||
          (s0 != NULL && s1 != NULL && strcmp(s0, s1) == 0))
             ? 0
             : 1;
}

/** \brief func */
/**
 * @brief Executes the Tank default operation.
 */
enum cdd_c_error Tank_default(enum Tank *out) {
  if (out)
    *out = Tank_BIG;
  return CDD_C_SUCCESS;
}

/** \brief func */
/**
 * @brief Executes the Tank to str operation.
 */
enum cdd_c_error Tank_to_str(const enum Tank tank, char **const str) {
  if (str == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  switch (tank) {
  case Tank_BIG:
    *str = strdup("BIG");
    if (!str)
      return CDD_C_ERROR_MEMORY;
    break;
  case Tank_SMALL:
    *str = strdup("SMALL");
    if (!str)
      return CDD_C_ERROR_MEMORY;
    break;
  case Tank_UNKNOWN:
  default:
    *str = strdup("UNKNOWN");
    if (!str)
      return CDD_C_ERROR_MEMORY;
  }

  if (*str == NULL)
    return CDD_C_ERROR_MEMORY;

  return CDD_C_SUCCESS;
}

/** \brief func */
/**
 * @brief Executes the Tank from str operation.
 */
enum cdd_c_error Tank_from_str(const char *str, enum Tank *val) {
  if (val == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  else if (str == NULL)
    *val = Tank_UNKNOWN;
  else if (strcmp(str, "BIG") == 0)
    *val = Tank_BIG;
  else if (strcmp(str, "SMALL") == 0)
    *val = Tank_SMALL;
  else if (strcmp(str, "UNKNOWN") == 0)
    *val = Tank_UNKNOWN;
  else
    *val = Tank_UNKNOWN;
  return CDD_C_SUCCESS;
}

/** \brief func */
/**
 * @brief Executes the HazE cleanup operation.
 */
void HazE_cleanup(struct HazE *haz_e) {
  if (haz_e == NULL)
    return;

  free((void *)haz_e->bzr);
  free(haz_e);
}

/** \brief func */
/**
 * @brief Executes the HazE default operation.
 */
enum cdd_c_error HazE_default(struct HazE **haz_e) {
  if (haz_e == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *haz_e = malloc(sizeof(**haz_e));
  if (*haz_e == NULL)
    return CDD_C_ERROR_MEMORY;
  Tank_default(&(*haz_e)->tank);
  (*haz_e)->bzr = NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the HazE deepcopy operation.
 */
enum cdd_c_error HazE_deepcopy(const struct HazE *haz_e_original,
                               struct HazE **haz_e_dest) {
  if (haz_e_dest == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (haz_e_original == NULL) {
    *haz_e_dest = NULL;
    return CDD_C_SUCCESS;
  }
  *haz_e_dest = malloc(sizeof(**haz_e_dest));
  if (*haz_e_dest == NULL)
    return CDD_C_ERROR_MEMORY;

  if (haz_e_original->bzr == NULL) {
    (*haz_e_dest)->bzr = NULL;
  } else {
    (*haz_e_dest)->bzr = strdup(haz_e_original->bzr);
    if ((*haz_e_dest)->bzr == NULL) {
      free(*haz_e_dest);
      *haz_e_dest = NULL;
      return CDD_C_ERROR_MEMORY;
    }
  }
  (*haz_e_dest)->tank = haz_e_original->tank;
  return CDD_C_SUCCESS;
}

/** \brief func */
/**
 * @brief Executes the HazE display operation.
 */
enum cdd_c_error HazE_display(const struct HazE *haz_e, FILE *fh) {
  char *s = NULL;
  int rc = HazE_to_json(haz_e, &s);
  if (rc != 0)
    return rc;
  rc = fprintf(fh, "%s\n", s);
  if (rc > 0)
    rc = 0;
  free(s);
  return rc;
}

/** \brief func */
/**
 * @brief Executes the HazE debug operation.
 */
enum cdd_c_error HazE_debug(const struct HazE *haz_e, FILE *fh) {
  int rc;
  if (haz_e == NULL) {
    rc = fputs("<null HazE>\n", fh);
    return rc < 0 ? rc : 0;
  }
  rc = fputs("struct HazE dbg = {\n", fh);
  if (rc < 0)
    return rc;
  {
    char *quoted = NULL;
    rc = quote_or_null(haz_e->bzr, &quoted);
    if (rc != 0)
      return rc;
    rc = fprintf(fh, "  /* const char * */ bzr = %s,\n", quoted);
    free(quoted);
    if (rc < 0)
      return rc;
  }
  rc = fprintf(fh, "  /* enum Tank */ tank = %d\n", haz_e->tank);
  if (rc < 0)
    return rc;
  rc = fputs("};\n", fh);
  return rc < 0 ? rc : 0;
}

/** \brief func */
/**
 * @brief Executes the HazE eq operation.
 */
enum cdd_c_error HazE_eq(const struct HazE *haz_e0, const struct HazE *haz_e1) {
  if (haz_e0 == NULL || haz_e1 == NULL)
    return haz_e0 == haz_e1 ? 0 : 1;

  if (haz_e0->tank != haz_e1->tank)
    return CDD_C_ERROR_UNKNOWN;

  return c_str_eq(haz_e0->bzr, haz_e1->bzr);
}

/** \brief func */
/**
 * @brief Executes the HazE to json operation.
 */
enum cdd_c_error HazE_to_json(const struct HazE *haz_e, char **json) {
  char *tank_str = NULL;
  int rc = 0;
  int need_comma = 0;

  if (json == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (haz_e == NULL) {
    c89stringutils_jasprintf(json, "null");
    return *json == NULL ? ENOMEM : 0;
  }

  c89stringutils_jasprintf(json, "{");
  if (*json == NULL)
    return CDD_C_ERROR_MEMORY;

  if (haz_e->bzr) {
    c89stringutils_jasprintf(json, "\"bzr\": \"%s\"", haz_e->bzr);
    need_comma = 1;
  } else {
    c89stringutils_jasprintf(json, "\"bzr\": null");
    need_comma = 1;
  }
  if (*json == NULL)
    return CDD_C_ERROR_MEMORY;

  if (need_comma) {
    c89stringutils_jasprintf(json, ",");
    if (*json == NULL)
      return CDD_C_ERROR_MEMORY;
  }
  if (Tank_to_str(haz_e->tank, &tank_str) != 0) {
    rc = ENOMEM;
    goto cleanup;
  }
  c89stringutils_jasprintf(json, "\"tank\": \"%s\"", tank_str);
  if (*json == NULL) {
    rc = ENOMEM;
    goto cleanup;
  }

  c89stringutils_jasprintf(json, "}");
  if (*json == NULL) {
    rc = ENOMEM;
  }

cleanup:
  free(tank_str);
  return rc;
}

/**
 * @brief Executes the HazE from jsonObject operation.
 */
enum cdd_c_error HazE_from_jsonObject(const JSON_Object *jsonObject,
                                      struct HazE **haz_e) {
  const char *bzr_str = NULL;
  const char *tank_str;
  int rc = 0;
  enum Tank tank_val;
  struct HazE *new_haz;

  if (jsonObject == NULL || haz_e == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  tank_str = json_object_get_string(jsonObject, "tank");
  if (tank_str == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = Tank_from_str(tank_str, &tank_val);
  if (rc != 0)
    return rc;

  new_haz = malloc(sizeof(*new_haz));
  if (new_haz == NULL)
    return CDD_C_ERROR_MEMORY;

  bzr_str = json_object_get_string(jsonObject, "bzr");
  if (bzr_str) {
    new_haz->bzr = strdup(bzr_str);
    if (new_haz->bzr == NULL) {
      free(new_haz);
      return CDD_C_ERROR_MEMORY;
    }
  } else {
    new_haz->bzr = NULL;
  }

  new_haz->tank = tank_val;
  *haz_e = new_haz;
  return CDD_C_SUCCESS;
}

/** \brief func */
/**
 * @brief Executes the HazE from json operation.
 */
enum cdd_c_error HazE_from_json(const char *json, struct HazE **haz_e) {
  JSON_Value *root = NULL;
  const JSON_Object *jsonObject = NULL;
  int rc;
  if (json == NULL || haz_e == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  root = json_parse_string(json);
  if (root == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  jsonObject = json_value_get_object(root);
  if (jsonObject == NULL) {
    json_value_free(root);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rc = HazE_from_jsonObject(jsonObject, haz_e);
  json_value_free(root);
  return rc;
}

/** \brief func */
/**
 * @brief Executes the FooE cleanup operation.
 */
void FooE_cleanup(struct FooE *foo_e) {
  if (foo_e == NULL)
    return;
  free((void *)foo_e->bar);
  HazE_cleanup(foo_e->haz);
  free(foo_e);
}

/** \brief func */
/**
 * @brief Executes the FooE default operation.
 */
enum cdd_c_error FooE_default(struct FooE **foo_e) {
  int rc;
  if (foo_e == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *foo_e = malloc(sizeof(**foo_e));
  if (*foo_e == NULL)
    return CDD_C_ERROR_MEMORY;

  memset(*foo_e, 0, sizeof(**foo_e));

  rc = HazE_default(&(*foo_e)->haz);
  if (rc != 0) {
    free(*foo_e);
    return CDD_C_ERROR_MEMORY;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the FooE deepcopy operation.
 */
enum cdd_c_error FooE_deepcopy(const struct FooE *foo_e_original,
                               struct FooE **foo_e_dest) {
  struct FooE *new_foo;
  if (foo_e_dest == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (foo_e_original == NULL) {
    *foo_e_dest = NULL;
    return CDD_C_SUCCESS;
  }

  new_foo = malloc(sizeof(*new_foo));
  if (new_foo == NULL)
    return CDD_C_ERROR_MEMORY;
  memset(new_foo, 0, sizeof(*new_foo));

  if (foo_e_original->bar != NULL) {
    new_foo->bar = strdup(foo_e_original->bar);
    if (new_foo->bar == NULL) {
      free(new_foo);
      return CDD_C_ERROR_MEMORY;
    }
  }

  new_foo->can = foo_e_original->can;

  if (HazE_deepcopy(foo_e_original->haz, &new_foo->haz) != 0) {
    FooE_cleanup(new_foo);
    return CDD_C_ERROR_MEMORY;
  }

  *foo_e_dest = new_foo;
  return CDD_C_SUCCESS;
}

/** \brief func */
/**
 * @brief Executes the FooE display operation.
 */
enum cdd_c_error FooE_display(const struct FooE *foo_e, FILE *fh) {
  char *s = NULL;
  int rc = FooE_to_json(foo_e, &s);
  if (rc != 0)
    return rc;
  rc = fprintf(fh, "%s\n", s);
  if (rc > 0)
    rc = 0;
  free(s);
  return rc;
}

/** \brief func */
/**
 * @brief Executes the FooE debug operation.
 */
enum cdd_c_error FooE_debug(const struct FooE *foo_e, FILE *fh) {
  int rc;
  if (foo_e == NULL) {
    rc = fputs("<null FooE>\n", fh);
    return rc < 0 ? rc : 0;
  }
  rc = fputs("struct FooE dbg = {\n", fh);
  if (rc < 0)
    return rc;

  {
    char *quoted = NULL;
    rc = quote_or_null(foo_e->bar, &quoted);
    if (rc != 0)
      return rc;
    rc = fprintf(fh, "  /* const char * */ bar = %s,\n", quoted);
    free(quoted);
    if (rc < 0)
      return rc;
  }
  rc = fprintf(fh, "  /* int can */ can = %d,\n", foo_e->can);
  if (rc < 0)
    return rc;

  rc = HazE_debug(foo_e->haz, fh);
  if (rc < 0)
    return rc;

  rc = fputs("};\n", fh);
  return rc < 0 ? rc : 0;
}

/** \brief func */
/**
 * @brief Executes the FooE eq operation.
 */
enum cdd_c_error FooE_eq(const struct FooE *foo_e0, const struct FooE *foo_e1) {
  if (foo_e0 == NULL || foo_e1 == NULL)
    return foo_e0 == foo_e1 ? 0 : 1;

  return (foo_e0->can == foo_e1->can &&
          c_str_eq(foo_e0->bar, foo_e1->bar) == 0 &&
          HazE_eq(foo_e0->haz, foo_e1->haz) == 0)
             ? 0
             : 1;
}

/** \brief func */
/**
 * @brief Executes the FooE to json operation.
 */
enum cdd_c_error FooE_to_json(const struct FooE *foo_e, char **const json) {
  char *haz_e_json = NULL;
  int rc = 0;

  if (json == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (foo_e == NULL) {
    c89stringutils_jasprintf(json, "null");
    return *json == NULL ? ENOMEM : 0;
  }

  c89stringutils_jasprintf(json, "{");
  if (*json == NULL)
    return CDD_C_ERROR_MEMORY;

  if (foo_e->bar) {
    c89stringutils_jasprintf(json, "\"bar\": \"%s\",", foo_e->bar);
  } else {
    c89stringutils_jasprintf(json, "\"bar\": null,");
  }
  if (*json == NULL)
    return CDD_C_ERROR_MEMORY;

  c89stringutils_jasprintf(json, "\"can\": %d,", foo_e->can);
  if (*json == NULL)
    return CDD_C_ERROR_MEMORY;

  if (HazE_to_json(foo_e->haz, &haz_e_json) != 0) {
    rc = ENOMEM;
    goto cleanup;
  }

  c89stringutils_jasprintf(json, "\"haz\":%s", haz_e_json);
  if (*json == NULL) {
    rc = ENOMEM;
    goto cleanup;
  }

  c89stringutils_jasprintf(json, "}");
  if (*json == NULL) {
    rc = ENOMEM;
  }

cleanup:
  free(haz_e_json);
  return rc;
}

/**
 * @brief Executes the FooE from jsonObject operation.
 */
enum cdd_c_error FooE_from_jsonObject(const JSON_Object *jsonObject,
                                      struct FooE **const foo_e) {
  int rc = 0;
  const char *bar_str = NULL;
  const JSON_Object *haz_obj = NULL;
  struct FooE *new_foo;

  if (jsonObject == NULL || foo_e == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  new_foo = calloc(1, sizeof(*new_foo));
  if (new_foo == NULL)
    return CDD_C_ERROR_MEMORY;

  bar_str = json_object_get_string(jsonObject, "bar");
  if (bar_str) {
    new_foo->bar = strdup(bar_str);
    if (new_foo->bar == NULL) {
      free(new_foo);
      return CDD_C_ERROR_MEMORY;
    }
  }

  new_foo->can = (int)json_object_get_number(jsonObject, "can");

  haz_obj = json_object_get_object(jsonObject, "haz");
  if (haz_obj != NULL) {
    rc = HazE_from_jsonObject(haz_obj, &new_foo->haz);
    if (rc != 0) {
      FooE_cleanup(new_foo);
      return rc;
    }
  }

  *foo_e = new_foo;
  return rc;
}

/** \brief func */
/**
 * @brief Executes the FooE from json operation.
 */
enum cdd_c_error FooE_from_json(const char *json, struct FooE **const foo_e) {
  JSON_Value *root = NULL;
  const JSON_Object *jsonObject = NULL;
  int rc;

  if (json == NULL || foo_e == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  root = json_parse_string(json);
  if (root == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  jsonObject = json_value_get_object(root);
  if (jsonObject == NULL) {
    json_value_free(root);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rc = FooE_from_jsonObject(jsonObject, foo_e);
  json_value_free(root);
  return rc;
}
