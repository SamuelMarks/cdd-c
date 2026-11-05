/**
 * @file struct.h
 * @brief Core Struct Lifecycle generation definitions and implementation.
 *
 * Provides functionality to generate C "Data Class" utilities:
 * - `_cleanup`: Recursive memory freeing.
 * - `_deepcopy`: Recursive independent copying.
 * - `_eq`: Deep equality comparison.
 * - `_default`: Initialization with default values.
 * - `_debug` / `_display`: Inspection utilities.
 *
 * Updates:
 * - Support for `nullptr` keyword (mapped to `NULL`).
 * - Support for binary literals `0b...` (mapped to decimal).
 * - Flexible array member support.
 * - Bit-field support via `bit_width`.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CODEGEN_STRUCT_H
#define C_CDD_CODEGEN_STRUCT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include <stddef.h>
#include <stdio.h>

#include "c_cdd/reflect.h"
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "classes/emit/enum.h"
/* clang-format on */

/**
 * @brief Represents a single field within a struct.
 * Used to drive generation logic based on type traits.
 */
struct StructField {
  char name[64]; /**< Field identifier */
  char type[64]; /**< C or Logical Field type (e.g. "string", "integer",
                    "object") */
  char ref[256]; /**< Reference type name (for objects/enums) or item type (for
                    arrays) */
  char **type_union;       /**< Optional type array (e.g. ["string","null"]) */
  size_t n_type_union;     /**< Count of type_union entries */
  char default_val[64];    /**< Default value literal (e.g. "5", "0b101",
                              "nullptr") or empty */
  char *schema_extra_json; /**< Serialized JSON for extra schema keywords */
  char *items_extra_json;  /**< Serialized JSON for array items keywords */
  char **items_type_union; /**< Optional items type array for arrays */
  size_t n_items_type_union; /**< Count of items_type_union entries */

  /* Validation Constraints */
  int has_min;           /**< 1 if minimum constraint exists */
  double min_val;        /**< Minimum value */
  int exclusive_min;     /**< 1 if exclusive minimum */
  int has_max;           /**< 1 if maximum constraint exists */
  double max_val;        /**< Maximum value */
  int exclusive_max;     /**< 1 if exclusive maximum */
  int has_min_len;       /**< 1 if minLength constraint exists */
  size_t min_len;        /**< Minimum length for strings */
  int has_max_len;       /**< 1 if maxLength constraint exists */
  size_t max_len;        /**< Maximum length for strings */
  char pattern[256];     /**< Regex pattern string */
  char format[32];       /**< Optional JSON Schema format (e.g. "uuid") */
  char description[256]; /**< Optional field description */
  int deprecated;        /**< 1 if deprecated=true */
  int deprecated_set;    /**< 1 if deprecated explicitly set */
  int read_only;         /**< 1 if readOnly=true */
  int read_only_set;     /**< 1 if readOnly explicitly set */
  int write_only;        /**< 1 if writeOnly=true */
  int write_only_set;    /**< 1 if writeOnly explicitly set */
  int has_min_items;     /**< 1 if minItems constraint exists */
  size_t min_items;      /**< Minimum items for arrays */
  int has_max_items;     /**< 1 if maxItems constraint exists */
  size_t max_items;      /**< Maximum items for arrays */
  int unique_items;      /**< 1 if uniqueItems constraint exists */
  int required;          /**< 1 if required in schema, 0 optional */

  /* C Type Properties */
  int is_flexible_array; /**< 1 if field is a Flexible Array Member `type
                            name[]`, 0 otherwise */
  char bit_width[64]; /**< Bit-field width literal (e.g. "3", "8"), or empty if
                         not a bit-field */

