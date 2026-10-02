/**
 * @file test_ffi_emitters_edge.h
 * @brief Edge cases and IO failure tests for FFI language emitters.
 *
 * @author Samuel Marks
 */

#ifndef TEST_FFI_EMITTERS_EDGE_H
#define TEST_FFI_EMITTERS_EDGE_H

/* clang-format off */
#include "ffi/test_ffi_emitters.h"
#include <greatest.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

extern C_CDD_EXPORT int g_fail_io_after;

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

TEST test_ffi_emit_java_pom_dir(void) {
  cdd_ffi_ir_t ir = {0};
  cdd_generate_bindings_config_t config = {0};

#ifdef _WIN32
  _mkdir("test_pom_dir");
  _mkdir("test_pom_dir/pom.xml");
#else
  mkdir("test_pom_dir", 0777);
  mkdir("test_pom_dir/pom.xml", 0777);
#endif

  config.output_dir = (char *)(size_t)(size_t)(size_t) "test_pom_dir";
  cdd_ffi_emit_java(&ir, &config);

#ifdef _WIN32
  _rmdir("test_pom_dir/pom.xml");
  _rmdir("test_pom_dir");
#else
  rmdir("test_pom_dir/pom.xml");
  rmdir("test_pom_dir");
#endif

  PASS();
}

TEST test_ffi_emit_matlab_m_dir(void) {
  cdd_ffi_ir_t ir = {0};
  cdd_generate_bindings_config_t config = {0};

#ifdef _WIN32
  _mkdir("test_m_dir");
  _mkdir("test_m_dir/mylib.m");
#else
  mkdir("test_m_dir", 0777);
  mkdir("test_m_dir/mylib.m", 0777);
#endif

  config.output_dir = (char *)(size_t)(size_t)(size_t) "test_m_dir";
  cdd_ffi_emit_matlab(&ir, &config);

#ifdef _WIN32
  _rmdir("test_m_dir/mylib.m");
  _rmdir("test_m_dir");
#else
  rmdir("test_m_dir/mylib.m");
  rmdir("test_m_dir");
#endif

  PASS();
}

TEST test_ffi_emit_napi_dir(void) {
  cdd_ffi_ir_t ir = {0};
  cdd_generate_bindings_config_t config = {0};

#ifdef _WIN32
  _mkdir("test_napi_dir");
  _mkdir("test_napi_dir/test_test_Lib_name.js");
  _mkdir("test_napi_dir/binding.gyp");
#else
  mkdir("test_napi_dir", 0777);
  mkdir("test_napi_dir/test_test_Lib_name.js", 0777);
  mkdir("test_napi_dir/binding.gyp", 0777);
#endif

  config.output_dir = (char *)(size_t)(size_t)(size_t) "test_napi_dir";
  config.library_name = (char *)(size_t)(size_t)(size_t) "test_Lib_name";
  config.generate_tests = 1;
  cdd_ffi_emit_napi(&ir, &config);

#ifdef _WIN32
  _rmdir("test_napi_dir/binding.gyp");
  _rmdir("test_napi_dir/test_test_Lib_name.js");
  _rmdir("test_napi_dir");
#else
  rmdir("test_napi_dir/binding.gyp");
  rmdir("test_napi_dir/test_test_Lib_name.js");
  rmdir("test_napi_dir");
#endif

  PASS();
}

TEST test_ffi_emit_objc_dir(void) {
  cdd_ffi_ir_t ir = {0};
  cdd_generate_bindings_config_t config = {0};
  int rc = 0;

#ifdef _WIN32
  _mkdir("test_objc_dir_new");
  _mkdir("test_objc_dir_new/Bindings.m");
#else
  mkdir("test_objc_dir_new", 0777);
  mkdir("test_objc_dir_new/Bindings.m", 0777);
#endif

  config.output_dir = (char *)(size_t)(size_t)(size_t) "test_objc_dir_new";
  config.library_name = (char *)(size_t)(size_t)(size_t) "MyLib";
  rc = cdd_ffi_emit_objc(&ir, &config);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

#ifdef _WIN32
  remove("test_objc_dir_new/Bindings.h");
  _rmdir("test_objc_dir_new/Bindings.m");
  _rmdir("test_objc_dir_new");
#else
  remove("test_objc_dir_new/Bindings.h");
  rmdir("test_objc_dir_new/Bindings.m");
  rmdir("test_objc_dir_new");
#endif

  PASS();
}

