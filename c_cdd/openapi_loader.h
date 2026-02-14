/**
 * @file openapi_loader.h
 * @brief Parser for OpenAPI v3.2 definitions.
 *
 * Provides functionalities to load an OpenAPI JSON specification into memory
 * structures. Supports:
 * - Paths and Operations
 * - Parameters (serialization styles, explode, allowEmptyValue, content)
 * - Request Bodies and content-types
 * - Response descriptions
 * - Component Schemas (parsed into StructFields for generation lookups)
 * - Security Schemes
 * - Root-level Servers
 * - Tags for resource grouping
 * - Component Path Items and Media Types (OAS 3.2)
 * - Path Item additionalOperations (OAS 3.2)
 * - Content-level $ref for Media Type Objects (OAS 3.2)
 * - Response Links and Callback Objects (OAS 3.2)
 * - components.links and components.callbacks (OAS 3.2)
 * - Example Objects and components.examples (OAS 3.2)
 * - OAuth2 flows in components.securitySchemes (OAS 3.2)
 * - Media Type Encoding by name and position: `encoding`, `prefixEncoding`,
 *   and `itemEncoding` (OAS 3.2)
 * - Parameter/Header content Media Type Objects (encoding/examples/schema)
 * - x- extensions on Paths, Webhooks, and Components objects
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

struct OpenAPI_Path;
struct OpenAPI_Server;
struct OpenAPI_Header;
struct OpenAPI_DocRegistry;

/**
 * @brief HTTP Verbs supported by the generator.
 */
enum OpenAPI_Verb {
  OA_VERB_GET,     /**< GET method */
  OA_VERB_POST,    /**< POST method */
  OA_VERB_PUT,     /**< PUT method */
  OA_VERB_DELETE,  /**< DELETE method */
  OA_VERB_PATCH,   /**< PATCH method */
  OA_VERB_HEAD,    /**< HEAD method */
  OA_VERB_OPTIONS, /**< OPTIONS method */
  OA_VERB_TRACE,   /**< TRACE method */
  OA_VERB_QUERY,   /**< QUERY method (OAS 3.2) */
  OA_VERB_UNKNOWN  /**< Fallback/Unsupported method */
};

/**
 * @brief Location of a parameter in the HTTP request.
 */
enum OpenAPI_ParamIn {
  OA_PARAM_IN_PATH,        /**< Path segment: /resource/{id} */
  OA_PARAM_IN_QUERY,       /**< Query string: /resource?id=1 */
  OA_PARAM_IN_QUERYSTRING, /**< Query string as a single value (OAS 3.2) */
  OA_PARAM_IN_HEADER,      /**< Header: X-Header: 1 */
  OA_PARAM_IN_COOKIE,      /**< Cookie: id=1 */
  OA_PARAM_IN_UNKNOWN      /**< Error state or unsupported location */
};

/**
 * @brief Serialization style for parameters (per RFC 6570 / OpenAPI Spec).
 */
enum OpenAPI_Style {
  OA_STYLE_FORM,            /**< name=value (Default for Query) */
  OA_STYLE_SIMPLE,          /**< value (Default for Path/Header) */
  OA_STYLE_MATRIX,          /**< ;name=value (Path-style) */
  OA_STYLE_LABEL,           /**< .value (Label-style) */
  OA_STYLE_SPACE_DELIMITED, /**< name=val1%20val2 */
  OA_STYLE_PIPE_DELIMITED,  /**< name=val1|val2 */
  OA_STYLE_DEEP_OBJECT,     /**< nested object serialization */
  OA_STYLE_COOKIE,          /**< cookie-style serialization */
  OA_STYLE_UNKNOWN          /**< Unknown style */
};

/**
 * @brief OpenAPI Security Scheme Type.
 */
enum OpenAPI_SecurityType {
  OA_SEC_APIKEY,    /**< type: apiKey */
  OA_SEC_HTTP,      /**< type: http (Basic, Bearer) */
  OA_SEC_MUTUALTLS, /**< type: mutualTLS */
  OA_SEC_OAUTH2,    /**< type: oauth2 */
  OA_SEC_OPENID,    /**< type: openIdConnect */
  OA_SEC_UNKNOWN    /**< parsing error / unsupported */
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
 * @brief Represents a JSON-compatible "any" value used by OpenAPI fields.
 */
enum OpenAPI_AnyType {
  OA_ANY_UNSET = 0,
  OA_ANY_NULL,
  OA_ANY_STRING,
  OA_ANY_NUMBER,
  OA_ANY_BOOL,
  OA_ANY_JSON /**< Serialized JSON for objects/arrays */
};

/**
 * @brief Represents a JSON-compatible value.
 */
struct OpenAPI_Any {
  enum OpenAPI_AnyType type; /**< Value type discriminator */
  char *string;              /**< String value (OA_ANY_STRING) */
  double number;             /**< Number value (OA_ANY_NUMBER) */
  int boolean;               /**< Boolean value (OA_ANY_BOOL) */
  char *json;                /**< Serialized JSON (OA_ANY_JSON) */
};

/**
 * @brief Indicates where Example Objects should be emitted.
 */
enum OpenAPI_ExampleLocation {
  OA_EXAMPLE_LOC_OBJECT = 0, /**< Write example(s) on the Parameter/Header */
  OA_EXAMPLE_LOC_MEDIA       /**< Write example(s) under Media Type content */
};

/**
 * @brief Represents an Example Object (or Reference Object via $ref).
 */
struct OpenAPI_Example {
  char *name;            /**< Example map key (when applicable) */
  char *ref;             /**< Optional $ref to components.examples */
  char *summary;         /**< Optional summary */
  char *description;     /**< Optional description */
  char *extensions_json; /**< Serialized JSON for x- extensions */