  /* Serde Reflection & Rewriting Metadata */
  char serde_rename[64];     /**< Explicit wire name override for ser & de */
  char serde_rename_ser[64]; /**< Explicit wire name override for serialize */
  char serde_rename_de[64];  /**< Explicit wire name override for deserialize */
  enum CddSerdeCaseRule serde_case; /**< Field-level case transformation rule */
  char serde_replace_from[32];      /**< Field-level substring replace source */
  char serde_replace_to[32];        /**< Field-level substring replace target */
  char **serde_aliases;   /**< Alternative JSON keys accepted on deserialize */
  size_t n_serde_aliases; /**< Count of serde_aliases */
  int serde_skip;         /**< 1 if skipped in both serialize and deserialize */
  int serde_skip_serializing;   /**< 1 if omitted during serialization */
  int serde_skip_deserializing; /**< 1 if ignored during deserialization */
  enum CddSerdeSkipIfKind
      serde_skip_if_kind;    /**< Conditional skip predicate kind */
  char serde_skip_if_fn[64]; /**< Custom C predicate function for skip_if */
  int serde_flatten; /**< 1 if nested object keys are flattened into parent */
  int serde_has_default;         /**< 1 if \@cdd default is set on this field */
  char serde_default_fn[64];     /**< Optional custom default initializer fn */
  char serde_with[64];           /**< Custom codec module prefix */
  char serde_serialize_with[64]; /**< Custom serializer function name */
  char serde_deserialize_with[64]; /**< Custom deserializer function name */

  /* CLI Argument Reflection Metadata (clap / structopt parity) */
  char cli_short; /**< Single-char short option (e.g. 'f'), 0 if none */
  char cli_short_aliases[8]; /**< Extra short option characters */
  char cli_long[64];         /**< Explicit long option name override */
  int cli_has_long; /**< 1 if long flag is enabled, -1 if explicitly disabled */
  int cli_negatable;  /**< 1 if `--no-<long>` boolean negation is enabled */
  int cli_positional; /**< 1 if argument is positional */
  int cli_index;      /**< 0-based explicit positional index, or -1 */
  int cli_last;       /**< 1 if positional only matched after `--` */
  int cli_trailing_var_arg;  /**< 1 if captures all remaining arguments */
  char cli_value_name[64];   /**< Placeholder shown in usage/help */
  char cli_help[256];        /**< Short one-line help text */
  char *cli_long_help;       /**< Heap-allocated multi-line help text */
  char cli_help_heading[64]; /**< Custom help section heading */
  char cli_env[64];          /**< Environment variable fallback name */
  int cli_hide_env;          /**< 1 to hide [env: ...] from --help */
  int cli_hide_env_values;   /**< 1 to hide env value from --help */
  char cli_default_missing_val[64]; /**< Value used when flag has no =val */
  int cli_hide_default_value;       /**< 1 to omit [default: ...] in --help */
  enum CddCliAction cli_action;     /**< CLI argument action behavior */
  enum CddCliValueHint cli_value_hint; /**< Shell completion value hint */
  char cli_value_delimiter; /**< Delimiter char to split array values, or 0 */
  int cli_num_args_min;     /**< Min values per occurrence (0 = default) */
  int cli_num_args_max;     /**< Max values per occurrence (-1 = unbounded) */
  int cli_require_equals;   /**< 1 to require --opt=val syntax */
  int cli_allow_hyphen_values;  /**< 1 to allow values starting with '-' */
  int cli_global;               /**< 1 if option propagates to subcommands */
  int cli_hidden;               /**< 1 to omit option from --help */
  int cli_exclusive;            /**< 1 if cannot be combined with other args */
  int cli_subcommand;           /**< 1 if field is the subcommand target */
  char cli_value_parser[64];    /**< Custom C validator/parser function name */
  char cli_group[64];           /**< ArgGroup name for mutual exclusion */
  char **cli_conflicts_with;    /**< Array of conflicting field names */
  size_t n_cli_conflicts_with;  /**< Count of cli_conflicts_with */
  char **cli_requires;          /**< Array of required companion field names */
  size_t n_cli_requires;        /**< Count of cli_requires */
  char **cli_required_unless;   /**< Array of alternative satisfying fields */
  size_t n_cli_required_unless; /**< Count of cli_required_unless */
  char cli_required_if_field[64]; /**< Conditional requirement target field */
  char cli_required_if_val[64];   /**< Conditional requirement trigger value */
  int cli_skip;                   /**< 1 if skipped in both CLI parse & emit */
  int cli_skip_to_cli;            /**< 1 if omitted during to_cli_argv */
  int cli_skip_from_cli;          /**< 1 if ignored during from_cli */

