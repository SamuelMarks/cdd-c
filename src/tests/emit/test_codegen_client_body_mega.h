/**
 * @file test_codegen_client_body_mega.h
 * @brief Unit tests for client body exhaustive mega form fuzzing.
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_CLIENT_BODY_MEGA_H
#define TEST_CODEGEN_CLIENT_BODY_MEGA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_codegen_client_body_common.h"
/* clang-format on */

TEST test_client_body_form_mega(void) {
  int io_fail;
  for (io_fail = 0; io_fail < 5000; ++io_fail) {
    /* extern C_CDD_EXPORT int g_fail_io_after; (moved to global) */
    /* extern C_CDD_EXPORT int g_io_calls; (moved to global) */
    struct OpenAPI_Spec spec = {0};
    struct OpenAPI_Operation op = {0};
    FILE *fp;
    int rc;
    int all_success = 1;

    memset(&op, 0, sizeof(op));
    memset(&spec, 0, sizeof(spec));

    op.req_body.content_type =
        (char *)(size_t)(size_t) "application/x-www-form-urlencoded";
    op.req_body.ref_name = (char *)(size_t)(size_t) "MockFormSchema";

    spec.n_defined_schemas = 5;
    spec.defined_schema_names = calloc(5, sizeof(char *));
    c_cdd_strdup("MockFormSchema", &spec.defined_schema_names[0]);
    c_cdd_strdup("ObjType1", &spec.defined_schema_names[1]);
    c_cdd_strdup("ObjType2", &spec.defined_schema_names[2]);
    c_cdd_strdup("ObjType3", &spec.defined_schema_names[3]);
    c_cdd_strdup("ObjType4", &spec.defined_schema_names[4]);

    spec.defined_schemas = calloc(5, sizeof(struct StructFields));

    /* Main schema */
    spec.defined_schemas[0].size = 11;
    spec.defined_schemas[0].fields = calloc(11, sizeof(struct StructField));
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[0].name,
             sizeof(spec.defined_schemas[0].fields[0].name), "fArrStrRsv");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[0].name,
             sizeof(spec.defined_schemas[0].fields[0].name), "fArrStrRsv");
#else
    strcpy(spec.defined_schemas[0].fields[0].name, "fArrStrRsv");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[0].type,
             sizeof(spec.defined_schemas[0].fields[0].type), "array");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[0].type,
             sizeof(spec.defined_schemas[0].fields[0].type), "array");
#else
    strcpy(spec.defined_schemas[0].fields[0].type, "array");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[0].ref,
             sizeof(spec.defined_schemas[0].fields[0].ref), "string");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[0].ref,
             sizeof(spec.defined_schemas[0].fields[0].ref), "string");
#else
    strcpy(spec.defined_schemas[0].fields[0].ref, "string");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[1].name,
             sizeof(spec.defined_schemas[0].fields[1].name), "fObjExp");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[1].name,
             sizeof(spec.defined_schemas[0].fields[1].name), "fObjExp");
#else
    strcpy(spec.defined_schemas[0].fields[1].name, "fObjExp");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[1].type,
             sizeof(spec.defined_schemas[0].fields[1].type), "object");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[1].type,
             sizeof(spec.defined_schemas[0].fields[1].type), "object");
#else
    strcpy(spec.defined_schemas[0].fields[1].type, "object");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[1].ref,
             sizeof(spec.defined_schemas[0].fields[1].ref), "ObjType1");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[1].ref,
             sizeof(spec.defined_schemas[0].fields[1].ref), "ObjType1");
#else
    strcpy(spec.defined_schemas[0].fields[1].ref, "ObjType1");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[2].name,
             sizeof(spec.defined_schemas[0].fields[2].name), "fObjNoExp");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[2].name,
             sizeof(spec.defined_schemas[0].fields[2].name), "fObjNoExp");
#else
    strcpy(spec.defined_schemas[0].fields[2].name, "fObjNoExp");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[2].type,
             sizeof(spec.defined_schemas[0].fields[2].type), "object");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[2].type,
             sizeof(spec.defined_schemas[0].fields[2].type), "object");
#else
    strcpy(spec.defined_schemas[0].fields[2].type, "object");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[2].ref,
             sizeof(spec.defined_schemas[0].fields[2].ref), "ObjType2");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[2].ref,
             sizeof(spec.defined_schemas[0].fields[2].ref), "ObjType2");
