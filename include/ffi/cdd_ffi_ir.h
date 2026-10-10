#ifndef CDD_FFI_IR_H
#define CDD_FFI_IR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <stddef.h>
/* clang-format on */

/**
 * @file cdd_ffi_ir.h
 * @brief Universal FFI Intermediate Representation (IR) structures.
 *
 * This file defines the normalized cross-language FFI model used to map
 * C structures to the 40 language emitters.
 */

/**
 * @struct cdd_ffi_error_t
 * @brief Standard FFI error structure for cross-language exception handling.
 */
typedef struct cdd_ffi_error_t {
  /** @brief Error code (0 for success, non-zero for error) */
  int code;
  /** @brief Null-terminated error message string */
  char *message;
} cdd_ffi_error_t;

/**
 * @enum cdd_ffi_primitive_kind_t
 * @brief Represents primitive types in a cross-language way.
 */
typedef enum cdd_ffi_primitive_kind_t {
  CDD_FFI_KIND_VOID,
  CDD_FFI_KIND_BOOL,
  CDD_FFI_KIND_INT8,
  CDD_FFI_KIND_UINT8,
  CDD_FFI_KIND_INT16,
  CDD_FFI_KIND_UINT16,
  CDD_FFI_KIND_INT32,
  CDD_FFI_KIND_UINT32,
  CDD_FFI_KIND_INT64,
  CDD_FFI_KIND_UINT64,
  CDD_FFI_KIND_FLOAT32,
  CDD_FFI_KIND_FLOAT64,
  CDD_FFI_KIND_STRUCT_REF,
  CDD_FFI_KIND_TEMPLATE_STRUCT_REF,
  CDD_FFI_KIND_ENUM_REF,
  CDD_FFI_KIND_TYPEDEF_REF,
  CDD_FFI_KIND_OPAQUE_PTR,
  CDD_FFI_KIND_FUNCTION_PTR,
  CDD_FFI_KIND_STD_STRING,
  CDD_FFI_KIND_STD_VECTOR,
  CDD_FFI_KIND_STD_SHARED_PTR,
  CDD_FFI_KIND_STD_UNIQUE_PTR
} cdd_ffi_primitive_kind_t;

/**
 * @enum cdd_ffi_trivia_kind_t
 * @brief Represents the specific kind of lexical trivia.
 */
typedef enum cdd_ffi_trivia_kind_t {
  CDD_FFI_TRIVIA_WHITESPACE,
  CDD_FFI_TRIVIA_COMMENT_LINE,
  CDD_FFI_TRIVIA_COMMENT_BLOCK,
  CDD_FFI_TRIVIA_CONTINUATION
} cdd_ffi_trivia_kind_t;

/**
 * @struct cdd_ffi_trivia_t
 * @brief Represents lexical trivia (whitespace, comments, continuations) to
 * allow exact formatting roundtrips.
 */
typedef struct cdd_ffi_trivia_t {
  /** @brief The kind of trivia */
  cdd_ffi_trivia_kind_t kind;
  /** @brief Raw exact text of the trivia */
  char *text;
  /** @brief Pointer to the next piece of trivia in the chain */
  struct cdd_ffi_trivia_t *next;
} cdd_ffi_trivia_t;

/**
 * @struct cdd_ffi_type_t
 * @brief Normalized representation of any C type (including pointers).
 */

/**
 * @enum cdd_ffi_number_prefix_t
 * @brief Represents the literal prefix for numbers.
 */
typedef enum cdd_ffi_number_prefix_t {
  CDD_FFI_NUM_PREFIX_NONE,
  CDD_FFI_NUM_PREFIX_HEX,
  CDD_FFI_NUM_PREFIX_OCTAL,
  CDD_FFI_NUM_PREFIX_BINARY
} cdd_ffi_number_prefix_t;

/**
 * @struct cdd_ffi_number_t
 * @brief Enhances IR Number representations to capture exact text, prefixes,
 * and C23 separators.
 */
typedef struct cdd_ffi_number_t {
  /** @brief Exact text spelling, e.g., "1'000" */
  char *raw_spelling;
  /** @brief Number prefix */
  cdd_ffi_number_prefix_t prefix;
  /** @brief Has C23 digit separators (e.g. 1'000) */
  int has_separators;
} cdd_ffi_number_t;

