/* To be appended to abstract_struct.c */

/* clang-format off */
#include "classes/parse/sql.h" /* For SqlDataType enum values */
/* clang-format on */

int cdd_c_abstract_hydrate(cdd_c_abstract_struct_t *out_astruct,
                           void **row_data, const cdd_c_column_meta_t *cols,
                           size_t n_cols) {
  size_t i;
  if (!out_astruct || !row_data || !cols)
    return -1;

  if (cdd_c_abstract_struct_init(out_astruct) != 0)
    return -1;

  for (i = 0; i < n_cols; ++i) {
    cdd_c_variant_t variant;
    const cdd_c_column_meta_t *col = &cols[i];
    void *raw_val = row_data[i];

    variant.type = CDD_C_VARIANT_TYPE_NULL;

    if (!raw_val) {
      /* Keep it null */
    } else {
      switch (col->inferred_type) {
      case SQL_TYPE_INT:
      case SQL_TYPE_BIGINT:
      case SQL_TYPE_BOOLEAN:
        variant.type = CDD_C_VARIANT_TYPE_INT;
        /* Assume drivers pass long long pointer boundaries for generic 64 bit
         * mappings */
        variant.value.i_val = *(long long *)raw_val;
        break;
      case SQL_TYPE_FLOAT:
      case SQL_TYPE_DOUBLE:
      case SQL_TYPE_DECIMAL:
        variant.type = CDD_C_VARIANT_TYPE_FLOAT;
        variant.value.f_val = *(double *)raw_val;
        break;
      case SQL_TYPE_VARCHAR:
      case SQL_TYPE_TEXT:
      case SQL_TYPE_CHAR:
      case SQL_TYPE_DATE:
      case SQL_TYPE_TIMESTAMP:
        variant.type = CDD_C_VARIANT_TYPE_STRING;
        variant.value.s_val = (char *)
            raw_val; /* String deep copy handled by cdd_c_abstract_set */
        break;
      case SQL_TYPE_BLOB:
        variant.type = CDD_C_VARIANT_TYPE_BLOB;
        /* Dynamic drivers need to provide size explicitly, we assume a size_t*
           precedes the unsigned char* or similar. Since we don't have lengths
           in void**, blob mapping generally fails cleanly here unless
           augmented. */
        variant.value.b_val.data = NULL;
        variant.value.b_val.size = 0;
        break;
      default:
        break;
      }
    }

    if (cdd_c_abstract_set(out_astruct, col->name, &variant) != 0) {
      cdd_c_abstract_struct_free(out_astruct);
      return -1;
    }
  }

  return 0;
}
