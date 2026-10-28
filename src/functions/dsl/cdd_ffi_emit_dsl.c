/**
 * @file cdd_ffi_emit_dsl.c
 * @brief FFI binding emitter for Dsl.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "dsl/cdd_ffi_emit_dsl.h"
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include "functions/ffi/cdd_ffi_ir_internal.h"
#include "functions/parse/fs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Emits a linked list of FFI trivia nodes to a file stream.
 *
 * @param[in] f Output file stream.
 * @param[in] trivia Head of the FFI trivia linked list.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t emit_trivia(FILE *f, cdd_ffi_trivia_t *trivia) {
  while (trivia) {
    if (trivia->text) {
      if (trivia->kind == CDD_FFI_TRIVIA_WHITESPACE) {
        fprintf(f, "%s", trivia->text);
      } else if (trivia->kind == CDD_FFI_TRIVIA_COMMENT_LINE) {
        fprintf(f, "//%s", trivia->text);
      } else if (trivia->kind == CDD_FFI_TRIVIA_COMMENT_BLOCK) {
        fprintf(f, "/*%s*/", trivia->text);
      }
    }
    trivia = trivia->next;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits an FFI type in CDD DSL syntax to a file stream.
 *
 * @param[in] f Output file stream.
 * @param[in] type FFI type descriptor to emit.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t emit_type(FILE *f, const cdd_ffi_type_t *type) {
  int i;
  for (i = 0; i < type->pointer_depth; i++) {
    if (type->is_const) {
      fprintf(f, "*const ");
    } else {
      fprintf(f, "*mut ");
    }
  }

  if (type->array_size > 0) {
    fprintf(f, "[%ld]", type->array_size);
  }

  switch (type->kind) {
  case CDD_FFI_KIND_VOID:
    fprintf(f, "void");
    break;
  case CDD_FFI_KIND_UINT8:
    fprintf(f, "u8");
    break;
  case CDD_FFI_KIND_INT16:
    fprintf(f, "i16");
    break;
  case CDD_FFI_KIND_UINT16:
    fprintf(f, "u16");
    break;
  case CDD_FFI_KIND_INT32:
    fprintf(f, "i32");
    break;
  case CDD_FFI_KIND_UINT32:
    fprintf(f, "u32");
    break;
  case CDD_FFI_KIND_INT64:
    fprintf(f, "i64");
    break;
  case CDD_FFI_KIND_FLOAT32:
    fprintf(f, "f32");
    break;
  case CDD_FFI_KIND_FLOAT64:
    fprintf(f, "f64");
    break;
  case CDD_FFI_KIND_BOOL:
    fprintf(f, "bool");
    break;
  case CDD_FFI_KIND_UINT64:
    fprintf(f, "size_t");
    break;
  case CDD_FFI_KIND_INT8:
    fprintf(f, "char");
    break;
  case CDD_FFI_KIND_STRUCT_REF:
  case CDD_FFI_KIND_ENUM_REF:
    if (type->ref_name) {
      fprintf(f, "%s", type->ref_name);
    }
    break;
  default:
    fprintf(f, "void");
    break;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits an FFI parameter or struct field in CDD DSL syntax to a file
 * stream.
 *
 * @param[in] f Output file stream.
 * @param[in] field FFI field descriptor to emit.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t emit_field(FILE *f, const cdd_ffi_field_t *field) {
  cdd_c_error_t rc;
  rc = emit_trivia(f, field->leading_trivia);

  if (field->intent != CDD_FFI_INTENT_UNKNOWN || field->array_length_ref) {
    fprintf(f, "[");
    if (field->intent == CDD_FFI_INTENT_IN)
      fprintf(f, "in");
    else if (field->intent == CDD_FFI_INTENT_OUT)
      fprintf(f, "out");
    else if (field->intent == CDD_FFI_INTENT_INOUT)
      fprintf(f, "inout");

    if (field->array_length_ref) {
      if (field->intent != CDD_FFI_INTENT_UNKNOWN)
        fprintf(f, ", ");
      fprintf(f, "len=%s", field->array_length_ref);
    }
    fprintf(f, "] ");
  }

  rc = emit_type(f, &field->type);
  if (field->name) {
    fprintf(f, " %s", field->name);
  }

  rc = emit_trivia(f, field->trailing_trivia);
  return rc;
}

/**
 * @brief Emits an FFI Intermediate Representation (`cdd_ffi_ir_t`) in CDD DSL
 * format.
 *
 * @param[in] ir FFI Intermediate Representation to emit.
 * @param[in] config Binding generation configuration parameters.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_emit_dsl(
    cdd_ffi_ir_t *ir, const struct cdd_generate_bindings_config *config) {
  FILE *f;
  size_t i, j;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!ir || !config) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

/* Just a basic dump for now, proper path construction is needed in real tool
 */
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_out_dsl.dsl", "wb") != 0) {
#else
  f = fopen("test_out_dsl.dsl", "wb");
  if (!f) {
#endif
    return CDD_C_ERROR_IO;
  }

  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];

    rc = emit_trivia(f, node->leading_trivia);

    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      fprintf(f, "func %s(", node->name);
      for (j = 0; j < node->fields_count; j++) {
        rc = emit_field(f, &node->fields[j]);
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      fprintf(f, ") ");

      if (node->raw_body) {
        fprintf(f, "{\n  body %%{%s}%%\n}\n", node->raw_body);
      } else {
        fprintf(f, ";\n");
      }
    }
  }

  fclose(f);
  return rc;
}
