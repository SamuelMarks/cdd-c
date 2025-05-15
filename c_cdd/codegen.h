#ifndef CODEGEN_H
#define CODEGEN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>

#include <c_cdd_export.h>

/*
 * Parse enum declaration lines:
 *
 * enum EnumName {
 *   MEMBER1,
 *   MEMBER2,
 *   MEMBER3 = -1
 * };
 *
 * fills array of members.
 */
struct EnumMembers {
  size_t capacity;
  size_t size;
  char **members;
};

/*
 * Parse a struct member line and extract its name and type.
 * Supports patterns like:
 *   const char *field;
 *   int field;
 *   double field;
 *   int field;  // for bool treated as integer
 *   struct OtherType *field;
 */
struct StructField {
  char name[64];
  char type[64]; /* e.g. string, integer, number, boolean, object(ref) */
  char ref[64];  /* for struct * */
};
struct StructFields {
  size_t capacity;
  size_t size;
  struct StructField *fields;
};

extern C_CDD_EXPORT void write_enum_from_str_func(FILE *, const char *,
                                                  const struct EnumMembers *);

extern C_CDD_EXPORT void write_enum_to_str_func(FILE *, const char *,
                                                const struct EnumMembers *);

extern C_CDD_EXPORT void write_struct_cleanup_func(FILE *, const char *,
                                                   const struct StructFields *);

extern C_CDD_EXPORT void write_struct_debug_func(FILE *, const char *,
                                                 const struct StructFields *);

extern C_CDD_EXPORT void
write_struct_deepcopy_func(FILE *, const char *, const struct StructFields *);

extern C_CDD_EXPORT void write_struct_default_func(FILE *, const char *,
                                                   const struct StructFields *);

extern C_CDD_EXPORT void write_struct_display_func(FILE *, const char *,
                                                   const struct StructFields *);

extern C_CDD_EXPORT void write_struct_eq_func(FILE *, const char *,
                                              const struct StructFields *);

extern C_CDD_EXPORT void
write_struct_from_jsonObject_func(FILE *, const char *,
                                  const struct StructFields *);

extern C_CDD_EXPORT void write_struct_from_json_func(FILE *, const char *);

extern C_CDD_EXPORT void write_struct_to_json_func(FILE *, const char *,
                                                   const struct StructFields *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CODEGEN_H */