  struct OpenAPI_Any data_value; /**< Example dataValue */
  int data_value_set;            /**< 1 if dataValue present */

  struct OpenAPI_Any value; /**< Deprecated value field */
  int value_set;            /**< 1 if value present */

  char *serialized_value; /**< Example serializedValue */
  char *external_value;   /**< Example externalValue */
};

/**
 * @brief OAuth2 Flow Types.
 */
enum OpenAPI_OAuthFlowType {
  OA_OAUTH_FLOW_IMPLICIT,
  OA_OAUTH_FLOW_PASSWORD,
  OA_OAUTH_FLOW_CLIENT_CREDENTIALS,
  OA_OAUTH_FLOW_AUTHORIZATION_CODE,
  OA_OAUTH_FLOW_DEVICE_AUTHORIZATION,
  OA_OAUTH_FLOW_UNKNOWN
};

/**
 * @brief OAuth2 scope name/description.
 */
struct OpenAPI_OAuthScope {
  char *name;        /**< Scope name */
  char *description; /**< Scope description */
};

/**
 * @brief OAuth2 Flow definition.
 */
struct OpenAPI_OAuthFlow {
  enum OpenAPI_OAuthFlowType type;   /**< Flow type */
  char *authorization_url;           /**< authorizationUrl */
  char *token_url;                   /**< tokenUrl */
  char *refresh_url;                 /**< refreshUrl */
  char *device_authorization_url;    /**< deviceAuthorizationUrl */
  struct OpenAPI_OAuthScope *scopes; /**< Scopes map */
  size_t n_scopes;                   /**< Scope count */
  char *extensions_json;             /**< Serialized JSON for x- extensions */
};

/**
 * @brief Represents a Link parameter entry.
 */
struct OpenAPI_LinkParam {
  char *name;               /**< Parameter name */
  struct OpenAPI_Any value; /**< Parameter value (literal or expression) */
};

/**
 * @brief Represents a Link Object (or Reference Object via $ref).
 */
struct OpenAPI_Link {
  char *name;            /**< Link name (map key) */
  char *ref;             /**< Optional $ref to a Link component */
  char *summary;         /**< Optional summary (Reference Object) */
  char *description;     /**< Optional description */
  char *extensions_json; /**< Serialized JSON for x- extensions */
  char *operation_ref;   /**< operationRef */
  char *operation_id;    /**< operationId */
  struct OpenAPI_LinkParam *parameters; /**< Parameters map */
  size_t n_parameters;                  /**< Parameters count */
  struct OpenAPI_Any request_body;      /**< Optional requestBody */
  int request_body_set;                 /**< 1 if requestBody present */
  struct OpenAPI_Server *server;        /**< Optional server override */
  int server_set;                       /**< 1 if server present */
};

/**
 * @brief Represents a Callback Object (or Reference Object via $ref).
 */
struct OpenAPI_Callback {
  char *name;                 /**< Callback name (map key) */
  char *ref;                  /**< Optional $ref to a Callback component */
  char *summary;              /**< Optional summary (Reference Object) */
  char *description;          /**< Optional description (Reference Object) */
  char *extensions_json;      /**< Serialized JSON for x- extensions */
  struct OpenAPI_Path *paths; /**< Callback expressions to Path Item Objects */
  size_t n_paths;             /**< Callback expressions count */
};

/**
 * @brief Represents a single security requirement entry.
 *
 * A Security Requirement Object maps a scheme name to a list of scopes.
 */
struct OpenAPI_SecurityRequirement {
  char *scheme;    /**< Security scheme name (or URI) */
  char **scopes;   /**< Array of scope strings */
  size_t n_scopes; /**< Number of scopes */
};

/**
 * @brief Represents one Security Requirement Object (AND across schemes).
 *
 * OpenAPI `security` is an array of these objects, where the array is OR.
 */
struct OpenAPI_SecurityRequirementSet {
  struct OpenAPI_SecurityRequirement
      *requirements;     /**< Array of requirements */
  size_t n_requirements; /**< Count */
  char *extensions_json; /**< Serialized JSON for x- extensions */
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
 * @brief Discriminator mapping entry (payload value -> schema ref/name).
 */
struct OpenAPI_DiscriminatorMap {
  char *value;  /**< Discriminator payload value */
  char *schema; /**< Schema name or URI reference */
};

/**
 * @brief Discriminator Object metadata.
 */
struct OpenAPI_Discriminator {
  char *property_name;                      /**< propertyName */
  struct OpenAPI_DiscriminatorMap *mapping; /**< mapping entries */
  size_t n_mapping;                         /**< mapping count */
  char *default_mapping;                    /**< defaultMapping */
  char *extensions_json; /**< Serialized JSON for x- extensions */
};

/**
 * @brief XML node type.
 */
enum OpenAPI_XmlNodeType {
  OA_XML_NODE_UNSET = 0,
  OA_XML_NODE_ELEMENT,
  OA_XML_NODE_ATTRIBUTE,
  OA_XML_NODE_TEXT,
  OA_XML_NODE_CDATA,
  OA_XML_NODE_NONE
};

/**
 * @brief XML Object metadata.
 */
struct OpenAPI_Xml {
  enum OpenAPI_XmlNodeType node_type; /**< nodeType */
  int node_type_set;                  /**< 1 if nodeType present */
  char *name;                         /**< name */
  char *namespace_uri;                /**< namespace */
  char *prefix;                       /**< prefix */
  int attribute;                      /**< attribute (deprecated) */
  int attribute_set;                  /**< 1 if attribute present */
  int wrapped;                        /**< wrapped (deprecated) */
  int wrapped_set;                    /**< 1 if wrapped present */
  char *extensions_json;              /**< Serialized JSON for x- extensions */
};

/**
 * @brief Represents an extracted Schema (Body or Response).
 */
struct OpenAPI_SchemaRef {
  int schema_is_boolean;    /**< 1 if schema is boolean (true/false) */
  int schema_boolean_value; /**< Boolean schema value when schema_is_boolean */
  char *ref_name;     /**< Component schema name when $ref targets components */
  char *ref;          /**< Raw $ref/$dynamicRef value for non-component refs */
  int ref_is_dynamic; /**< 1 if ref uses $dynamicRef */
  char *inline_type;  /**< Inline primitive type (e.g. "string"), if present */
  char **type_union;  /**< Full type array when schema uses type:[...] */
  size_t n_type_union;     /**< Count of type array entries */
  char *format;            /**< Optional schema format for inline primitives */
  int is_array;            /**< 1 if array, 0 otherwise */
  char **items_type_union; /**< Items type array when items uses type:[...] */
  size_t n_items_type_union; /**< Count of items type entries */
  char *items_format;        /**< Optional format for array items */
  char *items_ref;           /**< Raw $ref/$dynamicRef for array items */
  int items_ref_is_dynamic;  /**< 1 if items_ref uses $dynamicRef */

