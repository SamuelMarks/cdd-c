/**
 * @file test_codegen_struct_reflect.h
 * @brief Unit tests for runtime reflection descriptor generation, field/array
 * accessors, registry APIs, and fixed-array/bit-field code generation.
 *
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_STRUCT_REFLECT_H
#define TEST_CODEGEN_STRUCT_REFLECT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_codegen_struct_common.h"
/* clang-format on */

/**
 * @brief Unit test test_struct_reflection_and_codegen.
 * @return Greatest test result.
 */
TEST test_struct_reflection_and_codegen(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  if (tmp) {
    struct CodegenStructConfig cfg;
    struct StructFields sf_empty;
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_empty));
    {
      struct DummyReflStruct {
        int id;
        double score;
        int *opt_int;
        char *str_f;
      } dummy_inst;
      int opt_val = 99;
      int out_int = 0;
      const char *out_str = NULL;
      struct cdd_field_info dummy_fields[6];
      struct cdd_type_info dummy_ti;
      const struct cdd_field_info *found_f = NULL;
      struct StructFields sf_rf;
      int fail_step;
      cdd_c_error_t rc_step;
      const char *g_names[2] = {"AlphaStruct", "BetaStruct"};
      const char *bad_names[1] = {""};

      memset(&cfg, 0, sizeof(cfg));
      cfg.guard_macro = "TEST_REFL_GUARD";

      memset(&dummy_inst, 0, sizeof(dummy_inst));
      dummy_inst.id = 42;
      dummy_inst.score = 3.5;
      dummy_inst.opt_int = &opt_val;
      dummy_inst.str_f = NULL;
      memset(dummy_fields, 0, sizeof(dummy_fields));
      dummy_fields[0].name = "id";
      dummy_fields[0].wire_name = "userId";
      dummy_fields[0].de_wire_name = "inUserId";
      dummy_fields[0].kind = CDD_FIELD_KIND_INTEGER;
      dummy_fields[0].offset = offsetof(struct DummyReflStruct, id);
      dummy_fields[0].size = sizeof(int);
      dummy_fields[1].name = "skipped";
      dummy_fields[1].wire_name = "skipped";
      dummy_fields[1].skip = 1;
      dummy_fields[2].name = "score";
      dummy_fields[2].wire_name = "score";
      dummy_fields[2].kind = CDD_FIELD_KIND_NUMBER;
      dummy_fields[2].offset = offsetof(struct DummyReflStruct, score);
      dummy_fields[2].size = sizeof(double);
      dummy_fields[3].name = "bf";
      dummy_fields[3].wire_name = "bf";
      dummy_fields[3].kind = CDD_FIELD_KIND_BOOLEAN;
      dummy_fields[3].bit_width = 3;
      dummy_fields[3].get_bitfield = test_bf_get;
      dummy_fields[3].set_bitfield = test_bf_set;
      dummy_fields[4].name = "opt_int";
      dummy_fields[4].wire_name = "opt_int";
      dummy_fields[4].kind = CDD_FIELD_KIND_ENUM;
      dummy_fields[4].offset = offsetof(struct DummyReflStruct, opt_int);
      dummy_fields[4].is_pointer = 1;
      dummy_fields[5].name = "str_f";
      dummy_fields[5].wire_name = "str_f";
      dummy_fields[5].kind = CDD_FIELD_KIND_STRING;
      dummy_fields[5].offset = offsetof(struct DummyReflStruct, str_f);
      dummy_fields[5].is_pointer = 1;
      dummy_fields[5].skip = 1;

      dummy_ti.name = "DummyReflStruct";
      dummy_ti.size = sizeof(struct DummyReflStruct);
      dummy_ti.is_union = 0;
      dummy_ti.fields = dummy_fields;
      dummy_ti.n_fields = 6;
      dummy_ti.tag_offset = 0;

      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_find_field(NULL, "id", &found_f));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_find_field(&dummy_ti, NULL, &found_f));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_find_field(&dummy_ti, "id", NULL));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_find_field(&dummy_ti, "id", &found_f));
      ASSERT_STR_EQ("id", found_f->name);
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_find_field(&dummy_ti, "userId", &found_f));
      ASSERT_STR_EQ("id", found_f->name);
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_find_field(&dummy_ti, "inUserId", &found_f));
      ASSERT_STR_EQ("id", found_f->name);
      ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                cdd_reflect_find_field(&dummy_ti, "nonexistent", &found_f));

      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_visit_fields(NULL, &dummy_ti, NULL, NULL));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_visit_fields(&dummy_inst, NULL, NULL, NULL));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_visit_fields(&dummy_inst, &dummy_ti, NULL, NULL));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_visit_fields_mut(NULL, &dummy_ti, NULL, NULL));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_visit_fields_mut(&dummy_inst, NULL, NULL, NULL));
      ASSERT_EQ(
          CDD_C_ERROR_INVALID_ARGUMENT,
          cdd_reflect_visit_fields_mut(&dummy_inst, &dummy_ti, NULL, NULL));
      {
        int v_cnt = 0;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_visit_fields(&dummy_inst, &dummy_ti,
                                           test_reflect_visitor_cb, &v_cnt));
        ASSERT_EQ(4, v_cnt);
        v_cnt = -1;
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_visit_fields(&dummy_inst, &dummy_ti,
                                           test_reflect_visitor_cb, &v_cnt));

        v_cnt = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_visit_fields_mut(
                                     &dummy_inst, &dummy_ti,
                                     test_reflect_mut_visitor_cb, &v_cnt));
        ASSERT_EQ(4, v_cnt);
        ASSERT_EQ(52, dummy_inst.id);
        v_cnt = -1;
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_visit_fields_mut(&dummy_inst, &dummy_ti,
                                               test_reflect_mut_visitor_cb,
                                               &v_cnt));
      }

      /* Test cdd_reflect_get_int & cdd_reflect_set_int */
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_get_int(NULL, &dummy_fields[0], &out_int));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_get_int(&dummy_inst, NULL, &out_int));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_get_int(&dummy_inst, &dummy_fields[0], NULL));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_get_int(&dummy_inst, &dummy_fields[2], &out_int));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_get_int(&dummy_inst, &dummy_fields[0], &out_int));
      ASSERT_EQ(52, out_int);
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_get_int(&dummy_inst, &dummy_fields[3], &out_int));
      ASSERT_EQ(7, out_int);
      dummy_fields[3].get_bitfield = NULL;
      ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                cdd_reflect_get_int(&dummy_inst, &dummy_fields[3], &out_int));
      dummy_fields[3].get_bitfield = test_bf_get;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_get_int(&dummy_inst, &dummy_fields[4], &out_int));
      ASSERT_EQ(99, out_int);
      dummy_inst.opt_int = NULL;
      ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                cdd_reflect_get_int(&dummy_inst, &dummy_fields[4], &out_int));
      dummy_inst.opt_int = &opt_val;

      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_set_int(NULL, &dummy_fields[0], 10));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_set_int(&dummy_inst, NULL, 10));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_set_int(&dummy_inst, &dummy_fields[2], 10));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_set_int(&dummy_inst, &dummy_fields[0], 88));
      ASSERT_EQ(88, dummy_inst.id);
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_set_int(&dummy_inst, &dummy_fields[3], 15));
      ASSERT_EQ(15, dummy_inst.id);
      dummy_fields[3].set_bitfield = NULL;
      ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                cdd_reflect_set_int(&dummy_inst, &dummy_fields[3], 15));
      dummy_fields[3].set_bitfield = test_bf_set;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_set_int(&dummy_inst, &dummy_fields[4], 77));
      ASSERT_EQ(77, opt_val);
      dummy_inst.opt_int = NULL;
      ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                cdd_reflect_set_int(&dummy_inst, &dummy_fields[4], 77));
      dummy_inst.opt_int = &opt_val;

      /* Test cdd_reflect_get_string & cdd_reflect_set_string */
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_get_string(NULL, &dummy_fields[5], &out_str));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_get_string(&dummy_inst, NULL, &out_str));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_get_string(&dummy_inst, &dummy_fields[5], NULL));
      ASSERT_EQ(
          CDD_C_ERROR_INVALID_ARGUMENT,
          cdd_reflect_get_string(&dummy_inst, &dummy_fields[0], &out_str));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_set_string(NULL, &dummy_fields[5], "hi"));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_set_string(&dummy_inst, NULL, "hi"));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_set_string(&dummy_inst, &dummy_fields[0], "hi"));
