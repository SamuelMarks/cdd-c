/* To be appended to abstract_struct.c */

/* clang-format off */
#include "classes/emit/c_orm_meta.h"
#include <string.h>
/* clang-format on */

int cdd_c_specific_to_abstract(cdd_c_abstract_struct_t *out_astruct,
                               const void *in_struct,
                               const struct c_orm_meta *struct_meta) {
  size_t i;
  if (!out_astruct || !in_struct || !struct_meta)
    return -1;

  if (cdd_c_abstract_struct_init(out_astruct) != 0)
    return -1;

  for (i = 0; i < struct_meta->num_props; ++i) {
    const c_orm_prop_meta_t *prop = &struct_meta->props[i];
    cdd_c_variant_t val;
    val.type = CDD_C_VARIANT_TYPE_NULL;

    /* Direct reflection bypass mapping since generic representation doesn't
       know struct sizes safely yet, requires strict C_ORM type checking */
    if (strcmp(prop->type, "C_ORM_TYPE_INT32") == 0) {
      val.type = CDD_C_VARIANT_TYPE_INT;
      val.value.i_val = *(int *)((char *)in_struct + prop->offset);
    } else if (strcmp(prop->type, "C_ORM_TYPE_INT64") == 0) {
      val.type = CDD_C_VARIANT_TYPE_INT;
      val.value.i_val = *(long long *)((char *)in_struct + prop->offset);
    } else if (strcmp(prop->type, "C_ORM_TYPE_FLOAT") == 0) {
      val.type = CDD_C_VARIANT_TYPE_FLOAT;
      val.value.f_val = *(float *)((char *)in_struct + prop->offset);
    } else if (strcmp(prop->type, "C_ORM_TYPE_DOUBLE") == 0) {
      val.type = CDD_C_VARIANT_TYPE_FLOAT;
      val.value.f_val = *(double *)((char *)in_struct + prop->offset);
    } else if (strcmp(prop->type, "C_ORM_TYPE_STRING") == 0) {
      val.type = CDD_C_VARIANT_TYPE_STRING;
      if (prop->length > 0) {
        val.value.s_val = (char *)in_struct + prop->offset;
      } else {
        val.value.s_val = *(char **)((char *)in_struct + prop->offset);
      }
    }

    if (cdd_c_abstract_set(out_astruct, prop->name, &val) != 0) {
      cdd_c_abstract_struct_free(out_astruct);
      return -1;
    }
  }

  return 0;
}

int cdd_c_abstract_to_specific(void *out_struct,
                               const cdd_c_abstract_struct_t *in_astruct,
                               const struct c_orm_meta *struct_meta,
                               int strict_mapping) {
  size_t i;
  if (!out_struct || !in_astruct || !struct_meta)
    return -1;

  if (strict_mapping && in_astruct->count != struct_meta->num_props) {
    return -1;
  }

  for (i = 0; i < struct_meta->num_props; ++i) {
    const c_orm_prop_meta_t *prop = &struct_meta->props[i];
    cdd_c_variant_t *val = NULL;

    if (cdd_c_abstract_get(in_astruct, prop->name, &val) != 0) {
      if (strict_mapping)
        return -1;
      continue; /* Partial mapping allowed */
    }

    if (strcmp(prop->type, "C_ORM_TYPE_INT32") == 0) {
      if (val->type == CDD_C_VARIANT_TYPE_INT) {
        *(int *)((char *)out_struct + prop->offset) = (int)val->value.i_val;
      } else if (strict_mapping)
        return -1;
    } else if (strcmp(prop->type, "C_ORM_TYPE_INT64") == 0) {
      if (val->type == CDD_C_VARIANT_TYPE_INT) {
        *(long long *)((char *)out_struct + prop->offset) = val->value.i_val;
      } else if (strict_mapping)
        return -1;
    } else if (strcmp(prop->type, "C_ORM_TYPE_FLOAT") == 0) {
      if (val->type == CDD_C_VARIANT_TYPE_FLOAT) {
        *(float *)((char *)out_struct + prop->offset) = (float)val->value.f_val;
      } else if (strict_mapping)
        return -1;
    } else if (strcmp(prop->type, "C_ORM_TYPE_DOUBLE") == 0) {
      if (val->type == CDD_C_VARIANT_TYPE_FLOAT) {
        *(double *)((char *)out_struct + prop->offset) = val->value.f_val;
      } else if (strict_mapping)
        return -1;
    } else if (strcmp(prop->type, "C_ORM_TYPE_STRING") == 0) {
      if (val->type == CDD_C_VARIANT_TYPE_STRING) {
        if (prop->length > 0) {
          strncpy((char *)out_struct + prop->offset, val->value.s_val,
                  prop->length);
          ((char *)out_struct + prop->offset)[prop->length - 1] = '\0';
        } else {
          *(char **)((char *)out_struct + prop->offset) = strdup(
              val->value.s_val); /* Assumes user frees out_struct dynamically */
        }
      } else if (strict_mapping)
        return -1;
    }
  }

  return 0;
}