#else
    strcpy(spec.defined_schemas[0].fields[2].ref, "ObjType2");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[3].name,
             sizeof(spec.defined_schemas[0].fields[3].name), "fObjSpace");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[3].name,
             sizeof(spec.defined_schemas[0].fields[3].name), "fObjSpace");
#else
    strcpy(spec.defined_schemas[0].fields[3].name, "fObjSpace");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[3].type,
             sizeof(spec.defined_schemas[0].fields[3].type), "object");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[3].type,
             sizeof(spec.defined_schemas[0].fields[3].type), "object");
#else
    strcpy(spec.defined_schemas[0].fields[3].type, "object");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[3].ref,
             sizeof(spec.defined_schemas[0].fields[3].ref), "ObjType3");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[3].ref,
             sizeof(spec.defined_schemas[0].fields[3].ref), "ObjType3");
#else
    strcpy(spec.defined_schemas[0].fields[3].ref, "ObjType3");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[4].name,
             sizeof(spec.defined_schemas[0].fields[4].name), "fObjPipe");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[4].name,
             sizeof(spec.defined_schemas[0].fields[4].name), "fObjPipe");
#else
    strcpy(spec.defined_schemas[0].fields[4].name, "fObjPipe");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[4].type,
             sizeof(spec.defined_schemas[0].fields[4].type), "object");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[4].type,
             sizeof(spec.defined_schemas[0].fields[4].type), "object");
#else
    strcpy(spec.defined_schemas[0].fields[4].type, "object");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[4].ref,
             sizeof(spec.defined_schemas[0].fields[4].ref), "ObjType4");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[4].ref,
             sizeof(spec.defined_schemas[0].fields[4].ref), "ObjType4");
#else
    strcpy(spec.defined_schemas[0].fields[4].ref, "ObjType4");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[5].name,
             sizeof(spec.defined_schemas[0].fields[5].name), "fArrInt");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[5].name,
             sizeof(spec.defined_schemas[0].fields[5].name), "fArrInt");
#else
    strcpy(spec.defined_schemas[0].fields[5].name, "fArrInt");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[5].type,
             sizeof(spec.defined_schemas[0].fields[5].type), "array");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[5].type,
             sizeof(spec.defined_schemas[0].fields[5].type), "array");
#else
    strcpy(spec.defined_schemas[0].fields[5].type, "array");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[5].ref,
             sizeof(spec.defined_schemas[0].fields[5].ref), "integer");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[5].ref,
             sizeof(spec.defined_schemas[0].fields[5].ref), "integer");
#else
    strcpy(spec.defined_schemas[0].fields[5].ref, "integer");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[6].name,
             sizeof(spec.defined_schemas[0].fields[6].name), "fFormArrInt");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[6].name,
             sizeof(spec.defined_schemas[0].fields[6].name), "fFormArrInt");
#else
    strcpy(spec.defined_schemas[0].fields[6].name, "fFormArrInt");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[6].type,
             sizeof(spec.defined_schemas[0].fields[6].type), "array");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[6].type,
             sizeof(spec.defined_schemas[0].fields[6].type), "array");
#else
    strcpy(spec.defined_schemas[0].fields[6].type, "array");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[6].ref,
             sizeof(spec.defined_schemas[0].fields[6].ref), "integer");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[6].ref,
             sizeof(spec.defined_schemas[0].fields[6].ref), "integer");
#else
    strcpy(spec.defined_schemas[0].fields[6].ref, "integer");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[7].name,
             sizeof(spec.defined_schemas[0].fields[7].name), "fFormArrNum");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[7].name,
             sizeof(spec.defined_schemas[0].fields[7].name), "fFormArrNum");
#else
    strcpy(spec.defined_schemas[0].fields[7].name, "fFormArrNum");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[7].type,
             sizeof(spec.defined_schemas[0].fields[7].type), "array");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[7].type,
             sizeof(spec.defined_schemas[0].fields[7].type), "array");
#else
    strcpy(spec.defined_schemas[0].fields[7].type, "array");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[7].ref,
             sizeof(spec.defined_schemas[0].fields[7].ref), "number");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[7].ref,
             sizeof(spec.defined_schemas[0].fields[7].ref), "number");
#else
    strcpy(spec.defined_schemas[0].fields[7].ref, "number");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[8].name,
             sizeof(spec.defined_schemas[0].fields[8].name), "fFormArrBool");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[8].name,
             sizeof(spec.defined_schemas[0].fields[8].name), "fFormArrBool");