typedef struct cdd_ffi_type_t {
  /** @brief The base kind of the type */
  cdd_ffi_primitive_kind_t kind;
  /** @brief If true, it's a const type */
  int is_const;
  /** @brief Pointer depth (e.g., 2 for char**) */
  int pointer_depth;
  /** @brief Name of the struct/enum/typedef if applicable */
  char *ref_name;
  /** @brief If it's an array, the fixed size (-1 if dynamic/unknown) */
  long array_size;
  /** @brief Template parameters (if CDD_FFI_KIND_TEMPLATE_STRUCT_REF) */
  struct cdd_ffi_type_t *template_args;
  /** @brief Number of template parameters */
  size_t template_args_count;
  cdd_ffi_trivia_t *leading_trivia;
  cdd_ffi_trivia_t *trailing_trivia;
  char *raw_spelling;
} cdd_ffi_type_t;

/**
 * @enum cdd_ffi_param_intent_t
 * @brief Represents the data flow intent of a parameter (In, Out, InOut).
 */
typedef enum cdd_ffi_param_intent_t {
  CDD_FFI_INTENT_UNKNOWN,
  CDD_FFI_INTENT_IN,
  CDD_FFI_INTENT_OUT,
  CDD_FFI_INTENT_INOUT
} cdd_ffi_param_intent_t;

/**
 * @struct cdd_ffi_macro_expansion_t
 * @brief Models a macro expansion exactly as it appears in the source.
 */
typedef struct cdd_ffi_macro_expansion_t {
  /** @brief The name of the macro invoked */
  char *macro_name;
  /** @brief The exact string of the expansion arguments (if any), e.g. "(a, b)"
   */
  char *raw_args;
  /** @brief The C code that it expands to, captured for analysis if needed */
  char *expanded_text;
} cdd_ffi_macro_expansion_t;

/**
 * @struct cdd_ffi_field_t
 * @brief A field inside a struct, or an argument inside a function.
 */
typedef struct cdd_ffi_field_t {
  /** @brief Field name */
  char *name;
  /** @brief Field type */
  cdd_ffi_type_t type;
  /** @brief Optional docstring */
  char *doc;
  /** @brief Parameter intent (for function arguments) */
  cdd_ffi_param_intent_t intent;
  /** @brief Reference to another field defining the array length (if
   * applicable) */
  char *array_length_ref;
  cdd_ffi_trivia_t *leading_trivia;
  cdd_ffi_trivia_t *trailing_trivia;
} cdd_ffi_field_t;

/**
 * @enum cdd_ffi_node_kind_t
 * @brief The type of an exported FFI IR node.
 */
typedef enum cdd_ffi_node_kind_t {
  CDD_FFI_NODE_STRUCT,
  CDD_FFI_NODE_UNION,
  CDD_FFI_NODE_ENUM,
  CDD_FFI_NODE_FUNCTION,
  CDD_FFI_NODE_TYPEDEF,
  CDD_FFI_NODE_MACRO
} cdd_ffi_node_kind_t;

/**
 * @struct cdd_ffi_enum_variant_t
 * @brief A specific value inside an enum.
 */
typedef struct cdd_ffi_enum_variant_t {
  /** @brief The variant name (e.g., MY_ENUM_A) */
  char *name;
  /** @brief The raw string value or integer literal */
  char *value;
  /** @brief Optional docstring */
  char *doc;
  cdd_ffi_trivia_t *leading_trivia;
  cdd_ffi_trivia_t *trailing_trivia;
  struct cdd_ffi_number_t *number_details;
  char *raw_spelling;
} cdd_ffi_enum_variant_t;

/**
 * @struct cdd_ffi_base_class_t
 * @brief Represents a base class inherited by a C++ struct/class.
 */
typedef struct cdd_ffi_base_class_t {
  /** @brief The base class name */
  char *name;
  /** @brief True if inherited virtually */
  int is_virtual;
  /** @brief The access modifier (CDD_TOKEN_KEYWORD_PUBLIC etc. loosely mapped,
   * or just string like "public") */
  char *access;
} cdd_ffi_base_class_t;

/**
 * @struct cdd_ffi_virtual_method_t
 * @brief A virtual method available for Director overriding.
 */
typedef struct cdd_ffi_virtual_method_t {
  /** @brief Method name */
  char *name;
  /** @brief Method return type */
  cdd_ffi_type_t return_type;
  /** @brief Arguments */
  cdd_ffi_field_t *args;
  /** @brief Number of arguments */
  size_t args_count;
  /** @brief True if pure virtual (= 0) */
  int is_pure_virtual;
} cdd_ffi_virtual_method_t;

