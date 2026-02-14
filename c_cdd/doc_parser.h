/**
 * @file doc_parser.h
 * @brief Parser for extracting API metadata from C documentation comments.
 *
 * Scans comments for Doxygen-style annotations:
 * - `@route <VERB> <PATH>` (custom verbs are emitted via
 * `additionalOperations`)
 * - `@param <name> [flags] <description>`
 *   - flags: `[in:<path|query|header|cookie|querystring>] [required]`
 *     `[style:<form|simple|matrix|label|spaceDelimited|pipeDelimited|deepObject|cookie>]`
 *     `[explode:true|false] [allowReserved:true|false]`
 *     `[allowEmptyValue:true|false] [contentType:<media/type>] [format:<fmt>]`
 *     `[deprecated:true|false] [example:<json>]`
 * - `@return <status> [contentType:<media/type>] [summary:<text>]`
 *      `[example:<json>] <description>`
 *   - Repeat the same status with different `contentType` values to emit
 *     multi-content responses.
 * - `@operationId <id>`
 * - `@summary <text>`
 * - `@description <text>`
 * - `@tag <name>` or `@tags <name1, name2>`
 * - `@tagMeta <name> [summary:<text>] [description:<text>] [parent:<name>]`
 *      [kind:<value>] [externalDocs:<url>]
 *      [externalDocsDescription:<text>]`
 * - `@webhook <VERB> <PATH>` (emits operation under OpenAPI `webhooks`)
 * - `@deprecated [true|false]`
 * - `@externalDocs <url> [description]`
 * - `@security <scheme> [scope1, scope2]`
 * - `@securityScheme <name>
 * [type:<apiKey|http|mutualTLS|oauth2|openIdConnect>]` [description:<text>]
 * [deprecated:true|false] [paramName:<param>] [in:<query|header|cookie>]
 *      [scheme:<http-scheme>] [bearerFormat:<fmt>]
 *      [openIdConnectUrl:<url>] [oauth2MetadataUrl:<url>]
 *      [flow:<implicit|password|clientCredentials|authorizationCode|deviceAuthorization>]
 *      [authorizationUrl:<url>] [tokenUrl:<url>] [refreshUrl:<url>]
 *      [deviceAuthorizationUrl:<url>] [scopes:<scope1,scope2,...>]`
 * - `@server <url> [name=<name>] [description=<text>]`
 * - `@serverVar <name> [default:<value>] [enum:<v1,v2,...>]
 * [description:<text>]` (attaches to the most recent @server in the same doc
 * block)
 * - `@infoTitle <text>`
 * - `@infoVersion <text>`
 * - `@infoSummary <text>`
 * - `@infoDescription <text>`
 * - `@termsOfService <url>`
 * - `@contact [name:<text>] [url:<url>] [email:<email>]`
 * - `@license [name:<text>] [identifier:<spdx>] [url:<url>]`
 * - `@requestBody [required|required:true|required:false]
 *      [contentType:<media/type>] [example:<json>] <description>`
 * - `@responseHeader <status> <name> [type:<string|integer|number|boolean>]
 *      [format:<fmt>] [contentType:<media/type>]
 *      [required|required:true|required:false] [example:<json>] <description>`
 * - `@link <status> <name> [operationId:<id> | operationRef:<uri>]
 *      [parameters:<json>] [requestBody:<json>]
 *      [summary:<text>] [description:<text>]
 *      [serverUrl:<url>] [serverName:<name>] [serverDescription:<text>]
 *      <description>`
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
  char *format;       /**< Optional schema format override */
  int required;       /**< 1 if marked required, 0 otherwise */
  char *content_type; /**< Optional content media type */
  char *example;      /**< Optional example (raw JSON or string) */
  int deprecated;     /**< 1 if deprecated */
  int deprecated_set; /**< 1 if deprecated explicitly set */

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
  char *summary;      /**< Optional response summary */
  char *description;  /**< Response description */
  char *content_type; /**< Optional response content media type */
  char *example;      /**< Optional example (raw JSON or string) */
};

