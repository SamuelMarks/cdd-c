#include "cdd_c_error.h"
/* clang-format off */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <c89stringutils_string_extras.h>
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
    if (g_simple_json_fail_alloc == 0) return NULL;
  }
  return malloc(size);
}
static void *test_calloc(size_t count, size_t size) {
  if (g_simple_json_fail_alloc > 0) {
    g_simple_json_fail_alloc--;
    if (g_simple_json_fail_alloc == 0) return NULL;
  }
  return calloc(count, size);
}
static char *test_strdup(const char *s) {
  if (g_simple_json_fail_alloc > 0) {
    g_simple_json_fail_alloc--;
    if (g_simple_json_fail_alloc == 0) return NULL;
  }
  return strdup(s);
}
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
static int test_jasprintf(char **strp, const char *fmt, ...) {
  {
    int ret;
    va_list ap;
    va_start(ap, fmt);
    /* Since we only need to test allocation failure, we can just call the real jasprintf.
       However, jasprintf does the allocation. Wait, if we want jasprintf to fail,
       jasprintf is an external function.
       Since jasprintf uses malloc/realloc internally which we haven't overridden
       (we only #defined malloc locally in this file, which doesn't affect the external jasprintf library),
       the external jasprintf won't see our fail_alloc counter!
       So we MUST implement the jasprintf logic here. */
    {
      char *new_str;
      int len;
      va_list ap2;
      va_copy(ap2, ap);
      len = vsnprintf(NULL, 0, fmt, ap2);
      va_end(ap2);
      if (len < 0) {
/* LCOV_EXCL_START */
        va_end(ap);
        return -1;
/* LCOV_EXCL_STOP */
      }
      if (*strp == NULL) {
        new_str = malloc(len + 1);
        if (!new_str) { va_end(ap); return -1; }
        vsnprintf(new_str, len + 1, fmt, ap);
      } else {
        size_t old_len = strlen(*strp);
        new_str = malloc(old_len + len + 1);
        if (!new_str) { free(*strp); *strp = NULL; va_end(ap); return -1; }
        strcpy(new_str, *strp);
        vsnprintf(new_str + old_len, len + 1, fmt, ap);
        free(*strp);
      }
      *strp = new_str;
      ret = len;
    }
    va_end(ap);
    return ret;
    }
    #if defined(__clang__)
    #pragma clang diagnostic pop
    #endif
}
#define malloc test_malloc
#define calloc test_calloc
#undef strdup
#define strdup test_strdup
#undef c89stringutils_jasprintf
#define c89stringutils_jasprintf test_jasprintf
#endif

/* clang-format on */

static enum cdd_c_error quote_or_null(const char *const s, char **s1) {
  if (s == NULL) {
    *s1 = strdup("(null)");
    if (*s1 == NULL)
      return CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */
    return CDD_C_SUCCESS;
  }
  {
    const size_t n = strlen(s);
    size_t i;
    *s1 = malloc((n + 3) * sizeof(char));
    if (*s1 == NULL)
      return CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */
    (*s1)[0] = '"';
    for (i = 0; i < n; ++i)
      (*s1)[i + 1] = s[i];
    (*s1)[n + 1] = '"';
    (*s1)[n + 2] = '\0';
  }
  return CDD_C_SUCCESS;
}

static enum cdd_c_error c_str_eq(const char *const s0, const char *const s1) {
  return ((s0 == NULL && s1 == NULL) ||
          /* LCOV_EXCL_START */
          (s0 != NULL && s1 != NULL && strcmp(s0, s1) == 0))
             /* LCOV_EXCL_STOP */
             ? 0
             : 1;
}

enum cdd_c_error Tank_default(enum Tank *out) {
  if (out)
    *out = Tank_BIG;
  return CDD_C_SUCCESS;
}