TEST test_ffi_emit_perl_dir(void) {
  cdd_ffi_ir_t ir = {0};
  cdd_generate_bindings_config_t config = {0};
  int rc = 0;

#ifdef _WIN32
  _mkdir("test_perl_dir");
  _mkdir("test_perl_dir/Bindings.xs");
  _mkdir("test_perl_dir/Makefile.PL");
  _mkdir("test_perl_dir/typemap");
#else
  mkdir("test_perl_dir", 0777);
  mkdir("test_perl_dir/Bindings.xs", 0777);
#endif

  config.output_dir = (char *)(size_t)(size_t)(size_t) "test_perl_dir";
  config.module_name = (char *)(size_t)(size_t)(size_t) "Bindings";
  rc = cdd_ffi_emit_perl(&ir, &config);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

#ifdef _WIN32
  _rmdir("test_perl_dir/Bindings.xs");
  _mkdir("test_perl_dir/Makefile.PL");
#else
  rmdir("test_perl_dir/Bindings.xs");
  mkdir("test_perl_dir/Makefile.PL", 0777);
#endif
  rc = cdd_ffi_emit_perl(&ir, &config);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

#ifdef _WIN32
  _rmdir("test_perl_dir/Makefile.PL");
  _mkdir("test_perl_dir/typemap");
#else
  rmdir("test_perl_dir/Makefile.PL");
  mkdir("test_perl_dir/typemap", 0777);
#endif
  rc = cdd_ffi_emit_perl(&ir, &config);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

#ifdef _WIN32
  remove("test_perl_dir/Bindings.pm");
  remove("test_perl_dir/Bindings.xs");
  remove("test_perl_dir/Makefile.PL");
  _rmdir("test_perl_dir/typemap");
  _rmdir("test_perl_dir");
#else
  remove("test_perl_dir/Bindings.pm");
  remove("test_perl_dir/Bindings.xs");
  remove("test_perl_dir/Makefile.PL");
  rmdir("test_perl_dir/typemap");
  rmdir("test_perl_dir");
#endif

  PASS();
}

TEST test_ffi_emit_csharp_makedir(void) {
  cdd_ffi_ir_t *ir = create_dummy_ir();
  cdd_generate_bindings_config_t config = {0};
  const char *test_dir = "test_out_dir_cs_new";
  cdd_c_error_t rc = 0;

  remove("test_out_dir_cs_new/Bindings.cs");
  remove("test_out_dir_cs_new/BindingsTests.cs");
  remove("test_out_dir_cs_new/test_Lib_nameBindings.csproj");
  remove("test_out_dir_cs_new/TestMod.cs");
#ifdef _WIN32
  _rmdir(test_dir);
#else
  rmdir(test_dir);
#endif

  config.input = (char *)(size_t)(size_t) "my_input.h";
  config.output_dir = (char *)(size_t)(size_t)test_dir;
  config.library_name = (char *)(size_t)(size_t) "test_Lib_name";
  config.module_name = (char *)(size_t)(size_t) "TestMod";
  config.generate_tests = 1;

  rc = cdd_ffi_emit_csharp(ir, &config);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  remove("test_out_dir_cs_new/Bindings.cs");
  remove("test_out_dir_cs_new/BindingsTests.cs");
  remove("test_out_dir_cs_new/test_Lib_nameBindings.csproj");
  remove("test_out_dir_cs_new/TestMod.cs");
#ifdef _WIN32
  _rmdir(test_dir);
#else
  rmdir(test_dir);
#endif
  free_dummy_ir(ir);
  g_fail_io_after = -1;

  PASS();
}

