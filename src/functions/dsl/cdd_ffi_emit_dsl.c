/**
 * @file cdd_ffi_emit_dsl.c
 * @brief FFI binding emitter for the CDD FFI Domain-Specific Language (DSL).
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "dsl/cdd_ffi_emit_dsl.h"
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include "../ffi/cdd_ffi_emit_common.h"
#include "functions/ffi/cdd_ffi_ir_internal.h"
#include "functions/parse/fs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Emits a linked list of FFI trivia nodes to a file stream.
 *
 * @param[in,out] f Output file stream.
 * @param[in] trivia Head of the FFI trivia linked list.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO on write failure.
 */
static NO_DISCARD cdd_c_error_t emit_trivia(FILE *f,
                                            const cdd_ffi_trivia_t *trivia) {
  cdd_c_error_t rc;
  while (trivia) {
    if (trivia->text) {
      if (trivia->kind == CDD_FFI_TRIVIA_WHITESPACE) {
        rc = cdd_ffi_write_str(f, trivia->text);
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if (trivia->kind == CDD_FFI_TRIVIA_COMMENT_LINE) {
        rc = cdd_ffi_write_fmt(f, "/%c%s", '/', trivia->text);
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if (trivia->kind == CDD_FFI_TRIVIA_COMMENT_BLOCK) {
        rc = cdd_ffi_write_fmt(f, "/%c%s%c/", '*', trivia->text, '*');
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
    trivia = trivia->next;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits an FFI parameter or struct field in CDD DSL syntax to a file
 * stream.
 *
 * @param[in,out] f Output file stream.
 * @param[in] field FFI field descriptor to emit.
 * @return CDD_C_SUCCESS on success, or error code on I/O failure.
 */
static NO_DISCARD cdd_c_error_t emit_field(FILE *f,
                                           const cdd_ffi_field_t *field);

/**
 * @brief Emits an FFI type in CDD DSL syntax to a file stream.
 *
 * @param[in,out] f Output file stream.
 * @param[in] type FFI type descriptor to emit.
 * @return CDD_C_SUCCESS on success, or error code on I/O failure.
 */
static NO_DISCARD cdd_c_error_t emit_type(FILE *f, const cdd_ffi_type_t *type) {
  int i;
  cdd_c_error_t rc;

  for (i = 0; i < type->pointer_depth; i++) {
    rc = cdd_ffi_write_str(f, type->is_const ? "*const " : "*mut ");
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  if (type->array_size > 0) {
    rc = cdd_ffi_write_fmt(f, "[%ld]", type->array_size);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  switch (type->kind) {
  case CDD_FFI_KIND_VOID:
    return cdd_ffi_write_str(f, "void");
  case CDD_FFI_KIND_UINT8:
    return cdd_ffi_write_str(f, "u8");
  case CDD_FFI_KIND_INT16:
    return cdd_ffi_write_str(f, "i16");
  case CDD_FFI_KIND_UINT16:
    return cdd_ffi_write_str(f, "u16");
  case CDD_FFI_KIND_INT32:
    return cdd_ffi_write_str(f, "i32");
  case CDD_FFI_KIND_UINT32:
    return cdd_ffi_write_str(f, "u32");
  case CDD_FFI_KIND_INT64:
    return cdd_ffi_write_str(f, "i64");
  case CDD_FFI_KIND_FLOAT32:
    return cdd_ffi_write_str(f, "f32");
  case CDD_FFI_KIND_FLOAT64:
    return cdd_ffi_write_str(f, "f64");
  case CDD_FFI_KIND_BOOL:
    return cdd_ffi_write_str(f, "bool");
  case CDD_FFI_KIND_UINT64:
    return cdd_ffi_write_str(f, "size_t");
  case CDD_FFI_KIND_INT8:
    return cdd_ffi_write_str(f, "char");
  case CDD_FFI_KIND_STRUCT_REF:
  case CDD_FFI_KIND_ENUM_REF:
    if (type->ref_name)
      return cdd_ffi_write_str(f, type->ref_name);
    return CDD_C_SUCCESS;
  case CDD_FFI_KIND_FUNCTION_PTR: {
    size_t k;
    rc = cdd_ffi_write_str(f, "func(");
    if (rc != CDD_C_SUCCESS)
      return rc;
    for (k = 0; k < type->fn_params_count; k++) {
      rc = emit_field(f, &type->fn_params[k]);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (k + 1 < type->fn_params_count || type->fn_is_variadic) {
        rc = cdd_ffi_write_str(f, ", ");
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
    if (type->fn_is_variadic) {
      rc = cdd_ffi_write_str(f, "va_list");
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    rc = cdd_ffi_write_str(f, ") ");
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (type->fn_return_type)
      return emit_type(f, type->fn_return_type);
    return cdd_ffi_write_str(f, "void");
  }
  default:
    return cdd_ffi_write_str(f, "void");
  }
}

static NO_DISCARD cdd_c_error_t emit_field(FILE *f,
                                           const cdd_ffi_field_t *field) {
  cdd_c_error_t rc;

  rc = emit_trivia(f, field->leading_trivia);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (field->intent != CDD_FFI_INTENT_UNKNOWN || field->array_length_ref) {
    rc = cdd_ffi_write_str(f, "[");
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (field->intent == CDD_FFI_INTENT_IN) {
      rc = cdd_ffi_write_str(f, "in");
    } else if (field->intent == CDD_FFI_INTENT_OUT) {
      rc = cdd_ffi_write_str(f, "out");
    } else if (field->intent == CDD_FFI_INTENT_INOUT) {
      rc = cdd_ffi_write_str(f, "inout");
    }
    if (rc != CDD_C_SUCCESS)
      return rc;

    if (field->array_length_ref) {
      if (field->intent != CDD_FFI_INTENT_UNKNOWN) {
        rc = cdd_ffi_write_str(f, ", ");
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      rc = cdd_ffi_write_fmt(f, "len=%s", field->array_length_ref);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    rc = cdd_ffi_write_str(f, "] ");
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  rc = emit_type(f, &field->type);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (field->name) {
    rc = cdd_ffi_write_fmt(f, " %s", field->name);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  return emit_trivia(f, field->trailing_trivia);
}

/*
 * @brief Emits an FFI Intermediate Representation (`cdd_ffi_ir_t`) in CDD DSL
 * format.
 *
 * @param[in,out] ir FFI Intermediate Representation to emit.
 * @param[in] config Binding generation configuration parameters.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
C_CDD_EXPORT NO_DISCARD cdd_c_error_t cdd_ffi_emit_dsl(
    cdd_ffi_ir_t *ir, const struct cdd_generate_bindings_config *config) {
  FILE *f = NULL;
  size_t i, j;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!ir || !config) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  if (fopen_s(&f, "test_out_dsl.dsl", "wb") != 0)
    f = NULL;
#else
  f = fopen("test_out_dsl.dsl", "wb");
#endif
  if (!f) {
    return CDD_C_ERROR_IO;
  }

  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];

    rc = emit_trivia(f, node->leading_trivia);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;

    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      rc = cdd_ffi_write_fmt(f, "func %s(", node->name ? node->name : "anon");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      for (j = 0; j < node->fields_count; j++) {
        rc = emit_field(f, &node->fields[j]);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        if (j + 1 < node->fields_count) {
          rc = cdd_ffi_write_str(f, ", ");
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
      }
      rc = cdd_ffi_write_str(f, ") ");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;

      if (node->raw_body) {
        rc = cdd_ffi_write_fmt(f, "{\n  body %%{%s}%%\n}\n", node->raw_body);
      } else {
        rc = cdd_ffi_write_str(f, ";\n");
      }
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
  }

cleanup:
  rc = cdd_c_keep_err(rc, cdd_ffi_close_output_file(&f));
  return rc;
}