enum cdd_c_error Tank_to_str(const enum Tank tank, char **const str) {
  if (str == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  switch (tank) {
  case Tank_BIG:
    *str = strdup("BIG");
    if (!*str)
      return CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */
    break;
  case Tank_SMALL:
    *str = strdup("SMALL");
    if (!*str)
      return CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */
    break;
  case Tank_UNKNOWN:
  default:
    *str = strdup("UNKNOWN");
    if (!*str)
      return CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */
  }

  if (*str == NULL)
    return CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */

  return CDD_C_SUCCESS;
}

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

enum cdd_c_error HazE_cleanup(struct HazE *const haz_e) {
  if (haz_e == NULL)
    return CDD_C_SUCCESS;

  free((void *)haz_e->bzr);
  free(haz_e);
  return CDD_C_SUCCESS;
}

enum cdd_c_error HazE_default(struct HazE **haz_e) {
  if (haz_e == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *haz_e = malloc(sizeof(**haz_e));
  if (*haz_e == NULL)
    return CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */
  Tank_default(&(*haz_e)->tank);
  (*haz_e)->bzr = NULL;
  return CDD_C_SUCCESS;
}

enum cdd_c_error HazE_deepcopy(const struct HazE *const haz_e_original,
                               struct HazE **haz_e_dest) {
  if (haz_e_dest == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (haz_e_original == NULL) {
    *haz_e_dest = NULL;
    return CDD_C_SUCCESS;
  }
  *haz_e_dest = malloc(sizeof(**haz_e_dest));
  if (*haz_e_dest == NULL)
    return CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */

  if (haz_e_original->bzr == NULL) {
    (*haz_e_dest)->bzr = NULL;
  } else {
    (*haz_e_dest)->bzr = strdup(haz_e_original->bzr);
    if ((*haz_e_dest)->bzr == NULL) {
      free(*haz_e_dest);
      *haz_e_dest = NULL;
      return CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */
    }
  }
  (*haz_e_dest)->tank = haz_e_original->tank;
  return CDD_C_SUCCESS;
}

enum cdd_c_error HazE_display(const struct HazE *const haz_e, FILE *fh) {
  char *s = NULL;
  int rc = HazE_to_json(haz_e, &s);
  if (rc != 0) {
    free(s);
    return rc;
  }
  rc = fprintf(fh, "%s\n", s);
  if (rc >= 0) {
    if (fflush(fh) != 0)
      /* LCOV_EXCL_START */
      rc = -1;
    /* LCOV_EXCL_STOP */
    else
      rc = 0;
  }
  free(s);
  return rc;
}

enum cdd_c_error HazE_debug(const struct HazE *const haz_e, FILE *fh) {
  int rc;
  if (haz_e == NULL) {
    rc = fputs("<null HazE>\n", fh);
    return rc < 0 ? rc : 0;
  }
  rc = fputs("struct HazE dbg = {\n", fh);
  if (rc < 0)
    /* LCOV_EXCL_START */
    return rc;
  /* LCOV_EXCL_STOP */
  {
    char *quoted = NULL;
    rc = quote_or_null(haz_e->bzr, &quoted);
    if (rc != 0)
      return rc;
    rc = fprintf(fh, "  /* const char * */ bzr = %s,\n", quoted);
    free(quoted);
    if (rc < 0)
      /* LCOV_EXCL_START */
      return rc;
    /* LCOV_EXCL_STOP */
  }
  rc = fprintf(fh, "  /* enum Tank */ tank = %d\n", haz_e->tank);
  if (rc < 0)
    /* LCOV_EXCL_START */
    return rc;
  /* LCOV_EXCL_STOP */
  rc = fputs("};\n", fh);
  return rc < 0 ? rc : 0;
}

enum cdd_c_error HazE_eq(const struct HazE *const haz_e0,
                         const struct HazE *const haz_e1) {
  if (haz_e0 == NULL || haz_e1 == NULL)
    return haz_e0 == haz_e1 ? 0 : 1;

  if (haz_e0->tank != haz_e1->tank)
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_UNKNOWN;
  /* LCOV_EXCL_STOP */

  return c_str_eq(haz_e0->bzr, haz_e1->bzr);
}

enum cdd_c_error HazE_to_json(const struct HazE *const haz_e, char **json) {
  char *tank_str = NULL;
  int rc = 0;
  int need_comma = 0;