/**
 * @brief Represents a documented response header.
 */
struct DocResponseHeader {
  char *code;   /**< HTTP Status code (e.g. "200", "default") */
  char *name;   /**< Header name */
  char *type;   /**< Optional schema type */
  char *format; /**< Optional schema format */
  char
      *content_type; /**< Optional content media type (Header Object content) */
  char *description; /**< Header description */
  char *example;     /**< Optional example (raw JSON or string) */
  int required;      /**< 1 if required */
  int required_set;  /**< 1 if required explicitly set */
};

/**
 * @brief Represents a documented response link.
 */
struct DocLink {
  char *code;               /**< HTTP Status code (e.g. "200", "default") */
  char *name;               /**< Link name */
  char *operation_id;       /**< Optional operationId */
  char *operation_ref;      /**< Optional operationRef */
  char *summary;            /**< Optional summary */
  char *description;        /**< Optional description */
  char *parameters_json;    /**< Optional JSON object for parameters map */
  char *request_body_json;  /**< Optional JSON for requestBody */
  char *server_url;         /**< Optional server URL override */
  char *server_name;        /**< Optional server name */
  char *server_description; /**< Optional server description */
};

/**
 * @brief Represents a documented request body entry.
 */
struct DocRequestBody {
  char *content_type; /**< Optional request body content media type */
  char *description;  /**< Optional request body description */
  char *example;      /**< Optional example (raw JSON or string) */
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
 * @brief Supported security scheme types in doc annotations.
 */
enum DocSecurityType {
  DOC_SEC_UNSET = 0,
  DOC_SEC_APIKEY,
  DOC_SEC_HTTP,
  DOC_SEC_MUTUALTLS,
  DOC_SEC_OAUTH2,
  DOC_SEC_OPENID
};

/**
 * @brief Location of API key in doc annotations.
 */
enum DocSecurityIn {
  DOC_SEC_IN_UNSET = 0,
  DOC_SEC_IN_QUERY,
  DOC_SEC_IN_HEADER,
  DOC_SEC_IN_COOKIE
};

/**
 * @brief OAuth2 flow types in doc annotations.
 */
enum DocOAuthFlowType {
  DOC_OAUTH_FLOW_UNSET = 0,
  DOC_OAUTH_FLOW_IMPLICIT,
  DOC_OAUTH_FLOW_PASSWORD,
  DOC_OAUTH_FLOW_CLIENT_CREDENTIALS,
  DOC_OAUTH_FLOW_AUTHORIZATION_CODE,
  DOC_OAUTH_FLOW_DEVICE_AUTHORIZATION
};

/**
 * @brief OAuth2 scope entry in doc annotations.
 */
struct DocOAuthScope {
  char *name;        /**< Scope name */
  char *description; /**< Optional scope description */
};

/**
 * @brief OAuth2 flow entry in doc annotations.
 */
struct DocOAuthFlow {
  enum DocOAuthFlowType type;     /**< Flow type */
  char *authorization_url;        /**< authorizationUrl */
  char *token_url;                /**< tokenUrl */
  char *refresh_url;              /**< refreshUrl */
  char *device_authorization_url; /**< deviceAuthorizationUrl */
  struct DocOAuthScope *scopes;   /**< Scopes list */
  size_t n_scopes;                /**< Scope count */
};

/**
 * @brief Security Scheme definition in doc annotations.
 */
struct DocSecurityScheme {
  char *name;                 /**< Scheme key */
  enum DocSecurityType type;  /**< Scheme type */
  char *description;          /**< Optional description */
  int deprecated;             /**< 1 if deprecated=true */
  int deprecated_set;         /**< 1 if deprecated explicitly set */
  char *scheme;               /**< HTTP auth scheme */
  char *bearer_format;        /**< Optional bearerFormat */
  char *param_name;           /**< apiKey parameter name */
  enum DocSecurityIn in;      /**< apiKey location */
  char *open_id_connect_url;  /**< openIdConnectUrl */
  char *oauth2_metadata_url;  /**< oauth2MetadataUrl */
  struct DocOAuthFlow *flows; /**< OAuth2 flows */
  size_t n_flows;             /**< Flow count */
};

/**
 * @brief Represents a documented server entry.
 */
/**
 * @brief Represents a documented server variable entry.
 */
struct DocServerVar {
  char *name;           /**< Variable name */
  char *default_value;  /**< Default value (required) */
  char *description;    /**< Optional description */
  char **enum_values;   /**< Optional enum values */
  size_t n_enum_values; /**< Enum value count */
};

/**
 * @brief Represents a documented server entry.
 */
struct DocServer {
  char *url;                      /**< Server URL */
  char *name;                     /**< Optional server name */
  char *description;              /**< Optional server description */
  struct DocServerVar *variables; /**< Server variable definitions */
  size_t n_variables;             /**< Server variable count */
};

/**
 * @brief Represents metadata for a tag definition.
 */
struct DocTagMeta {
  char *name;                      /**< Tag name */
  char *summary;                   /**< Tag summary */
  char *description;               /**< Tag description */
  char *parent;                    /**< Parent tag name */
  char *kind;                      /**< Tag kind */
  char *external_docs_url;         /**< externalDocs URL */
  char *external_docs_description; /**< externalDocs description */
};

/**
 * @brief Container for extracted metadata.
 */
struct DocMetadata {
  char *route;              /**< Route path (e.g. "/users/{id}") */
  char *verb;               /**< HTTP Method (e.g. "GET", "POST") */
  int is_webhook;           /**< 1 if this route is a webhook */
  char *operation_id;       /**< Optional explicit operationId */
  char *summary;            /**< Operation summary */
  char *description;        /**< Operation description */
  char *info_title;         /**< Global info.title override */
  char *info_version;       /**< Global info.version override */
  char *info_summary;       /**< Global info.summary override */
  char *info_description;   /**< Global info.description override */
  char *terms_of_service;   /**< Global info.termsOfService override */
  char *contact_name;       /**< Global info.contact.name override */
  char *contact_url;        /**< Global info.contact.url override */
  char *contact_email;      /**< Global info.contact.email override */
  char *license_name;       /**< Global info.license.name override */
  char *license_identifier; /**< Global info.license.identifier override */
  char *license_url;        /**< Global info.license.url override */
  int deprecated;           /**< Deprecated flag */
  int deprecated_set;       /**< 1 if deprecated explicitly set */
  char **tags;              /**< Operation tags */
  size_t n_tags;            /**< Number of tags */
  char *external_docs_url;  /**< externalDocs URL */
  char *external_docs_description; /**< externalDocs description */

  struct DocParam *params; /**< Array of parameters */
  size_t n_params;         /**< Number of parameters */

  struct DocResponse *returns; /**< Array of responses */
  size_t n_returns;            /**< Number of responses */

  struct DocResponseHeader *response_headers; /**< Response header entries */
  size_t n_response_headers;                  /**< Response header count */

  struct DocLink *links; /**< Response links */
  size_t n_links;        /**< Response link count */

  struct DocSecurityRequirement *security; /**< Security requirements */
  size_t n_security;                       /**< Security requirement count */

  struct DocSecurityScheme *security_schemes; /**< Security scheme defs */
  size_t n_security_schemes;                  /**< Security scheme count */

  struct DocServer *servers; /**< Per-operation servers */
  size_t n_servers;          /**< Server count */

  struct DocRequestBody *request_bodies; /**< Request body entries */
  size_t n_request_bodies;               /**< Request body entry count */

  struct DocTagMeta *tag_meta; /**< Tag metadata entries */
  size_t n_tag_meta;           /**< Tag metadata entry count */

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