  /* Multipart / Form-urlencoded Extension */
  char *content_type;             /**< "application/json" or
                                     "application/x-www-form-urlencoded" */
  char *content_media_type;       /**< Schema contentMediaType */
  char *content_encoding;         /**< Schema contentEncoding */
  char *items_content_media_type; /**< Array item contentMediaType */
  char *items_content_encoding;   /**< Array item contentEncoding */
  int nullable;                   /**< 1 if type includes null (type array) */
  int items_nullable;             /**< 1 if items type includes null */
  char *summary;      /**< Reference Object summary (valid only with $ref) */
  char *description;  /**< Schema description or Reference Object description */
  int deprecated;     /**< 1 if deprecated=true */
  int deprecated_set; /**< 1 if deprecated explicitly set */
  int read_only;      /**< 1 if readOnly=true */
  int read_only_set;  /**< 1 if readOnly explicitly set */
  int write_only;     /**< 1 if writeOnly=true */
  int write_only_set; /**< 1 if writeOnly explicitly set */
  struct OpenAPI_Any const_value;   /**< Schema const value */
  int const_value_set;              /**< 1 if const present */
  struct OpenAPI_Any *examples;     /**< Schema examples array */
  size_t n_examples;                /**< Example count */
  struct OpenAPI_Any example;       /**< Schema example (deprecated) */
  int example_set;                  /**< 1 if example present */
  struct OpenAPI_Any default_value; /**< Schema default value */
  int default_value_set;            /**< 1 if default present */
  struct OpenAPI_Any *enum_values;  /**< Enum values (any JSON type) */
  size_t n_enum_values;             /**< Enum value count */
  char *schema_extra_json; /**< Serialized JSON for extra schema keywords */
  struct OpenAPI_ExternalDocs external_docs;  /**< Schema externalDocs */
  int external_docs_set;                      /**< 1 if externalDocs present */
  struct OpenAPI_Discriminator discriminator; /**< Schema discriminator */
  int discriminator_set;                      /**< 1 if discriminator present */
  struct OpenAPI_Xml xml;                     /**< Schema xml metadata */
  int xml_set;                                /**< 1 if xml present */
  struct OpenAPI_Any *items_enum_values; /**< Enum values for array items */
  size_t n_items_enum_values;            /**< Items enum count */
  int has_min;             /**< 1 if minimum or exclusiveMinimum present */
  double min_val;          /**< Minimum value */
  int exclusive_min;       /**< 1 if exclusive minimum */
  int has_max;             /**< 1 if maximum or exclusiveMaximum present */
  double max_val;          /**< Maximum value */
  int exclusive_max;       /**< 1 if exclusive maximum */
  int has_min_len;         /**< 1 if minLength present */
  size_t min_len;          /**< Minimum length */
  int has_max_len;         /**< 1 if maxLength present */
  size_t max_len;          /**< Maximum length */
  char *pattern;           /**< Regex pattern */
  int has_min_items;       /**< 1 if minItems present */
  size_t min_items;        /**< Minimum items */
  int has_max_items;       /**< 1 if maxItems present */
  size_t max_items;        /**< Maximum items */
  int unique_items;        /**< 1 if uniqueItems true */
  int items_has_min;       /**< 1 if items minimum/exclusiveMinimum present */
  double items_min_val;    /**< Items minimum value */
  int items_exclusive_min; /**< 1 if items exclusive minimum */
  int items_has_max;       /**< 1 if items maximum/exclusiveMaximum present */
  double items_max_val;    /**< Items maximum value */
  int items_exclusive_max; /**< 1 if items exclusive maximum */
  int items_has_min_len;   /**< 1 if items minLength present */
  size_t items_min_len;    /**< Items minimum length */
  int items_has_max_len;   /**< 1 if items maxLength present */
  size_t items_max_len;    /**< Items maximum length */
  char *items_pattern;     /**< Items regex pattern */
  int items_has_min_items; /**< 1 if items minItems present */
  size_t items_min_items;  /**< Items minimum items */
  int items_has_max_items; /**< 1 if items maxItems present */
  size_t items_max_items;  /**< Items maximum items */
  int items_unique_items;  /**< 1 if items uniqueItems true */
  struct OpenAPI_Any items_example;       /**< Items example (deprecated) */
  int items_example_set;                  /**< 1 if items example present */
  struct OpenAPI_Any *items_examples;     /**< Items examples array */
  size_t n_items_examples;                /**< Items examples count */
  struct OpenAPI_Any items_const_value;   /**< Items const value */
  int items_const_value_set;              /**< 1 if items const present */
  struct OpenAPI_Any items_default_value; /**< Items default value */
  int items_default_value_set;            /**< 1 if items default present */
  char *items_extra_json;      /**< Serialized JSON for array items keywords */
  int items_schema_is_boolean; /**< 1 if items schema is boolean */
  int items_schema_boolean_value; /**< Items schema boolean value */
  struct OpenAPI_MultipartField
      *multipart_fields;     /**< Array of fields (if multipart) */
  size_t n_multipart_fields; /**< Count of multipart fields */
};

/**
 * @brief Represents an Encoding Object for multipart/form or urlencoded
 * content.
 */
struct OpenAPI_Encoding {
  char *name;                     /**< Encoding map key (property name) */
  char *content_type;             /**< Optional contentType override */
  enum OpenAPI_Style style;       /**< Optional RFC6570-style serialization */
  int style_set;                  /**< 1 if style explicitly set */
  int explode;                    /**< 1 if explode explicitly set */
  int explode_set;                /**< 1 if explode explicitly set */
  int allow_reserved;             /**< 1 if allowReserved explicitly set */
  int allow_reserved_set;         /**< 1 if allowReserved explicitly set */
  char *extensions_json;          /**< Serialized JSON for x- extensions */
  struct OpenAPI_Header *headers; /**< Optional per-part headers */
  size_t n_headers;               /**< Header count */
  struct OpenAPI_Encoding *encoding;        /**< Nested encoding map */
  size_t n_encoding;                        /**< Nested encoding count */
  struct OpenAPI_Encoding *prefix_encoding; /**< Positional encodings */
  size_t n_prefix_encoding;                 /**< Positional encoding count */
  struct OpenAPI_Encoding *item_encoding;   /**< Item encoding (streaming) */
  int item_encoding_set;                    /**< 1 if itemEncoding present */
};

/**
 * @brief Represents a reusable Media Type Object (components.mediaTypes).
 */
struct OpenAPI_MediaType {
  char *name;            /**< Media type key (e.g. "application/json") */
  char *ref;             /**< Optional $ref to a Media Type Object */
  char *extensions_json; /**< Serialized JSON for x- extensions */
  struct OpenAPI_SchemaRef schema;      /**< Full-schema for complete content */
  int schema_set;                       /**< 1 if schema field was present */
  struct OpenAPI_SchemaRef item_schema; /**< Schema for sequential items */
  int item_schema_set;               /**< 1 if itemSchema field was present */
  struct OpenAPI_Any example;        /**< Shorthand example */
  int example_set;                   /**< 1 if example present */
  struct OpenAPI_Example *examples;  /**< Example Objects */
  size_t n_examples;                 /**< Example count */
  struct OpenAPI_Encoding *encoding; /**< Encoding map (form/multipart) */
  size_t n_encoding;                 /**< Encoding count */
  struct OpenAPI_Encoding *prefix_encoding; /**< Positional encodings */
  size_t n_prefix_encoding;                 /**< Positional encoding count */
  struct OpenAPI_Encoding *item_encoding;   /**< Item encoding (streaming) */
  int item_encoding_set;                    /**< 1 if itemEncoding present */
};

/**
 * @brief Represents a single operation parameter (e.g. "petId").
 */
struct OpenAPI_Parameter {
  char *name;              /**< Variable name */
  enum OpenAPI_ParamIn in; /**< Location */
  int required;            /**< 1 if mandatory, 0 optional */
  char *type;              /**< Basic type ("integer", "string", "array") */
  char *description;       /**< Optional description */
  char *extensions_json;   /**< Serialized JSON for x- extensions */
  char *ref;               /**< Optional $ref to a parameter component */
  int deprecated;          /**< 1 if deprecated */
  int deprecated_set;      /**< 1 if deprecated was explicitly set */
  char *content_type;      /**< Media type when parameter uses content */
  char *content_ref;       /**< Optional $ref to a Media Type Object */
  struct OpenAPI_MediaType *content_media_types; /**< Full content map */
  size_t n_content_media_types;                  /**< Content entry count */
  struct OpenAPI_SchemaRef schema; /**< Inline schema when present */
  int schema_set;                  /**< 1 if schema object was present */