  if (json == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (haz_e == NULL) {
    c89stringutils_jasprintf(json, "null");
    return *json == NULL ? CDD_C_ERROR_MEMORY : 0;
  }

  c89stringutils_jasprintf(json, "{");
  if (*json == NULL)
    return CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */

  if (haz_e->bzr) {
    c89stringutils_jasprintf(json, "\"bzr\": \"%s\"", haz_e->bzr);
    need_comma = 1;
  } else {
    c89stringutils_jasprintf(json, "\"bzr\": null");
    need_comma = 1;
  }
  if (*json == NULL)
    return CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */

  if (need_comma) {
    c89stringutils_jasprintf(json, ",");
    if (*json == NULL)
      return CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */
  }
  if (Tank_to_str(haz_e->tank, &tank_str) != 0) {
    rc = CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */
    goto cleanup;
  }
  c89stringutils_jasprintf(json, "\"tank\": \"%s\"", tank_str);
  if (*json == NULL) {
    rc = CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */
                             /* LCOV_EXCL_START */
    goto cleanup;
    /* LCOV_EXCL_STOP */
  }

  c89stringutils_jasprintf(json, "}");
  if (*json == NULL) {
    rc = CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */
  }

cleanup:
  free(tank_str);
  return rc;
}

enum cdd_c_error HazE_from_jsonObject(const JSON_Object *const jsonObject,
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
    /* LCOV_EXCL_START */
    return rc;
  /* LCOV_EXCL_STOP */

  new_haz = malloc(sizeof(*new_haz));
  if (new_haz == NULL)
    return CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */

  bzr_str = json_object_get_string(jsonObject, "bzr");
  if (bzr_str) {
    new_haz->bzr = strdup(bzr_str);
    if (new_haz->bzr == NULL) {
      free(new_haz);
      return CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */
    }
  } else {
    new_haz->bzr = NULL;
  }

  new_haz->tank = tank_val;
  *haz_e = new_haz;
  return CDD_C_SUCCESS;
}

enum cdd_c_error HazE_from_json(const char *const json, struct HazE **haz_e) {
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

enum cdd_c_error FooE_cleanup(struct FooE *const foo_e) {
  if (foo_e == NULL)
    return CDD_C_SUCCESS;
  free((void *)foo_e->bar);
  HazE_cleanup(foo_e->haz);
  free(foo_e);
  return CDD_C_SUCCESS;
}

enum cdd_c_error FooE_default(struct FooE **foo_e) {
  int rc;
  if (foo_e == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *foo_e = malloc(sizeof(**foo_e));
  if (*foo_e == NULL)
    return CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */

  memset(*foo_e, 0, sizeof(**foo_e));

  rc = HazE_default(&(*foo_e)->haz);
  if (rc != 0) {
    free(*foo_e);
    *foo_e = NULL;
    return CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */
  }
  return CDD_C_SUCCESS;
}

enum cdd_c_error FooE_deepcopy(const struct FooE *const foo_e_original,
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
    return CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */
  memset(new_foo, 0, sizeof(*new_foo));

  if (foo_e_original->bar != NULL) {
    new_foo->bar = strdup(foo_e_original->bar);
    if (new_foo->bar == NULL) {
      free(new_foo);
      return CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */
    }
  }

  new_foo->can = foo_e_original->can;

  if (HazE_deepcopy(foo_e_original->haz, &new_foo->haz) != 0) {
    FooE_cleanup(new_foo);
    return CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */
  }

  *foo_e_dest = new_foo;
  return CDD_C_SUCCESS;
}

enum cdd_c_error FooE_display(const struct FooE *foo_e, FILE *fh) {
  char *s = NULL;
  int rc = FooE_to_json(foo_e, &s);
  if (rc != 0) {
    free(s);
    return rc;
  }
  rc = fprintf(fh, "%s\n", s);
  if (rc >= 0) {
    if (fflush(fh) != 0)
      /* LCOV_EXCL_START */
      rc = -1;
    /* LCOV_EXCL_STOP */
    else
      rc = 0;
  }
  free(s);
  return rc;
}

enum cdd_c_error FooE_debug(const struct FooE *const foo_e, FILE *fh) {
  int rc;
  if (foo_e == NULL) {
    rc = fputs("<null FooE>\n", fh);
    return rc < 0 ? rc : 0;
  }
  rc = fputs("struct FooE dbg = {\n", fh);
  if (rc < 0)
    /* LCOV_EXCL_START */
    return rc;
  /* LCOV_EXCL_STOP */

