/**
 * @file test_codegen_struct_reflect_adv.h
 * @brief Unit tests for advanced reflection features: 64-bit bounds, format
 * and regex validation, uniqueItems, deep path accessors, table-driven
 * lifecycle, JSON serde, RFC 7396 JSON Merge Patch, and function invocation.
 *
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_STRUCT_REFLECT_ADV_H
#define TEST_CODEGEN_STRUCT_REFLECT_ADV_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_codegen_struct_common.h"
/* clang-format on */

/** @brief Test helper struct AdvChild. */
struct AdvChild {
  /** @brief Field id. */
  int id;
  /** @brief Field score. */
  double score;
  /** @brief Field label. */
  char *label;
  /** @brief Field flag. */
  int flag;
};

/** @brief Test helper struct AdvParent. */
struct AdvParent {
  /** @brief Field i64. */
  int64_t i64;
  /** @brief Field u64. */
  uint64_t u64;
  /** @brief Field i8. */
  int8_t i8;
  /** @brief Field u8. */
  uint8_t u8;
  /** @brief Field i16. */
  int16_t i16;
  /** @brief Field u16. */
  uint16_t u16;
  /** @brief Field u32. */
  uint32_t u32;
  /** @brief Field num. */
  double num;
  /** @brief Field bval. */
  int bval;
  /** @brief Field eval. */
  int eval;
  /** @brief Field str. */
  char *str;
  /** @brief Field inline_child. */
  struct AdvChild inline_child;
  /** @brief Field ptr_child. */
  struct AdvChild *ptr_child;
  /** @brief Field int_arr. */
  int *int_arr;
  /** @brief Field n_int_arr. */
  size_t n_int_arr;
  /** @brief Field dbl_arr. */
  double *dbl_arr;
  /** @brief Field n_dbl_arr. */
  size_t n_dbl_arr;
  /** @brief Field bool_arr. */
  int *bool_arr;
  /** @brief Field n_bool_arr. */
  size_t n_bool_arr;
  /** @brief Field enum_arr. */
  int *enum_arr;
  /** @brief Field n_enum_arr. */
  size_t n_enum_arr;
  /** @brief Field str_arr. */
  char **str_arr;
  /** @brief Field n_str_arr. */
  size_t n_str_arr;
  /** @brief Field val_obj_arr. */
  struct AdvChild *val_obj_arr;
  /** @brief Field n_val_obj_arr. */
  size_t n_val_obj_arr;
  /** @brief Field ptr_obj_arr. */
  struct AdvChild **ptr_obj_arr;
  /** @brief Field n_ptr_obj_arr. */
  size_t n_ptr_obj_arr;
  /** @brief Field fixed_ints. */
  int fixed_ints[2];
  /** @brief Field opt_int. */
  int *opt_int;
  /** @brief Field bf_u32. */
  unsigned int bf_u32 : 32;
  /** @brief Field bf_s4. */
  int bf_s4 : 4;
};

/** @brief Test helper union struct AdvUnion. */
struct AdvUnion {
  /** @brief Active union tag. */
  int tag;
  /** @brief Union payload data. */
  union AdvUnionData {
    /** @brief Integer branch. */
    int num_branch;
    /** @brief String branch. */
    char *str_branch;
  } /** @brief Union payload data. */ data;
};