#ifdef CDD_BUILD_TESTS
      g_cdd_alloc_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                cdd_reflect_set_string(&dummy_inst, &dummy_fields[5], "hi"));
      g_cdd_alloc_fail = 0;
#endif
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_set_string(&dummy_inst, &dummy_fields[5], "hello"));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_get_string(
                                   &dummy_inst, &dummy_fields[5], &out_str));
      ASSERT_STR_EQ("hello", out_str);
      /* Overwrite non-NULL string and then clear with NULL */
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_set_string(&dummy_inst, &dummy_fields[5], "world"));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_get_string(
                                   &dummy_inst, &dummy_fields[5], &out_str));
      ASSERT_STR_EQ("world", out_str);
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_set_string(&dummy_inst, &dummy_fields[5], NULL));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_get_string(
                                   &dummy_inst, &dummy_fields[5], &out_str));
      ASSERT_EQ(NULL, out_str);

      /* Test width-safe int64, double, bool, and inline char[N] accessors */
      {
        struct WidthTestStruct {
          int8_t i8;
          uint8_t u8;
          int16_t i16;
          uint16_t u16;
          uint32_t u32;
          int64_t i64;
          uint64_t u64;
          float f32;
          double f64;
          double *opt_f64;
          int b_flag;
          char inline_str[8];
        } ws;
        struct cdd_field_info wf[12];
        int64_t out_i64 = 0;
        double out_f64 = 0.0;
        double opt_d_val = 6.25;
        int out_bool = 0;
        const char *in_s = NULL;

        memset(&ws, 0, sizeof(ws));
        memset(wf, 0, sizeof(wf));
        ws.i8 = -12;
        ws.u8 = 200U;
        ws.i16 = -1234;
        ws.u16 = 50000U;
        ws.u32 = 3000000000UL;
        ws.i64 = (int64_t)-5000000L * (int64_t)1000L;
        ws.u64 = (uint64_t)7000000UL * (uint64_t)1000UL;
        ws.f32 = 1.5f;
        ws.f64 = 3.125;
        ws.opt_f64 = &opt_d_val;
        ws.b_flag = 1;
        CDD_STRCPY(ws.inline_str, sizeof(ws.inline_str), "init");

        wf[0].name = "i8";
        wf[0].c_type = "int8_t";
        wf[0].kind = CDD_FIELD_KIND_INTEGER;
        wf[0].offset = offsetof(struct WidthTestStruct, i8);
        wf[0].size = sizeof(int8_t);

        wf[1].name = "u8";
        wf[1].c_type = "uint8_t";
        wf[1].kind = CDD_FIELD_KIND_INTEGER;
        wf[1].offset = offsetof(struct WidthTestStruct, u8);
        wf[1].size = sizeof(uint8_t);

        wf[2].name = "i16";
        wf[2].c_type = "int16_t";
        wf[2].kind = CDD_FIELD_KIND_INTEGER;
        wf[2].offset = offsetof(struct WidthTestStruct, i16);
        wf[2].size = sizeof(int16_t);

        wf[3].name = "u16";
        wf[3].c_type = "unsigned short";
        wf[3].kind = CDD_FIELD_KIND_INTEGER;
        wf[3].offset = offsetof(struct WidthTestStruct, u16);
        wf[3].size = sizeof(uint16_t);

        wf[4].name = "u32";
        wf[4].c_type = "uint32_t";
        wf[4].kind = CDD_FIELD_KIND_INTEGER;
        wf[4].offset = offsetof(struct WidthTestStruct, u32);
        wf[4].size = sizeof(uint32_t);

        wf[5].name = "i64";
        wf[5].c_type = "int64_t";
        wf[5].kind = CDD_FIELD_KIND_INTEGER;
        wf[5].offset = offsetof(struct WidthTestStruct, i64);
        wf[5].size = sizeof(int64_t);

        wf[6].name = "u64";
        wf[6].c_type = "uint64_t";
        wf[6].kind = CDD_FIELD_KIND_INTEGER;
        wf[6].offset = offsetof(struct WidthTestStruct, u64);
        wf[6].size = sizeof(uint64_t);

        wf[7].name = "f32";
        wf[7].c_type = "float";
        wf[7].kind = CDD_FIELD_KIND_NUMBER;
        wf[7].offset = offsetof(struct WidthTestStruct, f32);
        wf[7].size = sizeof(float);

        wf[8].name = "f64";
        wf[8].c_type = "double";
        wf[8].kind = CDD_FIELD_KIND_NUMBER;
        wf[8].offset = offsetof(struct WidthTestStruct, f64);
        wf[8].size = sizeof(double);

        wf[9].name = "opt_f64";
        wf[9].c_type = "double *";
        wf[9].kind = CDD_FIELD_KIND_NUMBER;
        wf[9].offset = offsetof(struct WidthTestStruct, opt_f64);
        wf[9].size = sizeof(double *);
        wf[9].is_pointer = 1;

        wf[10].name = "b_flag";
        wf[10].c_type = "int";
        wf[10].kind = CDD_FIELD_KIND_BOOLEAN;
        wf[10].offset = offsetof(struct WidthTestStruct, b_flag);
        wf[10].size = sizeof(int);

        wf[11].name = "inline_str";
        wf[11].c_type = "char[8]";
        wf[11].kind = CDD_FIELD_KIND_STRING;
        wf[11].offset = offsetof(struct WidthTestStruct, inline_str);
        wf[11].size = sizeof(ws.inline_str);
        wf[11].fixed_array_len = 8;
        wf[11].is_pointer = 0;

        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_get_int64(NULL, &wf[0], &out_i64));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_get_int64(&ws, NULL, &out_i64));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_get_int64(&ws, &wf[0], NULL));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_get_int64(&ws, &wf[7], &out_i64));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_set_int64(NULL, &wf[0], 1));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_set_int64(&ws, NULL, 1));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_set_int64(&ws, &wf[7], 1));

        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_get_int64(&ws, &wf[0], &out_i64));
        ASSERT_EQ(-12, (int)out_i64);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_get_int64(&ws, &wf[1], &out_i64));
        ASSERT_EQ(200, (int)out_i64);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_get_int64(&ws, &wf[2], &out_i64));
        ASSERT_EQ(-1234, (int)out_i64);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_get_int64(&ws, &wf[3], &out_i64));
        ASSERT_EQ(50000, (int)out_i64);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_get_int64(&ws, &wf[4], &out_i64));
        ASSERT(out_i64 == (int64_t)3000000000UL);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_get_int64(&ws, &wf[5], &out_i64));
        ASSERT(out_i64 == (int64_t)-5000000L * (int64_t)1000L);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_get_int64(&ws, &wf[6], &out_i64));
        ASSERT(out_i64 == (int64_t)7000000L * (int64_t)1000L);

        /* Verify setting 1-byte and 2-byte fields does not clobber neighbors
         */
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_int64(&ws, &wf[0], -5));
        ASSERT_EQ(-5, (int)ws.i8);
        ASSERT_EQ(200, (int)ws.u8);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_int64(&ws, &wf[2], -777));
        ASSERT_EQ(-777, (int)ws.i16);
        ASSERT_EQ(50000, (int)ws.u16);
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_set_int64(&ws, &wf[5],
                                        (int64_t)9000000L * (int64_t)1000L));
        ASSERT(ws.i64 == (int64_t)9000000L * (int64_t)1000L);

        /* Double & Float accessors */
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_get_double(NULL, &wf[7], &out_f64));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_get_double(&ws, NULL, &out_f64));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_get_double(&ws, &wf[7], NULL));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_get_double(&ws, &wf[0], &out_f64));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_set_double(NULL, &wf[7], 1.0));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_set_double(&ws, NULL, 1.0));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_set_double(&ws, &wf[0], 1.0));

        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_get_double(&ws, &wf[7], &out_f64));
        ASSERT_EQ(1.5, out_f64);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_double(&ws, &wf[7], 2.25));
        ASSERT_EQ(2.25f, ws.f32);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_get_double(&ws, &wf[8], &out_f64));
        ASSERT_EQ(3.125, out_f64);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_double(&ws, &wf[8], 4.5));
        ASSERT_EQ(4.5, ws.f64);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_get_double(&ws, &wf[9], &out_f64));
        ASSERT_EQ(6.25, out_f64);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_double(&ws, &wf[9], 8.75));
        ASSERT_EQ(8.75, opt_d_val);
        ws.opt_f64 = NULL;
        ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                  cdd_reflect_get_double(&ws, &wf[9], &out_f64));
        ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                  cdd_reflect_set_double(&ws, &wf[9], 1.0));
        ws.opt_f64 = &opt_d_val;

        /* Bool accessors */
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_get_bool(NULL, &wf[10], &out_bool));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_get_bool(&ws, NULL, &out_bool));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_get_bool(&ws, &wf[10], NULL));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_get_bool(&ws, &wf[0], &out_bool));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_set_bool(NULL, &wf[10], 0));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_set_bool(&ws, NULL, 0));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_set_bool(&ws, &wf[0], 0));

        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_get_bool(&ws, &wf[10], &out_bool));
        ASSERT_EQ(1, out_bool);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_bool(&ws, &wf[10], 0));
        ASSERT_EQ(0, ws.b_flag);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_bool(&ws, &wf[10], 42));
        ASSERT_EQ(1, ws.b_flag);
        wf[10].bit_width = 1;
        wf[10].get_bitfield = NULL;
        ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                  cdd_reflect_get_bool(&ws, &wf[10], &out_bool));
        wf[10].bit_width = 0;

        /* Inline fixed-size char[8] string accessors */
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_get_string(&ws, &wf[11], &in_s));
        ASSERT_STR_EQ("init", in_s);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_string(&ws, &wf[11], "short"));
        ASSERT_STR_EQ("short", ws.inline_str);
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_set_string(&ws, &wf[11], "too_long_str"));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_string(&ws, &wf[11], NULL));
        ASSERT_STR_EQ("", ws.inline_str);
        wf[11].fixed_array_len = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_string(&ws, &wf[11], "by_sz"));
        ASSERT_STR_EQ("by_sz", ws.inline_str);

        /* Test cdd_reflect_get_uint64, cdd_reflect_set_uint64, and get_int
         * range check */
        {
          uint64_t out_u64 = 0;
          int out_narrow = 0;
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_get_uint64(NULL, &wf[6], &out_u64));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_get_uint64(&ws, NULL, &out_u64));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_get_uint64(&ws, &wf[6], NULL));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_get_uint64(&ws, &wf[7], &out_u64));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_get_uint64(&ws, &wf[6], &out_u64));
          ASSERT(out_u64 == (uint64_t)7000000UL * (uint64_t)1000UL);
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_set_uint64(
                        &ws, &wf[6], (uint64_t)8000000UL * (uint64_t)1000UL));
          ASSERT(ws.u64 == (uint64_t)8000000UL * (uint64_t)1000UL);
          /* Value > INT_MAX must fail cdd_reflect_get_int */
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_get_int(&ws, &wf[6], &out_narrow));
        }

        /* Test nullable pointer scalar widths (int8*, int16*, int64*, float*,
         * size_t*) */
        {
          struct PtrWidthStruct {
            int8_t *p_i8;
            uint16_t *p_u16;
            int64_t *p_i64;
            size_t *p_sz;
            float *p_f32;
            int *p_elem;
          } pws;
          int8_t v_i8 = -42;
          uint16_t v_u16 = 60000U;
          int64_t v_i64 = (int64_t)-3000000L * (int64_t)1000L;
          size_t v_sz = 123456U;
          float v_f32 = 3.5f;
          int v_elem = 777;
          struct cdd_field_info pwf[6];

          memset(&pws, 0, sizeof(pws));
          memset(pwf, 0, sizeof(pwf));
          pws.p_i8 = &v_i8;
          pws.p_u16 = &v_u16;
          pws.p_i64 = &v_i64;
          pws.p_sz = &v_sz;
          pws.p_f32 = &v_f32;
          pws.p_elem = &v_elem;

          pwf[0].name = "p_i8";
          pwf[0].c_type = "int8_t *";
          pwf[0].kind = CDD_FIELD_KIND_INTEGER;
          pwf[0].offset = offsetof(struct PtrWidthStruct, p_i8);
          pwf[0].size = sizeof(int8_t *);
          pwf[0].is_pointer = 1;

          pwf[1].name = "p_u16";
          pwf[1].c_type = "uint16_t *";
          pwf[1].kind = CDD_FIELD_KIND_INTEGER;
          pwf[1].offset = offsetof(struct PtrWidthStruct, p_u16);
          pwf[1].size = sizeof(uint16_t *);
          pwf[1].is_pointer = 1;

          pwf[2].name = "p_i64";
          pwf[2].c_type = "int64_t *";
          pwf[2].kind = CDD_FIELD_KIND_INTEGER;
          pwf[2].offset = offsetof(struct PtrWidthStruct, p_i64);
          pwf[2].size = sizeof(int64_t *);
          pwf[2].is_pointer = 1;

          pwf[3].name = "p_sz";
          pwf[3].c_type = "size_t *";
          pwf[3].kind = CDD_FIELD_KIND_INTEGER;
          pwf[3].offset = offsetof(struct PtrWidthStruct, p_sz);
          pwf[3].size = sizeof(size_t *);
          pwf[3].is_pointer = 1;

          pwf[4].name = "p_f32";
          pwf[4].c_type = "float *";
          pwf[4].kind = CDD_FIELD_KIND_NUMBER;
          pwf[4].offset = offsetof(struct PtrWidthStruct, p_f32);
          pwf[4].size = sizeof(float *);
          pwf[4].is_pointer = 1;

          pwf[5].name = "p_elem";
          pwf[5].c_type = NULL;
          pwf[5].kind = CDD_FIELD_KIND_INTEGER;
          pwf[5].offset = offsetof(struct PtrWidthStruct, p_elem);
          pwf[5].size = sizeof(int *);
          pwf[5].elem_size = sizeof(int);
          pwf[5].is_pointer = 1;

          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_get_int64(&pws, &pwf[0], &out_i64));
          ASSERT_EQ(-42, (int)out_i64);
          ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_int64(&pws, &pwf[0], -99));
          ASSERT_EQ(-99, (int)v_i8);

          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_get_int64(&pws, &pwf[1], &out_i64));
          ASSERT_EQ(60000, (int)out_i64);
          ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_int64(&pws, &pwf[1], 12345));
          ASSERT_EQ(12345, (int)v_u16);

          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_get_int64(&pws, &pwf[2], &out_i64));
          ASSERT(out_i64 == (int64_t)-3000000L * (int64_t)1000L);
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_set_int64(&pws, &pwf[2],
                                          (int64_t)4000000L * (int64_t)1000L));
          ASSERT(v_i64 == (int64_t)4000000L * (int64_t)1000L);

          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_get_int64(&pws, &pwf[3], &out_i64));
          ASSERT_EQ(123456, (int)out_i64);

          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_get_double(&pws, &pwf[4], &out_f64));
          ASSERT_EQ(3.5, out_f64);
          ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_double(&pws, &pwf[4], 7.25));
          ASSERT_EQ(7.25f, v_f32);

          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_get_int64(&pws, &pwf[5], &out_i64));
          ASSERT_EQ(777, (int)out_i64);
          pwf[5].elem_size = 0;
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_get_int64(&pws, &pwf[5], &out_i64));
          ASSERT_EQ(777, (int)out_i64);
          pwf[5].c_type = "custom_int_t *";
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_get_int64(&pws, &pwf[5], &out_i64));
          ASSERT_EQ(777, (int)out_i64);
        }

        /* Test Tagged Union active-variant visitation */
        {
          struct DummyUnion {
            int tag;
            union {
              int branch_a;
              double branch_b;
            } data;
          } du;
          struct cdd_field_info ufields[2];
          struct cdd_type_info uti;
          int u_cnt = 0;

          memset(&du, 0, sizeof(du));
          memset(ufields, 0, sizeof(ufields));
          ufields[0].name = "id";
          ufields[0].kind = CDD_FIELD_KIND_INTEGER;
          ufields[0].offset = offsetof(struct DummyUnion, data.branch_a);
          ufields[0].size = sizeof(int);
          ufields[1].name = "branch_b";
          ufields[1].kind = CDD_FIELD_KIND_NUMBER;
          ufields[1].offset = offsetof(struct DummyUnion, data.branch_b);
          ufields[1].size = sizeof(double);

          uti.name = "DummyUnion";
          uti.size = sizeof(struct DummyUnion);
          uti.is_union = 1;
          uti.fields = ufields;
          uti.n_fields = 2;
          uti.tag_offset = offsetof(struct DummyUnion, tag);

          du.tag = 0;
          du.data.branch_a = 10;
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_visit_fields(&du, &uti, test_reflect_visitor_cb,
                                             &u_cnt));
          ASSERT_EQ(1, u_cnt);

          u_cnt = 0;
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_visit_fields_mut(
                        &du, &uti, test_reflect_mut_visitor_cb, &u_cnt));
          ASSERT_EQ(1, u_cnt);
          ASSERT_EQ(20, du.data.branch_a);

          /* Out-of-bounds tag visits 0 fields */
          du.tag = -1;
          u_cnt = 0;
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_visit_fields(&du, &uti, test_reflect_visitor_cb,
                                             &u_cnt));
          ASSERT_EQ(0, u_cnt);
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_visit_fields_mut(
                        &du, &uti, test_reflect_mut_visitor_cb, &u_cnt));
          ASSERT_EQ(0, u_cnt);

          /* Skipped active union branch */
          du.tag = 1;
          ufields[1].skip = 1;
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_visit_fields(&du, &uti, test_reflect_visitor_cb,
                                             &u_cnt));
          ASSERT_EQ(0, u_cnt);
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_visit_fields_mut(
                        &du, &uti, test_reflect_mut_visitor_cb, &u_cnt));
          ASSERT_EQ(0, u_cnt);
          /* Bit-field union branch coverage */
          ufields[1].skip = 0;
          ufields[1].bit_width = 2;
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_visit_fields(&du, &uti, test_reflect_visitor_cb,
                                             &u_cnt));
          ASSERT_EQ(1, u_cnt);
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_visit_fields_mut(
                        &du, &uti, test_reflect_mut_visitor_cb, &u_cnt));
          ASSERT_EQ(2, u_cnt);
        }
      }

      /* Test cdd_reflect_array_len, cdd_reflect_array_at, and runtime aliases
       */
      {
        struct ArrayReflDummy {
          size_t n_dyn_ints;
          int *dyn_ints;
          int fix_ints[3];
        } ad;
        int dyn_buf[2] = {10, 20};
        const char *const f_aliases[] = {"legacy_id", "old_id"};
        struct cdd_field_info af[4];
        struct cdd_type_info ati;
        size_t alen = 0;
        const void *elem_ptr = NULL;
        void *mut_elem_ptr = NULL;

        memset(&ad, 0, sizeof(ad));
        ad.n_dyn_ints = 2;
        ad.dyn_ints = dyn_buf;
        ad.fix_ints[0] = 100;
        ad.fix_ints[1] = 200;
        ad.fix_ints[2] = 300;

        memset(af, 0, sizeof(af));
        af[0].name = "dyn_ints";
        af[0].wire_name = "dyn_ints";
        af[0].kind = CDD_FIELD_KIND_ARRAY;
        af[0].elem_kind = CDD_FIELD_KIND_INTEGER;
        af[0].offset = offsetof(struct ArrayReflDummy, dyn_ints);
        af[0].array_len_offset = offsetof(struct ArrayReflDummy, n_dyn_ints);
        af[0].elem_size = sizeof(int);
        af[0].is_pointer = 1;
        af[0].aliases = f_aliases;
        af[0].n_aliases = 2;

        af[1].name = "fix_ints";
        af[1].wire_name = "fix_ints";
        af[1].kind = CDD_FIELD_KIND_ARRAY;
        af[1].elem_kind = CDD_FIELD_KIND_INTEGER;
        af[1].offset = offsetof(struct ArrayReflDummy, fix_ints);
        af[1].fixed_array_len = 3;
        af[1].elem_size = sizeof(int);
        af[1].is_pointer = 0;

        af[2].name = "fam_ints";
        af[2].wire_name = "fam_ints";
        af[2].kind = CDD_FIELD_KIND_ARRAY;
        af[2].is_flexible_array = 1;
        af[2].array_len_offset = 0;

        af[3].name = "not_arr";
        af[3].wire_name = "not_arr";
        af[3].kind = CDD_FIELD_KIND_INTEGER;

        ati.name = "ArrayReflDummy";
        ati.size = sizeof(ad);
        ati.fields = af;
        ati.n_fields = 4;

        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_find_field(&ati, "old_id", &found_f));
        ASSERT_STR_EQ("dyn_ints", found_f->name);
        ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                  cdd_reflect_find_field(&ati, "nonexistent_alias", &found_f));

        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_array_len(NULL, &af[0], &alen));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_array_len(&ad, NULL, &alen));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_array_len(&ad, &af[0], NULL));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_array_len(&ad, &af[3], &alen));
        ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                  cdd_reflect_array_len(&ad, &af[2], &alen));

        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_array_len(&ad, &af[0], &alen));
        ASSERT_EQ(2u, alen);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_array_len(&ad, &af[1], &alen));
        ASSERT_EQ(3u, alen);

        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_array_at(NULL, &af[0], 0, &elem_ptr));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_array_at(&ad, NULL, 0, &elem_ptr));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_array_at(&ad, &af[0], 0, NULL));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_array_at(&ad, &af[3], 0, &elem_ptr));
        ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                  cdd_reflect_array_at(&ad, &af[0], 5, &elem_ptr));

        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_array_at(&ad, &af[0], 1, &elem_ptr));
        ASSERT_EQ(20, *(const int *)elem_ptr);
        ad.dyn_ints = NULL;
        ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                  cdd_reflect_array_at(&ad, &af[0], 0, &elem_ptr));
        ad.dyn_ints = dyn_buf;

        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_array_at(&ad, &af[1], 2, &elem_ptr));
        ASSERT_EQ(300, *(const int *)elem_ptr);

        /* Test cdd_reflect_array_at_mut & cdd_reflect_array_resize */
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_array_at_mut(NULL, &af[0], 0, &mut_elem_ptr));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_array_at_mut(&ad, NULL, 0, &mut_elem_ptr));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_array_at_mut(&ad, &af[0], 0, NULL));
        ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                  cdd_reflect_array_at_mut(&ad, &af[0], 9, &mut_elem_ptr));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_array_at_mut(&ad, &af[0], 1, &mut_elem_ptr));
        *(int *)mut_elem_ptr = 25;
        ASSERT_EQ(25, dyn_buf[1]);
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_array_at_mut(&ad, &af[1], 0, &mut_elem_ptr));
        *(int *)mut_elem_ptr = 111;
        ASSERT_EQ(111, ad.fix_ints[0]);

        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_array_resize(NULL, &af[0], 4));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_array_resize(&ad, NULL, 4));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_array_resize(&ad, &af[1], 4));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_array_resize(&ad, &af[2], 4));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_array_resize(&ad, &af[3], 4));

        ad.dyn_ints = NULL;
        ad.n_dyn_ints = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_array_resize(&ad, &af[0], 0));