  /* Rich C Type & Enum Array Reflection Extensions */
  int is_enum_array;       /**< Non-zero if array items reference an enum */
  char enum_array_ref[64]; /**< Referenced enum name for enum arrays */
  char c_type[64];         /**< Exact C type (e.g. "int64_t", "uint8_t") */
  int is_nullable_ptr;     /**< Non-zero if primitive is a nullable pointer */
  size_t fixed_array_len;  /**< Inline fixed C array length, or 0 if dynamic */
};

/**
 * @brief Union variant JSON type (for oneOf/anyOf codegen).
 */
enum UnionVariantJsonType {
  UNION_JSON_UNKNOWN = 0,
  UNION_JSON_OBJECT,
  UNION_JSON_STRING,
  UNION_JSON_INTEGER,
  UNION_JSON_NUMBER,
  UNION_JSON_BOOLEAN,
  UNION_JSON_ARRAY,
  UNION_JSON_NULL
};

/**
 * @brief Metadata for a union variant.
 */
struct UnionVariantMeta {
  enum UnionVariantJsonType json_type; /**< Expected JSON value type */
  char **required_props;               /**< Required property names */
  size_t n_required_props;             /**< Count of required properties */
  char **property_names;               /**< Defined property names */
  size_t n_property_names;             /**< Count of properties */
  char *disc_value; /**< Discriminator value for this variant */
};

/**
 * @brief Container for fields of a struct.
 */
struct StructFields {
  size_t capacity;                 /**< allocated capacity */
  size_t size;                     /**< number of fields */
  struct StructField *fields;      /**< dynamic array of fields */
  int is_enum;                     /**< 1 if schema is an enum */
  struct EnumMembers enum_members; /**< Enum values when is_enum=1 */
  char *schema_extra_json; /**< Serialized JSON for extra schema keywords */
  int is_union;            /**< 1 if schema represents a union (oneOf/anyOf) */
  int union_is_anyof;      /**< 1 if union came from anyOf (else oneOf) */
  char *union_discriminator; /**< Discriminator property name, if any */
  struct UnionVariantMeta *union_variants; /**< Per-variant metadata */
  size_t n_union_variants;                 /**< Count of union variants */

  /* Container-level Serde Reflection & Rewriting Metadata */
  char serde_rename[64]; /**< Container-level explicit schema name override */
  enum CddSerdeCaseRule serde_rename_all;     /**< Container-wide case rule */
  enum CddSerdeCaseRule serde_rename_all_ser; /**< Serialize-only case rule */
  enum CddSerdeCaseRule serde_rename_all_de;  /**< Deserialize-only case rule */
  char serde_replace_from[32];   /**< Container-wide substring replace source */
  char serde_replace_to[32];     /**< Container-wide substring replace target */
  char serde_prefix[64];         /**< Container-wide field prefix rule */
  char serde_suffix[64];         /**< Container-wide field suffix rule */
  int serde_deny_unknown_fields; /**< 1 if unknown JSON fields are rejected */
  int serde_default;             /**< 1 if missing fields use struct defaults */
  char serde_tag[64];            /**< Sum-type tag property name */
  char serde_content[64];        /**< Sum-type adjacent content property name */
  int serde_untagged;            /**< 1 if sum-type is untagged */