  /* Array Support */
  int is_array;     /**< 1 if this parameter is an array */
  char *items_type; /**< If array, the type of items (e.g. "integer") */

  /* Serialization settings */
  enum OpenAPI_Style style;  /**< Serialization style */
  int explode;               /**< 1 if explode=true */
  int explode_set;           /**< 1 if explode explicitly set */
  int allow_reserved;        /**< 1 if allowReserved=true */
  int allow_reserved_set;    /**< 1 if allowReserved was explicitly set */
  int allow_empty_value;     /**< 1 if allowEmptyValue=true */
  int allow_empty_value_set; /**< 1 if allowEmptyValue explicitly set */

  struct OpenAPI_Any example;                    /**< Shorthand example */
  int example_set;                               /**< 1 if example present */
  struct OpenAPI_Example *examples;              /**< Example Objects */
  size_t n_examples;                             /**< Example count */
  enum OpenAPI_ExampleLocation example_location; /**< Output location */
};

/**
 * @brief Represents a response/header Header Object.
 */
struct OpenAPI_Header {
  char *name;            /**< Header name (map key) */
  char *ref;             /**< Optional $ref to a header component */
  char *description;     /**< Optional description */
  int required;          /**< 1 if required */
  int deprecated;        /**< 1 if deprecated */
  int deprecated_set;    /**< 1 if deprecated was explicitly set */
  char *extensions_json; /**< Serialized JSON for x- extensions */
  char *content_type;    /**< Media type when header uses content */
  char *content_ref;     /**< Optional $ref to a Media Type Object */
  struct OpenAPI_MediaType *content_media_types; /**< Full content map */
  size_t n_content_media_types;                  /**< Content entry count */
  struct OpenAPI_SchemaRef schema; /**< Inline schema when present */
  int schema_set;                  /**< 1 if schema object was present */
  char *type;                      /**< Schema type */
  int is_array;                    /**< 1 if array */
  char *items_type;                /**< Array item type */
  enum OpenAPI_Style
      style;       /**< Serialization style (header only allows simple) */
  int style_set;   /**< 1 if style explicitly set */
  int explode;     /**< 1 if explode */
  int explode_set; /**< 1 if explode explicitly set */