#else
    strcpy(spec.defined_schemas[0].fields[8].name, "fFormArrBool");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[8].type,
             sizeof(spec.defined_schemas[0].fields[8].type), "array");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[8].type,
             sizeof(spec.defined_schemas[0].fields[8].type), "array");
#else
    strcpy(spec.defined_schemas[0].fields[8].type, "array");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[8].ref,
             sizeof(spec.defined_schemas[0].fields[8].ref), "boolean");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[8].ref,
             sizeof(spec.defined_schemas[0].fields[8].ref), "boolean");
#else
    strcpy(spec.defined_schemas[0].fields[8].ref, "boolean");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[9].name,
             sizeof(spec.defined_schemas[0].fields[9].name), "fArrNum");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[9].name,
             sizeof(spec.defined_schemas[0].fields[9].name), "fArrNum");
#else
    strcpy(spec.defined_schemas[0].fields[9].name, "fArrNum");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[9].type,
             sizeof(spec.defined_schemas[0].fields[9].type), "array");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[9].type,
             sizeof(spec.defined_schemas[0].fields[9].type), "array");
#else
    strcpy(spec.defined_schemas[0].fields[9].type, "array");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[9].ref,
             sizeof(spec.defined_schemas[0].fields[9].ref), "number");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[9].ref,
             sizeof(spec.defined_schemas[0].fields[9].ref), "number");
#else
    strcpy(spec.defined_schemas[0].fields[9].ref, "number");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[10].name,
             sizeof(spec.defined_schemas[0].fields[10].name), "fArrBool");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[10].name,
             sizeof(spec.defined_schemas[0].fields[10].name), "fArrBool");
#else
    strcpy(spec.defined_schemas[0].fields[10].name, "fArrBool");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[10].type,
             sizeof(spec.defined_schemas[0].fields[10].type), "array");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[10].type,
             sizeof(spec.defined_schemas[0].fields[10].type), "array");
#else
    strcpy(spec.defined_schemas[0].fields[10].type, "array");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[10].ref,
             sizeof(spec.defined_schemas[0].fields[10].ref), "boolean");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[0].fields[10].ref,
             sizeof(spec.defined_schemas[0].fields[10].ref), "boolean");
#else
    strcpy(spec.defined_schemas[0].fields[10].ref, "boolean");
#endif
#endif

    /* ObjType1 (explode=1) */
    spec.defined_schemas[1].size = 4;
    spec.defined_schemas[1].fields = calloc(4, sizeof(struct StructField));
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[1].fields[0].name,
             sizeof(spec.defined_schemas[1].fields[0].name), "s");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[1].fields[0].name,
             sizeof(spec.defined_schemas[1].fields[0].name), "s");
#else
    strcpy(spec.defined_schemas[1].fields[0].name, "s");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[1].fields[0].type,
             sizeof(spec.defined_schemas[1].fields[0].type), "string");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[1].fields[0].type,
             sizeof(spec.defined_schemas[1].fields[0].type), "string");
#else
    strcpy(spec.defined_schemas[1].fields[0].type, "string");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[1].fields[1].name,
             sizeof(spec.defined_schemas[1].fields[1].name), "i");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[1].fields[1].name,
             sizeof(spec.defined_schemas[1].fields[1].name), "i");
#else
    strcpy(spec.defined_schemas[1].fields[1].name, "i");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[1].fields[1].type,
             sizeof(spec.defined_schemas[1].fields[1].type), "integer");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[1].fields[1].type,
             sizeof(spec.defined_schemas[1].fields[1].type), "integer");
#else
    strcpy(spec.defined_schemas[1].fields[1].type, "integer");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[1].fields[2].name,
             sizeof(spec.defined_schemas[1].fields[2].name), "n");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[1].fields[2].name,
             sizeof(spec.defined_schemas[1].fields[2].name), "n");
#else
    strcpy(spec.defined_schemas[1].fields[2].name, "n");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[1].fields[2].type,
             sizeof(spec.defined_schemas[1].fields[2].type), "number");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[1].fields[2].type,
             sizeof(spec.defined_schemas[1].fields[2].type), "number");
#else
    strcpy(spec.defined_schemas[1].fields[2].type, "number");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[1].fields[3].name,
             sizeof(spec.defined_schemas[1].fields[3].name), "b");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[1].fields[3].name,
             sizeof(spec.defined_schemas[1].fields[3].name), "b");
#else
    strcpy(spec.defined_schemas[1].fields[3].name, "b");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[1].fields[3].type,
             sizeof(spec.defined_schemas[1].fields[3].type), "boolean");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[1].fields[3].type,
             sizeof(spec.defined_schemas[1].fields[3].type), "boolean");
