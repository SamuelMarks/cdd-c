/**
 * @file test_ffi_ocaml_fortran_cobol.h
 * @brief Comprehensive unit, branch, OOM, I/O fault-injection, DSL E2E, and
 * compiler smoke tests for FFI ABI layout, comment translation, OCaml, Fortran,
 * and COBOL emitters.
 */
#ifndef TEST_FFI_OCAML_FORTRAN_COBOL_H
/** @brief Header guard for test_ffi_ocaml_fortran_cobol.h */
#define TEST_FFI_OCAML_FORTRAN_COBOL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd/memory.h"
#include "c_cdd/no_discard.h"
#include "c_cdd/safe_crt.h"
#include "cdd_api.h"
#include "cdd_c_error.h"
#include "dsl/cdd_dsl_parser.h"
#include "dsl/cdd_dsl_tokenizer.h"
#include "dsl/cdd_ffi_emit_dsl.h"
#include "ffi/cdd_ffi_ir.h"
#include "functions/ffi/cdd_ffi_emit_cobol.h"
#include "functions/ffi/cdd_ffi_emit_common.h"
#include "functions/ffi/cdd_ffi_emit_fortran.h"
#include "functions/ffi/cdd_ffi_emit_ocaml.h"
#include "functions/ffi/cdd_ffi_ir_extractor.h"
#include "functions/parse/fs.h"
#include <greatest.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_MSC_VER)
#include <io.h>
#else
#include <unistd.h>
#endif
/* clang-format on */

/**
 * @brief Reads an entire generated artifact file into a newly `malloc`'d NUL
 * string for test assertions.
 *
 * @param[in] path Path to generated file.
 * @param[out] out_text Receives `malloc`'d file text (caller `free`s).
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO on failure.
 */
static NO_DISCARD cdd_c_error_t ofc_read_text_file(const char *path,
                                                   char **out_text) {
  FILE *fp = NULL;
  long fsz;
  size_t nread;
  char *buf;

  if (!path || !out_text)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_text = NULL;

#if defined(_MSC_VER)
  if (fopen_s(&fp, path, "r") != 0)
    fp = NULL;
#else
  fp = fopen(path, "r");
#endif
  if (!fp)
    return CDD_C_ERROR_IO;

  if (fseek(fp, 0, SEEK_END) != 0) {
    fclose(fp);
    return CDD_C_ERROR_IO;
  }
  fsz = ftell(fp);
  if (fsz < 0) {
    fclose(fp);
    return CDD_C_ERROR_IO;
  }
  if (fseek(fp, 0, SEEK_SET) != 0) {
    fclose(fp);
    return CDD_C_ERROR_IO;
  }

  buf = (char *)malloc((size_t)fsz + 1u);
  if (!buf) {
    fclose(fp);
    return CDD_C_ERROR_MEMORY;
  }
  nread = fread(buf, 1, (size_t)fsz, fp);
  buf[nread] = '\0';
  fclose(fp);
  *out_text = buf;
  return CDD_C_SUCCESS;
}

/**
 * @brief Tests `cdd_ffi_abi_profile_init` and `cdd_ffi_ir_compute_layout`
 * across LP64, LLP64, ILP32, padding, packed structs, bitfields, nested
 * structs, arrays, unions, cycles, unknown refs, overflows, and OOM.
 *
 * @return Greatest test result.
 */
TEST test_ffi_abi_layout_calculator(void) {
  cdd_ffi_abi_profile_t lp64;
  cdd_ffi_abi_profile_t llp64;
  cdd_ffi_abi_profile_t ilp32;
  cdd_ffi_abi_profile_t host_p;
  cdd_ffi_abi_profile_t bad_p;
  cdd_ffi_ir_t ir;
  cdd_ffi_ir_node_t nodes[8];
  cdd_ffi_field_t pad_fields[3];
  cdd_ffi_field_t bf_fields[3];
  cdd_ffi_field_t abi_fields[8];
  cdd_ffi_field_t u_fields[2];
  cdd_ffi_field_t fn_fields[1];
  cdd_c_error_t rc;

  /* 1. Profile initialization and NULL/invalid checks */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ffi_abi_profile_init(CDD_FFI_ABI_LP64, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ffi_abi_profile_init((cdd_ffi_abi_kind_t)99, &lp64));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_abi_profile_init(CDD_FFI_ABI_HOST, &host_p));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_abi_profile_init(CDD_FFI_ABI_LP64, &lp64));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_abi_profile_init(CDD_FFI_ABI_LLP64, &llp64));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_abi_profile_init(CDD_FFI_ABI_ILP32, &ilp32));
  ASSERT_EQ(8u, lp64.pointer_size);
  ASSERT_EQ(8u, lp64.long_size);
  ASSERT_EQ(8u, llp64.pointer_size);
  ASSERT_EQ(4u, llp64.long_size);
  ASSERT_EQ(4u, ilp32.pointer_size);
  ASSERT_EQ(4u, ilp32.int64_align);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ffi_ir_compute_layout(NULL, NULL));

  memset(&ir, 0, sizeof(ir));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_ir_compute_layout(&ir, NULL));

  memset(&bad_p, 0, sizeof(bad_p));
  bad_p.kind = CDD_FFI_ABI_LP64;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_ir_compute_layout(&ir, &bad_p));
  bad_p.pointer_size = 8;
  bad_p.pointer_align = 0;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ffi_ir_compute_layout(&ir, &bad_p));

  /* 2. Struct padding (`char` + `int32_t` + `char` -> 12 bytes) & packed (6) */
  memset(nodes, 0, sizeof(nodes));
  memset(pad_fields, 0, sizeof(pad_fields));
  memset(bf_fields, 0, sizeof(bf_fields));
  memset(abi_fields, 0, sizeof(abi_fields));
  memset(u_fields, 0, sizeof(u_fields));
  memset(fn_fields, 0, sizeof(fn_fields));

  pad_fields[0].name = cdd_test_mut_str("a");
  pad_fields[0].type.kind = CDD_FFI_KIND_INT8;
  pad_fields[1].name = cdd_test_mut_str("b");
  pad_fields[1].type.kind = CDD_FFI_KIND_INT32;
  pad_fields[2].name = cdd_test_mut_str("c");
  pad_fields[2].type.kind = CDD_FFI_KIND_INT8;

  nodes[0].kind = CDD_FFI_NODE_STRUCT;
  nodes[0].name = cdd_test_mut_str("PaddedStruct");
  nodes[0].fields = pad_fields;
  nodes[0].fields_count = 3;

  /* Node 1: Opaque empty struct */
  nodes[1].kind = CDD_FFI_NODE_STRUCT;
  nodes[1].name = cdd_test_mut_str("EmptyStruct");
  nodes[1].fields_count = 0;

  /* Node 2: Enum */
  nodes[2].kind = CDD_FFI_NODE_ENUM;
  nodes[2].name = cdd_test_mut_str("ColorEnum");

  /* Node 3: Typedef to int64 */
  nodes[3].kind = CDD_FFI_NODE_TYPEDEF;
  nodes[3].name = cdd_test_mut_str("MyI64");
  nodes[3].return_or_base_type.kind = CDD_FFI_KIND_INT64;

  /* Node 4: Union (`char[5]` + `int32_t` -> size 8, align 4) */
  u_fields[0].name = cdd_test_mut_str("raw5");
  u_fields[0].type.kind = CDD_FFI_KIND_UINT8;
  u_fields[0].type.array_size = 5;
  u_fields[1].name = cdd_test_mut_str("i32v");
  u_fields[1].type.kind = CDD_FFI_KIND_INT32;
  nodes[4].kind = CDD_FFI_NODE_UNION;
  nodes[4].name = cdd_test_mut_str("TestUnion");
  nodes[4].fields = u_fields;
  nodes[4].fields_count = 2;

  /* Node 5: Bitfield struct (3 + 5 bits share 4-byte int, 30 bits new int) */
  bf_fields[0].name = cdd_test_mut_str("b1");
  bf_fields[0].type.kind = CDD_FFI_KIND_UINT32;
  bf_fields[0].bitfield_width = 3;
  bf_fields[1].name = cdd_test_mut_str("b2");
  bf_fields[1].type.kind = CDD_FFI_KIND_UINT32;
  bf_fields[1].bitfield_width = 5;
  bf_fields[2].name = cdd_test_mut_str("b3");
  bf_fields[2].type.kind = CDD_FFI_KIND_UINT32;
  bf_fields[2].bitfield_width = 30;
  nodes[5].kind = CDD_FFI_NODE_STRUCT;
  nodes[5].name = cdd_test_mut_str("BitStruct");
  nodes[5].fields = bf_fields;
  nodes[5].fields_count = 3;

  /* Node 6: Composite ABI struct (`long`, `void *`, `PaddedStruct`, builtin
   * typedefs, `MyI64`, `ColorEnum`, `double`) */
  abi_fields[0].name = cdd_test_mut_str("f_long");
  abi_fields[0].type.kind = CDD_FFI_KIND_TYPEDEF_REF;
  abi_fields[0].type.ref_name = cdd_test_mut_str("long");
  abi_fields[1].name = cdd_test_mut_str("f_ptr");
  abi_fields[1].type.kind = CDD_FFI_KIND_OPAQUE_PTR;
  abi_fields[2].name = cdd_test_mut_str("f_nested");
  abi_fields[2].type.kind = CDD_FFI_KIND_STRUCT_REF;
  abi_fields[2].type.ref_name = cdd_test_mut_str("PaddedStruct");
  abi_fields[3].name = cdd_test_mut_str("f_sz");
  abi_fields[3].type.kind = CDD_FFI_KIND_TYPEDEF_REF;
  abi_fields[3].type.ref_name = cdd_test_mut_str("size_t");
  abi_fields[4].name = cdd_test_mut_str("f_my64");
  abi_fields[4].type.kind = CDD_FFI_KIND_TYPEDEF_REF;
  abi_fields[4].type.ref_name = cdd_test_mut_str("MyI64");
  abi_fields[5].name = cdd_test_mut_str("f_enum");
  abi_fields[5].type.kind = CDD_FFI_KIND_ENUM_REF;
  abi_fields[5].type.ref_name = cdd_test_mut_str("ColorEnum");
  abi_fields[6].name = cdd_test_mut_str("f_u16");
  abi_fields[6].type.kind = CDD_FFI_KIND_TYPEDEF_REF;
  abi_fields[6].type.ref_name = cdd_test_mut_str("uint16_t");
  abi_fields[7].name = cdd_test_mut_str("f_dbl");
  abi_fields[7].type.kind = CDD_FFI_KIND_TYPEDEF_REF;
  abi_fields[7].type.ref_name = cdd_test_mut_str("double");
  nodes[6].kind = CDD_FFI_NODE_STRUCT;
  nodes[6].name = cdd_test_mut_str("AbiComposite");
  nodes[6].fields = abi_fields;
  nodes[6].fields_count = 8;

  /* Node 7: Function + Macro exercise */
  fn_fields[0].name = cdd_test_mut_str("p0");
  fn_fields[0].type.kind = CDD_FFI_KIND_FLOAT32;
  nodes[7].kind = CDD_FFI_NODE_FUNCTION;
  nodes[7].name = cdd_test_mut_str("my_fn");
  nodes[7].return_or_base_type.kind = CDD_FFI_KIND_VOID;
  nodes[7].fields = fn_fields;
  nodes[7].fields_count = 1;

  ir.nodes = nodes;
  ir.nodes_count = 8;

  /* OOM on state allocation */
  g_cdd_alloc_fail = 1;
  rc = cdd_ffi_ir_compute_layout(&ir, &lp64);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  rc = cdd_ffi_ir_compute_layout(&ir, &lp64);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0u, pad_fields[0].offset_bytes);
  ASSERT_EQ(3u, pad_fields[1].pad_before_bytes);
  ASSERT_EQ(4u, pad_fields[1].offset_bytes);
  ASSERT_EQ(8u, pad_fields[2].offset_bytes);
  ASSERT_EQ(3u, nodes[0].trailing_pad_bytes);
  ASSERT_EQ(12u, nodes[0].size_bytes);
  ASSERT_EQ(4u, nodes[0].align_bytes);
  ASSERT_EQ(0u, nodes[1].size_bytes);
  ASSERT_EQ(8u, nodes[4].size_bytes);
  ASSERT_EQ(4u, nodes[4].align_bytes);
  ASSERT_EQ(3u, u_fields[0].union_tail_pad_bytes);
  ASSERT_EQ(4u, u_fields[1].union_tail_pad_bytes);
  ASSERT_EQ(8u, nodes[5].size_bytes);
  ASSERT_EQ(0u, bf_fields[1].size_bytes);
  ASSERT_EQ(0u, abi_fields[1].pad_before_bytes); /* LP64: 8-byte long + ptr */

  /* LLP64: 4-byte long followed by 8-byte pointer -> 4 bytes pad_before! */
  rc = cdd_ffi_ir_compute_layout(&ir, &llp64);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(4u, abi_fields[0].size_bytes);
  ASSERT_EQ(4u, abi_fields[1].pad_before_bytes);

  /* ILP32: 4-byte long followed by 4-byte pointer -> 0 bytes pad_before! */
  rc = cdd_ffi_ir_compute_layout(&ir, &ilp32);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(4u, abi_fields[0].size_bytes);
  ASSERT_EQ(4u, abi_fields[1].size_bytes);
  ASSERT_EQ(0u, abi_fields[1].pad_before_bytes);

  /* Packed struct (`size_bytes == 6`, `align_bytes == 1`) */
  nodes[0].is_packed = 1;
  rc = cdd_ffi_ir_compute_layout(&ir, &lp64);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0u, pad_fields[1].pad_before_bytes);
  ASSERT_EQ(0u, nodes[0].trailing_pad_bytes);
  ASSERT_EQ(6u, nodes[0].size_bytes);
  ASSERT_EQ(1u, nodes[0].align_bytes);
  nodes[0].is_packed = 0;

  /* Error branches: unknown STRUCT_REF, unknown TYPEDEF_REF, unknown ENUM_REF,
   * NULL ref_name, cycle, and array size overflow */
  abi_fields[2].type.ref_name = cdd_test_mut_str("MissingStruct");
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, cdd_ffi_ir_compute_layout(&ir, &lp64));
  abi_fields[2].type.ref_name = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ffi_ir_compute_layout(&ir, &lp64));
  abi_fields[2].type.ref_name = cdd_test_mut_str("PaddedStruct");

  abi_fields[4].type.ref_name = cdd_test_mut_str("MissingTypedef");
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, cdd_ffi_ir_compute_layout(&ir, &lp64));
  abi_fields[4].type.ref_name = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ffi_ir_compute_layout(&ir, &lp64));
  abi_fields[4].type.ref_name = cdd_test_mut_str("MyI64");

  abi_fields[5].type.ref_name = cdd_test_mut_str("MissingEnum");
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, cdd_ffi_ir_compute_layout(&ir, &lp64));
  abi_fields[5].type.ref_name = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ffi_ir_compute_layout(&ir, &lp64));
  abi_fields[5].type.ref_name = cdd_test_mut_str("ColorEnum");

  /* By-value cycle: PaddedStruct.fields[0] -> AbiComposite -> PaddedStruct */
  pad_fields[0].type.kind = CDD_FFI_KIND_STRUCT_REF;
  pad_fields[0].type.ref_name = cdd_test_mut_str("AbiComposite");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ffi_ir_compute_layout(&ir, &lp64));

  /* Invalid kind */
  pad_fields[0].type.kind = (cdd_ffi_primitive_kind_t)999;
  pad_fields[0].type.ref_name = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ffi_ir_compute_layout(&ir, &lp64));

  /* Unknown node kind falls back to zero-sized 1-byte aligned entry */
  pad_fields[0].type.kind = CDD_FFI_KIND_INT8;
  nodes[7].kind = (cdd_ffi_node_kind_t)99;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_ir_compute_layout(&ir, &lp64));
  ASSERT_EQ(0u, nodes[7].size_bytes);
  ASSERT_EQ(1u, nodes[7].align_bytes);

  {
    int sf;
    for (sf = 1; sf <= 35; ++sf) {
      g_cdd_step_fail = sf;
      g_cdd_last_alloc_err = cdd_ffi_ir_compute_layout(&ir, NULL);
      g_cdd_step_fail = 0;
    }
    for (sf = 1; sf <= 35; ++sf) {
      cdd_ffi_ir_t sort_ir;
      cdd_ffi_ir_node_t *snodes =
          (cdd_ffi_ir_node_t *)C_CDD_CALLOC(2, sizeof(cdd_ffi_ir_node_t));
      cdd_ffi_field_t sfield;
      memset(&sort_ir, 0, sizeof(sort_ir));
      memset(&sfield, 0, sizeof(sfield));
      snodes[0].kind = CDD_FFI_NODE_STRUCT;
      snodes[0].name = cdd_test_mut_str("S0");
      sfield.name = cdd_test_mut_str("f");
      sfield.type.kind = CDD_FFI_KIND_STRUCT_REF;
      sfield.type.ref_name = cdd_test_mut_str("S1");
      snodes[0].fields = &sfield;
      snodes[0].fields_count = 1;
      snodes[1].kind = CDD_FFI_NODE_STRUCT;
      snodes[1].name = cdd_test_mut_str("S1");
      sort_ir.nodes = snodes;
      sort_ir.nodes_count = 2;
      g_cdd_step_fail = sf;
      g_cdd_last_alloc_err = cdd_ffi_ir_topological_sort(&sort_ir);
      g_cdd_step_fail = 0;
      C_CDD_FREE(sort_ir.nodes);
    }
  }

  PASS();
}

/**
 * @brief Tests `cdd_ffi_write_*`, `cdd_ffi_close_output_file`,
 * `cdd_ffi_snprintf_checked`, `cdd_ffi_emit_translated_trivia`, and
 * `cdd_ffi_emit_translated_doc` including all NULL, truncation, nesting, word
 * wrapping, and `g_fail_io_after` branches.
 *
 * @return Greatest test result.
 */
TEST test_ffi_common_io_and_comment_translation(void) {
  char buf[16];
  FILE *fp = NULL;
  char *text = NULL;
  cdd_ffi_trivia_t t1;
  cdd_ffi_trivia_t t2;
  cdd_ffi_trivia_t t3;
  cdd_ffi_trivia_t t4;
  cdd_ffi_comment_style_t bad_style;
  cdd_ffi_comment_style_t ocaml_style = {NULL,  "(** ", " *)", "(*",
                                         "( *", "*)",   "* )", 0};
  cdd_ffi_comment_style_t fortran_style = {"! ", NULL, NULL, NULL,
                                           NULL, NULL, NULL, 32};
  cdd_ffi_comment_style_t cobol_style = {"*> ", NULL, NULL, NULL,
                                         NULL,  NULL, NULL, 0};
  int k;
  cdd_c_error_t rc;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_write_str(NULL, "hi"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_write_bytes(NULL, "hi", 2));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_close_output_file(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_close_output_file(&fp));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ffi_snprintf_checked(NULL, 10, "%s", "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ffi_snprintf_checked(buf, 0, "%s", "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ffi_snprintf_checked(buf, sizeof(buf), NULL));
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_ffi_snprintf_checked(buf, 4, "%s", "hello_world"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_write_fmt(NULL, "%s", "a"));
  ASSERT_EQ(CDD_C_SUCCESS, makedirs("test_ofc_out"));

  rc = cdd_ffi_open_output_file("test_ofc_out", "io_test.txt", &fp);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_write_str(fp, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_write_bytes(fp, NULL, 3));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_write_bytes(fp, NULL, 0));

  memset(&bad_style, 0, sizeof(bad_style));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ffi_emit_translated_trivia(NULL, NULL, &ocaml_style));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ffi_emit_translated_trivia(fp, NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ffi_emit_translated_trivia(fp, NULL, &bad_style));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ffi_emit_translated_doc(NULL, "doc", "", &ocaml_style));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ffi_emit_translated_doc(fp, "doc", "", &bad_style));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_emit_translated_doc(fp, NULL, NULL, &ocaml_style));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_emit_translated_doc(fp, "", NULL, &ocaml_style));

  memset(&t1, 0, sizeof(t1));
  memset(&t2, 0, sizeof(t2));
  memset(&t3, 0, sizeof(t3));
  memset(&t4, 0, sizeof(t4));
  t1.kind = CDD_FFI_TRIVIA_COMMENT_LINE;
  t1.text = cdd_test_mut_str("/// Single line with (* nested *) markers\n");
  t1.next = &t2;
  t2.kind = CDD_FFI_TRIVIA_WHITESPACE;
  t2.text = cdd_test_mut_str("\n\n");
  t2.next = &t3;
  t3.kind = CDD_FFI_TRIVIA_COMMENT_BLOCK;
  t3.text = cdd_test_mut_str("/**\n * Multi-line (* doc *)\n *\n * End\n */");
  t3.next = &t4;
  t4.kind = CDD_FFI_TRIVIA_CONTINUATION;
  t4.text = cdd_test_mut_str("\\\n");

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_emit_translated_trivia(fp, &t1, &ocaml_style));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_emit_translated_doc(
                fp,
                "/** Wrapped Fortran comment line with words and "
                "supercalifragilisticexpialidocious1234567890 */\n",
                "  ", &fortran_style));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_emit_translated_trivia(fp, &t1, &cobol_style));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_close_output_file(&fp));
  ASSERT(fp == NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_close_output_file(&fp));

  rc = ofc_read_text_file("test_ofc_out/io_test.txt", &text);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(strstr(text, "( * nested * )") != NULL);
  ASSERT(strstr(text, "( * doc * )") != NULL);
  ASSERT(strstr(text, "  ! Wrapped Fortran") != NULL);
  ASSERT(strstr(text, "*> Single line") != NULL);
  free(text);

  /* Fault-injection sweep across every write & close point */
  for (k = 1; k <= 35; k++) {
    FILE *f2 = NULL;
    g_fail_io_after = k;
    rc = cdd_ffi_open_output_file("test_ofc_out", "io_fail.txt", &f2);
    if (rc == CDD_C_SUCCESS) {
      rc = cdd_ffi_emit_translated_trivia(f2, &t1, &ocaml_style);
      if (rc == CDD_C_SUCCESS) {
        rc = cdd_ffi_emit_translated_doc(f2, "/** a\n * b */", "  ",
                                         &fortran_style);
      }
      {
        cdd_c_error_t rc_c = cdd_ffi_close_output_file(&f2);
        if (rc == CDD_C_SUCCESS && rc_c != CDD_C_SUCCESS)
          rc = rc_c;
      }
    }
    g_fail_io_after = -1;
  }

  PASS();
}

/**
 * @brief Tests typed function-pointer signature extraction, JSON round-trip,
 * and DSL round-trip (`fn_return_type`, `fn_params`, `fn_is_variadic`).
 *
 * @return Greatest test result.
 */