TEST test_ffi_emit_cpp_trampoline_edge_cases(void) {
  cdd_ffi_ir_t ir = {0};
  cdd_ffi_ir_node_t nodes[2];
  cdd_generate_bindings_config_t config = {0};
  char long_name[300];
  const char *test_dir = "test_out_cpp_edge";
  memset(nodes, 0, sizeof(nodes));
  memset(long_name, 'A', sizeof(long_name) - 1);
  long_name[sizeof(long_name) - 1] = '\0';
  memcpy(long_name + sizeof(long_name) - 12, "_Trampoline", 11);

  nodes[0].kind = CDD_FFI_NODE_STRUCT;
  nodes[0].name = (char *)(size_t) "_Trampoline";

  nodes[1].kind = CDD_FFI_NODE_STRUCT;
  nodes[1].name = long_name;

  ir.nodes = nodes;
  ir.nodes_count = 2;

#ifdef _WIN32
  _mkdir(test_dir);
#else
  mkdir(test_dir, 0777);
#endif

  config.input = (char *)(size_t)(size_t) "my_input.h";
  config.output_dir = (char *)(size_t)(size_t)test_dir;
  config.library_name = (char *)(size_t)(size_t) "test_Lib_name";
  config.module_name = (char *)(size_t)(size_t) "TestMod";

  ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_emit_cpp(&ir, &config));

  remove("test_out_cpp_edge/test_Lib_name.hpp");
#ifdef _WIN32
  _rmdir(test_dir);
#else
  rmdir(test_dir);
#endif

  g_fail_io_after = -1;
  PASS();
}

