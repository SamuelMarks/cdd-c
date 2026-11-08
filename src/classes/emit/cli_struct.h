/**
 * @file cli_struct.h
 * @brief Declarative C89 CLI parser, serializer, help, version, and shell
 * completion code generator (`structopt` / `clap` parity).
 *
 * Generates pure ISO C90 (C89) command-line interface parsers and formatters
 * from `struct StructFields` and `struct EnumMembers` reflection metadata:
 * - `<Struct>_from_cli`: Allocates and populates a struct from `(argc, argv)`
 *   with 3-stage precedence (`Default` < `Environment` < `CLI argv`).
 * - `<Struct>_parse_cli_into`: Parses CLI arguments into an existing struct
 *   instance (supporting flattened structs and nested subcommands).
 * - `<Struct>_to_cli_argv`: Serializes a populated C struct back into an
 *   `(argc, argv)` vector for subprocess invocation and roundtrip testing.
 * - `<Struct>_print_help`: Emits structured `clap`-style `--help` output.
 * - `<Struct>_print_version`: Emits `--version` output.
 * - `<Struct>_print_completions`: Emits shell completion scripts for `bash`,
 *   `zsh`, `fish`, and `powershell`.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CODEGEN_CLI_STRUCT_H
#define C_CDD_CODEGEN_CLI_STRUCT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stddef.h>
#include <stdio.h>

#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "classes/emit/enum.h"
#include "classes/emit/struct.h"
/* clang-format on */

struct TypeDefList;

/**
 * @brief Configuration options for declarative CLI code generation.
 */
struct CodegenCliConfig {
  const char *guard_macro; /**< Optional preprocessor guard macro */
  int emit_cli;            /**< Force CLI generation even if !is_cli_command */
  const struct TypeDefList
      *type_registry; /**< Optional type registry for enums/flattening/unions */
};