#ifdef CDD_BUILD_TESTS
        g_cdd_alloc_fail = 1;
        ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_reflect_array_resize(&ad, &af[0], 3));
        g_cdd_alloc_fail = 0;
#endif
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_array_resize(&ad, &af[0], 3));
        ASSERT_EQ(3u, ad.n_dyn_ints);
        ASSERT_EQ(0, ad.dyn_ints[2]);
        ad.dyn_ints[0] = 42;
        /* Shrink array */
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_array_resize(&ad, &af[0], 1));
        ASSERT_EQ(1u, ad.n_dyn_ints);
        ASSERT_EQ(42, ad.dyn_ints[0]);
        /* Resize to 0 frees array */
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_array_resize(&ad, &af[0], 0));
        ASSERT_EQ(0u, ad.n_dyn_ints);
        ASSERT_EQ(NULL, ad.dyn_ints);

        /* Test cdd_reflect_validate_field and cdd_reflect_validate */
        {
          struct ValDummy {
            int age;
            double score;
            const char *title;
            size_t n_items;
            int *items;
          } vd;
          int item_buf[2] = {1, 2};
          struct cdd_field_info vf[4];
          struct cdd_type_info vti;

          memset(&vd, 0, sizeof(vd));
          memset(vf, 0, sizeof(vf));
          vd.age = 25;
          vd.score = 0.5;
          vd.title = "abc";
          vd.n_items = 2;
          vd.items = item_buf;

          vf[0].name = "age";
          vf[0].kind = CDD_FIELD_KIND_INTEGER;
          vf[0].offset = offsetof(struct ValDummy, age);
          vf[0].size = sizeof(int);
          vf[0].has_min = 1;
          vf[0].min_val = 0.0;
          vf[0].has_max = 1;
          vf[0].max_val = 100.0;

          vf[1].name = "score";
          vf[1].kind = CDD_FIELD_KIND_NUMBER;
          vf[1].offset = offsetof(struct ValDummy, score);
          vf[1].size = sizeof(double);
          vf[1].has_min = 1;
          vf[1].min_val = 0.0;
          vf[1].has_max = 1;
          vf[1].max_val = 1.0;

          vf[2].name = "title";
          vf[2].kind = CDD_FIELD_KIND_STRING;
          vf[2].offset = offsetof(struct ValDummy, title);
          vf[2].size = sizeof(char *);
          vf[2].is_pointer = 1;
          vf[2].required = 1;
          vf[2].has_min_len = 1;
          vf[2].min_len = 2;
          vf[2].has_max_len = 1;
          vf[2].max_len = 5;

          vf[3].name = "items";
          vf[3].kind = CDD_FIELD_KIND_ARRAY;
          vf[3].offset = offsetof(struct ValDummy, items);
          vf[3].array_len_offset = offsetof(struct ValDummy, n_items);
          vf[3].elem_size = sizeof(int);
          vf[3].is_pointer = 1;
          vf[3].has_min_len = 1;
          vf[3].min_len = 1;
          vf[3].has_max_len = 1;
          vf[3].max_len = 3;

          vti.name = "ValDummy";
          vti.size = sizeof(vd);
          vti.is_union = 0;
          vti.fields = vf;
          vti.n_fields = 4;
          vti.tag_offset = 0;

          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_validate_field(NULL, &vf[0]));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_validate_field(&vd, NULL));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_validate(NULL, &vti));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_validate(&vd, NULL));

          ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_validate(&vd, &vti));

          vd.age = -5;
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_validate(&vd, &vti));
          vd.age = 150;
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_validate(&vd, &vti));
          vd.age = 25;

          vd.score = -0.1;
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_validate(&vd, &vti));
          vd.score = 1.5;
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_validate(&vd, &vti));
          vd.score = 0.5;

          vd.title = NULL;
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_validate(&vd, &vti));
          vf[2].is_nullable = 1;
          ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_validate_field(&vd, &vf[2]));
          vf[2].is_nullable = 0;
          vd.title = "a";
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_validate_field(&vd, &vf[2]));
          vd.title = "abcdef";
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_validate_field(&vd, &vf[2]));
          vd.title = "abc";

          vd.n_items = 0;
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_validate_field(&vd, &vf[3]));
          vd.n_items = 4;
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_validate_field(&vd, &vf[3]));
          vd.n_items = 2;

          /* Error percolation branches in cdd_reflect_validate_field */
          vf[0].bit_width = 2;
          vf[0].get_bitfield = NULL;
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_validate_field(&vd, &vf[0]));
          vf[0].bit_width = 0;

          vf[1].is_pointer = 1;
          vf[1].is_nullable = 1;
          {
            struct {
              double *p;
            } np = {NULL};
            struct cdd_field_info nvf;
            memset(&nvf, 0, sizeof(nvf));
            nvf.kind = CDD_FIELD_KIND_NUMBER;
            nvf.is_pointer = 1;
            nvf.bit_width = 1;
            nvf.has_min = 1;
            ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                      cdd_reflect_validate_field(&np, &nvf));
          }
          vf[1].is_pointer = 0;
          vf[1].is_nullable = 0;

          vf[2].is_nullable = 1;
          vd.title = NULL;
          vf[2].bit_width = 1;
          ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_validate_field(&vd, &vf[2]));
          vf[2].bit_width = 0;
          vf[2].is_nullable = 0;
          vd.title = "abc";

          vf[3].is_flexible_array = 1;
          vf[3].array_len_offset = 0;
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_validate_field(&vd, &vf[3]));
          vf[3].is_flexible_array = 0;
          vf[3].array_len_offset = offsetof(struct ValDummy, n_items);
        }
      }

      /* Test bit-field constant expression evaluation in
       * write_struct_reflect_func */
      {
        struct StructFields sf_bw;
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_bw));
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_bw, "b_add", "integer",
                                                   NULL, NULL, "(1 + 2 * 3)"));
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_bw, "b_sub", "integer",
                                                   NULL, NULL, "(8 - 3)"));
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_bw, "b_mul", "integer",
                                                   NULL, NULL, "(2 * 2)"));
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_bw, "b_div", "integer",
                                                   NULL, NULL, "(8 / 2)"));
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_bw, "b_mod", "integer",
                                                   NULL, NULL, "(9 % 5)"));
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_bw, "b_shl", "integer",
                                                   NULL, NULL, "(1 << 2)"));
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_bw, "b_shr", "integer",
                                                   NULL, NULL, "(8 >> 1)"));
        ASSERT_EQ(CDD_C_SUCCESS,
                  struct_fields_add(&sf_bw, "b_bit", "integer", NULL, NULL,
                                    "((0x3 & 0x1) | (0x2 ^ 0x0))"));
        ASSERT_EQ(CDD_C_SUCCESS,
                  struct_fields_add(&sf_bw, "b_un", "integer", NULL, NULL,
                                    "(+4 + -1 + (~0 & 1))"));
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_bw, "b_hex", "integer",
                                                   NULL, NULL, "0X0aAuL"));
        ASSERT_EQ(CDD_C_SUCCESS,
                  struct_fields_add(&sf_bw, "b_dec_sfx", "integer", NULL, NULL,
                                    "4uL"));
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_bw, "b_sym", "integer",
                                                   NULL, NULL, "MAX_BITS"));
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_bw, "b_zero", "integer",
                                                   NULL, NULL, "0"));
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_bw, "s_fix", "string",
                                                   NULL, NULL, NULL));
        sf_bw.fields[13].fixed_array_len = 16;
        ASSERT_EQ(CDD_C_SUCCESS,
                  write_struct_reflect_func(tmp, "BwExprRefl", &sf_bw, NULL));
        struct_fields_free(&sf_bw);
      }

      /* Test enum reflection lookup and runtime registries */
      {
        struct cdd_enum_variant_info ev[3];
        struct cdd_enum_info ei;
        const struct cdd_enum_variant_info *found_ev = NULL;
        const struct cdd_type_info *found_ti = NULL;
        const struct cdd_enum_info *found_ei = NULL;
        struct cdd_type_info extra_ti[10];
        struct cdd_enum_info extra_ei[10];
        char extra_names[10][16];
        int r_idx;

        memset(ev, 0, sizeof(ev));
        ev[0].name = "ACTIVE";
        ev[0].wire_name = "active";
        ev[0].value = 1;
        ev[1].name = "INACTIVE";
        ev[1].wire_name = "inactive";
        ev[1].value = 2;

        ei.name = "StatusEnum";
        ei.variants = ev;
        ei.n_variants = 2;
        ei.other_idx = -1;

        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_find_enum_variant(NULL, "ACTIVE", &found_ev));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_find_enum_variant(&ei, NULL, &found_ev));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_find_enum_variant(&ei, "ACTIVE", NULL));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_find_enum_variant(&ei, "ACTIVE", &found_ev));
        ASSERT_EQ(1, found_ev->value);
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_find_enum_variant(&ei, "inactive", &found_ev));
        ASSERT_EQ(2, found_ev->value);
        ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                  cdd_reflect_find_enum_variant(&ei, "missing", &found_ev));

        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_find_enum_by_value(NULL, 1, &found_ev));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_find_enum_by_value(&ei, 1, NULL));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_find_enum_by_value(&ei, 2, &found_ev));
        ASSERT_STR_EQ("INACTIVE", found_ev->name);
        ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                  cdd_reflect_find_enum_by_value(&ei, 99, &found_ev));

        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_clear_registry());
        ASSERT_EQ(
            CDD_C_SUCCESS,
            cdd_reflect_clear_registry()); /* idempotent when already empty */

        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_register_type(NULL));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_register_enum(NULL));
        {
          struct cdd_type_info bad_ti = {0};
          struct cdd_enum_info bad_ei = {0};
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_register_type(&bad_ti));
          bad_ti.name = "";
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_register_type(&bad_ti));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_register_enum(&bad_ei));
          bad_ei.name = "";
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_register_enum(&bad_ei));
        }