#else
    strcpy(spec.defined_schemas[1].fields[3].type, "boolean");
#endif
#endif

    /* ObjType2 (explode=0) */
    spec.defined_schemas[2].size = 4;
    spec.defined_schemas[2].fields = calloc(4, sizeof(struct StructField));
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[2].fields[0].name,
             sizeof(spec.defined_schemas[2].fields[0].name), "s");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[2].fields[0].name,
             sizeof(spec.defined_schemas[2].fields[0].name), "s");
#else
    strcpy(spec.defined_schemas[2].fields[0].name, "s");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[2].fields[0].type,
             sizeof(spec.defined_schemas[2].fields[0].type), "string");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[2].fields[0].type,
             sizeof(spec.defined_schemas[2].fields[0].type), "string");
#else
    strcpy(spec.defined_schemas[2].fields[0].type, "string");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[2].fields[1].name,
             sizeof(spec.defined_schemas[2].fields[1].name), "i");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[2].fields[1].name,
             sizeof(spec.defined_schemas[2].fields[1].name), "i");
#else
    strcpy(spec.defined_schemas[2].fields[1].name, "i");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[2].fields[1].type,
             sizeof(spec.defined_schemas[2].fields[1].type), "integer");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[2].fields[1].type,
             sizeof(spec.defined_schemas[2].fields[1].type), "integer");
#else
    strcpy(spec.defined_schemas[2].fields[1].type, "integer");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[2].fields[2].name,
             sizeof(spec.defined_schemas[2].fields[2].name), "n");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[2].fields[2].name,
             sizeof(spec.defined_schemas[2].fields[2].name), "n");
#else
    strcpy(spec.defined_schemas[2].fields[2].name, "n");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[2].fields[2].type,
             sizeof(spec.defined_schemas[2].fields[2].type), "number");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[2].fields[2].type,
             sizeof(spec.defined_schemas[2].fields[2].type), "number");
#else
    strcpy(spec.defined_schemas[2].fields[2].type, "number");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[2].fields[3].name,
             sizeof(spec.defined_schemas[2].fields[3].name), "b");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[2].fields[3].name,
             sizeof(spec.defined_schemas[2].fields[3].name), "b");
#else
    strcpy(spec.defined_schemas[2].fields[3].name, "b");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[2].fields[3].type,
             sizeof(spec.defined_schemas[2].fields[3].type), "boolean");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[2].fields[3].type,
             sizeof(spec.defined_schemas[2].fields[3].type), "boolean");
#else
    strcpy(spec.defined_schemas[2].fields[3].type, "boolean");
#endif
#endif

    /* ObjType3 (spaceDelimited) */
    spec.defined_schemas[3].size = 4;
    spec.defined_schemas[3].fields = calloc(4, sizeof(struct StructField));
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[3].fields[0].name,
             sizeof(spec.defined_schemas[3].fields[0].name), "s");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[3].fields[0].name,
             sizeof(spec.defined_schemas[3].fields[0].name), "s");
#else
    strcpy(spec.defined_schemas[3].fields[0].name, "s");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[3].fields[0].type,
             sizeof(spec.defined_schemas[3].fields[0].type), "string");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[3].fields[0].type,
             sizeof(spec.defined_schemas[3].fields[0].type), "string");
#else
    strcpy(spec.defined_schemas[3].fields[0].type, "string");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[3].fields[1].name,
             sizeof(spec.defined_schemas[3].fields[1].name), "i");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[3].fields[1].name,
             sizeof(spec.defined_schemas[3].fields[1].name), "i");
#else
    strcpy(spec.defined_schemas[3].fields[1].name, "i");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[3].fields[1].type,
             sizeof(spec.defined_schemas[3].fields[1].type), "integer");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[3].fields[1].type,
             sizeof(spec.defined_schemas[3].fields[1].type), "integer");
#else
    strcpy(spec.defined_schemas[3].fields[1].type, "integer");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[3].fields[2].name,
             sizeof(spec.defined_schemas[3].fields[2].name), "n");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[3].fields[2].name,
             sizeof(spec.defined_schemas[3].fields[2].name), "n");
#else
    strcpy(spec.defined_schemas[3].fields[2].name, "n");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[3].fields[2].type,
             sizeof(spec.defined_schemas[3].fields[2].type), "number");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[3].fields[2].type,
             sizeof(spec.defined_schemas[3].fields[2].type), "number");
