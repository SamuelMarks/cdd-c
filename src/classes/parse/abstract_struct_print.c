/* To be appended to abstract_struct.c */

/* clang-format off */
#include <stdio.h>
/* clang-format on */

void cdd_c_abstract_print(const cdd_c_abstract_struct_t *astruct) {
  size_t i;
  if (!astruct) {
    printf("NULL astruct\n");
    return;
  }
  printf("{\n");
  for (i = 0; i < astruct->count; ++i) {
    printf("  \"%s\": ", astruct->kvs[i].key);
    switch (astruct->kvs[i].value.type) {
    case CDD_C_VARIANT_TYPE_NULL:
      printf("null");
      break;
    case CDD_C_VARIANT_TYPE_INT:
#if defined(_MSC_VER)
      printf("%I64d", astruct->kvs[i].value.value.i_val);
#else
      printf("%lld", astruct->kvs[i].value.value.i_val);
#endif
      break;
    case CDD_C_VARIANT_TYPE_FLOAT:
      printf("%f", astruct->kvs[i].value.value.f_val);
      break;
    case CDD_C_VARIANT_TYPE_STRING:
      printf("\"%s\"", astruct->kvs[i].value.value.s_val);
      break;
    case CDD_C_VARIANT_TYPE_BLOB:
      printf("<blob:%lu bytes>",
             (unsigned long)astruct->kvs[i].value.value.b_val.size);
      break;
    default:
      printf("<unknown>");
      break;
    }
    if (i < astruct->count - 1) {
      printf(",");
    }
    printf("\n");
  }
  printf("}\n");
}