#ifdef CDD_BUILD_TESTS
        g_cdd_alloc_fail = 1;
        ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_reflect_register_type(&dummy_ti));
        g_cdd_alloc_fail = 1;
        ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_reflect_register_enum(&ei));
        g_cdd_alloc_fail = 0;
#endif
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_register_type(&dummy_ti));
        /* Duplicate registration overwrites in-place */
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_register_type(&dummy_ti));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_register_enum(&ei));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_register_enum(&ei));

        for (r_idx = 0; r_idx < 9; ++r_idx) {
          CDD_SNPRINTF(extra_names[r_idx], sizeof(extra_names[r_idx]),
                       "ExtraT%d", r_idx);
          memset(&extra_ti[r_idx], 0, sizeof(extra_ti[r_idx]));
          extra_ti[r_idx].name = extra_names[r_idx];
          memset(&extra_ei[r_idx], 0, sizeof(extra_ei[r_idx]));
          extra_ei[r_idx].name = extra_names[r_idx];
          ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_register_type(&extra_ti[r_idx]));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_register_enum(&extra_ei[r_idx]));
        }

        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_type_by_name(NULL, &found_ti));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_type_by_name("DummyReflStruct", NULL));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_type_by_name("DummyReflStruct", &found_ti));
        ASSERT_EQ(&dummy_ti, found_ti);
        ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                  cdd_reflect_type_by_name("MissingType", &found_ti));

        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_enum_by_name(NULL, &found_ei));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_enum_by_name("StatusEnum", NULL));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_enum_by_name("StatusEnum", &found_ei));
        ASSERT_EQ(&ei, found_ei);
        ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                  cdd_reflect_enum_by_name("MissingEnum", &found_ei));

        /* Test enum variant alias lookup */
        {
          const char *const v_aliases[] = {"legacy_active", "enabled"};
          ev[0].aliases = v_aliases;
          ev[0].n_aliases = 2;
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_find_enum_variant(&ei, "enabled", &found_ev));
          ASSERT_EQ(1, found_ev->value);
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, cdd_reflect_find_enum_variant(
                                               &ei, "not_an_alias", &found_ev));
        }

        /* Test function reflection registry and parameter lookup */
        {
          struct cdd_param_info fparams[2];
          struct cdd_func_info fi;
          struct cdd_func_info extra_fi[10];
          const struct cdd_func_info *found_fi = NULL;
          const struct cdd_param_info *found_param = NULL;
          size_t reg_cnt = 0;

          memset(fparams, 0, sizeof(fparams));
          fparams[0].name = "user_id";
          fparams[0].c_type = "int";
          fparams[0].kind = CDD_FIELD_KIND_INTEGER;
          fparams[1].name = "out_name";
          fparams[1].c_type = "char **";
          fparams[1].kind = CDD_FIELD_KIND_STRING;
          fparams[1].is_pointer = 1;
          fparams[1].is_out = 1;

          memset(&fi, 0, sizeof(fi));
          fi.name = "get_user_name";
          fi.return_c_type = "cdd_c_error_t";
          fi.params = fparams;
          fi.n_params = 2;

          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_register_func(NULL));
          {
            struct cdd_func_info bad_fi = {0};
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_register_func(&bad_fi));
            bad_fi.name = "";
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_register_func(&bad_fi));
          }