#else
    strcpy(spec.defined_schemas[3].fields[2].type, "number");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[3].fields[3].name,
             sizeof(spec.defined_schemas[3].fields[3].name), "b");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[3].fields[3].name,
             sizeof(spec.defined_schemas[3].fields[3].name), "b");
#else
    strcpy(spec.defined_schemas[3].fields[3].name, "b");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[3].fields[3].type,
             sizeof(spec.defined_schemas[3].fields[3].type), "boolean");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[3].fields[3].type,
             sizeof(spec.defined_schemas[3].fields[3].type), "boolean");
#else
    strcpy(spec.defined_schemas[3].fields[3].type, "boolean");
#endif
#endif

    /* ObjType4 (pipeDelimited) */
    spec.defined_schemas[4].size = 4;
    spec.defined_schemas[4].fields = calloc(4, sizeof(struct StructField));
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[4].fields[0].name,
             sizeof(spec.defined_schemas[4].fields[0].name), "s");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[4].fields[0].name,
             sizeof(spec.defined_schemas[4].fields[0].name), "s");
#else
    strcpy(spec.defined_schemas[4].fields[0].name, "s");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[4].fields[0].type,
             sizeof(spec.defined_schemas[4].fields[0].type), "string");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[4].fields[0].type,
             sizeof(spec.defined_schemas[4].fields[0].type), "string");
#else
    strcpy(spec.defined_schemas[4].fields[0].type, "string");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[4].fields[1].name,
             sizeof(spec.defined_schemas[4].fields[1].name), "i");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[4].fields[1].name,
             sizeof(spec.defined_schemas[4].fields[1].name), "i");
#else
    strcpy(spec.defined_schemas[4].fields[1].name, "i");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[4].fields[1].type,
             sizeof(spec.defined_schemas[4].fields[1].type), "integer");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[4].fields[1].type,
             sizeof(spec.defined_schemas[4].fields[1].type), "integer");
#else
    strcpy(spec.defined_schemas[4].fields[1].type, "integer");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[4].fields[2].name,
             sizeof(spec.defined_schemas[4].fields[2].name), "n");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[4].fields[2].name,
             sizeof(spec.defined_schemas[4].fields[2].name), "n");
#else
    strcpy(spec.defined_schemas[4].fields[2].name, "n");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[4].fields[2].type,
             sizeof(spec.defined_schemas[4].fields[2].type), "number");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[4].fields[2].type,
             sizeof(spec.defined_schemas[4].fields[2].type), "number");
#else
    strcpy(spec.defined_schemas[4].fields[2].type, "number");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[4].fields[3].name,
             sizeof(spec.defined_schemas[4].fields[3].name), "b");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[4].fields[3].name,
             sizeof(spec.defined_schemas[4].fields[3].name), "b");
#else
    strcpy(spec.defined_schemas[4].fields[3].name, "b");
#endif
#endif
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[4].fields[3].type,
             sizeof(spec.defined_schemas[4].fields[3].type), "boolean");
#else
#if defined(_MSC_VER)
    strcpy_s(spec.defined_schemas[4].fields[3].type,
             sizeof(spec.defined_schemas[4].fields[3].type), "boolean");
#else
    strcpy(spec.defined_schemas[4].fields[3].type, "boolean");
