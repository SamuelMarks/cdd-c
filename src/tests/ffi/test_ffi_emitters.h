#ifndef TEST_FFI_EMITTERS_H
#define TEST_FFI_EMITTERS_H

#include "ffi/cdd_ffi_ir.h"
#include "functions/ffi/cdd_ffi_emit_ada.h"
#include "functions/ffi/cdd_ffi_emit_clojure.h"
#include "functions/ffi/cdd_ffi_emit_common_lisp.h"
#include "functions/ffi/cdd_ffi_emit_cpp.h"
#include "functions/ffi/cdd_ffi_emit_crystal.h"
#include "functions/ffi/cdd_ffi_emit_csharp.h"
#include "functions/ffi/cdd_ffi_emit_d.h"
#include "functions/ffi/cdd_ffi_emit_dart.h"
#include "functions/ffi/cdd_ffi_emit_delphi.h"
#include "functions/ffi/cdd_ffi_emit_elixir.h"
#include "functions/ffi/cdd_ffi_emit_erlang.h"
#include "functions/ffi/cdd_ffi_emit_fortran.h"
#include "functions/ffi/cdd_ffi_emit_fsharp.h"
#include "functions/ffi/cdd_ffi_emit_go.h"
#include "functions/ffi/cdd_ffi_emit_groovy.h"
#include "functions/ffi/cdd_ffi_emit_haskell.h"
#include "functions/ffi/cdd_ffi_emit_java.h"
#include "functions/ffi/cdd_ffi_emit_julia.h"
#include "functions/ffi/cdd_ffi_emit_kotlin.h"
#include "functions/ffi/cdd_ffi_emit_lua.h"
#include "functions/ffi/cdd_ffi_emit_matlab.h"
#include "functions/ffi/cdd_ffi_emit_napi.h"
#include "functions/ffi/cdd_ffi_emit_nim.h"
#include "functions/ffi/cdd_ffi_emit_objc.h"
#include "functions/ffi/cdd_ffi_emit_ocaml.h"
#include "functions/ffi/cdd_ffi_emit_odin.h"
#include "functions/ffi/cdd_ffi_emit_perl.h"
#include "functions/ffi/cdd_ffi_emit_php.h"
#include "functions/ffi/cdd_ffi_emit_python.h"
#include "functions/ffi/cdd_ffi_emit_r.h"
#include "functions/ffi/cdd_ffi_emit_racket.h"
#include "functions/ffi/cdd_ffi_emit_ruby.h"
#include "functions/ffi/cdd_ffi_emit_rust.h"
#include "functions/ffi/cdd_ffi_emit_scala.h"
#include "functions/ffi/cdd_ffi_emit_scheme.h"
#include "functions/ffi/cdd_ffi_emit_swift.h"
#include "functions/ffi/cdd_ffi_emit_tcl.h"
#include "functions/ffi/cdd_ffi_emit_typescript.h"
#include "functions/ffi/cdd_ffi_emit_vlang.h"
#include "functions/ffi/cdd_ffi_emit_webassembly.h"
#include "functions/ffi/cdd_ffi_emit_zig.h"
#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

