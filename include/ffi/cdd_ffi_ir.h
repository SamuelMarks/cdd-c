/**
 * @file cdd_ffi_ir.h
 * @brief Universal FFI Intermediate Representation (IR) structures.
 *
 * This file defines the normalized cross-language FFI model used to map
 * C structures, unions, enums, functions, callbacks, and ABI layouts to
 * target language emitters.
 */
#ifndef CDD_FFI_IR_H
/** @brief Header guard for cdd_ffi_ir.h */
#define CDD_FFI_IR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "c_cdd/no_discard.h"
#include "cdd_c_error.h"
#include <stddef.h>
/* clang-format on */

struct cdd_ffi_field_t;

/**
 * @brief Standard FFI error structure for cross-language exception handling.
 */
typedef struct cdd_ffi_error_t {
  /** @brief Error code (0 for success, non-zero for error) */
  int code;
  /** @brief Null-terminated error message string */
  char *message;
} cdd_ffi_error_t;

/**
 * @brief Predefined target ABI data models for struct/union byte-layout
 * calculation.
 */
typedef enum cdd_ffi_abi_kind_t {
  /** @brief Auto-detect current host platform ABI */
  CDD_FFI_ABI_HOST = 0,
  /** @brief 64-bit Unix/macOS/Linux LP64 ABI (8-byte pointers and longs) */
  CDD_FFI_ABI_LP64 = 1,
  /** @brief 64-bit Windows/MSVC LLP64 ABI (8-byte pointers, 4-byte longs) */
  CDD_FFI_ABI_LLP64 = 2,
  /** @brief 32-bit x86/WASM ILP32 ABI (4-byte pointers and longs) */
  CDD_FFI_ABI_ILP32 = 3
} cdd_ffi_abi_kind_t;

/**
 * @brief Target ABI primitive byte sizes and natural alignments.
 */
typedef struct cdd_ffi_abi_profile_t {
  /** @brief Selected ABI model kind */
  cdd_ffi_abi_kind_t kind;
  /** @brief Byte size of data/function pointers (4 or 8) */
  size_t pointer_size;
  /** @brief Natural alignment of pointers in bytes (4 or 8) */
  size_t pointer_align;
  /** @brief Byte size of C `long` / `unsigned long` (8 on LP64, 4 on LLP64) */
  size_t long_size;
  /** @brief Natural alignment of C `long` in bytes */
  size_t long_align;
  /** @brief Byte size of underlying C `enum` representation (typically 4) */
  size_t enum_size;
  /** @brief Natural alignment of C `enum` in bytes (typically 4) */
  size_t enum_align;
  /** @brief Natural alignment of 64-bit integers in bytes (4 or 8) */
  size_t int64_align;
  /** @brief Natural alignment of 64-bit IEEE doubles in bytes (4 or 8) */
  size_t double_align;
  /** @brief Non-zero to fall back to pointer size for unresolved external refs
   */
  int allow_unresolved_refs;
} cdd_ffi_abi_profile_t;

/**
 * @brief Represents primitive and composite type categories in a cross-language
 * way.
 */
