#include "c_cdd/safe_crt.h"
#include "cdd_c_error.h"
#ifndef va_copy
#if defined(_MSC_VER) || defined(__GNUC__)
#define va_copy(dest, src) (dest = src)
#else
#define va_copy(dest, src) memcpy(&dest, &src, sizeof(va_list))
#endif
#endif

/* clang-format off */#include "c_cdd/safe_crt_msvc.h"

#include <c89stringutils_string_extras.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#define c89stringutils_jasprintf jasprintf

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
#include "simple_mocks_export.h"

#ifdef CDD_BUILD_TESTS
SIMPLE_MOCKS_EXPORT int g_simple_json_fail_alloc = 0;

static void *test_malloc(size_t size) {
  if (g_simple_json_fail_alloc > 0) {
    g_simple_json_fail_alloc--;
    if (g_simple_json_fail_alloc == 0)
      return NULL;
  }
  return malloc(size);
}

static void *test_calloc(size_t count, size_t size) {
  if (g_simple_json_fail_alloc > 0) {
    g_simple_json_fail_alloc--;
    if (g_simple_json_fail_alloc == 0)
      return NULL;
  }
  return calloc(count, size);
}

static char *test_strdup(const char *s) {
  if (g_simple_json_fail_alloc > 0) {
    g_simple_json_fail_alloc--;
    if (g_simple_json_fail_alloc == 0)
      return NULL;
  }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  return _strdup(s);
#else
  return strdup(s);
#endif
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 2, 3)))
#endif
static int test_jasprintf(char **unto, const char *fmt, ...) {
  va_list args;
  char *new_part = NULL;
  int rc;

  if (g_simple_json_fail_alloc > 0) {
    g_simple_json_fail_alloc--;
    if (g_simple_json_fail_alloc == 0) {
      if (*unto) {
        free(*unto);
        *unto = NULL;
      }
      return -1;
    }
  }

  va_start(args, fmt);
  c89stringutils_vasprintf(&new_part, fmt, args);
  va_end(args);

  rc = c89stringutils_jasprintf(unto, "%s", new_part);
  free(new_part);
  return rc;
}

#undef malloc
#define malloc test_malloc
#undef calloc
#define calloc test_calloc
#undef strdup
#define strdup test_strdup
#undef c89stringutils_jasprintf
#define c89stringutils_jasprintf test_jasprintf
#endif

/* clang-format on */

