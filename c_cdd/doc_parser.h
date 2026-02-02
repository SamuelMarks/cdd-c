/**
 * @file doc_parser.h
 * @brief Parser for extracting API metadata from C documentation comments.
 *
 * Scans comments for Doxygen-style annotations:
 * - `@route <VERB> <PATH>`
 * - `@param <name> [flags] <description>`
 * - `@return <status> <description>`
 * - `@summary <text>`
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_DOC_PARSER_H
#define C_CDD_DOC_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "c_cdd_export.h"
#include <stddef.h>

/**
 * @brief Represents a documented parameter.
 */
struct DocParam {
  char *name;        /**< Parameter name */
  char *in_loc;      /**< Explicit location (e.g. "path", "query"), or NULL */
  char *description; /**< Parameter description */
  int required;      /**< 1 if marked required, 0 otherwise */
};

/**
 * @brief Represents a documented response.
 */
struct DocResponse {
  char *code;        /**< HTTP Status code (e.g. "200", "default") */
  char *description; /**< Response description */
};

/**
 * @brief Container for extracted metadata.
 */
struct DocMetadata {
  char *route;   /**< Route path (e.g. "/users/{id}") */
  char *verb;    /**< HTTP Method (e.g. "GET", "POST") */
  char *summary; /**< Operation summary */

  struct DocParam *params; /**< Array of parameters */
  size_t n_params;         /**< Number of parameters */

  struct DocResponse *returns; /**< Array of responses */
  size_t n_returns;            /**< Number of responses */
};

/**
 * @brief Initialize a DocMetadata structure.
 *
 * @param[out] meta The structure to initialize.
 * @return 0 on success, EINVAL if meta is NULL.
 */
extern C_CDD_EXPORT int doc_metadata_init(struct DocMetadata *meta);

/**
 * @brief Free resources within a DocMetadata structure.
 *
 * @param[in] meta The structure to free.
 */
extern C_CDD_EXPORT void doc_metadata_free(struct DocMetadata *meta);

/**
 * @brief Parse a raw comment string into structured metadata.
 *
 * Handles block comments (`\/__ ... __\/`) and line comments (`/// ...`).
 * Strips decorative asterisks and whitespace.
 * Parses annotations line by line.
 *
 * @param[in] comment The raw comment string to parse.
 * @param[out] out The destination structure (must be initialized).
 * @return 0 on success, ENOMEM on allocation failure, EINVAL on bad input.
 */
extern C_CDD_EXPORT int doc_parse_block(const char *comment,
                                        struct DocMetadata *out);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_DOC_PARSER_H */
