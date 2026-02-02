/**
 * @file openapi_loader.h
 * @brief Parser for OpenAPI v3.0 definitions.
 *
 * Provides functionalities to load an OpenAPI JSON specification into memory
 * structures. Supports:
 * - Paths and Operations
 * - Parameters (serialization styles, explode)
 * - Request Bodies and content-types
 * - Component Schemas (parsed into StructFields for generation lookups)
 * - Security Schemes
 * - Tags for resource grouping
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_OPENAPI_LOADER_H
#define C_CDD_OPENAPI_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <parson.h>
#include <stddef.h>

#include "c_cdd_export.h"
#include "codegen_struct.h" /* For StructFields definition */

/**
 * @brief HTTP Verbs supported by the generator.
 */
enum OpenAPI_Verb {
  OA_VERB_GET,    /**< GET method */
  OA_VERB_POST,   /**< POST method */
  OA_VERB_PUT,    /**< PUT method */
  OA_VERB_DELETE, /**< DELETE method */
  OA_VERB_PATCH,  /**< PATCH method */
  OA_VERB_HEAD,   /**< HEAD method */
  OA_VERB_UNKNOWN /**< Fallback/Unsupported method */
};

/**
 * @brief Location of a parameter in the HTTP request.
 */
enum OpenAPI_ParamIn {
  OA_PARAM_IN_PATH,   /**< Path segment: /resource/{id} */
  OA_PARAM_IN_QUERY,  /**< Query string: /resource?id=1 */
  OA_PARAM_IN_HEADER, /**< Header: X-Header: 1 */
  OA_PARAM_IN_COOKIE, /**< Cookie: id=1 */
  OA_PARAM_IN_UNKNOWN /**< Error state or unsupported location */
};

/**
 * @brief Serialization style for parameters (per RFC 6570 / OpenAPI Spec).
 */
enum OpenAPI_Style {
  OA_STYLE_FORM,            /**< name=value (Default for Query) */
  OA_STYLE_SIMPLE,          /**< value (Default for Path/Header) */
  OA_STYLE_SPACE_DELIMITED, /**< name=val1%20val2 */
  OA_STYLE_PIPE_DELIMITED,  /**< name=val1|val2 */
  OA_STYLE_DEEP_OBJECT,     /**< nested object serialization */
  OA_STYLE_UNKNOWN          /**< Unknown style */
};

/**
 * @brief OpenAPI Security Scheme Type.
 */
enum OpenAPI_SecurityType {
  OA_SEC_APIKEY, /**< type: apiKey */
  OA_SEC_HTTP,   /**< type: http (Basic, Bearer) */
  OA_SEC_OAUTH2, /**< type: oauth2 */
  OA_SEC_OPENID, /**< type: openIdConnect */
  OA_SEC_UNKNOWN /**< parsing error / unsupported */
};

/**
 * @brief Location of API Key security param.
 */
enum OpenAPI_SecurityIn {
  OA_SEC_IN_QUERY,  /**< in: query */
  OA_SEC_IN_HEADER, /**< in: header */
  OA_SEC_IN_COOKIE, /**< in: cookie */
  OA_SEC_IN_UNKNOWN /**< unknown location */
};

/**
 * @brief Represents a field in a multipart request.
 */
struct OpenAPI_MultipartField {
  char *name;    /**< Field name */
  char *type;    /**< Type ("string", "integer") or NULL for file */
  int is_binary; /**< 1 if this is a file upload, 0 otherwise */
};

/**
 * @brief Represents an extracted Schema (Body or Response).
 */
struct OpenAPI_SchemaRef {
  char *ref_name; /**< The cleaned type name (e.g. "Pet") or NULL if
                     inline/primitive */
  int is_array;   /**< 1 if array, 0 otherwise */

  /* Multipart / Form-urlencoded Extension */
  char *content_type; /**< "application/json" or
                         "application/x-www-form-urlencoded" */
  struct OpenAPI_MultipartField
      *multipart_fields;     /**< Array of fields (if multipart) */
  size_t n_multipart_fields; /**< Count of multipart fields */
};

/**
 * @brief Represents a single operation parameter (e.g. "petId").
 */
struct OpenAPI_Parameter {
  char *name;              /**< Variable name */
  enum OpenAPI_ParamIn in; /**< Location */
  int required;            /**< 1 if mandatory, 0 optional */
  char *type;              /**< Basic type ("integer", "string", "array") */