/**
 * @brief Initializes the AdvChild, AdvParent, AdvUnion, AdvEnum, and func_ti
 * reflection descriptors and registers them in the global reflection registry.
 *
 * @param[out] child_fields Array of 4 child field descriptors.
 * @param[out] child_ti Child type descriptor.
 * @param[out] parent_fields Array of 22 parent field descriptors.
 * @param[out] parent_ti Parent type descriptor.
 * @param[out] union_fields Array of 2 union field descriptors.
 * @param[out] union_ti Union type descriptor.
 * @param[out] enum_vars Array of 2 enum variant descriptors.
 * @param[out] enum_ti Enum descriptor.
 * @param[out] func_params Array of 2 function parameter descriptors.
 * @param[out] func_ti Function descriptor.
 * @param[in] child_aliases Array of 1 child alias string.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t setup_adv_reflection_descriptors(
    struct cdd_field_info *child_fields, struct cdd_type_info *child_ti,
    struct cdd_field_info *parent_fields, struct cdd_type_info *parent_ti,
    struct cdd_field_info *union_fields, struct cdd_type_info *union_ti,
    struct cdd_enum_variant_info *enum_vars, struct cdd_enum_info *enum_ti,
    struct cdd_param_info *func_params, struct cdd_func_info *func_ti,
    const char *const *child_aliases) {
  cdd_c_error_t rc;
  struct AdvParent dummy_p;

  memset(child_fields, 0, 4 * sizeof(struct cdd_field_info));
  memset(child_ti, 0, sizeof(*child_ti));
  memset(parent_fields, 0, 22 * sizeof(struct cdd_field_info));
  memset(parent_ti, 0, sizeof(*parent_ti));
  memset(union_fields, 0, 2 * sizeof(struct cdd_field_info));
  memset(union_ti, 0, sizeof(*union_ti));
  memset(enum_vars, 0, 2 * sizeof(struct cdd_enum_variant_info));
  memset(enum_ti, 0, sizeof(*enum_ti));
  memset(func_params, 0, 2 * sizeof(struct cdd_param_info));
  memset(func_ti, 0, sizeof(*func_ti));
  memset(&dummy_p, 0, sizeof(dummy_p));

  rc = cdd_reflect_clear_registry();
  if (rc != CDD_C_SUCCESS)
    return rc;

  enum_vars[0].name = "ALPHA";
  enum_vars[0].wire_name = "alpha";
  enum_vars[0].value = 10;
  enum_vars[1].name = "BETA";
  enum_vars[1].wire_name = "beta";
  enum_vars[1].value = 20;
  enum_ti->name = "AdvEnum";
  enum_ti->variants = enum_vars;
  enum_ti->n_variants = 2;
  enum_ti->other_idx = -1;
  rc = cdd_reflect_register_enum(enum_ti);
  if (rc != CDD_C_SUCCESS)
    return rc;

  child_fields[0].name = "id";
  child_fields[0].wire_name = "id";
  child_fields[0].c_type = "int";
  child_fields[0].kind = CDD_FIELD_KIND_INTEGER;
  child_fields[0].offset = offsetof(struct AdvChild, id);
  child_fields[0].size = sizeof(int);
  child_fields[0].default_val = "0b101";

  child_fields[1].name = "score";
  child_fields[1].wire_name = "score";
  child_fields[1].c_type = "double";
  child_fields[1].kind = CDD_FIELD_KIND_NUMBER;
  child_fields[1].offset = offsetof(struct AdvChild, score);
  child_fields[1].size = sizeof(double);
  child_fields[1].default_val = "2.5";

  child_fields[2].name = "label";
  child_fields[2].wire_name = "label";
  child_fields[2].de_wire_name = "label";
  child_fields[2].c_type = "char *";
  child_fields[2].kind = CDD_FIELD_KIND_STRING;
  child_fields[2].offset = offsetof(struct AdvChild, label);
  child_fields[2].size = sizeof(char *);
  child_fields[2].is_pointer = 1;
  child_fields[2].is_nullable = 1;
  child_fields[2].default_val = "init_lbl";
  child_fields[2].aliases = child_aliases;
  child_fields[2].n_aliases = 1;

  child_fields[3].name = "flag";
  child_fields[3].wire_name = "flag";
  child_fields[3].c_type = "int";
  child_fields[3].kind = CDD_FIELD_KIND_BOOLEAN;
  child_fields[3].offset = offsetof(struct AdvChild, flag);
  child_fields[3].size = sizeof(int);
  child_fields[3].default_val = "true";

  child_ti->name = "AdvChild";
  child_ti->size = sizeof(struct AdvChild);
  child_ti->fields = child_fields;
  child_ti->n_fields = 4;
  rc = cdd_reflect_register_type(child_ti);
  if (rc != CDD_C_SUCCESS)
    return rc;

  parent_fields[0].name = "i64";
  parent_fields[0].wire_name = "i64";
  parent_fields[0].c_type = "int64_t";
  parent_fields[0].kind = CDD_FIELD_KIND_INTEGER;
  parent_fields[0].offset = offsetof(struct AdvParent, i64);
  parent_fields[0].size = sizeof(int64_t);
  parent_fields[0].default_val = "42";

  parent_fields[1].name = "u64";
  parent_fields[1].wire_name = "u64";
  parent_fields[1].c_type = "uint64_t";
  parent_fields[1].kind = CDD_FIELD_KIND_INTEGER;
  parent_fields[1].offset = offsetof(struct AdvParent, u64);
  parent_fields[1].size = sizeof(uint64_t);

  parent_fields[2].name = "i8";
  parent_fields[2].wire_name = "i8";
  parent_fields[2].c_type = "int8_t";
  parent_fields[2].kind = CDD_FIELD_KIND_INTEGER;
  parent_fields[2].offset = offsetof(struct AdvParent, i8);
  parent_fields[2].size = sizeof(int8_t);

  parent_fields[3].name = "u8";
  parent_fields[3].wire_name = "u8";
  parent_fields[3].c_type = "uint8_t";
  parent_fields[3].kind = CDD_FIELD_KIND_INTEGER;
  parent_fields[3].offset = offsetof(struct AdvParent, u8);
  parent_fields[3].size = sizeof(uint8_t);

  parent_fields[4].name = "i16";
  parent_fields[4].wire_name = "i16";
  parent_fields[4].c_type = "int16_t";
  parent_fields[4].kind = CDD_FIELD_KIND_INTEGER;
  parent_fields[4].offset = offsetof(struct AdvParent, i16);
  parent_fields[4].size = sizeof(int16_t);

  parent_fields[5].name = "u16";
  parent_fields[5].wire_name = "u16";
  parent_fields[5].c_type = "uint16_t";
  parent_fields[5].kind = CDD_FIELD_KIND_INTEGER;
  parent_fields[5].offset = offsetof(struct AdvParent, u16);
  parent_fields[5].size = sizeof(uint16_t);

  parent_fields[6].name = "u32";
  parent_fields[6].wire_name = "u32";
  parent_fields[6].c_type = "uint32_t";
  parent_fields[6].kind = CDD_FIELD_KIND_INTEGER;
  parent_fields[6].offset = offsetof(struct AdvParent, u32);
  parent_fields[6].size = sizeof(uint32_t);

  parent_fields[7].name = "num";
  parent_fields[7].wire_name = "num";
  parent_fields[7].c_type = "double";
  parent_fields[7].kind = CDD_FIELD_KIND_NUMBER;
  parent_fields[7].offset = offsetof(struct AdvParent, num);
  parent_fields[7].size = sizeof(double);

  parent_fields[8].name = "bval";
  parent_fields[8].wire_name = "bval";
  parent_fields[8].c_type = "int";
  parent_fields[8].kind = CDD_FIELD_KIND_BOOLEAN;
  parent_fields[8].offset = offsetof(struct AdvParent, bval);
  parent_fields[8].size = sizeof(int);

  parent_fields[9].name = "eval";
  parent_fields[9].wire_name = "eval";
  parent_fields[9].c_type = "enum AdvEnum";
  parent_fields[9].ref_name = "AdvEnum";
  parent_fields[9].kind = CDD_FIELD_KIND_ENUM;
  parent_fields[9].offset = offsetof(struct AdvParent, eval);
  parent_fields[9].size = sizeof(int);
  parent_fields[9].default_val = "alpha";

  parent_fields[10].name = "str";
  parent_fields[10].wire_name = "str~name/1";
  parent_fields[10].c_type = "char *";
  parent_fields[10].kind = CDD_FIELD_KIND_STRING;
  parent_fields[10].offset = offsetof(struct AdvParent, str);
  parent_fields[10].size = sizeof(char *);
  parent_fields[10].is_pointer = 1;
  parent_fields[10].is_nullable = 1;

  parent_fields[11].name = "inline_child";
  parent_fields[11].wire_name = "inline_child";
  parent_fields[11].c_type = "struct AdvChild";
  parent_fields[11].ref_name = "AdvChild";
  parent_fields[11].kind = CDD_FIELD_KIND_OBJECT;
  parent_fields[11].offset = offsetof(struct AdvParent, inline_child);
  parent_fields[11].size = sizeof(struct AdvChild);
  parent_fields[11].is_pointer = 0;

  parent_fields[12].name = "ptr_child";
  parent_fields[12].wire_name = "ptr_child";
  parent_fields[12].c_type = "struct AdvChild *";
  parent_fields[12].ref_name = "AdvChild";
  parent_fields[12].kind = CDD_FIELD_KIND_OBJECT;
  parent_fields[12].offset = offsetof(struct AdvParent, ptr_child);
  parent_fields[12].size = sizeof(struct AdvChild *);
  parent_fields[12].is_pointer = 1;
  parent_fields[12].is_nullable = 1;

  parent_fields[13].name = "int_arr";
  parent_fields[13].wire_name = "int_arr";
  parent_fields[13].c_type = "int *";
  parent_fields[13].kind = CDD_FIELD_KIND_ARRAY;
  parent_fields[13].elem_kind = CDD_FIELD_KIND_INTEGER;
  parent_fields[13].elem_size = sizeof(int);
  parent_fields[13].offset = offsetof(struct AdvParent, int_arr);
  parent_fields[13].size = sizeof(int *);
  parent_fields[13].array_len_offset = offsetof(struct AdvParent, n_int_arr);
  parent_fields[13].is_pointer = 1;

  parent_fields[14].name = "dbl_arr";
  parent_fields[14].wire_name = "dbl_arr";
  parent_fields[14].c_type = "double *";
  parent_fields[14].kind = CDD_FIELD_KIND_ARRAY;
  parent_fields[14].elem_kind = CDD_FIELD_KIND_NUMBER;
  parent_fields[14].elem_size = sizeof(double);
  parent_fields[14].offset = offsetof(struct AdvParent, dbl_arr);
  parent_fields[14].size = sizeof(double *);
  parent_fields[14].array_len_offset = offsetof(struct AdvParent, n_dbl_arr);
  parent_fields[14].is_pointer = 1;

  parent_fields[15].name = "bool_arr";
  parent_fields[15].wire_name = "bool_arr";
  parent_fields[15].c_type = "int *";
  parent_fields[15].kind = CDD_FIELD_KIND_ARRAY;
  parent_fields[15].elem_kind = CDD_FIELD_KIND_BOOLEAN;
  parent_fields[15].elem_size = sizeof(int);
  parent_fields[15].offset = offsetof(struct AdvParent, bool_arr);
  parent_fields[15].size = sizeof(int *);
  parent_fields[15].array_len_offset = offsetof(struct AdvParent, n_bool_arr);
  parent_fields[15].is_pointer = 1;

  parent_fields[16].name = "enum_arr";
  parent_fields[16].wire_name = "enum_arr";
  parent_fields[16].c_type = "int *";
  parent_fields[16].ref_name = "AdvEnum";
  parent_fields[16].kind = CDD_FIELD_KIND_ARRAY;
  parent_fields[16].elem_kind = CDD_FIELD_KIND_ENUM;
  parent_fields[16].elem_size = sizeof(int);
  parent_fields[16].offset = offsetof(struct AdvParent, enum_arr);
  parent_fields[16].size = sizeof(int *);
  parent_fields[16].array_len_offset = offsetof(struct AdvParent, n_enum_arr);
  parent_fields[16].is_pointer = 1;

  parent_fields[17].name = "str_arr";
  parent_fields[17].wire_name = "str_arr";
  parent_fields[17].c_type = "char **";
  parent_fields[17].kind = CDD_FIELD_KIND_ARRAY;
  parent_fields[17].elem_kind = CDD_FIELD_KIND_STRING;
  parent_fields[17].elem_size = sizeof(char *);
  parent_fields[17].offset = offsetof(struct AdvParent, str_arr);
  parent_fields[17].size = sizeof(char **);
  parent_fields[17].array_len_offset = offsetof(struct AdvParent, n_str_arr);
  parent_fields[17].is_pointer = 1;

  parent_fields[18].name = "val_obj_arr";
  parent_fields[18].wire_name = "val_obj_arr";
  parent_fields[18].c_type = "struct AdvChild *";
  parent_fields[18].ref_name = "AdvChild";
  parent_fields[18].kind = CDD_FIELD_KIND_ARRAY;
  parent_fields[18].elem_kind = CDD_FIELD_KIND_OBJECT;
  parent_fields[18].elem_size = sizeof(struct AdvChild);
  parent_fields[18].offset = offsetof(struct AdvParent, val_obj_arr);
  parent_fields[18].size = sizeof(struct AdvChild *);
  parent_fields[18].array_len_offset =
      offsetof(struct AdvParent, n_val_obj_arr);
  parent_fields[18].is_pointer = 1;

  parent_fields[19].name = "ptr_obj_arr";
  parent_fields[19].wire_name = "ptr_obj_arr";
  parent_fields[19].c_type = "struct AdvChild **";
  parent_fields[19].ref_name = "AdvChild";
  parent_fields[19].kind = CDD_FIELD_KIND_ARRAY;
  parent_fields[19].elem_kind = CDD_FIELD_KIND_OBJECT;
  parent_fields[19].elem_size = sizeof(struct AdvChild *);
  parent_fields[19].offset = offsetof(struct AdvParent, ptr_obj_arr);
  parent_fields[19].size = sizeof(struct AdvChild **);
  parent_fields[19].array_len_offset =
      offsetof(struct AdvParent, n_ptr_obj_arr);
  parent_fields[19].is_pointer = 1;

  parent_fields[20].name = "fixed_ints";
  parent_fields[20].wire_name = "fixed_ints";
  parent_fields[20].c_type = "int[2]";
  parent_fields[20].kind = CDD_FIELD_KIND_ARRAY;
  parent_fields[20].elem_kind = CDD_FIELD_KIND_INTEGER;
  parent_fields[20].elem_size = sizeof(int);
  parent_fields[20].offset = offsetof(struct AdvParent, fixed_ints);
  parent_fields[20].size = sizeof(dummy_p.fixed_ints);
  parent_fields[20].fixed_array_len = 2;

  parent_fields[21].name = "opt_int";
  parent_fields[21].wire_name = "opt_int";
  parent_fields[21].c_type = "int *";
  parent_fields[21].kind = CDD_FIELD_KIND_INTEGER;
  parent_fields[21].offset = offsetof(struct AdvParent, opt_int);
  parent_fields[21].size = sizeof(int *);
  parent_fields[21].is_pointer = 1;
  parent_fields[21].is_nullable = 1;

  parent_ti->name = "AdvParent";
  parent_ti->size = sizeof(struct AdvParent);
  parent_ti->fields = parent_fields;
  parent_ti->n_fields = 22;
  rc = cdd_reflect_register_type(parent_ti);
  if (rc != CDD_C_SUCCESS)
    return rc;

  union_fields[0].name = "num_branch";
  union_fields[0].wire_name = "num_branch";
  union_fields[0].c_type = "int";
  union_fields[0].kind = CDD_FIELD_KIND_INTEGER;
  union_fields[0].offset = offsetof(struct AdvUnion, data.num_branch);
  union_fields[0].size = sizeof(int);

  union_fields[1].name = "str_branch";
  union_fields[1].wire_name = "str_branch";
  union_fields[1].c_type = "char *";
  union_fields[1].kind = CDD_FIELD_KIND_STRING;
  union_fields[1].offset = offsetof(struct AdvUnion, data.str_branch);
  union_fields[1].size = sizeof(char *);
  union_fields[1].is_pointer = 1;

  union_ti->name = "AdvUnion";
  union_ti->size = sizeof(struct AdvUnion);
  union_ti->is_union = 1;
  union_ti->fields = union_fields;
  union_ti->n_fields = 2;
  union_ti->tag_offset = offsetof(struct AdvUnion, tag);

  func_params[0].name = "a";
  func_params[0].c_type = "int";
  func_params[0].kind = CDD_FIELD_KIND_INTEGER;
  func_params[1].name = "b";
  func_params[1].c_type = "int";
  func_params[1].kind = CDD_FIELD_KIND_INTEGER;
  func_ti->name = "dummy_add";
  func_ti->return_c_type = "int";
  func_ti->return_kind = CDD_FIELD_KIND_INTEGER;
  func_ti->params = func_params;
  func_ti->n_params = 2;
  func_ti->invoke = dummy_add_invoke_trampoline;
  rc = cdd_reflect_register_func(func_ti);
  if (rc != CDD_C_SUCCESS)
    return rc;

  return CDD_C_SUCCESS;
}

/**
 * @brief Tests advanced reflection features: 64-bit unsigned/narrowing bounds,
 * regex & format validation, uniqueItems, deep array shrink cleanup,
 * `cdd_reflect_visit_deep_mut`, path accessors (`cdd_reflect_resolve_path`,
 * `get/set_*_by_path`), table-driven runtime lifecycle (`default`, `cleanup`,
 * `deepcopy`, `eq`), table-driven JSON serde (`to_json`, `from_json`),
 * RFC 7396 JSON Merge Patch (`merge_patch`, `apply_merge_patch`), and
 * `cdd_reflect_invoke_func` / `cdd_reflect_invoke_by_name`.
 *
 * @return TEST result.
 */
