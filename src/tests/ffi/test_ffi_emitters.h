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
  if (!ir)
    return NULL;

  ir->nodes = (cdd_ffi_ir_node_t *)calloc(10, sizeof(cdd_ffi_ir_node_t));
  ir->nodes_capacity = 10;
  ir->nodes_count = 5;

  /* node 0: Struct */
  ir->nodes[0].kind = CDD_FFI_NODE_STRUCT;
  ir->nodes[0].name = "TestStruct";
  ir->nodes[0].fields = (cdd_ffi_field_t *)calloc(2, sizeof(cdd_ffi_field_t));
  ir->nodes[0].fields_count = 2;
  ir->nodes[0].fields[0].name = "field1";
  ir->nodes[0].fields[0].type.kind = CDD_FFI_KIND_INT32;
  ir->nodes[0].fields[1].name = "field2";
  ir->nodes[0].fields[1].type.kind = CDD_FFI_KIND_FLOAT32;
  ir->nodes[0].fields[1].type.pointer_depth = 1;

  /* node 1: Enum */
  ir->nodes[1].kind = CDD_FFI_NODE_ENUM;
  ir->nodes[1].name = "TestEnum";
  ir->nodes[1].variants =
      (cdd_ffi_enum_variant_t *)calloc(2, sizeof(cdd_ffi_enum_variant_t));
  ir->nodes[1].variants_count = 2;
  ir->nodes[1].variants[0].name = "VARIANT_A";
  ir->nodes[1].variants[1].name = "VARIANT_B";

  /* node 2: Function (void return) */
  ir->nodes[2].kind = CDD_FFI_NODE_FUNCTION;
  ir->nodes[2].name = "test_func_void";
  ir->nodes[2].return_or_base_type.kind = CDD_FFI_KIND_VOID;
  ir->nodes[2].fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  ir->nodes[2].fields_count = 1;
  ir->nodes[2].fields[0].name = "param1";
  ir->nodes[2].fields[0].type.kind = CDD_FFI_KIND_STRUCT_REF;
  ir->nodes[2].fields[0].type.ref_name = "TestStruct";

  /* node 3: Function (int return, no params) */
  ir->nodes[3].kind = CDD_FFI_NODE_FUNCTION;
  ir->nodes[3].name = "test_func_int";
  ir->nodes[3].return_or_base_type.kind = CDD_FFI_KIND_INT32;
  ir->nodes[3].fields_count = 0;

  /* node 4: Union */
  ir->nodes[4].kind = CDD_FFI_NODE_UNION;
  ir->nodes[4].name = "TestUnion";

  return ir;
}

static void free_dummy_ir(cdd_ffi_ir_t *ir) {
  if (!ir)
    return;
  free(ir->nodes[0].fields);
  free(ir->nodes[1].variants);
  free(ir->nodes[2].fields);
  free(ir->nodes);
  free(ir);
}

#define TEST_EMITTER(lang)                                                     \
  TEST test_ffi_emit_##lang(void) {                                            \
    cdd_ffi_ir_t *ir = create_dummy_ir();                                      \
    cdd_generate_bindings_config_t config = {0};                               \
    config.output_dir = "build/test_out_dir";                                  \
    config.library_name = "testlib";                                           \
    config.module_name = "TestMod";                                            \
                                                                               \
    cdd_ffi_emit_##lang(NULL, &config);                                        \
                                                                               \
    config.output_dir = NULL;                                                  \
    cdd_ffi_emit_##lang(ir, &config);                                          \
    config.output_dir = "build/test_out_dir";                                  \
                                                                               \
    cdd_ffi_emit_##lang(ir, &config);                                          \
                                                                               \
    /* Also test with no fields in struct/func */                              \
    ir->nodes[0].fields_count = 0;                                             \
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