typedef enum cdd_ffi_primitive_kind_t {
  CDD_FFI_KIND_VOID,       /**< `void` type */
  CDD_FFI_KIND_BOOL,       /**< Boolean (`bool` / `_Bool`) */
  CDD_FFI_KIND_INT8,       /**< Signed 8-bit integer (`int8_t`/`char`) */
  CDD_FFI_KIND_UINT8,      /**< Unsigned 8-bit integer (`uint8_t`) */
  CDD_FFI_KIND_INT16,      /**< Signed 16-bit integer (`int16_t`) */
  CDD_FFI_KIND_UINT16,     /**< Unsigned 16-bit integer (`uint16_t`) */
  CDD_FFI_KIND_INT32,      /**< Signed 32-bit integer (`int32_t`) */
  CDD_FFI_KIND_UINT32,     /**< Unsigned 32-bit integer (`uint32_t`) */
  CDD_FFI_KIND_INT64,      /**< Signed 64-bit integer (`int64_t`) */
  CDD_FFI_KIND_UINT64,     /**< Unsigned 64-bit integer (`uint64_t`) */
  CDD_FFI_KIND_FLOAT32,    /**< 32-bit IEEE float (`float`) */
  CDD_FFI_KIND_FLOAT64,    /**< 64-bit IEEE double (`double`) */
  CDD_FFI_KIND_STRUCT_REF, /**< Named struct or union reference */
  CDD_FFI_KIND_TEMPLATE_STRUCT_REF, /**< C++ template struct instantiation */
  CDD_FFI_KIND_ENUM_REF,            /**< Named enum reference */
  CDD_FFI_KIND_TYPEDEF_REF,         /**< Named typedef alias reference */
  CDD_FFI_KIND_OPAQUE_PTR,          /**< Opaque handle pointer (`void *`) */
  CDD_FFI_KIND_FUNCTION_PTR,        /**< C callback / function pointer */
  CDD_FFI_KIND_STD_STRING,          /**< C++ `std::string` */
  CDD_FFI_KIND_STD_VECTOR,          /**< C++ `std::vector<T>` */
  CDD_FFI_KIND_STD_SHARED_PTR,      /**< C++ `std::shared_ptr<T>` */
  CDD_FFI_KIND_STD_UNIQUE_PTR       /**< C++ `std::unique_ptr<T>` */
} cdd_ffi_primitive_kind_t;

/**
 * @brief Represents the specific kind of lexical trivia.
 */
typedef enum cdd_ffi_trivia_kind_t {
  CDD_FFI_TRIVIA_WHITESPACE,    /**< Whitespace and newline sequence */
  CDD_FFI_TRIVIA_COMMENT_LINE,  /**< Single-line comment */
  CDD_FFI_TRIVIA_COMMENT_BLOCK, /**< Multi-line or inline block comment */
  CDD_FFI_TRIVIA_CONTINUATION   /**< Preprocessor backslash line continuation */
} cdd_ffi_trivia_kind_t;

/**
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
 * @brief Represents the literal prefix for numbers.
 */
typedef enum cdd_ffi_number_prefix_t {
  CDD_FFI_NUM_PREFIX_NONE,  /**< Base-10 decimal literal */
  CDD_FFI_NUM_PREFIX_HEX,   /**< Base-16 `0x` / `0X` literal */
  CDD_FFI_NUM_PREFIX_OCTAL, /**< Base-8 leading-`0` literal */
  CDD_FFI_NUM_PREFIX_BINARY /**< Base-2 `0b` / `0B` literal */
} cdd_ffi_number_prefix_t;

/**
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

/**
 * @brief Normalized representation of any C type (including pointers, fixed
 * arrays, templates, and typed function pointers).
 */
typedef struct cdd_ffi_type_t {
  /** @brief The base kind of the type */
  cdd_ffi_primitive_kind_t kind;
  /** @brief If true, it's a const type */
  int is_const;
  /** @brief Pointer depth (e.g., 2 for char**) */
  int pointer_depth;
  /** @brief Name of the struct/enum/typedef if applicable */
  char *ref_name;
  /** @brief If it's an array, the fixed size (-1 if dynamic/unknown, 0 if not
   * an array) */
  long array_size;
  /** @brief Template parameters (if CDD_FFI_KIND_TEMPLATE_STRUCT_REF) */
  struct cdd_ffi_type_t *template_args;
  /** @brief Number of template parameters */
  size_t template_args_count;
  /** @brief Leading lexical trivia */
  cdd_ffi_trivia_t *leading_trivia;
  /** @brief Trailing lexical trivia */
  cdd_ffi_trivia_t *trailing_trivia;
  /** @brief Exact raw type spelling */
  char *raw_spelling;
  /** @brief Optional callback return type when `kind ==
   * CDD_FFI_KIND_FUNCTION_PTR` */
  struct cdd_ffi_type_t *fn_return_type;
  /** @brief Optional callback parameter array when `kind ==
   * CDD_FFI_KIND_FUNCTION_PTR` */
  struct cdd_ffi_field_t *fn_params;
  /** @brief Number of callback parameters in `fn_params` */
  size_t fn_params_count;
  /** @brief Non-zero if callback signature is variadic (`...`) */
  int fn_is_variadic;
} cdd_ffi_type_t;

/**
 * @brief Represents the data flow intent of a parameter (In, Out, InOut).
 */