TEST test_reflection_advanced_features(void) {
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

  /* 1. 64-bit unsigned & narrowing overflow validation */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_uint64(&p1, &parent_fields[1],
                                                  ((uint64_t)1U << 63) + 123U));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_get_uint64(&p1, &parent_fields[1], &v_u64));
  ASSERT_EQ(((uint64_t)1U << 63) + 123U, v_u64);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_get_int64(&p1, &parent_fields[1], &v_i64));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_set_int64(&p1, &parent_fields[1], -1));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_set_uint64(&p1, &parent_fields[0],
                                   ((uint64_t)1U << 63) + 1U));

  p1.i64 = -5;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_get_uint64(&p1, &parent_fields[0], &v_u64));
  p1.i8 = -2;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_get_uint64(&p1, &parent_fields[2], &v_u64));
  p1.i16 = -2;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_get_uint64(&p1, &parent_fields[4], &v_u64));
  p1.eval = -2;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_get_uint64(&p1, &parent_fields[9], &v_u64));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_set_int64(&p1, &parent_fields[2], 200));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_set_int64(&p1, &parent_fields[3], 300));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_set_int64(&p1, &parent_fields[4], 40000));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_set_int64(&p1, &parent_fields[5], 70000));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_set_int64(&p1, &parent_fields[6], -1));

  /* 2. Validation: exclusive_min/max, formats, regex patterns, unique_items */
  p1.i64 = 10;
  parent_fields[0].has_min = 1;
  parent_fields[0].min_val = 10.0;
  parent_fields[0].exclusive_min = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_validate_field(&p1, &parent_fields[0]));
  p1.i64 = 11;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_validate_field(&p1, &parent_fields[0]));
  parent_fields[0].has_max = 1;
  parent_fields[0].max_val = 11.0;
  parent_fields[0].exclusive_max = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_validate_field(&p1, &parent_fields[0]));
  parent_fields[0].has_min = 0;
  parent_fields[0].has_max = 0;
  parent_fields[0].exclusive_min = 0;
  parent_fields[0].exclusive_max = 0;

  parent_fields[0].format = "uint32";
  p1.i64 = -1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_validate_field(&p1, &parent_fields[0]));
  parent_fields[0].format = "uint64";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_validate_field(&p1, &parent_fields[0]));
  parent_fields[0].format = NULL;
  p1.i64 = 10;

  p1.num = 5.0;
  parent_fields[7].has_min = 1;
  parent_fields[7].min_val = 5.0;
  parent_fields[7].exclusive_min = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_validate_field(&p1, &parent_fields[7]));
  p1.num = 6.0;
  parent_fields[7].has_max = 1;
  parent_fields[7].max_val = 6.0;
  parent_fields[7].exclusive_max = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_validate_field(&p1, &parent_fields[7]));
  parent_fields[7].has_min = 0;
  parent_fields[7].has_max = 0;
  parent_fields[7].exclusive_min = 0;
  parent_fields[7].exclusive_max = 0;

  /* Enum validation */
  p1.eval = 999;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_validate_field(&p1, &parent_fields[9]));
  p1.eval = 10;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_validate_field(&p1, &parent_fields[9]));

  /* String formats & regex patterns */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_set_string(&p1, &parent_fields[10],
                                   "550e8400-e29b-41d4-a716-446655440000"));
  parent_fields[10].format = "uuid";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_validate_field(&p1, &parent_fields[10]));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_set_string(&p1, &parent_fields[10], "bad-uuid"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_validate_field(&p1, &parent_fields[10]));

  parent_fields[10].format = "email";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_set_string(&p1, &parent_fields[10], "a@b.com"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_validate_field(&p1, &parent_fields[10]));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_set_string(&p1, &parent_fields[10], "bad-email"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_validate_field(&p1, &parent_fields[10]));

  parent_fields[10].format = "date";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_set_string(&p1, &parent_fields[10], "2026-11-05"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_validate_field(&p1, &parent_fields[10]));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_set_string(&p1, &parent_fields[10], "2026-13-40"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_validate_field(&p1, &parent_fields[10]));

  parent_fields[10].format = "date-time";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_string(&p1, &parent_fields[10],
                                                  "2026-11-05T12:34:56Z"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_validate_field(&p1, &parent_fields[10]));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_string(&p1, &parent_fields[10],
                                                  "2026-11-05T25:99:99Z"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_validate_field(&p1, &parent_fields[10]));

  parent_fields[10].format = "ipv4";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_set_string(&p1, &parent_fields[10], "192.168.1.1"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_validate_field(&p1, &parent_fields[10]));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_set_string(&p1, &parent_fields[10], "999.1.1.1"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_validate_field(&p1, &parent_fields[10]));

  parent_fields[10].format = "ipv6";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_set_string(&p1, &parent_fields[10], "2001:db8::1"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_validate_field(&p1, &parent_fields[10]));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_set_string(&p1, &parent_fields[10], "not_ipv6"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_validate_field(&p1, &parent_fields[10]));

  parent_fields[10].format = "uri";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_string(&p1, &parent_fields[10],
                                                  "https://example.com"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_validate_field(&p1, &parent_fields[10]));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_set_string(&p1, &parent_fields[10], ":/bad"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_validate_field(&p1, &parent_fields[10]));

  parent_fields[10].format = "hostname";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_set_string(&p1, &parent_fields[10], "api.example.com"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_validate_field(&p1, &parent_fields[10]));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_set_string(&p1, &parent_fields[10], "-badhost-"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_validate_field(&p1, &parent_fields[10]));
  parent_fields[10].format = NULL;

  /* Regex pattern matching */
  parent_fields[10].pattern = "^[a-z]{2,4}-\\d+|[A-Z]+\\w*\\s?.?$";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_set_string(&p1, &parent_fields[10], "abc-123"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_validate_field(&p1, &parent_fields[10]));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_set_string(&p1, &parent_fields[10], "XYZ_9 !"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_validate_field(&p1, &parent_fields[10]));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_set_string(&p1, &parent_fields[10], "999-bad"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_validate_field(&p1, &parent_fields[10]));
  parent_fields[10].pattern = NULL;

  /* Array unique_items & deep shrink cleanup */
  parent_fields[13].unique_items = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_array_resize(&p1, &parent_fields[13], 2));
  p1.int_arr[0] = 7;
  p1.int_arr[1] = 7;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_validate_field(&p1, &parent_fields[13]));
  p1.int_arr[1] = 8;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_validate_field(&p1, &parent_fields[13]));
  parent_fields[13].unique_items = 0;

  /* String array unique_items and shrink cleanup */
  parent_fields[17].unique_items = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_array_resize(&p1, &parent_fields[17], 2));
  p1.str_arr[0] = (char *)C_CDD_MALLOC(4);
  memcpy(p1.str_arr[0], "dup", 4);
  p1.str_arr[1] = (char *)C_CDD_MALLOC(4);
  memcpy(p1.str_arr[1], "dup", 4);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_validate_field(&p1, &parent_fields[17]));
  memcpy(p1.str_arr[1], "two", 4);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_validate_field(&p1, &parent_fields[17]));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_array_resize(&p1, &parent_fields[17], 1));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_array_resize(&p1, &parent_fields[17], 0));
  parent_fields[17].unique_items = 0;

  /* 3. Table-driven default, deepcopy, eq, to_json, from_json, merge_patch */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_struct_default(&parent_ti, (void **)&p_def));
  ASSERT(p_def != NULL);
  ASSERT_EQ(42, p_def->i64);
  ASSERT_EQ(10, p_def->eval);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_struct_default(&child_ti, (void **)&p_def->ptr_child));
  ASSERT_EQ(5, p_def->ptr_child->id);
  ASSERT_STR_EQ("init_lbl", p_def->ptr_child->label);
  ASSERT_EQ(1, p_def->ptr_child->flag);

  /* Populate arrays & nested fields via path accessors */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_array_resize(p_def, &parent_fields[13], 2));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_set_int64_by_path(p_def, &parent_ti, "/int_arr/0", 11));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_set_int64_by_path(p_def, &parent_ti, "int_arr[1]", 22));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_get_int64_by_path(p_def, &parent_ti,
                                                         "/int_arr/1", &v_i64));
  ASSERT_EQ(22, v_i64);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_array_resize(p_def, &parent_fields[14], 1));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_double_by_path(p_def, &parent_ti,
                                                          "/dbl_arr/0", 3.25));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_get_double_by_path(
                               p_def, &parent_ti, "dbl_arr[0]", &v_dbl));
  ASSERT_EQ(3.25, v_dbl);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_array_resize(p_def, &parent_fields[17], 1));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_string_by_path(
                               p_def, &parent_ti, "/str_arr/0", "elem0"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_get_string_by_path(
                               p_def, &parent_ti, "str_arr[0]", &v_str));
  ASSERT_STR_EQ("elem0", v_str);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_int64_by_path(p_def, &parent_ti,
                                                         "/ptr_child/id", 77));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_get_int64_by_path(
                               p_def, &parent_ti, "ptr_child.id", &v_i64));
  ASSERT_EQ(77, v_i64);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_set_double_by_path(
                               p_def, &parent_ti, "/ptr_child/score", 9.5));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_get_double_by_path(
                               p_def, &parent_ti, "ptr_child.score", &v_dbl));
  ASSERT_EQ(9.5, v_dbl);

  /* RFC 6901 ~0 and ~1 escaping in path resolution */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_set_string_by_path(p_def, &parent_ti, "/str~0name~11",
                                           "escaped_ok"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_get_string_by_path(
                               p_def, &parent_ti, "/str~0name~11", &v_str));
  ASSERT_STR_EQ("escaped_ok", v_str);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_resolve_path(NULL, &parent_ti, "/i64", &res_f,
                                     &res_parent, &res_ptr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_reflect_resolve_path(p_def, &parent_ti, "/", &res_f,
                                     &res_parent, &res_ptr));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            cdd_reflect_resolve_path(p_def, &parent_ti, "/nonexistent", &res_f,
                                     &res_parent, &res_ptr));

  /* Populate object pointer array & value object array */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_array_resize(p_def, &parent_fields[18], 1));
  p_def->val_obj_arr[0].id = 301;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_set_string(&p_def->val_obj_arr[0], &child_fields[2],
                                   "val_child"));

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_array_resize(p_def, &parent_fields[19], 1));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_default(
                               &child_ti, (void **)&p_def->ptr_obj_arr[0]));
  p_def->ptr_obj_arr[0]->id = 401;

  /* Test cdd_reflect_visit_deep_mut */
  {
    int mut_cnt = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_visit_deep_mut(
                  NULL, &parent_ti, test_reflect_mut_visitor_cb, &mut_cnt));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_reflect_visit_deep_mut(
                  p_def, &parent_ti, test_reflect_mut_visitor_cb, &mut_cnt));
    ASSERT(mut_cnt > 22);
    ASSERT_EQ(87, p_def->ptr_child->id);
    ASSERT_EQ(311, p_def->val_obj_arr[0].id);
    ASSERT_EQ(411, p_def->ptr_obj_arr[0]->id);
  }

  /* Deepcopy and eq */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_struct_deepcopy(p_def, (void **)&p_copy, &parent_ti));
  ASSERT(p_copy != NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_struct_eq(p_def, p_copy, &parent_ti, &is_eq));
  ASSERT_EQ(1, is_eq);

  /* Modify p_copy and compute RFC 7396 JSON Merge Patch */
  p_copy->i64 = 999;
  p_copy->ptr_child->id = 555;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_struct_eq(p_def, p_copy, &parent_ti, &is_eq));
  ASSERT_EQ(0, is_eq);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_merge_patch(p_def, p_copy, &parent_ti, &patch_json));
  ASSERT(patch_json != NULL);
  ASSERT(strstr(patch_json, "999") != NULL);
  ASSERT(strstr(patch_json, "555") != NULL);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_apply_merge_patch(p_def, &parent_ti, patch_json));
  ASSERT_EQ(999, p_def->i64);
  ASSERT_EQ(555, p_def->ptr_child->id);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_struct_eq(p_def, p_copy, &parent_ti, &is_eq));
  ASSERT_EQ(1, is_eq);
  C_CDD_FREE(patch_json);

  /* Table-driven JSON round-trip */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_to_json(p_def, &parent_ti, &json_out));
  ASSERT(json_out != NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_from_json(json_out, &parent_ti, (void **)&p_from_json));
  ASSERT(p_from_json != NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_reflect_struct_eq(p_def, p_from_json, &parent_ti, &is_eq));
  ASSERT_EQ(1, is_eq);
  C_CDD_FREE(json_out);

  /* Union runtime lifecycle & serde */
  {
    struct AdvUnion u1;
    struct AdvUnion *u_copy = NULL;
    struct AdvUnion *u_json = NULL;
    char *u_str = NULL;
    memset(&u1, 0, sizeof(u1));
    u1.tag = 2;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_reflect_set_string(&u1, &union_fields[1], "union_hello"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_reflect_struct_deepcopy(&u1, (void **)&u_copy, &union_ti));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_reflect_struct_eq(&u1, u_copy, &union_ti, &is_eq));
    ASSERT_EQ(1, is_eq);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_to_json(&u1, &union_ti, &u_str));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_reflect_from_json(u_str, &union_ti, (void **)&u_json));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_reflect_struct_eq(&u1, u_json, &union_ti, &is_eq));
    ASSERT_EQ(1, is_eq);
    C_CDD_FREE(u_str);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_cleanup(&u1, &union_ti));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_cleanup(u_copy, &union_ti));
    C_CDD_FREE(u_copy);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_cleanup(u_json, &union_ti));
    C_CDD_FREE(u_json);
  }

  /* Function invocation trampoline tests */
  {
    int arg_a = 15, arg_b = 27, sum_out = 0;
    const void *call_args[2];
    call_args[0] = &arg_a;
    call_args[1] = &arg_b;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_invoke_func(NULL, call_args, &sum_out));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_invoke_func(&func_ti, NULL, &sum_out));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_reflect_invoke_by_name("dummy_add", call_args, &sum_out));
    ASSERT_EQ(42, sum_out);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_reflect_invoke_by_name(NULL, call_args, &sum_out));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_reflect_invoke_by_name("missing_fn", call_args, &sum_out));
    func_ti.invoke = NULL;
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_reflect_invoke_func(&func_ti, call_args, &sum_out));
  }

  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_cleanup(&p1, &parent_ti));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_cleanup(p_def, &parent_ti));
  C_CDD_FREE(p_def);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_cleanup(p_copy, &parent_ti));
  C_CDD_FREE(p_copy);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_struct_cleanup(p_from_json, &parent_ti));
  C_CDD_FREE(p_from_json);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_reflect_clear_registry());
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_STRUCT_REFLECT_ADV_H */