static cdd_ffi_ir_t *create_dummy_ir(void) {
  cdd_ffi_ir_t *ir = (cdd_ffi_ir_t *)calloc(1, sizeof(cdd_ffi_ir_t));
  ir->nodes = (cdd_ffi_ir_node_t *)calloc(30, sizeof(cdd_ffi_ir_node_t));

  /* node 0: Struct with all types */
  ir->nodes[0].kind = CDD_FFI_NODE_STRUCT;
  ir->nodes[0].name = "TestStruct";
  ir->nodes[0].doc = "Struct doc";
  ir->nodes[0].fields = (cdd_ffi_field_t *)calloc(30, sizeof(cdd_ffi_field_t));
  ir->nodes[0].fields_count = 29;
  ir->nodes[0].fields[0].name = "f_void";
  ir->nodes[0].fields[0].type.kind = CDD_FFI_KIND_VOID;
  ir->nodes[0].fields[1].name = "f_bool";
  ir->nodes[0].fields[1].type.kind = CDD_FFI_KIND_BOOL;
  ir->nodes[0].fields[2].name = "f_i8";
  ir->nodes[0].fields[2].type.kind = CDD_FFI_KIND_INT8;
  ir->nodes[0].fields[3].name = "f_u8";
  ir->nodes[0].fields[3].type.kind = CDD_FFI_KIND_UINT8;
  ir->nodes[0].fields[4].name = "f_i16";
  ir->nodes[0].fields[4].type.kind = CDD_FFI_KIND_INT16;
  ir->nodes[0].fields[5].name = "f_u16";
  ir->nodes[0].fields[5].type.kind = CDD_FFI_KIND_UINT16;
  ir->nodes[0].fields[6].name = "f_i32";
  ir->nodes[0].fields[6].type.kind = CDD_FFI_KIND_INT32;
  ir->nodes[0].fields[7].name = "f_u32";
  ir->nodes[0].fields[7].type.kind = CDD_FFI_KIND_UINT32;
  ir->nodes[0].fields[8].name = "f_i64";
  ir->nodes[0].fields[8].type.kind = CDD_FFI_KIND_INT64;
  ir->nodes[0].fields[9].name = "f_u64";
  ir->nodes[0].fields[9].type.kind = CDD_FFI_KIND_UINT64;
  ir->nodes[0].fields[10].name = "f_f32";
  ir->nodes[0].fields[10].type.kind = CDD_FFI_KIND_FLOAT32;
  ir->nodes[0].fields[11].name = "f_f64";
  ir->nodes[0].fields[11].type.kind = CDD_FFI_KIND_FLOAT64;
  ir->nodes[0].fields[12].name = "f_struct_ref";
  ir->nodes[0].fields[12].type.kind = CDD_FFI_KIND_STRUCT_REF;
  ir->nodes[0].fields[12].type.ref_name = "OtherStruct";
  ir->nodes[0].fields[13].name = "f_enum_ref";
  ir->nodes[0].fields[13].type.kind = CDD_FFI_KIND_ENUM_REF;
  ir->nodes[0].fields[13].type.ref_name = "OtherEnum";
  ir->nodes[0].fields[14].name = "f_opaque";
  ir->nodes[0].fields[14].type.kind = CDD_FFI_KIND_OPAQUE_PTR;
  ir->nodes[0].fields[15].name = "f_func_ptr";
  ir->nodes[0].fields[15].type.kind = CDD_FFI_KIND_FUNCTION_PTR;
  ir->nodes[0].fields[16].name = "f_string";
  ir->nodes[0].fields[16].type.kind = CDD_FFI_KIND_STD_STRING;
  ir->nodes[0].fields[17].name = "f_ptr";
  ir->nodes[0].fields[17].type.kind = CDD_FFI_KIND_INT32;
  ir->nodes[0].fields[17].type.pointer_depth = 1;
  ir->nodes[0].fields[18].name = "f_const_u8_ptr";
  ir->nodes[0].fields[18].type.kind = CDD_FFI_KIND_UINT8;
  ir->nodes[0].fields[18].type.pointer_depth = 1;
  ir->nodes[0].fields[18].type.is_const = 1;
  ir->nodes[0].fields[19].name = "f_arr";
  ir->nodes[0].fields[19].type.kind = CDD_FFI_KIND_INT32;
  ir->nodes[0].fields[19].type.array_size = 4;
  ir->nodes[0].fields[20].name = "f_unknown";
  ir->nodes[0].fields[20].type.kind = 999; /* INVALID KIND */
  ir->nodes[0].fields[21].name = "f_typedef_ref";
  ir->nodes[0].fields[21].type.kind = CDD_FFI_KIND_TYPEDEF_REF;
  ir->nodes[0].fields[21].type.ref_name = "size_t";
  ir->nodes[0].fields[22].name = "f_struct_ref_ptr";
  ir->nodes[0].fields[22].type.kind = CDD_FFI_KIND_STRUCT_REF;
  ir->nodes[0].fields[22].type.ref_name = "OtherStruct";
  ir->nodes[0].fields[22].type.pointer_depth = 1;
  ir->nodes[0].fields[23].name = "f_typedef_ref_ptr";
  ir->nodes[0].fields[23].type.kind = CDD_FFI_KIND_TYPEDEF_REF;
  ir->nodes[0].fields[23].type.ref_name = "OtherTypedef";
  ir->nodes[0].fields[23].type.pointer_depth = 1;
  ir->nodes[0].fields[24].name = "f_void_ptr2";
  ir->nodes[0].fields[24].type.kind = CDD_FFI_KIND_VOID;
  ir->nodes[0].fields[24].type.pointer_depth = 2;
  ir->nodes[0].fields[25].name = "f_u8_ptr";
  ir->nodes[0].fields[25].type.kind = CDD_FFI_KIND_UINT8;
  ir->nodes[0].fields[25].type.pointer_depth = 1;
  ir->nodes[0].fields[25].type.is_const = 0;
  ir->nodes[0].fields[26].name = "f_void_ptr1";
  ir->nodes[0].fields[26].type.kind = CDD_FFI_KIND_VOID;
  ir->nodes[0].fields[26].type.pointer_depth = 1;
  ir->nodes[0].fields[27].name = "f_i8_ptr_ptr";
  ir->nodes[0].fields[27].type.kind = CDD_FFI_KIND_INT8;
  ir->nodes[0].fields[27].type.pointer_depth = 2;

  ir->nodes[0].fields[28].name = "f_const_u8_ptr";
  ir->nodes[0].fields[28].type.kind = CDD_FFI_KIND_UINT8;
  ir->nodes[0].fields[28].type.pointer_depth = 1;
  ir->nodes[0].fields[28].type.is_const = 1;

  /* node 1: Enum */
  ir->nodes[1].kind = CDD_FFI_NODE_ENUM;
  ir->nodes[1].name = "TestEnum";
  ir->nodes[1].doc = "Enum doc";
  ir->nodes[1].variants =
      (cdd_ffi_enum_variant_t *)calloc(2, sizeof(cdd_ffi_enum_variant_t));
  ir->nodes[1].variants_count = 2;
  ir->nodes[1].variants[0].name = "VARIANT_A";
  ir->nodes[1].variants[0].value = "10";
  ir->nodes[1].variants[0].doc = "Variant doc";
  ir->nodes[1].variants[1].name = "VARIANT_B";

  /* node 2: Function (void return) */
  ir->nodes[2].kind = CDD_FFI_NODE_FUNCTION;
  ir->nodes[2].name = "test_func_void";
  ir->nodes[2].return_or_base_type.kind = CDD_FFI_KIND_VOID;
  ir->nodes[2].fields = (cdd_ffi_field_t *)calloc(3, sizeof(cdd_ffi_field_t));
  ir->nodes[2].fields_count = 3;
  ir->nodes[2].fields[0].name = "param1";
  ir->nodes[2].fields[0].type.kind = CDD_FFI_KIND_STRUCT_REF;
  ir->nodes[2].fields[0].type.ref_name = "TestStruct";
  ir->nodes[2].fields[1].name = "in";
  ir->nodes[2].fields[1].type.kind = CDD_FFI_KIND_INT32;
  ir->nodes[2].fields[2].name = "const_str";
  ir->nodes[2].fields[2].type.kind = CDD_FFI_KIND_INT8;
  ir->nodes[2].fields[2].type.pointer_depth = 1;
  ir->nodes[2].fields[2].type.is_const = 1;

  /* node 3: Function (int return, no params) */
  ir->nodes[3].kind = CDD_FFI_NODE_FUNCTION;
  ir->nodes[3].name = "testFunc";
  ir->nodes[3].return_or_base_type.kind = CDD_FFI_KIND_INT32;
  ir->nodes[3].fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  ir->nodes[3].fields_count = 1;
  ir->nodes[3].fields[0].name = "function";
  ir->nodes[3].fields[0].type.kind = CDD_FFI_KIND_INT32;

  /* node 4: Union */
  ir->nodes[4].kind = CDD_FFI_NODE_UNION;
  ir->nodes[4].name = "TestUnion";
  ir->nodes[4].fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  ir->nodes[4].fields_count = 1;
  ir->nodes[4].fields[0].name = "field1";
  ir->nodes[4].fields[0].type.kind = CDD_FFI_KIND_INT32;

  /* node 5: Function (void return, no params) */
  ir->nodes[5].kind = CDD_FFI_NODE_FUNCTION;
  ir->nodes[5].name = "Test_Func";
  ir->nodes[5].return_or_base_type.kind = CDD_FFI_KIND_VOID;
  ir->nodes[5].fields_count = 0;

  /* node 6: Function (int return, with params) */
  ir->nodes[6].kind = CDD_FFI_NODE_FUNCTION;
  ir->nodes[6].name = "test_func_int_with_args";
  ir->nodes[6].return_or_base_type.kind = CDD_FFI_KIND_INT32;
  ir->nodes[6].fields = (cdd_ffi_field_t *)calloc(17, sizeof(cdd_ffi_field_t));
  ir->nodes[6].fields_count = 17;
  ir->nodes[6].fields[0].name = "interface";
  ir->nodes[6].fields[0].type.kind = CDD_FFI_KIND_INT32;
  ir->nodes[6].fields[1].name = "type";
  ir->nodes[6].fields[1].type.kind = CDD_FFI_KIND_BOOL;
  ir->nodes[6].fields[2].name = "func";
  ir->nodes[6].fields[2].type.kind = CDD_FFI_KIND_INT8;
  ir->nodes[6].fields[3].name = "p_u8";
  ir->nodes[6].fields[3].type.kind = CDD_FFI_KIND_UINT8;
  ir->nodes[6].fields[4].name = "p_i16";
  ir->nodes[6].fields[4].type.kind = CDD_FFI_KIND_INT16;
  ir->nodes[6].fields[5].name = "p_u16";
  ir->nodes[6].fields[5].type.kind = CDD_FFI_KIND_UINT16;
  ir->nodes[6].fields[6].name = "p_u32";
  ir->nodes[6].fields[6].type.kind = CDD_FFI_KIND_UINT32;
  ir->nodes[6].fields[7].name = "p_i64";
  ir->nodes[6].fields[7].type.kind = CDD_FFI_KIND_INT64;
  ir->nodes[6].fields[8].name = "p_u64";
  ir->nodes[6].fields[8].type.kind = CDD_FFI_KIND_UINT64;
  ir->nodes[6].fields[9].name = "p_f32";
  ir->nodes[6].fields[9].type.kind = CDD_FFI_KIND_FLOAT32;
  ir->nodes[6].fields[10].name = "p_f64";
  ir->nodes[6].fields[10].type.kind = CDD_FFI_KIND_FLOAT64;
  ir->nodes[6].fields[11].name = "p_func_ptr";
  ir->nodes[6].fields[11].type.kind = CDD_FFI_KIND_FUNCTION_PTR;
  ir->nodes[6].fields[12].name = "p_enum_ref";
  ir->nodes[6].fields[12].type.kind = CDD_FFI_KIND_ENUM_REF;
  ir->nodes[6].fields[13].name = "p_typedef_ref";
  ir->nodes[6].fields[13].type.kind = CDD_FFI_KIND_TYPEDEF_REF;
  ir->nodes[6].fields[13].type.ref_name = "MyTypedef";
  ir->nodes[6].fields[14].name = "p_opaque";
  ir->nodes[6].fields[14].type.kind = CDD_FFI_KIND_OPAQUE_PTR;
  ir->nodes[6].fields[15].name = "p_unknown";
  ir->nodes[6].fields[15].type.kind = 999;
  ir->nodes[6].fields[16].name = "p_void";
  ir->nodes[6].fields[16].type.kind = CDD_FFI_KIND_VOID;

  /* node 7: Null ref_name field */
  ir->nodes[7].kind = CDD_FFI_NODE_STRUCT;
  ir->nodes[7].name = "BadStruct";
  ir->nodes[7].fields = (cdd_ffi_field_t *)calloc(3, sizeof(cdd_ffi_field_t));
  ir->nodes[7].fields_count = 3;
  ir->nodes[7].fields[0].name = NULL;
  ir->nodes[7].fields[0].type.kind = CDD_FFI_KIND_STRUCT_REF;
  ir->nodes[7].fields[0].type.ref_name = NULL;
  ir->nodes[7].fields[0].type.pointer_depth = 0;
  ir->nodes[7].fields[1].name = "f_bad2";
  ir->nodes[7].fields[1].type.kind = CDD_FFI_KIND_ENUM_REF;
  ir->nodes[7].fields[1].type.ref_name = NULL;
  ir->nodes[7].fields[1].type.pointer_depth = 0;
  ir->nodes[7].fields[2].name = "f_bad3";
  ir->nodes[7].fields[2].type.kind = CDD_FFI_KIND_TYPEDEF_REF;
  ir->nodes[7].fields[2].type.ref_name = NULL;
  ir->nodes[7].fields[2].type.pointer_depth = 0;
  ir->nodes[7].fields[0].type.ref_name = NULL;

  /* node 8: Typedef */
  ir->nodes[8].kind = CDD_FFI_NODE_TYPEDEF;
  ir->nodes[8].name = "MyTypedef";
  ir->nodes[8].return_or_base_type.kind = CDD_FFI_KIND_INT32;

  /* node 9: Macro */
  ir->nodes[9].kind = CDD_FFI_NODE_MACRO;
  ir->nodes[9].name = "MyMacro";

  /* node 10: Trampoline struct */
  ir->nodes[10].kind = CDD_FFI_NODE_STRUCT;
  ir->nodes[10].name = "TestClass_Trampoline";
  ir->nodes[10].fields = (cdd_ffi_field_t *)calloc(4, sizeof(cdd_ffi_field_t));
  ir->nodes[10].fields_count = 4;
  ir->nodes[10].fields[0].name = "ctx";
  ir->nodes[10].fields[1].name = "cb_AddRef";
  ir->nodes[10].fields[2].name = "cb_Release";
  ir->nodes[10].fields[3].name = "cb_MyVirtualFunc";

  /* node 11: Function with class and this args */
  ir->nodes[11].kind = CDD_FFI_NODE_FUNCTION;
  ir->nodes[11].name = "test_func_cpp_args";
  ir->nodes[11].doc = "My docstring";
  ir->nodes[11].return_or_base_type.kind = CDD_FFI_KIND_VOID;
  ir->nodes[11].fields = (cdd_ffi_field_t *)calloc(2, sizeof(cdd_ffi_field_t));
  ir->nodes[11].fields_count = 2;
  ir->nodes[11].fields[0].name = "class";
  ir->nodes[11].fields[0].type.kind = CDD_FFI_KIND_INT32;
  ir->nodes[11].fields[1].name = "this";
  ir->nodes[11].fields[1].type.kind = CDD_FFI_KIND_INT32;

  /* node 12: Base Class struct */
  ir->nodes[12].kind = CDD_FFI_NODE_STRUCT;
  ir->nodes[12].name = "BaseClass";

  /* node 13: Derived struct */
  ir->nodes[13].kind = CDD_FFI_NODE_STRUCT;
  ir->nodes[13].name = "DerivedStruct";
  ir->nodes[13].base_classes =
      (cdd_ffi_base_class_t *)calloc(2, sizeof(cdd_ffi_base_class_t));
  ir->nodes[13].base_classes_count = 2;
  ir->nodes[13].base_classes[0].name = "BaseClass";
  ir->nodes[13].base_classes[1].name = "OtherBaseClass";
  ir->nodes[13].base_classes[1].is_virtual = 1;

  /* node 14: Function returning char* */
  ir->nodes[14].kind = CDD_FFI_NODE_FUNCTION;
  ir->nodes[14].name = "test_func_char_ptr";
  ir->nodes[14].return_or_base_type.kind = CDD_FFI_KIND_INT8;
  ir->nodes[14].return_or_base_type.pointer_depth = 1;
  ir->nodes[14].fields_count = 0;

  /* node 15: Function with out, inout, and variadic args */
  ir->nodes[15].kind = CDD_FFI_NODE_FUNCTION;
  ir->nodes[15].name = "test_func_out_inout_var";
  ir->nodes[15].return_or_base_type.kind = CDD_FFI_KIND_VOID;
  ir->nodes[15].fields = (cdd_ffi_field_t *)calloc(4, sizeof(cdd_ffi_field_t));
  ir->nodes[15].fields_count = 4;
  ir->nodes[15].is_variadic = 1;
  ir->nodes[15].fields[0].name = "out_arg";
  ir->nodes[15].fields[0].type.kind = CDD_FFI_KIND_INT32;
  ir->nodes[15].fields[0].type.pointer_depth = 1;
  ir->nodes[15].fields[0].intent = CDD_FFI_INTENT_OUT;
  ir->nodes[15].fields[1].name = "inout_arg";
  ir->nodes[15].fields[1].type.kind = CDD_FFI_KIND_STRUCT_REF;
  ir->nodes[15].fields[1].type.ref_name = "TestStruct";
  ir->nodes[15].fields[1].type.pointer_depth = 1;
  ir->nodes[15].fields[1].intent = CDD_FFI_INTENT_INOUT;
  ir->nodes[15].fields[2].name = "string_out";
  ir->nodes[15].fields[2].type.kind = CDD_FFI_KIND_INT8;
  ir->nodes[15].fields[2].type.pointer_depth = 2;
  ir->nodes[15].fields[2].intent = CDD_FFI_INTENT_OUT;
  ir->nodes[15].fields[3].name = "u8_ptr_arg";
  ir->nodes[15].fields[3].type.kind = CDD_FFI_KIND_UINT8;
  ir->nodes[15].fields[3].type.pointer_depth = 1;

  /* node 16: Function returning uint8* */
  ir->nodes[16].kind = CDD_FFI_NODE_FUNCTION;
  ir->nodes[16].name = "test_func_uchar_ptr";
  ir->nodes[16].return_or_base_type.kind = CDD_FFI_KIND_UINT8;
  ir->nodes[16].return_or_base_type.pointer_depth = 1;
  ir->nodes[16].fields_count = 0;

  /* node 17: Python specific GIL */
  ir->nodes[17].kind = CDD_FFI_NODE_FUNCTION;
  ir->nodes[17].name = "test_gil";
  ir->nodes[17].requires_gil_release = 1;

  /* node 18: Function with array out param */
  ir->nodes[18].kind = CDD_FFI_NODE_FUNCTION;
  ir->nodes[18].name = "test_arr_out";
  ir->nodes[18].fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  ir->nodes[18].fields_count = 1;
  ir->nodes[18].fields[0].name = "arr_out";
  ir->nodes[18].fields[0].intent = CDD_FFI_INTENT_OUT;
  ir->nodes[18].fields[0].type.kind = CDD_FFI_KIND_INT32;
  ir->nodes[18].fields[0].type.pointer_depth = 1;
  ir->nodes[18].fields[0].array_length_ref = "4";

  /* node 19: Macros string */
  ir->nodes[19].kind = CDD_FFI_NODE_MACRO;
  ir->nodes[19].name = "MY_STR_MACRO";
  ir->nodes[19].evaluated_value = "\"hello\"";
  ir->nodes[19].inferred_type = CDD_FFI_MACRO_TYPE_STRING;

  /* node 20: Macro float */
  ir->nodes[20].kind = CDD_FFI_NODE_MACRO;
  ir->nodes[20].name = "MY_FLT_MACRO";
  ir->nodes[20].evaluated_value = "3.14";
  ir->nodes[20].inferred_type = CDD_FFI_MACRO_TYPE_FLOAT;

  /* node 21: Macro int */
  ir->nodes[21].kind = CDD_FFI_NODE_MACRO;
  ir->nodes[21].name = "MY_INT_MACRO";
  ir->nodes[21].evaluated_value = "42";
  ir->nodes[21].inferred_type = CDD_FFI_MACRO_TYPE_INT;

  /* node 22: Empty struct */
  ir->nodes[22].kind = CDD_FFI_NODE_STRUCT;
  ir->nodes[22].name = "EmptyStruct";
  ir->nodes[22].fields_count = 0;

  /* node 23: Empty enum */
  ir->nodes[23].kind = CDD_FFI_NODE_ENUM;
  ir->nodes[23].name = "EmptyEnum";
  ir->nodes[23].variants_count = 0;

  /* node 24: Function with various OUT params */
  ir->nodes[24].kind = CDD_FFI_NODE_FUNCTION;
  ir->nodes[24].name = "test_func_many_out";
  ir->nodes[24].fields = (cdd_ffi_field_t *)calloc(3, sizeof(cdd_ffi_field_t));
  ir->nodes[24].fields_count = 3;
  ir->nodes[24].fields[0].name = "out_i64";
  ir->nodes[24].fields[0].type.kind = CDD_FFI_KIND_INT64;
  ir->nodes[24].fields[0].type.pointer_depth = 1;
  ir->nodes[24].fields[0].intent = CDD_FFI_INTENT_OUT;
  ir->nodes[24].fields[1].name = "out_f32";
  ir->nodes[24].fields[1].type.kind = CDD_FFI_KIND_FLOAT32;
  ir->nodes[24].fields[1].type.pointer_depth = 1;
  ir->nodes[24].fields[1].intent = CDD_FFI_INTENT_OUT;
  ir->nodes[24].fields[2].name = "out_f64";
  ir->nodes[24].fields[2].type.kind = CDD_FFI_KIND_FLOAT64;
  ir->nodes[24].fields[2].type.pointer_depth = 1;
  ir->nodes[24].fields[2].intent = CDD_FFI_INTENT_OUT;

  ir->nodes_count = 25;
  ir->nodes_capacity = 30;

  return ir;
}

