#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#else
#include <sys/errno.h>
#endif

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) && !defined(strdup)
#define strdup _strdup
#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) */

#include <parson.h>

#include <c89stringutils_string_extras.h>

#include "simple_json.h"

int quote_or_null(const char *const s, char **s1) {
  if (s == NULL) {
    *s1 = strdup("(null)");
    if (*s1 == NULL)
      return ENOMEM;
    return 0;
  }
  {
    const size_t n = strlen(s);
    size_t i;
    *s1 = malloc((n + 3) * sizeof(char));
    if (*s1 == NULL)
      return ENOMEM;
    (*s1)[0] = '"';
    for (i = 0; i < n; ++i)
      (*s1)[i + 1] = s[i];
    (*s1)[n + 1] = '"';
    (*s1)[n + 2] = '\0';
  }
  return 0;
}

bool c_str_eq(const char *const s0, const char *const s1) {
  return (s0 == NULL && s1 == NULL) ||
         (s0 != NULL && s1 != NULL && strcmp(s0, s1) == 0);
}

enum Tank Tank_default(void) { return Tank_BIG; }

int Tank_to_str(const enum Tank tank, char **const str) {
  if (str == NULL)
    return EINVAL;
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
  }

  if (*str == NULL)
    return ENOMEM;

  return 0;
}

int Tank_from_str(const char *str, enum Tank *val) {
  if (val == NULL)
    return EINVAL;
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
  return 0;
}

void HazE_cleanup(struct HazE *const haz_e) {
  if (haz_e == NULL)
    return;

  free((void *)haz_e->bzr);
  free(haz_e);
}

int HazE_default(struct HazE **haz_e) {
  if (haz_e == NULL)
    return EINVAL;
  *haz_e = malloc(sizeof(**haz_e));
  if (*haz_e == NULL)
    return ENOMEM;
  (*haz_e)->tank = Tank_default();
  (*haz_e)->bzr = NULL;
  return 0;
}

int HazE_deepcopy(const struct HazE *const haz_e_original,
                  struct HazE **haz_e_dest) {
  if (haz_e_dest == NULL)
    return EINVAL;
  if (haz_e_original == NULL) {
    *haz_e_dest = NULL;
    return 0;
  }
  *haz_e_dest = malloc(sizeof(**haz_e_dest));
  if (*haz_e_dest == NULL)
    return ENOMEM;

  if (haz_e_original->bzr == NULL) {
    (*haz_e_dest)->bzr = NULL;
  } else {
    (*haz_e_dest)->bzr = strdup(haz_e_original->bzr);
    if ((*haz_e_dest)->bzr == NULL) {
      free(*haz_e_dest);
      *haz_e_dest = NULL;
      return ENOMEM;
    }
  }
  (*haz_e_dest)->tank = haz_e_original->tank;
  return 0;
}