typedef enum cdd_ffi_param_intent_t {
  CDD_FFI_INTENT_UNKNOWN, /**< Unannotated parameter intent */
  CDD_FFI_INTENT_IN,      /**< Read-only input parameter (`[in]`) */
  CDD_FFI_INTENT_OUT,     /**< Write-only output parameter (`[out]`) */
  CDD_FFI_INTENT_INOUT    /**< Read-write parameter (`[inout]`) */
} cdd_ffi_param_intent_t;

/**
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
 * @brief A field inside a struct/union, or an argument inside a function or
 * callback.
 */
typedef struct cdd_ffi_field_t {
  /** @brief Field or parameter name */
  char *name;
  /** @brief Field or parameter type */
  cdd_ffi_type_t type;
  /** @brief Optional docstring */
  char *doc;
  /** @brief Parameter intent (for function arguments) */
  cdd_ffi_param_intent_t intent;
  /** @brief Reference to another field defining the array length (if
   * applicable) */
  char *array_length_ref;
  /** @brief Leading lexical trivia */
  cdd_ffi_trivia_t *leading_trivia;
  /** @brief Trailing lexical trivia */
  cdd_ffi_trivia_t *trailing_trivia;
  /** @brief Computed byte offset of this field within parent struct/union */
  size_t offset_bytes;
  /** @brief Computed byte size of this field */
  size_t size_bytes;
  /** @brief Computed natural alignment of this field in bytes */
  size_t align_bytes;
  /** @brief Computed padding bytes immediately preceding this struct field */
  size_t pad_before_bytes;
  /** @brief Computed trailing bytes needed inside a union variant */
  size_t union_tail_pad_bytes;
  /** @brief C bitfield width in bits (0 when not a bitfield) */
  int bitfield_width;
} cdd_ffi_field_t;

/**
 * @brief The type of an exported FFI IR node.
 */
typedef enum cdd_ffi_node_kind_t {
  CDD_FFI_NODE_STRUCT,   /**< C/C++ struct or class record */
  CDD_FFI_NODE_UNION,    /**< C union record */
  CDD_FFI_NODE_ENUM,     /**< C enumeration */
  CDD_FFI_NODE_FUNCTION, /**< Exported function or C++ flat method wrapper */
  CDD_FFI_NODE_TYPEDEF,  /**< C `typedef` alias */
  CDD_FFI_NODE_MACRO     /**< Preprocessor `#define` constant or macro */
} cdd_ffi_node_kind_t;

/**
 * @brief A specific value inside an enum.
 */
typedef struct cdd_ffi_enum_variant_t {
  /** @brief The variant name (e.g., MY_ENUM_A) */
  char *name;
  /** @brief The raw string value or integer literal */
  char *value;
  /** @brief Optional docstring */
  char *doc;
  /** @brief Leading lexical trivia */
  cdd_ffi_trivia_t *leading_trivia;
  /** @brief Trailing lexical trivia */
  cdd_ffi_trivia_t *trailing_trivia;
  /** @brief Detailed numeric literal representation */
  struct cdd_ffi_number_t *number_details;
  /** @brief Exact raw variant spelling */
  char *raw_spelling;
} cdd_ffi_enum_variant_t;

/**
 * @brief Represents a base class inherited by a C++ struct/class.
 */
typedef struct cdd_ffi_base_class_t {
  /** @brief The base class name */
  char *name;
  /** @brief True if inherited virtually */
  int is_virtual;
  /** @brief The access modifier (e.g. "public") */
  char *access;
} cdd_ffi_base_class_t;

/**
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
 * @brief The inferred type of an evaluated macro.
 */
typedef enum cdd_ffi_macro_type_t {
  CDD_FFI_MACRO_TYPE_UNKNOWN, /**< Unevaluated or non-constant macro */
  CDD_FFI_MACRO_TYPE_INT,     /**< Integer constant macro */
  CDD_FFI_MACRO_TYPE_FLOAT,   /**< Floating-point constant macro */
  CDD_FFI_MACRO_TYPE_STRING   /**< String literal constant macro */
} cdd_ffi_macro_type_t;