static void free_dummy_ir(cdd_ffi_ir_t *ir) {
  if (!ir)
    return;
  free(ir->nodes[0].fields);
  free(ir->nodes[1].variants);
  free(ir->nodes[2].fields);
  free(ir->nodes[4].fields);
  free(ir->nodes[6].fields);
  free(ir->nodes[7].fields);
  free(ir->nodes[10].fields);
  free(ir->nodes[11].fields);
  free(ir->nodes[13].base_classes);
  free(ir->nodes[15].fields);
  free(ir->nodes);
  free(ir);
}

#define TEST_EMITTER(lang)                                                     \
  TEST test_ffi_emit_##lang(void) {                                            \
    cdd_ffi_ir_t *ir = create_dummy_ir();                                      \
    cdd_generate_bindings_config_t config = {0};                               \
    int i;                                                                     \
    config.input = "my_input.h";                                               \
    config.output_dir = "build/test_out_dir";                                  \
    config.library_name = "test_Lib_name";                                     \
    config.module_name = "TestMod";                                            \
    config.generate_tests = 1;                                                 \
                                                                               \
    cdd_ffi_emit_##lang(ir, &config);                                          \
                                                                               \
    config.input = NULL;                                                       \
    cdd_ffi_emit_##lang(ir, &config);                                          \
                                                                               \
    config.module_name = NULL;                                                 \
    cdd_ffi_emit_##lang(ir, &config);                                          \
    config.module_name = "TestMod";                                            \
                                                                               \
    for (i = 1; i <= 5; i++) {                                                 \
      g_fail_io_after = i;                                                     \
      cdd_ffi_emit_##lang(ir, &config);                                        \
    }                                                                          \
    for (i = 555; i <= 557; i++) {                                             \
      g_fail_io_after = i;                                                     \
      cdd_ffi_emit_##lang(ir, &config);                                        \
    }                                                                          \
    g_fail_io_after = -1;                                                      \
                                                                               \
    cdd_ffi_emit_##lang(NULL, &config);                                        \
                                                                               \
    config.output_dir = NULL;                                                  \
    cdd_ffi_emit_##lang(ir, &config);                                          \
                                                                               \
    config.output_dir = "/dev/null/invalid_dir";                               \
    cdd_ffi_emit_##lang(ir, &config);                                          \
                                                                               \
    config.output_dir = "build/test_out_dir";                                  \
    ir->nodes[0].fields_count = 0;                                             \
    cdd_ffi_emit_##lang(ir, &config);                                          \
                                                                               \
    ir->nodes_count = 0;                                                       \
    cdd_ffi_emit_##lang(ir, &config);                                          \
                                                                               \
    free_dummy_ir(ir);                                                         \
    PASS();                                                                    \
  }