TEST test_ffi_typed_fnptr_roundtrip(void) {
  const char *hdr_path = "test_ofc_out/fnptr_demo.h";
  const char *hdr_src =
      "typedef int (*event_cb_t)(const char *msg, int code);\n"
      "struct PackedHook {\n"
      "  char tag;\n"
      "  int (*on_tick)(double dt);\n"
      "} __attribute__((packed));\n"
      "void register_hook(int (*cb)(int a, ...)) {}\n";
  cdd_generate_bindings_config_t cfg;
  cdd_ffi_ir_t *ir = NULL;
  cdd_ffi_ir_t *ir_json = NULL;
  cdd_ffi_ir_t *ir_dsl = NULL;
  char *json_str = NULL;
  char *dsl_text = NULL;
  cdd_dsl_token_list_t toks;
  size_t i;
  int saw_typedef_cb = 0;
  int saw_struct_cb = 0;
  int saw_param_cb = 0;
  cdd_c_error_t rc;

  ASSERT_EQ(CDD_C_SUCCESS, makedirs("test_ofc_out"));
  ASSERT_EQ(CDD_C_SUCCESS, write_to_file(hdr_path, hdr_src));

  memset(&cfg, 0, sizeof(cfg));
  rc = cdd_ffi_ir_extract_exports(hdr_path, hdr_src, &cfg, &ir);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(ir != NULL);

  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *n = &ir->nodes[i];
    if (n->kind == CDD_FFI_NODE_TYPEDEF && n->name &&
        strcmp(n->name, "event_cb_t") == 0) {
      ASSERT_EQ(CDD_FFI_KIND_FUNCTION_PTR, n->return_or_base_type.kind);
      ASSERT(n->return_or_base_type.fn_return_type != NULL);
      ASSERT_EQ(2u, n->return_or_base_type.fn_params_count);
      saw_typedef_cb = 1;
    } else if (n->kind == CDD_FFI_NODE_STRUCT && n->name &&
               strcmp(n->name, "PackedHook") == 0) {
      size_t f;
      ASSERT_EQ(1, n->is_packed);
      for (f = 0; f < n->fields_count; f++) {
        if (n->fields[f].name && strcmp(n->fields[f].name, "on_tick") == 0) {
          ASSERT_EQ(CDD_FFI_KIND_FUNCTION_PTR, n->fields[f].type.kind);
          ASSERT(n->fields[f].type.fn_return_type != NULL);
          ASSERT_EQ(1u, n->fields[f].type.fn_params_count);
          saw_struct_cb = 1;
        }
      }
    } else if (n->kind == CDD_FFI_NODE_FUNCTION && n->name &&
               strcmp(n->name, "register_hook") == 0 && n->fields_count == 1) {
      ASSERT_EQ(CDD_FFI_KIND_FUNCTION_PTR, n->fields[0].type.kind);
      ASSERT_EQ(1, n->fields[0].type.fn_is_variadic);
      ASSERT_EQ(1u, n->fields[0].type.fn_params_count);
      saw_param_cb = 1;
    }
  }
  ASSERT_EQ(1, saw_typedef_cb);
  ASSERT_EQ(1, saw_struct_cb);
  ASSERT_EQ(1, saw_param_cb);

  /* JSON roundtrip */
  rc = cdd_ffi_ir_to_json(ir, &json_str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(strstr(json_str, "\"fn_return_type\"") != NULL);
  ASSERT(strstr(json_str, "\"fn_params\"") != NULL);
  ASSERT(strstr(json_str, "\"fn_is_variadic\"") != NULL);

  rc = cdd_ffi_ir_from_json(json_str, &ir_json);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free(json_str);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_ir_free(ir_json));
  C_CDD_FREE(ir_json);

  /* DSL roundtrip */
  cfg.output_dir = "test_ofc_out";
  rc = cdd_ffi_emit_dsl(ir, &cfg);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = ofc_read_text_file("test_out_dsl.dsl", &dsl_text);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(strstr(dsl_text, "func(") != NULL);
  ASSERT(strstr(dsl_text, "va_list") != NULL);

  rc = cdd_dsl_tokenize(dsl_text, &toks);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_dsl_parse(&toks, &ir_dsl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_dsl_token_list_free(&toks));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_ir_free(ir_dsl));
  C_CDD_FREE(ir_dsl);
  free(dsl_text);
  remove("test_out_dsl.dsl");

  ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_ir_free(ir));
  C_CDD_FREE(ir);
  PASS();
}

/**
 * @brief Comprehensive unit, syntax, wrapper, OOM, and `g_fail_io_after` tests
 * for the OCaml emitter (`cdd_ffi_emit_ocaml`).
 *
 * @return Greatest test result.
 */
TEST test_ffi_ocaml_comprehensive(void) {
  char id_buf[64];
  cdd_ffi_ir_t ir;
  cdd_ffi_ir_node_t nodes[12];
  cdd_ffi_enum_variant_t evars[4];
  cdd_ffi_number_t c23_num;
  cdd_ffi_field_t s_fields[4];
  cdd_ffi_field_t u_fields[2];
  cdd_ffi_field_t cb_params[1];
  cdd_ffi_type_t cb_ret;
  cdd_ffi_field_t fn_out_fields[3];
  cdd_ffi_field_t fn_err_fields[2];
  cdd_generate_bindings_config_t cfg;
  char *ml_txt = NULL;
  char *mli_txt = NULL;
  char *dune_txt = NULL;
  char *stub_txt = NULL;
  char *test_txt = NULL;
  int k;
  cdd_c_error_t rc;

  /* Identifier & keyword sanitizers */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ocaml_sanitize_lid("a", NULL, 10));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ocaml_sanitize_lid("a", id_buf, 2));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ocaml_sanitize_uid("a", NULL, 10));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ocaml_sanitize_uid("a", id_buf, 2));

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ocaml_sanitize_lid("MyStruct", id_buf, sizeof(id_buf)));
  ASSERT_STR_EQ("my_struct", id_buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ocaml_sanitize_lid("type", id_buf, sizeof(id_buf)));
  ASSERT_STR_EQ("type_", id_buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ocaml_sanitize_lid("end", id_buf, sizeof(id_buf)));
  ASSERT_STR_EQ("end_", id_buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ocaml_sanitize_lid("val", id_buf, sizeof(id_buf)));
  ASSERT_STR_EQ("val_", id_buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ocaml_sanitize_lid("module", id_buf, sizeof(id_buf)));
  ASSERT_STR_EQ("module_", id_buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ocaml_sanitize_lid("9lives", id_buf, sizeof(id_buf)));
  ASSERT_STR_EQ("v_9lives", id_buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ocaml_sanitize_lid(NULL, id_buf, sizeof(id_buf)));
  ASSERT_STR_EQ("unnamed_", id_buf);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ocaml_sanitize_uid("my_enum", id_buf, sizeof(id_buf)));
  ASSERT_STR_EQ("My_enum", id_buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ocaml_sanitize_uid("9enum", id_buf, sizeof(id_buf)));
  ASSERT_STR_EQ("M_9enum", id_buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ocaml_sanitize_uid("---", id_buf, sizeof(id_buf)));
  ASSERT_STR_EQ("Unnamed", id_buf);

  memset(&cfg, 0, sizeof(cfg));
  memset(&ir, 0, sizeof(ir));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_emit_ocaml(NULL, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_emit_ocaml(&ir, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_emit_ocaml(&ir, &cfg));

  memset(nodes, 0, sizeof(nodes));
  memset(evars, 0, sizeof(evars));
  memset(&c23_num, 0, sizeof(c23_num));
  memset(s_fields, 0, sizeof(s_fields));
  memset(u_fields, 0, sizeof(u_fields));
  memset(cb_params, 0, sizeof(cb_params));
  memset(&cb_ret, 0, sizeof(cb_ret));
  memset(fn_out_fields, 0, sizeof(fn_out_fields));
  memset(fn_err_fields, 0, sizeof(fn_err_fields));

  /* 0..2: INT, FLOAT, STRING macros */
  nodes[0].kind = CDD_FFI_NODE_MACRO;
  nodes[0].name = cdd_test_mut_str("MAX_ITEMS");
  nodes[0].inferred_type = CDD_FFI_MACRO_TYPE_INT;
  nodes[0].evaluated_value = cdd_test_mut_str("1'000");

  nodes[1].kind = CDD_FFI_NODE_MACRO;
  nodes[1].name = cdd_test_mut_str("PI_APPROX");
  nodes[1].inferred_type = CDD_FFI_MACRO_TYPE_FLOAT;
  nodes[1].evaluated_value = cdd_test_mut_str("3.1415");

  nodes[2].kind = CDD_FFI_NODE_MACRO;
  nodes[2].name = cdd_test_mut_str("TAG_NAME");
  nodes[2].inferred_type = CDD_FFI_MACRO_TYPE_STRING;
  nodes[2].evaluated_value = cdd_test_mut_str("cdd");

  /* 3: Enum with C23 hex/oct/bin/digit-separator literals */
  c23_num.raw_spelling = cdd_test_mut_str("1'024");
  evars[0].name = cdd_test_mut_str("VAL_SEP");
  evars[0].number_details = &c23_num;
  evars[1].name = cdd_test_mut_str("VAL_HEX");
  evars[1].value = cdd_test_mut_str("0x1F");
  evars[2].name = cdd_test_mut_str("VAL_OCT");
  evars[2].value = cdd_test_mut_str("077");
  evars[3].name = cdd_test_mut_str("VAL_NEG");
  evars[3].value = cdd_test_mut_str("-5");
  nodes[3].kind = CDD_FFI_NODE_ENUM;
  nodes[3].name = cdd_test_mut_str("my_enum");
  nodes[3].variants = evars;
  nodes[3].variants_count = 4;

  /* 4: Opaque struct (fields_count == 0 -> no `seal`) */
  nodes[4].kind = CDD_FFI_NODE_STRUCT;
  nodes[4].name = cdd_test_mut_str("OpaqueCtx");
  nodes[4].fields_count = 0;

  /* 5: Sealed struct with keyword field, fixed array, typed ptr, callback */
  cb_params[0].name = cdd_test_mut_str("code");
  cb_params[0].type.kind = CDD_FFI_KIND_INT32;
  cb_ret.kind = CDD_FFI_KIND_BOOL;
  s_fields[0].name = cdd_test_mut_str("type");
  s_fields[0].type.kind = CDD_FFI_KIND_INT32;
  s_fields[1].name = cdd_test_mut_str("arr");
  s_fields[1].type.kind = CDD_FFI_KIND_FLOAT64;
  s_fields[1].type.array_size = 4;
  s_fields[2].name = cdd_test_mut_str("ctx_ptr");
  s_fields[2].type.kind = CDD_FFI_KIND_STRUCT_REF;
  s_fields[2].type.ref_name = cdd_test_mut_str("OpaqueCtx");
  s_fields[2].type.pointer_depth = 1;
  s_fields[3].name = cdd_test_mut_str("on_done");
  s_fields[3].type.kind = CDD_FFI_KIND_FUNCTION_PTR;
  s_fields[3].type.fn_return_type = &cb_ret;
  s_fields[3].type.fn_params = cb_params;
  s_fields[3].type.fn_params_count = 1;
  nodes[5].kind = CDD_FFI_NODE_STRUCT;
  nodes[5].name = cdd_test_mut_str("MyStruct");
  nodes[5].doc = cdd_test_mut_str("/** Struct with (* nested *) comment */");
  nodes[5].fields = s_fields;
  nodes[5].fields_count = 4;

  /* 6: Union */
  u_fields[0].name = cdd_test_mut_str("as_i64");
  u_fields[0].type.kind = CDD_FFI_KIND_INT64;
  u_fields[1].name = cdd_test_mut_str("as_f64");
  u_fields[1].type.kind = CDD_FFI_KIND_FLOAT64;
  nodes[6].kind = CDD_FFI_NODE_UNION;
  nodes[6].name = cdd_test_mut_str("MyUnion");
  nodes[6].fields = u_fields;
  nodes[6].fields_count = 2;

  /* 7: Typedef */
  nodes[7].kind = CDD_FFI_NODE_TYPEDEF;
  nodes[7].name = cdd_test_mut_str("HandleAlias");
  nodes[7].return_or_base_type.kind = CDD_FFI_KIND_STRUCT_REF;
  nodes[7].return_or_base_type.ref_name = cdd_test_mut_str("OpaqueCtx");
  nodes[7].return_or_base_type.pointer_depth = 1;

  /* 8: 0-arg function with runtime lock release */
  nodes[8].kind = CDD_FFI_NODE_FUNCTION;
  nodes[8].name = cdd_test_mut_str("MyZeroArg");
  nodes[8].return_or_base_type.kind = CDD_FFI_KIND_VOID;
  nodes[8].requires_gil_release = 1;

  /* 9: Function with `[in]`, `[out]`, and `[inout]` tuple wrapper */
  fn_out_fields[0].name = cdd_test_mut_str("label");
  fn_out_fields[0].type.kind = CDD_FFI_KIND_INT8;
  fn_out_fields[0].type.pointer_depth = 1;
  fn_out_fields[0].type.is_const = 1;
  fn_out_fields[0].intent = CDD_FFI_INTENT_IN;
  fn_out_fields[1].name = cdd_test_mut_str("out_num");
  fn_out_fields[1].type.kind = CDD_FFI_KIND_INT32;
  fn_out_fields[1].type.pointer_depth = 1;
  fn_out_fields[1].intent = CDD_FFI_INTENT_OUT;
  fn_out_fields[2].name = cdd_test_mut_str("io_rec");
  fn_out_fields[2].type.kind = CDD_FFI_KIND_STRUCT_REF;
  fn_out_fields[2].type.ref_name = cdd_test_mut_str("MyStruct");
  fn_out_fields[2].type.pointer_depth = 1;
  fn_out_fields[2].intent = CDD_FFI_INTENT_INOUT;
  nodes[9].kind = CDD_FFI_NODE_FUNCTION;
  nodes[9].name = cdd_test_mut_str("compute_pair");
  nodes[9].return_or_base_type.kind = CDD_FFI_KIND_INT32;
  nodes[9].fields = fn_out_fields;
  nodes[9].fields_count = 3;

  /* 10: Function with `cdd_ffi_error_t *` out-param -> `result` wrapper */
  fn_err_fields[0].name = cdd_test_mut_str("out_val");
  fn_err_fields[0].type.kind = CDD_FFI_KIND_FLOAT64;
  fn_err_fields[0].type.pointer_depth = 1;
  fn_err_fields[0].intent = CDD_FFI_INTENT_OUT;
  fn_err_fields[1].name = cdd_test_mut_str("err");
  fn_err_fields[1].type.kind = CDD_FFI_KIND_STRUCT_REF;
  fn_err_fields[1].type.ref_name = cdd_test_mut_str("cdd_ffi_error_t");
  fn_err_fields[1].type.pointer_depth = 1;
  fn_err_fields[1].intent = CDD_FFI_INTENT_OUT;
  nodes[10].kind = CDD_FFI_NODE_FUNCTION;
  nodes[10].name = cdd_test_mut_str("fallible_op");
  nodes[10].return_or_base_type.kind = CDD_FFI_KIND_VOID;
  nodes[10].fields = fn_err_fields;
  nodes[10].fields_count = 2;

  /* 11: Variadic function returning cdd_ffi_error_t by value */
  nodes[11].kind = CDD_FFI_NODE_FUNCTION;
  nodes[11].name = cdd_test_mut_str("var_err_op");
  nodes[11].return_or_base_type.kind = CDD_FFI_KIND_STRUCT_REF;
  nodes[11].return_or_base_type.ref_name = cdd_test_mut_str("cdd_ffi_error_t");
  nodes[11].is_variadic = 1;

  ir.nodes = nodes;
  ir.nodes_count = 12;

  ASSERT_EQ(CDD_C_SUCCESS, makedirs("test_ofc_out/ocaml"));
  cfg.output_dir = "test_ofc_out/ocaml";
  cfg.library_name = "demo_ocaml";
  cfg.generate_tests = 1;

  rc = cdd_ffi_emit_ocaml(&ir, &cfg);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  ASSERT_EQ(CDD_C_SUCCESS,
            ofc_read_text_file("test_ofc_out/ocaml/demo_ocaml.ml", &ml_txt));
  ASSERT_EQ(CDD_C_SUCCESS,
            ofc_read_text_file("test_ofc_out/ocaml/demo_ocaml.mli", &mli_txt));
  ASSERT_EQ(CDD_C_SUCCESS,
            ofc_read_text_file("test_ofc_out/ocaml/dune", &dune_txt));
  ASSERT_EQ(
      CDD_C_SUCCESS,
      ofc_read_text_file("test_ofc_out/ocaml/demo_ocaml_stubs.c", &stub_txt));
  ASSERT_EQ(
      CDD_C_SUCCESS,
      ofc_read_text_file("test_ofc_out/ocaml/demo_ocaml_test.ml", &test_txt));

  ASSERT(strstr(ml_txt, "Dl.dlopen") != NULL);
  ASSERT(strstr(ml_txt, "~from:_lib") != NULL);
  ASSERT(strstr(ml_txt, "let max_items = 1_000") != NULL);
  ASSERT(strstr(ml_txt, "module My_enum = struct") != NULL);
  ASSERT(strstr(ml_txt, "0o77") != NULL);
  ASSERT(strstr(ml_txt, "view ~read:of_int ~write:to_int int") != NULL);
  ASSERT(strstr(ml_txt, "type my_struct") != NULL);
  ASSERT(strstr(ml_txt, "(ptr opaque_ctx)") != NULL);
  ASSERT(strstr(ml_txt, "(Ctypes.array 4 double)") != NULL);
  ASSERT(strstr(ml_txt, "Foreign.funptr") != NULL);
  ASSERT(strstr(ml_txt, "union \"MyUnion\"") != NULL);
  ASSERT(strstr(ml_txt, "seal my_union") != NULL);
  ASSERT(strstr(ml_txt, "~release_runtime_lock:true \"MyZeroArg\" (void @-> "
                        "returning void)") != NULL);
  ASSERT(strstr(ml_txt, "Ctypes.allocate") != NULL);
  ASSERT(strstr(ml_txt, "(!@ _out_") != NULL);
  ASSERT(strstr(mli_txt, "result") != NULL);
  ASSERT(strstr(dune_txt, "(libraries ctypes ctypes.foreign)") != NULL);
  ASSERT(strstr(stub_txt, "caml_release_runtime_system") != NULL);
  ASSERT(strstr(test_txt, "sizeof Demo_ocaml.my_struct > 0") != NULL);

  free(ml_txt);
  free(mli_txt);
  free(dune_txt);
  free(stub_txt);
  free(test_txt);

  /* Full g_fail_io_after stream error sweep */
  for (k = 1; k <= 2000; k++) {
    g_fail_io_after = k;
    rc = cdd_ffi_emit_ocaml(&ir, &cfg);
    g_fail_io_after = -1;
    if (rc == CDD_C_SUCCESS)
      break;
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }

  PASS();
}

/**
 * @brief Comprehensive unit, ABI (`VALUE` vs `INTENT`), line-wrapping, BOZ
 * literal, union `TRANSFER`, callback `ABSTRACT INTERFACE`, string wrapper,
 * OOM, and `g_fail_io_after` tests for `cdd_ffi_emit_fortran`.
 *
 * @return Greatest test result.
 */