/**
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

  /** @brief True if the function requires GIL release (e.g., \@ffi_release_gil
   * or \@blocking) */
  int requires_gil_release;

  /** @brief True if the function accepts a variable number of arguments (...)
   */
  int is_variadic;
  /** @brief Leading lexical trivia */
  cdd_ffi_trivia_t *leading_trivia;
  /** @brief Trailing lexical trivia */
  cdd_ffi_trivia_t *trailing_trivia;

  /** @brief Macro expansions contained within this node */
  struct cdd_ffi_macro_expansion_t *macro_expansions;
  /** @brief Number of macro expansions */
  size_t macro_expansions_count;
  /** @brief Raw function body text (if captured) */
  char *raw_body;

  /** @brief Computed total aligned byte size of struct, union, enum, or typedef
   */
  size_t size_bytes;
  /** @brief Computed natural alignment in bytes */
  size_t align_bytes;
  /** @brief Computed trailing padding bytes at end of struct */
  size_t trailing_pad_bytes;
  /** @brief Non-zero if struct/union uses packed (`#pragma pack(1)`) layout */
  int is_packed;
} cdd_ffi_ir_node_t;

/**
 * @brief The root FFI Intermediate Representation encompassing the whole API.
 */
typedef struct cdd_ffi_ir_t {
  /** @brief Array of exported nodes */
  cdd_ffi_ir_node_t *nodes;
  /** @brief Total number of nodes */
  size_t nodes_count;
  /** @brief Capacity of the nodes array */
  size_t nodes_capacity;
  /** @brief Module name identifier */
  char *module_name;
} cdd_ffi_ir_t;

/**
 * @brief Initializes a `cdd_ffi_abi_profile_t` for a predefined target ABI
 * model.
 *
 * @param[in] kind Predefined ABI model (`CDD_FFI_ABI_HOST`, `LP64`, `LLP64`, or
 * `ILP32`).
 * @param[out] out_profile Destination ABI profile structure to populate.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT on NULL or
 * invalid `kind`.
 */
C_CDD_EXPORT NO_DISCARD cdd_c_error_t cdd_ffi_abi_profile_init(
    cdd_ffi_abi_kind_t kind, cdd_ffi_abi_profile_t *out_profile);

/**
 * @brief Computes byte offsets, inter-field padding, trailing padding, total
 * aligned byte sizes, and alignments for all structs, unions, enums, and
 * typedefs in `ir`.
 *
 * @param[in,out] ir Target FFI IR whose nodes and fields will be annotated.
 * @param[in] abi Optional target ABI profile (uses `CDD_FFI_ABI_HOST` if NULL).
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL/cycle
 * or invalid layout, or CDD_C_ERROR_NOT_FOUND on unresolved type references.
 */
C_CDD_EXPORT NO_DISCARD cdd_c_error_t
cdd_ffi_ir_compute_layout(cdd_ffi_ir_t *ir, const cdd_ffi_abi_profile_t *abi);

/**
 * @brief Performs a topological sort of the FFI IR nodes to resolve
 * dependencies.
 *
 * @param[in,out] ir The IR to sort.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
C_CDD_EXPORT NO_DISCARD cdd_c_error_t
cdd_ffi_ir_topological_sort(cdd_ffi_ir_t *ir);

/**
 * @brief Serializes the FFI IR into a JSON string.
 *
 * @param[in] ir The IR to serialize.
 * @param[out] out_json The resulting JSON string (must be freed with free()).
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
C_CDD_EXPORT NO_DISCARD cdd_c_error_t cdd_ffi_ir_to_json(const cdd_ffi_ir_t *ir,
                                                         char **out_json);

/**
 * @brief Deserializes a JSON string into an FFI IR structure.
 *
 * @param[in] json_str The JSON string to parse.
 * @param[out] out_ir The resulting IR structure (must be freed with
 * cdd_ffi_ir_free()).
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
C_CDD_EXPORT NO_DISCARD cdd_c_error_t
cdd_ffi_ir_from_json(const char *json_str, cdd_ffi_ir_t **out_ir);

/**
 * @brief Frees all memory associated with the FFI IR.
 *
 * @param[in,out] ir The IR to free.
 * @return CDD_C_SUCCESS on success, or error code.
 */
C_CDD_EXPORT NO_DISCARD cdd_c_error_t cdd_ffi_ir_free(cdd_ffi_ir_t *ir);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_FFI_IR_H */
