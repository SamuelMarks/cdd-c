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
#include "classes/emit/struct_internal.h"
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
    g_cdd_last_alloc_err = struct_fields_free(&sf_bw);
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
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_to_json_object(NULL, &parent_ti, (void *)1));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_to_json_object(p_def, NULL, (void *)1));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_to_json_object(p_def, &parent_ti, NULL));
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
      {
        int oom_k;
        struct AdvChild c_oom;
        memset(&c_oom, 0, sizeof(c_oom));
        for (oom_k = 1; oom_k <= 20; ++oom_k) {
          g_cdd_alloc_fail = oom_k;
          if (cdd_reflect_to_json(&c_oom, &child_ti, &oom_str) == CDD_C_SUCCESS)
            C_CDD_FREE(oom_str);
        }
        g_cdd_alloc_fail = 1;
        ASSERT_EQ(CDD_C_ERROR_MEMORY,
                  cdd_reflect_to_json(p_def, &parent_ti, &oom_str));
        g_cdd_alloc_fail = 0;
      }
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

      g_cdd_last_alloc_err =
          c_cdd_strdup("{}", &sf_cli.fields[0].schema_extra_json);
      g_cdd_last_alloc_err =
          c_cdd_strdup("{}", &sf_cli.fields[0].items_extra_json);
      g_cdd_last_alloc_err =
          c_cdd_strdup("help", &sf_cli.fields[0].cli_long_help);
      sf_cli.fields[0].n_type_union = 1;
      sf_cli.fields[0].type_union = (char **)C_CDD_CALLOC(1, sizeof(char *));
      g_cdd_last_alloc_err =
          c_cdd_strdup("string", &sf_cli.fields[0].type_union[0]);
      sf_cli.fields[0].n_items_type_union = 1;
      sf_cli.fields[0].items_type_union =
          (char **)C_CDD_CALLOC(1, sizeof(char *));
      g_cdd_last_alloc_err =
          c_cdd_strdup("integer", &sf_cli.fields[0].items_type_union[0]);
      g_cdd_last_alloc_err = c_cdd_strdup("{}", &sf_cli.schema_extra_json);
      g_cdd_last_alloc_err = c_cdd_strdup("about", &sf_cli.cli_long_about);
      g_cdd_last_alloc_err = c_cdd_strdup("before", &sf_cli.cli_before_help);
      g_cdd_last_alloc_err = c_cdd_strdup("after", &sf_cli.cli_after_help);
      g_cdd_last_alloc_err = c_cdd_strdup("type", &sf_cli.union_discriminator);
      sf_cli.n_union_variants = 1;
      sf_cli.union_variants = (struct UnionVariantMeta *)C_CDD_CALLOC(
          1, sizeof(struct UnionVariantMeta));
      g_cdd_last_alloc_err =
          c_cdd_strdup("v1", &sf_cli.union_variants[0].disc_value);
      sf_cli.union_variants[0].n_required_props = 1;
      sf_cli.union_variants[0].required_props =
          (char **)C_CDD_CALLOC(1, sizeof(char *));
      g_cdd_last_alloc_err =
          c_cdd_strdup("id", &sf_cli.union_variants[0].required_props[0]);
      sf_cli.union_variants[0].n_property_names = 1;
      sf_cli.union_variants[0].property_names =
          (char **)C_CDD_CALLOC(1, sizeof(char *));
      g_cdd_last_alloc_err =
          c_cdd_strdup("id", &sf_cli.union_variants[0].property_names[0]);
      g_cdd_last_alloc_err = struct_fields_free(&sf_cli);
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
      g_cdd_last_alloc_err = struct_fields_free(&sf_cg);
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
      g_cdd_last_alloc_err = c_cdd_strdup("s0", &extra_inst.fix_strs[0]);
      g_cdd_last_alloc_err = c_cdd_strdup("s1", &extra_inst.fix_strs[1]);
      {
        void *ec_ptr = NULL;
        struct TwoChildWrap {
          struct AdvChild c1;
          struct AdvChild c2;
        } tcw;
        struct cdd_field_info tcw_f[2];
        struct cdd_type_info tcw_ti;
        memset(&tcw, 0, sizeof(tcw));
        g_cdd_last_alloc_err = c_cdd_strdup("l1", &tcw.c1.label);
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
        g_cdd_last_alloc_err = c_cdd_strdup("hi", &p_def->str_arr[1]);
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_array_resize(p_def, &parent_fields[19], 1));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_deepcopy(
                                     p_def, (void **)&p_cmp, &parent_ti));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_struct_eq(p_def, p_cmp, &parent_ti, &is_eq));
        ASSERT_EQ(1, is_eq);

        C_CDD_FREE(p_cmp->str_arr[1]);
        g_cdd_last_alloc_err = c_cdd_strdup("diff_str", &p_cmp->str_arr[1]);
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_struct_eq(p_def, p_cmp, &parent_ti, &is_eq));
        ASSERT_EQ(0, is_eq);
        C_CDD_FREE(p_cmp->str_arr[1]);
        g_cdd_last_alloc_err = c_cdd_strdup("hi", &p_cmp->str_arr[1]);

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
            g_cdd_last_alloc_err = c_cdd_strdup("ustr", &u0_a.data.str_branch);
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
            g_cdd_last_alloc_err = c_cdd_strdup("old0", &fsw.arr[0]);
            g_cdd_last_alloc_err = c_cdd_strdup("old1", &fsw.arr[1]);
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
            int bf_dummy = 0;
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
              g_cdd_last_alloc_err = cdd_reflect_visit_deep(
                  p_def, &parent_ti, test_reflect_visitor_cb, &c1);
              g_cdd_last_alloc_err = cdd_reflect_visit_deep_mut(
                  p_def, &parent_ti, test_reflect_mut_visitor_cb, &c2);
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
          g_cdd_last_alloc_err =
              c_cdd_strdup("vo_lbl", &p_def->val_obj_arr[0].label);
          p_def->opt_int = (int *)C_CDD_MALLOC(sizeof(int));
          *p_def->opt_int = 55;
          for (oom_step = 1; oom_step <= 25; ++oom_step) {
            void *d_out = NULL;
            g_cdd_alloc_fail = oom_step;
            if (cdd_reflect_struct_deepcopy(p_def, &d_out, &parent_ti) ==
                CDD_C_SUCCESS) {
              g_cdd_last_alloc_err =
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
              g_cdd_last_alloc_err =
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
              g_cdd_last_alloc_err =
                  cdd_reflect_struct_cleanup(j_out, &parent_ti);
              C_CDD_FREE(j_out);
              break;
            }
          }
          parent_fields[12].flatten = 0;
          if (p_cmp->ptr_child) {
            g_cdd_last_alloc_err =
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

      /* 10. Test cdd_reflect_get/set_uint64_by_path,
       * cdd_reflect_get/set_bool_by_path, uint64_t > INT64_MAX, grouped regex
       * patterns, depth >= 16 guard, and cdd_reflect_to_json_schema */
      {
        int b_out = 0;
        uint64_t u_out = 0;
        uint64_t big_u64 = ((uint64_t)1U << 63) + 1024U;
        char *schema_json = NULL;
        struct AdvParent *p_u64 = NULL;

        /* uint64_by_path & bool_by_path */
        ASSERT_EQ(
            CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_get_uint64_by_path(p_def, &parent_ti, "/u64", NULL));
        ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                  cdd_reflect_get_uint64_by_path(p_def, &parent_ti, "/missing",
                                                 &u_out));
        ASSERT_EQ(
            CDD_C_ERROR_NOT_FOUND,
            cdd_reflect_set_uint64_by_path(p_def, &parent_ti, "/missing", 1U));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_get_uint64_by_path(p_def, &parent_ti,
                                                 "/dbl_arr/0", &u_out));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_set_uint64_by_path(p_def, &parent_ti,
                                                 "/dbl_arr/0", 1U));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_uint64_by_path(
                                     p_def, &parent_ti, "/u64", big_u64));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_get_uint64_by_path(
                                     p_def, &parent_ti, "/u64", &u_out));
        ASSERT(u_out == big_u64);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_uint64_by_path(
                                     p_def, &parent_ti, "/int_arr/0", 77U));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_get_uint64_by_path(
                                     p_def, &parent_ti, "/int_arr/0", &u_out));
        ASSERT_EQ(77U, u_out);

        ASSERT_EQ(
            CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_get_bool_by_path(p_def, &parent_ti, "/bval", NULL));
        ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                  cdd_reflect_get_bool_by_path(p_def, &parent_ti, "/missing",
                                               &b_out));
        ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, cdd_reflect_set_bool_by_path(
                                             p_def, &parent_ti, "/missing", 1));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_get_bool_by_path(p_def, &parent_ti, "/int_arr/0",
                                               &b_out));
        ASSERT_EQ(
            CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_set_bool_by_path(p_def, &parent_ti, "/int_arr/0", 1));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_set_bool_by_path(p_def, &parent_ti, "/bval", 0));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_get_bool_by_path(p_def, &parent_ti,
                                                              "/bval", &b_out));
        ASSERT_EQ(0, b_out);
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_array_resize(p_def, &parent_fields[15], 2));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_bool_by_path(
                                     p_def, &parent_ti, "/bool_arr/1", 1));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_get_bool_by_path(
                                     p_def, &parent_ti, "/bool_arr/1", &b_out));
        ASSERT_EQ(1, b_out);

        /* uint64_t > INT64_MAX validation, eq, and JSON round-trip */
        parent_fields[1].has_min = 1;
        parent_fields[1].min_val = 0.0;
        parent_fields[1].format = "uint64";
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_validate_field(p_def, &parent_fields[1]));
        parent_fields[1].format = "int32";
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_validate_field(p_def, &parent_fields[1]));
        parent_fields[1].format = "uint32";
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_validate_field(p_def, &parent_fields[1]));
        parent_fields[1].format = NULL;
        parent_fields[1].has_min = 0;

        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_deepcopy(
                                     p_def, (void **)&p_u64, &parent_ti));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_struct_eq(p_def, p_u64, &parent_ti, &is_eq));
        ASSERT_EQ(1, is_eq);
        p_u64->u64 = 42U;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_struct_eq(p_def, p_u64, &parent_ti, &is_eq));
        ASSERT_EQ(0, is_eq);
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_to_json(p_def, &parent_ti, &json_out));
        ASSERT(json_out != NULL);
        C_CDD_FREE(json_out);
        ASSERT_EQ(
            CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_apply_merge_patch(p_u64, &parent_ti, "{\"u64\":-5}"));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_apply_merge_patch(
                                     p_u64, &parent_ti, "{\"u64\":12345}"));
        ASSERT_EQ(12345U, p_u64->u64);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_cleanup(p_u64, &parent_ti));
        C_CDD_FREE(p_u64);
        p_def->u64 = 100U;

        /* Parenthesized regex groups and quantifiers on groups */
        parent_fields[10].pattern = "^(foo|bar)+_((ab){2,3})?\\)$";
        ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_string(&p1, &parent_fields[10],
                                                        "foobar_abab)"));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_validate_field(&p1, &parent_fields[10]));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_set_string(&p1, &parent_fields[10], "foo_)"));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_validate_field(&p1, &parent_fields[10]));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_set_string(&p1, &parent_fields[10], "baz_)"));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_validate_field(&p1, &parent_fields[10]));
        parent_fields[10].pattern = "^([a-z)]+)\\*$";
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_set_string(&p1, &parent_fields[10], "ab)*"));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_validate_field(&p1, &parent_fields[10]));
        /* Unbalanced parentheses error branches */
        parent_fields[10].pattern = "^(abc$";
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_validate_field(&p1, &parent_fields[10]));
        parent_fields[10].pattern = "^abc)$";
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_validate_field(&p1, &parent_fields[10]));
        parent_fields[10].pattern = "^(abc){x}$";
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_validate_field(&p1, &parent_fields[10]));
        parent_fields[10].pattern = NULL;

        /* Depth >= 16 recursion guard in visit_deep, visit_deep_mut, validate
         */
        {
          struct DeepChainNode {
            struct DeepChainNode *next;
          } nodes[18];
          static struct cdd_field_info nf;
          static struct cdd_type_info nti;
          int d_idx;
          int d_cnt = 0;
          memset(nodes, 0, sizeof(nodes));
          for (d_idx = 0; d_idx < 17; ++d_idx) {
            nodes[d_idx].next = &nodes[d_idx + 1];
          }
          memset(&nf, 0, sizeof(nf));
          nf.name = "next";
          nf.ref_name = "DeepChainNode";
          nf.kind = CDD_FIELD_KIND_OBJECT;
          nf.offset = 0;
          nf.size = sizeof(void *);
          nf.is_pointer = 1;
          memset(&nti, 0, sizeof(nti));
          nti.name = "DeepChainNode";
          nti.size = sizeof(struct DeepChainNode);
          nti.fields = &nf;
          nti.n_fields = 1;
          ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_register_type(&nti));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_visit_deep(&nodes[0], &nti,
                                           test_reflect_visitor_cb, &d_cnt));
          ASSERT_EQ(16, d_cnt);
          d_cnt = 0;
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_visit_deep_mut(
                        &nodes[0], &nti, test_reflect_mut_visitor_cb, &d_cnt));
          ASSERT_EQ(16, d_cnt);
          ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_validate(&nodes[0], &nti));
        }

        /* cdd_reflect_to_json_schema tests */
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_to_json_schema(NULL, &schema_json));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_to_json_schema(&parent_ti, NULL));
        parent_fields[0].required = 1;
        parent_fields[0].doc = "Primary ID";
        parent_fields[0].deprecated = 1;
        parent_fields[0].read_only = 1;
        parent_fields[1].write_only = 1;
        parent_fields[1].has_min = 1;
        parent_fields[1].exclusive_min = 1;
        parent_fields[1].min_val = 0.0;
        parent_fields[1].has_max = 1;
        parent_fields[1].exclusive_max = 1;
        parent_fields[1].max_val = 1000.0;
        parent_fields[2].skip = 1;
        parent_fields[10].has_min_len = 1;
        parent_fields[10].min_len = 1;
        parent_fields[10].has_max_len = 1;
        parent_fields[10].max_len = 64;
        parent_fields[10].pattern = "^[a-z]+$";
        parent_fields[10].format = "hostname";
        parent_fields[13].has_min_len = 1;
        parent_fields[13].min_len = 1;
        parent_fields[13].has_max_len = 1;
        parent_fields[13].max_len = 10;
        parent_fields[13].unique_items = 1;
        g_cdd_alloc_fail = 1;
        ASSERT_EQ(CDD_C_ERROR_MEMORY,
                  cdd_reflect_to_json_schema(&parent_ti, &schema_json));
        g_cdd_alloc_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_reflect_to_json_schema(&parent_ti, &schema_json));
        ASSERT(schema_json != NULL);
        ASSERT(strstr(schema_json, "\"title\":\"AdvParent\"") != NULL);
        ASSERT(strstr(schema_json, "\"required\":[\"i64\"]") != NULL);
        C_CDD_FREE(schema_json);
        parent_fields[0].required = 0;
        parent_fields[0].doc = NULL;
        parent_fields[0].deprecated = 0;
        parent_fields[0].read_only = 0;
        parent_fields[1].write_only = 0;
        parent_fields[1].has_min = 0;
        parent_fields[1].exclusive_min = 0;
        parent_fields[1].has_max = 0;
        parent_fields[1].exclusive_max = 0;
        parent_fields[2].skip = 0;
        parent_fields[10].has_min_len = 0;
        parent_fields[10].has_max_len = 0;
        parent_fields[10].pattern = NULL;
        parent_fields[10].format = NULL;
        parent_fields[13].has_min_len = 0;
        parent_fields[13].has_max_len = 0;
        parent_fields[13].unique_items = 0;

        /* Test cdd_reflect_to_form_urlencoded,
         * cdd_reflect_from_form_urlencoded, cdd_reflect_from_cli, and
         * cdd_reflect_apply_json_patch */
        {
          char *form_out = NULL;
          struct AdvParent *p_form = NULL;
          struct AdvParent *p_cli = NULL;
          const char *cli_argv[8];

          ASSERT_EQ(
              CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_to_form_urlencoded(NULL, &parent_ti, &form_out));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_to_form_urlencoded(p_def, NULL, &form_out));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_to_form_urlencoded(p_def, &parent_ti, NULL));
          g_cdd_alloc_fail = 1;
          ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_reflect_to_form_urlencoded(
                                            p_def, &parent_ti, &form_out));
          g_cdd_alloc_fail = 2;
          ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_reflect_to_form_urlencoded(
                                            p_def, &parent_ti, &form_out));
          g_cdd_alloc_fail = 0;
          ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_to_form_urlencoded(
                                       p_def, &parent_ti, &form_out));
          ASSERT(form_out != NULL);

          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_from_form_urlencoded(NULL, &parent_ti,
                                                     (void **)&p_form));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_from_form_urlencoded(form_out, NULL,
                                                     (void **)&p_form));
          ASSERT_EQ(
              CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_from_form_urlencoded(form_out, &parent_ti, NULL));
          g_cdd_alloc_fail = 1;
          ASSERT_EQ(CDD_C_ERROR_MEMORY,
                    cdd_reflect_from_form_urlencoded(form_out, &parent_ti,
                                                     (void **)&p_form));
          g_cdd_alloc_fail = 0;
          ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_from_form_urlencoded(
                                       "&&i64=42&num=2.5&bval=true&eval=alpha&"
                                       "str=hello+world&unknown=1&&",
                                       &parent_ti, (void **)&p_form));
          ASSERT(p_form != NULL);
          ASSERT_EQ(42, p_form->i64);
          ASSERT_EQ(1, p_form->bval);
          ASSERT_STR_EQ("hello world", p_form->str);
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_struct_cleanup(p_form, &parent_ti));
          C_CDD_FREE(p_form);
          C_CDD_FREE(form_out);

          /* Invalid form scalar values */
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_from_form_urlencoded(
                        "i64=not_an_int", &parent_ti, (void **)&p_form));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_from_form_urlencoded(
                        "num=not_a_num", &parent_ti, (void **)&p_form));
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_from_form_urlencoded(
                        "eval=bad_enum", &parent_ti, (void **)&p_form));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_from_form_urlencoded(
                        "inline_child=bad_obj", &parent_ti, (void **)&p_form));
          parent_fields[9].ref_name = "UnregisteredEnum";
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_from_form_urlencoded("eval=alpha", &parent_ti,
                                                     (void **)&p_form));
          parent_fields[9].ref_name = "AdvEnum";

          /* CLI reflection tests */
          cli_argv[0] = "prog";
          cli_argv[1] = "--i64=99";
          cli_argv[2] = "--bval";
          cli_argv[3] = "--str";
          cli_argv[4] = "cli_val";
          cli_argv[5] = "--unknown=skip";
          ASSERT_EQ(
              CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_from_cli(-1, cli_argv, &parent_ti, (void **)&p_cli));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_from_cli(1, NULL, &parent_ti, (void **)&p_cli));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_from_cli(1, cli_argv, NULL, (void **)&p_cli));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_from_cli(1, cli_argv, &parent_ti, NULL));
          g_cdd_alloc_fail = 1;
          ASSERT_EQ(
              CDD_C_ERROR_MEMORY,
              cdd_reflect_from_cli(6, cli_argv, &parent_ti, (void **)&p_cli));
          g_cdd_alloc_fail = 0;
          ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_from_cli(6, cli_argv, &parent_ti,
                                                        (void **)&p_cli));
          ASSERT(p_cli != NULL);
          ASSERT_EQ(99, p_cli->i64);
          ASSERT_EQ(1, p_cli->bval);
          ASSERT_STR_EQ("cli_val", p_cli->str);
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_struct_cleanup(p_cli, &parent_ti));
          C_CDD_FREE(p_cli);

          cli_argv[0] = "--str";
          ASSERT_EQ(
              CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_from_cli(1, cli_argv, &parent_ti, (void **)&p_cli));
          cli_argv[0] = "--i64=bad";
          ASSERT_EQ(
              CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_from_cli(1, cli_argv, &parent_ti, (void **)&p_cli));

          /* RFC 6902 JSON Patch tests */
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_apply_json_patch(NULL, &parent_ti, "[]"));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_apply_json_patch(p_def, NULL, "[]"));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_apply_json_patch(p_def, &parent_ti, NULL));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_apply_json_patch(p_def, &parent_ti, "{}"));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_apply_json_patch(p_def, &parent_ti, "[{}]"));
          ASSERT_EQ(
              CDD_C_ERROR_NOT_FOUND,
              cdd_reflect_apply_json_patch(
                  p_def, &parent_ti,
                  "[{\"op\":\"replace\",\"path\":\"/nope\",\"value\":1}]"));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_apply_json_patch(
                        p_def, &parent_ti,
                        "[{\"op\":\"replace\",\"path\":\"/i64\"}]"));
          ASSERT_EQ(
              CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_apply_json_patch(
                  p_def, &parent_ti,
                  "[{\"op\":\"replace\",\"path\":\"/i64\",\"value\":[]}]"));
          ASSERT_EQ(
              CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_apply_json_patch(
                  p_def, &parent_ti, "[{\"op\":\"test\",\"path\":\"/i64\"}]"));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_apply_json_patch(
                        p_def, &parent_ti,
                        "[{\"op\":\"test\",\"path\":\"/i64\",\"value\":[]}]"));
          ASSERT_EQ(
              CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_apply_json_patch(
                  p_def, &parent_ti, "[{\"op\":\"copy\",\"path\":\"/i64\"}]"));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_apply_json_patch(
                        p_def, &parent_ti,
                        "[{\"op\":\"unknown\",\"path\":\"/i64\"}]"));
          ASSERT_EQ(
              CDD_C_SUCCESS,
              cdd_reflect_apply_json_patch(
                  p_def, &parent_ti,
                  "[{\"op\":\"replace\",\"path\":\"/i64\",\"value\":321},"
                  "{\"op\":\"replace\",\"path\":\"/num\",\"value\":9.5},"
                  "{\"op\":\"replace\",\"path\":\"/bval\",\"value\":true},"
                  "{\"op\":\"replace\",\"path\":\"/"
                  "str\",\"value\":\"patched\"}]"));
          ASSERT_EQ(
              CDD_C_SUCCESS,
              cdd_reflect_apply_json_patch(
                  p_def, &parent_ti,
                  "[{\"op\":\"test\",\"path\":\"/i64\",\"value\":321},"
                  "{\"op\":\"test\",\"path\":\"/str\",\"value\":\"patched\"},"
                  "{\"op\":\"copy\",\"from\":\"/i64\",\"path\":\"/u32\"},"
                  "{\"op\":\"move\",\"from\":\"/str\",\"path\":\"/inline_child/"
                  "label\"},"
                  "{\"op\":\"remove\",\"path\":\"/opt_int\"}]"));
          ASSERT_EQ(321, p_def->i64);
          ASSERT_EQ(321U, p_def->u32);
          ASSERT(p_def->str == NULL);
          ASSERT_STR_EQ("patched", p_def->inline_child.label);
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_apply_json_patch(
                        p_def, &parent_ti,
                        "[{\"op\":\"test\",\"path\":\"/i64\",\"value\":999}]"));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_apply_json_patch(
                        p_def, &parent_ti,
                        "[{\"op\":\"test\",\"path\":\"/inline_child/"
                        "label\",\"value\":\"wrong\"}]"));
          ASSERT_EQ(
              CDD_C_SUCCESS,
              cdd_reflect_apply_json_patch(
                  p_def, &parent_ti,
                  "[{\"op\":\"add\",\"path\":\"/dbl_arr/0\",\"value\":7.75}]"));

          /* Cover negative int64 write on uint64 field, non-nullable NULL
           * pointer in serialize_field_to_json, and resolve_path_mut NULL */
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_set_int64(p_def, &parent_fields[1], -1));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_set_int64_by_path(NULL, &parent_ti, "/i64", 1));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_resolve_path(
                        p_def, &parent_ti,
                        "/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
                        &res_f, &res_parent, &res_ptr));
          parent_fields[21].is_nullable = 0;
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_to_json(p_def, &parent_ti, &json_out));
          C_CDD_FREE(json_out);
          parent_fields[21].is_nullable = 1;

          /* Cover deepcopy_single_field when child_ti is NULL on object field
           * and when unknown field kind is compared */
          parent_fields[11].ref_name = "";
          ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_deepcopy(
                                       p_def, (void **)&p_form, &parent_ti));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_struct_cleanup(p_form, &parent_ti));
          C_CDD_FREE(p_form);
          parent_fields[11].ref_name = "AdvChild";

          parent_fields[0].kind = CDD_FIELD_KIND_UNKNOWN;
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_struct_eq(p_def, p_copy, &parent_ti, &is_eq));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_to_json(p_def, &parent_ti, &json_out));
          C_CDD_FREE(json_out);
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_from_json("{}", &parent_ti, (void **)&p_form));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_struct_cleanup(p_form, &parent_ti));
          C_CDD_FREE(p_form);
          parent_fields[0].kind = CDD_FIELD_KIND_INTEGER;

          /* Cover oversized bracket segment, hostname trailing '-', '\S' regex
           * atom, and number enum JSON deserialization */
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_resolve_path(
                        p_def, &parent_ti,
                        "/[aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa]",
                        &res_f, &res_parent, &res_ptr));

          parent_fields[10].format = "hostname";
          ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_string(
                                       p_def, &parent_fields[10], "bad-host-"));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_validate_field(p_def, &parent_fields[10]));
          parent_fields[10].format = NULL;
          parent_fields[10].pattern = "^\\S+$";
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_validate_field(p_def, &parent_fields[10]));
          parent_fields[10].pattern = NULL;

          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_from_json("{\"eval\":20}", &parent_ti,
                                          (void **)&p_form));
          ASSERT_EQ(20, p_form->eval);
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_struct_cleanup(p_form, &parent_ti));
          C_CDD_FREE(p_form);

          /* Cover pointer uint64 set when *pptr is non-NULL, oversized path
           * segment without leading '/' */
          {
            uint64_t tmp_u64 = 10U;
            p_def->opt_int = (int *)(void *)&tmp_u64;
            parent_fields[21].c_type = "uint64_t *";
            parent_fields[21].size = sizeof(void *);
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_set_uint64(p_def, &parent_fields[21], 88U));
            parent_fields[21].c_type = "int *";
            p_def->opt_int = NULL;
          }

          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_resolve_path(
                        p_def, &parent_ti,
                        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
                        &res_f, &res_parent, &res_ptr));

          parent_fields[10].format = "date-time";
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_set_string(p_def, &parent_fields[10],
                                           "2026-11-06T25:00:00Z"));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_validate_field(p_def, &parent_fields[10]));
          parent_fields[10].format = NULL;

          /* Cover set_uint64 > max_i64 on narrower field, regex_match_atom at
           * c == '\0', and ~0 escape in bracket path segment */
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_set_uint64(p_def, &parent_fields[6], big_u64));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_set_uint64(p_def, &parent_fields[1], big_u64));
          parent_fields[10].pattern = "^\\d+\\d$";
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_set_string(p_def, &parent_fields[10], "12"));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_validate_field(p_def, &parent_fields[10]));
          parent_fields[10].pattern = NULL;
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_resolve_path(p_def, &parent_ti, "/int_arr/[~1]",
                                             &res_f, &res_parent, &res_ptr));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_apply_merge_patch(
                        p_def, &parent_ti, "{\"inline_child\":{\"id\":77}}"));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_set_uint64(NULL, &parent_fields[1], 1U));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_from_json("[]", &parent_ti, (void **)&p_form));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_apply_merge_patch(p_def, &parent_ti, "[]"));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_resolve_path(
                                       p_def, &parent_ti, "/inline_child//id",
                                       &res_f, &res_parent, &res_ptr));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_resolve_path(p_def, NULL, "/i64", &res_f,
                                             &res_parent, &res_ptr));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_resolve_path(p_def, &parent_ti, NULL, &res_f,
                                             &res_parent, &res_ptr));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_resolve_path(p_def, &parent_ti, "/int_arr/[0",
                                             &res_f, &res_parent, &res_ptr));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_resolve_path(p_def, &parent_ti, "/i64", NULL,
                                             &res_parent, &res_ptr));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_resolve_path(p_def, &parent_ti, "/i64", &res_f,
                                             NULL, &res_ptr));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_resolve_path(p_def, &parent_ti, "/i64", &res_f,
                                             &res_parent, NULL));
          ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                    cdd_reflect_resolve_path(p_def, &parent_ti, "/~2", &res_f,
                                             &res_parent, &res_ptr));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_reflect_resolve_path(p_def, &parent_ti, "/[]", &res_f,
                                             &res_parent, &res_ptr));
          parent_fields[0].bit_width = 4;
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_reflect_resolve_path(p_def, &parent_ti, "/i64", &res_f,
                                             &res_parent, &res_ptr));
          ASSERT_EQ((void *)NULL, res_ptr);
          parent_fields[0].bit_width = 0;
          {
            const char *saved_ref = parent_fields[11].ref_name;
            parent_fields[11].ref_name = NULL;
            ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                      cdd_reflect_resolve_path(p_def, &parent_ti,
                                               "/inline_child/id", &res_f,
                                               &res_parent, &res_ptr));
            parent_fields[11].ref_name = saved_ref;

            saved_ref = parent_fields[18].ref_name;
            parent_fields[18].ref_name = NULL;
            ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                      cdd_reflect_resolve_path(p_def, &parent_ti,
                                               "/val_obj_arr/0/id", &res_f,
                                               &res_parent, &res_ptr));
            parent_fields[18].ref_name = "";
            ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                      cdd_reflect_resolve_path(p_def, &parent_ti,
                                               "/val_obj_arr/0/id", &res_f,
                                               &res_parent, &res_ptr));
            parent_fields[18].ref_name = saved_ref;
          }
          {
            const char *null_al[2] = {NULL, "valid_al"};
            struct cdd_enum_variant_info ev_null_al;
            struct cdd_enum_info ei_null_al;
            const struct cdd_enum_variant_info *found_ev = NULL;
            struct cdd_param_info fp_null_name;
            struct cdd_func_info fi_zero;
            const struct cdd_param_info *found_fp = NULL;
            memset(&ev_null_al, 0, sizeof(ev_null_al));
            memset(&ei_null_al, 0, sizeof(ei_null_al));
            ev_null_al.name = "V";
            ev_null_al.wire_name = "v";
            ev_null_al.aliases = null_al;
            ev_null_al.n_aliases = 2;
            ei_null_al.name = "ENullAl";
            ei_null_al.variants = &ev_null_al;
            ei_null_al.n_variants = 1;
            ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_find_enum_variant(
                                         &ei_null_al, "valid_al", &found_ev));
            memset(&fp_null_name, 0, sizeof(fp_null_name));
            memset(&fi_zero, 0, sizeof(fi_zero));
            fi_zero.name = "f_zero";
            fi_zero.params = &fp_null_name;
            fi_zero.n_params = 1;
            ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                      cdd_reflect_find_param(&fi_zero, "x", &found_fp));
            fi_zero.n_params = 0;
            ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                      cdd_reflect_invoke_func(&fi_zero, NULL, NULL));
          }
          {
            unsigned int bw_out = 0;
            FILE *fp_ref = cdd_test_tmpfile_global();
            struct StructFields sf_ref;
            struct FuncSignature fsig;
            struct FuncParamReflection fparams[8];
            struct CodegenStructConfig cfg_empty;
            char fn_name_buf[] = "fn_void_zero";
            int k_io;
            ASSERT(fp_ref != NULL);
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_struct_eval_bitwidth_str(NULL, &bw_out));
            ASSERT_EQ(CDD_C_SUCCESS, cdd_struct_eval_bitwidth_str("", &bw_out));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_struct_eval_bitwidth_str("/", &bw_out));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_struct_eval_bitwidth_str(":", &bw_out));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_struct_eval_bitwidth_str("(4", &bw_out));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_struct_eval_bitwidth_str("'", &bw_out));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_struct_eval_bitwidth_str("'a", &bw_out));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_struct_eval_bitwidth_str("0x1aF9uUlL", &bw_out));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_struct_eval_bitwidth_str("0B10", &bw_out));
            ASSERT_EQ(CDD_C_SUCCESS, cdd_struct_eval_bitwidth_str(
                                         "8 / 0 + 8 % 0 + 5 % 3", &bw_out));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_struct_eval_bitwidth_str("4 < 1", &bw_out));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_struct_eval_bitwidth_str("4 > 1", &bw_out));

            ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_ref));
            ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf_ref, "f1", "string",
                                                       NULL, NULL, NULL));
            sf_ref.fields[0].n_serde_aliases = 1;
            sf_ref.fields[0].serde_aliases = NULL;
            ASSERT_EQ(CDD_C_SUCCESS, write_struct_reflect_func(
                                         fp_ref, "RefEdge", &sf_ref, NULL));
            sf_ref.fields[0].n_serde_aliases = 0;
            g_cdd_last_alloc_err = struct_fields_free(&sf_ref);

            memset(&cfg_empty, 0, sizeof(cfg_empty));
            memset(&fsig, 0, sizeof(fsig));
            memset(fparams, 0, sizeof(fparams));
            fsig.name = fn_name_buf;
            ASSERT_EQ(CDD_C_SUCCESS,
                      write_func_reflect_func(fp_ref, &fsig, NULL));
            CDD_STRCPY(fsig.return_type, sizeof(fsig.return_type), "void");
            for (k_io = 0; k_io < 20; ++k_io) {
              cdd_c_error_t rc_io;
              g_fail_io_after = k_io;
              g_io_calls = 0;
              rc_io = write_func_reflect_func(fp_ref, &fsig, &cfg_empty);
              g_fail_io_after = -1;
              if (rc_io == CDD_C_SUCCESS)
                break;
            }
            fsig.is_variadic = 1;
            CDD_STRCPY(fparams[0].name, sizeof(fparams[0].name), "p0");
            CDD_STRCPY(fparams[0].type, sizeof(fparams[0].type), "char");
            CDD_STRCPY(fparams[1].name, sizeof(fparams[1].name), "p1");
            CDD_STRCPY(fparams[1].type, sizeof(fparams[1].type), "float");
            CDD_STRCPY(fparams[2].name, sizeof(fparams[2].name), "p2");
            CDD_STRCPY(fparams[2].type, sizeof(fparams[2].type), "bool");
            CDD_STRCPY(fparams[3].name, sizeof(fparams[3].name), "p3");
            CDD_STRCPY(fparams[3].type, sizeof(fparams[3].type), "union U");
            CDD_STRCPY(fparams[4].name, sizeof(fparams[4].name), "p4");
            CDD_STRCPY(fparams[4].type, sizeof(fparams[4].type), "long");
            CDD_STRCPY(fparams[5].name, sizeof(fparams[5].name), "p5");
            CDD_STRCPY(fparams[5].type, sizeof(fparams[5].type), "short");
            CDD_STRCPY(fparams[6].name, sizeof(fparams[6].name), "p6");
            CDD_STRCPY(fparams[6].type, sizeof(fparams[6].type), "size_t");
            CDD_STRCPY(fparams[7].name, sizeof(fparams[7].name), "p7");
            CDD_STRCPY(fparams[7].type, sizeof(fparams[7].type),
                       "const int **");
            fsig.params = fparams;
            fsig.n_params = 8;
            ASSERT_EQ(CDD_C_SUCCESS,
                      write_func_reflect_func(fp_ref, &fsig, &cfg_empty));
            fclose(fp_ref);
          }
          {
            struct LifeEdge {
              int *ptr_int;
              int enum_v;
              char fix_str[8];
              void *unreg_ptr_obj;
              int unreg_val_obj;
              int flex_arr[2];
              int fix_int_arr[2];
              char *fix_str_arr[2];
              void **obj_ptr_arr;
              size_t n_obj_ptr_arr;
              const char *str_p;
              int *dyn_arr;
              size_t n_dyn_arr;
              int inline_obj;
              int scalar_i;
              struct AdvChild fix_obj_arr[1];
            };
            struct cdd_field_info lfields[14];
            struct cdd_type_info lti;
            struct cdd_type_info uti;
            struct cdd_field_info ufield;
            struct LifeEdge le1, le2;
            void *l_out = NULL;
            void *child_null_arr[1] = {NULL};
            const char *s_arr1[2] = {NULL, "a"};
            const char *s_arr2[2] = {"a", NULL};
            int eq_res = 0;
            int u1[2] = {99, 10};
            int u2[2] = {99, 10};
            memset(lfields, 0, sizeof(lfields));
            memset(&lti, 0, sizeof(lti));
            memset(&le1, 0, sizeof(le1));
            memset(&le2, 0, sizeof(le2));

            lfields[0].name = "ptr_int";
            lfields[0].kind = CDD_FIELD_KIND_INTEGER;
            lfields[0].is_pointer = 1;
            lfields[0].default_val = "1";
            lfields[0].offset = offsetof(struct LifeEdge, ptr_int);
            lfields[0].size = sizeof(int *);

            lfields[1].name = "enum_v";
            lfields[1].kind = CDD_FIELD_KIND_ENUM;
            lfields[1].ref_name = NULL;
            lfields[1].default_val = "-2";
            lfields[1].offset = offsetof(struct LifeEdge, enum_v);
            lfields[1].size = sizeof(int);

            lfields[2].name = "fix_str";
            lfields[2].kind = CDD_FIELD_KIND_STRING;
            lfields[2].is_pointer = 0;
            lfields[2].default_val = "";
            lfields[2].fixed_array_len = 8;
            lfields[2].offset = offsetof(struct LifeEdge, fix_str);
            lfields[2].size = 8;

            lfields[3].name = "unreg_ptr_obj";
            lfields[3].kind = CDD_FIELD_KIND_OBJECT;
            lfields[3].is_pointer = 1;
            lfields[3].ref_name = NULL;
            lfields[3].offset = offsetof(struct LifeEdge, unreg_ptr_obj);
            lfields[3].size = sizeof(void *);

            lfields[4].name = "unreg_val_obj";
            lfields[4].kind = CDD_FIELD_KIND_OBJECT;
            lfields[4].is_pointer = 0;
            lfields[4].ref_name = "";
            lfields[4].offset = offsetof(struct LifeEdge, unreg_val_obj);
            lfields[4].size = sizeof(int);

            lfields[5].name = "flex_arr";
            lfields[5].kind = CDD_FIELD_KIND_ARRAY;
            lfields[5].is_pointer = 0;
            lfields[5].is_flexible_array = 1;
            lfields[5].elem_kind = CDD_FIELD_KIND_INTEGER;
            lfields[5].elem_size = sizeof(int);
            lfields[5].offset = offsetof(struct LifeEdge, flex_arr);

            lfields[6].name = "fix_int_arr";
            lfields[6].kind = CDD_FIELD_KIND_ARRAY;
            lfields[6].fixed_array_len = 2;
            lfields[6].elem_kind = CDD_FIELD_KIND_INTEGER;
            lfields[6].elem_size = sizeof(int);
            lfields[6].offset = offsetof(struct LifeEdge, fix_int_arr);
            lfields[6].size = sizeof(le1.fix_int_arr);

            lfields[7].name = "obj_ptr_arr";
            lfields[7].kind = CDD_FIELD_KIND_ARRAY;
            lfields[7].is_pointer = 1;
            lfields[7].elem_kind = CDD_FIELD_KIND_OBJECT;
            lfields[7].elem_size = sizeof(void *);
            lfields[7].ref_name = "AdvChild";
            lfields[7].offset = offsetof(struct LifeEdge, obj_ptr_arr);
            lfields[7].array_len_offset =
                offsetof(struct LifeEdge, n_obj_ptr_arr);
            lfields[7].size = sizeof(void *);

            lfields[8].name = "str_p";
            lfields[8].kind = CDD_FIELD_KIND_STRING;
            lfields[8].is_pointer = 1;
            lfields[8].offset = offsetof(struct LifeEdge, str_p);
            lfields[8].size = sizeof(char *);

            lfields[9].name = "dyn_arr";
            lfields[9].kind = CDD_FIELD_KIND_ARRAY;
            lfields[9].is_pointer = 1;
            lfields[9].elem_kind = CDD_FIELD_KIND_STRING;
            lfields[9].elem_size = sizeof(char *);
            lfields[9].offset = offsetof(struct LifeEdge, dyn_arr);
            lfields[9].array_len_offset = offsetof(struct LifeEdge, n_dyn_arr);
            lfields[9].size = sizeof(int *);

            lfields[10].name = "fix_str_arr";
            lfields[10].kind = CDD_FIELD_KIND_ARRAY;
            lfields[10].fixed_array_len = 2;
            lfields[10].elem_kind = CDD_FIELD_KIND_STRING;
            lfields[10].elem_size = sizeof(char *);
            lfields[10].offset = offsetof(struct LifeEdge, fix_str_arr);
            lfields[10].size = sizeof(le1.fix_str_arr);

            lfields[11].name = "inline_obj";
            lfields[11].kind = CDD_FIELD_KIND_OBJECT;
            lfields[11].is_pointer = 0;
            lfields[11].ref_name = "";
            lfields[11].offset = offsetof(struct LifeEdge, inline_obj);
            lfields[11].size = sizeof(int);

            lfields[12].name = "scalar_i";
            lfields[12].kind = CDD_FIELD_KIND_INTEGER;
            lfields[12].offset = offsetof(struct LifeEdge, scalar_i);
            lfields[12].size = sizeof(int);

            lfields[13].name = "fix_obj_arr";
            lfields[13].kind = CDD_FIELD_KIND_ARRAY;
            lfields[13].fixed_array_len = 1;
            lfields[13].elem_kind = CDD_FIELD_KIND_OBJECT;
            lfields[13].elem_size = sizeof(struct AdvChild);
            lfields[13].ref_name = "AdvChild";
            lfields[13].offset = offsetof(struct LifeEdge, fix_obj_arr);
            lfields[13].size = sizeof(le1.fix_obj_arr);

            lti.name = "LifeEdge";
            lti.size = 0;
            lti.fields = lfields;
            lti.n_fields = 14;
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_struct_default(&lti, NULL));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_struct_default(&lti, &l_out));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_struct_deepcopy(&le1, NULL, &lti));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_struct_deepcopy(&le1, &l_out, NULL));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_struct_deepcopy(&le1, &l_out, &lti));
            lti.size = sizeof(struct LifeEdge);
            ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_default(&lti, &l_out));
            lfields[1].ref_name = "";
            C_CDD_FREE(l_out);
            ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_default(&lti, &l_out));
            ((struct LifeEdge *)l_out)->unreg_ptr_obj = C_CDD_MALLOC(4);
            ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_cleanup(l_out, &lti));
            C_CDD_FREE(l_out);

            le1.unreg_ptr_obj = NULL;
            le1.obj_ptr_arr = child_null_arr;
            le1.n_obj_ptr_arr = 1;
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_struct_deepcopy(&le1, &l_out, &lti));
            ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_cleanup(l_out, &lti));
            C_CDD_FREE(l_out);

            le1.unreg_ptr_obj = &eq_res;
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_struct_deepcopy(&le1, &l_out, &lti));
            ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_cleanup(l_out, &lti));
            C_CDD_FREE(l_out);

            lfields[3].ref_name = "MissingType";
            ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                      cdd_reflect_struct_deepcopy(&le1, &l_out, &lti));
            lfields[3].ref_name = NULL;

            le1.str_p = "hello";
            le2.str_p = NULL;
            ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_eq_single_field(
                                         &le1, &le2, &lfields[8], &eq_res));
            ASSERT_EQ(0, eq_res);
            ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_eq_single_field(
                                         &le2, &le1, &lfields[8], &eq_res));
            ASSERT_EQ(0, eq_res);

            le1.unreg_ptr_obj = NULL;
            le2.unreg_ptr_obj = NULL;
            ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_eq_single_field(
                                         &le1, &le2, &lfields[3], &eq_res));
            ASSERT_EQ(1, eq_res);
            le1.unreg_ptr_obj = &eq_res;
            le2.unreg_ptr_obj = NULL;
            ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_eq_single_field(
                                         &le1, &le2, &lfields[3], &eq_res));
            ASSERT_EQ(0, eq_res);
            ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_eq_single_field(
                                         &le2, &le1, &lfields[3], &eq_res));
            ASSERT_EQ(0, eq_res);

            *(const char ***)&le1.dyn_arr = s_arr1;
            le1.n_dyn_arr = 2;
            *(const char ***)&le2.dyn_arr = s_arr2;
            le2.n_dyn_arr = 2;
            ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_eq_single_field(
                                         &le1, &le2, &lfields[9], &eq_res));
            ASSERT_EQ(0, eq_res);
            s_arr1[0] = "a";
            ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_eq_single_field(
                                         &le1, &le2, &lfields[9], &eq_res));
            ASSERT_EQ(0, eq_res);

            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_struct_eq(&le1, &le2, &lti, NULL));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_struct_eq(&le1, &le1, &lti, &eq_res));
            ASSERT_EQ(1, eq_res);
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_struct_eq(NULL, &le2, &lti, &eq_res));
            ASSERT_EQ(0, eq_res);
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_struct_eq(&le1, NULL, &lti, &eq_res));
            ASSERT_EQ(0, eq_res);

            memset(&ufield, 0, sizeof(ufield));
            memset(&uti, 0, sizeof(uti));
            ufield.name = "val";
            ufield.kind = CDD_FIELD_KIND_INTEGER;
            ufield.offset = sizeof(int);
            ufield.size = sizeof(int);
            uti.name = "UEdge";
            uti.size = sizeof(u1);
            uti.is_union = 1;
            uti.tag_offset = 0;
            uti.fields = &ufield;
            uti.n_fields = 1;
            ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_cleanup(u1, &uti));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_struct_deepcopy(u1, &l_out, &uti));
            C_CDD_FREE(l_out);
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_struct_eq(u1, u2, &uti, &eq_res));
            u1[0] = 1;
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_struct_deepcopy(u1, &l_out, &uti));
            C_CDD_FREE(l_out);
            u1[0] = 99;
            {
              int v_cnt = 0;
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_reflect_visit_fields(
                            u1, &uti, test_reflect_visitor_cb, &v_cnt));
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_reflect_visit_fields_mut(
                            u1, &uti, test_reflect_mut_visitor_cb, &v_cnt));
              ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                        cdd_reflect_visit_deep_mut(
                            u1, NULL, test_reflect_mut_visitor_cb, &v_cnt));
              ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                        cdd_reflect_visit_deep_mut(u1, &uti, NULL, &v_cnt));
            }
            {
              char *u_json = NULL;
              void *u_obj = NULL;
              ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_to_json(u1, &uti, &u_json));
              C_CDD_FREE(u_json);
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_reflect_from_json("{}", &uti, &u_obj));
              C_CDD_FREE(u_obj);
              ASSERT_EQ(
                  CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_from_json("{\"val\":\"bad\"}", &uti, &u_obj));
            }
          }

          /* 12. Remaining branch coverage for struct.c, struct_reflect_emit.c,
           * struct_reflect_registry.c, struct_reflect_access.c,
           * struct_reflect_validate.c, and struct_reflect_json.c */
          {
            unsigned int bw_out = 0;
            FILE *fp_extra = cdd_test_tmpfile_global();
            struct StructFields sf_ex;
            struct FuncSignature fsig_ex;
            struct FuncParamReflection fparam_ex;
            char fn_ex_name[] = "fn_out_const";
            size_t sc_w = 0;
            int sc_u = 0;
            struct cdd_field_info f_sc;
            struct cdd_field_info bf_tmp_u32;
            struct cdd_field_info bf_tmp_s4;
            int out_i = 0;
            int64_t out_i64 = 0;

            ASSERT(fp_extra != NULL);
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_struct_eval_bitwidth_str("\t4", &bw_out));
            ASSERT_EQ(4U, bw_out);

            ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_ex));
            ASSERT_EQ(CDD_C_SUCCESS,
                      struct_fields_add(&sf_ex, "fix_children", "array",
                                        "#/components/schemas/Child", NULL,
                                        NULL));
            sf_ex.fields[0].fixed_array_len = 2;
            ASSERT_EQ(CDD_C_SUCCESS,
                      struct_fields_add(&sf_ex, "flex_s", "string", NULL, NULL,
                                        NULL));
            sf_ex.fields[1].is_flexible_array = 1;
            ASSERT_EQ(CDD_C_SUCCESS,
                      struct_fields_add(&sf_ex, "ptr_sub", "object", "Sub",
                                        NULL, NULL));
            CDD_STRCPY(sf_ex.fields[2].c_type, sizeof(sf_ex.fields[2].c_type),
                       "struct Sub *");
            ASSERT_EQ(CDD_C_SUCCESS, write_struct_deepcopy_func(fp_extra, "ExS",
                                                                &sf_ex, NULL));
            ASSERT_EQ(CDD_C_SUCCESS,
                      write_struct_reflect_func(fp_extra, "ExS", &sf_ex, NULL));
            g_cdd_last_alloc_err = struct_fields_free(&sf_ex);

            memset(&fsig_ex, 0, sizeof(fsig_ex));
            memset(&fparam_ex, 0, sizeof(fparam_ex));
            fsig_ex.name = fn_ex_name;
            CDD_STRCPY(fsig_ex.return_type, sizeof(fsig_ex.return_type), "int");
            CDD_STRCPY(fparam_ex.name, sizeof(fparam_ex.name), "out_val");
            CDD_STRCPY(fparam_ex.type, sizeof(fparam_ex.type), "const char *");
            fsig_ex.params = &fparam_ex;
            fsig_ex.n_params = 1;
            ASSERT_EQ(CDD_C_SUCCESS,
                      write_func_reflect_func(fp_extra, &fsig_ex, NULL));
            fclose(fp_extra);

            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_resolve_path(p_def, &parent_ti, ".i64",
                                               &res_f, &res_parent, &res_ptr));

            memset(&f_sc, 0, sizeof(f_sc));
            f_sc.is_pointer = 1;
            f_sc.c_type = "char *";
            ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_resolve_field_scalar_width(
                                         &f_sc, &sc_w, &sc_u));
            ASSERT_EQ(1U, sc_w);
            f_sc.c_type = "short *";
            ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_resolve_field_scalar_width(
                                         &f_sc, &sc_w, &sc_u));
            ASSERT_EQ(2U, sc_w);

            p_def->i64 = (int64_t)INT_MIN - 100;
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_get_int(p_def, &parent_fields[0], &out_i));
            p_def->i64 = 10;

            /* Unsigned 4-bit bitfield in get_int64, 32-bit bitfield bounds */
            memset(&bf_tmp_s4, 0, sizeof(bf_tmp_s4));
            bf_tmp_s4.name = "bf_s4";
            bf_tmp_s4.c_type = "unsigned int";
            bf_tmp_s4.kind = CDD_FIELD_KIND_INTEGER;
            bf_tmp_s4.bit_width = 4;
            bf_tmp_s4.get_bitfield = test_bf_get;
            bf_tmp_s4.set_bitfield = test_bf_set;
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_get_int64(p_def, &bf_tmp_s4, &out_i64));

            memset(&bf_tmp_u32, 0, sizeof(bf_tmp_u32));
            bf_tmp_u32.name = "bf_u32";
            bf_tmp_u32.c_type = "unsigned int";
            bf_tmp_u32.kind = CDD_FIELD_KIND_INTEGER;
            bf_tmp_u32.bit_width = 32;
            bf_tmp_u32.get_bitfield = test_bf_get;
            bf_tmp_u32.set_bitfield = test_bf_set;
            ASSERT_EQ(
                CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_reflect_set_int64(p_def, &bf_tmp_u32, ((int64_t)1 << 33)));
            bf_tmp_u32.c_type = "int";
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_set_int64(p_def, &bf_tmp_u32,
                                            (int64_t)INT_MIN - 10));

            /* Narrow integer negative bounds in set_int64 */
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_set_int64(p_def, &parent_fields[3], -1));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_set_int64(p_def, &parent_fields[2], -129));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_set_int64(p_def, &parent_fields[5], -1));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_set_int64(p_def, &parent_fields[4], -40000));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_set_int64(p_def, &parent_fields[6],
                                            ((int64_t)1 << 33)));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_set_uint64(p_def, NULL, 1U));

            /* Array resize elem_size == 0 & unregistered pointer object shrink
             */
            parent_fields[13].elem_size = 0;
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_array_resize(p_def, &parent_fields[13], 1));
            parent_fields[13].elem_size = sizeof(int);

            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_array_resize(p_def, &parent_fields[19], 1));
            p_def->ptr_obj_arr[0] =
                (struct AdvChild *)C_CDD_CALLOC(1, sizeof(struct AdvChild));
            parent_fields[19].ref_name = "MissingType";
            {
              int dv_cnt = 0;
              void *saved_pchild = p_def->ptr_child;
              p_def->ptr_child = NULL;
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_reflect_visit_deep_mut(p_def, &parent_ti,
                                                   test_reflect_mut_visitor_cb,
                                                   &dv_cnt));
              p_def->ptr_child = (struct AdvChild *)saved_pchild;
              parent_fields[11].ref_name = "MissingType";
              ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_visit_deep(
                                           p_def, &parent_ti,
                                           test_reflect_visitor_cb, &dv_cnt));
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_reflect_visit_deep_mut(p_def, &parent_ti,
                                                   test_reflect_mut_visitor_cb,
                                                   &dv_cnt));
              ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_validate(p_def, &parent_ti));
              parent_fields[11].ref_name = "AdvChild";
            }
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_array_resize(p_def, &parent_fields[19], 0));
            parent_fields[19].ref_name = "AdvChild";

            /* NULL element in ptr_obj_arr during visit_deep and visit_deep_mut,
             * and visitor failure inside object array */
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_array_resize(p_def, &parent_fields[19], 2));
            p_def->ptr_obj_arr[0] = NULL;
            p_def->ptr_obj_arr[1] =
                (struct AdvChild *)C_CDD_CALLOC(1, sizeof(struct AdvChild));
            {
              int dv_cnt = 0;
              ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_visit_deep(
                                           p_def, &parent_ti,
                                           test_reflect_visitor_cb, &dv_cnt));
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_reflect_visit_deep_mut(p_def, &parent_ti,
                                                   test_reflect_mut_visitor_cb,
                                                   &dv_cnt));
              dv_cnt = -22;
              ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                        cdd_reflect_visit_deep_mut(p_def, &parent_ti,
                                                   test_reflect_mut_visitor_cb,
                                                   &dv_cnt));
            }
            /* Failing child element inside ptr_obj_arr during validate */
            child_fields[0].has_min = 1;
            child_fields[0].min_val = 100.0;
            p_def->ptr_obj_arr[1]->id = 5;
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate(p_def, &parent_ti));
            child_fields[0].has_min = 0;
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_array_resize(p_def, &parent_fields[19], 0));

            /* Regex & format validation edge cases */
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_set_string(p_def, &parent_fields[10], "a"));
            parent_fields[10].pattern = "^[]a]$";
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_set_string(p_def, &parent_fields[10], "_"));
            parent_fields[10].pattern = "^[\\w]$";
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_set_string(p_def, &parent_fields[10], "-"));
            parent_fields[10].pattern = "^[\\-]$";
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));
            parent_fields[10].pattern = "(a)?[b";
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_set_string(p_def, &parent_fields[10], "a"));
            parent_fields[10].pattern = "(a)+[b";
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_set_string(p_def, &parent_fields[10], "a\\"));
            parent_fields[10].pattern = "a\\";
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));
            parent_fields[10].pattern = "a{x}|b";
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));
            parent_fields[10].pattern = NULL;

            parent_fields[10].format = "email";
            ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_string(
                                         p_def, &parent_fields[10], "@b.com"));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));
            ASSERT_EQ(
                CDD_C_SUCCESS,
                cdd_reflect_set_string(p_def, &parent_fields[10], "a@b@c.com"));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));
            ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_string(
                                         p_def, &parent_fields[10], "a@.com"));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));
            ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_string(
                                         p_def, &parent_fields[10], "a@b."));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));

            parent_fields[10].format = "date";
            ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_string(
                                         p_def, &parent_fields[10], "2026"));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_set_string(p_def, &parent_fields[10],
                                             "2026X01-01"));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_set_string(p_def, &parent_fields[10],
                                             "2026-01X01"));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_set_string(p_def, &parent_fields[10],
                                             "2026-00-01"));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_set_string(p_def, &parent_fields[10],
                                             "2026-01-00"));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_set_string(p_def, &parent_fields[10],
                                             "2026-01-32"));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));

            parent_fields[10].format = "date-time";
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_set_string(p_def, &parent_fields[10],
                                             "2026-01-01T12:00"));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_set_string(p_def, &parent_fields[10],
                                             "2026-01-01T12X00:00Z"));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_set_string(p_def, &parent_fields[10],
                                             "2026-01-01T12:00X00Z"));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_set_string(p_def, &parent_fields[10],
                                             "2026-01-01T12:60:00Z"));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_set_string(p_def, &parent_fields[10],
                                             "2026-01-01T12:00:61Z"));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));

            parent_fields[10].format = "ipv4";
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_set_string(p_def, &parent_fields[10],
                                             "0001.1.1.1"));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));
            ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_string(
                                         p_def, &parent_fields[10], "1.2.3"));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));

            parent_fields[10].format = "ipv6";
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_set_string(p_def, &parent_fields[10], ":"));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));

            parent_fields[10].format = "uri";
            ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_string(
                                         p_def, &parent_fields[10], "nocolon"));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));
            ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_string(
                                         p_def, &parent_fields[10], "://foo"));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_set_string(p_def, &parent_fields[10],
                                             "1http://foo"));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));

            parent_fields[10].format = "hostname";
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_set_string(p_def, &parent_fields[10], ""));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_set_string(p_def, &parent_fields[10],
                                             "my-host.example.com"));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_validate_field(p_def, &parent_fields[10]));
            parent_fields[10].format = NULL;

            /* Integer format & number max-only & enum MissingEnum validation */
            parent_fields[0].format = "int32";
            p_def->i64 = 10;
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_validate_field(p_def, &parent_fields[0]));
            p_def->i64 = (int64_t)INT_MIN - 10;
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[0]));
            parent_fields[0].format = "uint32";
            p_def->i64 = 10;
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_validate_field(p_def, &parent_fields[0]));
            parent_fields[0].format = "uint64";
            p_def->i64 = -1;
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[0]));
            p_def->i64 = 10;
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_validate_field(p_def, &parent_fields[0]));
            parent_fields[0].format = NULL;

            parent_fields[9].ref_name = "MissingEnum";
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_validate_field(p_def, &parent_fields[9]));
            parent_fields[9].ref_name = "AdvEnum";

            parent_fields[7].has_min = 0;
            parent_fields[7].has_max = 1;
            parent_fields[7].max_val = 100.0;
            p_def->num = 5.0;
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_validate_field(p_def, &parent_fields[7]));
            parent_fields[7].has_max = 0;

            /* Unique items on str_arr, ptr_obj_arr, and 1-element int_arr */
            parent_fields[17].unique_items = 1;
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_array_resize(p_def, &parent_fields[17], 0));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_array_resize(p_def, &parent_fields[17], 2));
            ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                      cdd_reflect_validate_field(p_def, &parent_fields[17]));
            g_cdd_last_alloc_err = c_cdd_strdup("s2", &p_def->str_arr[1]);
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_validate_field(p_def, &parent_fields[17]));
            C_CDD_FREE(p_def->str_arr[1]);
            p_def->str_arr[1] = NULL;
            g_cdd_last_alloc_err = c_cdd_strdup("s1", &p_def->str_arr[0]);
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_validate_field(p_def, &parent_fields[17]));
            g_cdd_last_alloc_err = c_cdd_strdup("s2", &p_def->str_arr[1]);
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_validate_field(p_def, &parent_fields[17]));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_array_resize(p_def, &parent_fields[17], 0));
            parent_fields[17].unique_items = 0;

            parent_fields[19].unique_items = 1;
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_array_resize(p_def, &parent_fields[19], 2));
            p_def->ptr_obj_arr[0] =
                (struct AdvChild *)C_CDD_CALLOC(1, sizeof(struct AdvChild));
            p_def->ptr_obj_arr[1] =
                (struct AdvChild *)C_CDD_CALLOC(1, sizeof(struct AdvChild));
            p_def->ptr_obj_arr[0]->id = 1;
            p_def->ptr_obj_arr[1]->id = 2;
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_validate_field(p_def, &parent_fields[19]));
            parent_fields[19].ref_name = "MissingType";
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_validate_field(p_def, &parent_fields[19]));
            parent_fields[19].ref_name = "AdvChild";
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_array_resize(p_def, &parent_fields[19], 0));
            parent_fields[19].unique_items = 0;

            parent_fields[13].unique_items = 1;
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_array_resize(p_def, &parent_fields[13], 1));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_reflect_validate_field(p_def, &parent_fields[13]));
            parent_fields[13].unique_items = 0;

            /* JSON, Merge Patch, Schema, Form, CLI, and JSON Patch edge cases
             */
            {
              char *j_str = NULL;
              void *j_obj = NULL;
              int dummy_obj = 0;
              ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                        cdd_reflect_from_json_object(NULL, &parent_ti, p_def));
              ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                        cdd_reflect_from_json_object(&dummy_obj, NULL, p_def));
              ASSERT_EQ(
                  CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_from_json_object(&dummy_obj, &parent_ti, NULL));
              ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                        cdd_reflect_to_json(p_def, NULL, &j_str));
              ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                        cdd_reflect_to_json(p_def, &parent_ti, NULL));
              ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                        cdd_reflect_from_json("{}", NULL, &j_obj));
              ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                        cdd_reflect_from_json("{}", &parent_ti, NULL));
              ASSERT_EQ(
                  CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_reflect_merge_patch(p_def, NULL, &parent_ti, &j_str));
              ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                        cdd_reflect_merge_patch(p_def, &p1, NULL, &j_str));
              ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                        cdd_reflect_merge_patch(p_def, &p1, &parent_ti, NULL));
              ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                        cdd_reflect_apply_merge_patch(p_def, NULL, "{}"));
              ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                        cdd_reflect_apply_merge_patch(p_def, &parent_ti, NULL));

              /* wire_name == NULL, skip_serializing, skip_deserializing,
               * enum wire_name == NULL and MissingEnum in to_json & schema */
              parent_fields[1].skip_serializing = 1;
              parent_fields[1].skip_deserializing = 1;
              ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_apply_merge_patch(
                                           p_def, &parent_ti, "{\"u64\":1}"));
              parent_fields[0].wire_name = NULL;
              enum_vars[0].wire_name = NULL;
              enum_vars[1].skip = 1;
              p_def->eval = 10;
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_reflect_array_resize(p_def, &parent_fields[16], 1));
              p_def->enum_arr[0] = 10;
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_reflect_to_json(p_def, &parent_ti, &j_str));
              C_CDD_FREE(j_str);

              parent_fields[9].ref_name = "MissingEnum";
              parent_fields[16].ref_name = "MissingEnum";
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_reflect_to_json(p_def, &parent_ti, &j_str));
              C_CDD_FREE(j_str);

              parent_fields[0].required = 1;
              parent_fields[1].required = 1;
              parent_fields[7].has_min = 1;
              parent_fields[7].exclusive_min = 1;
              parent_fields[7].has_max = 1;
              parent_fields[7].exclusive_max = 1;
              parent_fields[10].has_min_len = 0;
              parent_fields[10].has_max_len = 0;
              parent_fields[10].pattern = NULL;
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_reflect_to_json_schema(&parent_ti, &j_str));
              C_CDD_FREE(j_str);
              parent_fields[7].exclusive_min = 0;
              parent_fields[7].exclusive_max = 0;
              parent_fields[9].ref_name = "AdvEnum";
              parent_fields[16].ref_name = "AdvEnum";
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_reflect_to_json_schema(&parent_ti, &j_str));
              C_CDD_FREE(j_str);
              parent_fields[0].required = 0;
              parent_fields[1].required = 0;

              enum_vars[0].wire_name = "alpha";
              enum_vars[1].skip = 0;
              parent_fields[7].has_min = 0;
              parent_fields[7].has_max = 0;
              parent_fields[1].skip_serializing = 0;
              parent_fields[1].skip_deserializing = 0;

              /* Schema with no required fields */
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_reflect_to_json_schema(&child_ti, &j_str));
              C_CDD_FREE(j_str);

              /* Merge patch with wire_name == NULL, skip, skip_serializing */
              parent_fields[2].skip = 1;
              parent_fields[3].skip_serializing = 1;
              p_def->i64 = 999;
              p1.i64 = 111;
              ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_merge_patch(
                                           &p1, p_def, &parent_ti, &j_str));
              C_CDD_FREE(j_str);
              parent_fields[0].wire_name = "i64";
              parent_fields[2].skip = 0;
              parent_fields[3].skip_serializing = 0;

              /* apply_merge_patch with non-object for object field, and
               * multi-key where first key fails */
              ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                        cdd_reflect_apply_merge_patch(
                            p_def, &parent_ti, "{\"inline_child\":123}"));
              ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                        cdd_reflect_apply_merge_patch(
                            p_def, &parent_ti, "{\"i64\":\"bad\",\"u64\":2}"));

              /* deserialize_field_from_json edge cases:
               * - flattened pointer child when NULL and when already non-NULL
               * - required = 1 with default_val != NULL in from_json
               * - non-nullable optional pointer field with null in JSON
               * - non-nullable required pointer field with null in JSON
               * - opt_int and ptr_child already allocated before merge patch
               * - ptr_obj_arr with null when element is already NULL
               * - val_obj_arr with null element (invalid) */
              parent_fields[12].flatten = 1;
              ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_apply_merge_patch(
                                           p_def, &parent_ti, "{\"id\":88}"));
              ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_apply_merge_patch(
                                           p_def, &parent_ti, "{\"id\":89}"));
              parent_fields[12].flatten = 0;

              parent_fields[0].required = 1;
              parent_fields[0].default_val = "0";
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_reflect_from_json("{}", &parent_ti, &j_obj));
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_reflect_struct_cleanup(j_obj, &parent_ti));
              C_CDD_FREE(j_obj);
              parent_fields[0].required = 0;
              parent_fields[0].default_val = "42";

              parent_fields[21].required = 0;
              parent_fields[21].is_nullable = 0;
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_reflect_apply_merge_patch(p_def, &parent_ti,
                                                      "{\"opt_int\":null}"));
              parent_fields[21].required = 1;
              ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                        cdd_reflect_apply_merge_patch(p_def, &parent_ti,
                                                      "{\"opt_int\":null}"));
              parent_fields[21].required = 0;
              parent_fields[21].is_nullable = 1;

              p_def->opt_int = (int *)C_CDD_CALLOC(1, sizeof(int));
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_reflect_apply_merge_patch(
                            p_def, &parent_ti,
                            "{\"opt_int\":7,\"ptr_child\":{\"id\":9},"
                            "\"ptr_obj_arr\":[null]}"));
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_reflect_apply_merge_patch(
                            p_def, &parent_ti, "{\"ptr_child\":{\"id\":10}}"));
              ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                        cdd_reflect_apply_merge_patch(
                            p_def, &parent_ti, "{\"val_obj_arr\":[null]}"));

              /* Form URL-encoded & CLI edge cases */
              parent_fields[0].wire_name = NULL;
              parent_fields[2].skip = 1;
              parent_fields[3].skip_serializing = 1;
              parent_fields[10].is_pointer = 0;
              ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_to_form_urlencoded(
                                           p_def, &parent_ti, &j_str));
              C_CDD_FREE(j_str);
              parent_fields[0].wire_name = "i64";
              parent_fields[2].skip = 0;
              parent_fields[3].skip_serializing = 0;
              parent_fields[10].is_pointer = 1;

              ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_from_form_urlencoded(
                                           "", &parent_ti, &j_obj));
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_reflect_struct_cleanup(j_obj, &parent_ti));
              C_CDD_FREE(j_obj);

              parent_fields[2].skip = 1;
              parent_fields[3].skip_deserializing = 1;
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_reflect_from_form_urlencoded(
                            "barekey&i8=1&u8=1&bval=1&bval=false&trail",
                            &parent_ti, &j_obj));
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_reflect_struct_cleanup(j_obj, &parent_ti));
              C_CDD_FREE(j_obj);
              parent_fields[2].skip = 0;
              parent_fields[3].skip_deserializing = 0;

              ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                        cdd_reflect_from_form_urlencoded("i64=12x", &parent_ti,
                                                         &j_obj));
              ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                        cdd_reflect_from_form_urlencoded("num=1.5x", &parent_ti,
                                                         &j_obj));
              {
                char big_kv[450];
                memset(big_kv, 'k', 150);
                big_kv[150] = '=';
                memset(big_kv + 151, 'v', 280);
                big_kv[431] = '\0';
                ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_from_form_urlencoded(
                                             big_kv, &parent_ti, &j_obj));
                ASSERT_EQ(CDD_C_SUCCESS,
                          cdd_reflect_struct_cleanup(j_obj, &parent_ti));
                C_CDD_FREE(j_obj);
              }

              {
                const char *two_al[2] = {"lbl_alias", "lbl_alias2"};
                child_fields[2].aliases = two_al;
                child_fields[2].n_aliases = 2;
                ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_apply_merge_patch(
                                             &p_def->inline_child, &child_ti,
                                             "{\"lbl_alias\":\"hi\"}"));
                child_fields[2].aliases = child_aliases;
                child_fields[2].n_aliases = 1;
              }
              ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                        cdd_reflect_from_cli(1, NULL, &parent_ti, &j_obj));
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_reflect_from_cli(0, NULL, &parent_ti, &j_obj));
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_reflect_struct_cleanup(j_obj, &parent_ti));
              C_CDD_FREE(j_obj);
              {
                JSON_Value *jv_p = NULL;
                JSON_Object *jo_p = NULL;
                ASSERT_EQ(CDD_C_SUCCESS,
                          cdd_json_parse_string(
                              "{\"id\":77,\"ptr_child\":{\"id\":78}}", &jv_p));
                g_cdd_last_alloc_err = cdd_json_value_get_object(jv_p, &jo_p);
                parent_fields[12].flatten = 1;
                ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_from_json_object(
                                             jo_p, &parent_ti, p_def));
                parent_fields[12].flatten = 0;
                ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_from_json_object(
                                             jo_p, &parent_ti, p_def));
                g_cdd_last_alloc_err = cdd_json_value_free(jv_p);
              }
              {
                char big_cli[160];
                const char *cli_args[2];
                memset(big_cli, 'a', sizeof(big_cli) - 1);
                big_cli[0] = '-';
                big_cli[1] = '-';
                big_cli[140] = '=';
                big_cli[sizeof(big_cli) - 1] = '\0';
                cli_args[0] = NULL;
                cli_args[1] = big_cli;
                ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_from_cli(
                                             2, cli_args, &parent_ti, &j_obj));
                ASSERT_EQ(CDD_C_SUCCESS,
                          cdd_reflect_struct_cleanup(j_obj, &parent_ti));
                C_CDD_FREE(j_obj);
              }

              /* JSON Patch edge cases */
              ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                        cdd_reflect_apply_json_patch(p_def, &parent_ti, "bad"));
              ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                        cdd_reflect_apply_json_patch(p_def, &parent_ti, "{}"));
              ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                        cdd_reflect_apply_json_patch(p_def, &parent_ti,
                                                     "[{\"op\":\"add\"}]"));
              ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_string(
                                           p_def, &parent_fields[10], NULL));
              ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                        cdd_reflect_apply_json_patch(
                            p_def, &parent_ti,
                            "[{\"op\":\"test\",\"path\":\"/str~0name~11\","
                            "\"value\":\"ok_s\"}]"));
              ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                        cdd_reflect_apply_json_patch(
                            p_def, &parent_ti,
                            "[{\"op\":\"copy\",\"from\":\"/missing\","
                            "\"path\":\"/i16\"}]"));
              ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_string(
                                           p_def, &parent_fields[10], "ok_s"));
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_reflect_apply_json_patch(
                            p_def, &parent_ti,
                            "[{\"op\":\"test\",\"path\":\"/str~0name~11\","
                            "\"value\":\"ok_s\"},{\"op\":\"copy\",\"from\":"
                            "\"/i64\",\"path\":\"/i16\"}]"));
            }
          }
        }
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
