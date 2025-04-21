#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#else
#include <sys/errno.h>
#endif

#include <parson.h>

#include <c89stringutils_string_extras.h>

#include "simple_json.h"

int Tank_to_str(const enum Tank tank, char **const str) {
  int rc = 0;
  switch (tank) {
  case BIG:
    *str = strdup("BIG");
    break;
  case SMALL:
    *str = strdup("SMALL");
    break;
  case UNKNOWN:
  default:
    *str = strdup("UNKNOWN");
  }

  if (*str == NULL)
    rc = ENOMEM;

  return rc;
}

int Tank_from_str(const char *str, enum Tank *const tank) {
  int rc = 0;
  if (str == NULL || tank == NULL)
    rc = EINVAL;
  else if (strcmp(str, "BIG") == 0)
    *tank = BIG;
  else if (strcmp(str, "SMALL") == 0)
    *tank = SMALL;
  else
    *tank = UNKNOWN;
  return rc;
}

void HazE_cleanup(struct HazE *const haz_e) {
  if (haz_e == NULL)
    return;
  else if (haz_e->bzr != NULL)
    free((void *)haz_e->bzr);
  free(haz_e);
}

bool HazE_eq(const struct HazE *const haz_e0, const struct HazE *const haz_e1) {
  if (haz_e0 == NULL || haz_e1 == NULL)
    return haz_e0 == NULL && haz_e1 == NULL;
  return haz_e0->tank == haz_e1->tank && strcmp(haz_e0->bzr, haz_e1->bzr) == 0;
}

int HazE_to_json(const struct HazE *const haz_e, char **json) {
  char *tank_str = NULL;
  int rc = 0;

  jasprintf(json, "{");
  if (*json == NULL)
    return ENOMEM;
  jasprintf(json, "\"bzr\": \"%s\",", haz_e->bzr);
  if (Tank_to_str(haz_e->tank, &tank_str) != 0)
    return ENOMEM;
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
  const char *tank_str = NULL;
  int rc;
  enum Tank tank_val;

  if (jsonObject == NULL || haz_e == NULL)
    return EINVAL;

  bzr_str = json_object_get_string(jsonObject, "bzr");
  if (bzr_str == NULL)
    return EINVAL;

  tank_str = json_object_get_string(jsonObject, "tank");
  if (tank_str == NULL)
    return EINVAL;

  rc = Tank_from_str(tank_str, &tank_val);
  if (rc != 0)
    return rc;

  *haz_e = malloc(sizeof(**haz_e));
  if (*haz_e == NULL)
    return ENOMEM;

  (*haz_e)->bzr = strdup(bzr_str);
  if ((*haz_e)->bzr == NULL) {
    free(*haz_e);
    *haz_e = NULL;
    return ENOMEM;
  }

  (*haz_e)->tank = tank_val;
  return rc;
}

int HazE_from_json(const char *const json, struct HazE const **haz_e) {
  JSON_Value *root = NULL;
  const JSON_Object *jsonObject = NULL;
  int rc = 0;
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
  if (foo_e->bar != NULL)
    free((void *)foo_e->bar);
  HazE_cleanup(foo_e->haz);
  free(foo_e);
}

bool FooE_eq(const struct FooE *const foo_e0, const struct FooE *const foo_e1) {
  if (foo_e0 == NULL || foo_e1 == NULL)
    return foo_e0 == NULL && foo_e1 == NULL;
  return strcmp(foo_e0->bar, foo_e1->bar) == 0 && foo_e0->can == foo_e1->can &&
         HazE_eq(foo_e0->haz, foo_e1->haz);
}

int FooE_to_json(const struct FooE *const foo_e, char **const json) {
  char *haz_e_json = NULL;
  int rc = 0;

  jasprintf(json, "{");
  if (*json == NULL)
    return ENOMEM;
  jasprintf(json, "\"bar\": \"%s\",", foo_e->bar);
  if (*json == NULL)
    return ENOMEM;
  jasprintf(json, "\"can\": %d,", foo_e->can);
  if (*json == NULL)
    return ENOMEM;
  if (HazE_to_json(foo_e->haz, &haz_e_json) != 0)
    return ENOMEM;
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
  int rc;
  const char *bar_str = NULL;
  const JSON_Object *haz_obj = NULL;

  if (jsonObject == NULL || foo_e == NULL)
    return EINVAL;

  bar_str = json_object_get_string(jsonObject, "bar");
  if (bar_str == NULL)
    return EINVAL;

  *foo_e = malloc(sizeof(**foo_e));
  if (*foo_e == NULL)
    return ENOMEM;

  (*foo_e)->bar = strdup(bar_str);
  if ((*foo_e)->bar == NULL) {
    free(*foo_e);
    *foo_e = NULL;
    return ENOMEM;
  }

  (*foo_e)->can = (int)json_object_get_number(jsonObject, "can");

  haz_obj = json_object_get_object(jsonObject, "haz");
  if (haz_obj == NULL) {
    free((void *)(*foo_e)->bar);
    free(*foo_e);
    *foo_e = NULL;
    return EINVAL;
  }

  rc = HazE_from_jsonObject(haz_obj, &(*foo_e)->haz);
  if (rc != 0) {
    free((void *)(*foo_e)->bar);
    free(*foo_e);
    *foo_e = NULL;
  }

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