TEST test_ffi_fortran_comprehensive(void) {
  char id_buf[128];
  FILE *wfp = NULL;
  char *wtxt = NULL;
  cdd_ffi_ir_t ir;
  cdd_ffi_ir_node_t nodes[11];
  cdd_ffi_enum_variant_t evars[4];
  cdd_ffi_field_t s_fields[3];
  cdd_ffi_field_t u_fields[2];
  cdd_ffi_field_t cb_params[1];
  cdd_ffi_type_t cb_ret;
  cdd_ffi_field_t fn_fields[6];
  cdd_generate_bindings_config_t cfg;
  char *f90_txt = NULL;
  char *fpm_txt = NULL;
  char *ftest_txt = NULL;
  int k;
  cdd_c_error_t rc;

  /* Sanitizer & 120-col wrapper direct checks */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_fortran_sanitize_ident("x", 0, NULL, 16));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_fortran_sanitize_ident("x", 0, id_buf, 4));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_fortran_sanitize_ident(NULL, 0, id_buf, sizeof(id_buf)));
  ASSERT_STR_EQ("arg_0", id_buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_fortran_sanitize_ident("module", 1, id_buf, sizeof(id_buf)));
  ASSERT_STR_EQ("module_", id_buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_fortran_sanitize_ident("very_long_c_identifier_exceeding_sixty_"
                                       "three_characters_by_a_wide_margin_xyz",
                                       0, id_buf, sizeof(id_buf)));
  ASSERT(strlen(id_buf) <= 63u);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_fortran_write_wrapped_line(NULL, "INTEGER :: x"));
  ASSERT_EQ(CDD_C_SUCCESS, makedirs("test_ofc_out/fortran"));
  rc = cdd_ffi_open_output_file("test_ofc_out/fortran", "wrap_check.f90", &wfp);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_fortran_write_wrapped_line(wfp, NULL));
  ASSERT_EQ(
      CDD_C_SUCCESS,
      cdd_fortran_write_wrapped_line(
          wfp,
          "    FUNCTION super_long_fn(arg_alpha_001, arg_beta_002, "
          "arg_gamma_003, arg_delta_004, arg_eps_005, arg_zeta_006) BIND(C, "
          "NAME=\"super_long_c_symbol_name_that_forces_character_context_"
          "continuation_across_120_columns_cleanly\") RESULT(cdd_ret_)"));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_fortran_write_wrapped_line(
                wfp, "  ! Very long comment line exceeding one hundred and "
                     "twenty columns to exercise free-form comment wrapping "
                     "cleanly in Fortran 2008 output"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_close_output_file(&wfp));
  ASSERT_EQ(CDD_C_SUCCESS,
            ofc_read_text_file("test_ofc_out/fortran/wrap_check.f90", &wtxt));
  ASSERT(strstr(wtxt, "&\n      &") != NULL);
  free(wtxt);

  memset(&cfg, 0, sizeof(cfg));
  memset(&ir, 0, sizeof(ir));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_emit_fortran(NULL, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_emit_fortran(&ir, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_emit_fortran(&ir, &cfg));

  memset(nodes, 0, sizeof(nodes));
  memset(evars, 0, sizeof(evars));
  memset(s_fields, 0, sizeof(s_fields));
  memset(u_fields, 0, sizeof(u_fields));
  memset(cb_params, 0, sizeof(cb_params));
  memset(&cb_ret, 0, sizeof(cb_ret));
  memset(fn_fields, 0, sizeof(fn_fields));

  /* 0..2: INT, FLOAT, STRING macros */
  nodes[0].kind = CDD_FFI_NODE_MACRO;
  nodes[0].name = cdd_test_mut_str("F_MAX");
  nodes[0].inferred_type = CDD_FFI_MACRO_TYPE_INT;
  nodes[0].evaluated_value = cdd_test_mut_str("1'000");

  nodes[1].kind = CDD_FFI_NODE_MACRO;
  nodes[1].name = cdd_test_mut_str("F_PI");
  nodes[1].inferred_type = CDD_FFI_MACRO_TYPE_FLOAT;
  nodes[1].evaluated_value = cdd_test_mut_str("3.14");

  nodes[2].kind = CDD_FFI_NODE_MACRO;
  nodes[2].name = cdd_test_mut_str("F_TAG");
  nodes[2].inferred_type = CDD_FFI_MACRO_TYPE_STRING;
  nodes[2].evaluated_value = cdd_test_mut_str("fort");

  /* 3: Enum with hex, octal, binary, C23 digit-separator */
  evars[0].name = cdd_test_mut_str("E_HEX");
  evars[0].value = cdd_test_mut_str("0x1A");
  evars[1].name = cdd_test_mut_str("E_OCT");
  evars[1].value = cdd_test_mut_str("077");
  evars[2].name = cdd_test_mut_str("E_BIN");
  evars[2].value = cdd_test_mut_str("0b101");
  evars[3].name = cdd_test_mut_str("E_SEP");
  evars[3].value = cdd_test_mut_str("1'000");
  nodes[3].kind = CDD_FFI_NODE_ENUM;
  nodes[3].name = cdd_test_mut_str("ModeEnum");
  nodes[3].variants = evars;
  nodes[3].variants_count = 4;

  /* 4: Opaque empty struct */
  nodes[4].kind = CDD_FFI_NODE_STRUCT;
  nodes[4].name = cdd_test_mut_str("OpaqueState");
  nodes[4].fields_count = 0;

  /* 5: Concrete struct with fixed array & typed callback */
  cb_params[0].name = cdd_test_mut_str("step");
  cb_params[0].type.kind = CDD_FFI_KIND_INT32;
  cb_ret.kind = CDD_FFI_KIND_INT32;
  s_fields[0].name = cdd_test_mut_str("coords");
  s_fields[0].type.kind = CDD_FFI_KIND_FLOAT64;
  s_fields[0].type.array_size = 3;
  s_fields[1].name = cdd_test_mut_str("cb");
  s_fields[1].type.kind = CDD_FFI_KIND_FUNCTION_PTR;
  s_fields[1].type.fn_return_type = &cb_ret;
  s_fields[1].type.fn_params = cb_params;
  s_fields[1].type.fn_params_count = 1;
  s_fields[2].name = cdd_test_mut_str("label");
  s_fields[2].type.kind = CDD_FFI_KIND_INT8;
  s_fields[2].type.array_size = 16;
  nodes[5].kind = CDD_FFI_NODE_STRUCT;
  nodes[5].name = cdd_test_mut_str("Vec3");
  nodes[5].fields = s_fields;
  nodes[5].fields_count = 3;

  /* 6: Union (8 bytes) */
  u_fields[0].name = cdd_test_mut_str("i_val");
  u_fields[0].type.kind = CDD_FFI_KIND_INT32;
  u_fields[1].name = cdd_test_mut_str("d_val");
  u_fields[1].type.kind = CDD_FFI_KIND_FLOAT64;
  nodes[6].kind = CDD_FFI_NODE_UNION;
  nodes[6].name = cdd_test_mut_str("NumberUnion");
  nodes[6].fields = u_fields;
  nodes[6].fields_count = 2;

  /* 7: Callback typedef */
  nodes[7].kind = CDD_FFI_NODE_TYPEDEF;
  nodes[7].name = cdd_test_mut_str("TickFn");
  nodes[7].return_or_base_type.kind = CDD_FFI_KIND_FUNCTION_PTR;
  nodes[7].return_or_base_type.fn_return_type = &cb_ret;
  nodes[7].return_or_base_type.fn_params = cb_params;
  nodes[7].return_or_base_type.fn_params_count = 1;

  /* 8 & 9: Case-colliding functions `Foo` and `foo`!
   * `Foo` tests unnamed arg (`NULL` -> `arg_0`), scalar `VALUE`, `[out]`
   * `INTENT(OUT)` (NO `VALUE`!), `[inout]` `INTENT(INOUT)`, `const char *`
   * string, fixed array, and `const char *` return! */
  fn_fields[0].name = NULL; /* unnamed -> arg_0 */
  fn_fields[0].type.kind = CDD_FFI_KIND_INT32;
  fn_fields[1].name = cdd_test_mut_str("out_v");
  fn_fields[1].type.kind = CDD_FFI_KIND_FLOAT64;
  fn_fields[1].type.pointer_depth = 1;
  fn_fields[1].intent = CDD_FFI_INTENT_OUT;
  fn_fields[2].name = cdd_test_mut_str("inout_vec");
  fn_fields[2].type.kind = CDD_FFI_KIND_STRUCT_REF;
  fn_fields[2].type.ref_name = cdd_test_mut_str("Vec3");
  fn_fields[2].type.pointer_depth = 1;
  fn_fields[2].intent = CDD_FFI_INTENT_INOUT;
  fn_fields[3].name = cdd_test_mut_str("in_s");
  fn_fields[3].type.kind = CDD_FFI_KIND_INT8;
  fn_fields[3].type.pointer_depth = 1;
  fn_fields[3].type.is_const = 1;
  fn_fields[3].intent = CDD_FFI_INTENT_IN;
  fn_fields[4].name = cdd_test_mut_str("arr4");
  fn_fields[4].type.kind = CDD_FFI_KIND_INT32;
  fn_fields[4].type.array_size = 4;
  fn_fields[5].name = cdd_test_mut_str("const_p");
  fn_fields[5].type.kind = CDD_FFI_KIND_INT32;
  fn_fields[5].type.pointer_depth = 1;
  fn_fields[5].type.is_const = 1;

  nodes[8].kind = CDD_FFI_NODE_FUNCTION;
  nodes[8].name = cdd_test_mut_str("Foo");
  nodes[8].return_or_base_type.kind = CDD_FFI_KIND_INT8;
  nodes[8].return_or_base_type.pointer_depth = 1;
  nodes[8].return_or_base_type.is_const = 1;
  nodes[8].fields = fn_fields;
  nodes[8].fields_count = 6;

  nodes[9].kind = CDD_FFI_NODE_FUNCTION;
  nodes[9].name = cdd_test_mut_str("foo");
  nodes[9].return_or_base_type.kind = CDD_FFI_KIND_VOID;

  /* 10: Variadic C function */
  nodes[10].kind = CDD_FFI_NODE_FUNCTION;
  nodes[10].name = cdd_test_mut_str("var_printf");
  nodes[10].return_or_base_type.kind = CDD_FFI_KIND_INT32;
  nodes[10].is_variadic = 1;

  ir.nodes = nodes;
  ir.nodes_count = 11;

  cfg.output_dir = "test_ofc_out/fortran";
  cfg.library_name = "demo_f90";
  cfg.module_name = "demo_f90_mod";
  cfg.generate_tests = 1;

  rc = cdd_ffi_emit_fortran(&ir, &cfg);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  ASSERT_EQ(
      CDD_C_SUCCESS,
      ofc_read_text_file("test_ofc_out/fortran/demo_f90_mod.f90", &f90_txt));
  ASSERT_EQ(CDD_C_SUCCESS,
            ofc_read_text_file("test_ofc_out/fortran/fpm.toml", &fpm_txt));
  ASSERT_EQ(CDD_C_SUCCESS,
            ofc_read_text_file("test_ofc_out/fortran/demo_f90_mod_test.f90",
                               &ftest_txt));

  ASSERT(strstr(f90_txt, "INTEGER(C_INT32_T), VALUE :: arg_0") != NULL);
  ASSERT(strstr(f90_txt, "REAL(C_DOUBLE), INTENT(OUT) :: out_v") != NULL);
  ASSERT(strstr(f90_txt, "TYPE(vec3), INTENT(INOUT) :: inout_vec") != NULL);
  ASSERT(strstr(f90_txt, "INTEGER(C_INT32_T), INTENT(IN) :: const_p") != NULL);
  ASSERT(strstr(f90_txt, "INTEGER(C_INT32_T), DIMENSION(4) :: arr4") != NULL);
  ASSERT(strstr(f90_txt, "RESULT(cdd_ret_)") != NULL);
  ASSERT(strstr(f90_txt, "BIND(C, NAME=\"Foo\")") != NULL);
  ASSERT(strstr(f90_txt, "BIND(C, NAME=\"foo\")") != NULL);
  ASSERT(strstr(f90_txt, "TYPE(C_FUNPTR)") != NULL);
  ASSERT(strstr(f90_txt, "ABSTRACT INTERFACE") != NULL);
  ASSERT(strstr(f90_txt, "TYPE(C_PTR) :: ptr = C_NULL_PTR") != NULL);
  ASSERT(strstr(f90_txt, "INTEGER(C_INT8_T) :: data(8)") != NULL);
  ASSERT(strstr(f90_txt, "INT(Z'1A', C_INT)") != NULL);
  ASSERT(strstr(f90_txt, "INT(O'77', C_INT)") != NULL);
  ASSERT(strstr(f90_txt, "INT(B'101', C_INT)") != NULL);
  ASSERT(strstr(f90_txt, "! Skipped variadic C function") != NULL);
  ASSERT(strstr(f90_txt, "cdd_c_ptr_to_f_string") != NULL);
  ASSERT(strstr(fpm_txt, "name = \"demo_f90\"") != NULL);
  ASSERT(strstr(ftest_txt, "C_SIZEOF") != NULL);

  free(f90_txt);
  free(fpm_txt);
  free(ftest_txt);

  /* OOM & g_fail_io_after sweeps */
  for (k = 1; k <= 200; k++) {
    g_cdd_alloc_fail = k;
    rc = cdd_ffi_emit_fortran(&ir, &cfg);
    g_cdd_alloc_fail = 0;
    if (rc == CDD_C_SUCCESS)
      break;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  }
  for (k = 1; k <= 200; k++) {
    g_cdd_strdup_fail = k;
    rc = cdd_ffi_emit_fortran(&ir, &cfg);
    g_cdd_strdup_fail = 0;
    if (rc == CDD_C_SUCCESS)
      break;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  }

  for (k = 1; k <= 2000; k++) {
    g_fail_io_after = k;
    rc = cdd_ffi_emit_fortran(&ir, &cfg);
    g_fail_io_after = -1;
    if (rc == CDD_C_SUCCESS)
      break;
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }

  PASS();
}

/**
 * @brief Comprehensive unit, Copybook layout (`FILLER`, `REDEFINES`, `OCCURS`,
 * `CONSTANT AS`, `88`), `.cbl` `BY VALUE`/`BY REFERENCE`/`RETURNING`, C89 shim,
 * OOM, and `g_fail_io_after` tests for `cdd_ffi_emit_cobol`.
 *
 * @return Greatest test result.
 */
TEST test_ffi_cobol_comprehensive(void) {
  char w_a[64];
  char w_b[64];
  cdd_ffi_ir_t ir;
  cdd_ffi_ir_node_t nodes[11];
  cdd_ffi_enum_variant_t evars[2];
  cdd_ffi_field_t pad_fields[12];
  cdd_ffi_field_t u_fields[2];
  cdd_ffi_field_t fn1_fields[3];
  cdd_ffi_field_t fn2_fields[1];
  cdd_generate_bindings_config_t cfg;
  char *cpy_txt = NULL;
  char *cbl_txt = NULL;
  char *shim_h_txt = NULL;
  char *shim_c_txt = NULL;
  char *ctest_txt = NULL;
  int k;
  cdd_c_error_t rc;

  /* Word normalizer, truncation disambiguation, and reserved-word checks */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cobol_sanitize_word("a", NULL, 0, NULL, 16));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cobol_sanitize_word("a", NULL, 0, w_a, 4));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cobol_sanitize_word("myStructName", NULL, 0, w_a, sizeof(w_a)));
  ASSERT_STR_EQ("MY-STRUCT-NAME", w_a);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cobol_sanitize_word("DATA", NULL, 0, w_a, sizeof(w_a)));
  ASSERT_STR_EQ("CDD-DATA", w_a);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cobol_sanitize_word("STATUS", NULL, 0, w_a, sizeof(w_a)));
  ASSERT_STR_EQ("CDD-STATUS", w_a);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cobol_sanitize_word("VALUE", NULL, 0, w_a, sizeof(w_a)));
  ASSERT_STR_EQ("CDD-VALUE", w_a);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cobol_sanitize_word("RECORD", NULL, 0, w_a, sizeof(w_a)));
  ASSERT_STR_EQ("CDD-RECORD", w_a);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cobol_sanitize_word("return_code", NULL, 0, w_a, sizeof(w_a)));
  ASSERT_STR_EQ("CDD-RETURN-CODE", w_a);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cobol_sanitize_word("1st_item", NULL, 0, w_a, sizeof(w_a)));
  ASSERT_STR_EQ("CDD-1ST-ITEM", w_a);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cobol_sanitize_word(
                               "extremely_long_c_function_identifier_alpha_one",
                               NULL, 0, w_a, sizeof(w_a)));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cobol_sanitize_word(
                               "extremely_long_c_function_identifier_alpha_two",
                               NULL, 0, w_b, sizeof(w_b)));
  ASSERT(strlen(w_a) <= 30u);
  ASSERT(strlen(w_b) <= 30u);
  ASSERT(strcmp(w_a, w_b) != 0);

  memset(&cfg, 0, sizeof(cfg));
  memset(&ir, 0, sizeof(ir));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_emit_cobol(NULL, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_emit_cobol(&ir, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_emit_cobol(&ir, &cfg));

  memset(nodes, 0, sizeof(nodes));
  memset(evars, 0, sizeof(evars));
  memset(pad_fields, 0, sizeof(pad_fields));
  memset(u_fields, 0, sizeof(u_fields));
  memset(fn1_fields, 0, sizeof(fn1_fields));
  memset(fn2_fields, 0, sizeof(fn2_fields));

  /* 0..2: INT (hex), FLOAT, STRING macros */
  nodes[0].kind = CDD_FFI_NODE_MACRO;
  nodes[0].name = cdd_test_mut_str("COB_MASK");
  nodes[0].inferred_type = CDD_FFI_MACRO_TYPE_INT;
  nodes[0].evaluated_value = cdd_test_mut_str("0x1F");

  nodes[1].kind = CDD_FFI_NODE_MACRO;
  nodes[1].name = cdd_test_mut_str("COB_RATE");
  nodes[1].inferred_type = CDD_FFI_MACRO_TYPE_FLOAT;
  nodes[1].evaluated_value = cdd_test_mut_str("2.5");

  nodes[2].kind = CDD_FFI_NODE_MACRO;
  nodes[2].name = cdd_test_mut_str("COB_NAME");
  nodes[2].inferred_type = CDD_FFI_MACRO_TYPE_STRING;
  nodes[2].evaluated_value = cdd_test_mut_str("cobol_abi");

  /* 3: Enum with reserved words */
  evars[0].name = cdd_test_mut_str("STATUS");
  evars[0].value = cdd_test_mut_str("0");
  evars[1].name = cdd_test_mut_str("RECORD");
  evars[1].value = cdd_test_mut_str("0x10");
  nodes[3].kind = CDD_FFI_NODE_ENUM;
  nodes[3].name = cdd_test_mut_str("CobEnum");
  nodes[3].variants = evars;
  nodes[3].variants_count = 2;

  /* 4: Opaque struct */
  nodes[4].kind = CDD_FFI_NODE_STRUCT;
  nodes[4].name = cdd_test_mut_str("OpaqueSession");
  nodes[4].fields_count = 0;

  /* 5: Struct exercising all COBOL native-endian usages + FILLER padding +
   * OCCURS */
  pad_fields[0].name = cdd_test_mut_str("f_i8");
  pad_fields[0].type.kind = CDD_FFI_KIND_INT8;
  pad_fields[1].name = cdd_test_mut_str("f_i32");
  pad_fields[1].type.kind = CDD_FFI_KIND_INT32;
  pad_fields[2].name = cdd_test_mut_str("f_u8");
  pad_fields[2].type.kind = CDD_FFI_KIND_UINT8;
  pad_fields[3].name = cdd_test_mut_str("f_i16");
  pad_fields[3].type.kind = CDD_FFI_KIND_INT16;
  pad_fields[4].name = cdd_test_mut_str("f_u16");
  pad_fields[4].type.kind = CDD_FFI_KIND_UINT16;
  pad_fields[5].name = cdd_test_mut_str("f_u32");
  pad_fields[5].type.kind = CDD_FFI_KIND_UINT32;
  pad_fields[6].name = cdd_test_mut_str("f_i64");
  pad_fields[6].type.kind = CDD_FFI_KIND_INT64;
  pad_fields[7].name = cdd_test_mut_str("f_u64");
  pad_fields[7].type.kind = CDD_FFI_KIND_UINT64;
  pad_fields[8].name = cdd_test_mut_str("f_f32");
  pad_fields[8].type.kind = CDD_FFI_KIND_FLOAT32;
  pad_fields[8].type.array_size = 2;
  pad_fields[9].name = cdd_test_mut_str("f_f64");
  pad_fields[9].type.kind = CDD_FFI_KIND_FLOAT64;
  pad_fields[10].name = cdd_test_mut_str("f_ptr");
  pad_fields[10].type.kind = CDD_FFI_KIND_OPAQUE_PTR;
  pad_fields[11].name = cdd_test_mut_str("f_cb");
  pad_fields[11].type.kind = CDD_FFI_KIND_FUNCTION_PTR;
  nodes[5].kind = CDD_FFI_NODE_STRUCT;
  nodes[5].name = cdd_test_mut_str("FullRecord");
  nodes[5].fields = pad_fields;
  nodes[5].fields_count = 12;

  /* 6: Union with REDEFINES & tail FILLER */
  u_fields[0].name = cdd_test_mut_str("small_i32");
  u_fields[0].type.kind = CDD_FFI_KIND_INT32;
  u_fields[1].name = cdd_test_mut_str("wide_f64");
  u_fields[1].type.kind = CDD_FFI_KIND_FLOAT64;
  nodes[6].kind = CDD_FFI_NODE_UNION;
  nodes[6].name = cdd_test_mut_str("CobUnion");
  nodes[6].fields = u_fields;
  nodes[6].fields_count = 2;

  /* 7: Direct C scalar/pointer function (`BY VALUE`, `BY REFERENCE`,
   * `RETURNING`) */
  fn1_fields[0].name = cdd_test_mut_str("in_str");
  fn1_fields[0].type.kind = CDD_FFI_KIND_INT8;
  fn1_fields[0].type.pointer_depth = 1;
  fn1_fields[0].type.is_const = 1;
  fn1_fields[0].intent = CDD_FFI_INTENT_IN;
  fn1_fields[1].name = cdd_test_mut_str("in_num");
  fn1_fields[1].type.kind = CDD_FFI_KIND_INT32;
  fn1_fields[2].name = cdd_test_mut_str("out_num");
  fn1_fields[2].type.kind = CDD_FFI_KIND_INT32;
  fn1_fields[2].type.pointer_depth = 1;
  fn1_fields[2].intent = CDD_FFI_INTENT_OUT;
  nodes[7].kind = CDD_FFI_NODE_FUNCTION;
  nodes[7].name = cdd_test_mut_str("direct_calc");
  nodes[7].return_or_base_type.kind = CDD_FFI_KIND_INT32;
  nodes[7].fields = fn1_fields;
  nodes[7].fields_count = 3;

  /* 8: Void function -> RETURNING OMITTED */
  nodes[8].kind = CDD_FFI_NODE_FUNCTION;
  nodes[8].name = cdd_test_mut_str("void_reset");
  nodes[8].return_or_base_type.kind = CDD_FFI_KIND_VOID;

  /* 9: By-value struct arg & return -> C89 shim */
  fn2_fields[0].name = cdd_test_mut_str("rec_in");
  fn2_fields[0].type.kind = CDD_FFI_KIND_STRUCT_REF;
  fn2_fields[0].type.ref_name = cdd_test_mut_str("FullRecord");
  nodes[9].kind = CDD_FFI_NODE_FUNCTION;
  nodes[9].name = cdd_test_mut_str("transform_rec");
  nodes[9].return_or_base_type.kind = CDD_FFI_KIND_STRUCT_REF;
  nodes[9].return_or_base_type.ref_name = cdd_test_mut_str("FullRecord");
  nodes[9].fields = fn2_fields;
  nodes[9].fields_count = 1;

  /* 10: Variadic + const char * return -> C89 shim */
  nodes[10].kind = CDD_FFI_NODE_FUNCTION;
  nodes[10].name = cdd_test_mut_str("fmt_msg");
  nodes[10].return_or_base_type.kind = CDD_FFI_KIND_INT8;
  nodes[10].return_or_base_type.pointer_depth = 1;
  nodes[10].return_or_base_type.is_const = 1;
  nodes[10].is_variadic = 1;

  ir.nodes = nodes;
  ir.nodes_count = 11;

  ASSERT_EQ(CDD_C_SUCCESS, makedirs("test_ofc_out/cobol"));
  cfg.output_dir = "test_ofc_out/cobol";
  cfg.library_name = "demo_cob";
  cfg.module_name = "demo_cob_mod";
  cfg.generate_tests = 1;

  rc = cdd_ffi_emit_cobol(&ir, &cfg);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  ASSERT_EQ(CDD_C_SUCCESS,
            ofc_read_text_file("test_ofc_out/cobol/demo_cob.cpy", &cpy_txt));
  ASSERT_EQ(
      CDD_C_SUCCESS,
      ofc_read_text_file("test_ofc_out/cobol/demo_cob_mod.cbl", &cbl_txt));
  ASSERT_EQ(CDD_C_SUCCESS,
            ofc_read_text_file("test_ofc_out/cobol/demo_cob_cobol_shim.h",
                               &shim_h_txt));
  ASSERT_EQ(CDD_C_SUCCESS,
            ofc_read_text_file("test_ofc_out/cobol/demo_cob_cobol_shim.c",
                               &shim_c_txt));
  ASSERT_EQ(CDD_C_SUCCESS,
            ofc_read_text_file("test_ofc_out/cobol/demo_cob_mod_test.cbl",
                               &ctest_txt));

  ASSERT(strstr(cpy_txt, "CONSTANT AS H\"1F\"") != NULL);
  ASSERT(strstr(cpy_txt, "88 IS-STATUS VALUE 0.") != NULL);
  ASSERT(strstr(cpy_txt, "OPAQUE-SESSION-HANDLE USAGE POINTER.") != NULL);
  ASSERT(strstr(cpy_txt, "05 FILLER PIC X(3).") != NULL);
  ASSERT(strstr(cpy_txt, "USAGE BINARY-CHAR SIGNED") != NULL);
  ASSERT(strstr(cpy_txt, "USAGE BINARY-CHAR UNSIGNED") != NULL);
  ASSERT(strstr(cpy_txt, "USAGE BINARY-SHORT SIGNED") != NULL);
  ASSERT(strstr(cpy_txt, "USAGE BINARY-SHORT UNSIGNED") != NULL);
  ASSERT(strstr(cpy_txt, "USAGE BINARY-LONG SIGNED") != NULL);
  ASSERT(strstr(cpy_txt, "USAGE BINARY-LONG UNSIGNED") != NULL);
  ASSERT(strstr(cpy_txt, "USAGE BINARY-DOUBLE SIGNED") != NULL);
  ASSERT(strstr(cpy_txt, "USAGE BINARY-DOUBLE UNSIGNED") != NULL);
  ASSERT(strstr(cpy_txt, "OCCURS 2 TIMES USAGE COMP-1") != NULL);
  ASSERT(strstr(cpy_txt, "USAGE COMP-2") != NULL);
  ASSERT(strstr(cpy_txt, "USAGE POINTER") != NULL);
  ASSERT(strstr(cpy_txt, "USAGE PROGRAM-POINTER") != NULL);
  ASSERT(strstr(cpy_txt, "REDEFINES COB-UNION-RAW") != NULL);
  ASSERT(strstr(cpy_txt, "10 FILLER PIC X(4).") != NULL);

  ASSERT(strstr(cbl_txt, ">>SOURCE FORMAT IS FREE") != NULL);
  ASSERT(strstr(cbl_txt, "CALL \"direct_calc\"") != NULL);
  ASSERT(strstr(cbl_txt, "BY VALUE LS-ARG-1") != NULL);
  ASSERT(strstr(cbl_txt, "BY REFERENCE LS-ARG-2") != NULL);
  ASSERT(strstr(cbl_txt, "RETURNING LS-RET") != NULL);
  ASSERT(strstr(cbl_txt, "RETURNING OMITTED") != NULL);
  ASSERT(strstr(cbl_txt, "CALL \"cdd_cobol_transform_rec\"") != NULL);
  ASSERT(strstr(shim_h_txt, "cdd_cobol_str_to_pic_x") != NULL);
  ASSERT(strstr(shim_c_txt, "cdd_cobol_transform_rec") != NULL);
  ASSERT(strstr(ctest_txt, "FUNCTION BYTE-LENGTH(FULL-RECORD)") != NULL);

  free(cpy_txt);
  free(cbl_txt);
  free(shim_h_txt);
  free(shim_c_txt);
  free(ctest_txt);

  /* OOM & g_fail_io_after sweeps */
  for (k = 1; k <= 200; k++) {
    g_cdd_alloc_fail = k;
    rc = cdd_ffi_emit_cobol(&ir, &cfg);
    g_cdd_alloc_fail = 0;
    if (rc == CDD_C_SUCCESS)
      break;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  }
  for (k = 1; k <= 200; k++) {
    g_cdd_strdup_fail = k;
    rc = cdd_ffi_emit_cobol(&ir, &cfg);
    g_cdd_strdup_fail = 0;
    if (rc == CDD_C_SUCCESS)
      break;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  }

  for (k = 1; k <= 2000; k++) {
    g_fail_io_after = k;
    rc = cdd_ffi_emit_cobol(&ir, &cfg);
    g_fail_io_after = -1;
    if (rc == CDD_C_SUCCESS)
      break;
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }

  PASS();
}