#ifdef CDD_BUILD_TESTS
          g_cdd_alloc_fail = 1;
          ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_reflect_register_func(&fi));
          g_cdd_alloc_fail = 0;
#endif
          ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_register_func(&fi));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_register_func(&fi));
          for (r_idx = 0; r_idx < 9; ++r_idx) {
            memset(&extra_fi[r_idx], 0, sizeof(extra_fi[r_idx]));
            extra_fi[r_idx].name = extra_names[r_idx];
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_register_func(&extra_fi[r_idx]));
          }
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_func_by_name(NULL, &found_fi));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_func_by_name("get_user_name", NULL));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_func_by_name("get_user_name", &found_fi));
          ASSERT_EQ(&fi, found_fi);
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_func_by_name("missing_fn", &found_fi));

          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_find_param(NULL, "user_id", &found_param));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_find_param(&fi, NULL, &found_param));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_find_param(&fi, "user_id", NULL));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_find_param(&fi, "out_name", &found_param));
          ASSERT_EQ(1, found_param->is_out);
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_find_param(&fi, "no_such", &found_param));

          /* Test registry count and index-based enumeration APIs */
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_registered_types_count(NULL));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_registered_types_count(&reg_cnt));
          ASSERT_EQ(10u, reg_cnt);
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_registered_type_at(0, NULL));
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_registered_type_at(99, &found_ti));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_registered_type_at(0, &found_ti));
          ASSERT_EQ(&dummy_ti, found_ti);

          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_registered_enums_count(NULL));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_registered_enums_count(&reg_cnt));
          ASSERT_EQ(10u, reg_cnt);
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_registered_enum_at(0, NULL));
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_registered_enum_at(99, &found_ei));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_registered_enum_at(0, &found_ei));
          ASSERT_EQ(&ei, found_ei);

          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_registered_funcs_count(NULL));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_registered_funcs_count(&reg_cnt));
          ASSERT_EQ(10u, reg_cnt);
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_registered_func_at(0, NULL));
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_registered_func_at(99, &found_fi));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_registered_func_at(0, &found_fi));
          ASSERT_EQ(&fi, found_fi);
        }

        /* Test cdd_reflect_visit_deep with nested objects and object arrays
         */
        {
          struct DeepChild {
            int x;
          } child_inst, arr_children[2];
          struct DeepParent {
            struct DeepChild *child_ptr;
            size_t n_children;
            struct DeepChild *children;
          } parent_inst;
          struct cdd_field_info child_fields[1];
          struct cdd_type_info child_ti;
          struct cdd_field_info parent_fields[2];
          struct cdd_type_info parent_ti;
          int deep_cnt = 0;

          memset(&child_inst, 0, sizeof(child_inst));
          memset(arr_children, 0, sizeof(arr_children));
          memset(&parent_inst, 0, sizeof(parent_inst));
          parent_inst.child_ptr = &child_inst;
          parent_inst.n_children = 2;
          parent_inst.children = arr_children;

          memset(child_fields, 0, sizeof(child_fields));
          child_fields[0].name = "x";
          child_fields[0].kind = CDD_FIELD_KIND_INTEGER;
          child_fields[0].offset = offsetof(struct DeepChild, x);
          child_fields[0].size = sizeof(int);

          child_ti.name = "DeepChild";
          child_ti.size = sizeof(struct DeepChild);
          child_ti.is_union = 0;
          child_ti.fields = child_fields;
          child_ti.n_fields = 1;
          child_ti.tag_offset = 0;

          memset(parent_fields, 0, sizeof(parent_fields));
          parent_fields[0].name = "child_ptr";
          parent_fields[0].ref_name = "DeepChild";
          parent_fields[0].kind = CDD_FIELD_KIND_OBJECT;
          parent_fields[0].offset = offsetof(struct DeepParent, child_ptr);
          parent_fields[0].size = sizeof(struct DeepChild *);
          parent_fields[0].is_pointer = 1;

          parent_fields[1].name = "children";
          parent_fields[1].ref_name = "DeepChild";
          parent_fields[1].kind = CDD_FIELD_KIND_ARRAY;
          parent_fields[1].elem_kind = CDD_FIELD_KIND_OBJECT;
          parent_fields[1].offset = offsetof(struct DeepParent, children);
          parent_fields[1].array_len_offset =
              offsetof(struct DeepParent, n_children);
          parent_fields[1].elem_size = sizeof(struct DeepChild);
          parent_fields[1].is_pointer = 1;

          parent_ti.name = "DeepParent";
          parent_ti.size = sizeof(struct DeepParent);
          parent_ti.is_union = 0;
          parent_ti.fields = parent_fields;
          parent_ti.n_fields = 2;
          parent_ti.tag_offset = 0;

          ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_register_type(&child_ti));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_register_type(&parent_ti));

          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_visit_deep(NULL, &parent_ti,
                                           test_reflect_visitor_cb, &deep_cnt));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_visit_deep(&parent_inst, NULL,
                                           test_reflect_visitor_cb, &deep_cnt));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_visit_deep(&parent_inst, &parent_ti, NULL,
                                           &deep_cnt));

          /* Visits: child_ptr (1) + child.x (1) + children (1) + 2*child.x
           * (2) = 5 */
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_visit_deep(&parent_inst, &parent_ti,
                                           test_reflect_visitor_cb, &deep_cnt));
          ASSERT_EQ(5, deep_cnt);

          /* Error return from visitor at top-level, nested object, and nested
           * array element */
          deep_cnt = -1;
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_visit_deep(&parent_inst, &parent_ti,
                                           test_reflect_visitor_cb, &deep_cnt));
          child_fields[0].name = "fail_me";
          deep_cnt = 0;
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_visit_deep(&parent_inst, &parent_ti,
                                           test_reflect_visitor_cb, &deep_cnt));
          parent_inst.child_ptr = NULL;
          deep_cnt = 0;
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_visit_deep(&parent_inst, &parent_ti,
                                           test_reflect_visitor_cb, &deep_cnt));
          child_fields[0].name = "x";

          /* Self-referential cycle stops immediately via visited-set cycle
           * detection */
          child_fields[0].kind = CDD_FIELD_KIND_OBJECT;
          child_fields[0].ref_name = "DeepChild";
          child_fields[0].is_pointer = 0;
          deep_cnt = 0;
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_visit_deep(&child_inst, &child_ti,
                                           test_reflect_visitor_cb, &deep_cnt));
          ASSERT_EQ(1, deep_cnt);
        }

        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_clear_registry());
      }

      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                write_struct_reflect_func(NULL, "S", &sf_empty, NULL));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                write_struct_reflect_func(tmp, NULL, &sf_empty, NULL));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                write_struct_reflect_func(tmp, "S", NULL, NULL));

      ASSERT_EQ(CDD_C_SUCCESS,
                write_struct_reflect_func(tmp, "EmptyRefl", &sf_empty, NULL));
      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_rf));
      ASSERT_EQ(CDD_C_SUCCESS,
                struct_fields_add(&sf_rf, "i", "integer", NULL, NULL, NULL));
      sf_rf.fields[0].is_nullable_ptr = 1;
      ASSERT_EQ(CDD_C_SUCCESS,
                struct_field_add_alias(&sf_rf.fields[0], "i_alias1"));
      ASSERT_EQ(CDD_C_SUCCESS,
                struct_field_add_alias(&sf_rf.fields[0], "i_alias2"));
      ASSERT_EQ(CDD_C_SUCCESS,
                struct_fields_add(&sf_rf, "n", "number", NULL, NULL, NULL));
      ASSERT_EQ(CDD_C_SUCCESS,
                struct_fields_add(&sf_rf, "b", "boolean", NULL, NULL, NULL));
      ASSERT_EQ(CDD_C_SUCCESS,
                struct_fields_add(&sf_rf, "s", "string", NULL, NULL, NULL));
      ASSERT_EQ(CDD_C_SUCCESS,
                struct_fields_add(&sf_rf, "e", "enum", "MyEnum", NULL, NULL));
      ASSERT_EQ(CDD_C_SUCCESS,
                struct_fields_add(&sf_rf, "o", "object", "MyObj", NULL, NULL));
      ASSERT_EQ(CDD_C_SUCCESS,
                struct_fields_add(&sf_rf, "a", "array", "string", NULL, NULL));
      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_rf, "fix_a", "array",
                                                 "integer", NULL, NULL));
      sf_rf.fields[7].fixed_array_len = 4;
      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_rf, "fam_a", "array",
                                                 "integer", NULL, NULL));
      sf_rf.fields[8].is_flexible_array = 1;
      ASSERT_EQ(CDD_C_SUCCESS,
                struct_fields_add(&sf_rf, "bf", "integer", NULL, NULL, "4"));
      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_rf, "bf_anon", "integer",
                                                 NULL, NULL, "2"));
      CDD_STRCPY(sf_rf.fields[10].name, sizeof(sf_rf.fields[10].name), "");
      ASSERT_EQ(CDD_C_SUCCESS,
                struct_fields_add(&sf_rf, "u", "custom", NULL, NULL, NULL));
      CDD_STRCPY(sf_rf.fields[11].name, sizeof(sf_rf.fields[11].name), "");
      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_rf, "a_num", "array",
                                                 "number", NULL, NULL));
      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_rf, "a_bool", "array",
                                                 "boolean", NULL, NULL));
      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_rf, "a_enum", "array",
                                                 "MyEnum", NULL, NULL));
      sf_rf.fields[14].is_enum_array = 1;
      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_rf, "a_obj", "array",
                                                 "MyObj", NULL, NULL));
      ASSERT_EQ(CDD_C_SUCCESS,
                write_struct_reflect_func(tmp, "FullRefl", &sf_rf, &cfg));
      cfg.guard_macro = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                write_struct_reflect_func(tmp, "NoGuardRefl", &sf_rf, &cfg));
      /* Also test tagged union reflection */
      sf_rf.is_union = 1;
      ASSERT_EQ(CDD_C_SUCCESS,
                write_struct_reflect_func(tmp, "UnionRefl", &sf_rf, &cfg));
      for (fail_step = 0; fail_step < 45; ++fail_step) {
        g_fail_io_after = fail_step;
        g_io_calls = 0;
        rc_step = write_struct_reflect_func(tmp, "UnionRefl", &sf_rf, &cfg);
        if (rc_step == CDD_C_SUCCESS)
          break;
        ASSERT_EQ(CDD_C_ERROR_IO, rc_step);
      }
      sf_rf.is_union = 0;
      cfg.guard_macro = "TEST_REFL_GUARD";
      for (fail_step = 0; fail_step < 45; ++fail_step) {
        g_fail_io_after = fail_step;
        g_io_calls = 0;
        rc_step = write_struct_reflect_func(tmp, "FullRefl", &sf_rf, &cfg);
        if (rc_step == CDD_C_SUCCESS)
          break;
        ASSERT_EQ(CDD_C_ERROR_IO, rc_step);
      }
      g_fail_io_after = -1;
      struct_fields_free(&sf_rf);

      /* Test write_func_reflect_func */
      {
        struct FuncSignature fsig;
        struct FuncParamReflection fps[8];
        char empty_name[] = "";
        char fn_name[] = "compute_metrics";
        char fn_doc[] = "Computes metrics";
        memset(&fsig, 0, sizeof(fsig));
        memset(fps, 0, sizeof(fps));
        CDD_STRCPY(fps[0].type, sizeof(fps[0].type), "int");
        CDD_STRCPY(fps[0].name, sizeof(fps[0].name), "count");
        fps[0].doc = "Item count";
        fps[0].has_required_flag = 1;
        fps[0].required = 1;
        fps[0].has_nullable_flag = 1;
        fps[0].is_nullable = 0;
        CDD_STRCPY(fps[1].type, sizeof(fps[1].type), "const char *");
        CDD_STRCPY(fps[1].name, sizeof(fps[1].name), "label");
        CDD_STRCPY(fps[2].type, sizeof(fps[2].type), "double *");
        CDD_STRCPY(fps[2].name, sizeof(fps[2].name), "out_score");
        CDD_STRCPY(fps[3].type, sizeof(fps[3].type), "_Bool");
        CDD_STRCPY(fps[3].name, sizeof(fps[3].name), "flag");
        CDD_STRCPY(fps[4].type, sizeof(fps[4].type), "enum MyE");
        CDD_STRCPY(fps[4].name, sizeof(fps[4].name), "ev");
        CDD_STRCPY(fps[5].type, sizeof(fps[5].type), "struct MyS *");
        CDD_STRCPY(fps[5].name, sizeof(fps[5].name), "obj");
        CDD_STRCPY(fps[6].type, sizeof(fps[6].type), "custom_t");
        CDD_STRCPY(fps[6].name, sizeof(fps[6].name), "cust");
        CDD_STRCPY(fps[7].type, sizeof(fps[7].type), "char **");
        CDD_STRCPY(fps[7].name, sizeof(fps[7].name), "argv");

        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  write_func_reflect_func(NULL, &fsig, NULL));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  write_func_reflect_func(tmp, NULL, NULL));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  write_func_reflect_func(tmp, &fsig, NULL));
        fsig.name = empty_name;
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  write_func_reflect_func(tmp, &fsig, NULL));
        fsig.name = fn_name;
        CDD_STRCPY(fsig.return_type, sizeof(fsig.return_type), "cdd_c_error_t");
        fsig.doc = fn_doc;
        fsig.route_method = "POST";
        fsig.route_path = "/metrics";
        fsig.summary = "Compute endpoint";
        fsig.is_deprecated = 1;
        fsig.params = fps;
        fsig.n_params = 8;

        ASSERT_EQ(CDD_C_SUCCESS, write_func_reflect_func(tmp, &fsig, &cfg));
        cfg.guard_macro = NULL;
        fsig.doc = NULL;
        fsig.route_method = NULL;
        fsig.route_path = NULL;
        fsig.summary = NULL;
        CDD_STRCPY(fsig.return_type, sizeof(fsig.return_type), "void");
        ASSERT_EQ(CDD_C_SUCCESS, write_func_reflect_func(tmp, &fsig, &cfg));
        cfg.guard_macro = "TEST_REFL_GUARD";
        for (fail_step = 0; fail_step < 20; ++fail_step) {
          g_fail_io_after = fail_step;
          g_io_calls = 0;
          rc_step = write_func_reflect_func(tmp, &fsig, &cfg);
          if (rc_step == CDD_C_SUCCESS)
            break;
          ASSERT_EQ(CDD_C_ERROR_IO, rc_step);
        }
        g_fail_io_after = -1;
      }

      /* Test inline fixed-size arrays & bit-field stack temporaries across
       * direct code generators */
      {
        struct StructFields sf_fix;
        struct EnumMembers em_al;
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_fix));
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_fix, "fix_str", "string",
                                                   NULL, "\"hello\"", NULL));
        sf_fix.fields[0].fixed_array_len = 16;
        sf_fix.fields[0].serde_skip_if_kind = CDD_SERDE_SKIP_IF_EMPTY;
        sf_fix.fields[0].cli_short = 's';
        ASSERT_EQ(CDD_C_SUCCESS,
                  struct_fields_add(&sf_fix, "fix_str_ro", "string", NULL,
                                    "ro_val", NULL));
        sf_fix.fields[1].fixed_array_len = 8;
        sf_fix.fields[1].read_only = 1;
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_fix, "fix_ints", "array",
                                                   "integer", NULL, NULL));
        sf_fix.fields[2].fixed_array_len = 3;
        sf_fix.fields[2].serde_skip_if_kind = CDD_SERDE_SKIP_IF_EMPTY;
        sf_fix.fields[2].cli_short = 'i';
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_fix, "fix_nums", "array",
                                                   "number", NULL, NULL));
        sf_fix.fields[3].fixed_array_len = 2;
        sf_fix.fields[3].cli_short = 'n';
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_fix, "fix_strs", "array",
                                                   "string", NULL, NULL));
        sf_fix.fields[4].fixed_array_len = 2;
        sf_fix.fields[4].cli_short = 'a';
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_fix, "fix_objs", "array",
                                                   "SubObj", NULL, NULL));
        sf_fix.fields[5].fixed_array_len = 2;
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_fix, "bf_int", "integer",
                                                   NULL, NULL, "3"));
        CDD_STRCPY(sf_fix.fields[6].serde_default_fn,
                   sizeof(sf_fix.fields[6].serde_default_fn), "def_bf_int");
        sf_fix.fields[6].cli_short = 'b';
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_fix, "bf_ro", "integer",
                                                   NULL, NULL, "2"));
        sf_fix.fields[7].read_only = 1;
        CDD_STRCPY(sf_fix.fields[7].serde_default_fn,
                   sizeof(sf_fix.fields[7].serde_default_fn), "def_bf_ro");
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_fix, "bf_with",
                                                   "integer", NULL, NULL, "4"));
        CDD_STRCPY(sf_fix.fields[8].serde_deserialize_with,
                   sizeof(sf_fix.fields[8].serde_deserialize_with),
                   "de_bf_with");
        CDD_STRCPY(sf_fix.fields[8].cli_value_parser,
                   sizeof(sf_fix.fields[8].cli_value_parser), "parse_bf_with");
        sf_fix.fields[8].cli_short = 'w';
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_fix, "bf_bool",
                                                   "boolean", NULL, NULL, "1"));
        CDD_STRCPY(sf_fix.fields[9].serde_default_fn,
                   sizeof(sf_fix.fields[9].serde_default_fn), "def_bf_bool");
        sf_fix.fields[9].cli_short = 'f';
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_fix, "bf_enum", "enum",
                                                   "MyEnum", "\"A\"", "2"));
        sf_fix.fields[10].cli_short = 'e';
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_fix, "pos_str", "string",
                                                   NULL, NULL, NULL));
        sf_fix.fields[11].fixed_array_len = 12;
        sf_fix.fields[11].cli_positional = 1;
        ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_fix, "pos_arr", "array",
                                                   "string", NULL, NULL));
        sf_fix.fields[12].fixed_array_len = 2;
        sf_fix.fields[12].cli_positional = 1;

        ASSERT_EQ(CDD_C_SUCCESS, write_struct_declaration_h(tmp, "FixBfStruct",
                                                            &sf_fix, NULL));
        for (fail_step = 0; fail_step < 40; ++fail_step) {
          g_fail_io_after = fail_step;
          g_io_calls = 0;
          rc_step =
              write_struct_cleanup_func(tmp, "FixBfStruct", &sf_fix, NULL);
          if (rc_step == CDD_C_SUCCESS)
            break;
          ASSERT_EQ(CDD_C_ERROR_IO, rc_step);
        }
        g_fail_io_after = -1;
        ASSERT_EQ(CDD_C_SUCCESS, write_struct_deepcopy_func(tmp, "FixBfStruct",
                                                            &sf_fix, NULL));
        for (fail_step = 0; fail_step < 60; ++fail_step) {
          g_fail_io_after = fail_step;
          g_io_calls = 0;
          rc_step = write_struct_eq_func(tmp, "FixBfStruct", &sf_fix, NULL);
          if (rc_step == CDD_C_SUCCESS)
            break;
          ASSERT_EQ(CDD_C_ERROR_IO, rc_step);
        }
        g_fail_io_after = -1;
        for (fail_step = 0; fail_step < 60; ++fail_step) {
          g_fail_io_after = fail_step;
          g_io_calls = 0;
          rc_step =
              write_struct_default_func(tmp, "FixBfStruct", &sf_fix, NULL);
          if (rc_step == CDD_C_SUCCESS)
            break;
          ASSERT_EQ(CDD_C_ERROR_IO, rc_step);
        }
        g_fail_io_after = -1;
        for (fail_step = 0; fail_step < 80; ++fail_step) {
          g_fail_io_after = fail_step;
          g_io_calls = 0;
          rc_step = write_struct_debug_func(tmp, "FixBfStruct", &sf_fix, NULL);
          if (rc_step == CDD_C_SUCCESS)
            break;
          ASSERT_EQ(CDD_C_ERROR_IO, rc_step);
        }
        g_fail_io_after = -1;
        ASSERT_EQ(CDD_C_SUCCESS,
                  write_struct_to_json_func(tmp, "FixBfStruct", &sf_fix, NULL));
        ASSERT_EQ(CDD_C_SUCCESS, write_struct_from_jsonObject_func(
                                     tmp, "FixBfStruct", &sf_fix, NULL));
        ASSERT_EQ(CDD_C_SUCCESS, write_struct_cli_all_funcs(tmp, "FixBfStruct",
                                                            &sf_fix, NULL));
        struct_fields_free(&sf_fix);

        ASSERT_EQ(CDD_C_SUCCESS, enum_members_init(&em_al));
        ASSERT_EQ(CDD_C_SUCCESS, enum_members_add(&em_al, "ONE"));
        ASSERT_EQ(CDD_C_SUCCESS, enum_member_add_alias(&em_al, 0, "first"));
        ASSERT_EQ(CDD_C_SUCCESS, enum_member_add_alias(&em_al, 0, "uno"));
        for (fail_step = 0; fail_step < 30; ++fail_step) {
          g_fail_io_after = fail_step;
          g_io_calls = 0;
          rc_step = write_enum_reflect_func(tmp, "AliasEnum", &em_al, NULL);
          if (rc_step == CDD_C_SUCCESS)
            break;
          ASSERT_EQ(CDD_C_ERROR_IO, rc_step);
        }
        g_fail_io_after = -1;
        enum_members_free(&em_al);
      }

      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                write_struct_generic_dispatch_macros(NULL, g_names, 2));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                write_struct_generic_dispatch_macros(tmp, NULL, 2));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                write_struct_generic_dispatch_macros(tmp, g_names, 0));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                write_struct_generic_dispatch_macros(tmp, bad_names, 1));
      ASSERT_EQ(CDD_C_SUCCESS,
                write_struct_generic_dispatch_macros(tmp, g_names, 2));
      for (fail_step = 0; fail_step < 80; ++fail_step) {
        g_fail_io_after = fail_step;
        g_io_calls = 0;
        rc_step = write_struct_generic_dispatch_macros(tmp, g_names, 2);
        if (rc_step == CDD_C_SUCCESS)
          break;
        ASSERT_EQ(CDD_C_ERROR_IO, rc_step);
      }
      g_fail_io_after = -1;
    }
    struct_fields_free(&sf_empty);
    fclose(tmp);
  }
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_STRUCT_REFLECT_H */