TEST_EMITTER(ada)
TEST_EMITTER(clojure)
TEST_EMITTER(common_lisp)
TEST_EMITTER(cpp)
TEST_EMITTER(crystal)
TEST_EMITTER(csharp)
TEST_EMITTER(dart)
TEST_EMITTER(d)
TEST_EMITTER(delphi)
TEST_EMITTER(elixir)
TEST_EMITTER(erlang)
TEST_EMITTER(fortran)
TEST_EMITTER(fsharp)
TEST_EMITTER(go)
TEST_EMITTER(groovy)
TEST_EMITTER(haskell)
TEST_EMITTER(java)
TEST_EMITTER(julia)
TEST_EMITTER(kotlin)
TEST_EMITTER(lua)
TEST_EMITTER(matlab)
TEST_EMITTER(napi)
TEST_EMITTER(nim)
TEST_EMITTER(objc)
TEST_EMITTER(ocaml)
TEST_EMITTER(odin)
TEST_EMITTER(perl)
TEST_EMITTER(php)
TEST_EMITTER(python)
TEST_EMITTER(racket)
TEST_EMITTER(r)
TEST_EMITTER(ruby)
TEST_EMITTER(rust)
TEST_EMITTER(scala)
TEST_EMITTER(scheme)
TEST_EMITTER(swift)
TEST_EMITTER(tcl)
TEST_EMITTER(typescript)
TEST_EMITTER(vlang)
TEST_EMITTER(webassembly)
TEST_EMITTER(zig)

