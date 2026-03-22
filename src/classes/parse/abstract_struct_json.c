/* To be appended to abstract_struct.c */

/* clang-format off */
#include "parson.h"
/* clang-format on */

int cdd_c_abstract_struct_to_json(const cdd_c_abstract_struct_t *astruct,
                                  char **out_json) {
  JSON_Value *root_val;
  JSON_Object *root_obj;
  size_t i;

  if (!astruct || !out_json)
    return -1;

  root_val = json_value_init_object();
  root_obj = json_value_get_object(root_val);
  if (!root_val || !root_obj)
    return -1;

  for (i = 0; i < astruct->count; ++i) {
    const cdd_c_abstract_struct_kv_t *kv = &astruct->kvs[i];
    switch (kv->value.type) {
    case CDD_C_VARIANT_TYPE_NULL:
      json_object_set_null(root_obj, kv->key);
      break;
    case CDD_C_VARIANT_TYPE_INT:
      json_object_set_number(root_obj, kv->key, (double)kv->value.value.i_val);
      break;
    case CDD_C_VARIANT_TYPE_FLOAT:
      json_object_set_number(root_obj, kv->key, kv->value.value.f_val);
      break;
    case CDD_C_VARIANT_TYPE_STRING:
      json_object_set_string(root_obj, kv->key, kv->value.value.s_val);
      break;
    case CDD_C_VARIANT_TYPE_BLOB:
      /* Blobs don't cleanly serialize to JSON. Serialize as base64 or drop.
       * Dropping for now to avoid large string allocs in parson */
      json_object_set_string(root_obj, kv->key, "<blob>");
      break;
    default:
      break;
    }
  }

  *out_json = json_serialize_to_string(root_val);
  json_value_free(root_val);
  if (!*out_json)
    return -1;

  return 0;
}

int cdd_c_abstract_struct_from_json(const char *json_str,
                                    cdd_c_abstract_struct_t *out_astruct) {
  JSON_Value *root_val;
  JSON_Object *root_obj;
  size_t count, i;

  if (!json_str || !out_astruct)
    return -1;

  if (cdd_c_abstract_struct_init(out_astruct) != 0)
    return -1;

  root_val = json_parse_string(json_str);
  if (!root_val)
    return -1;

  if (json_value_get_type(root_val) != JSONObject) {
    json_value_free(root_val);
    cdd_c_abstract_struct_free(out_astruct);
    return -1;
  }

  root_obj = json_value_get_object(root_val);
  count = json_object_get_count(root_obj);

  for (i = 0; i < count; ++i) {
    const char *key = json_object_get_name(root_obj, i);
    JSON_Value *val = json_object_get_value_at(root_obj, i);
    cdd_c_variant_t variant;

    variant.type = CDD_C_VARIANT_TYPE_NULL;

    switch (json_value_get_type(val)) {
    case JSONString:
      variant.type = CDD_C_VARIANT_TYPE_STRING;
      variant.value.s_val = (char *)json_value_get_string(
          val); /* Needs duplicate in abstract_set */
      break;
    case JSONNumber: {
      double num = json_value_get_number(val);
      if (num == (double)((long long)num)) {
        variant.type = CDD_C_VARIANT_TYPE_INT;
        variant.value.i_val = (long long)num;
      } else {
        variant.type = CDD_C_VARIANT_TYPE_FLOAT;
        variant.value.f_val = num;
      }
    } break;
    case JSONBoolean:
      variant.type = CDD_C_VARIANT_TYPE_INT;
      variant.value.i_val = json_value_get_boolean(val) ? 1 : 0;
      break;
    case JSONNull:
      variant.type = CDD_C_VARIANT_TYPE_NULL;
      break;
    default:
      break; /* Array/Object not currently handled generically via variant */
    }

    if (cdd_c_abstract_set(out_astruct, key, &variant) != 0) {
      json_value_free(root_val);
      cdd_c_abstract_struct_free(out_astruct);
      return -1;
    }
  }

  json_value_free(root_val);
  return 0;
}