  {
    char *quoted = NULL;
    rc = quote_or_null(foo_e->bar, &quoted);
    if (rc != 0)
      return rc;
    rc = fprintf(fh, "  /* const char * */ bar = %s,\n", quoted);
    free(quoted);
    if (rc < 0)
      /* LCOV_EXCL_START */
      return rc;
    /* LCOV_EXCL_STOP */
  }
  rc = fprintf(fh, "  /* int can */ can = %d,\n", foo_e->can);
  if (rc < 0)
    /* LCOV_EXCL_START */
    return rc;
  /* LCOV_EXCL_STOP */

  rc = HazE_debug(foo_e->haz, fh);
  if (rc < 0)
    /* LCOV_EXCL_START */
    return rc;
  /* LCOV_EXCL_STOP */

  rc = fputs("};\n", fh);
  return rc < 0 ? rc : 0;
}

enum cdd_c_error FooE_eq(const struct FooE *const foo_e0,
                         const struct FooE *const foo_e1) {
  if (foo_e0 == NULL || foo_e1 == NULL)
    return foo_e0 == foo_e1 ? 0 : 1;

  return (foo_e0->can == foo_e1->can &&
          c_str_eq(foo_e0->bar, foo_e1->bar) == 0 &&
          HazE_eq(foo_e0->haz, foo_e1->haz) == 0)
             ? 0
             : 1;
}

enum cdd_c_error FooE_to_json(const struct FooE *const foo_e,
                              char **const json) {
  char *haz_e_json = NULL;
  int rc = 0;

  if (json == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (foo_e == NULL) {
    c89stringutils_jasprintf(json, "null");
    return *json == NULL ? CDD_C_ERROR_MEMORY : 0;
  }

  c89stringutils_jasprintf(json, "{");
  if (*json == NULL)
    return CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */

  if (foo_e->bar) {
    c89stringutils_jasprintf(json, "\"bar\": \"%s\",", foo_e->bar);
  } else {
    c89stringutils_jasprintf(json, "\"bar\": null,");
  }
  if (*json == NULL)
    return CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */

  c89stringutils_jasprintf(json, "\"can\": %d,", foo_e->can);
  if (*json == NULL)
    return CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */

  if (HazE_to_json(foo_e->haz, &haz_e_json) != 0) {
    rc = CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */
    goto cleanup;
  }

  c89stringutils_jasprintf(json, "\"haz\":%s", haz_e_json);
  if (*json == NULL) {
    rc = CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */
                             /* LCOV_EXCL_START */
    goto cleanup;
    /* LCOV_EXCL_STOP */
  }

  c89stringutils_jasprintf(json, "}");
  if (*json == NULL) {
    rc = CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */
  }

cleanup:
  free(haz_e_json);
  return rc;
}

enum cdd_c_error FooE_from_jsonObject(const JSON_Object *const jsonObject,
                                      struct FooE **const foo_e) {
  int rc = 0;
  const char *bar_str = NULL;
  const JSON_Object *haz_obj = NULL;
  struct FooE *new_foo;

  if (jsonObject == NULL || foo_e == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  new_foo = (struct FooE *)calloc(1, sizeof(*new_foo));
  if (new_foo == NULL)
    return CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */

  bar_str = json_object_get_string(jsonObject, "bar");
  if (bar_str) {
    new_foo->bar = strdup(bar_str);
    if (new_foo->bar == NULL) {
      free(new_foo);
      return CDD_C_ERROR_MEMORY; /* LCOV_EXCL_LINE */
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

enum cdd_c_error FooE_from_json(const char *const json,
                                struct FooE **const foo_e) {
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