static cdd_c_error_t quote_or_null(const char *s, char **s1) {
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

static cdd_c_error_t c_str_eq(const char *s0, const char *s1) {
  return ((s0 == NULL && s1 == NULL) ||
          (s0 != NULL && s1 != NULL && strcmp(s0, s1) == 0))
             ? 0
             : 1;
}

cdd_c_error_t Tank_default(enum Tank *out) {
  if (out)
    *out = Tank_BIG;
  return CDD_C_SUCCESS;
}

cdd_c_error_t Tank_to_str(const enum Tank tank, char **str) {
  if (str == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  switch (tank) {
  case Tank_BIG:
    *str = strdup("BIG");
    break;
  case Tank_SMALL:
    *str = strdup("SMALL");
    break;
  case Tank_UNKNOWN:
  default:
    *str = strdup("UNKNOWN");
    break;
  }

  if (*str == NULL)
    return CDD_C_ERROR_MEMORY;
  return CDD_C_SUCCESS;
}

cdd_c_error_t Tank_from_str(const char *str, enum Tank *val) {
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

cdd_c_error_t HazE_cleanup(struct HazE *haz_e) {
  if (haz_e == NULL)
    return CDD_C_SUCCESS;

  free((void *)(size_t)haz_e->bzr);
  free(haz_e);
  return CDD_C_SUCCESS;
}

cdd_c_error_t HazE_default(struct HazE **haz_e) {
  if (haz_e == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *haz_e = malloc(sizeof(**haz_e));
  if (*haz_e == NULL)
    return CDD_C_ERROR_MEMORY;
  (*haz_e)->tank = Tank_BIG;
  (*haz_e)->bzr = NULL;
  return CDD_C_SUCCESS;
}

cdd_c_error_t HazE_deepcopy(const struct HazE *haz_e_original,
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

cdd_c_error_t HazE_display(const struct HazE *haz_e, FILE *fh) {
  char *s = NULL;
  cdd_c_error_t rc = HazE_to_json(haz_e, &s);
  if (rc != CDD_C_SUCCESS) {
    free(s);
    return rc;
  }
  rc = (cdd_c_error_t)fprintf(fh, "%s\n", s);
  if (rc > 0)
    rc = CDD_C_SUCCESS;
  free(s);
  return rc;
}

cdd_c_error_t HazE_debug(const struct HazE *haz_e, FILE *fh) {
  cdd_c_error_t rc;
  if (haz_e == NULL) {
    rc = (cdd_c_error_t)fputs("<null HazE>\n", fh);
    return rc < 0 ? rc : CDD_C_SUCCESS;
  }
  rc = (cdd_c_error_t)fputs("struct HazE dbg = {\n", fh);
  if (rc < 0)
    return rc;
  {
    char *quoted = NULL;
    rc = quote_or_null(haz_e->bzr, &quoted);
    if (rc != CDD_C_SUCCESS)
      return rc;
    fprintf(fh, "  /* const char * */ bzr = %s,\n", quoted);
    free(quoted);
  }
  fprintf(fh, "  /* enum Tank */ tank = %d\n", (int)haz_e->tank);
  rc = (cdd_c_error_t)fputs("};\n", fh);
  return rc < 0 ? rc : CDD_C_SUCCESS;
}

cdd_c_error_t HazE_eq(const struct HazE *haz_e0, const struct HazE *haz_e1) {
  if (haz_e0 == NULL || haz_e1 == NULL)
    return haz_e0 == haz_e1 ? 0 : 1;

  if (haz_e0->tank != haz_e1->tank)
    return CDD_C_ERROR_UNKNOWN;

  return c_str_eq(haz_e0->bzr, haz_e1->bzr);
}

cdd_c_error_t HazE_to_json(const struct HazE *haz_e, char **json) {
  char *tank_str = NULL;
  int rc = 0;

  if (json == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (haz_e == NULL) {
    c89stringutils_jasprintf(json, "null");
    return *json == NULL ? CDD_C_ERROR_MEMORY : 0;
  }

  c89stringutils_jasprintf(json, "{");
  if (*json == NULL) {
    rc = CDD_C_ERROR_MEMORY;
    goto cleanup;
  }

  if (haz_e->bzr) {
    c89stringutils_jasprintf(json, "\"bzr\": \"%s\"", haz_e->bzr);
  } else {
    c89stringutils_jasprintf(json, "\"bzr\": null");
  }
  if (*json == NULL) {
    rc = CDD_C_ERROR_MEMORY;
    goto cleanup;
  }

  c89stringutils_jasprintf(json, ",");
  if (*json == NULL)
    goto cleanup;

  {
    rc = Tank_to_str(haz_e->tank, &tank_str);
    if (rc != CDD_C_SUCCESS) {
      goto cleanup;
    }
  }
  c89stringutils_jasprintf(json, "\"tank\": \"%s\"", tank_str);
  if (*json == NULL) {
    rc = CDD_C_ERROR_MEMORY;
    goto cleanup;
  }

  c89stringutils_jasprintf(json, "}");
  if (*json == NULL) {
  }

cleanup:
  free(tank_str);
  return rc;
}

cdd_c_error_t HazE_from_jsonObject(const JSON_Object *jsonObject,
                                   struct HazE **haz_e) {
  const char *bzr_str = NULL;
  const char *tank_str;
  enum Tank tank_val;
  struct HazE *new_haz;

  if (jsonObject == NULL || haz_e == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  tank_str = json_object_get_string(jsonObject, "tank");
  if (tank_str == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  Tank_from_str(tank_str, &tank_val);

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

cdd_c_error_t HazE_from_json(const char *json, struct HazE **haz_e) {
  JSON_Value *root = NULL;
  const JSON_Object *jsonObject = NULL;
  int rc;
  (void)rc;
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

cdd_c_error_t FooE_cleanup(struct FooE *foo_e) {
  if (foo_e == NULL)
    return CDD_C_SUCCESS;
  free((void *)(size_t)foo_e->bar);
  HazE_cleanup(foo_e->haz);
  free(foo_e);
  return CDD_C_SUCCESS;
}

cdd_c_error_t FooE_default(struct FooE **foo_e) {
  int rc;
  (void)rc;
  if (foo_e == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *foo_e = malloc(sizeof(**foo_e));
  if (*foo_e == NULL)
    return CDD_C_ERROR_MEMORY;

  memset(*foo_e, 0, sizeof(**foo_e));

  rc = HazE_default(&(*foo_e)->haz);
  if (rc != CDD_C_SUCCESS) {
    free(*foo_e);
    *foo_e = NULL;
    return rc;
  }
  return CDD_C_SUCCESS;
}

cdd_c_error_t FooE_deepcopy(const struct FooE *foo_e_original,
                            struct FooE **foo_e_dest) {
  struct FooE *new_foo;
  if (foo_e_dest == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (foo_e_original == NULL) {
    *foo_e_dest = NULL;
    return CDD_C_SUCCESS;
  }

  new_foo = calloc(1, sizeof(*new_foo));
  if (new_foo == NULL)
    return CDD_C_ERROR_MEMORY;

  if (foo_e_original->bar != NULL) {
    new_foo->bar = strdup(foo_e_original->bar);
    if (new_foo->bar == NULL) {
      free(new_foo);
      return CDD_C_ERROR_MEMORY;
    }
  }

  new_foo->can = foo_e_original->can;

  if (foo_e_original->haz) {
    cdd_c_error_t deep_rc = HazE_deepcopy(foo_e_original->haz, &new_foo->haz);
    if (deep_rc != CDD_C_SUCCESS) {
      FooE_cleanup(new_foo);
      return deep_rc;
    }
  }

  *foo_e_dest = new_foo;
  return CDD_C_SUCCESS;
}

cdd_c_error_t FooE_display(const struct FooE *foo_e, FILE *fh) {
  char *s = NULL;
  cdd_c_error_t rc = FooE_to_json(foo_e, &s);
  if (rc != CDD_C_SUCCESS) {
    free(s);
    return rc;
  }
  rc = (cdd_c_error_t)fprintf(fh, "%s\n", s);
  if (rc > 0)
    rc = CDD_C_SUCCESS;
  free(s);
  return rc;
}

cdd_c_error_t FooE_debug(const struct FooE *foo_e, FILE *fh) {
  cdd_c_error_t rc;
  if (foo_e == NULL) {
    rc = (cdd_c_error_t)fputs("<null FooE>\n", fh);
    return rc < 0 ? rc : CDD_C_SUCCESS;
  }
  rc = (cdd_c_error_t)fputs("struct FooE dbg = {\n", fh);
  if (rc < 0)
    return rc;

  {
    char *quoted = NULL;
    rc = quote_or_null(foo_e->bar, &quoted);
    if (rc != CDD_C_SUCCESS)
      return rc;
    fprintf(fh, "  /* const char * */ bar = %s,\n", quoted);
    free(quoted);
  }
  fprintf(fh, "  /* int can */ can = %d,\n", foo_e->can);

  fputs("  /* struct HazE * */ haz = ", fh);
  rc = HazE_debug(foo_e->haz, fh);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = (cdd_c_error_t)fputs("};\n", fh);
  return rc < 0 ? rc : CDD_C_SUCCESS;
}

cdd_c_error_t FooE_eq(const struct FooE *foo_e0, const struct FooE *foo_e1) {
  if (foo_e0 == NULL || foo_e1 == NULL)
    return foo_e0 == foo_e1 ? 0 : 1;

  return (foo_e0->can == foo_e1->can &&
          c_str_eq(foo_e0->bar, foo_e1->bar) == 0 &&
          HazE_eq(foo_e0->haz, foo_e1->haz) == 0)
             ? 0
             : 1;
}

cdd_c_error_t FooE_to_json(const struct FooE *foo_e, char **json) {
  char *haz_e_json = NULL;
  int rc = 0;

  if (json == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (foo_e == NULL) {
    c89stringutils_jasprintf(json, "null");
    return *json == NULL ? CDD_C_ERROR_MEMORY : 0;
  }

  c89stringutils_jasprintf(json, "{");
  if (*json == NULL) {
    rc = CDD_C_ERROR_MEMORY;
    goto cleanup;
  }

  if (foo_e->bar) {
    c89stringutils_jasprintf(json, "\"bar\": \"%s\",", foo_e->bar);
  } else {
    c89stringutils_jasprintf(json, "\"bar\": null,");
  }
  if (*json == NULL) {
    rc = CDD_C_ERROR_MEMORY;
    goto cleanup;
  }

  c89stringutils_jasprintf(json, "\"can\": %d,", foo_e->can);
  if (*json == NULL) {
    rc = CDD_C_ERROR_MEMORY;
    goto cleanup;
  }

  if (HazE_to_json(foo_e->haz, &haz_e_json) != 0) {
    goto cleanup;
  }

  c89stringutils_jasprintf(json, "\"haz\":%s", haz_e_json);
  if (*json == NULL) {
    rc = CDD_C_ERROR_MEMORY;
    goto cleanup;
  }

  c89stringutils_jasprintf(json, "}");
  if (*json == NULL) {
  }

cleanup:
  free(haz_e_json);
  return rc;
}

cdd_c_error_t FooE_from_jsonObject(const JSON_Object *jsonObject,
                                   struct FooE **foo_e) {
  int rc = 0;
  const char *bar_str = NULL;
  const JSON_Object *haz_obj = NULL;
  struct FooE *new_foo;

  if (jsonObject == NULL || foo_e == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  new_foo = (struct FooE *)calloc(1, sizeof(*new_foo));
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

cdd_c_error_t FooE_from_json(const char *json, struct FooE **foo_e) {
  JSON_Value *root = NULL;
  const JSON_Object *jsonObject = NULL;
  int rc;

  (void)rc;
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

cdd_c_error_t run_mocks_test(void) {
  const enum Tank t = Tank_BIG;
  cdd_c_error_t rc;
  char *tank_as_str = NULL;
  struct HazE haz_e;
  struct FooE foo_e;
  char *haz_e_json = NULL;
  char *foo_e_json = NULL;
  struct HazE *haz_e0 = NULL;
  struct FooE *foo_e0 = NULL;

  static const char *const haz_e_mock0 =
      "{\"bzr\": \"some_bzr\",\"tank\": \"SMALL\"}";
  static const char *const foo_e_mock0 =
      "{\"bar\": \"some_bar\",\"can\": 5,\"haz\":{\"bzr\": "
      "\"some_bzr\",\"tank\": \"SMALL\"}}";

  haz_e.bzr = "some_bzr";
  haz_e.tank = Tank_SMALL;

  foo_e.bar = "some_bar";
  foo_e.can = 5;
  foo_e.haz = &haz_e;

  rc = Tank_to_str(t, &tank_as_str);
  if (rc != CDD_C_SUCCESS)
    return rc;
  free(tank_as_str);

  rc = HazE_to_json(&haz_e, &haz_e_json);
  if (rc != CDD_C_SUCCESS)
    return rc;
  free(haz_e_json);

  rc = HazE_from_json(haz_e_mock0, &haz_e0);
  if (rc != CDD_C_SUCCESS)
    return rc;
  HazE_eq(haz_e0, &haz_e);
  HazE_cleanup(haz_e0);

  rc = FooE_to_json(&foo_e, &foo_e_json);
  if (rc != CDD_C_SUCCESS)
    return rc;
  free(foo_e_json);

  rc = FooE_from_json(foo_e_mock0, &foo_e0);
  if (rc != CDD_C_SUCCESS)
    return rc;
  FooE_eq(foo_e0, &foo_e);
  FooE_cleanup(foo_e0);

  return CDD_C_SUCCESS;
}