TEST test_ffi_emit_rust_fresh_dir(void) {
  cdd_ffi_ir_t *ir = create_dummy_ir();
  cdd_generate_bindings_config_t config = {0};
  const char *test_dir = "test_out_rust_fresh";
  cdd_c_error_t rc = 0;

  remove("test_out_rust_fresh/src/lib.rs");
  remove("test_out_rust_fresh/src/sys.rs");
  remove("test_out_rust_fresh/tests/test.rs");
  remove("test_out_rust_fresh/Cargo.toml");
#ifdef _WIN32
  _rmdir("test_out_rust_fresh/src");
  _rmdir("test_out_rust_fresh/tests");
  _mkdir(test_dir);
#else
  rmdir("test_out_rust_fresh/src");
  rmdir("test_out_rust_fresh/tests");
  mkdir(test_dir, 0777);
#endif

  config.input = (char *)(size_t)(size_t) "my_input.h";
  config.output_dir = (char *)(size_t)(size_t)test_dir;
  config.library_name = (char *)(size_t)(size_t) "test_Lib_name";
  config.module_name = (char *)(size_t)(size_t) "TestMod";
  config.generate_tests = 1;

  rc = cdd_ffi_emit_rust(ir, &config);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  {
    int k;
    for (k = 1; k <= 4; k++) {
      g_fail_io_after = k;
      rc = cdd_ffi_emit_rust(ir, &config);
      ASSERT_EQ(CDD_C_ERROR_IO, rc);
    }
  }

  remove("test_out_rust_fresh/src/lib.rs");
  remove("test_out_rust_fresh/src/sys.rs");
  remove("test_out_rust_fresh/tests/test.rs");
  remove("test_out_rust_fresh/tests/integration_test.rs");
  remove("test_out_rust_fresh/Cargo.toml");
#ifdef _WIN32
  _rmdir("test_out_rust_fresh/src");
  _rmdir("test_out_rust_fresh/tests");
  _rmdir(test_dir);
#else
  rmdir("test_out_rust_fresh/src");
  rmdir("test_out_rust_fresh/tests");
  rmdir(test_dir);
#endif
  free_dummy_ir(ir);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Test IO failure in Rust emitter when files fail to open (f is NULL).
 *
 * @return GREATEST test result.
 */
TEST test_ffi_emit_rust_io_null(void) {
  cdd_ffi_ir_t *ir = create_dummy_ir();
  cdd_generate_bindings_config_t config = {0};
  const char *test_dir = "test_out_rust_null";
  cdd_c_error_t rc = 0;

#ifdef _WIN32
  _mkdir(test_dir);
  _mkdir("test_out_rust_null/src");
  _mkdir("test_out_rust_null/tests");
#else
  mkdir(test_dir, 0777);
  mkdir("test_out_rust_null/src", 0777);
  mkdir("test_out_rust_null/tests", 0777);
#endif

  config.input = (char *)(size_t)(size_t) "my_input.h";
  config.output_dir = (char *)(size_t)(size_t)test_dir;
  config.library_name = (char *)(size_t)(size_t) "test_Lib_name";
  config.module_name = (char *)(size_t)(size_t) "TestMod";
  config.generate_tests = 1;

  /* 1. Cargo.toml is a dir -> fopen returns NULL -> g_fail_io_after triggers
   * with f == NULL */
#ifdef _WIN32
  _mkdir("test_out_rust_null/Cargo.toml");
#else
  mkdir("test_out_rust_null/Cargo.toml", 0777);
#endif
  g_fail_io_after = 1;
  rc = cdd_ffi_emit_rust(ir, &config);
  ASSERT_EQ(CDD_C_ERROR_IO, rc);
#ifdef _WIN32
  _rmdir("test_out_rust_null/Cargo.toml");
#else
  rmdir("test_out_rust_null/Cargo.toml");
#endif

  /* 2. src/sys.rs is a dir -> fopen returns NULL -> g_fail_io_after triggers
   * with f == NULL */
#ifdef _WIN32
  _mkdir("test_out_rust_null/src/sys.rs");
#else
  mkdir("test_out_rust_null/src/sys.rs", 0777);
#endif
  g_fail_io_after = 2;
  rc = cdd_ffi_emit_rust(ir, &config);
  ASSERT_EQ(CDD_C_ERROR_IO, rc);
  remove("test_out_rust_null/Cargo.toml");
#ifdef _WIN32
  _rmdir("test_out_rust_null/src/sys.rs");
#else
  rmdir("test_out_rust_null/src/sys.rs");
#endif

  /* 3. src/lib.rs is a dir -> fopen returns NULL -> g_fail_io_after triggers
   * with f == NULL */
#ifdef _WIN32
  _mkdir("test_out_rust_null/src/lib.rs");
#else
  mkdir("test_out_rust_null/src/lib.rs", 0777);
#endif
  g_fail_io_after = 3;
  rc = cdd_ffi_emit_rust(ir, &config);
  ASSERT_EQ(CDD_C_ERROR_IO, rc);
  remove("test_out_rust_null/Cargo.toml");
  remove("test_out_rust_null/src/sys.rs");
#ifdef _WIN32
  _rmdir("test_out_rust_null/src/lib.rs");
#else
  rmdir("test_out_rust_null/src/lib.rs");
#endif

  /* 4. tests/integration_test.rs is a dir -> fopen returns NULL ->
   * g_fail_io_after triggers with f == NULL */
#ifdef _WIN32
  _mkdir("test_out_rust_null/tests/integration_test.rs");
#else
  mkdir("test_out_rust_null/tests/integration_test.rs", 0777);
#endif
  g_fail_io_after = 4;
  rc = cdd_ffi_emit_rust(ir, &config);
  ASSERT_EQ(CDD_C_ERROR_IO, rc);
  remove("test_out_rust_null/Cargo.toml");
  remove("test_out_rust_null/src/sys.rs");
  remove("test_out_rust_null/src/lib.rs");
#ifdef _WIN32
  _rmdir("test_out_rust_null/tests/integration_test.rs");
  _rmdir("test_out_rust_null/tests");
  _rmdir("test_out_rust_null/src");
  _rmdir(test_dir);
#else
  rmdir("test_out_rust_null/tests/integration_test.rs");
  rmdir("test_out_rust_null/tests");
  rmdir("test_out_rust_null/src");
  rmdir(test_dir);
#endif

  free_dummy_ir(ir);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Test IO failure when writing deps.edn in Clojure emitter.
 *
 * @return GREATEST test result.
 */
TEST test_ffi_emit_clojure_io_fail(void) {
  cdd_ffi_ir_t *ir = create_dummy_ir();
  cdd_generate_bindings_config_t config = {0};
  const char *test_dir = "test_out_clj_io";
  cdd_c_error_t rc = 0;

#ifdef _WIN32
  _mkdir(test_dir);
#else
  mkdir(test_dir, 0777);
#endif

  config.input = (char *)(size_t)(size_t) "my_input.h";
  config.output_dir = (char *)(size_t)(size_t)test_dir;
  config.library_name = (char *)(size_t)(size_t) "test_Lib_name";
  config.module_name = (char *)(size_t)(size_t) "TestMod";

  /* Case 1: f is non-NULL when g_fail_io_after == 556 */
  g_fail_io_after = 556;
  rc = cdd_ffi_emit_clojure(ir, &config);
  ASSERT_EQ(CDD_C_ERROR_IO, rc);

  remove("test_out_clj_io/TestMod.clj");
  remove("test_out_clj_io/deps.edn");

  /* Case 2: deps.edn is a directory so f is NULL when g_fail_io_after == 556 */
#ifdef _WIN32
  _mkdir("test_out_clj_io/deps.edn");
#else
  mkdir("test_out_clj_io/deps.edn", 0777);
#endif
  g_fail_io_after = 556;
  rc = cdd_ffi_emit_clojure(ir, &config);
  ASSERT_EQ(CDD_C_ERROR_IO, rc);

  g_fail_io_after = -1;
  remove("test_out_clj_io/TestMod.clj");
#ifdef _WIN32
  _rmdir("test_out_clj_io/deps.edn");
  _rmdir(test_dir);
#else
  rmdir("test_out_clj_io/deps.edn");
  rmdir(test_dir);
#endif
  free_dummy_ir(ir);
  PASS();
}

/**
 * @brief Test IO failure when writing module.modulemap in Swift emitter.
 *
 * @return GREATEST test result.
 */
TEST test_ffi_emit_swift_io_fail(void) {
  cdd_ffi_ir_t *ir = create_dummy_ir();
  cdd_generate_bindings_config_t config = {0};
  const char *test_dir = "test_out_swift_io";
  cdd_c_error_t rc = 0;

#ifdef _WIN32
  _mkdir(test_dir);
#else
  mkdir(test_dir, 0777);
#endif

  config.input = (char *)(size_t)(size_t) "my_input.h";
  config.output_dir = (char *)(size_t)(size_t)test_dir;
  config.library_name = (char *)(size_t)(size_t) "test_Lib_name";
  config.module_name = (char *)(size_t)(size_t) "TestMod";

  /* Case 1: f is non-NULL when g_fail_io_after triggers on module.modulemap */
  g_fail_io_after = 2;
  rc = cdd_ffi_emit_swift(ir, &config);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

  remove("test_out_swift_io/TestMod.swift");
  remove("test_out_swift_io/module.modulemap");

  /* Case 2: module.modulemap is a directory so f is NULL when g_fail_io_after
   * triggers */
#ifdef _WIN32
  _mkdir("test_out_swift_io/module.modulemap");
#else
  mkdir("test_out_swift_io/module.modulemap", 0777);
#endif
  g_fail_io_after = 2;
  rc = cdd_ffi_emit_swift(ir, &config);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

  g_fail_io_after = -1;
  remove("test_out_swift_io/TestMod.swift");
#ifdef _WIN32
  _rmdir("test_out_swift_io/module.modulemap");
  _rmdir(test_dir);
#else
  rmdir("test_out_swift_io/module.modulemap");
  rmdir(test_dir);
#endif
  free_dummy_ir(ir);
  PASS();
}

#ifdef CDD_BUILD_TESTS

#endif

SUITE(ffi_emitters_edge_suite) {
  RUN_TEST(test_ffi_emit_clojure_io_fail);
  RUN_TEST(test_ffi_emit_cpp_trampoline_edge_cases);
  RUN_TEST(test_ffi_emit_csharp_makedir);
#ifdef CDD_BUILD_TESTS
#endif
  RUN_TEST(test_ffi_emit_java_pom_dir);
  RUN_TEST(test_ffi_emit_matlab_m_dir);
  RUN_TEST(test_ffi_emit_napi_dir);
  RUN_TEST(test_ffi_emit_objc_dir);
  RUN_TEST(test_ffi_emit_perl_dir);
  RUN_TEST(test_ffi_emit_rust_fresh_dir);
  RUN_TEST(test_ffi_emit_rust_io_null);
  RUN_TEST(test_ffi_emit_swift_io_fail);
}

#ifdef __cplusplus
}
#endif

#endif /* !TEST_FFI_EMITTERS_EDGE_H */
