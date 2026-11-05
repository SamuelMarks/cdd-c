/**
 * @file test_codegen_struct_reflect_edges.h
 * @brief Unit tests for reflection edge cases, OOM injection loops, malformed
 * regex/format strings, unregistered references, and deep validation errors.
 *
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_STRUCT_REFLECT_EDGES_H
#define TEST_CODEGEN_STRUCT_REFLECT_EDGES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_codegen_struct_reflect_adv.h"
/* clang-format on */

/**
 * @brief Tests exhaustive reflection edge cases, OOM paths, and error branches.
 *
 * @return TEST result.
 */
TEST test_reflection_edge_cases_and_oom(void) {
  struct cdd_field_info child_fields[4];
  struct cdd_type_info child_ti;
  struct cdd_field_info parent_fields[22];
  struct cdd_type_info parent_ti;
  struct cdd_field_info union_fields[2];
  struct cdd_type_info union_ti;
  struct cdd_enum_variant_info enum_vars[2];
  struct cdd_enum_info enum_ti;
  struct cdd_param_info func_params[2];
  struct cdd_func_info func_ti;
  const char *child_aliases[1] = {"lbl_alias"};

  struct AdvParent p1;
  struct AdvParent *p_def = NULL;
  struct AdvParent *p_copy = NULL;
  struct AdvParent *p_from_json = NULL;
  char *json_out = NULL;
  char *patch_json = NULL;
  int is_eq = 0;
  int64_t v_i64 = 0;
  uint64_t v_u64 = 0;
  double v_dbl = 0.0;
  const char *v_str = NULL;
  const struct cdd_field_info *res_f = NULL;
  const void *res_parent = NULL;
  const void *res_ptr = NULL;

  memset(&p1, 0, sizeof(p1));
  ASSERT_EQ(CDD_C_SUCCESS, setup_adv_reflection_descriptors(
                               child_fields, &child_ti, parent_fields,
                               &parent_ti, union_fields, &union_ti, enum_vars,
                               &enum_ti, func_params, &func_ti, child_aliases));

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_array_resize(&p1, &parent_fields[13], 2));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_struct_default(&parent_ti, (void **)&p_def));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_struct_default(&child_ti, (void **)&p_def->ptr_child));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_array_resize(p_def, &parent_fields[13], 2));
  p_def->int_arr[0] = 11;
  p_def->int_arr[1] = 22;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_array_resize(p_def, &parent_fields[14], 1));
  p_def->dbl_arr[0] = 3.25;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_array_resize(p_def, &parent_fields[17], 1));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_string_by_path(
                               p_def, &parent_ti, "/str_arr/0", "elem0"));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_set_string_by_path(p_def, &parent_ti, "/str~0name~11",
                                           "escaped_ok"));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_array_resize(p_def, &parent_fields[18], 1));
  p_def->val_obj_arr[0].id = 311;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_set_string(&p_def->val_obj_arr[0], &child_fields[2],
                                   "val_child"));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_array_resize(p_def, &parent_fields[19], 1));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_default(
                               &child_ti, (void **)&p_def->ptr_obj_arr[0]));
  p_def->ptr_obj_arr[0]->id = 411;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_struct_deepcopy(p_def, (void **)&p_copy, &parent_ti));
  p_copy->i64 = 999;
  p_copy->ptr_child->id = 555;
  p_def->i64 = 999;
  p_def->ptr_child->id = 555;

  /* Additional edge-case and branch coverage tests */
  {
    /* Bit-field 32-bit unsigned and signed 4-bit bounds */
    struct cdd_field_info bf_u32_f;
    struct cdd_field_info bf_s4_f;
    memset(&bf_u32_f, 0, sizeof(bf_u32_f));
    memset(&bf_s4_f, 0, sizeof(bf_s4_f));
    bf_u32_f.name = "bf_u32";
    bf_u32_f.c_type = "unsigned int";
    bf_u32_f.kind = CDD_FIELD_KIND_INTEGER;
    bf_u32_f.bit_width = 32;
    bf_u32_f.get_bitfield = test_bf_get;
    bf_u32_f.set_bitfield = test_bf_set;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_get_int64(&p1, &bf_u32_f, &v_i64));
    ASSERT_EQ(7, v_i64);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_get_uint64(&p1, &bf_u32_f, &v_u64));
    ASSERT_EQ(7U, v_u64);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_reflect_set_int64(&p1, &bf_u32_f, (int64_t)0xFFFFFFFFUL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_set_int64(&p1, &bf_u32_f, -1));

    bf_u32_f.c_type = "int";
    ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_int64(&p1, &bf_u32_f, 100));
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        cdd_reflect_set_int64(&p1, &bf_u32_f, (int64_t)INT_MAX + (int64_t)10));

    bf_s4_f.name = "bf_s4";
    bf_s4_f.c_type = "int";
    bf_s4_f.kind = CDD_FIELD_KIND_INTEGER;
    bf_s4_f.bit_width = 4;
    bf_s4_f.get_bitfield = test_bf_get;
    bf_s4_f.set_bitfield = test_bf_set;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_int64(&p1, &bf_s4_f, 7));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_set_int64(&p1, &bf_s4_f, 8));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_set_int64(&p1, &bf_s4_f, -9));
    bf_s4_f.c_type = "unsigned int";
    ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_int64(&p1, &bf_s4_f, 15));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_set_int64(&p1, &bf_s4_f, 16));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_set_int64(&p1, &bf_s4_f, -1));
    bf_s4_f.get_bitfield = NULL;
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_reflect_get_uint64(&p1, &bf_s4_f, &v_u64));
  }

  /* Regex character classes, escapes, quantifiers, and error branches */
  {
    parent_fields[10].pattern =
        "^[]\\d\\w\\s\\x][^0-9]\\D\\W\\S\\+a{2}b{1,}c*$";
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_reflect_set_string(&p1, &parent_fields[10], "]aa a+aabbb"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_reflect_validate_field(&p1, &parent_fields[10]));

    /* Unclosed bracket class and malformed `{` quantifiers */
    parent_fields[10].pattern = "[abc";
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_validate_field(&p1, &parent_fields[10]));
    parent_fields[10].pattern = "a{x}";
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_validate_field(&p1, &parent_fields[10]));
    parent_fields[10].pattern = "a{1,x}";
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_validate_field(&p1, &parent_fields[10]));
    parent_fields[10].pattern = "a{1,2";
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_validate_field(&p1, &parent_fields[10]));
    parent_fields[10].pattern = "unanchored_sub";
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_reflect_set_string(&p1, &parent_fields[10],
                                     "prefix_unanchored_sub_suffix"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_reflect_validate_field(&p1, &parent_fields[10]));
    parent_fields[10].pattern = NULL;
  }

  /* Object array unique_items validation */
  {
    parent_fields[18].unique_items = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_reflect_array_resize(p_def, &parent_fields[18], 2));
    p_def->val_obj_arr[0].id = 500;
    p_def->val_obj_arr[1].id = 500;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_string(&p_def->val_obj_arr[0],
                                                    &child_fields[2], "same"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_string(&p_def->val_obj_arr[1],
                                                    &child_fields[2], "same"));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_validate_field(p_def, &parent_fields[18]));
    p_def->val_obj_arr[1].id = 501;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_reflect_validate_field(p_def, &parent_fields[18]));
    parent_fields[18].unique_items = 0;
  }

  /* Bool, enum, and opt_int JSON round-trip & flatten object serde */
  {
    int opt_v = 777;
    char *flat_json = NULL;
    struct AdvParent *p_flat = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_reflect_array_resize(p_def, &parent_fields[15], 2));
    p_def->bool_arr[0] = 1;
    p_def->bool_arr[1] = 0;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_reflect_array_resize(p_def, &parent_fields[16], 2));
    p_def->enum_arr[0] = 10;
    p_def->enum_arr[1] = 20;
    p_def->opt_int = (int *)C_CDD_MALLOC(sizeof(int));
    *p_def->opt_int = opt_v;
    parent_fields[11].flatten = 1;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_reflect_to_json(p_def, &parent_ti, &flat_json));
    ASSERT(flat_json != NULL);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_reflect_from_json(flat_json, &parent_ti, (void **)&p_flat));
    ASSERT(p_flat != NULL);
    ASSERT(p_flat->opt_int != NULL);
    ASSERT_EQ(777, *p_flat->opt_int);
    ASSERT_EQ(2U, p_flat->n_bool_arr);
    ASSERT_EQ(1, p_flat->bool_arr[0]);
    ASSERT_EQ(2U, p_flat->n_enum_arr);
    ASSERT_EQ(20, p_flat->enum_arr[1]);
    C_CDD_FREE(flat_json);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_cleanup(p_flat, &parent_ti));
    C_CDD_FREE(p_flat);
    parent_fields[11].flatten = 0;
  }

  /* Path accessors through array of objects and error paths */
  {
    ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_int64_by_path(
                                 p_def, &parent_ti, "val_obj_arr[0].id", 808));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_reflect_get_int64_by_path(p_def, &parent_ti,
                                            "/val_obj_arr/0/id", &v_i64));
    ASSERT_EQ(808, v_i64);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_int64_by_path(
                                 p_def, &parent_ti, "ptr_obj_arr[0].id", 909));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_reflect_get_int64_by_path(p_def, &parent_ti,
                                            "/ptr_obj_arr/0/id", &v_i64));
    ASSERT_EQ(909, v_i64);

    /* Path error cases */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_get_int64_by_path(p_def, &parent_ti, "/i64", NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_get_double_by_path(p_def, &parent_ti, "/num", NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_get_string_by_path(p_def, &parent_ti, "/str~0name~11",
                                             NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_resolve_path(p_def, &parent_ti, "int_arr[abc]",
                                       &res_f, &res_parent, &res_ptr));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_resolve_path(p_def, &parent_ti, "int_arr[]", &res_f,
                                       &res_parent, &res_ptr));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_reflect_resolve_path(p_def, &parent_ti, "/i64/sub", &res_f,
                                       &res_parent, &res_ptr));
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        cdd_reflect_get_int64_by_path(p_def, &parent_ti, "/dbl_arr/0", &v_i64));
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        cdd_reflect_set_int64_by_path(p_def, &parent_ti, "/dbl_arr/0", 1));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_get_double_by_path(p_def, &parent_ti, "/int_arr/0",
                                             &v_dbl));
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        cdd_reflect_set_double_by_path(p_def, &parent_ti, "/int_arr/0", 1.0));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_get_string_by_path(p_def, &parent_ti, "/int_arr/0",
                                             &v_str));
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        cdd_reflect_set_string_by_path(p_def, &parent_ti, "/int_arr/0", "bad"));
  }

  /* Null/invalid argument checks on new APIs */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_struct_default(NULL, (void **)&p_def));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_struct_cleanup(&p1, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_cleanup(NULL, &parent_ti));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_struct_deepcopy(NULL, (void **)&p_copy, &parent_ti));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_struct_eq(&p1, &p1, NULL, &is_eq));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_struct_eq(&p1, NULL, &parent_ti, &is_eq));
  ASSERT_EQ(0, is_eq);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_to_json(NULL, &parent_ti, &json_out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_from_json(NULL, &parent_ti, (void **)&p_from_json));
  ASSERT_EQ(
      CDD_C_ERROR_INVALID_ARGUMENT,
      cdd_reflect_from_json("not_json", &parent_ti, (void **)&p_from_json));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_merge_patch(NULL, p_def, &parent_ti, &patch_json));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_apply_merge_patch(NULL, &parent_ti, "{}"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_apply_merge_patch(p_def, &parent_ti, "invalid"));

  /* Bitwidth char & binary literal evaluation via write_struct_reflect_func */
  {
    struct StructFields sf_bw;
    FILE *tmp_bw = tmpfile();
    ASSERT(tmp_bw != NULL);
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_bw));
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_bw, "b_bin", "integer", NULL,
                                               NULL, "0b101u"));
    ASSERT_EQ(CDD_C_SUCCESS,
              struct_fields_add(&sf_bw, "b_chr", "integer", NULL, NULL, "'a'"));
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_bw, "arr_lim", "array",
                                               "integer", NULL, NULL));
    sf_bw.fields[2].has_min_items = 1;
    sf_bw.fields[2].min_items = 1;
    sf_bw.fields[2].has_max_items = 1;
    sf_bw.fields[2].max_items = 5;
    sf_bw.fields[2].unique_items = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              write_struct_reflect_func(tmp_bw, "BwStruct", &sf_bw, NULL));
    struct_fields_free(&sf_bw);
    fclose(tmp_bw);
  }

  /* Exhaustive error/branch/OOM tests on reflection runtime engine */
  {
    /* Oversized path segment */
    char long_path[200];
    memset(long_path, 'a', sizeof(long_path) - 1);
    long_path[0] = '/';
    long_path[sizeof(long_path) - 1] = '\0';
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_resolve_path(p_def, &parent_ti, long_path, &res_f,
                                       &res_parent, &res_ptr));
    long_path[0] = '[';
    long_path[sizeof(long_path) - 2] = ']';
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_resolve_path(p_def, &parent_ti, long_path, &res_f,
                                       &res_parent, &res_ptr));

    /* Shrink ptr_obj_arr and val_obj_arr via cdd_reflect_array_resize */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_reflect_array_resize(p_def, &parent_fields[18], 0));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_reflect_array_resize(p_def, &parent_fields[19], 0));

    /* Re-populate for JSON edge cases */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_reflect_array_resize(p_def, &parent_fields[18], 1));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_reflect_array_resize(p_def, &parent_fields[19], 2));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_default(
                                 &child_ti, (void **)&p_def->ptr_obj_arr[0]));
    p_def->ptr_obj_arr[1] = NULL; /* NULL element in ptr_obj_arr */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_reflect_array_resize(p_def, &parent_fields[17], 2));
    C_CDD_FREE(p_def->str_arr[0]);
    p_def->str_arr[0] = NULL; /* NULL element in str_arr */
    p_def->str_arr[1] = (char *)C_CDD_MALLOC(3);
    memcpy(p_def->str_arr[1], "hi", 3);

    /* Deep validate recursive failure on nested ptr_child, inline_child, and
     * arrays */
    child_fields[0].has_min = 1;
    child_fields[0].min_val = 1000.0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_validate(p_def, &parent_ti));
    p_def->inline_child.id = 2000;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_validate(p_def, &parent_ti));
    p_def->ptr_child->id = 2000;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_validate(p_def, &parent_ti));
    p_def->val_obj_arr[0].id = 2000;
    p_def->ptr_obj_arr[0]->id = 2000;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_validate(p_def, &parent_ti));
    child_fields[0].has_min = 0;

    /* Serialization with NULL elements in str_arr / ptr_obj_arr and numeric
     * enum fallback */
    p_def->eval = 99; /* Unmapped enum variant -> numeric fallback */
    p_def->enum_arr[0] = 99;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_to_json(p_def, &parent_ti, &json_out));
    ASSERT(json_out != NULL);
    C_CDD_FREE(json_out);
    p_def->eval = 10;
    p_def->enum_arr[0] = 10;

    /* Deserialization error branches: bad types, missing required, other_idx
     * enum */
    enum_ti.other_idx = 1;
    {
      struct AdvParent *p_tmp = NULL;
      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_reflect_from_json("{\"eval\":\"unknown_var\",\"enum_arr\":[10]}",
                                &parent_ti, (void **)&p_tmp));
      ASSERT_EQ(20, p_tmp->eval);
      ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_cleanup(p_tmp, &parent_ti));
      C_CDD_FREE(p_tmp);
    }
    enum_ti.other_idx = -1;
    {
      struct AdvParent *p_tmp = NULL;
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_from_json("{\"eval\":\"unknown_var\"}", &parent_ti,
                                      (void **)&p_tmp));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_from_json("{\"eval\":true}", &parent_ti,
                                      (void **)&p_tmp));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_from_json("{\"i64\":\"bad\"}", &parent_ti,
                                      (void **)&p_tmp));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_from_json("{\"num\":\"bad\"}", &parent_ti,
                                      (void **)&p_tmp));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_from_json("{\"bval\":\"bad\"}", &parent_ti,
                                      (void **)&p_tmp));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_from_json("{\"str~name/1\":123}", &parent_ti,
                                      (void **)&p_tmp));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_from_json("{\"ptr_child\":123}", &parent_ti,
                                      (void **)&p_tmp));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_from_json("{\"int_arr\":123}", &parent_ti,
                                      (void **)&p_tmp));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_from_json("{\"fixed_ints\":[1,2,3]}", &parent_ti,
                                      (void **)&p_tmp));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_from_json("{\"int_arr\":[\"x\"]}", &parent_ti,
                                      (void **)&p_tmp));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_from_json("{\"dbl_arr\":[\"x\"]}", &parent_ti,
                                      (void **)&p_tmp));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_from_json("{\"bool_arr\":[\"x\"]}", &parent_ti,
                                      (void **)&p_tmp));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_from_json("{\"enum_arr\":[true]}", &parent_ti,
                                      (void **)&p_tmp));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_from_json("{\"enum_arr\":[\"bad\"]}", &parent_ti,
                                      (void **)&p_tmp));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_from_json("{\"str_arr\":[123]}", &parent_ti,
                                      (void **)&p_tmp));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_from_json("{\"val_obj_arr\":[123]}", &parent_ti,
                                      (void **)&p_tmp));
      ASSERT_EQ(
          CDD_C_ERROR_INVALID_ARGUMENT,
          cdd_reflect_from_json("{\"i64\":null}", &parent_ti, (void **)&p_tmp));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_from_json(
                    "{\"ptr_child\":null,\"opt_int\":null,\"str~name/1\":null}",
                    &parent_ti, (void **)&p_tmp));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_cleanup(p_tmp, &parent_ti));
      C_CDD_FREE(p_tmp);
    }

    /* Merge patch with null clearing and inline object diffing */
    {
      char *mp = NULL;
      C_CDD_FREE(p_copy->opt_int);
      p_copy->opt_int = NULL;
      p_copy->inline_child.id = 777;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_merge_patch(p_def, p_copy, &parent_ti, &mp));
      ASSERT(mp != NULL);
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_apply_merge_patch(p_def, &parent_ti, mp));
      ASSERT_EQ(777, p_def->inline_child.id);
      ASSERT(p_def->opt_int == NULL);
      C_CDD_FREE(mp);
    }

    /* OOM injection on default, deepcopy, to_json, from_json, merge_patch */
    {
      void *oom_out = NULL;
      char *oom_str = NULL;
      g_cdd_alloc_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                cdd_reflect_struct_default(&child_ti, &oom_out));
      g_cdd_alloc_fail = 2;
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                cdd_reflect_struct_default(&child_ti, &oom_out));
      g_cdd_alloc_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                cdd_reflect_struct_deepcopy(p_def, &oom_out, &parent_ti));
      g_cdd_alloc_fail = 2;
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                cdd_reflect_struct_deepcopy(p_def, &oom_out, &parent_ti));
      g_cdd_alloc_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                cdd_reflect_to_json(p_def, &parent_ti, &oom_str));
      g_cdd_alloc_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                cdd_reflect_merge_patch(p_def, p_copy, &parent_ti, &oom_str));
      g_cdd_alloc_fail = 0;
    }

    /* Additional format/regex/bitfield/union/eq/serde edge-case tests */
    {
      struct AdvParent p_a, p_b;
      struct AdvUnion u_a, u_b;
      char long_pat[300];
      memset(&p_a, 0, sizeof(p_a));
      memset(&p_b, 0, sizeof(p_b));
      memset(&u_a, 0, sizeof(u_a));
      memset(&u_b, 0, sizeof(u_b));

      /* Oversized regex pattern buffer */
      memset(long_pat, 'a', sizeof(long_pat) - 1);
      long_pat[sizeof(long_pat) - 1] = '\0';
      parent_fields[10].pattern = long_pat;
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_validate_field(&p1, &parent_fields[10]));
      long_pat[254] = '\\';
      long_pat[255] = 'd';
      long_pat[256] = '\0';
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_validate_field(&p1, &parent_fields[10]));
      parent_fields[10].pattern = NULL;

      /* Additional format failure branches */
      parent_fields[10].format = "uuid";
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_set_string(&p1, &parent_fields[10],
                                       "550e8400Xe29b-41d4-a716-446655440000"));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_validate_field(&p1, &parent_fields[10]));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_set_string(&p1, &parent_fields[10],
                                       "550e840G-e29b-41d4-a716-446655440000"));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_validate_field(&p1, &parent_fields[10]));

      parent_fields[10].format = "email";
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_set_string(&p1, &parent_fields[10], "a@b"));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_validate_field(&p1, &parent_fields[10]));

      parent_fields[10].format = "date";
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_set_string(&p1, &parent_fields[10], "202X-01-01"));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_validate_field(&p1, &parent_fields[10]));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_string(&p1, &parent_fields[10],
                                                      "2026-01-01-extra"));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_validate_field(&p1, &parent_fields[10]));

      parent_fields[10].format = "date-time";
      ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_string(&p1, &parent_fields[10],
                                                      "2026-01-01X12:00:00Z"));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_validate_field(&p1, &parent_fields[10]));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_string(&p1, &parent_fields[10],
                                                      "2026-01-01T1X:00:00Z"));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_validate_field(&p1, &parent_fields[10]));

      parent_fields[10].format = "ipv4";
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_set_string(&p1, &parent_fields[10], "1.2.3."));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_validate_field(&p1, &parent_fields[10]));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_set_string(&p1, &parent_fields[10], "1.2.3.4x"));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_validate_field(&p1, &parent_fields[10]));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_set_string(&p1, &parent_fields[10], ".1.2.3"));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_validate_field(&p1, &parent_fields[10]));

      parent_fields[10].format = "ipv6";
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_set_string(&p1, &parent_fields[10], "2001:db8::g"));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_validate_field(&p1, &parent_fields[10]));

      parent_fields[10].format = "hostname";
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_set_string(&p1, &parent_fields[10], "bad_host!"));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_validate_field(&p1, &parent_fields[10]));
      parent_fields[10].format = "custom_unknown_fmt";
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_validate_field(&p1, &parent_fields[10]));
      parent_fields[10].format = NULL;

      /* Union inactive tag (0) cleanup/deepcopy/eq/to_json */
      u_a.tag = 0;
      u_b.tag = 0;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_struct_eq(&u_a, &u_b, &union_ti, &is_eq));
      ASSERT_EQ(1, is_eq);
      u_b.tag = 1;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_struct_eq(&u_a, &u_b, &union_ti, &is_eq));
      ASSERT_EQ(0, is_eq);
      ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_cleanup(&u_a, &union_ti));
      {
        struct AdvUnion *u_z = NULL;
        char *uj = NULL;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_struct_deepcopy(&u_a, (void **)&u_z, &union_ti));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_to_json(&u_a, &union_ti, &uj));
        C_CDD_FREE(uj);
        C_CDD_FREE(u_z);
      }

      /* eq_single_field mismatch branches */
      p_a.num = 1.0;
      p_b.num = 2.0;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_struct_eq(&p_a, &p_b, &parent_ti, &is_eq));
      ASSERT_EQ(0, is_eq);
      p_b.num = 1.0;

      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_set_string(&p_a, &parent_fields[10], "s1"));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_set_string(&p_b, &parent_fields[10], "s2"));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_struct_eq(&p_a, &p_b, &parent_ti, &is_eq));
      ASSERT_EQ(0, is_eq);
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_set_string(&p_b, &parent_fields[10], "s1"));

      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_struct_default(&child_ti, (void **)&p_a.ptr_child));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_struct_eq(&p_a, &p_b, &parent_ti, &is_eq));
      ASSERT_EQ(0, is_eq);

      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_array_resize(&p_a, &parent_fields[13], 1));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_struct_eq(&p_a, &p_b, &parent_ti, &is_eq));
      ASSERT_EQ(0, is_eq);
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_array_resize(&p_b, &parent_fields[13], 1));
      p_a.int_arr[0] = 1;
      p_b.int_arr[0] = 2;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_struct_eq(&p_a, &p_b, &parent_ti, &is_eq));
      ASSERT_EQ(0, is_eq);
      p_b.int_arr[0] = 1;

      ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_cleanup(&p_a, &parent_ti));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_cleanup(&p_b, &parent_ti));
    }

    /* Exercise struct_fields_free and struct_field_add_cli_* helpers */
    {
      struct StructFields sf_cli;
      struct StructField *got_f = NULL;
      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_cli));
      ASSERT_EQ(CDD_C_SUCCESS,
                struct_fields_add(&sf_cli, "f1", "string", NULL, NULL, NULL));
      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_get(&sf_cli, "f1", &got_f));
      ASSERT(got_f != NULL);
      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_get(&sf_cli, "missing", &got_f));
      ASSERT(got_f == NULL);
      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_get(NULL, "f1", &got_f));
      ASSERT(got_f == NULL);

      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                struct_field_add_alias(NULL, "a"));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                struct_field_add_alias(&sf_cli.fields[0], ""));
      ASSERT_EQ(CDD_C_SUCCESS, struct_field_add_alias(&sf_cli.fields[0], "a1"));
      ASSERT_EQ(CDD_C_SUCCESS, struct_field_add_alias(&sf_cli.fields[0], "a1"));

      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                struct_fields_add_cli_alias(NULL, "ca"));
      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add_cli_alias(&sf_cli, "ca1"));

      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                struct_field_add_cli_conflict(NULL, "c1"));
      ASSERT_EQ(CDD_C_SUCCESS,
                struct_field_add_cli_conflict(&sf_cli.fields[0], "c1"));

      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                struct_field_add_cli_requires(NULL, "r1"));
      ASSERT_EQ(CDD_C_SUCCESS,
                struct_field_add_cli_requires(&sf_cli.fields[0], "r1"));

      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                struct_field_add_cli_required_unless(NULL, "ru1"));
      ASSERT_EQ(CDD_C_SUCCESS,
                struct_field_add_cli_required_unless(&sf_cli.fields[0], "ru1"));

      c_cdd_strdup("{}", &sf_cli.fields[0].schema_extra_json);
      c_cdd_strdup("{}", &sf_cli.fields[0].items_extra_json);
      c_cdd_strdup("help", &sf_cli.fields[0].cli_long_help);
      sf_cli.fields[0].n_type_union = 1;
      sf_cli.fields[0].type_union = (char **)C_CDD_CALLOC(1, sizeof(char *));
      c_cdd_strdup("string", &sf_cli.fields[0].type_union[0]);
      sf_cli.fields[0].n_items_type_union = 1;
      sf_cli.fields[0].items_type_union =
          (char **)C_CDD_CALLOC(1, sizeof(char *));
      c_cdd_strdup("integer", &sf_cli.fields[0].items_type_union[0]);
      c_cdd_strdup("{}", &sf_cli.schema_extra_json);
      c_cdd_strdup("about", &sf_cli.cli_long_about);
      c_cdd_strdup("before", &sf_cli.cli_before_help);
      c_cdd_strdup("after", &sf_cli.cli_after_help);
      c_cdd_strdup("type", &sf_cli.union_discriminator);
      sf_cli.n_union_variants = 1;
      sf_cli.union_variants = (struct UnionVariantMeta *)C_CDD_CALLOC(
          1, sizeof(struct UnionVariantMeta));
      c_cdd_strdup("v1", &sf_cli.union_variants[0].disc_value);
      sf_cli.union_variants[0].n_required_props = 1;
      sf_cli.union_variants[0].required_props =
          (char **)C_CDD_CALLOC(1, sizeof(char *));
      c_cdd_strdup("id", &sf_cli.union_variants[0].required_props[0]);
      sf_cli.union_variants[0].n_property_names = 1;
      sf_cli.union_variants[0].property_names =
          (char **)C_CDD_CALLOC(1, sizeof(char *));
      c_cdd_strdup("id", &sf_cli.union_variants[0].property_names[0]);
      struct_fields_free(&sf_cli);
    }

    /* Exercise write_struct_cleanup_func & write_struct_default_func
     * fixed-array and bit-field default branches */
    {
      struct StructFields sf_cg;
      FILE *tmp_cg = tmpfile();
      ASSERT(tmp_cg != NULL);
      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_cg));
      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_cg, "fix_str_arr", "array",
                                                 "string", NULL, NULL));
      sf_cg.fields[0].fixed_array_len = 3;
      ASSERT_EQ(CDD_C_SUCCESS,
                struct_fields_add(&sf_cg, "fix_obj_arr", "array",
                                  "#/components/schemas/SubObj", NULL, NULL));
      sf_cg.fields[1].fixed_array_len = 2;
      ASSERT_EQ(CDD_C_SUCCESS,
                struct_fields_add(&sf_cg, "bf_fn", "integer", NULL, NULL, "4"));
      CDD_STRCPY(sf_cg.fields[2].serde_default_fn,
                 sizeof(sf_cg.fields[2].serde_default_fn), "init_bf_fn");
      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_cg, "norm_fn", "integer",
                                                 NULL, NULL, NULL));
      CDD_STRCPY(sf_cg.fields[3].serde_default_fn,
                 sizeof(sf_cg.fields[3].serde_default_fn), "init_norm_fn");
      ASSERT_EQ(CDD_C_SUCCESS,
                struct_fields_add(&sf_cg, "fix_str_def", "string", NULL,
                                  "\"hello\"", NULL));
      sf_cg.fields[4].fixed_array_len = 16;
      ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_cg, "bf_enum", "enum",
                                                 "MyEnum", "\"A\"", "3"));
      ASSERT_EQ(CDD_C_SUCCESS,
                write_struct_cleanup_func(tmp_cg, "CgStruct", &sf_cg, NULL));
      ASSERT_EQ(CDD_C_SUCCESS,
                write_struct_default_func(tmp_cg, "CgStruct", &sf_cg, NULL));
      ASSERT_EQ(CDD_C_SUCCESS,
                write_struct_deepcopy_func(tmp_cg, "CgStruct", &sf_cg, NULL));
      ASSERT_EQ(CDD_C_SUCCESS,
                write_struct_eq_func(tmp_cg, "CgStruct", &sf_cg, NULL));
      ASSERT_EQ(CDD_C_SUCCESS,
                struct_fields_add(&sf_cg, "dyn_obj_arr", "array",
                                  "#/components/schemas/SubObj", NULL, NULL));
      ASSERT_EQ(CDD_C_SUCCESS,
                write_struct_deepcopy_func(tmp_cg, "CgStruct2", &sf_cg, NULL));
      struct_fields_free(&sf_cg);
      fclose(tmp_cg);
    }

    /* Additional bit-field, pointer uint64, and fixed array branches */
    {
      struct ExtraStruct {
        int bf_storage;
        uint64_t *pu64;
        char *fix_strs[2];
        unsigned char raw_bytes[4];
      } extra_inst, extra_copy;
      uint64_t u64_target = 100U;
      struct cdd_field_info extra_f[4];
      struct cdd_type_info extra_ti;

      memset(&extra_inst, 0, sizeof(extra_inst));
      memset(&extra_copy, 0, sizeof(extra_copy));
      memset(extra_f, 0, sizeof(extra_f));
      memset(&extra_ti, 0, sizeof(extra_ti));

      extra_f[0].name = "pu64";
      extra_f[0].c_type = "uint64_t *";
      extra_f[0].kind = CDD_FIELD_KIND_INTEGER;
      extra_f[0].offset = offsetof(struct ExtraStruct, pu64);
      extra_f[0].size = sizeof(uint64_t *);
      extra_f[0].elem_size = sizeof(uint64_t);
      extra_f[0].is_pointer = 1;

      ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                cdd_reflect_get_uint64(&extra_inst, &extra_f[0], &v_u64));
      ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                cdd_reflect_set_uint64(&extra_inst, &extra_f[0], 50U));
      extra_inst.pu64 = &u64_target;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_set_uint64(&extra_inst, &extra_f[0], 999U));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_get_uint64(&extra_inst, &extra_f[0], &v_u64));
      ASSERT_EQ(999U, v_u64);

      extra_f[1].name = "bf";
      extra_f[1].c_type = "int";
      extra_f[1].kind = CDD_FIELD_KIND_INTEGER;
      extra_f[1].bit_width = 4;
      extra_f[1].get_bitfield = test_bf_get_fail;
      extra_f[1].set_bitfield = test_bf_set;
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_get_int64(&extra_inst, &extra_f[1], &v_i64));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_get_uint64(&extra_inst, &extra_f[1], &v_u64));
      extra_f[1].get_bitfield = test_bf_get;

      extra_f[2].name = "fix_strs";
      extra_f[2].c_type = "char *[2]";
      extra_f[2].kind = CDD_FIELD_KIND_ARRAY;
      extra_f[2].elem_kind = CDD_FIELD_KIND_STRING;
      extra_f[2].elem_size = sizeof(char *);
      extra_f[2].offset = offsetof(struct ExtraStruct, fix_strs);
      extra_f[2].size = sizeof(extra_inst.fix_strs);
      extra_f[2].fixed_array_len = 2;

      extra_f[3].name = "raw_bytes";
      extra_f[3].c_type = "unsigned char[4]";
      extra_f[3].kind = CDD_FIELD_KIND_UNKNOWN;
      extra_f[3].offset = offsetof(struct ExtraStruct, raw_bytes);
      extra_f[3].size = sizeof(extra_inst.raw_bytes);

      extra_ti.name = "ExtraStruct";
      extra_ti.size = sizeof(extra_inst);
      extra_ti.fields = extra_f;
      extra_ti.n_fields = 4;

      extra_inst.pu64 = &u64_target;
      c_cdd_strdup("s0", &extra_inst.fix_strs[0]);
      c_cdd_strdup("s1", &extra_inst.fix_strs[1]);
      {
        void *ec_ptr = NULL;
        struct TwoChildWrap {
          struct AdvChild c1;
          struct AdvChild c2;
        } tcw;
        struct cdd_field_info tcw_f[2];
        struct cdd_type_info tcw_ti;
        memset(&tcw, 0, sizeof(tcw));
        c_cdd_strdup("l1", &tcw.c1.label);
        memset(tcw_f, 0, sizeof(tcw_f));
        tcw_f[0].name = "c1";
        tcw_f[0].kind = CDD_FIELD_KIND_OBJECT;
        tcw_f[0].ref_name = "UnregChild";
        tcw_f[0].offset = offsetof(struct TwoChildWrap, c1);
        tcw_f[0].size = sizeof(struct AdvChild);
        tcw_f[1].name = "c2";
        tcw_f[1].kind = CDD_FIELD_KIND_OBJECT;
        tcw_f[1].ref_name = "AdvChild";
        tcw_f[1].offset = offsetof(struct TwoChildWrap, c2);
        tcw_f[1].size = sizeof(struct AdvChild);
        memset(&tcw_ti, 0, sizeof(tcw_ti));
        tcw_ti.name = "TwoChildWrap";
        tcw_ti.size = sizeof(tcw);
        tcw_ti.fields = tcw_f;
        tcw_ti.n_fields = 2;
        ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                  cdd_reflect_struct_deepcopy(&tcw, &ec_ptr, &tcw_ti));
        C_CDD_FREE(tcw.c1.label);

        g_cdd_alloc_fail = 2;
        ASSERT_EQ(CDD_C_ERROR_MEMORY,
                  cdd_reflect_struct_deepcopy(&extra_inst, &ec_ptr, &extra_ti));
        g_cdd_alloc_fail = 0;
        extra_inst.pu64 = NULL;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_struct_deepcopy(&extra_inst, &ec_ptr, &extra_ti));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_eq(&extra_inst, ec_ptr,
                                                       &extra_ti, &is_eq));
        ASSERT_EQ(1, is_eq);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_cleanup(ec_ptr, &extra_ti));
        C_CDD_FREE(ec_ptr);
      }
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_struct_cleanup(&extra_inst, &extra_ti));
    }

    /* Final branch coverage sweep for all reflection functions */
    {
      /* 1. Escape sequences \d, \w, \s inside bracketed character classes */
      parent_fields[10].pattern = "^[\\d][\\w][\\s]$";
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_set_string(&p1, &parent_fields[10], "5a "));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_validate_field(&p1, &parent_fields[10]));
      parent_fields[10].pattern = NULL;

      /* 2. Required non-nullable NULL pointer validation failure */
      parent_fields[21].required = 1;
      parent_fields[21].is_nullable = 0;
      p1.opt_int = NULL;
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_validate_field(&p1, &parent_fields[21]));
      parent_fields[21].required = 0;
      parent_fields[21].is_nullable = 1;

      /* 3. int32 format out of range */
      parent_fields[0].format = "int32";
      p1.i64 = ((int64_t)1 << 40);
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_validate_field(&p1, &parent_fields[0]));
      parent_fields[0].format = NULL;
      p1.i64 = 10;

      /* 4. String and array min_len / max_len violations */
      parent_fields[10].has_min_len = 1;
      parent_fields[10].min_len = 10;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_reflect_set_string(&p1, &parent_fields[10], "short"));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_validate_field(&p1, &parent_fields[10]));
      parent_fields[10].has_min_len = 0;
      parent_fields[10].has_max_len = 1;
      parent_fields[10].max_len = 2;
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_validate_field(&p1, &parent_fields[10]));
      parent_fields[10].has_max_len = 0;

      parent_fields[13].has_min_len = 1;
      parent_fields[13].min_len = 5;
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_validate_field(&p1, &parent_fields[13]));
      parent_fields[13].has_min_len = 0;
      parent_fields[13].has_max_len = 1;
      parent_fields[13].max_len = 1;
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_validate_field(&p1, &parent_fields[13]));
      parent_fields[13].has_max_len = 0;

      /* 5. Visitor early abort & self-cycle on validate and visit_deep_mut */
      {
        int bad_cnt = -1;
        struct AdvChild cyc_child;
        memset(&cyc_child, 0, sizeof(cyc_child));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_visit_deep_mut(p_def, &parent_ti,
                                             test_reflect_mut_visitor_cb,
                                             &bad_cnt));
        child_fields[0].kind = CDD_FIELD_KIND_OBJECT;
        child_fields[0].ref_name = "AdvChild";
        child_fields[0].is_pointer = 0;
        bad_cnt = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_visit_deep_mut(
                                     &cyc_child, &child_ti,
                                     test_reflect_mut_visitor_cb, &bad_cnt));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_validate(&cyc_child, &child_ti));
        child_fields[0].kind = CDD_FIELD_KIND_INTEGER;
        child_fields[0].c_type = "int";
        child_fields[0].ref_name = NULL;
      }

      /* 6. Path resolution failure propagation in get/set_*_by_path */
      ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                cdd_reflect_get_int64_by_path(p_def, &parent_ti, "/no_field",
                                              &v_i64));
      ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, cdd_reflect_set_int64_by_path(
                                           p_def, &parent_ti, "/no_field", 1));
      ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                cdd_reflect_get_double_by_path(p_def, &parent_ti, "/no_field",
                                               &v_dbl));
      ASSERT_EQ(
          CDD_C_ERROR_NOT_FOUND,
          cdd_reflect_set_double_by_path(p_def, &parent_ti, "/no_field", 1.0));
      ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                cdd_reflect_get_string_by_path(p_def, &parent_ti, "/no_field",
                                               &v_str));
      ASSERT_EQ(
          CDD_C_ERROR_NOT_FOUND,
          cdd_reflect_set_string_by_path(p_def, &parent_ti, "/no_field", "x"));
      ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                cdd_reflect_resolve_path(p_def, &parent_ti, "/int_arr/99",
                                         &res_f, &res_parent, &res_ptr));
      ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                cdd_reflect_resolve_path(p_def, &parent_ti, "/int_arr/0/sub",
                                         &res_f, &res_parent, &res_ptr));
      ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                cdd_reflect_resolve_path(&p1, &parent_ti, "/ptr_child/id",
                                         &res_f, &res_parent, &res_ptr));

      /* 7. Default numeric enum fallback & bitfield without setter */
      parent_fields[9].default_val = "20";
      {
        struct AdvParent *p_num_ev = NULL;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_struct_default(&parent_ti, (void **)&p_num_ev));
        ASSERT_EQ(20, p_num_ev->eval);
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_struct_cleanup(p_num_ev, &parent_ti));
        C_CDD_FREE(p_num_ev);
      }
      parent_fields[9].default_val = "alpha";

      /* 8. eq_single_field array element mismatches (string & object) and
       * pointer double NULL vs non-NULL */
      {
        struct AdvParent *p_cmp = NULL;
        C_CDD_FREE(p_def->opt_int);
        p_def->opt_int = NULL;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_array_resize(p_def, &parent_fields[17], 2));
        C_CDD_FREE(p_def->str_arr[0]);
        p_def->str_arr[0] = NULL;
        C_CDD_FREE(p_def->str_arr[1]);
        c_cdd_strdup("hi", &p_def->str_arr[1]);
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_array_resize(p_def, &parent_fields[19], 1));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_deepcopy(
                                     p_def, (void **)&p_cmp, &parent_ti));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_struct_eq(p_def, p_cmp, &parent_ti, &is_eq));
        ASSERT_EQ(1, is_eq);

        C_CDD_FREE(p_cmp->str_arr[1]);
        c_cdd_strdup("diff_str", &p_cmp->str_arr[1]);
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_struct_eq(p_def, p_cmp, &parent_ti, &is_eq));
        ASSERT_EQ(0, is_eq);
        C_CDD_FREE(p_cmp->str_arr[1]);
        c_cdd_strdup("hi", &p_cmp->str_arr[1]);

        p_cmp->val_obj_arr[0].id = 99999;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_struct_eq(p_def, p_cmp, &parent_ti, &is_eq));
        ASSERT_EQ(0, is_eq);
        p_cmp->val_obj_arr[0].id = p_def->val_obj_arr[0].id;

        p_cmp->opt_int = (int *)C_CDD_MALLOC(sizeof(int));
        *p_cmp->opt_int = 123;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_struct_eq(p_def, p_cmp, &parent_ti, &is_eq));
        ASSERT_EQ(0, is_eq);
        C_CDD_FREE(p_cmp->opt_int);
        p_cmp->opt_int = NULL;

        /* Test eq_single_field on pointer NUMBER when one or both are NULL */
        {
          struct PdblDummy {
            double *pd;
          } d1, d2;
          double dv = 1.5;
          struct cdd_field_info pdbl_f;
          struct cdd_type_info pdbl_ti;
          d1.pd = NULL;
          d2.pd = &dv;
          memset(&pdbl_f, 0, sizeof(pdbl_f));
          pdbl_f.name = "pd";
          pdbl_f.c_type = "double *";
          pdbl_f.kind = CDD_FIELD_KIND_NUMBER;
          pdbl_f.offset = 0;
          pdbl_f.size = sizeof(double *);
          pdbl_f.is_pointer = 1;
          memset(&pdbl_ti, 0, sizeof(pdbl_ti));
          pdbl_ti.name = "PdblDummy";
          pdbl_ti.size = sizeof(d1);
          pdbl_ti.fields = &pdbl_f;
          pdbl_ti.n_fields = 1;
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_struct_eq(&d1, &d2, &pdbl_ti, &is_eq));
          ASSERT_EQ(0, is_eq);
          d2.pd = NULL;
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_struct_eq(&d1, &d2, &pdbl_ti, &is_eq));
          ASSERT_EQ(1, is_eq);
        }

        /* Skip & flatten pointer object in serialize / deserialize / patch */
        parent_fields[0].skip = 1;
        parent_fields[12].flatten = 1;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_struct_cleanup(p_cmp->ptr_child, &child_ti));
        C_CDD_FREE(p_cmp->ptr_child);
        p_cmp->ptr_child = NULL;
        {
          char *sj = NULL;
          struct AdvParent *p_des = NULL;
          ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_to_json(p_def, &parent_ti, &sj));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_from_json(sj, &parent_ti, (void **)&p_des));
          parent_fields[12].flatten = 0;
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_apply_merge_patch(
                        p_cmp, &parent_ti, "{\"ptr_child\":{\"id\":7}}"));
          ASSERT(p_cmp->ptr_child != NULL);
          ASSERT_EQ(7, p_cmp->ptr_child->id);
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_struct_cleanup(p_des, &parent_ti));
          C_CDD_FREE(p_des);
          C_CDD_FREE(sj);
        }
        parent_fields[0].skip = 0;

        /* Required field missing in JSON deserialization */
        parent_fields[7].required = 1;
        {
          struct AdvParent *p_req = NULL;
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_from_json("{}", &parent_ti, (void **)&p_req));
        }
        parent_fields[7].required = 0;

        /* 9. Exhaustive OOM & unregistered ref_name / malformed regex / path
         * edge cases */
        {
          int *b_ptr_val = NULL;
          struct cdd_field_info bptr_f;
          void *oom_tmp = NULL;
          char *oom_j = NULL;
          int oom_step;
          char long_pat[300];
          struct AdvParent *p_flat = NULL;

          memset(&bptr_f, 0, sizeof(bptr_f));
          bptr_f.name = "bptr";
          bptr_f.c_type = "int *";
          bptr_f.kind = CDD_FIELD_KIND_BOOLEAN;
          bptr_f.offset = 0;
          bptr_f.size = sizeof(int *);
          bptr_f.is_pointer = 1;
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_get_bool(&b_ptr_val, &bptr_f, &is_eq));

          /* 8-byte unsigned pointer NULL in get_uint64 and set_uint64 */
          {
            uint64_t *u64_ptr_val = NULL;
            uint64_t u64_out = 0;
            struct cdd_field_info u64p_f;
            memset(&u64p_f, 0, sizeof(u64p_f));
            u64p_f.name = "u64p";
            u64p_f.c_type = "uint64_t *";
            u64p_f.kind = CDD_FIELD_KIND_INTEGER;
            u64p_f.offset = 0;
            u64p_f.size = sizeof(uint64_t *);
            u64p_f.elem_size = sizeof(uint64_t);
            u64p_f.is_pointer = 1;
            ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                      cdd_reflect_get_uint64(&u64_ptr_val, &u64p_f, &u64_out));
            ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                      cdd_reflect_set_uint64(&u64_ptr_val, &u64p_f, 100U));
          }

          /* Signed 4-byte integer overflow in cdd_reflect_set_int64 */
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_set_int64(&p_def->inline_child,
                                          &child_fields[0],
                                          ((int64_t)1 << 40)));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_set_int64(&p_def->inline_child,
                                          &child_fields[0],
                                          -((int64_t)1 << 40)));

          /* Union eq when both tags are 0 (outside 1..n_fields) and union
           * deepcopy on pointer field */
          {
            struct AdvUnion u0_a, u0_b;
            struct AdvUnion *u_dc = NULL;
            memset(&u0_a, 0, sizeof(u0_a));
            memset(&u0_b, 0, sizeof(u0_b));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_struct_eq(&u0_a, &u0_b, &union_ti, &is_eq));
            ASSERT_EQ(1, is_eq);
            u0_a.tag = 2;
            c_cdd_strdup("ustr", &u0_a.data.str_branch);
            ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_deepcopy(
                                         &u0_a, (void **)&u_dc, &union_ti));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_struct_cleanup(u_dc, &union_ti));
            C_CDD_FREE(u_dc);
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_struct_cleanup(&u0_a, &union_ti));
          }

          /* Fixed-length string array overwrite in deserialize_field_from_json
           * and integer default_val "0" vs "0x10" */
          {
            struct FixStrWrap {
              char *arr[2];
            } fsw;
            struct cdd_field_info fsw_f;
            struct cdd_type_info fsw_ti;
            struct AdvParent *p_zero_def = NULL;
            parent_fields[6].default_val = "0";
            ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_default(
                                         &parent_ti, (void **)&p_zero_def));
            ASSERT_EQ(0U, p_zero_def->u32);
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_struct_cleanup(p_zero_def, &parent_ti));
            C_CDD_FREE(p_zero_def);
            parent_fields[6].default_val = "100";

            memset(&fsw, 0, sizeof(fsw));
            c_cdd_strdup("old0", &fsw.arr[0]);
            c_cdd_strdup("old1", &fsw.arr[1]);
            memset(&fsw_f, 0, sizeof(fsw_f));
            fsw_f.name = "arr";
            fsw_f.wire_name = "arr";
            fsw_f.c_type = "char *[2]";
            fsw_f.kind = CDD_FIELD_KIND_ARRAY;
            fsw_f.elem_kind = CDD_FIELD_KIND_STRING;
            fsw_f.elem_size = sizeof(char *);
            fsw_f.offset = 0;
            fsw_f.size = sizeof(fsw.arr);
            fsw_f.fixed_array_len = 2;
            memset(&fsw_ti, 0, sizeof(fsw_ti));
            fsw_ti.name = "FixStrWrap";
            fsw_ti.size = sizeof(fsw);
            fsw_ti.fields = &fsw_f;
            fsw_ti.n_fields = 1;
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_apply_merge_patch(
                          &fsw, &fsw_ti, "{\"arr\":[null,\"new1\"]}"));
            ASSERT_EQ(NULL, fsw.arr[0]);
            ASSERT_STR_EQ("new1", fsw.arr[1]);
            ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_cleanup(&fsw, &fsw_ti));
          }

          /* Malformed regex inside quantifier, alternation error, buffer
           * overflow, and bracket class at end-of-string */
          parent_fields[10].pattern = "^hi[a]$";
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_validate_field(p_def, &parent_fields[10]));
          parent_fields[10].pattern = "[a*";
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_validate_field(p_def, &parent_fields[10]));
          parent_fields[10].pattern = "a*[b";
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_validate_field(p_def, &parent_fields[10]));
          parent_fields[10].pattern = "[b|a";
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_validate_field(p_def, &parent_fields[10]));
          memset(long_pat, 'a', sizeof(long_pat) - 1);
          long_pat[sizeof(long_pat) - 1] = '\0';
          parent_fields[10].pattern = long_pat;
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_validate_field(p_def, &parent_fields[10]));
          long_pat[254] = '\\';
          long_pat[255] = 'd';
          long_pat[256] = '\0';
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_validate_field(p_def, &parent_fields[10]));
          parent_fields[10].pattern = NULL;

          /* validate_string_format NULL/empty, validate_field bit-field
           * getter error, array_len error */
          parent_fields[10].format = "";
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_validate_field(p_def, &parent_fields[10]));
          parent_fields[10].format = NULL;

          {
            struct cdd_field_info bf_tmp_f;
            struct cdd_type_info bf_tmp_ti;
            int bf_dummy = 0, bf_copy_dummy = 0;
            void *bf_copy_ptr = NULL;
            memset(&bf_tmp_f, 0, sizeof(bf_tmp_f));
            bf_tmp_f.name = "bf";
            bf_tmp_f.c_type = "int";
            bf_tmp_f.kind = CDD_FIELD_KIND_INTEGER;
            bf_tmp_f.bit_width = 4;
            bf_tmp_f.has_min = 1;
            bf_tmp_f.get_bitfield = test_bf_get_fail;
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(&bf_dummy, &bf_tmp_f));
            bf_tmp_f.has_min = 0;
            bf_tmp_f.get_bitfield = test_bf_get;
            bf_tmp_f.set_bitfield = NULL;
            memset(&bf_tmp_ti, 0, sizeof(bf_tmp_ti));
            bf_tmp_ti.name = "BfTmp";
            bf_tmp_ti.size = sizeof(int);
            bf_tmp_ti.fields = &bf_tmp_f;
            bf_tmp_ti.n_fields = 1;
            ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_deepcopy(
                                         &bf_dummy, &bf_copy_ptr, &bf_tmp_ti));
            C_CDD_FREE(bf_copy_ptr);
            (void)bf_copy_dummy;
          }

          parent_fields[13].is_flexible_array = 1;
          parent_fields[13].array_len_offset = 0;
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_validate_field(p_def, &parent_fields[13]));
          parent_fields[13].is_flexible_array = 0;
          parent_fields[13].array_len_offset =
              offsetof(struct AdvParent, n_int_arr);

          /* Visitor early abort inside nested object / object array in
           * visit_deep & visit_deep_mut */
          if (!p_def->ptr_child) {
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_struct_default(&child_ti,
                                                 (void **)&p_def->ptr_child));
          }
          {
            int lim;
            for (lim = 1; lim <= 30; ++lim) {
              int c1 = 100 - lim;
              int c2 = 100 - lim;
              cdd_reflect_visit_deep(p_def, &parent_ti, test_reflect_visitor_cb,
                                     &c1);
              cdd_reflect_visit_deep_mut(p_def, &parent_ti,
                                         test_reflect_mut_visitor_cb, &c2);
            }
          }

          /* Path resolution edge cases: ~0 escape, invalid bracket,
           * unregistered child type, NULL element in ptr_obj_arr */
          {
            char huge_seg[200];
            memset(huge_seg, 'x', sizeof(huge_seg) - 1);
            huge_seg[0] = '/';
            huge_seg[1] = '[';
            huge_seg[sizeof(huge_seg) - 1] = '\0';
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_resolve_path(p_def, &parent_ti, huge_seg,
                                               &res_f, &res_parent, &res_ptr));
            huge_seg[1] = '~';
            huge_seg[2] = '0';
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_resolve_path(p_def, &parent_ti, huge_seg,
                                               &res_f, &res_parent, &res_ptr));
            ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                      cdd_reflect_resolve_path(p_def, &parent_ti,
                                               "/missing~0name~11", &res_f,
                                               &res_parent, &res_ptr));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_resolve_path(p_def, &parent_ti,
                                               "/int_arr/[abc]", &res_f,
                                               &res_parent, &res_ptr));

            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_array_resize(p_def, &parent_fields[19], 2));
            p_def->ptr_obj_arr[1] = NULL;
            ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                      cdd_reflect_resolve_path(p_def, &parent_ti,
                                               "/ptr_obj_arr/1/id", &res_f,
                                               &res_parent, &res_ptr));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_array_resize(p_def, &parent_fields[19], 1));

            parent_fields[11].ref_name = "UnregType";
            ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                      cdd_reflect_resolve_path(p_def, &parent_ti,
                                               "/inline_child/id", &res_f,
                                               &res_parent, &res_ptr));
            parent_fields[11].ref_name = "";
            ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                      cdd_reflect_resolve_path(p_def, &parent_ti,
                                               "/inline_child/id", &res_f,
                                               &res_parent, &res_ptr));
            parent_fields[11].ref_name = "AdvChild";

            parent_fields[18].ref_name = "UnregType";
            ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                      cdd_reflect_resolve_path(p_def, &parent_ti,
                                               "/val_obj_arr/0/id", &res_f,
                                               &res_parent, &res_ptr));
            parent_fields[18].ref_name = "AdvChild";
          }

          /* Unregistered ref_name error branches across deepcopy, eq, to_json,
           * from_json, merge_patch, apply_merge_patch */
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_array_resize(p_def, &parent_fields[18], 1));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_struct_cleanup(p_cmp, &parent_ti));
          C_CDD_FREE(p_cmp);
          ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_deepcopy(
                                       p_def, (void **)&p_cmp, &parent_ti));
          parent_fields[11].ref_name = "UnregChild";
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_struct_deepcopy(p_def, &oom_tmp, &parent_ti));
          p_cmp->inline_child.id = 9999;
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_struct_eq(p_def, p_cmp, &parent_ti, &is_eq));
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_to_json(p_def, &parent_ti, &oom_j));
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_from_json("{\"inline_child\":{\"id\":1}}",
                                          &parent_ti, &oom_tmp));
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_merge_patch(p_def, p_cmp, &parent_ti, &oom_j));
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_apply_merge_patch(
                        p_def, &parent_ti, "{\"inline_child\":{\"id\":1}}"));
          parent_fields[11].flatten = 1;
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_from_json("{\"id\":1}", &parent_ti, &oom_tmp));
          parent_fields[11].flatten = 0;
          parent_fields[11].ref_name = "AdvChild";
          p_cmp->inline_child.id = p_def->inline_child.id;

          parent_fields[18].ref_name = "UnregChild";
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_struct_deepcopy(p_def, &oom_tmp, &parent_ti));
          p_cmp->val_obj_arr[0].id = 9999;
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_struct_eq(p_def, p_cmp, &parent_ti, &is_eq));
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_to_json(p_def, &parent_ti, &oom_j));
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_from_json("{\"val_obj_arr\":[{\"id\":1}]}",
                                          &parent_ti, &oom_tmp));
          parent_fields[18].ref_name = "AdvChild";
          p_cmp->val_obj_arr[0].id = p_def->val_obj_arr[0].id;

          parent_fields[9].ref_name = "UnregEnum";
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_from_json("{\"eval\":\"alpha\"}", &parent_ti,
                                          &oom_tmp));
          parent_fields[9].ref_name = "AdvEnum";

          parent_fields[16].ref_name = "UnregEnum";
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_from_json("{\"enum_arr\":[\"alpha\"]}",
                                          &parent_ti, &oom_tmp));
          parent_fields[16].ref_name = "AdvEnum";

          /* Nested child error during serialize_field_to_json /
           * build_merge_patch_obj / apply_merge_patch_obj */
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_array_resize(p_def, &parent_fields[18], 1));
          child_fields[2].kind = CDD_FIELD_KIND_OBJECT;
          child_fields[2].ref_name = "UnregNested";
          child_fields[2].is_pointer = 0;
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_to_json(p_def, &parent_ti, &oom_j));
          parent_fields[11].skip = 1;
          parent_fields[12].skip = 1;
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_to_json(p_def, &parent_ti, &oom_j));
          parent_fields[11].skip = 0;
          parent_fields[12].skip = 0;
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_array_resize(p_def, &parent_fields[18], 0));
          p_cmp->inline_child.id = 8888;
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_merge_patch(p_def, p_cmp, &parent_ti, &oom_j));
          p_cmp->inline_child.id = p_def->inline_child.id;
          ASSERT_EQ(
              CDD_C_ERROR_NOT_FOUND,
              cdd_reflect_apply_merge_patch(
                  p_def, &parent_ti, "{\"inline_child\":{\"label\":{}}}"));
          ASSERT_EQ(
              CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_apply_merge_patch(
                  p_def, &parent_ti, "{\"inline_child\":{\"id\":\"bad\"}}"));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_from_json("{\"inline_child\":{\"id\":\"bad\"}}",
                                          &parent_ti, &oom_tmp));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_from_json("{\"ptr_child\":{\"id\":\"bad\"}}",
                                          &parent_ti, &oom_tmp));
          ASSERT_EQ(
              CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_from_json("{\"val_obj_arr\":[{\"id\":\"bad\"}]}",
                                    &parent_ti, &oom_tmp));
          ASSERT_EQ(
              CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_from_json("{\"ptr_obj_arr\":[{\"id\":\"bad\"}]}",
                                    &parent_ti, &oom_tmp));
          ASSERT_EQ(
              CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_from_json("{\"u8\":999}", &parent_ti, &oom_tmp));
          parent_fields[13].c_type = "uint8_t *";
          parent_fields[13].elem_size = sizeof(uint8_t);
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_from_json("{\"int_arr\":[999]}", &parent_ti,
                                          &oom_tmp));
          parent_fields[13].c_type = "int *";
          parent_fields[13].elem_size = sizeof(int);
          child_fields[2].kind = CDD_FIELD_KIND_STRING;
          child_fields[2].ref_name = NULL;
          child_fields[2].is_pointer = 1;

          /* Deserialization overwriting existing string/null in fixed/dynamic
           * arrays and flatten pointer object */
          parent_fields[12].flatten = 1;
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_from_json("{\"id\":42,\"str_arr\":[\"a\",null],"
                                          "\"ptr_obj_arr\":[{\"id\":3},null]}",
                                          &parent_ti, (void **)&p_flat));
          ASSERT(p_flat != NULL && p_flat->ptr_child != NULL);
          ASSERT_EQ(42, p_flat->ptr_child->id);
          /* Apply merge patch onto p_flat where str_arr[0] and ptr_obj_arr[0]
           * are already non-NULL */
          ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_apply_merge_patch(
                                       p_flat, &parent_ti,
                                       "{\"id\":43,\"str_arr\":[null,\"b\"],"
                                       "\"ptr_obj_arr\":[null,{\"id\":4}]}"));
          parent_fields[12].flatten = 0;
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_struct_cleanup(p_flat, &parent_ti));
          C_CDD_FREE(p_flat);

          /* Exhaustive OOM step loop for deepcopy, from_json, apply_merge_patch
           */
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_array_resize(p_def, &parent_fields[18], 1));
          c_cdd_strdup("vo_lbl", &p_def->val_obj_arr[0].label);
          p_def->opt_int = (int *)C_CDD_MALLOC(sizeof(int));
          *p_def->opt_int = 55;
          for (oom_step = 1; oom_step <= 25; ++oom_step) {
            void *d_out = NULL;
            g_cdd_alloc_fail = oom_step;
            if (cdd_reflect_struct_deepcopy(p_def, &d_out, &parent_ti) ==
                CDD_C_SUCCESS) {
              cdd_reflect_struct_cleanup(d_out, &parent_ti);
              C_CDD_FREE(d_out);
              break;
            }
          }
          for (oom_step = 1; oom_step <= 25; ++oom_step) {
            void *j_out = NULL;
            g_cdd_alloc_fail = oom_step;
            if (cdd_reflect_from_json("{\"opt_int\":1,\"ptr_child\":{\"id\":1},"
                                      "\"int_arr\":[1],\"str_arr\":[\"s\"],"
                                      "\"ptr_obj_arr\":[{\"id\":2}]}",
                                      &parent_ti, &j_out) == CDD_C_SUCCESS) {
              cdd_reflect_struct_cleanup(j_out, &parent_ti);
              C_CDD_FREE(j_out);
              break;
            }
          }
          parent_fields[12].flatten = 1;
          for (oom_step = 1; oom_step <= 10; ++oom_step) {
            void *j_out = NULL;
            g_cdd_alloc_fail = oom_step;
            if (cdd_reflect_from_json("{\"id\":1}", &parent_ti, &j_out) ==
                CDD_C_SUCCESS) {
              cdd_reflect_struct_cleanup(j_out, &parent_ti);
              C_CDD_FREE(j_out);
              break;
            }
          }
          parent_fields[12].flatten = 0;
          if (p_cmp->ptr_child) {
            cdd_reflect_struct_cleanup(p_cmp->ptr_child, &child_ti);
            C_CDD_FREE(p_cmp->ptr_child);
            p_cmp->ptr_child = NULL;
          }
          g_cdd_alloc_fail = 1;
          ASSERT_EQ(CDD_C_ERROR_MEMORY,
                    cdd_reflect_apply_merge_patch(
                        p_cmp, &parent_ti, "{\"ptr_child\":{\"id\":7}}"));
          g_cdd_alloc_fail = 0;
        }

        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_cleanup(p_cmp, &parent_ti));
        C_CDD_FREE(p_cmp);
      }
    }
  }

  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_cleanup(&p1, &parent_ti));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_cleanup(p_def, &parent_ti));
  C_CDD_FREE(p_def);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_cleanup(p_copy, &parent_ti));
  C_CDD_FREE(p_copy);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_clear_registry());
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_STRUCT_REFLECT_EDGES_H */