  /* Container-level CLI Reflection Metadata (clap / structopt Command) */
  int is_cli_command;           /**< 1 if \@cdd cli / \@cli is active */
  char cli_name[64];            /**< Binary or subcommand name override */
  char cli_bin_name[64];        /**< Usage display binary name override */
  char cli_version[64];         /**< Version string override */
  char cli_author[128];         /**< Author string override */
  char cli_about[256];          /**< Short one-line command summary */
  char *cli_long_about;         /**< Heap-allocated multi-line description */
  char *cli_before_help;        /**< Heap-allocated banner before --help */
  char *cli_after_help;         /**< Heap-allocated footer after --help */
  char cli_usage_override[256]; /**< Custom Usage: string override */
  enum CddSerdeCaseRule cli_rename_all;     /**< Case rule for long flags */
  enum CddSerdeCaseRule cli_rename_all_env; /**< Case rule for env vars */
  char cli_env_prefix[64];            /**< Prefix for inferred env variables */
  int cli_arg_required_else_help;     /**< 1 if argc==1 prints help and exits */
  int cli_subcommand_required;        /**< 1 if subcommand is mandatory */
  int cli_allow_external_subcommands; /**< 1 to allow catch-all subcommands */
  int cli_disable_help_flag;          /**< 1 to omit built-in -h/--help */
  int cli_disable_version_flag;       /**< 1 to omit built-in -V/--version */
  int cli_disable_help_subcommand;    /**< 1 to omit built-in help subcommand */
  int cli_propagate_version; /**< 1 to propagate version to subcommands */
  int cli_allow_windows_slash_flags; /**< 1 to accept /flag and /? on _WIN32 */
  int cli_allow_response_files;      /**< 1 to expand `@file.rsp` arguments */
  int cli_case_insensitive_flags;    /**< 1 to match flags case-insensitively */
  char **cli_aliases;                /**< Subcommand aliases array */
  size_t n_cli_aliases;              /**< Count of cli_aliases */
};

/**
 * @brief Configuration options for struct code generation.
 */
struct CodegenStructConfig {
  /**
   * @brief Macro name to guard generated functions (e.g. "DATA_UTILS").
   * If NULL, no \#ifdef/\#endif block is generated.
   */
  const char *guard_macro;
};

/**
 * @brief Initialize a StructFields container.
 *
 * @param[out] sf Pointer to container.
 * @return 0 on success, EINVAL if NULL, ENOMEM if alloc fails.
 */
extern C_CDD_EXPORT cdd_c_error_t struct_fields_init(struct StructFields *sf);

/**
 * @brief Free memory within a StructFields container.
 *
 * @param[in] sf Pointer to container.
 */
extern C_CDD_EXPORT void struct_fields_free(struct StructFields *sf);

/**
 * @brief Add a field to the container.
 *
 * @param[in,out] sf Pointer to the container.
 * @param[in] name Field name.
 * @param[in] type Field type ("integer", "string", "object", etc.)
 * @param[in] ref Reference type (nullable).
 * @param[in] default_val Default value literal (nullable).
 * @param[in] bit_width Bit-field width literal (nullable, e.g. "3").
 * @return 0 on success, ENOMEM on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t struct_fields_add(
    struct StructFields *sf, const char *name, const char *type,
    const char *ref, const char *default_val, const char *bit_width);

/**
 * @param[out] _out_val Pointer to store the result
 * @brief Search for a field by name.
 *
 * @param[in] sf Pointer to the container.
 * @param[in] name Field name to find.
 * @return Pointer to the field if found, NULL otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
struct_fields_get(const struct StructFields *sf, const char *name,
                  struct StructField **_out_val);

/**
 * @brief Appends a deserialization alias key to a StructField.
 *
 * @param[in,out] f Pointer to the target StructField.
 * @param[in] alias Alternative JSON key string to accept during
 * deserialization.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t struct_field_add_alias(struct StructField *f,
                                                         const char *alias);

/**
 * @brief Appends a CLI subcommand alias to a StructFields container.
 *
 * @param[in,out] sf Pointer to the target StructFields container.
 * @param[in] alias Subcommand alias string to accept.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
struct_fields_add_cli_alias(struct StructFields *sf, const char *alias);

/**
 * @brief Appends a conflicting CLI field name to a StructField.
 *
 * @param[in,out] f Pointer to the target StructField.
 * @param[in] conflict Name of the conflicting field.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
struct_field_add_cli_conflict(struct StructField *f, const char *conflict);

/**
 * @brief Appends a required companion CLI field name to a StructField.
 *
 * @param[in,out] f Pointer to the target StructField.
 * @param[in] req Name of the required field.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
struct_field_add_cli_requires(struct StructField *f, const char *req);

/**
 * @brief Appends an alternative satisfying CLI field name (`required_unless`)
 * to a StructField.
 *
 * @param[in,out] f Pointer to the target StructField.
 * @param[in] req Name of the alternative field.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
struct_field_add_cli_required_unless(struct StructField *f, const char *req);

/* --- Generator Functions --- */