#endif
#endif

    op.n_req_body_media_types = 1;
    op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
    op.req_body_media_types[0].name =
        (char *)(size_t)(size_t) "application/x-www-form-urlencoded";
    op.req_body_media_types[0].n_encoding = 11;
    op.req_body_media_types[0].encoding =
        calloc(11, sizeof(struct OpenAPI_Encoding));

    op.req_body_media_types[0].encoding[0].name =
        (char *)(size_t)(size_t) "fArrStrRsv";
    op.req_body_media_types[0].encoding[0].style = OA_STYLE_FORM;
    op.req_body_media_types[0].encoding[0].style_set = 1;
    op.req_body_media_types[0].encoding[0].allow_reserved = 1;
    op.req_body_media_types[0].encoding[0].allow_reserved_set = 1;

    op.req_body_media_types[0].encoding[1].name =
        (char *)(size_t)(size_t) "fObjExp";
    op.req_body_media_types[0].encoding[1].style = OA_STYLE_FORM;
    op.req_body_media_types[0].encoding[1].style_set = 1;
    op.req_body_media_types[0].encoding[1].explode = 1;
    op.req_body_media_types[0].encoding[1].explode_set = 1;
    op.req_body_media_types[0].encoding[1].allow_reserved = 1;
    op.req_body_media_types[0].encoding[1].allow_reserved_set = 1;

    op.req_body_media_types[0].encoding[2].name =
        (char *)(size_t)(size_t) "fObjNoExp";
    op.req_body_media_types[0].encoding[2].style = OA_STYLE_FORM;
    op.req_body_media_types[0].encoding[2].style_set = 1;
    op.req_body_media_types[0].encoding[2].explode = 0;
    op.req_body_media_types[0].encoding[2].explode_set = 1;

    op.req_body_media_types[0].encoding[3].name =
        (char *)(size_t)(size_t) "fObjSpace";
    op.req_body_media_types[0].encoding[3].style = OA_STYLE_SPACE_DELIMITED;
    op.req_body_media_types[0].encoding[3].style_set = 1;

    op.req_body_media_types[0].encoding[4].name =
        (char *)(size_t)(size_t) "fObjPipe";
    op.req_body_media_types[0].encoding[4].style = OA_STYLE_PIPE_DELIMITED;
    op.req_body_media_types[0].encoding[4].style_set = 1;

    op.req_body_media_types[0].encoding[5].name =
        (char *)(size_t)(size_t) "fArrInt";
    op.req_body_media_types[0].encoding[5].style = OA_STYLE_SPACE_DELIMITED;
    op.req_body_media_types[0].encoding[5].style_set = 1;

    op.req_body_media_types[0].encoding[6].name =
        (char *)(size_t)(size_t) "fFormArrInt";
    op.req_body_media_types[0].encoding[6].style = OA_STYLE_FORM;
    op.req_body_media_types[0].encoding[6].style_set = 1;
    op.req_body_media_types[0].encoding[6].explode = 1;
    op.req_body_media_types[0].encoding[6].explode_set = 1;

    op.req_body_media_types[0].encoding[7].name =
        (char *)(size_t)(size_t) "fFormArrNum";
    op.req_body_media_types[0].encoding[7].style = OA_STYLE_FORM;
    op.req_body_media_types[0].encoding[7].style_set = 1;
    op.req_body_media_types[0].encoding[7].explode = 1;
    op.req_body_media_types[0].encoding[7].explode_set = 1;

    op.req_body_media_types[0].encoding[8].name =
        (char *)(size_t)(size_t) "fFormArrBool";
    op.req_body_media_types[0].encoding[8].style = OA_STYLE_FORM;
    op.req_body_media_types[0].encoding[8].style_set = 1;
    op.req_body_media_types[0].encoding[8].explode = 1;
    op.req_body_media_types[0].encoding[8].explode_set = 1;

    op.req_body_media_types[0].encoding[9].name =
        (char *)(size_t)(size_t) "fArrNum";
    op.req_body_media_types[0].encoding[9].style = OA_STYLE_SPACE_DELIMITED;
    op.req_body_media_types[0].encoding[9].style_set = 1;

    op.req_body_media_types[0].encoding[10].name =
        (char *)(size_t)(size_t) "fArrBool";
    op.req_body_media_types[0].encoding[10].style = OA_STYLE_SPACE_DELIMITED;
    op.req_body_media_types[0].encoding[10].style_set = 1;

#if defined(_MSC_VER)
    if (((fp = cdd_test_tmpfile_global()) == NULL))
      fp = NULL;
#else
    fp = cdd_test_tmpfile_global();
#endif
    g_io_calls = 0;
    g_fail_io_after = io_fail;
    rc = codegen_client_write_body(fp, &op, &spec, "/path", NULL);
    g_fail_io_after = -1;
    if (fp)
      fclose(fp);

    free(spec.defined_schemas[0].fields);
    free(spec.defined_schemas[1].fields);
    free(spec.defined_schemas[2].fields);
    free(spec.defined_schemas[3].fields);
    free(spec.defined_schemas[4].fields);
    free(spec.defined_schemas);
    free(spec.defined_schema_names[0]);
    free(spec.defined_schema_names[1]);
    free(spec.defined_schema_names[2]);
    free(spec.defined_schema_names[3]);
    free(spec.defined_schema_names[4]);
    free(spec.defined_schema_names);
    free(op.req_body_media_types[0].encoding);
    free(op.req_body_media_types);

    if (rc != CDD_C_SUCCESS)
      all_success = 0;
    if (all_success)
      break;
  }
  PASS();
}

SUITE(client_body_mega_suite) { RUN_TEST(test_client_body_form_mega); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_CLIENT_BODY_MEGA_H */