  struct OpenAPI_Any example;                    /**< Shorthand example */
  int example_set;                               /**< 1 if example present */
  struct OpenAPI_Example *examples;              /**< Example Objects */
  size_t n_examples;                             /**< Example count */
  enum OpenAPI_ExampleLocation example_location; /**< Output location */
};

/**
 * @brief Represents a defined Response (Success or Error).
 */
struct OpenAPI_Response {
  char *code;            /**< HTTP Status string ("200", "404", "default") */
  char *summary;         /**< Optional response summary */
  char *description;     /**< Human-readable description */
  char *content_type;    /**< Response media type (e.g. "application/json") */
  char *content_ref;     /**< Optional $ref to a Media Type Object */
  char *extensions_json; /**< Serialized JSON for x- extensions */
  struct OpenAPI_MediaType *content_media_types; /**< Full content map */
  size_t n_content_media_types;                  /**< Content entry count */
  struct OpenAPI_SchemaRef schema; /**< The body schema definition */
  struct OpenAPI_Header *headers;  /**< Response headers */
  size_t n_headers;                /**< Header count */
  struct OpenAPI_Link *links;      /**< Response links */
  size_t n_links;                  /**< Link count */
  char *ref;                       /**< Optional $ref to a response component */

  struct OpenAPI_Any example; /**< Shorthand example for response content */
  int example_set;            /**< 1 if example present */
  struct OpenAPI_Example *examples; /**< Example Objects for response content */
  size_t n_examples;                /**< Example count */
};

/**
 * @brief Represents a Request Body Object (operation or component).
 */
struct OpenAPI_RequestBody {
  char *ref;             /**< Optional $ref to a requestBody component */
  char *description;     /**< Optional description */
  int required;          /**< 1 if requestBody.required = true */
  int required_set;      /**< 1 if required was explicitly set */
  char *extensions_json; /**< Serialized JSON for x- extensions */
  char *content_ref;     /**< Optional $ref to a Media Type Object */
  struct OpenAPI_MediaType *content_media_types; /**< Full content map */
  size_t n_content_media_types;                  /**< Content entry count */
  struct OpenAPI_SchemaRef schema; /**< The body schema definition */
  struct OpenAPI_Any example;      /**< Shorthand example for request content */
  int example_set;                 /**< 1 if example present */
  struct OpenAPI_Example *examples; /**< Example Objects for request content */
  size_t n_examples;                /**< Example count */
};

/**
 * @brief Represents a Security Scheme definition (Component).
 */
struct OpenAPI_SecurityScheme {
  char *name;                      /**< Scheme identifier (key in dictionary) */
  enum OpenAPI_SecurityType type;  /**< Type of security */
  char *description;               /**< Optional description */
  int deprecated;                  /**< 1 if deprecated */
  int deprecated_set;              /**< 1 if deprecated explicitly set */
  char *extensions_json;           /**< Serialized JSON for x- extensions */
  char *scheme;                    /**< HTTP scheme (e.g. "bearer", "basic") */
  char *bearer_format;             /**< Optional bearerFormat */
  char *key_name;                  /**< Parameter name (for apiKey) */
  enum OpenAPI_SecurityIn in;      /**< Location (for apiKey) */
  char *open_id_connect_url;       /**< openIdConnectUrl, if provided */
  char *oauth2_metadata_url;       /**< oauth2MetadataUrl, if provided */
  struct OpenAPI_OAuthFlow *flows; /**< OAuth2 flows */
  size_t n_flows;                  /**< OAuth2 flow count */
};

/**
 * @brief External documentation reference.
 */
struct OpenAPI_ExternalDocs {
  char *description;     /**< Optional description */
  char *url;             /**< REQUIRED URL */
  char *extensions_json; /**< Serialized JSON for x- extensions */
};

/**
 * @brief Tag metadata (Top-level Tag Object).
 */
struct OpenAPI_Tag {
  char *name;            /**< Tag name */
  char *summary;         /**< Optional summary */
  char *description;     /**< Optional description */
  char *parent;          /**< Optional parent tag name */
  char *kind;            /**< Optional kind value */
  char *extensions_json; /**< Serialized JSON for x- extensions */
  struct OpenAPI_ExternalDocs external_docs; /**< Optional external docs */
};

/**
 * @brief Represents a single HTTP operation (endpoint).
 */
struct OpenAPI_Operation {
  enum OpenAPI_Verb verb; /**< HTTP Method */
  char *method;           /**< Raw HTTP method string (for additional ops) */
  int is_additional;  /**< 1 if sourced from Path Item additionalOperations */
  char *operation_id; /**< The unique ID used for C function naming */
  char *summary;      /**< Short summary of operation */
  char *description;  /**< Long description of operation */
  int deprecated;     /**< 1 if deprecated=true */
  char *extensions_json; /**< Serialized JSON for x- extensions */