/**
 * @brief Generate the `_cleanup` function.
 * Frees memory recursively.
 *
 * @param[in] fp File pointer to write to.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Fields descriptor.
 * @param[in] config Configuration options.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_struct_cleanup_func(
    FILE *fp, const char *struct_name, const struct StructFields *sf,
    const struct CodegenStructConfig *config);

/**
 * @brief Generate the `_deepcopy` function.
 * Creates an independent copy of the struct.
 *
 * @param[in] fp File pointer to write to.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Fields descriptor.
 * @param[in] config Configuration options.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_struct_deepcopy_func(
    FILE *fp, const char *struct_name, const struct StructFields *sf,
    const struct CodegenStructConfig *config);

/**
 * @brief Generate the `_eq` function.
 * Performs deep equality checking.
 *
 * @param[in] fp File pointer to write to.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Fields descriptor.
 * @param[in] config Configuration options.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_struct_eq_func(
    FILE *fp, const char *struct_name, const struct StructFields *sf,
    const struct CodegenStructConfig *config);

/**
 * @brief Generate the `_default` function.
 * Allocates and initializes the struct with default values.
 * Handles `nullptr` and `0b` binary literals by checking `numeric_parser`
 * logic.
 *
 * @param[in] fp File pointer to write to.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Fields descriptor.
 * @param[in] config Configuration options.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_struct_default_func(
    FILE *fp, const char *struct_name, const struct StructFields *sf,
    const struct CodegenStructConfig *config);

/**
 * @brief Generate the `_debug` function.
 * Prints struct contents for debugging.
 *
 * @param[in] fp File pointer to write to.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Fields descriptor.
 * @param[in] config Configuration options.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_struct_debug_func(
    FILE *fp, const char *struct_name, const struct StructFields *sf,
    const struct CodegenStructConfig *config);

/**
 * @brief Generate the `_display` function.
 * Wrapper around JSON serialization for printing.
 *
 * @param[in] fp File pointer to write to.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Fields descriptor.
 * @param[in] config Configuration options.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_struct_display_func(
    FILE *fp, const char *struct_name, const struct StructFields *sf,
    const struct CodegenStructConfig *config);

/**
 * @param[out] _out_val Pointer to store the result
 * @brief Helper to extract type name from a reference path.
 *
 * @param[in] ref Reference string (e.g. "#/components/schemas/Type").
 * @return The base type name ("Type") or the input string if no slash.
 */
extern C_CDD_EXPORT cdd_c_error_t get_type_from_ref(const char *ref,
                                                    const char **_out_val);

/**
 * @brief Emits runtime reflection descriptor tables (`struct cdd_field_info[]`
 * and `struct cdd_type_info`) and `<struct_name>_reflect` accessor for a C
 * struct.
 *
 * @param[in] fp File pointer to write to.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Fields descriptor.
 * @param[in] config Optional configuration options.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_struct_reflect_func(
    FILE *fp, const char *struct_name, const struct StructFields *sf,
    const struct CodegenStructConfig *config);

struct FuncSignature;

/**
 * @brief Emits runtime function reflection descriptor tables
 * (`struct cdd_param_info[]` and `struct cdd_func_info`) and
 * `<func_name>_reflect` accessor for a C function signature.
 *
 * @param[in] fp File pointer to write to.
 * @param[in] sig Extracted function signature descriptor.
 * @param[in] config Optional configuration options.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
write_func_reflect_func(FILE *fp, const struct FuncSignature *sig,
                        const struct CodegenStructConfig *config);

/**
 * @brief Emits C11/C23 `_Generic` polymorphic reflection dispatch macros
 * (`CDD_REFLECT`, `CDD_TO_JSON`, `CDD_CLEANUP`, `CDD_DEBUG`) for a set of
 * struct names.
 *
 * @param[in] fp File pointer to write to.
 * @param[in] struct_names Array of struct names.
 * @param[in] n_structs Number of struct names in the array.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_struct_generic_dispatch_macros(
    FILE *fp, const char *const *struct_names, size_t n_structs);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CODEGEN_STRUCT_H */