/**
 * @brief Tests DSL-to-Bindings E2E (`"ocaml,fortran,cobol,dsl"` and `"cbl"`)
 * plus strict C89 and optional external compiler (`gfortran`, `cobc`, `ocamlc`)
 * smoke compilation when installed on the host.
 *
 * @return Greatest test result.
 */
TEST test_ffi_dsl_e2e_and_compiler_smoke(void) {
  const char *dsl_spec =
      "enum CalcStatus {\n"
      "  CALC_OK = 0,\n"
      "  CALC_ERR = 0x10\n"
      "}\n"
      "macro_const CALC_VERSION = 2;\n"
      "func calc_run([in] *const char name, [out] *mut i32 out_code) i32;\n";
  const char *dsl_file = "test_ofc_out/e2e_spec.dsl";
  cdd_generate_bindings_config_t cfg;
  cdd_c_error_t rc;

  ASSERT_EQ(CDD_C_SUCCESS, makedirs("test_ofc_out/e2e"));
  ASSERT_EQ(CDD_C_SUCCESS, write_to_file(dsl_file, dsl_spec));

  memset(&cfg, 0, sizeof(cfg));
  cfg.input = dsl_file;
  cfg.output_dir = "test_ofc_out/e2e";
  cfg.library_name = "calc_e2e";
  cfg.module_name = "calc_e2e_mod";
  cfg.generate_tests = 1;

  /* 1. Reject invalid / trailing-comma target_langs */
  cfg.target_langs = "ocaml,unknown_lang_xyz";
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            cdd_generate_bindings_from_dsl(dsl_spec, &cfg));
  cfg.target_langs = "ocaml,";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_generate_bindings_from_dsl(dsl_spec, &cfg));
  cfg.target_langs = ",ocaml";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_generate_bindings(&cfg));

  /* 2. Generate ocaml,fortran,cobol,dsl from DSL string AND .dsl file */
  cfg.target_langs = "ocaml,fortran,cobol,dsl";
  rc = cdd_generate_bindings_from_dsl(dsl_spec, &cfg);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  cfg.target_langs = "cbl";
  rc = cdd_generate_bindings(&cfg);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  remove("test_out_dsl.dsl");

#if !defined(_WIN32)
  /* 3. Strict C89 & external toolchain smoke compilation when installed */
  {
    int sys_rc;
    sys_rc = system("command -v cc >/dev/null 2>&1");
    if (sys_rc == 0) {
      sys_rc = system("cc -std=c89 -pedantic -Wall -Wextra -Werror -c "
                      "test_ofc_out/e2e/calc_e2e_cobol_shim.c "
                      "-o test_ofc_out/e2e/calc_e2e_cobol_shim.o");
      ASSERT_EQ(0, sys_rc);
      sys_rc = system("cc -std=c89 -pedantic -Wall -Wextra -Werror "
                      "-DCDD_OCAML_STUB_STANDALONE -c "
                      "test_ofc_out/e2e/calc_e2e_stubs.c "
                      "-o test_ofc_out/e2e/calc_e2e_stubs.o");
      ASSERT_EQ(0, sys_rc);
    }

    sys_rc = system("command -v gfortran >/dev/null 2>&1");
    if (sys_rc == 0) {
      sys_rc = system(
          "(cd test_ofc_out/e2e && "
          "gfortran -c -Wall -Wextra -Werror -std=f2008 calc_e2e_mod.f90 && "
          "gfortran -fsyntax-only -Wall -Wextra -Werror -std=f2008 "
          "calc_e2e_mod_test.f90)");
      ASSERT_EQ(0, sys_rc);
    }

    sys_rc = system("command -v cobc >/dev/null 2>&1");
    if (sys_rc == 0) {
      sys_rc =
          system("(cd test_ofc_out/e2e && "
                 "cobc -fsyntax-only -free -Wall -I . calc_e2e_mod.cbl && "
                 "cobc -fsyntax-only -free -Wall -I . calc_e2e_mod_test.cbl)");
      ASSERT_EQ(0, sys_rc);
    }

    sys_rc = system("ocamlfind query ctypes >/dev/null 2>&1");
    if (sys_rc == 0) {
      sys_rc = system("(cd test_ofc_out/e2e && "
                      "ocamlfind ocamlc -package ctypes,ctypes.foreign -c "
                      "calc_e2e.mli calc_e2e.ml)");
      ASSERT_EQ(0, sys_rc);
    }
  }
#endif

  PASS();
}

/**
 * @brief Variadic test helper forwarding directly to
 * `cdd_ffi_vsnprintf_checked`.
 *
 * @param[out] buf Destination buffer.
 * @param[in] cap Buffer capacity.
 * @param[in] fmt Format string.
 * @return Status from `cdd_ffi_vsnprintf_checked`.
 */
#if defined(__GNUC__) || defined(__clang__)
static NO_DISCARD cdd_c_error_t ofc_call_vsnprintf(char *buf, size_t cap,
                                                   const char *fmt, ...)
    __attribute__((format(printf, 3, 4)));
#endif
static NO_DISCARD cdd_c_error_t ofc_call_vsnprintf(char *buf, size_t cap,
                                                   const char *fmt, ...) {
  va_list ap;
  cdd_c_error_t rc;
  va_start(ap, fmt);
  rc = cdd_ffi_vsnprintf_checked(buf, cap, fmt, ap);
  va_end(ap);
  return rc;
}

/**
 * @brief Exercises remaining edge, overflow, OOM, and stream-error branches in
 * `cdd_ffi_emit_common.c`, `cdd_ffi_ir.c`, `cdd_dsl_parser.c`, and
 * `cdd_ffi_ir_extractor.c`.
 *
 * @return Greatest test result.
 */
TEST test_ffi_batch2_branch_and_oom_edges(void) {
  FILE *fp = NULL;
  char sbuf[32];
  char long_name[1100];
  int fail_idx;
  cdd_c_error_t rc;

  ASSERT_EQ(CDD_C_SUCCESS, makedirs("test_ofc_out/b2"));
  ASSERT_EQ(CDD_C_SUCCESS, write_to_file("test_ofc_out/b2/ro.txt", "abc"));

  /* 1. Common I/O error branches (read-only stream, closed fd, truncation) */
#if defined(_MSC_VER)
  if (fopen_s(&fp, "test_ofc_out/b2/ro.txt", "r") != 0)
    fp = NULL;
#else
  fp = fopen("test_ofc_out/b2/ro.txt", "r");
#endif
  ASSERT(fp != NULL);
  ASSERT_EQ(CDD_C_ERROR_IO, cdd_ffi_write_str(fp, "x"));
  ASSERT_EQ(CDD_C_ERROR_IO, cdd_ffi_write_bytes(fp, "x", 1));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_write_bytes(fp, "x", 0));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_write_bytes(fp, NULL, 0));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_write_fmt(fp, NULL));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_ffi_write_fmt(fp, "%5000s", "x"));
#if defined(_WIN32)
  {
    cdd_c_error_t close_rc = cdd_ffi_close_output_file(&fp);
    ASSERT(close_rc == CDD_C_SUCCESS || close_rc == CDD_C_ERROR_IO);
  }
#else
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_close_output_file(&fp));
#endif

#if !defined(_WIN32)
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_open_output_file("test_ofc_out/b2", "badclose.txt", &fp));
  fputc('z', fp);
  close(fileno(fp));
  ASSERT_EQ(CDD_C_ERROR_IO, cdd_ffi_close_output_file(&fp));
#endif

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            ofc_call_vsnprintf(NULL, sizeof(sbuf), "%s", "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            ofc_call_vsnprintf(sbuf, 0, "%s", "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            ofc_call_vsnprintf(sbuf, sizeof(sbuf), NULL));

  memset(long_name, 'a', sizeof(long_name) - 1);
  long_name[sizeof(long_name) - 1] = '\0';
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_ffi_open_output_file("test_ofc_out/b2", long_name, &fp));

  /* 2. Comment translator styles, delimiters, wrapping, and I/O fault loop */
  {
    cdd_ffi_comment_style_t bad_s1 = {NULL, "/*", NULL, NULL,
                                      NULL, NULL, NULL, 0};
    cdd_ffi_comment_style_t bad_s2 = {NULL, NULL, "*/", NULL,
                                      NULL, NULL, NULL, 0};
    cdd_ffi_comment_style_t plain_blk = {NULL, "/* ", " */", NULL,
                                         NULL, NULL,  NULL,  0};
    cdd_ffi_comment_style_t ocaml_s = {NULL,  "(* ", " *)", "(*",
                                       "( *", "*)",  "* )", 0};
    cdd_ffi_comment_style_t tight_ln = {"!",  NULL, NULL, NULL,
                                        NULL, NULL, NULL, 10};
    cdd_ffi_comment_style_t half_esc = {NULL, "(* ", " *)", "(*",
                                        NULL, "*)",  NULL,  0};
    cdd_ffi_comment_style_t empty_pfx = {"",   NULL, NULL, NULL,
                                         NULL, NULL, NULL, 0};
    cdd_ffi_trivia_t t1, t2, t3, t4, t5;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_open_output_file("test_ofc_out/b2", "cmts.txt", &fp));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_emit_translated_doc(NULL, "doc", "", &plain_blk));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_emit_translated_doc(fp, "doc", "", NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_emit_translated_trivia(fp, NULL, &bad_s1));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_emit_translated_trivia(fp, NULL, &bad_s2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_emit_translated_doc(fp, "doc", "", &bad_s1));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_emit_translated_doc(fp, "doc", "", &bad_s2));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_emit_translated_doc(fp, "   \t  ", "", &tight_ln));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_emit_translated_doc(fp, "doc", NULL, &plain_blk));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_emit_translated_doc(fp, "doc", "", &half_esc));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_emit_translated_doc(fp, "//", "", &empty_pfx));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_emit_translated_doc(
                                 fp, "(*(*start and end*)*)", "  ", &ocaml_s));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_emit_translated_doc(fp, "a*)*)", "  ", &ocaml_s));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_emit_translated_doc(
                  fp, " \t /*! d1\t\r\n * \r\n *tightstar\r\n*/ \t\r\n", "  ",
                  &ocaml_s));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_emit_translated_doc(fp, "/***/", "  ", &tight_ln));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_emit_translated_doc(fp, "/**x*/", "  ", &tight_ln));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_emit_translated_doc(fp, "/* unclosed", "  ", &tight_ln));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_emit_translated_doc(fp, "/x", "  ", &tight_ln));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_emit_translated_doc(fp, "x", "  ", &tight_ln));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_emit_translated_doc(fp, "//x", "  ", &tight_ln));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_emit_translated_doc(fp, "/**/", "  ", &tight_ln));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_emit_translated_doc(fp, "/*x*/", "", &tight_ln));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_emit_translated_doc(fp, "/// slash3", "  ", &tight_ln));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_emit_translated_doc(fp, "//! bang", "  ", &tight_ln));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_emit_translated_doc(fp, "//", "  ", &tight_ln));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_emit_translated_doc(
                  fp, "supercalifragilisticexpialidocious word wrap\n\nline2",
                  "  ", &tight_ln));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_emit_translated_doc(
                  fp, "supercalifragilisticexpialidocious", "", &tight_ln));

    memset(&t1, 0, sizeof(t1));
    memset(&t2, 0, sizeof(t2));
    memset(&t3, 0, sizeof(t3));
    memset(&t4, 0, sizeof(t4));
    memset(&t5, 0, sizeof(t5));
    t1.kind = CDD_FFI_TRIVIA_WHITESPACE;
    t1.text = cdd_test_mut_str(" ");
    t1.next = &t2;
    t2.kind = CDD_FFI_TRIVIA_WHITESPACE;
    t2.text = cdd_test_mut_str("\n\n");
    t2.next = &t3;
    t3.kind = (cdd_ffi_trivia_kind_t)99;
    t3.text = cdd_test_mut_str("ignored");
    t3.next = &t4;
    t4.kind = CDD_FFI_TRIVIA_COMMENT_LINE;
    t4.text = NULL;
    t4.next = &t5;
    t5.kind = CDD_FFI_TRIVIA_COMMENT_BLOCK;
    t5.text = cdd_test_mut_str("/* plain */");
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_emit_translated_trivia(fp, &t1, &plain_blk));

    for (fail_idx = 1; fail_idx <= 40; ++fail_idx) {
      g_fail_io_after = fail_idx;
      rc = cdd_ffi_emit_translated_doc(
          fp, "/**\n * (*a*)\n *\n * supercalifragilistic wrap line\n */", "  ",
          &ocaml_s);
      g_fail_io_after = -1;
      if (rc == CDD_C_SUCCESS)
        break;
      ASSERT_EQ(CDD_C_ERROR_IO, rc);
    }
    for (fail_idx = 1; fail_idx <= 40; ++fail_idx) {
      g_fail_io_after = fail_idx;
      rc = cdd_ffi_emit_translated_doc(
          fp, "/**\n * \n * supercalifragilisticexpialidocious wrap\n */", "  ",
          &tight_ln);
      g_fail_io_after = -1;
      if (rc == CDD_C_SUCCESS)
        break;
      ASSERT_EQ(CDD_C_ERROR_IO, rc);
    }
    g_fail_io_after = 1;
    ASSERT_EQ(CDD_C_ERROR_IO,
              cdd_ffi_emit_translated_doc(fp, "//", "  ", &tight_ln));
    g_fail_io_after = 2;
    ASSERT_EQ(CDD_C_ERROR_IO,
              cdd_ffi_emit_translated_doc(fp, "//", "  ", &tight_ln));
    g_fail_io_after = 1;
    ASSERT_EQ(CDD_C_ERROR_IO,
              cdd_ffi_emit_translated_doc(fp, "single (*x*)", "  ", &ocaml_s));
    g_fail_io_after = -1;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_close_output_file(&fp));
  }

  /* 3. IR toposort & layout calculator branches (typedefs, cycles, overflows)
   */
  {
    cdd_ffi_ir_t ir;
    cdd_ffi_ir_node_t nodes[6];
    cdd_ffi_field_t flds[34];
    cdd_ffi_type_t tmpl_arg;
    cdd_ffi_type_t fn_ret;
    cdd_ffi_field_t fn_prm;
    cdd_ffi_abi_profile_t abi;
    const char *builtin_names[31];
    size_t bi;

    /* 3a. Toposort with template_args, fn_return_type, fn_params + fail hook */
    for (fail_idx = 0; fail_idx <= 5; ++fail_idx) {
      cdd_ffi_ir_node_t *heap_nodes =
          (cdd_ffi_ir_node_t *)calloc(4, sizeof(cdd_ffi_ir_node_t));
      ASSERT(heap_nodes != NULL);
      memset(&ir, 0, sizeof(ir));
      memset(&tmpl_arg, 0, sizeof(tmpl_arg));
      memset(&fn_ret, 0, sizeof(fn_ret));
      memset(&fn_prm, 0, sizeof(fn_prm));

      heap_nodes[0].kind = CDD_FFI_NODE_FUNCTION;
      heap_nodes[0].name = cdd_test_mut_str("use_fn");
      heap_nodes[0].return_or_base_type.kind = CDD_FFI_KIND_TEMPLATE_STRUCT_REF;
      tmpl_arg.kind = CDD_FFI_KIND_STRUCT_REF;
      tmpl_arg.ref_name = cdd_test_mut_str("DepA");
      heap_nodes[0].return_or_base_type.template_args = &tmpl_arg;
      heap_nodes[0].return_or_base_type.template_args_count = 1;
      fn_ret.kind = CDD_FFI_KIND_STRUCT_REF;
      fn_ret.ref_name = cdd_test_mut_str("DepB");
      fn_prm.type.kind = CDD_FFI_KIND_STRUCT_REF;
      fn_prm.type.ref_name = cdd_test_mut_str("DepC");
      heap_nodes[0].return_or_base_type.fn_return_type = &fn_ret;
      heap_nodes[0].return_or_base_type.fn_params = &fn_prm;
      heap_nodes[0].return_or_base_type.fn_params_count = 1;

      heap_nodes[1].kind = CDD_FFI_NODE_STRUCT;
      heap_nodes[1].name = cdd_test_mut_str("DepA");
      heap_nodes[2].kind = CDD_FFI_NODE_STRUCT;
      heap_nodes[2].name = cdd_test_mut_str("DepB");
      heap_nodes[3].kind = CDD_FFI_NODE_STRUCT;
      heap_nodes[3].name = cdd_test_mut_str("DepC");
      ir.nodes = heap_nodes;
      ir.nodes_count = 4;

      if (fail_idx > 0) {
        g_cdd_ffi_ir_toposort_fail = fail_idx;
        rc = cdd_ffi_ir_topological_sort(&ir);
        g_cdd_ffi_ir_toposort_fail = 0;
        if (rc == CDD_C_SUCCESS) {
          free(ir.nodes);
          break;
        }
      } else {
        ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_ir_topological_sort(&ir));
      }
      free(ir.nodes);
    }

    /* 3b. Built-in typedef names + STD_SHARED_PTR / STD_UNIQUE_PTR + packed
     * union */
    builtin_names[0] = "size_t";
    builtin_names[1] = "ssize_t";
    builtin_names[2] = "ptrdiff_t";
    builtin_names[3] = "intptr_t";
    builtin_names[4] = "uintptr_t";
    builtin_names[5] = "long";
    builtin_names[6] = "unsigned long";
    builtin_names[7] = "signed long";
    builtin_names[8] = "long int";
    builtin_names[9] = "unsigned long int";
    builtin_names[10] = "int8_t";
    builtin_names[11] = "uint8_t";
    builtin_names[12] = "char";
    builtin_names[13] = "unsigned char";
    builtin_names[14] = "signed char";
    builtin_names[15] = "bool";
    builtin_names[16] = "_Bool";
    builtin_names[17] = "int16_t";
    builtin_names[18] = "uint16_t";
    builtin_names[19] = "short";
    builtin_names[20] = "unsigned short";
    builtin_names[21] = "int32_t";
    builtin_names[22] = "uint32_t";
    builtin_names[23] = "int";
    builtin_names[24] = "unsigned int";
    builtin_names[25] = "float";
    builtin_names[26] = "int64_t";
    builtin_names[27] = "uint64_t";
    builtin_names[28] = "long long";
    builtin_names[29] = "unsigned long long";
    builtin_names[30] = "double";

    memset(&ir, 0, sizeof(ir));
    memset(nodes, 0, sizeof(nodes));
    memset(flds, 0, sizeof(flds));
    for (bi = 0; bi < 31; ++bi) {
      flds[bi].type.kind = CDD_FFI_KIND_TYPEDEF_REF;
      flds[bi].type.ref_name = cdd_test_mut_str(builtin_names[bi]);
    }
    flds[31].type.kind = CDD_FFI_KIND_STD_SHARED_PTR;
    flds[32].type.kind = CDD_FFI_KIND_STD_UNIQUE_PTR;
    flds[33].type.kind = CDD_FFI_KIND_VOID;
    flds[33].type.array_size = 4;
    flds[33].bitfield_width = 1;

    nodes[0].kind = CDD_FFI_NODE_STRUCT;
    nodes[0].name = cdd_test_mut_str("AllBuiltins");
    nodes[0].fields = flds;
    nodes[0].fields_count = 34;

    nodes[1].kind = CDD_FFI_NODE_UNION;
    nodes[1].name = cdd_test_mut_str("PackedU");
    nodes[1].is_packed = 1;
    nodes[1].fields = flds;
    nodes[1].fields_count = 2;

    nodes[2].kind = CDD_FFI_NODE_UNION;
    nodes[2].name = cdd_test_mut_str("EmptyU");

    ir.nodes = nodes;
    ir.nodes_count = 3;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_ir_compute_layout(&ir, NULL));
    ASSERT_EQ(1u, nodes[1].align_bytes);

    /* 3c. Unresolved refs allowed vs cycles vs overflows vs invalid ABI */
    memset(&abi, 0, sizeof(abi));
    abi.kind = (cdd_ffi_abi_kind_t)99;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_ir_compute_layout(&ir, &abi));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_abi_profile_init(CDD_FFI_ABI_LP64, &abi));
    abi.pointer_align = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_ir_compute_layout(&ir, &abi));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_abi_profile_init(CDD_FFI_ABI_LP64, &abi));
    abi.long_size = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_ir_compute_layout(&ir, &abi));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_abi_profile_init(CDD_FFI_ABI_LP64, &abi));
    abi.long_align = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_ir_compute_layout(&ir, &abi));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_abi_profile_init(CDD_FFI_ABI_LP64, &abi));
    abi.enum_size = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_ir_compute_layout(&ir, &abi));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_abi_profile_init(CDD_FFI_ABI_LP64, &abi));
    abi.enum_align = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_ir_compute_layout(&ir, &abi));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_abi_profile_init(CDD_FFI_ABI_LP64, &abi));
    abi.int64_align = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_ir_compute_layout(&ir, &abi));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_abi_profile_init(CDD_FFI_ABI_LP64, &abi));
    abi.double_align = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_ir_compute_layout(&ir, &abi));

    /* Unresolved ENUM_REF, TYPEDEF_REF, STRUCT_REF, and unknown kind with
     * allow_unresolved_refs */
    memset(&abi, 0, sizeof(abi));
    abi.kind = CDD_FFI_ABI_LP64;
    abi.allow_unresolved_refs = 1;
    memset(flds, 0, sizeof(flds));
    flds[0].type.kind = CDD_FFI_KIND_ENUM_REF;
    flds[0].type.ref_name = cdd_test_mut_str("MissingE");
    flds[1].type.kind = CDD_FFI_KIND_TYPEDEF_REF;
    flds[1].type.ref_name = NULL;
    flds[2].type.kind = CDD_FFI_KIND_TYPEDEF_REF;
    flds[2].type.ref_name = cdd_test_mut_str("MissingT");
    flds[3].type.kind = CDD_FFI_KIND_STRUCT_REF;
    flds[3].type.ref_name = NULL;
    flds[4].type.kind = (cdd_ffi_primitive_kind_t)99;
    nodes[0].kind = CDD_FFI_NODE_STRUCT;
    nodes[0].fields = flds;
    nodes[0].fields_count = 5;
    ir.nodes = nodes;
    ir.nodes_count = 1;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_ir_compute_layout(&ir, &abi));

    /* Strict NULL/unknown ref errors */
    flds[0].type.kind = CDD_FFI_KIND_ENUM_REF;
    flds[0].type.ref_name = NULL;
    nodes[0].fields_count = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_ir_compute_layout(&ir, NULL));
    flds[0].type.ref_name = cdd_test_mut_str("MissingE");
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, cdd_ffi_ir_compute_layout(&ir, NULL));
    flds[0].type.kind = CDD_FFI_KIND_TYPEDEF_REF;
    flds[0].type.ref_name = NULL;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_ir_compute_layout(&ir, NULL));
    flds[0].type.ref_name = cdd_test_mut_str("MissingT");
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, cdd_ffi_ir_compute_layout(&ir, NULL));
    flds[0].type.kind = CDD_FFI_KIND_STRUCT_REF;
    flds[0].type.ref_name = NULL;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_ir_compute_layout(&ir, NULL));
    flds[0].type.kind = (cdd_ffi_primitive_kind_t)99;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_ir_compute_layout(&ir, NULL));

    /* Cycles through TYPEDEF, ENUM_REF, UNION, FUNCTION */
    memset(nodes, 0, sizeof(nodes));
    nodes[0].kind = CDD_FFI_NODE_TYPEDEF;
    nodes[0].name = cdd_test_mut_str("TycA");
    nodes[0].return_or_base_type.kind = CDD_FFI_KIND_TYPEDEF_REF;
    nodes[0].return_or_base_type.ref_name = cdd_test_mut_str("TycA");
    ir.nodes_count = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_ir_compute_layout(&ir, NULL));

    nodes[0].return_or_base_type.kind = CDD_FFI_KIND_ENUM_REF;
    nodes[0].return_or_base_type.ref_name = cdd_test_mut_str("TycA");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_ir_compute_layout(&ir, NULL));

    memset(flds, 0, sizeof(flds));
    flds[0].type.kind = CDD_FFI_KIND_STRUCT_REF;
    flds[0].type.ref_name = cdd_test_mut_str("Ucyc");
    nodes[0].kind = CDD_FFI_NODE_UNION;
    nodes[0].name = cdd_test_mut_str("Ucyc");
    nodes[0].fields = flds;
    nodes[0].fields_count = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_ir_compute_layout(&ir, NULL));

    nodes[0].kind = CDD_FFI_NODE_FUNCTION;
    nodes[0].name = cdd_test_mut_str("Fcyc");
    nodes[0].return_or_base_type.kind = CDD_FFI_KIND_STRUCT_REF;
    nodes[0].return_or_base_type.ref_name = cdd_test_mut_str("Ucyc");
    nodes[1].kind = CDD_FFI_NODE_UNION;
    nodes[1].name = cdd_test_mut_str("Ucyc");
    nodes[1].fields = flds;
    nodes[1].fields_count = 1;
    ir.nodes_count = 2;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_ir_compute_layout(&ir, NULL));

    nodes[0].return_or_base_type.kind = CDD_FFI_KIND_INT32;
    nodes[0].return_or_base_type.ref_name = NULL;
    nodes[0].fields = flds;
    nodes[0].fields_count = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_ir_compute_layout(&ir, NULL));