  struct OpenAPI_SecurityRequirementSet *security; /**< Op-level security */
  size_t n_security;                               /**< Security count */
  int security_set;                                /**< 1 if field present */

  struct OpenAPI_Parameter *parameters; /**< Array of parameters */
  size_t n_parameters;                  /**< Param count */

  char **tags;   /**< Array of tags used for grouping (e.g. ["pet"]) */
  size_t n_tags; /**< Number of tags */

  struct OpenAPI_SchemaRef req_body; /**< Request body definition */
  struct OpenAPI_MediaType
      *req_body_media_types;      /**< RequestBody content map */
  size_t n_req_body_media_types;  /**< RequestBody content count */
  int req_body_required;          /**< 1 if requestBody.required = true */
  int req_body_required_set;      /**< 1 if required was explicitly set */
  char *req_body_description;     /**< Optional request body description */
  char *req_body_extensions_json; /**< Serialized JSON for requestBody x- */
  char *req_body_ref; /**< Optional $ref to components.requestBodies */
  struct OpenAPI_ExternalDocs external_docs; /**< Optional external docs */

  struct OpenAPI_Server *servers; /**< Operation-level servers */
  size_t n_servers;               /**< Operation-level server count */

  /* Response Handling */
  struct OpenAPI_Response *responses; /**< Listing of all defined responses */
  size_t n_responses;                 /**< Count of responses */
  char *responses_extensions_json;    /**< x- extensions on Responses Object */

  struct OpenAPI_Callback *callbacks; /**< Callback definitions */
  size_t n_callbacks;                 /**< Callback count */
};

/**
 * @brief Represents a URL Path template.
 */
struct OpenAPI_Path {
  char *route;           /* The route string (e.g. "/pets/{petId}") */
  char *ref;             /**< Optional $ref to a Path Item Object */
  char *summary;         /**< Optional path summary */
  char *description;     /**< Optional path description */
  char *extensions_json; /**< Serialized JSON for x- extensions */
  struct OpenAPI_Parameter *parameters; /**< Path-level parameters */
  size_t n_parameters;                  /**< Path-level parameter count */
  struct OpenAPI_Server *servers;       /**< Path-level servers */
  size_t n_servers;                     /**< Path-level server count */

