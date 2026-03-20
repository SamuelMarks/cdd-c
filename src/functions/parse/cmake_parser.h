/**
 * @file cmake_parser.h
 * @brief Auxiliary parser for basic CMakeLists.txt structure manipulation.
 *
 * Implements basic regex/text-based scanning of CMake files to locate
 * targets and safely append MSVC-specific target_compile_options and link
 * libraries.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CMAKE_PARSER_H
#define C_CDD_CMAKE_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
/* clang-format on */

/**
 * @brief Represents a parsed CMake target modification intent.
 */
struct CMakeModifier {
  char *filepath;        /**< Path to CMakeLists.txt */
  char *target_name;     /**< Name of the target (or NULL for global) */
  char **compile_opts;   /**< Array of string options (e.g. "/W4") */
  size_t compile_opts_n; /**< Number of compile options */
  char **link_libs;      /**< Array of library names (e.g. "ws2_32.lib") */
  size_t link_libs_n;    /**< Number of link libraries */
};

/**
 * @brief Initialize a CMake modifier structure.
 *
 * @param[out] mod The structure to initialize.
 * @param[in] filepath The CMakeLists.txt path.
 * @param[in] target_name Optional target name. Pass NULL to apply globally via
 * add_compile_options/link_libraries.
 * @return 0 on success, ENOMEM on failure.
 */
extern C_CDD_EXPORT /**
                     * @brief Executes the cmake modifier init operation.
                     */
    int
    cmake_modifier_init(struct CMakeModifier *mod, const char *filepath,
                        const char *target_name);

/**
 * @brief Add a compile option to be injected.
 */
extern C_CDD_EXPORT /**
                     * @brief Executes the cmake modifier add compile opt
                     * operation.
                     */
    int
    cmake_modifier_add_compile_opt(struct CMakeModifier *mod, const char *opt);

/**
 * @brief Add a link library to be injected.
 */
extern C_CDD_EXPORT /**
                     * @brief Executes the cmake modifier add link lib
                     * operation.
                     */
    int
    cmake_modifier_add_link_lib(struct CMakeModifier *mod, const char *lib);

/**
 * @brief Free resources associated with a CMake modifier.
 */
extern C_CDD_EXPORT /**
                     * @brief Executes the cmake modifier free operation.
                     */
    void
    cmake_modifier_free(struct CMakeModifier *mod);

/**
 * @brief Apply the modifications to the target CMakeLists.txt file.
 *
 * Scans the file. If `target_name` is set, looks for
 * `target_compile_options(target_name ...)` and
 * `target_link_libraries(target_name ...)`. If found, appends the new values
 * within an `if(MSVC)` block. If not found, appends new commands to the end of
 * the file. Creates a `.patch` file instead of direct modification.
 *
 * @param[in] mod The configured modifier.
 * @param[out] out_diff A newly allocated string containing the unified diff.
 * @return 0 on success.
 */
extern C_CDD_EXPORT /**
                     * @brief Executes the cmake modifier apply diff operation.
                     */
    int
    cmake_modifier_apply_diff(const struct CMakeModifier *mod, char **out_diff);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CMAKE_PARSER_H */