/**
 * @brief Resolves the effective CLI long option name (without leading `--`)
 * for a struct field.
 *
 * Uses `f->cli_long` if explicitly set; otherwise transforms `f->name` using
 * `sf->cli_rename_all` (defaulting to `kebab-case`).
 *
 * @param[in] sf Parent struct container metadata (may be NULL).
 * @param[in] f Field metadata descriptor.
 * @param[out] out_long Destination buffer for the long flag name.
 * @param[in] out_sz Capacity of `out_long` in bytes.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_cli_resolve_field_long(
    const struct StructFields *sf, const struct StructField *f, char *out_long,
    size_t out_sz);

/**
 * @brief Resolves the effective environment variable name for a struct field.
 *
 * Prepends `sf->cli_env_prefix` when `f->cli_env` was inferred or when
 * `f->cli_env` does not already begin with the prefix.
 *
 * @param[in] sf Parent struct container metadata (may be NULL).
 * @param[in] f Field metadata descriptor.
 * @param[out] out_env Destination buffer for the environment variable name.
 * @param[in] out_sz Capacity of `out_env` in bytes.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_cli_resolve_field_env(
    const struct StructFields *sf, const struct StructField *f, char *out_env,
    size_t out_sz);

/**
 * @brief Resolves the display placeholder `<VALUE_NAME>` for a struct field in
 * `--help` and `Usage:` output.
 *
 * Uses `f->cli_value_name` if set; otherwise transforms `f->name` to
 * `SCREAMING_SNAKE_CASE`.
 *
 * @param[in] f Field metadata descriptor.
 * @param[out] out_val_name Destination buffer for the value placeholder.
 * @param[in] out_sz Capacity of `out_val_name` in bytes.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_cli_resolve_value_name(
    const struct StructField *f, char *out_val_name, size_t out_sz);

/**
 * @brief Computes a bounded Levenshtein edit distance to suggest the closest
 * matching CLI flag or subcommand when an unknown token is encountered.
 *
 * @param[in] unknown_arg Unrecognized CLI argument string (e.g. `"--ouput"`).
 * @param[in] candidates Array of valid candidate strings.
 * @param[in] n_candidates Number of elements in `candidates`.
 * @param[out] out_best Output pointer receiving the closest candidate string,
 * or NULL if no candidate is within the similarity threshold (distance <= 2).
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if inputs
 * are NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_cli_suggest_closest(const char *unknown_arg, const char *const *candidates,
                        size_t n_candidates, const char **out_best);

/**
 * @brief Expands `@file.rsp` response file arguments in an `argv` vector
 * (MSVC `cl.exe` / `link.exe` style) into a newly allocated argument vector.
 *
 * Supports double-quoted `"..."` and single-quoted `'...'` tokens, backslash
 * escapes, `#` comment lines, and nested `@file.rsp` includes up to depth 16
 * (rejecting deeper or cyclic includes with `CDD_C_ERROR_INVALID_ARGUMENT`).
 *
 * @param[in] argc Input argument count.
 * @param[in] argv Input argument vector.
 * @param[out] out_argc Output expanded argument count.
 * @param[out] out_argv Output newly allocated expanded argument vector (free
 * with `cdd_cli_free_argv`).
 * @return CDD_C_SUCCESS on success, or error code on I/O, cycle, or OOM error.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_cli_expand_response_files(
    int argc, char **argv, int *out_argc, char ***out_argv);

/**
 * @brief Frees an argument vector allocated by `cdd_cli_expand_response_files`
 * or `<Struct>_to_cli_argv`.
 *
 * @param[in] argc Number of arguments in `argv`.
 * @param[in,out] argv Heap-allocated argument vector to free.
 * @return CDD_C_SUCCESS.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_cli_free_argv(int argc, char **argv);

/**
 * @brief Emits C prototypes for all generated CLI functions (`_from_cli`,
 * `_parse_cli_into`, `_to_cli_argv`, `_print_help`, `_print_version`,
 * `_print_completions`) into a header stream.
 *
 * @param[in,out] fp Output header stream.
 * @param[in] struct_name Name of the C struct.
 * @param[in] config Optional CLI code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_struct_cli_declarations_h(
    FILE *fp, const char *struct_name, const struct CodegenCliConfig *config);

/**
 * @brief Generates the `<Struct>_print_version` function implementation.
 *
 * @param[in,out] fp Output C stream.
 * @param[in] struct_name Name of the C struct.
 * @param[in] sf Struct fields & container CLI metadata.
 * @param[in] config Optional CLI code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_struct_print_version_func(
    FILE *fp, const char *struct_name, const struct StructFields *sf,
    const struct CodegenCliConfig *config);

/**
 * @brief Generates the `<Struct>_print_help` function implementation.
 *
 * @param[in,out] fp Output C stream.
 * @param[in] struct_name Name of the C struct.
 * @param[in] sf Struct fields & container CLI metadata.
 * @param[in] config Optional CLI code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_struct_print_help_func(
    FILE *fp, const char *struct_name, const struct StructFields *sf,
    const struct CodegenCliConfig *config);

/**
 * @brief Generates the `<Struct>_print_completions` function implementation
 * supporting `"bash"`, `"zsh"`, `"fish"`, and `"powershell"` / `"pwsh"`.
 *
 * @param[in,out] fp Output C stream.
 * @param[in] struct_name Name of the C struct.
 * @param[in] sf Struct fields & container CLI metadata.
 * @param[in] config Optional CLI code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_struct_print_completions_func(
    FILE *fp, const char *struct_name, const struct StructFields *sf,
    const struct CodegenCliConfig *config);

/**
 * @brief Generates the `<Struct>_parse_cli_into` function implementation for
 * parsing CLI arguments into an already-allocated struct instance.
 *
 * @param[in,out] fp Output C stream.
 * @param[in] struct_name Name of the C struct.
 * @param[in] sf Struct fields & container CLI metadata.
 * @param[in] config Optional CLI code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_struct_cli_parse_into_func(
    FILE *fp, const char *struct_name, const struct StructFields *sf,
    const struct CodegenCliConfig *config);

/**
 * @brief Generates the `<Struct>_from_cli` function implementation.
 *
 * @param[in,out] fp Output C stream.
 * @param[in] struct_name Name of the C struct.
 * @param[in] sf Struct fields & container CLI metadata.
 * @param[in] config Optional CLI code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_struct_from_cli_func(
    FILE *fp, const char *struct_name, const struct StructFields *sf,
    const struct CodegenCliConfig *config);

/**
 * @brief Generates the `<Struct>_to_cli_argv` function implementation to
 * serialize a populated C struct back into an `(argc, argv)` array.
 *
 * @param[in,out] fp Output C stream.
 * @param[in] struct_name Name of the C struct.
 * @param[in] sf Struct fields & container CLI metadata.
 * @param[in] config Optional CLI code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_struct_to_cli_argv_func(
    FILE *fp, const char *struct_name, const struct StructFields *sf,
    const struct CodegenCliConfig *config);

/**
 * @brief Convenience generator that emits all CLI functions (`_print_version`,
 * `_print_help`, `_print_completions`, `_parse_cli_into`, `_from_cli`,
 * `_to_cli_argv`) for a struct.
 *
 * @param[in,out] fp Output C stream.
 * @param[in] struct_name Name of the C struct.
 * @param[in] sf Struct fields & container CLI metadata.
 * @param[in] config Optional CLI code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_struct_cli_all_funcs(
    FILE *fp, const char *struct_name, const struct StructFields *sf,
    const struct CodegenCliConfig *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CODEGEN_CLI_STRUCT_H */