  struct OpenAPI_Operation *operations; /**< Array of operations on this path */
  size_t n_operations;                  /**< Operation count */
  struct OpenAPI_Operation
      *additional_operations;     /**< additionalOperations map */
  size_t n_additional_operations; /**< additionalOperations count */
};

/**
 * @brief Represents a Server Variable definition.
 */
struct OpenAPI_ServerVariable {
  char *name;            /**< Variable name */
  char **enum_values;    /**< Allowed values (optional) */
  size_t n_enum_values;  /**< Count of enum values */
  char *default_value;   /**< Default value */
  char *description;     /**< Optional description */
  char *extensions_json; /**< Serialized JSON for x- extensions */
};

/**
 * @brief Represents a Server object.
 */
struct OpenAPI_Server {
  char *url;             /**< Server URL */
  char *description;     /**< Optional description */
  char *name;            /**< Optional name */
  char *extensions_json; /**< Serialized JSON for x- extensions */
  struct OpenAPI_ServerVariable *variables; /**< Server variable definitions */
  size_t n_variables;                       /**< Count of variables */
};

/**
 * @brief Contact information for the API.
 */
struct OpenAPI_Contact {
  char *name;            /**< Contact name */
  char *url;             /**< Contact URL */
  char *email;           /**< Contact email */
  char *extensions_json; /**< Serialized JSON for x- extensions */
};

/**
 * @brief License information for the API.
 */
struct OpenAPI_License {
  char *name;            /**< License name */
  char *identifier;      /**< SPDX identifier */
  char *url;             /**< License URL */
  char *extensions_json; /**< Serialized JSON for x- extensions */
};

/**
 * @brief Info metadata for the API.
 */
struct OpenAPI_Info {
  char *title;                    /**< API title */
  char *summary;                  /**< API summary */
  char *description;              /**< API description */
  char *terms_of_service;         /**< Terms of service URL */
  char *version;                  /**< API version */
  char *extensions_json;          /**< Serialized JSON for x- extensions */
  struct OpenAPI_Contact contact; /**< Contact metadata */
  struct OpenAPI_License license; /**< License metadata */
};

/**
 * @brief Registry entry for resolving multi-document OpenAPI $ref targets.
 */
struct OpenAPI_DocRegistryEntry {
  char *base_uri;            /**< Canonical document base URI (no fragment) */
  struct OpenAPI_Spec *spec; /**< Parsed document */
};

/**
 * @brief Registry for multi-document OpenAPI resolution.
 */
struct OpenAPI_DocRegistry {
  struct OpenAPI_DocRegistryEntry *entries; /**< Registered documents */
  size_t count;                             /**< Entry count */
  size_t capacity;                          /**< Allocated capacity */
};

/**
 * @brief Root container for the parsed specification.
 */
struct OpenAPI_Spec {
  char *openapi_version;  /**< OpenAPI version string */
  int is_schema_document; /**< 1 if root is a JSON Schema document */
  char *schema_root_json; /**< Serialized root schema (schema docs) */
  char *self_uri;         /**< Optional $self URI */
  char *retrieval_uri;    /**< Optional retrieval URI (context) */
  char *document_uri;     /**< Resolved base URI (no fragment) */
  struct OpenAPI_DocRegistry *doc_registry; /**< Optional registry */
  char *json_schema_dialect;                /**< Optional jsonSchemaDialect */
  char *extensions_json;            /**< Serialized JSON for x- extensions */
  char *paths_extensions_json;      /**< x- extensions on Paths Object */
  char *webhooks_extensions_json;   /**< x- extensions on Webhooks Object */
  char *components_extensions_json; /**< x- extensions on Components Object */
  struct OpenAPI_Info info;         /**< Info metadata */
  struct OpenAPI_ExternalDocs external_docs; /**< Optional external docs */
  struct OpenAPI_Tag *tags;                  /**< Top-level tag metadata */
  size_t n_tags;                             /**< Tag count */

  struct OpenAPI_SecurityRequirementSet *security; /**< Root security */
  size_t n_security;                               /**< Root security count */
  int security_set;                                /**< 1 if field present */

  struct OpenAPI_Server *servers; /**< Array of servers */
  size_t n_servers;               /**< Server count */

  struct OpenAPI_Path *paths; /**< Array of paths */
  size_t n_paths;             /**< Path count */

  struct OpenAPI_Path *webhooks; /**< Array of webhooks (Path Item Objects) */
  size_t n_webhooks;             /**< Webhook count */

  /* Component Path Items */
  struct OpenAPI_Path *component_path_items; /**< components.pathItems */
  char **component_path_item_names;          /**< components.pathItems keys */
  size_t n_component_path_items;             /**< Count of pathItems */

  struct OpenAPI_SecurityScheme
      *security_schemes;     /**< Defined security components */
  size_t n_security_schemes; /**< Count of schemes */

  /* Component Parameters */
  struct OpenAPI_Parameter *component_parameters; /**< components.parameters */
  char **component_parameter_names; /**< components.parameters keys */
  size_t n_component_parameters;    /**< Count of parameters */

  /* Component Responses */
  struct OpenAPI_Response *component_responses; /**< components.responses */
  char **component_response_names; /**< components.responses keys */
  size_t n_component_responses;    /**< Count of responses */

  /* Component Headers */
  struct OpenAPI_Header *component_headers; /**< components.headers */
  char **component_header_names;            /**< components.headers keys */
  size_t n_component_headers;               /**< Count of headers */

  /* Component Request Bodies */
  struct OpenAPI_RequestBody
      *component_request_bodies;       /**< components.requestBodies */
  char **component_request_body_names; /**< components.requestBodies keys */
  size_t n_component_request_bodies;   /**< Count of requestBodies */