  /* Array Support */
  int is_array;     /**< 1 if this parameter is an array */
  char *items_type; /**< If array, the type of items (e.g. "integer") */

  /* Serialization settings */
  enum OpenAPI_Style style; /**< Serialization style */
  int explode;              /**< 1 if explode=true */
};

/**
 * @brief Represents a defined Response (Success or Error).
 */
struct OpenAPI_Response {
  char *code; /**< HTTP Status string ("200", "404", "default") */
  struct OpenAPI_SchemaRef schema; /**< The body schema definition */
};

/**
 * @brief Represents a Security Scheme definition (Component).
 */
struct OpenAPI_SecurityScheme {
  char *name;                     /**< Scheme identifier (key in dictionary) */
  enum OpenAPI_SecurityType type; /**< Type of security */
  char *scheme;                   /**< HTTP scheme (e.g. "bearer", "basic") */
  char *key_name;                 /**< Parameter name (for apiKey) */
  enum OpenAPI_SecurityIn in;     /**< Location (for apiKey) */
};

/**
 * @brief Represents a single HTTP operation (endpoint).
 */
struct OpenAPI_Operation {
  enum OpenAPI_Verb verb; /**< HTTP Method */
  char *operation_id;     /**< The unique ID used for C function naming */

  struct OpenAPI_Parameter *parameters; /**< Array of parameters */
  size_t n_parameters;                  /**< Param count */

  char **tags;   /**< Array of tags used for grouping (e.g. ["pet"]) */
  size_t n_tags; /**< Number of tags */

  struct OpenAPI_SchemaRef req_body; /**< Request body definition */

  /* Response Handling */
  struct OpenAPI_Response *responses; /**< Listing of all defined responses */
  size_t n_responses;                 /**< Count of responses */
};

/**
 * @brief Represents a URL Path template.
 */
struct OpenAPI_Path {
  char *route; /* The route string (e.g. "/pets/{petId}") */

  struct OpenAPI_Operation *operations; /**< Array of operations on this path */
  size_t n_operations;                  /**< Operation count */
};

/**
 * @brief Root container for the parsed specification.
 */
struct OpenAPI_Spec {
  struct OpenAPI_Path *paths; /**< Array of paths */
  size_t n_paths;             /**< Path count */

  struct OpenAPI_SecurityScheme
      *security_schemes;     /**< Defined security components */
  size_t n_security_schemes; /**< Count of schemes */

  /* Global Schema Data (for looking up struct definitions during body gen) */
  struct StructFields
      *defined_schemas;        /**< Array of parsed schemas structure */
  char **defined_schema_names; /**< Array of schema names matching indices */
  size_t n_defined_schemas;    /**< Count of defined schemas */
};

/* --- Lifecycle --- */

/**
 * @brief Initialize a Spec structure to zero.
 * @param[out] spec Pointer to spec structure to initialize.
 */
extern C_CDD_EXPORT void openapi_spec_init(struct OpenAPI_Spec *spec);

/**
 * @brief Free a Spec structure and all nested allocations.
 * @param[in] spec Pointer to spec structure to free.
 */
extern C_CDD_EXPORT void openapi_spec_free(struct OpenAPI_Spec *spec);

/* --- Loader --- */

/**
 * @brief Parse "paths" and "components" from a JSON Value.
 *
 * Traverses the JSON to populate the Spec structure.
 * 1. Extracts Paths, Operations, Params, Bodies, and Tags.
 * 2. Extracts Security Schemes from components.
 * 3. Extracts and flattening Schemas definitions.
 *
 * @param[in] root The root JSON Value of the OpenAPI file.
 * @param[out] out Destination structure to populate.
 * @return 0 on success, error code (EINVAL/ENOMEM) on failure.
 */
extern C_CDD_EXPORT int openapi_load_from_json(const JSON_Value *root,
                                               struct OpenAPI_Spec *out);

/**
 * @brief Look up a schema definition by name in the loaded spec.
 *
 * @param[in] spec The spec to search.
 * @param[in] name The schema name (e.g. "LoginRequest").
 * @return Pointer to StructFields if found, NULL otherwise.
 */
extern C_CDD_EXPORT const struct StructFields *
openapi_spec_find_schema(const struct OpenAPI_Spec *spec, const char *name);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_OPENAPI_LOADER_H */
