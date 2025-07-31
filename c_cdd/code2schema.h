#ifndef CODE2SCHEMA_H
#define CODE2SCHEMA_H

#ifdef __cplusplus
#include <cstdlib>
#include <cstring>
extern "C" {
#else
#include <stdlib.h>
#include <string.h>

#if __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#else
#include <c_cdd_stdbool.h>
#endif /* __STDC_VERSION__ >= 199901L */

#endif /* __cplusplus */

#include <parson.h>

#include <c_cdd_export.h>

#include "codegen.h"

extern C_CDD_EXPORT int code2schema_main(int, char **argv);

extern C_CDD_EXPORT void struct_fields_free(struct StructFields *);

extern C_CDD_EXPORT void enum_members_init(struct EnumMembers *);

extern C_CDD_EXPORT void enum_members_free(struct EnumMembers *);

extern C_CDD_EXPORT void enum_members_add(struct EnumMembers *, const char *);

extern C_CDD_EXPORT void struct_fields_init(struct StructFields *);

extern C_CDD_EXPORT void struct_fields_add(struct StructFields *, const char *,
                                           const char *, const char *);

extern C_CDD_EXPORT int parse_struct_member_line(const char *,
                                                 struct StructFields *);

extern C_CDD_EXPORT void
write_struct_to_json_schema(JSON_Object *, const char *, struct StructFields *);

extern C_CDD_EXPORT bool str_starts_with(const char *, const char *);

extern C_CDD_EXPORT void trim_trailing(char *);

extern C_CDD_EXPORT int json_array_to_enum_members(const JSON_Array *,
                                                   struct EnumMembers *);

extern C_CDD_EXPORT int json_object_to_struct_fields(const JSON_Object *,
                                                     struct StructFields *,
                                                     const JSON_Object *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CODE2SCHEMA_H */