/**
 * @enum cdd_ffi_macro_type_t
 * @brief The inferred type of an evaluated macro.
 */
typedef enum cdd_ffi_macro_type_t {
  CDD_FFI_MACRO_TYPE_UNKNOWN,
  CDD_FFI_MACRO_TYPE_INT,
  CDD_FFI_MACRO_TYPE_FLOAT,
  CDD_FFI_MACRO_TYPE_STRING
} cdd_ffi_macro_type_t;

/**
 * @struct cdd_ffi_ir_node_t
 * @brief A single exported node (Struct, Function, etc.) in the FFI IR.
 */
typedef struct cdd_ffi_ir_node_t {
  /** @brief What type of export this is */
  cdd_ffi_node_kind_t kind;
  /** @brief The name of the export */
  char *name;
  /** @brief Optional docstring */
  char *doc;

  /** @brief Fields (if Struct/Union) or Arguments (if Function) */
  cdd_ffi_field_t *fields;
  /** @brief Number of fields/arguments */
  size_t fields_count;

  /** @brief Base classes (if this is a C++ class/struct with inheritance) */
  cdd_ffi_base_class_t *base_classes;
  /** @brief Number of base classes */
  size_t base_classes_count;

  /** @brief Virtual methods (if this is a polymorphic C++ class) */
  cdd_ffi_virtual_method_t *virtual_methods;
  /** @brief Number of virtual methods */
  size_t virtual_methods_count;

  /** @brief Enum variants (if Enum) */
  cdd_ffi_enum_variant_t *variants;
  /** @brief Number of enum variants */
  size_t variants_count;

  /** @brief Return type (if Function) or underlying type (if Typedef) */
  cdd_ffi_type_t return_or_base_type;

  /** @brief Evaluated value (if Macro) */
  char *evaluated_value;
  /** @brief Inferred type of the macro evaluation (if Macro) */
  cdd_ffi_macro_type_t inferred_type;

  /** @brief True if the function requires GIL release (e.g., @ffi_release_gil
   * or @blocking) */
  int requires_gil_release;

  /** @brief True if the function accepts a variable number of arguments (...)
   */
  int is_variadic;
  cdd_ffi_trivia_t *leading_trivia;
  cdd_ffi_trivia_t *trailing_trivia;

  /** @brief Macro expansions contained within this node */
  struct cdd_ffi_macro_expansion_t *macro_expansions;
  /** @brief Number of macro expansions */
  size_t macro_expansions_count;
  char *raw_body;
} cdd_ffi_ir_node_t;

/**
 * @struct cdd_ffi_ir_t
 * @brief The root FFI Intermediate Representation encompassing the whole API.
 */
typedef struct cdd_ffi_ir_t {
  /** @brief Array of exported nodes */
  cdd_ffi_ir_node_t *nodes;
  /** @brief Total number of nodes */
  size_t nodes_count;
  /** @brief Capacity of the nodes array */
  size_t nodes_capacity;
  char *module_name;
} cdd_ffi_ir_t;

/**
 * @brief Performs a topological sort of the FFI IR nodes to resolve
 * dependencies.
 * @param ir The IR to sort.
 * @return 0 on success, or an error code (e.g., EINVAL or ENOMEM).
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_ir_topological_sort(cdd_ffi_ir_t *ir);

/**
 * @brief Serializes the FFI IR into a JSON string.
 * @param[in] ir The IR to serialize.
 * @param[out] out_json The resulting JSON string (must be freed with free()).
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_ir_to_json(const cdd_ffi_ir_t *ir,
                                              char **out_json);

/**
 * @brief Deserializes a JSON string into an FFI IR structure.
 * @param[in] json_str The JSON string to parse.
 * @param[out] out_ir The resulting IR structure (must be freed with
 * cdd_ffi_ir_free()).
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
C_CDD_EXPORT cdd_c_error_t cdd_ffi_ir_from_json(const char *json_str,
                                                cdd_ffi_ir_t **out_ir);

/**
 * @brief Frees all memory associated with the FFI IR.
 * @param ir The IR to free.
 */
C_CDD_EXPORT void cdd_ffi_ir_free(cdd_ffi_ir_t *ir);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_FFI_IR_H */