SUITE(ffi_emitters_suite) {
  RUN_TEST(test_ffi_emit_ada);
  RUN_TEST(test_ffi_emit_clojure);
  RUN_TEST(test_ffi_emit_common_lisp);
  RUN_TEST(test_ffi_emit_cpp);
  RUN_TEST(test_ffi_emit_crystal);
  RUN_TEST(test_ffi_emit_csharp);
  RUN_TEST(test_ffi_emit_dart);
  RUN_TEST(test_ffi_emit_d);
  RUN_TEST(test_ffi_emit_delphi);
  RUN_TEST(test_ffi_emit_elixir);
  RUN_TEST(test_ffi_emit_erlang);
  RUN_TEST(test_ffi_emit_fortran);
  RUN_TEST(test_ffi_emit_fsharp);
  RUN_TEST(test_ffi_emit_go);
  RUN_TEST(test_ffi_emit_groovy);
  RUN_TEST(test_ffi_emit_haskell);
  RUN_TEST(test_ffi_emit_java);
  RUN_TEST(test_ffi_emit_julia);
  RUN_TEST(test_ffi_emit_kotlin);
  RUN_TEST(test_ffi_emit_lua);
  RUN_TEST(test_ffi_emit_matlab);
  RUN_TEST(test_ffi_emit_napi);
  RUN_TEST(test_ffi_emit_nim);
  RUN_TEST(test_ffi_emit_objc);
  RUN_TEST(test_ffi_emit_ocaml);
  RUN_TEST(test_ffi_emit_odin);
  RUN_TEST(test_ffi_emit_perl);
  RUN_TEST(test_ffi_emit_php);
  RUN_TEST(test_ffi_emit_python);
  RUN_TEST(test_ffi_emit_racket);
  RUN_TEST(test_ffi_emit_r);
  RUN_TEST(test_ffi_emit_ruby);
  RUN_TEST(test_ffi_emit_rust);
  RUN_TEST(test_ffi_emit_scala);
  RUN_TEST(test_ffi_emit_scheme);
  RUN_TEST(test_ffi_emit_swift);
  RUN_TEST(test_ffi_emit_tcl);
  RUN_TEST(test_ffi_emit_typescript);
  RUN_TEST(test_ffi_emit_vlang);
  RUN_TEST(test_ffi_emit_webassembly);
  RUN_TEST(test_ffi_emit_zig);
}

#ifdef __cplusplus
}
#endif

#endif
