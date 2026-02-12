/**
 * @file doc_parser.h
 * @brief Parser for extracting API metadata from C documentation comments.
 *
 * Scans comments for Doxygen-style annotations:
 * - `@route <VERB> <PATH>`
 * - `@param <name> [flags] <description>`
 *   - flags: `[in:<path|query|header|cookie|querystring>] [required]`
 *     `[style:<form|simple|matrix|label|spaceDelimited|pipeDelimited|deepObject|cookie>]`
 *     `[explode:true|false] [allowReserved:true|false]
 * [allowEmptyValue:true|false] [contentType:<media/type>]`
 * - `@return <status> [contentType:<media/type>] <description>`
 * - `@operationId <id>`
 * - `@summary <text>`
 * - `@description <text>`
 * - `@tag <name>` or `@tags <name1, name2>`
 * - `@deprecated [true|false]`
 * - `@externalDocs <url> [description]`
 * - `@security <scheme> [scope1, scope2]`
 * - `@server <url> [name=<name>] [description=<text>]`
 * - `@requestBody [required|required:true|required:false]
 *      [contentType:<media/type>] <description>`
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
 * @brief Parameter serialization styles (OpenAPI-compatible).
 */
enum DocParamStyle {
  DOC_PARAM_STYLE_UNSET = 0,
  DOC_PARAM_STYLE_FORM,
  DOC_PARAM_STYLE_SIMPLE,
  DOC_PARAM_STYLE_MATRIX,
  DOC_PARAM_STYLE_LABEL,
  DOC_PARAM_STYLE_SPACE_DELIMITED,
  DOC_PARAM_STYLE_PIPE_DELIMITED,
  DOC_PARAM_STYLE_DEEP_OBJECT,
  DOC_PARAM_STYLE_COOKIE
};

/**
 * @brief Represents a documented parameter.
 */
struct DocParam {
  char *name;         /**< Parameter name */
  char *in_loc;       /**< Explicit location (e.g. "path", "query"), or NULL */
  char *description;  /**< Parameter description */
  int required;       /**< 1 if marked required, 0 otherwise */
  char *content_type; /**< Optional content media type */

  /* Optional OpenAPI serialization flags */
  enum DocParamStyle style;  /**< Parameter style override */
  int style_set;             /**< 1 if style explicitly set */
  int explode;               /**< 1 if explode=true */
  int explode_set;           /**< 1 if explode explicitly set */
  int allow_reserved;        /**< 1 if allowReserved=true */
  int allow_reserved_set;    /**< 1 if allowReserved explicitly set */
  int allow_empty_value;     /**< 1 if allowEmptyValue=true */
  int allow_empty_value_set; /**< 1 if allowEmptyValue explicitly set */
};

/**
 * @brief Represents a documented response.
 */
struct DocResponse {
  char *code;         /**< HTTP Status code (e.g. "200", "default") */
  char *description;  /**< Response description */
  char *content_type; /**< Optional response content media type */
};

/**
 * @brief Represents a documented security requirement.
 */
struct DocSecurityRequirement {
  char *scheme;    /**< Security scheme name */
  char **scopes;   /**< Optional scopes */
  size_t n_scopes; /**< Scope count */
};

/**
 * @brief Represents a documented server entry.
 */
struct DocServer {
  char *url;         /**< Server URL */
  char *name;        /**< Optional server name */
  char *description; /**< Optional server description */
};

/**
 * @brief Container for extracted metadata.
 */
struct DocMetadata {
  char *route;                     /**< Route path (e.g. "/users/{id}") */
  char *verb;                      /**< HTTP Method (e.g. "GET", "POST") */
  char *operation_id;              /**< Optional explicit operationId */
  char *summary;                   /**< Operation summary */
  char *description;               /**< Operation description */
  int deprecated;                  /**< Deprecated flag */
  int deprecated_set;              /**< 1 if deprecated explicitly set */
  char **tags;                     /**< Operation tags */
  size_t n_tags;                   /**< Number of tags */
  char *external_docs_url;         /**< externalDocs URL */
  char *external_docs_description; /**< externalDocs description */

  struct DocParam *params; /**< Array of parameters */
  size_t n_params;         /**< Number of parameters */

  struct DocResponse *returns; /**< Array of responses */
  size_t n_returns;            /**< Number of responses */

  struct DocSecurityRequirement *security; /**< Security requirements */
  size_t n_security;                       /**< Security requirement count */

  struct DocServer *servers; /**< Per-operation servers */
  size_t n_servers;          /**< Server count */

  char *request_body_description;  /**< Request body description */
  int request_body_required;       /**< Request body required flag */
  int request_body_required_set;   /**< 1 if required explicitly set */
  char *request_body_content_type; /**< Request body content type */
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