#if !defined(_WIN64)
    /* Size/alignment arithmetic overflows (LP64 / 32-bit where
     * 2 * (size_t)LONG_MAX + 1 == SIZE_MAX) */
    memset(nodes, 0, sizeof(nodes));
    memset(flds, 0, sizeof(flds));
    flds[0].type.kind = CDD_FFI_KIND_UINT64;
    flds[0].type.array_size = LONG_MAX;
    nodes[0].kind = CDD_FFI_NODE_STRUCT;
    nodes[0].name = cdd_test_mut_str("ArrOvf");
    nodes[0].fields = flds;
    nodes[0].fields_count = 1;
    ir.nodes_count = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_ir_compute_layout(&ir, NULL));

    /* 2 * LONG_MAX + 1 = SIZE_MAX: test field addition, alignment, trailing
     * pad, union, and bitfield overflows */
    flds[0].type.kind = CDD_FFI_KIND_UINT8;
    flds[0].type.array_size = LONG_MAX;
    flds[1].type.kind = CDD_FFI_KIND_UINT8;
    flds[1].type.array_size = LONG_MAX;
    flds[2].type.kind = CDD_FFI_KIND_UINT64;
    flds[2].type.array_size = 0;
    nodes[0].fields_count = 3;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_ir_compute_layout(&ir, NULL));

    /* Exact cur_offset = SIZE_MAX (2*LONG_MAX + 1), then
     * align_up_checked(SIZE_MAX, 8) overflows */
    flds[2].type.kind = CDD_FFI_KIND_UINT8;
    flds[3].type.kind = CDD_FFI_KIND_UINT64;
    nodes[0].fields_count = 4;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_ir_compute_layout(&ir, NULL));

    /* Bitfield align overflow and bitfield max_sz - f_size overflow */
    flds[3].bitfield_width = 4;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_ir_compute_layout(&ir, NULL));
    flds[2].bitfield_width = 0;
    flds[2].type.kind = CDD_FFI_KIND_UINT64;
    flds[2].bitfield_width = 4;
    nodes[0].fields_count = 3;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_ir_compute_layout(&ir, NULL));

    /* Struct trailing pad overflow: field0 uint64 (8), field1 uint8[LONG_MAX],
     * field2 uint8[LONG_MAX - 8] -> cur_offset = SIZE_MAX - 1, struct_align = 8
     */
    memset(flds, 0, sizeof(flds));
    flds[0].type.kind = CDD_FFI_KIND_UINT64;
    flds[1].type.kind = CDD_FFI_KIND_UINT8;
    flds[1].type.array_size = LONG_MAX;
    flds[2].type.kind = CDD_FFI_KIND_UINT8;
    flds[2].type.array_size = LONG_MAX - 8;
    nodes[0].fields_count = 3;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_ir_compute_layout(&ir, NULL));

    /* Union align_up_checked overflow: variant0 S_sz_max_minus_1 (SIZE_MAX-1,
     * align 1), variant1 uint64 (8, align 8) */
    nodes[0].is_packed = 1;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_ir_compute_layout(&ir, NULL));
    flds[3].type.kind = CDD_FFI_KIND_STRUCT_REF;
    flds[3].type.ref_name = cdd_test_mut_str("ArrOvf");
    flds[4].type.kind = CDD_FFI_KIND_UINT64;
    nodes[1].kind = CDD_FFI_NODE_UNION;
    nodes[1].name = cdd_test_mut_str("UOvf");
    nodes[1].fields = &flds[3];
    nodes[1].fields_count = 2;
    ir.nodes_count = 2;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_ir_compute_layout(&ir, NULL));
    /* aligned_off = SIZE_MAX - 15 (already 8-aligned), f_size = 32 ->
     * aligned_off > max_sz - f_size */
    memset(flds, 0, sizeof(flds));
    flds[0].type.kind = CDD_FFI_KIND_UINT8;
    flds[0].type.array_size = LONG_MAX - 7;
    flds[1].type.kind = CDD_FFI_KIND_UINT8;
    flds[1].type.array_size = LONG_MAX - 7;
    flds[2].type.kind = CDD_FFI_KIND_UINT64;
    flds[2].type.array_size = 4;
    nodes[0].is_packed = 0;
    nodes[0].fields = flds;
    nodes[0].fields_count = 3;
    ir.nodes_count = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_ir_compute_layout(&ir, NULL));

    /* Same for bitfield: aligned_off = SIZE_MAX - 7 (8-aligned), f_size = 8 ->
     * aligned_off > max_sz - 8 */
    flds[0].type.array_size = LONG_MAX - 3;
    flds[1].type.array_size = LONG_MAX - 3;
    flds[2].type.kind = CDD_FFI_KIND_UINT64;
    flds[2].type.array_size = 0;
    flds[2].bitfield_width = 4;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_ir_compute_layout(&ir, NULL));
#endif
  }

  /* 4. DSL parser all primitives + function-pointer error & OOM paths */
  {
    const char *rich_dsl =
        "func prims(i16 a, u16 b, u32 c, i64 d, isize e, u64 f, usize g, "
        "size_t h, f32 i, f64 j, bool k, "
        "func(func(i32 x) i32 inner, va_list, MyStruct s) func(u8 y) void cb) "
        "i32;";
    const char *cb_dsl = "func f(func(i32 a, va_list, u8 b) i32 cb) i32;";
    const char *cb0_dsl = "func f(func() i32 cb) i32;";
    const char *bad_dsls[5];
    size_t di;
    cdd_dsl_token_list_t toks;
    cdd_ffi_ir_t *parsed = NULL;

    ASSERT_EQ(CDD_C_SUCCESS, cdd_dsl_tokenize(cb0_dsl, &toks));
    for (fail_idx = 1; fail_idx <= 60; ++fail_idx) {
      g_ffi_extractor_alloc_fail = fail_idx;
      rc = cdd_dsl_parse(&toks, &parsed);
      g_ffi_extractor_alloc_fail = 0;
      if (rc == CDD_C_SUCCESS) {
        ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_ir_free(parsed));
        C_CDD_FREE(parsed);
        parsed = NULL;
        break;
      }
      ASSERT(parsed == NULL);
    }
    ASSERT_EQ(CDD_C_SUCCESS, cdd_dsl_token_list_free(&toks));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_dsl_tokenize(cb_dsl, &toks));
    for (fail_idx = 1; fail_idx <= 120; ++fail_idx) {
      g_ffi_extractor_alloc_fail = fail_idx;
      rc = cdd_dsl_parse(&toks, &parsed);
      g_ffi_extractor_alloc_fail = 0;
      if (rc == CDD_C_SUCCESS) {
        ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_ir_free(parsed));
        C_CDD_FREE(parsed);
        parsed = NULL;
        break;
      }
      ASSERT(parsed == NULL);
    }
    ASSERT_EQ(CDD_C_SUCCESS, cdd_dsl_token_list_free(&toks));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_dsl_tokenize(rich_dsl, &toks));
    for (fail_idx = 1; fail_idx <= 160; ++fail_idx) {
      g_ffi_extractor_alloc_fail = fail_idx;
      rc = cdd_dsl_parse(&toks, &parsed);
      g_ffi_extractor_alloc_fail = 0;
      if (rc == CDD_C_SUCCESS) {
        ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_ir_free(parsed));
        C_CDD_FREE(parsed);
        parsed = NULL;
        break;
      }
      ASSERT(parsed == NULL);
    }
    ASSERT_EQ(CDD_C_SUCCESS, cdd_dsl_parse(&toks, &parsed));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_ir_free(parsed));
    C_CDD_FREE(parsed);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_dsl_token_list_free(&toks));

    bad_dsls[0] = "func f(func i32 cb);";
    bad_dsls[1] = "func f(func(i32 a";
    bad_dsls[2] = "func f(func([in] ) i32 cb);";
    bad_dsls[3] = "func f(func(i32 a) ;);";
    bad_dsls[4] = "func f() :;";
    for (di = 0; di < 5; ++di) {
      ASSERT_EQ(CDD_C_SUCCESS, cdd_dsl_tokenize(bad_dsls[di], &toks));
      ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_dsl_parse(&toks, &parsed));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_dsl_token_list_free(&toks));
    }
  }

  /* 5. C/C++ FFI extractor callback, packed, macro, and virtual OOM paths */
  {
    const char *c_hdr_1 = "#pragma pack(push, 1)\n"
                          "#define MY_STR_M \"hello\"\n"
                          "#define MY_FLT_M 2.5\n"
                          "struct SubS { int x; };\n"
                          "union SubU { int y; };\n"
                          "enum SubE { SUB_E0 = 0 };\n"
                          "struct PackedCb {\n"
                          "  struct SubS on_match;\n"
                          "  int plain_int;\n"
                          "};\n"
                          "int sp_fn(int a , int b ) { return a + b; }\n"
                          "int empty_seg_fn(int a , ) { return a; }\n";
    const char *c_hdr_2 =
        "   \n"
        "} /* stray brace */\n"
        "int my_typedef = 1;\n"
        "typedef int NormalTd;\n"
        "int z;typedef struct SubS ( * * RetStructTd ) ( const  char * \t "
        "const p, "
        "struct  SubS *s, union SubU *u, enum  SubE e, void *, const void *, "
        "unsigned int, int very_long_parameter_identifier_name_xyz, "
        "void (*nested)(int, int), ... );\n";
    const char *c_hdr_3 =
        "typedef int (*DupVoidCb)(void);\n"
        "typedef int (*DupVoidCb)(void);\n"
        "typedef int (*DupTdCb)(int a, struct SubS s);\n"
        "typedef int (*DupTdCb)(int a, struct SubS s);\n"
        "typedef struct { int x; } BraceTd;\n"
        "typedef int (*)(int);\n"
        "typedef void (*AbcVoidCb)(int, abc, char, *, void);\n"
        "   (*EmptyRetCb)(int);\n"
        "typedef int (*NoClose1;\n"
        "typedef int (*NoClose2)(int;\n"
        "typedef int (*EmptySegCb)(int a, , *);\n";
    const char *c_hdr_4 = "class ClsSkip { int (*f)(int); };\n"
                          "template <typename T> void tmpl_cb(int (*f)(int));\n"
                          "(*lead_paren)(int);\n"
                          "int )bad_order(int (*f;\n"
                          "int uncl_cb(int (*f;\n"
                          "int *(*ret_fp)(int);\n"
                          "__attribute__((unused)) (*attr_cb)(int);\n"
                          "int __attribute_x(int (*f)(int));\n"
                          "SubS reg_cb_sp ( int (*cb_sp)(void) );\n"
                          "void reg_anon_cb(int (*)(void));\n"
                          "void reg_anon_cb(int (*)(void));\n"
                          "void bad_cb_inner(int (*bad_inner));\n";
    const char *oom_mem_hdr =
        "#pragma pack(1)\n"
        "typedef struct SubS (*OomTd1)(enum SubE e, struct SubS s, int p);\n"
        "SubS oom_cb_fn(int (*)(void));\n"
        "void oom_named_cb(int (*nm)(int));\n"
        "typedef int UnfinishedAtEof";
    const char *struct_cb_raw =
        "#pragma pack(1)\n"
        "struct PackedCb {\n"
        "  int ( * * on_match ) ( void * , unsigned int );\n"
        "  int (*on_plain)(int x);\n"
        "  int (*on_added)(int x);\n"
        "  int (*)(int);\n"
        "  int (not_a_fp)(int);\n"
        "} __attribute__((packed));\n"
        "struct FwdOnly;\n"
        "struct AttrAfter { int x; }; /* packed */\n"
        "struct NoInnerSemi { int (f) }\n"
        "struct NoSemiAtEnd { int x; }\n";
    char long_ret_td[310];
    char long_stmt_s[620];
    char span_4k[4200];
    char long_td[600];
    char long_fn[180];
    char c_hdr[3600];
    char full_struct_raw[5600];
    const char *c_path = "test_ofc_out/b2/ext_cov.h";
    cdd_generate_bindings_config_t cfg;
    cdd_ffi_ir_t *ir_out = NULL;

    memset(long_ret_td, 'a', sizeof(long_ret_td) - 1);
    memcpy(long_ret_td, "typedef struct ", 15);
    memcpy(long_ret_td + 275, " (*LongRetTd)(int);\n", 21);

    memset(long_td, ' ', sizeof(long_td) - 1);
    memcpy(long_td, "typedef int", 11);
    memcpy(long_td + 530, "(*LongTd)(int);\n", 16);
    long_td[546] = '\0';

    memset(long_fn, 'a', 135);
    memcpy(long_fn, "void ", 5);
    memcpy(long_fn + 135, "(int (*cb)(int));\n", 19);

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_snprintf_checked(c_hdr, sizeof(c_hdr), "%s%s%s%s%s%s%s",
                                       c_hdr_1, c_hdr_2, c_hdr_3, c_hdr_4,
                                       long_ret_td, long_td, long_fn));
    ASSERT_EQ(CDD_C_SUCCESS, write_to_file(c_path, c_hdr_1));
    memset(&cfg, 0, sizeof(cfg));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_ir_extract_exports(c_path, c_hdr, &cfg, &ir_out));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_ir_free(ir_out));
    C_CDD_FREE(ir_out);
    ir_out = NULL;

    /* Direct OOM sweep over extract_func_ptr_typedefs_and_packed */
    ASSERT_EQ(CDD_C_SUCCESS, write_to_file(c_path, "int d_v;\n"));
    for (fail_idx = 1; fail_idx <= 60; ++fail_idx) {
      cdd_ffi_ir_t oom_ir;
      memset(&oom_ir, 0, sizeof(oom_ir));
      oom_ir.nodes_count = 1;
      oom_ir.nodes_capacity = 1;
      oom_ir.nodes = (cdd_ffi_ir_node_t *)calloc(1, sizeof(cdd_ffi_ir_node_t));
      oom_ir.nodes[0].kind = CDD_FFI_NODE_STRUCT;
      oom_ir.nodes[0].name = NULL;
      g_ffi_extractor_alloc_fail = fail_idx;
      rc = cdd_ffi_extract_single_file_exports_test(&oom_ir, c_path,
                                                    oom_mem_hdr, &cfg);
      g_ffi_extractor_alloc_fail = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_ir_free(&oom_ir));
      if (rc == CDD_C_SUCCESS)
        break;
    }

    /* Pre-populated union, unnamed node, >4k span, >512 stmt, and PackedCb
     * matching/adding struct callback fields + OOM */
    memset(long_stmt_s, ' ', sizeof(long_stmt_s) - 1);
    memcpy(long_stmt_s, "struct BigStmt { int (", 22);
    memcpy(long_stmt_s + 550, "*big)(int); };\n", 16);
    memset(span_4k, ' ', sizeof(span_4k) - 1);
    memcpy(span_4k, "struct Span4k { int x; }", 24);
    memcpy(span_4k + 4120, ";\n", 3);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_snprintf_checked(full_struct_raw, sizeof(full_struct_raw),
                                       "%s%s%s", long_stmt_s, span_4k,
                                       struct_cb_raw));

    for (fail_idx = 0; fail_idx <= 40; ++fail_idx) {
      cdd_ffi_ir_t pre_ir;
      memset(&pre_ir, 0, sizeof(pre_ir));
      pre_ir.nodes_count = 10;
      pre_ir.nodes_capacity = 12;
      pre_ir.nodes = (cdd_ffi_ir_node_t *)calloc(12, sizeof(cdd_ffi_ir_node_t));
      pre_ir.nodes[0].kind = CDD_FFI_NODE_UNION;
      pre_ir.nodes[0].name = strdup("PreU");
      pre_ir.nodes[1].kind = CDD_FFI_NODE_STRUCT;
      pre_ir.nodes[1].name = NULL;
      pre_ir.nodes[2].kind = CDD_FFI_NODE_STRUCT;
      pre_ir.nodes[2].name = strdup("PackedCb");
      pre_ir.nodes[2].fields_count = 3;
      pre_ir.nodes[2].fields =
          (cdd_ffi_field_t *)calloc(3, sizeof(cdd_ffi_field_t));
      pre_ir.nodes[2].fields[0].name = NULL;
      pre_ir.nodes[2].fields[1].name = strdup("on_match");
      pre_ir.nodes[2].fields[1].type.kind = CDD_FFI_KIND_STRUCT_REF;
      pre_ir.nodes[2].fields[1].type.ref_name = strdup("OldRef");
      pre_ir.nodes[2].fields[2].name = strdup("on_plain");
      pre_ir.nodes[2].fields[2].type.kind = CDD_FFI_KIND_INT32;
      pre_ir.nodes[3].kind = CDD_FFI_NODE_STRUCT;
      pre_ir.nodes[3].name = strdup("FwdOnly");
      pre_ir.nodes[4].kind = CDD_FFI_NODE_STRUCT;
      pre_ir.nodes[4].name = strdup("NoInnerSemi");
      pre_ir.nodes[5].kind = CDD_FFI_NODE_STRUCT;
      pre_ir.nodes[5].name = strdup("NoSemiAtEnd");
      pre_ir.nodes[6].kind = CDD_FFI_NODE_STRUCT;
      pre_ir.nodes[6].name = strdup("AttrAfter");
      pre_ir.nodes[7].kind = CDD_FFI_NODE_STRUCT;
      pre_ir.nodes[7].name = strdup("BigStmt");
      pre_ir.nodes[8].kind = CDD_FFI_NODE_STRUCT;
      pre_ir.nodes[8].name = strdup("Span4k");
      pre_ir.nodes[9].kind = CDD_FFI_NODE_STRUCT;
      pre_ir.nodes[9].name = strdup("UnclosedTail");

      if (fail_idx > 0)
        g_ffi_extractor_alloc_fail = fail_idx;
      rc = cdd_ffi_extract_single_file_exports_test(&pre_ir, c_path,
                                                    full_struct_raw, &cfg);
      g_ffi_extractor_alloc_fail = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_ir_free(&pre_ir));
      if (fail_idx > 0 && rc == CDD_C_SUCCESS)
        break;
    }

    /* Also test unclosed `{` at very end of buffer (`rbrace == NULL`) */
    {
      cdd_ffi_ir_t uncl_ir;
      memset(&uncl_ir, 0, sizeof(uncl_ir));
      uncl_ir.nodes_count = 1;
      uncl_ir.nodes_capacity = 2;
      uncl_ir.nodes = (cdd_ffi_ir_node_t *)calloc(2, sizeof(cdd_ffi_ir_node_t));
      uncl_ir.nodes[0].kind = CDD_FFI_NODE_STRUCT;
      uncl_ir.nodes[0].name = strdup("UnclosedTail");
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_ffi_extract_single_file_exports_test(
                    &uncl_ir, c_path, "/* struct UnclosedTail { */", &cfg));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_ir_free(&uncl_ir));
    }
  }

  PASS();
}