  /* Component Media Types */
  struct OpenAPI_MediaType *component_media_types; /**< components.mediaTypes */
  char **component_media_type_names; /**< components.mediaTypes keys */
  size_t n_component_media_types;    /**< Count of mediaTypes */

  struct OpenAPI_Example *component_examples; /**< components.examples */
  char **component_example_names;             /**< components.examples keys */
  size_t n_component_examples;                /**< Count of examples */

  /* Component Links */
  struct OpenAPI_Link *component_links; /**< components.links */
  size_t n_component_links;             /**< Count of links */

  /* Component Callbacks */
  struct OpenAPI_Callback *component_callbacks; /**< components.callbacks */
  size_t n_component_callbacks;                 /**< Count of callbacks */

  /* Component Schemas stored as raw JSON when non-struct compatible */
  char **raw_schema_names; /**< components.schemas keys for raw schemas */
  char **raw_schema_json;  /**< Serialized JSON for raw schemas */
  size_t n_raw_schemas;    /**< Count of raw schemas */

  /* Global Schema Data (for looking up struct definitions during body gen) */
  struct StructFields
      *defined_schemas;          /**< Array of parsed schemas structure */
  char **defined_schema_names;   /**< Array of schema names matching indices */
  char **defined_schema_ids;     /**< Optional $id values for defined schemas */
  char **defined_schema_anchors; /**< Optional $anchor values */
  char **defined_schema_dynamic_anchors; /**< Optional $dynamicAnchor values */
  size_t n_defined_schemas;              /**< Count of defined schemas */
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
 * @brief Initialize a document registry.
 * @param[out] registry Pointer to registry to initialize.
 */
extern C_CDD_EXPORT void
openapi_doc_registry_init(struct OpenAPI_DocRegistry *registry);

/**
 * @brief Free a document registry and its URI entries.
 *
 * Does NOT free the OpenAPI_Spec instances referenced by the registry.
 *
 * @param[in] registry Pointer to registry to free.
 */
extern C_CDD_EXPORT void
openapi_doc_registry_free(struct OpenAPI_DocRegistry *registry);

/**
 * @brief Register a parsed document with the registry.
 *
 * Uses the document's resolved base URI for matching $ref targets.
 *
 * @param[in,out] registry Registry to add to.
 * @param[in] spec Parsed OpenAPI specification.
 * @return 0 on success, ENOMEM on allocation failure, EINVAL on invalid args.
 */
extern C_CDD_EXPORT int
openapi_doc_registry_add(struct OpenAPI_DocRegistry *registry,
                         struct OpenAPI_Spec *spec);

/**
 * @brief Parse an OpenAPI or Schema document from a JSON Value.
 *
 * Traverses the JSON to populate the Spec structure. For OpenAPI documents:
 * 1. Extracts Paths, Operations, Params, Bodies, and Tags.
 * 2. Extracts Security Schemes from components.
 * 3. Extracts and flattening Schemas definitions.
 *
 * For Schema documents (JSON Schema at the root), the loader sets
 * `is_schema_document` and stores the serialized root schema in
 * `schema_root_json`.
 *
 * @param[in] root The root JSON Value of the OpenAPI or Schema document.
 * @param[out] out Destination structure to populate.
 * @return 0 on success, error code (EINVAL/ENOMEM) on failure.
 */
extern C_CDD_EXPORT int openapi_load_from_json(const JSON_Value *root,
                                               struct OpenAPI_Spec *out);

/**
 * @brief Parse a JSON Value with document context for multi-doc resolution.
 *
 * Resolves relative $self and $ref bases using the provided retrieval URI,
 * and optionally consults a registry for external component references.
 *
 * For Schema documents, $id (when present) is used as the base URI for
 * registry resolution.
 *
 * If a registry is provided, this function registers the parsed document on
 * success.
 *
 * @param[in] root The root JSON Value of the OpenAPI file.
 * @param[in] retrieval_uri Retrieval/base URI for this document (optional).
 * @param[out] out The spec structure to populate (must be initialized).
 * @param[in,out] registry Optional registry for multi-doc resolution.
 * @return 0 on success, EINVAL on invalid input, ENOMEM on allocation failure.
 */
extern C_CDD_EXPORT int openapi_load_from_json_with_context(
    const JSON_Value *root, const char *retrieval_uri, struct OpenAPI_Spec *out,
    struct OpenAPI_DocRegistry *registry);

/**
 * @brief Look up a schema definition by name in the loaded spec.
 *
 * @param[in] spec The spec to search.
 * @param[in] name The schema name (e.g. "LoginRequest").
 * @return Pointer to StructFields if found, NULL otherwise.
 */
extern C_CDD_EXPORT const struct StructFields *
openapi_spec_find_schema(const struct OpenAPI_Spec *spec, const char *name);

/**
 * @brief Find a schema definition by following a SchemaRef.
 *
 * Uses the document registry when present to resolve external component refs.
 *
 * @param[in] spec The current specification.
 * @param[in] ref The schema reference.
 * @return Pointer to StructFields if found, NULL otherwise.
 */
extern C_CDD_EXPORT const struct StructFields *
openapi_spec_find_schema_for_ref(const struct OpenAPI_Spec *spec,
                                 const struct OpenAPI_SchemaRef *ref);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_OPENAPI_LOADER_H */