int HazE_display(const struct HazE *const haz_e, FILE *fh) {
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

int HazE_debug(const struct HazE *const haz_e, FILE *fh) {
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

bool HazE_eq(const struct HazE *const haz_e0, const struct HazE *const haz_e1) {
  if (haz_e0 == NULL || haz_e1 == NULL)
    return haz_e0 == haz_e1;

  if (haz_e0->tank != haz_e1->tank)
    return false;

  return c_str_eq(haz_e0->bzr, haz_e1->bzr);
}

int HazE_to_json(const struct HazE *const haz_e, char **json) {
  char *tank_str = NULL;
  int rc = 0;
  int need_comma = 0;

  if (json == NULL)
    return EINVAL;
  if (haz_e == NULL) {
    jasprintf(json, "null");
    return *json == NULL ? ENOMEM : 0;
  }

  jasprintf(json, "{");
  if (*json == NULL)
    return ENOMEM;

  if (haz_e->bzr) {
    jasprintf(json, "\"bzr\": \"%s\"", haz_e->bzr);
    need_comma = 1;
  } else {
    jasprintf(json, "\"bzr\": null");
    need_comma = 1;
  }
  if (*json == NULL)
    return ENOMEM;

  if (need_comma) {
    jasprintf(json, ",");
    if (*json == NULL)
      return ENOMEM;
  }
  if (Tank_to_str(haz_e->tank, &tank_str) != 0) {
    rc = ENOMEM;
    goto cleanup;
  }
  jasprintf(json, "\"tank\": \"%s\"", tank_str);
  if (*json == NULL) {
    rc = ENOMEM;
    goto cleanup;
  }

  jasprintf(json, "}");
  if (*json == NULL) {
    rc = ENOMEM;
  }

cleanup:
  free(tank_str);
  return rc;
}

int HazE_from_jsonObject(const JSON_Object *const jsonObject,
                         struct HazE **haz_e) {
  const char *bzr_str = NULL;
  const char *tank_str;
  int rc = 0;
  enum Tank tank_val;
  struct HazE *new_haz;

  if (jsonObject == NULL || haz_e == NULL)
    return EINVAL;

  tank_str = json_object_get_string(jsonObject, "tank");
  if (tank_str == NULL)
    return EINVAL;
  rc = Tank_from_str(tank_str, &tank_val);
  if (rc != 0)
    return rc;

  new_haz = malloc(sizeof(*new_haz));
  if (new_haz == NULL)
    return ENOMEM;

  bzr_str = json_object_get_string(jsonObject, "bzr");
  if (bzr_str) {
    new_haz->bzr = strdup(bzr_str);
    if (new_haz->bzr == NULL) {
      free(new_haz);
      return ENOMEM;
    }
  } else {
    new_haz->bzr = NULL;
  }

  new_haz->tank = tank_val;
  *haz_e = new_haz;
  return 0;
}

int HazE_from_json(const char *const json, struct HazE **haz_e) {
  JSON_Value *root = NULL;
  const JSON_Object *jsonObject = NULL;
  int rc;
  if (json == NULL || haz_e == NULL)
    return EINVAL;

  root = json_parse_string(json);
  if (root == NULL)
    return EINVAL;

  jsonObject = json_value_get_object(root);
  if (jsonObject == NULL) {
    json_value_free(root);
    return EINVAL;
  }

  rc = HazE_from_jsonObject(jsonObject, haz_e);
  json_value_free(root);
  return rc;
}

void FooE_cleanup(struct FooE *const foo_e) {
  if (foo_e == NULL)
    return;
  free((void *)foo_e->bar);
  HazE_cleanup(foo_e->haz);
  free(foo_e);
}

int FooE_default(struct FooE **foo_e) {
  int rc;
  if (foo_e == NULL)
    return EINVAL;

  *foo_e = malloc(sizeof(**foo_e));
  if (*foo_e == NULL)
    return ENOMEM;

  memset(*foo_e, 0, sizeof(**foo_e));

  rc = HazE_default(&(*foo_e)->haz);
  if (rc != 0) {
    free(*foo_e);
    *foo_e = NULL;
    return ENOMEM;
  }
  return 0;
}

int FooE_deepcopy(const struct FooE *const foo_e_original,
                  struct FooE **foo_e_dest) {
  struct FooE *new_foo;
  if (foo_e_dest == NULL)
    return EINVAL;

  if (foo_e_original == NULL) {
    *foo_e_dest = NULL;
    return 0;
  }

  new_foo = malloc(sizeof(*new_foo));
  if (new_foo == NULL)
    return ENOMEM;
  memset(new_foo, 0, sizeof(*new_foo));

  if (foo_e_original->bar != NULL) {
    new_foo->bar = strdup(foo_e_original->bar);
    if (new_foo->bar == NULL) {
      free(new_foo);
      return ENOMEM;
    }
  }

  new_foo->can = foo_e_original->can;

  if (HazE_deepcopy(foo_e_original->haz, &new_foo->haz) != 0) {
    FooE_cleanup(new_foo);
    return ENOMEM;
  }

  *foo_e_dest = new_foo;
  return 0;
}

int FooE_display(const struct FooE *foo_e, FILE *fh) {
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

int FooE_debug(const struct FooE *const foo_e, FILE *fh) {
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

bool FooE_eq(const struct FooE *const foo_e0, const struct FooE *const foo_e1) {
  if (foo_e0 == NULL || foo_e1 == NULL)
    return foo_e0 == foo_e1;

  return (foo_e0->can == foo_e1->can && c_str_eq(foo_e0->bar, foo_e1->bar) &&
          HazE_eq(foo_e0->haz, foo_e1->haz));
}

int FooE_to_json(const struct FooE *const foo_e, char **const json) {
  char *haz_e_json = NULL;
  int rc = 0;

  if (json == NULL)
    return EINVAL;
  if (foo_e == NULL) {
    jasprintf(json, "null");
    return *json == NULL ? ENOMEM : 0;
  }

  jasprintf(json, "{");
  if (*json == NULL)
    return ENOMEM;

  if (foo_e->bar) {
    jasprintf(json, "\"bar\": \"%s\",", foo_e->bar);
  } else {
    jasprintf(json, "\"bar\": null,");
  }
  if (*json == NULL)
    return ENOMEM;

  jasprintf(json, "\"can\": %d,", foo_e->can);
  if (*json == NULL)
    return ENOMEM;

  if (HazE_to_json(foo_e->haz, &haz_e_json) != 0) {
    rc = ENOMEM;
    goto cleanup;
  }

  jasprintf(json, "\"haz\":%s", haz_e_json);
  if (*json == NULL) {
    rc = ENOMEM;
    goto cleanup;
  }

  jasprintf(json, "}");
  if (*json == NULL) {
    rc = ENOMEM;
  }

cleanup:
  free(haz_e_json);
  return rc;
}

int FooE_from_jsonObject(const JSON_Object *const jsonObject,
                         struct FooE **const foo_e) {
  int rc = 0;
  const char *bar_str = NULL;
  const JSON_Object *haz_obj = NULL;
  struct FooE *new_foo;

  if (jsonObject == NULL || foo_e == NULL)
    return EINVAL;

  new_foo = calloc(sizeof(*new_foo), 1);
  if (new_foo == NULL)
    return ENOMEM;

  bar_str = json_object_get_string(jsonObject, "bar");
  if (bar_str) {
    new_foo->bar = strdup(bar_str);
    if (new_foo->bar == NULL) {
      free(new_foo);
      return ENOMEM;
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

int FooE_from_json(const char *const json, struct FooE **const foo_e) {
  JSON_Value *root = NULL;
  const JSON_Object *jsonObject = NULL;
  int rc;

  if (json == NULL || foo_e == NULL)
    return EINVAL;

  root = json_parse_string(json);
  if (root == NULL)
    return EINVAL;

  jsonObject = json_value_get_object(root);
  if (jsonObject == NULL) {
    json_value_free(root);
    return EINVAL;
  }

  rc = FooE_from_jsonObject(jsonObject, foo_e);
  json_value_free(root);
  return rc;
}