/**
 * @brief Exercises remaining sanitizer, literal-normalizer, ABI-mapping,
 * zero-arg/anon/union/bitfield/array/callback/shim, and `g_fail_io_after`
 * branches across `cdd_ffi_ir_extractor.c`, `cdd_ffi_emit_ocaml.c`,
 * `cdd_ffi_emit_fortran.c`, and `cdd_ffi_emit_cobol.c`.
 *
 * @return Greatest test result.
 */
TEST test_ffi_batch3_100_percent_edges(void) {
  char buf[128];
  char small4[4];
  char small5[5];
  char small8[8];
  char long_id[280];
  char long_digits[150];
  char long_fdigits[150];
  size_t idx;
  int k;
  cdd_c_error_t rc;

  for (idx = 0; idx < 270; idx++) {
    long_id[idx] = (char)('a' + (idx % 26));
  }
  long_id[270] = '\0';

  for (idx = 0; idx < 140; idx++) {
    long_digits[idx] = '9';
    long_fdigits[idx] = (idx == 5) ? '.' : '5';
  }
  long_digits[140] = '\0';
  long_fdigits[140] = '\0';

  ASSERT_EQ(CDD_C_SUCCESS, makedirs("test_ofc_out/b3"));

  /* 1. Remaining 3 branches in cdd_ffi_ir_extractor.c */
  {
    cdd_ffi_ir_t ext_ir;
    cdd_generate_bindings_config_t ecfg;
    ASSERT_EQ(CDD_C_SUCCESS, write_to_file("test_ofc_out/b3/nobrace.h",
                                           "struct OnlyFwdNoBraces;\n"));
    ASSERT_EQ(CDD_C_SUCCESS,
              write_to_file(
                  "test_ofc_out/b3/f13.h",
                  "typedef (*implicit_ret_cb)(int a);\n"
                  "int my_variad_f13(int a, ...);\n"
                  "int my_fn_with_13(void (*cb)(int));\n"
                  "int __attribute__((cdecl)) fn_attr_cb(void (*cb)(int));\n"));
    memset(&ecfg, 0, sizeof(ecfg));
    memset(&ext_ir, 0, sizeof(ext_ir));
    ext_ir.nodes_count = 1;
    ext_ir.nodes_capacity = 2;
    ext_ir.nodes = (cdd_ffi_ir_node_t *)calloc(2, sizeof(cdd_ffi_ir_node_t));
    ext_ir.nodes[0].kind = CDD_FFI_NODE_STRUCT;
    ext_ir.nodes[0].name = strdup("OnlyFwdNoBraces");
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_extract_single_file_exports_test(
                                 &ext_ir, "test_ofc_out/b3/nobrace.h",
                                 "struct OnlyFwdNoBraces;\n", &ecfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_ir_free(&ext_ir));

    memset(&ext_ir, 0, sizeof(ext_ir));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_extract_single_file_exports_test(
                  &ext_ir, "test_ofc_out/b3/f13.h",
                  "typedef (*implicit_ret_cb)(int a);\n"
                  "int my_variad_f13(int a, ...);\n"
                  "int my_fn_with_13(void (*cb)(int));\n"
                  "int __attribute__((cdecl)) fn_attr_cb(void (*cb)(int));\n",
                  &ecfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_ir_free(&ext_ir));
  }

  /* 2. OCaml sanitizer & emitter edge cases */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ocaml_sanitize_lid(NULL, buf, sizeof(buf)));
  ASSERT_STR_EQ("unnamed_", buf);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ocaml_sanitize_lid("", buf, sizeof(buf)));
  ASSERT_STR_EQ("unnamed_", buf);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ocaml_sanitize_lid("foo1Bar", buf, sizeof(buf)));
  ASSERT_STR_EQ("foo1_bar", buf);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ocaml_sanitize_lid("_Bar", buf, sizeof(buf)));
  ASSERT_STR_EQ("_bar", buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ocaml_sanitize_lid("abC", small5, sizeof(small5)));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ocaml_sanitize_lid("'abc", buf, sizeof(buf)));
  ASSERT_STR_EQ("v_'abc", buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ocaml_sanitize_lid("999", small4, sizeof(small4)));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ocaml_sanitize_lid("foo--bar", buf, sizeof(buf)));
  ASSERT_STR_EQ("foo_bar", buf);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ocaml_sanitize_lid("-", buf, sizeof(buf)));
  ASSERT_STR_EQ("unnamed_", buf);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ocaml_sanitize_lid("---", buf, sizeof(buf)));
  ASSERT_STR_EQ("unnamed_", buf);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_ocaml_sanitize_uid(NULL, buf, sizeof(buf)));
  ASSERT_STR_EQ("Unnamed", buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ocaml_sanitize_uid("Foo--Bar", buf, sizeof(buf)));
  ASSERT_STR_EQ("Foo_Bar", buf);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ocaml_sanitize_uid(long_id, buf, sizeof(buf)));

  /* 3. Fortran sanitizer & line-wrapper edge cases */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_fortran_sanitize_ident(NULL, 3, buf, sizeof(buf)));
  ASSERT_STR_EQ("arg_3", buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_fortran_sanitize_ident("123_abc", 0, buf, sizeof(buf)));
  ASSERT_STR_EQ("f_123_abc", buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_fortran_sanitize_ident("_abc", 0, buf, sizeof(buf)));
  ASSERT_STR_EQ("f_abc", buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_fortran_sanitize_ident("a__b__", 0, buf, sizeof(buf)));
  ASSERT_STR_EQ("a_b", buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_fortran_sanitize_ident(
                "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghi"
                "jklmnopqr",
                0, buf, sizeof(buf)));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_fortran_sanitize_ident(long_id, 0, buf, sizeof(buf)));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_fortran_sanitize_ident("integer", 0, small8, sizeof(small8)));

  {
    FILE *wfp = NULL;
    char long_cmt[280];
    char long_nosp[280];
    char long_sq[280];
    char long_sp[280];
    char all_sp[150];
    char forced_sp[150];

    long_cmt[0] = '\t';
    long_cmt[1] = '!';
    long_cmt[2] = ' ';
    for (idx = 3; idx < 265; idx++)
      long_cmt[idx] = 'c';
    long_cmt[265] = '\r';
    long_cmt[266] = '\n';
    long_cmt[267] = '\0';

    for (idx = 0; idx < 260; idx++)
      long_nosp[idx] = 'x';
    long_nosp[260] = '\0';

    long_sq[0] = '\'';
    for (idx = 1; idx < 258; idx++)
      long_sq[idx] = 'q';
    long_sq[258] = '\'';
    long_sq[259] = '\0';

    for (idx = 0; idx < 110; idx++)
      long_sp[idx] = 'a';
    long_sp[110] = ',';
    long_sp[111] = ' ';
    long_sp[112] = ' ';
    long_sp[113] = '"';
    for (idx = 114; idx < 245; idx++)
      long_sp[idx] = 's';
    long_sp[245] = '"';
    long_sp[246] = '\0';

    for (idx = 0; idx < 135; idx++)
      all_sp[idx] = ' ';
    all_sp[135] = '\0';

    for (idx = 0; idx < 120; idx++)
      forced_sp[idx] = 'x';
    forced_sp[120] = ' ';
    forced_sp[121] = ' ';
    forced_sp[122] = 'y';
    forced_sp[123] = '\0';

    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_open_output_file("test_ofc_out/b3",
                                                      "wrap_edge.f90", &wfp));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_fortran_write_wrapped_line(wfp, ""));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_fortran_write_wrapped_line(wfp, "\r\n"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_fortran_write_wrapped_line(wfp, long_cmt));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_fortran_write_wrapped_line(wfp, long_nosp));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_fortran_write_wrapped_line(wfp, long_sq));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_fortran_write_wrapped_line(wfp, long_sp));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_fortran_write_wrapped_line(wfp, all_sp));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_fortran_write_wrapped_line(wfp, forced_sp));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_close_output_file(&wfp));

    for (k = 1; k <= 12; k++) {
      ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_open_output_file("test_ofc_out/b3",
                                                        "wrap_fail.f90", &wfp));
      g_fail_io_after = k;
      rc = cdd_fortran_write_wrapped_line(wfp, long_cmt);
      g_fail_io_after = -1;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_close_output_file(&wfp));
      if (rc == CDD_C_SUCCESS)
        break;
      ASSERT_EQ(CDD_C_ERROR_IO, rc);
    }
    for (k = 1; k <= 12; k++) {
      ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_open_output_file(
                                   "test_ofc_out/b3", "wrap_fail2.f90", &wfp));
      g_fail_io_after = k;
      rc = cdd_fortran_write_wrapped_line(wfp, long_sp);
      g_fail_io_after = -1;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_close_output_file(&wfp));
      if (rc == CDD_C_SUCCESS)
        break;
      ASSERT_EQ(CDD_C_ERROR_IO, rc);
    }
    for (k = 1; k <= 12; k++) {
      ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_open_output_file(
                                   "test_ofc_out/b3", "wrap_fail3.f90", &wfp));
      g_fail_io_after = k;
      rc = cdd_fortran_write_wrapped_line(wfp, all_sp);
      g_fail_io_after = -1;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_close_output_file(&wfp));
      if (rc == CDD_C_SUCCESS)
        break;
      ASSERT_EQ(CDD_C_ERROR_IO, rc);
    }
  }

  /* 4. COBOL word sanitizer edge cases */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cobol_sanitize_word(NULL, NULL, 0, buf, sizeof(buf)));
  ASSERT_STR_EQ("UNNAMED", buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cobol_sanitize_word(NULL, NULL, 1, buf, sizeof(buf)));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cobol_sanitize_word("", "", 0, buf, sizeof(buf)));
  ASSERT_STR_EQ("UNNAMED", buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cobol_sanitize_word("", NULL, 1, buf, sizeof(buf)));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cobol_sanitize_word("XMLParser2Test", NULL, 0,
                                                   buf, sizeof(buf)));
  ASSERT_STR_EQ("XML-PARSER2-TEST", buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cobol_sanitize_word("a_Bc", NULL, 0, buf, sizeof(buf)));
  ASSERT_STR_EQ("A-BC", buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cobol_sanitize_word("XML", NULL, 0, buf, sizeof(buf)));
  ASSERT_STR_EQ("XML", buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cobol_sanitize_word("foo---", NULL, 0, buf, sizeof(buf)));
  ASSERT_STR_EQ("FOO", buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cobol_sanitize_word(long_id, NULL, 0, buf, sizeof(buf)));
  ASSERT_EQ(
      CDD_C_ERROR_MEMORY,
      cdd_cobol_sanitize_word("long_name", NULL, 123, small8, sizeof(small8)));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cobol_sanitize_word("long_name", NULL, 12,
                                                   small8, sizeof(small8)));

  /* 5. Rich multi-emitter IR covering all remaining OCaml, Fortran & COBOL
   * branches + full I/O & OOM fault sweeps */
  {
    cdd_ffi_ir_t ir;
    cdd_ffi_ir_node_t nodes[56];
    cdd_ffi_trivia_t triv_top;
    cdd_ffi_trivia_t triv_fn;
    cdd_ffi_enum_variant_t m_var_null[1];
    cdd_ffi_enum_variant_t m_var_val[1];
    cdd_ffi_number_t num_null_spell;
    cdd_ffi_enum_variant_t evars[12];
    cdd_ffi_field_t err_fields[2];
    cdd_ffi_field_t anon_s_fields[1];
    cdd_ffi_field_t zero_sz_fields[1];
    cdd_ffi_field_t rich_s_fields[25];
    cdd_ffi_field_t u_fields[7];
    cdd_ffi_type_t cb0_ret;
    cdd_ffi_type_t cb_voidp_ret;
    cdd_ffi_type_t cb2_ret;
    cdd_ffi_field_t cb2_params[2];
    cdd_ffi_field_t fn_outs_fields[24];
    cdd_ffi_field_t fn_wrap_fields[5];
    cdd_ffi_field_t fn_farr_fields[17];
    cdd_ffi_field_t fn_cstr_ptr_fields[1];
    cdd_ffi_field_t fn_u_byval_fields[2];
    cdd_generate_bindings_config_t cfg;
    char id63_a[65];
    char long_hex[145];
    char long_bin[145];
    char long_oct[145];

    for (idx = 0; idx < 63; idx++)
      id63_a[idx] = 'z';
    id63_a[63] = '\0';

    long_hex[0] = '0';
    long_hex[1] = 'x';
    long_bin[0] = '0';
    long_bin[1] = 'b';
    long_oct[0] = '0';
    for (idx = 2; idx < 140; idx++) {
      long_hex[idx] = 'F';
      long_bin[idx] = '1';
      long_oct[idx - 1] = '7';
    }
    long_oct[139] = '7';
    long_hex[140] = '\0';
    long_bin[140] = '\0';
    long_oct[140] = '\0';

    memset(&ir, 0, sizeof(ir));
    memset(nodes, 0, sizeof(nodes));
    memset(&triv_top, 0, sizeof(triv_top));
    memset(&triv_fn, 0, sizeof(triv_fn));
    memset(m_var_null, 0, sizeof(m_var_null));
    memset(m_var_val, 0, sizeof(m_var_val));
    memset(&num_null_spell, 0, sizeof(num_null_spell));
    memset(evars, 0, sizeof(evars));
    memset(err_fields, 0, sizeof(err_fields));
    memset(anon_s_fields, 0, sizeof(anon_s_fields));
    memset(zero_sz_fields, 0, sizeof(zero_sz_fields));
    memset(rich_s_fields, 0, sizeof(rich_s_fields));
    memset(u_fields, 0, sizeof(u_fields));
    memset(&cb0_ret, 0, sizeof(cb0_ret));
    memset(&cb_voidp_ret, 0, sizeof(cb_voidp_ret));
    memset(&cb2_ret, 0, sizeof(cb2_ret));
    memset(cb2_params, 0, sizeof(cb2_params));
    memset(fn_outs_fields, 0, sizeof(fn_outs_fields));
    memset(fn_wrap_fields, 0, sizeof(fn_wrap_fields));
    memset(fn_farr_fields, 0, sizeof(fn_farr_fields));
    memset(fn_cstr_ptr_fields, 0, sizeof(fn_cstr_ptr_fields));
    memset(fn_u_byval_fields, 0, sizeof(fn_u_byval_fields));
    memset(&cfg, 0, sizeof(cfg));

    /* 0: Unnamed non-empty union BEFORE cdd_ffi_error_t (tests name == NULL in
     * builtin-err scan AND unnamed union in OCaml/COBOL tests/shim!) */
    triv_top.kind = CDD_FFI_TRIVIA_COMMENT_BLOCK;
    triv_top.text = cdd_test_mut_str("/* top trivia */");
    anon_s_fields[0].name = cdd_test_mut_str("");
    anon_s_fields[0].doc = cdd_test_mut_str("Anon field doc");
    anon_s_fields[0].type.kind = CDD_FFI_KIND_INT32;
    nodes[0].kind = CDD_FFI_NODE_UNION;
    nodes[0].name = NULL;
    nodes[0].leading_trivia = &triv_top;
    nodes[0].doc = cdd_test_mut_str("Top union doc");
    nodes[0].fields = anon_s_fields;
    nodes[0].fields_count = 1;

    /* 1: Macro named "cdd_ffi_error_t" BEFORE struct cdd_ffi_error_t */
    nodes[1].kind = CDD_FFI_NODE_MACRO;
    nodes[1].name = cdd_test_mut_str("cdd_ffi_error_t");
    nodes[1].inferred_type = CDD_FFI_MACRO_TYPE_INT;
    nodes[1].evaluated_value = cdd_test_mut_str("1_000");

    /* 2: Macro named "MyDupType" + variants[0].value == NULL */
    nodes[2].kind = CDD_FFI_NODE_MACRO;
    nodes[2].name = cdd_test_mut_str("MyDupType");
    nodes[2].variants = m_var_null;
    nodes[2].variants_count = 1;

    /* 3: Macro with variants[0].value != NULL (" ( + 42u ) ") */
    m_var_val[0].value = cdd_test_mut_str(" ( + 42u ) ");
    nodes[3].kind = CDD_FFI_NODE_MACRO;
    nodes[3].name = cdd_test_mut_str(id63_a);
    nodes[3].inferred_type = CDD_FFI_MACRO_TYPE_INT;
    nodes[3].variants = m_var_val;
    nodes[3].variants_count = 1;

    /* 4..20: Int & Float & String literal normalizer edge macros */
    nodes[4].kind = CDD_FFI_NODE_MACRO;
    nodes[4].name = cdd_test_mut_str(id63_a); /* 63-char collision! */
    nodes[4].inferred_type = CDD_FFI_MACRO_TYPE_INT;
    nodes[4].evaluated_value = cdd_test_mut_str("");

    nodes[5].kind = CDD_FFI_NODE_MACRO;
    nodes[5].name = cdd_test_mut_str("M_EMPTY_LEN");
    nodes[5].inferred_type = CDD_FFI_MACRO_TYPE_INT;
    nodes[5].evaluated_value = cdd_test_mut_str(" ( + u ) ");

    nodes[6].kind = CDD_FFI_NODE_MACRO;
    nodes[6].name = cdd_test_mut_str("M_BAD_HEX");
    nodes[6].inferred_type = CDD_FFI_MACRO_TYPE_INT;
    nodes[6].evaluated_value = cdd_test_mut_str(" - 0x1'Z ");

    nodes[7].kind = CDD_FFI_NODE_MACRO;
    nodes[7].name = cdd_test_mut_str("M_NEG_HEX");
    nodes[7].inferred_type = CDD_FFI_MACRO_TYPE_INT;
    nodes[7].evaluated_value = cdd_test_mut_str("-0x1A");

    nodes[8].kind = CDD_FFI_NODE_MACRO;
    nodes[8].name = cdd_test_mut_str("M_LONG_HEX");
    nodes[8].inferred_type = CDD_FFI_MACRO_TYPE_INT;
    nodes[8].evaluated_value = long_hex;

    nodes[9].kind = CDD_FFI_NODE_MACRO;
    nodes[9].name = cdd_test_mut_str("M_BAD_BIN");
    nodes[9].inferred_type = CDD_FFI_MACRO_TYPE_INT;
    nodes[9].evaluated_value = cdd_test_mut_str("-0b1'2");

    nodes[10].kind = CDD_FFI_NODE_MACRO;
    nodes[10].name = cdd_test_mut_str("M_LONG_BIN");
    nodes[10].inferred_type = CDD_FFI_MACRO_TYPE_INT;
    nodes[10].evaluated_value = long_bin;

    nodes[11].kind = CDD_FFI_NODE_MACRO;
    nodes[11].name = cdd_test_mut_str("M_BAD_OCT");
    nodes[11].inferred_type = CDD_FFI_MACRO_TYPE_INT;
    nodes[11].evaluated_value = cdd_test_mut_str("-07'9");

    nodes[12].kind = CDD_FFI_NODE_MACRO;
    nodes[12].name = cdd_test_mut_str("M_LONG_OCT");
    nodes[12].inferred_type = CDD_FFI_MACRO_TYPE_INT;
    nodes[12].evaluated_value = long_oct;

    nodes[13].kind = CDD_FFI_NODE_MACRO;
    nodes[13].name = cdd_test_mut_str("M_BAD_DEC");
    nodes[13].inferred_type = CDD_FFI_MACRO_TYPE_INT;
    nodes[13].evaluated_value = cdd_test_mut_str("12$3");

    nodes[14].kind = CDD_FFI_NODE_MACRO;
    nodes[14].name = cdd_test_mut_str("M_APOS_ONLY");
    nodes[14].inferred_type = CDD_FFI_MACRO_TYPE_INT;
    nodes[14].evaluated_value = cdd_test_mut_str("'''");

    nodes[15].kind = CDD_FFI_NODE_MACRO;
    nodes[15].name = cdd_test_mut_str("M_HUGE_INT");
    nodes[15].inferred_type = CDD_FFI_MACRO_TYPE_INT;
    nodes[15].evaluated_value = long_digits;

    nodes[16].kind = CDD_FFI_NODE_MACRO;
    nodes[16].name = cdd_test_mut_str("M_FLT_EMPTY");
    nodes[16].inferred_type = CDD_FFI_MACRO_TYPE_FLOAT;
    nodes[16].evaluated_value = cdd_test_mut_str("");

    nodes[17].kind = CDD_FFI_NODE_MACRO;
    nodes[17].name = cdd_test_mut_str("M_FLT_SEP");
    nodes[17].inferred_type = CDD_FFI_MACRO_TYPE_FLOAT;
    nodes[17].evaluated_value = cdd_test_mut_str("1'000.5");

    nodes[18].kind = CDD_FFI_NODE_MACRO;
    nodes[18].name = cdd_test_mut_str("M_FLT_NOCH");
    nodes[18].inferred_type = CDD_FFI_MACRO_TYPE_FLOAT;
    nodes[18].evaluated_value = cdd_test_mut_str("abc");

    nodes[19].kind = CDD_FFI_NODE_MACRO;
    nodes[19].name = cdd_test_mut_str("M_FLT_NODOT");
    nodes[19].inferred_type = CDD_FFI_MACRO_TYPE_FLOAT;
    nodes[19].evaluated_value = cdd_test_mut_str("42");

    nodes[20].kind = CDD_FFI_NODE_MACRO;
    nodes[20].name = cdd_test_mut_str("M_FLT_HUGE");
    nodes[20].inferred_type = CDD_FFI_MACRO_TYPE_FLOAT;
    nodes[20].evaluated_value = long_fdigits;

    nodes[21].kind = CDD_FFI_NODE_MACRO;
    nodes[21].name = cdd_test_mut_str("M_STR_UNQ");
    nodes[21].inferred_type = CDD_FFI_MACRO_TYPE_STRING;
    nodes[21].evaluated_value = cdd_test_mut_str("raw_unquoted");

    nodes[22].kind = CDD_FFI_NODE_MACRO;
    nodes[22].name = cdd_test_mut_str("M_UNKNOWN_KIND");
    nodes[22].inferred_type = CDD_FFI_MACRO_TYPE_UNKNOWN;
    nodes[22].evaluated_value = cdd_test_mut_str("foo");

    /* 23: Enum with doc, binary, octal 07'7, neg hex, zero, 08, null spell */
    evars[0].name = cdd_test_mut_str("EV_BIN");
    evars[0].doc = cdd_test_mut_str("Enum var doc");
    evars[0].value = cdd_test_mut_str("0b1'01");
    evars[1].name = cdd_test_mut_str("EV_NEG_BIN");
    evars[1].value = cdd_test_mut_str("-0b10");
    evars[2].name = cdd_test_mut_str("EV_OCT");
    evars[2].value = cdd_test_mut_str("07'7");
    evars[3].name = cdd_test_mut_str("EV_NEG_DEC");
    evars[3].value = cdd_test_mut_str("-42");
    evars[4].name = cdd_test_mut_str("EV_BAD_OCT_LOW");
    evars[4].value = cdd_test_mut_str("07$");
    evars[5].name = cdd_test_mut_str("EV_ZERO_APOS");
    evars[5].value = cdd_test_mut_str("0'7");
    evars[6].name = cdd_test_mut_str("EV_08");
    evars[6].value = cdd_test_mut_str("08");
    evars[7].name = cdd_test_mut_str("EV_HEX_SEP");
    evars[7].value = cdd_test_mut_str("0x1'A");
    evars[8].name = cdd_test_mut_str("EV_NEG_HEX");
    evars[8].value = cdd_test_mut_str("-0x1A");
    evars[9].name = cdd_test_mut_str("EV_NULL_SPELL");
    evars[9].value = cdd_test_mut_str("1'2");
    evars[9].number_details = &num_null_spell;
    evars[10].name = cdd_test_mut_str("EV_NEG_OCT");
    evars[10].value = cdd_test_mut_str("-077");
    evars[11].name = cdd_test_mut_str("EV_AUTO");
    evars[11].value = NULL;
    nodes[23].kind = CDD_FFI_NODE_ENUM;
    nodes[23].name = cdd_test_mut_str("EdgeEnum");
    nodes[23].variants = evars;
    nodes[23].variants_count = 12;

    /* 24: Unnamed non-empty enum */
    nodes[24].kind = CDD_FFI_NODE_ENUM;
    nodes[24].name = NULL;
    nodes[24].variants = evars;
    nodes[24].variants_count = 1;

    /* 25: Built-in cdd_ffi_error_t struct present in IR! */
    err_fields[0].name = cdd_test_mut_str("code");
    err_fields[0].type.kind = CDD_FFI_KIND_INT32;
    err_fields[1].name = cdd_test_mut_str("message");
    err_fields[1].type.kind = CDD_FFI_KIND_INT8;
    err_fields[1].type.pointer_depth = 1;
    err_fields[1].type.is_const = 1;
    nodes[25].kind = CDD_FFI_NODE_STRUCT;
    nodes[25].name = cdd_test_mut_str("cdd_ffi_error_t");
    nodes[25].fields = err_fields;
    nodes[25].fields_count = 2;

    /* 26: Opaque struct (0 fields) */
    nodes[26].kind = CDD_FFI_NODE_STRUCT;
    nodes[26].name = cdd_test_mut_str("OpaqueRec");

    /* 27: Empty union (0 fields) */
    nodes[27].kind = CDD_FFI_NODE_UNION;
    nodes[27].name = cdd_test_mut_str("EmptyUnion");

    /* 28: Unnamed struct */
    nodes[28].kind = CDD_FFI_NODE_STRUCT;
    nodes[28].name = NULL;
    nodes[28].fields = anon_s_fields;
    nodes[28].fields_count = 1;

    /* 29..30: "my_sub_t" and "sub_t" structs (substring test for
     * fortran_import_add left_ok == 0!) */
    nodes[29].kind = CDD_FFI_NODE_STRUCT;
    nodes[29].name = cdd_test_mut_str("my_sub_t");
    nodes[29].fields = anon_s_fields;
    nodes[29].fields_count = 1;

    nodes[30].kind = CDD_FFI_NODE_STRUCT;
    nodes[30].name = cdd_test_mut_str("sub_t");
    nodes[30].fields = anon_s_fields;
    nodes[30].fields_count = 1;

    /* 31: Zero-byte non-empty struct ZeroSz { MissingStruct x; } */
    zero_sz_fields[0].name = cdd_test_mut_str("x");
    zero_sz_fields[0].type.kind = CDD_FFI_KIND_STRUCT_REF;
    zero_sz_fields[0].type.ref_name = cdd_test_mut_str("MissingStruct");
    nodes[31].kind = CDD_FFI_NODE_STRUCT;
    nodes[31].name = cdd_test_mut_str("ZeroSz");
    nodes[31].fields = zero_sz_fields;
    nodes[31].fields_count = 1;

    /* 32: Function named "MyDupType" BEFORE struct MyDupType */
    triv_fn.kind = CDD_FFI_TRIVIA_COMMENT_BLOCK;
    triv_fn.text = cdd_test_mut_str("/* fn trivia */");
    nodes[32].kind = CDD_FFI_NODE_FUNCTION;
    nodes[32].name = cdd_test_mut_str("MyDupType");
    nodes[32].leading_trivia = &triv_fn;
    nodes[32].doc = cdd_test_mut_str("Fn doc");
    nodes[32].return_or_base_type.kind = CDD_FFI_KIND_VOID;

    /* 33: Struct "MyDupType" */
    rich_s_fields[0].name = cdd_test_mut_str("9lead-i32");
    rich_s_fields[0].doc = cdd_test_mut_str("Lead field doc");
    rich_s_fields[0].type.kind = CDD_FFI_KIND_INT32;
    rich_s_fields[1].name = cdd_test_mut_str("bf_a");
    rich_s_fields[1].type.kind = CDD_FFI_KIND_UINT32;
    rich_s_fields[1].bitfield_width = 3;
    rich_s_fields[2].name = cdd_test_mut_str("bf_b");
    rich_s_fields[2].type.kind = CDD_FFI_KIND_UINT32;
    rich_s_fields[2].bitfield_width = 5;
    rich_s_fields[3].name = NULL;
    rich_s_fields[3].type.kind = CDD_FFI_KIND_UINT32;
    rich_s_fields[3].bitfield_width = 2;
    rich_s_fields[4].name = long_id;
    rich_s_fields[4].type.kind = CDD_FFI_KIND_STD_SHARED_PTR;
    rich_s_fields[5].name = cdd_test_mut_str("uptr");
    rich_s_fields[5].type.kind = CDD_FFI_KIND_STD_UNIQUE_PTR;
    rich_s_fields[6].name = cdd_test_mut_str("uiptr");
    rich_s_fields[6].type.kind = CDD_FFI_KIND_TYPEDEF_REF;
    rich_s_fields[6].type.ref_name = cdd_test_mut_str("uintptr_t");
    rich_s_fields[7].name = cdd_test_mut_str("unres_td");
    rich_s_fields[7].type.kind = CDD_FFI_KIND_TYPEDEF_REF;
    rich_s_fields[7].type.ref_name = cdd_test_mut_str("MissingTypedef");
    rich_s_fields[8].name = cdd_test_mut_str("enum_as_td");
    rich_s_fields[8].type.kind = CDD_FFI_KIND_TYPEDEF_REF;
    rich_s_fields[8].type.ref_name = cdd_test_mut_str("EdgeEnum");
    rich_s_fields[9].name = cdd_test_mut_str("enum_as_st");
    rich_s_fields[9].type.kind = CDD_FFI_KIND_STRUCT_REF;
    rich_s_fields[9].type.ref_name = cdd_test_mut_str("EdgeEnum");
    rich_s_fields[10].name = cdd_test_mut_str("unres_st_arr");
    rich_s_fields[10].type.kind = CDD_FFI_KIND_STRUCT_REF;
    rich_s_fields[10].type.ref_name = cdd_test_mut_str("MissingStruct");
    rich_s_fields[10].type.array_size = 3;
    rich_s_fields[11].name = cdd_test_mut_str("unres_en");
    rich_s_fields[11].type.kind = CDD_FFI_KIND_ENUM_REF;
    rich_s_fields[11].type.ref_name = cdd_test_mut_str("MissingEnum");
    rich_s_fields[12].name = cdd_test_mut_str("c_arr");
    rich_s_fields[12].type.kind = CDD_FFI_KIND_INT32;
    rich_s_fields[12].type.array_size = 2;
    rich_s_fields[12].type.is_const = 1;
    rich_s_fields[13].name = cdd_test_mut_str("cstr_arr");
    rich_s_fields[13].type.kind = CDD_FFI_KIND_INT8;
    rich_s_fields[13].type.pointer_depth = 1;
    rich_s_fields[13].type.array_size = 4;
    rich_s_fields[14].name = cdd_test_mut_str("bool_ptr");
    rich_s_fields[14].type.kind = CDD_FFI_KIND_BOOL;
    rich_s_fields[14].type.pointer_depth = 1;
    rich_s_fields[15].name = cdd_test_mut_str("c_ptr");
    rich_s_fields[15].type.kind = CDD_FFI_KIND_INT32;
    rich_s_fields[15].type.pointer_depth = 1;
    rich_s_fields[15].type.is_const = 1;
    rich_s_fields[16].name = cdd_test_mut_str("deep_ptr");
    rich_s_fields[16].type.kind = CDD_FFI_KIND_INT32;
    rich_s_fields[16].type.pointer_depth = 35;
    cb0_ret.kind = CDD_FFI_KIND_VOID;
    rich_s_fields[17].name = NULL;
    rich_s_fields[17].type.kind = CDD_FFI_KIND_FUNCTION_PTR;
    rich_s_fields[17].type.fn_return_type = &cb0_ret;
    rich_s_fields[18].name = cdd_test_mut_str("var_cb");
    rich_s_fields[18].type.kind = CDD_FFI_KIND_FUNCTION_PTR;
    rich_s_fields[18].type.fn_return_type = &cb0_ret;
    rich_s_fields[18].type.fn_is_variadic = 1;
    rich_s_fields[19].name = cdd_test_mut_str("tmpl_fld");
    rich_s_fields[19].type.kind = CDD_FFI_KIND_TEMPLATE_STRUCT_REF;
    rich_s_fields[19].type.ref_name = cdd_test_mut_str("sub_t");
    rich_s_fields[20].name = cdd_test_mut_str("null_ref_st");
    rich_s_fields[20].type.kind = CDD_FFI_KIND_STRUCT_REF;
    rich_s_fields[20].type.ref_name = NULL;
    rich_s_fields[21].name = cdd_test_mut_str("f32_v");
    rich_s_fields[21].type.kind = CDD_FFI_KIND_FLOAT32;
    rich_s_fields[22].name = cdd_test_mut_str("f64_v");
    rich_s_fields[22].type.kind = CDD_FFI_KIND_FLOAT64;
    rich_s_fields[23].name = cdd_test_mut_str("fixed_cstr");
    rich_s_fields[23].type.kind = CDD_FFI_KIND_INT8;
    rich_s_fields[23].type.array_size = 8;
    rich_s_fields[24].name = cdd_test_mut_str("tail_b");
    rich_s_fields[24].type.kind = CDD_FFI_KIND_INT8;
    nodes[33].kind = CDD_FFI_NODE_STRUCT;
    nodes[33].name = cdd_test_mut_str("MyDupType");
    nodes[33].fields = rich_s_fields;
    nodes[33].fields_count = 25;

    /* 34: Union "RichUnion" with zero-sz array, normal array, ptr, opaque_ptr,
     * funptr, opaque struct, and MyDupType (triggers MyDupType lookup!) */
    u_fields[0].name = cdd_test_mut_str("u_zarr");
    u_fields[0].type.kind = CDD_FFI_KIND_STRUCT_REF;
    u_fields[0].type.ref_name = cdd_test_mut_str("MissingStruct");
    u_fields[0].type.array_size = 2;
    u_fields[1].name = cdd_test_mut_str("u_arr");
    u_fields[1].type.kind = CDD_FFI_KIND_INT32;
    u_fields[1].type.array_size = 2;
    u_fields[2].name = cdd_test_mut_str("u_ptr");
    u_fields[2].type.kind = CDD_FFI_KIND_VOID;
    u_fields[2].type.pointer_depth = 1;
    u_fields[3].name = cdd_test_mut_str("u_opqp");
    u_fields[3].type.kind = CDD_FFI_KIND_OPAQUE_PTR;
    u_fields[4].name = cdd_test_mut_str("u_fp");
    u_fields[4].type.kind = CDD_FFI_KIND_FUNCTION_PTR;
    u_fields[5].name = cdd_test_mut_str("u_opq");
    u_fields[5].type.kind = CDD_FFI_KIND_STRUCT_REF;
    u_fields[5].type.ref_name = cdd_test_mut_str("OpaqueRec");
    u_fields[6].name = cdd_test_mut_str("u_dup");
    u_fields[6].type.kind = CDD_FFI_KIND_STRUCT_REF;
    u_fields[6].type.ref_name = cdd_test_mut_str("MyDupType");
    nodes[34].kind = CDD_FFI_NODE_UNION;
    nodes[34].name = cdd_test_mut_str("RichUnion");
    nodes[34].fields = u_fields;
    nodes[34].fields_count = 7;

    /* 35..40: Typedefs */
    nodes[35].kind = CDD_FFI_NODE_TYPEDEF;
    nodes[35].name = cdd_test_mut_str("UnionAlias");
    nodes[35].return_or_base_type.kind = CDD_FFI_KIND_STRUCT_REF;
    nodes[35].return_or_base_type.ref_name = cdd_test_mut_str("RichUnion");

    nodes[36].kind = CDD_FFI_NODE_TYPEDEF;
    nodes[36].name = cdd_test_mut_str("IntAlias");
    nodes[36].return_or_base_type.kind = CDD_FFI_KIND_INT32;

    nodes[37].kind = CDD_FFI_NODE_TYPEDEF;
    nodes[37].name = cdd_test_mut_str("VoidZeroCb");
    nodes[37].return_or_base_type.kind = CDD_FFI_KIND_FUNCTION_PTR;
    nodes[37].return_or_base_type.fn_return_type = &cb0_ret;

    cb_voidp_ret.kind = CDD_FFI_KIND_VOID;
    cb_voidp_ret.pointer_depth = 1;
    cb2_ret.kind = CDD_FFI_KIND_INT32;
    cb2_params[0].name = cdd_test_mut_str("p_a");
    cb2_params[0].type.kind = CDD_FFI_KIND_STRUCT_REF;
    cb2_params[0].type.ref_name = cdd_test_mut_str("my_sub_t");
    cb2_params[1].name = cdd_test_mut_str("p_b");
    cb2_params[1].type.kind = CDD_FFI_KIND_STRUCT_REF;
    cb2_params[1].type.ref_name = cdd_test_mut_str("sub_t");
    nodes[38].kind = CDD_FFI_NODE_TYPEDEF;
    nodes[38].name = cdd_test_mut_str("TwoArgVoidPtrCb");
    nodes[38].return_or_base_type.kind = CDD_FFI_KIND_FUNCTION_PTR;
    nodes[38].return_or_base_type.fn_return_type = &cb_voidp_ret;
    nodes[38].return_or_base_type.fn_params = cb2_params;
    nodes[38].return_or_base_type.fn_params_count = 2;

    nodes[39].kind = CDD_FFI_NODE_TYPEDEF;
    nodes[39].name = cdd_test_mut_str("VarFnTd");
    nodes[39].return_or_base_type.kind = CDD_FFI_KIND_FUNCTION_PTR;
    nodes[39].return_or_base_type.fn_return_type = &cb2_ret;
    nodes[39].return_or_base_type.fn_is_variadic = 1;

    nodes[40].kind = CDD_FFI_NODE_TYPEDEF;
    nodes[40].name = cdd_test_mut_str("BareFnTd");
    nodes[40].return_or_base_type.kind = CDD_FFI_KIND_FUNCTION_PTR;

    /* 41: Zero-arg non-variadic function returning cdd_ffi_error_t by value */
    nodes[41].kind = CDD_FFI_NODE_FUNCTION;
    nodes[41].name = cdd_test_mut_str("zero_arg_err_fn");
    nodes[41].return_or_base_type.kind = CDD_FFI_KIND_STRUCT_REF;
    nodes[41].return_or_base_type.ref_name =
        cdd_test_mut_str("cdd_ffi_error_t");

    /* 42: Function exercising ALL [out] pointee kinds & unmarshaled [out] edge
     * cases */
    fn_outs_fields[0].name = cdd_test_mut_str("o_voidp");
    fn_outs_fields[0].type.kind = CDD_FFI_KIND_VOID;
    fn_outs_fields[0].type.pointer_depth = 1;
    fn_outs_fields[0].intent = CDD_FFI_INTENT_OUT;
    fn_outs_fields[1].name = cdd_test_mut_str("o_noref");
    fn_outs_fields[1].type.kind = CDD_FFI_KIND_STRUCT_REF;
    fn_outs_fields[1].type.pointer_depth = 1;
    fn_outs_fields[1].intent = CDD_FFI_INTENT_OUT;
    fn_outs_fields[2].name = cdd_test_mut_str("o_miss");
    fn_outs_fields[2].type.kind = CDD_FFI_KIND_STRUCT_REF;
    fn_outs_fields[2].type.ref_name = cdd_test_mut_str("MissingStruct");
    fn_outs_fields[2].type.pointer_depth = 1;
    fn_outs_fields[2].intent = CDD_FFI_INTENT_OUT;
    fn_outs_fields[3].name = cdd_test_mut_str("o_empu");
    fn_outs_fields[3].type.kind = CDD_FFI_KIND_STRUCT_REF;
    fn_outs_fields[3].type.ref_name = cdd_test_mut_str("EmptyUnion");
    fn_outs_fields[3].type.pointer_depth = 1;
    fn_outs_fields[3].intent = CDD_FFI_INTENT_OUT;
    fn_outs_fields[4].name = cdd_test_mut_str("o_pp");
    fn_outs_fields[4].type.kind = CDD_FFI_KIND_VOID;
    fn_outs_fields[4].type.pointer_depth = 2;
    fn_outs_fields[4].intent = CDD_FFI_INTENT_OUT;
    fn_outs_fields[5].name = cdd_test_mut_str("o_opqp");
    fn_outs_fields[5].type.kind = CDD_FFI_KIND_OPAQUE_PTR;
    fn_outs_fields[5].type.pointer_depth = 1;
    fn_outs_fields[5].intent = CDD_FFI_INTENT_OUT;
    fn_outs_fields[6].name = cdd_test_mut_str("o_bool");
    fn_outs_fields[6].type.kind = CDD_FFI_KIND_BOOL;
    fn_outs_fields[6].type.pointer_depth = 1;
    fn_outs_fields[6].intent = CDD_FFI_INTENT_OUT;
    fn_outs_fields[7].name = cdd_test_mut_str("o_i8");
    fn_outs_fields[7].type.kind = CDD_FFI_KIND_INT8;
    fn_outs_fields[7].type.pointer_depth = 1;
    fn_outs_fields[7].intent = CDD_FFI_INTENT_OUT;
    fn_outs_fields[8].name = cdd_test_mut_str("o_u8");
    fn_outs_fields[8].type.kind = CDD_FFI_KIND_UINT8;
    fn_outs_fields[8].type.pointer_depth = 1;
    fn_outs_fields[8].intent = CDD_FFI_INTENT_OUT;
    fn_outs_fields[9].name = cdd_test_mut_str("o_i16");
    fn_outs_fields[9].type.kind = CDD_FFI_KIND_INT16;
    fn_outs_fields[9].type.pointer_depth = 1;
    fn_outs_fields[9].intent = CDD_FFI_INTENT_OUT;
    fn_outs_fields[10].name = cdd_test_mut_str("o_u16");
    fn_outs_fields[10].type.kind = CDD_FFI_KIND_UINT16;
    fn_outs_fields[10].type.pointer_depth = 1;
    fn_outs_fields[10].intent = CDD_FFI_INTENT_OUT;
    fn_outs_fields[11].name = cdd_test_mut_str("o_u32");
    fn_outs_fields[11].type.kind = CDD_FFI_KIND_UINT32;
    fn_outs_fields[11].type.pointer_depth = 1;
    fn_outs_fields[11].intent = CDD_FFI_INTENT_OUT;
    fn_outs_fields[12].name = cdd_test_mut_str("o_i64");
    fn_outs_fields[12].type.kind = CDD_FFI_KIND_INT64;
    fn_outs_fields[12].type.pointer_depth = 1;
    fn_outs_fields[12].intent = CDD_FFI_INTENT_OUT;
    fn_outs_fields[13].name = cdd_test_mut_str("o_u64");
    fn_outs_fields[13].type.kind = CDD_FFI_KIND_UINT64;
    fn_outs_fields[13].type.pointer_depth = 1;
    fn_outs_fields[13].intent = CDD_FFI_INTENT_OUT;
    fn_outs_fields[14].name = cdd_test_mut_str("o_uni");
    fn_outs_fields[14].type.kind = CDD_FFI_KIND_STRUCT_REF;
    fn_outs_fields[14].type.ref_name = cdd_test_mut_str("RichUnion");
    fn_outs_fields[14].type.pointer_depth = 1;
    fn_outs_fields[14].intent = CDD_FFI_INTENT_OUT;
    fn_outs_fields[15].name = cdd_test_mut_str("o_st_norm");
    fn_outs_fields[15].type.kind = CDD_FFI_KIND_STRUCT_REF;
    fn_outs_fields[15].type.ref_name = cdd_test_mut_str("sub_t");
    fn_outs_fields[15].type.pointer_depth = 1;
    fn_outs_fields[15].intent = CDD_FFI_INTENT_OUT;
    fn_outs_fields[16].name = cdd_test_mut_str("o_enm");
    fn_outs_fields[16].type.kind = CDD_FFI_KIND_ENUM_REF;
    fn_outs_fields[16].type.ref_name = cdd_test_mut_str("EdgeEnum");
    fn_outs_fields[16].type.pointer_depth = 1;
    fn_outs_fields[16].intent = CDD_FFI_INTENT_OUT;
    fn_outs_fields[17].name = cdd_test_mut_str("o_enm_miss");
    fn_outs_fields[17].type.kind = CDD_FFI_KIND_ENUM_REF;
    fn_outs_fields[17].type.ref_name = cdd_test_mut_str("MissingEnum");
    fn_outs_fields[17].type.pointer_depth = 1;
    fn_outs_fields[17].intent = CDD_FFI_INTENT_OUT;
    fn_outs_fields[18].name = cdd_test_mut_str("o_enm_null");
    fn_outs_fields[18].type.kind = CDD_FFI_KIND_ENUM_REF;
    fn_outs_fields[18].type.ref_name = NULL;
    fn_outs_fields[18].type.pointer_depth = 1;
    fn_outs_fields[18].intent = CDD_FFI_INTENT_OUT;
    fn_outs_fields[19].name = cdd_test_mut_str("o_td");
    fn_outs_fields[19].type.kind = CDD_FFI_KIND_TYPEDEF_REF;
    fn_outs_fields[19].type.ref_name = cdd_test_mut_str("IntAlias");
    fn_outs_fields[19].type.pointer_depth = 1;
    fn_outs_fields[19].intent = CDD_FFI_INTENT_OUT;
    fn_outs_fields[20].name = cdd_test_mut_str("in_err");
    fn_outs_fields[20].type.kind = CDD_FFI_KIND_STRUCT_REF;
    fn_outs_fields[20].type.ref_name = cdd_test_mut_str("cdd_ffi_error_t");
    fn_outs_fields[20].type.pointer_depth = 1;
    fn_outs_fields[20].intent = CDD_FFI_INTENT_IN;
    fn_outs_fields[21].name = cdd_test_mut_str("err1");
    fn_outs_fields[21].type.kind = CDD_FFI_KIND_STRUCT_REF;
    fn_outs_fields[21].type.ref_name = cdd_test_mut_str("cdd_ffi_error_t");
    fn_outs_fields[21].type.pointer_depth = 1;
    fn_outs_fields[21].intent = CDD_FFI_INTENT_OUT;
    fn_outs_fields[22].name = cdd_test_mut_str("err_none");
    fn_outs_fields[22].type.kind = CDD_FFI_KIND_STRUCT_REF;
    fn_outs_fields[22].type.ref_name = cdd_test_mut_str("cdd_ffi_error_t");
    fn_outs_fields[22].type.pointer_depth = 1;
    fn_outs_fields[22].intent = CDD_FFI_INTENT_UNKNOWN;
    fn_outs_fields[23].name = cdd_test_mut_str("dup_byval");
    fn_outs_fields[23].type.kind = CDD_FFI_KIND_STRUCT_REF;
    fn_outs_fields[23].type.ref_name = cdd_test_mut_str("MyDupType");
    nodes[42].kind = CDD_FFI_NODE_FUNCTION;
    nodes[42].name = cdd_test_mut_str("mega_out_fn");
    nodes[42].return_or_base_type.kind = CDD_FFI_KIND_INT32;
    nodes[42].fields = fn_outs_fields;
    nodes[42].fields_count = 24;

    /* 43: Variadic wrapped function returning void* with err**, [inout] void*,
     * [inout] int32*, [out] int32*, and const int32 */
    fn_wrap_fields[0].name = cdd_test_mut_str("err_pp");
    fn_wrap_fields[0].type.kind = CDD_FFI_KIND_STRUCT_REF;
    fn_wrap_fields[0].type.ref_name = cdd_test_mut_str("cdd_ffi_error_t");
    fn_wrap_fields[0].type.pointer_depth = 2;
    fn_wrap_fields[1].name = cdd_test_mut_str("io_voidp");
    fn_wrap_fields[1].type.kind = CDD_FFI_KIND_VOID;
    fn_wrap_fields[1].type.pointer_depth = 1;
    fn_wrap_fields[1].intent = CDD_FFI_INTENT_INOUT;
    fn_wrap_fields[2].name = cdd_test_mut_str("io_i32");
    fn_wrap_fields[2].type.kind = CDD_FFI_KIND_INT32;
    fn_wrap_fields[2].type.pointer_depth = 1;
    fn_wrap_fields[2].intent = CDD_FFI_INTENT_INOUT;
    fn_wrap_fields[3].name = cdd_test_mut_str("out_v");
    fn_wrap_fields[3].type.kind = CDD_FFI_KIND_INT32;
    fn_wrap_fields[3].type.pointer_depth = 1;
    fn_wrap_fields[3].intent = CDD_FFI_INTENT_OUT;
    fn_wrap_fields[4].name = cdd_test_mut_str("in_v");
    fn_wrap_fields[4].type.kind = CDD_FFI_KIND_INT32;
    fn_wrap_fields[4].type.is_const = 1;
    nodes[43].kind = CDD_FFI_NODE_FUNCTION;
    nodes[43].name = cdd_test_mut_str("var_wrapped_fn");
    nodes[43].return_or_base_type.kind = CDD_FFI_KIND_VOID;
    nodes[43].return_or_base_type.pointer_depth = 1;
    nodes[43].fields = fn_wrap_fields;
    nodes[43].fields_count = 5;
    nodes[43].is_variadic = 1;

    /* 44: Fortran/COBOL array/pointer/template/substring-import edge function
     */
    fn_farr_fields[0].name = cdd_test_mut_str("arr_out");
    fn_farr_fields[0].type.kind = CDD_FFI_KIND_INT32;
    fn_farr_fields[0].type.array_size = 4;
    fn_farr_fields[0].size_bytes = 16;
    fn_farr_fields[0].intent = CDD_FFI_INTENT_OUT;
    fn_farr_fields[1].name = cdd_test_mut_str("arr_io");
    fn_farr_fields[1].type.kind = CDD_FFI_KIND_INT32;
    fn_farr_fields[1].type.array_size = 4;
    fn_farr_fields[1].intent = CDD_FFI_INTENT_INOUT;
    fn_farr_fields[2].name = cdd_test_mut_str("arr_in");
    fn_farr_fields[2].type.kind = CDD_FFI_KIND_INT8;
    fn_farr_fields[2].type.array_size = 8;
    fn_farr_fields[2].intent = CDD_FFI_INTENT_IN;
    fn_farr_fields[3].name = cdd_test_mut_str("arr_const_no_intent");
    fn_farr_fields[3].type.kind = CDD_FFI_KIND_INT32;
    fn_farr_fields[3].type.array_size = 2;
    fn_farr_fields[3].type.is_const = 1;
    fn_farr_fields[4].name = cdd_test_mut_str("enum_as_st_param");
    fn_farr_fields[4].type.kind = CDD_FFI_KIND_STRUCT_REF;
    fn_farr_fields[4].type.ref_name = cdd_test_mut_str("EdgeEnum");
    fn_farr_fields[5].name = cdd_test_mut_str("arr_ptrs");
    fn_farr_fields[5].type.kind = CDD_FFI_KIND_INT32;
    fn_farr_fields[5].type.pointer_depth = 1;
    fn_farr_fields[5].type.array_size = 3;
    fn_farr_fields[5].intent = CDD_FFI_INTENT_OUT;
    fn_farr_fields[6].name = cdd_test_mut_str("cu8_str");
    fn_farr_fields[6].type.kind = CDD_FFI_KIND_UINT8;
    fn_farr_fields[6].type.pointer_depth = 1;
    fn_farr_fields[6].type.is_const = 1;
    fn_farr_fields[7].name = cdd_test_mut_str("in_nc_cstr");
    fn_farr_fields[7].type.kind = CDD_FFI_KIND_INT8;
    fn_farr_fields[7].type.pointer_depth = 1;
    fn_farr_fields[7].intent = CDD_FFI_INTENT_IN;
    fn_farr_fields[8].name = cdd_test_mut_str("cstr_out");
    fn_farr_fields[8].type.kind = CDD_FFI_KIND_INT8;
    fn_farr_fields[8].type.pointer_depth = 1;
    fn_farr_fields[8].type.is_const = 1;
    fn_farr_fields[8].intent = CDD_FFI_INTENT_OUT;
    fn_farr_fields[9].name = cdd_test_mut_str("cstr_io");
    fn_farr_fields[9].type.kind = CDD_FFI_KIND_INT8;
    fn_farr_fields[9].type.pointer_depth = 1;
    fn_farr_fields[9].type.is_const = 1;
    fn_farr_fields[9].intent = CDD_FFI_INTENT_INOUT;
    fn_farr_fields[10].name = cdd_test_mut_str("fp_ptr");
    fn_farr_fields[10].type.kind = CDD_FFI_KIND_FUNCTION_PTR;
    fn_farr_fields[10].type.pointer_depth = 1;
    fn_farr_fields[11].name = cdd_test_mut_str("in_nc_i32");
    fn_farr_fields[11].type.kind = CDD_FFI_KIND_INT32;
    fn_farr_fields[11].type.pointer_depth = 1;
    fn_farr_fields[11].intent = CDD_FFI_INTENT_IN;
    fn_farr_fields[12].name = cdd_test_mut_str("in_mysub");
    fn_farr_fields[12].type.kind = CDD_FFI_KIND_STRUCT_REF;
    fn_farr_fields[12].type.ref_name = cdd_test_mut_str("my_sub_t");
    fn_farr_fields[13].name = cdd_test_mut_str("in_sub");
    fn_farr_fields[13].type.kind = CDD_FFI_KIND_TEMPLATE_STRUCT_REF;
    fn_farr_fields[13].type.ref_name = cdd_test_mut_str("sub_t");
    fn_farr_fields[13].type.pointer_depth = 1;
    fn_farr_fields[13].intent = CDD_FFI_INTENT_IN;
    fn_farr_fields[14].name = cdd_test_mut_str("tmpl_val");
    fn_farr_fields[14].type.kind = CDD_FFI_KIND_TEMPLATE_STRUCT_REF;
    fn_farr_fields[14].type.ref_name = cdd_test_mut_str("sub_t");
    fn_farr_fields[15].name = cdd_test_mut_str("opq_p");
    fn_farr_fields[15].type.kind = CDD_FFI_KIND_STRUCT_REF;
    fn_farr_fields[15].type.ref_name = cdd_test_mut_str("OpaqueRec");
    fn_farr_fields[15].type.pointer_depth = 1;
    fn_farr_fields[15].intent = CDD_FFI_INTENT_OUT;
    fn_farr_fields[16].name = cdd_test_mut_str("fp_byval");
    fn_farr_fields[16].type.kind = CDD_FFI_KIND_FUNCTION_PTR;
    nodes[44].kind = CDD_FFI_NODE_FUNCTION;
    nodes[44].name = cdd_test_mut_str("arr_and_ptr_fn");
    nodes[44].return_or_base_type.kind = CDD_FFI_KIND_OPAQUE_PTR;
    nodes[44].fields = fn_farr_fields;
    nodes[44].fields_count = 17;

    /* 45..51: cstr-arg & shim-routed functions */
    fn_cstr_ptr_fields[0].name = cdd_test_mut_str("s");
    fn_cstr_ptr_fields[0].type.kind = CDD_FFI_KIND_INT8;
    fn_cstr_ptr_fields[0].type.pointer_depth = 1;
    fn_cstr_ptr_fields[0].type.is_const = 1;
    nodes[45].kind = CDD_FFI_NODE_FUNCTION;
    nodes[45].name = cdd_test_mut_str("cstr_ret_ptr");
    nodes[45].return_or_base_type.kind = CDD_FFI_KIND_VOID;
    nodes[45].return_or_base_type.pointer_depth = 1;
    nodes[45].fields = fn_cstr_ptr_fields;
    nodes[45].fields_count = 1;

    nodes[46].kind = CDD_FFI_NODE_FUNCTION;
    nodes[46].name = cdd_test_mut_str("cstr_ret_fp");
    nodes[46].return_or_base_type.kind = CDD_FFI_KIND_FUNCTION_PTR;
    nodes[46].fields = fn_cstr_ptr_fields;
    nodes[46].fields_count = 1;

    nodes[47].kind = CDD_FFI_NODE_FUNCTION;
    nodes[47].name = cdd_test_mut_str("cstr_ret_opq");
    nodes[47].return_or_base_type.kind = CDD_FFI_KIND_STRUCT_REF;
    nodes[47].return_or_base_type.ref_name = cdd_test_mut_str("OpaqueRec");
    nodes[47].fields = fn_cstr_ptr_fields;
    nodes[47].fields_count = 1;

    fn_u_byval_fields[0].name = cdd_test_mut_str("u_in");
    fn_u_byval_fields[0].type.kind = CDD_FFI_KIND_STRUCT_REF;
    fn_u_byval_fields[0].type.ref_name = cdd_test_mut_str("RichUnion");
    fn_u_byval_fields[1].name = cdd_test_mut_str("z_in");
    fn_u_byval_fields[1].type.kind = CDD_FFI_KIND_STRUCT_REF;
    fn_u_byval_fields[1].type.ref_name = cdd_test_mut_str("ZeroSz");
    nodes[48].kind = CDD_FFI_NODE_FUNCTION;
    nodes[48].name = cdd_test_mut_str("union_roundtrip");
    nodes[48].return_or_base_type.kind = CDD_FFI_KIND_STRUCT_REF;
    nodes[48].return_or_base_type.ref_name = cdd_test_mut_str("RichUnion");
    nodes[48].fields = fn_u_byval_fields;
    nodes[48].fields_count = 2;

    /* Shim-routed functions returning const int, FUNCTION_PTR, and const void*
     */
    nodes[49].kind = CDD_FFI_NODE_FUNCTION;
    nodes[49].name = cdd_test_mut_str("shim_ret_const_int");
    nodes[49].return_or_base_type.kind = CDD_FFI_KIND_INT32;
    nodes[49].return_or_base_type.is_const = 1;
    nodes[49].fields = fn_u_byval_fields;
    nodes[49].fields_count = 1;

    nodes[50].kind = CDD_FFI_NODE_FUNCTION;
    nodes[50].name = cdd_test_mut_str("shim_ret_fp");
    nodes[50].return_or_base_type.kind = CDD_FFI_KIND_FUNCTION_PTR;
    nodes[50].fields = fn_u_byval_fields;
    nodes[50].fields_count = 1;

    /* 51: Unnamed 0-param variadic function returning const void* */
    nodes[51].kind = CDD_FFI_NODE_FUNCTION;
    nodes[51].name = NULL;
    nodes[51].return_or_base_type.kind = CDD_FFI_KIND_VOID;
    nodes[51].return_or_base_type.pointer_depth = 1;
    nodes[51].return_or_base_type.is_const = 1;
    nodes[51].is_variadic = 1;

    /* 52: Empty enum (variants_count == 0) */
    nodes[52].kind = CDD_FFI_NODE_ENUM;
    nodes[52].name = cdd_test_mut_str("EmptyEnum");

    /* 53: Quoted string macro */
    nodes[53].kind = CDD_FFI_NODE_MACRO;
    nodes[53].name = cdd_test_mut_str("M_STR_QUOTED");
    nodes[53].inferred_type = CDD_FFI_MACRO_TYPE_STRING;
    nodes[53].evaluated_value = cdd_test_mut_str("\"quoted_str\"");

    /* 54: Shim-routed function returning void */
    nodes[54].kind = CDD_FFI_NODE_FUNCTION;
    nodes[54].name = cdd_test_mut_str("shim_ret_void");
    nodes[54].return_or_base_type.kind = CDD_FFI_KIND_VOID;
    nodes[54].fields = fn_u_byval_fields;
    nodes[54].fields_count = 1;

    /* 55: C-string-arg function returning void */
    nodes[55].kind = CDD_FFI_NODE_FUNCTION;
    nodes[55].name = cdd_test_mut_str("cstr_arg_void");
    nodes[55].return_or_base_type.kind = CDD_FFI_KIND_VOID;
    nodes[55].fields = fn_cstr_ptr_fields;
    nodes[55].fields_count = 1;

    ir.nodes = nodes;
    ir.nodes_count = 56;
    ir.module_name = cdd_test_mut_str("My--Edge--Mod--");

    /* Run with library_name == NULL while ir.module_name != NULL! */
    cfg.output_dir = "test_ofc_out/b3";
    cfg.library_name = NULL;
    cfg.module_name = "---";
    cfg.generate_tests = 1;

    /* Test relaxed ABI layout fallback failures on Fortran & COBOL */
    for (k = 1; k <= 80; k++) {
      g_cdd_step_fail = k;
      rc = cdd_ffi_emit_fortran(&ir, &cfg);
      g_cdd_step_fail = 0;
      if (rc == CDD_C_SUCCESS)
        break;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
    }
    for (k = 1; k <= 80; k++) {
      g_cdd_step_fail = k;
      rc = cdd_ffi_emit_cobol(&ir, &cfg);
      g_cdd_step_fail = 0;
      if (rc == CDD_C_SUCCESS)
        break;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
    }

    g_cdd_alloc_fail = 2;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_ffi_emit_fortran(&ir, &cfg));
    g_cdd_alloc_fail = 2;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_ffi_emit_cobol(&ir, &cfg));
    g_cdd_alloc_fail = 0;

    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_emit_ocaml(&ir, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_emit_fortran(&ir, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_emit_cobol(&ir, &cfg));

    /* Also run with library_name == ""! */
    cfg.library_name = "";
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_emit_cobol(&ir, &cfg));

    /* Full I/O fault sweeps while nodes[51].is_variadic == 1! */
    for (k = 1; k <= 4000; k++) {
      g_fail_io_after = k;
      rc = cdd_ffi_emit_ocaml(&ir, &cfg);
      g_fail_io_after = -1;
      if (rc == CDD_C_SUCCESS)
        break;
      ASSERT_EQ(CDD_C_ERROR_IO, rc);
    }
    for (k = 1; k <= 4000; k++) {
      g_fail_io_after = k;
      rc = cdd_ffi_emit_fortran(&ir, &cfg);
      g_fail_io_after = -1;
      if (rc == CDD_C_SUCCESS)
        break;
      ASSERT_EQ(CDD_C_ERROR_IO, rc);
    }
    for (k = 1; k <= 4000; k++) {
      g_fail_io_after = k;
      rc = cdd_ffi_emit_cobol(&ir, &cfg);
      g_fail_io_after = -1;
      if (rc == CDD_C_SUCCESS)
        break;
      ASSERT_EQ(CDD_C_ERROR_IO, rc);
    }
    for (k = 1; k <= 4000; k++) {
      g_ffi_emit_fail_step = k;
      rc = cdd_ffi_emit_ocaml(&ir, &cfg);
      g_ffi_emit_fail_step = 0;
      if (rc == CDD_C_SUCCESS)
        break;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
    }
    for (k = 1; k <= 4000; k++) {
      g_ffi_emit_fail_step = k;
      rc = cdd_ffi_emit_fortran(&ir, &cfg);
      g_ffi_emit_fail_step = 0;
      if (rc == CDD_C_SUCCESS)
        break;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
    }
    for (k = 1; k <= 4000; k++) {
      g_ffi_emit_fail_step = k;
      rc = cdd_ffi_emit_cobol(&ir, &cfg);
      g_ffi_emit_fail_step = 0;
      if (rc == CDD_C_SUCCESS)
        break;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
    }

    /* Also test ir.module_name == NULL, library_name == NULL, non-variadic
     * unnamed fn, and char* [4] return */
    ir.module_name = NULL;
    cfg.library_name = NULL;
    nodes[51].is_variadic = 0;
    nodes[51].return_or_base_type.kind = CDD_FFI_KIND_INT8;
    nodes[51].return_or_base_type.array_size = 4;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_emit_ocaml(&ir, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_emit_fortran(&ir, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_emit_cobol(&ir, &cfg));

    /* Zero-function IR: OOM + I/O sweeps + Fortran CONTAINS flags */
    ir.nodes_count = 2; /* Only nodes[0..1] (0 functions) */
    for (k = 1; k <= 50; k++) {
      g_cdd_alloc_fail = k;
      rc = cdd_ffi_emit_cobol(&ir, &cfg);
      g_cdd_alloc_fail = 0;
      if (rc == CDD_C_SUCCESS)
        break;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    }
    for (k = 1; k <= 50; k++) {
      g_cdd_strdup_fail = k;
      rc = cdd_ffi_emit_cobol(&ir, &cfg);
      g_cdd_strdup_fail = 0;
      if (rc == CDD_C_SUCCESS)
        break;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    }
    for (k = 1; k <= 200; k++) {
      g_fail_io_after = k;
      rc = cdd_ffi_emit_cobol(&ir, &cfg);
      g_fail_io_after = -1;
      if (rc == CDD_C_SUCCESS)
        break;
      ASSERT_EQ(CDD_C_ERROR_IO, rc);
    }
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_emit_fortran(&ir, &cfg));

    /* Only has_cstr_ret_wrapper == 1 (no union, no cstr arg) */
    {
      cdd_ffi_ir_t cstr_ir;
      cdd_ffi_ir_node_t cstr_fn;
      memset(&cstr_ir, 0, sizeof(cstr_ir));
      memset(&cstr_fn, 0, sizeof(cstr_fn));
      cstr_fn.kind = CDD_FFI_NODE_FUNCTION;
      cstr_fn.name = cdd_test_mut_str("only_cstr_ret");
      cstr_fn.return_or_base_type.kind = CDD_FFI_KIND_INT8;
      cstr_fn.return_or_base_type.pointer_depth = 1;
      cstr_ir.nodes = &cstr_fn;
      cstr_ir.nodes_count = 1;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_emit_fortran(&cstr_ir, &cfg));

      /* Only has_cstr_arg_wrapper == 1 (no union, no cstr ret) */
      cstr_fn.return_or_base_type.kind = CDD_FFI_KIND_VOID;
      cstr_fn.return_or_base_type.pointer_depth = 0;
      cstr_fn.fields = fn_cstr_ptr_fields;
      cstr_fn.fields_count = 1;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_emit_fortran(&cstr_ir, &cfg));
    }
  }

  PASS();
}

/**
 * @brief Executes the `ffi_ocaml_fortran_cobol_suite` test suite.
 */
SUITE(ffi_ocaml_fortran_cobol_suite) {
  RUN_TEST(test_ffi_abi_layout_calculator);
  RUN_TEST(test_ffi_common_io_and_comment_translation);
  RUN_TEST(test_ffi_typed_fnptr_roundtrip);
  RUN_TEST(test_ffi_ocaml_comprehensive);
  RUN_TEST(test_ffi_fortran_comprehensive);
  RUN_TEST(test_ffi_cobol_comprehensive);
  RUN_TEST(test_ffi_dsl_e2e_and_compiler_smoke);
  RUN_TEST(test_ffi_batch2_branch_and_oom_edges);
  RUN_TEST(test_ffi_batch3_100_percent_edges);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_FFI_OCAML_FORTRAN_COBOL_H */
