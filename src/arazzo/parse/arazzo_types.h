/**
 * @file arazzo_types.h
 * @brief Strongly-typed Intermediate Representation (IR) for Arazzo v1.1.0.
 *
 * Defines C89-compliant enums and structures representing the Arazzo
 * Specification v1.1.0, including Info, Source Descriptions, Workflows,
 * Steps, Parameters, Reusable Components, Success/Failure Actions,
 * Criteria, Selectors, Expression Types, Payload Replacements, and
 * Runtime Expressions.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_ARAZZO_TYPES_H
/** @brief Header guard for arazzo_types.h */
#define C_CDD_ARAZZO_TYPES_H

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "openapi/parse/openapi_types.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Supported Arazzo Specification semantic versions.
 */
enum Arazzo_Version {
  ARAZZO_VER_UNKNOWN = 0, /**< Unrecognized or missing version */
  ARAZZO_VER_1_0_0,       /**< Arazzo Specification 1.0.0 */
  ARAZZO_VER_1_0_1,       /**< Arazzo Specification 1.0.1 */
  ARAZZO_VER_1_1_0        /**< Arazzo Specification 1.1.0 */
};

/**
 * @brief Source Description specification type (`openapi`, `asyncapi`,
 * `arazzo`).
 */
enum Arazzo_SourceType {
  ARAZZO_SOURCE_UNKNOWN = 0, /**< Unspecified or unknown source type */
  ARAZZO_SOURCE_OPENAPI,     /**< OpenAPI 3.x / Swagger 2.0 specification */
  ARAZZO_SOURCE_ASYNCAPI,    /**< AsyncAPI specification */
  ARAZZO_SOURCE_ARAZZO       /**< External Arazzo specification */
};

/**
 * @brief Parameter location (`in`) for Arazzo 1.1.0 Parameter Objects.
 */
enum Arazzo_ParamIn {
  ARAZZO_PARAM_IN_UNKNOWN = 0, /**< Unspecified parameter location */
  ARAZZO_PARAM_IN_PATH,        /**< Path parameter (`path`) */
  ARAZZO_PARAM_IN_QUERY,       /**< Query parameter (`query`) */
  ARAZZO_PARAM_IN_QUERYSTRING, /**< Raw query string (`querystring`) */
  ARAZZO_PARAM_IN_HEADER,      /**< HTTP header parameter (`header`) */
  ARAZZO_PARAM_IN_COOKIE       /**< Cookie parameter (`cookie`) */
};

/**
 * @brief Control-flow action type for Success and Failure Actions.
 */
enum Arazzo_ActionType {
  ARAZZO_ACTION_UNKNOWN = 0, /**< Unrecognized action type */
  ARAZZO_ACTION_END,         /**< Terminate workflow execution (`end`) */
  ARAZZO_ACTION_GOTO,        /**< Branch to target step or workflow (`goto`) */
  ARAZZO_ACTION_RETRY        /**< Retry failed step (`retry`) */
};

/**
 * @brief Asynchronous messaging action direction for event-driven steps.
 */
enum Arazzo_AsyncAction {
  ARAZZO_ASYNC_NONE = 0, /**< Synchronous HTTP operation step */
  ARAZZO_ASYNC_SEND,     /**< Publish/send message on channel (`send`) */
  ARAZZO_ASYNC_RECEIVE /**< Subscribe/receive message on channel (`receive`) */
};

/**
 * @brief Criterion evaluation type for assertions and selectors.
 */
enum Arazzo_CriterionType {
  ARAZZO_CRITERION_SIMPLE = 0,     /**< Simple boolean/comparison expression */
  ARAZZO_CRITERION_REGEX,          /**< Regular expression pattern match */
  ARAZZO_CRITERION_JSONPATH,       /**< JSONPath query expression */
  ARAZZO_CRITERION_XPATH,          /**< XPath query expression */
  ARAZZO_CRITERION_EXPRESSION_TYPE /**< Extended Expression Type Object */
};

/**
 * @brief Expression dialect for Expression Type Objects.
 */
enum Arazzo_ExpressionDialect {
  ARAZZO_DIALECT_UNKNOWN = 0, /**< Unknown dialect */
  ARAZZO_DIALECT_JSONPATH,    /**< JSONPath dialect (e.g. RFC 9535) */
  ARAZZO_DIALECT_XPATH,       /**< XPath dialect (e.g. xpath-30, xpath-10) */
  ARAZZO_DIALECT_JSONPOINTER  /**< JSON Pointer dialect (RFC 6901) */
};

/**
 * @brief Mutually-exclusive target discriminator for an Arazzo Step Object.
 */
enum Arazzo_StepTargetKind {
  ARAZZO_STEP_TARGET_NONE = 0,       /**< No target set (invalid) */
  ARAZZO_STEP_TARGET_OPERATION_ID,   /**< `operationId` target */
  ARAZZO_STEP_TARGET_OPERATION_PATH, /**< `operationPath` JSON Pointer target */
  ARAZZO_STEP_TARGET_CHANNEL_PATH,   /**< `channelPath` AsyncAPI target */
  ARAZZO_STEP_TARGET_WORKFLOW_ID     /**< Nested `workflowId` target */
};

/**
 * @brief Strongly-typed Arazzo 1.1.0 Runtime Expression root classification.
 */
enum Arazzo_RuntimeExprKind {
  ARAZZO_EXPR_LITERAL = 0,         /**< Literal value or string template */
  ARAZZO_EXPR_URL,                 /**< `$url` */
  ARAZZO_EXPR_METHOD,              /**< `$method` */
  ARAZZO_EXPR_STATUS_CODE,         /**< `$statusCode` */
  ARAZZO_EXPR_REQUEST_HEADER,      /**< `$request.header.<name>` */
  ARAZZO_EXPR_REQUEST_QUERY,       /**< `$request.query.<name>` */
  ARAZZO_EXPR_REQUEST_PATH,        /**< `$request.path.<name>` */
  ARAZZO_EXPR_REQUEST_BODY,        /**< `$request.body#<pointer>` */
  ARAZZO_EXPR_RESPONSE_HEADER,     /**< `$response.header.<name>` */
  ARAZZO_EXPR_RESPONSE_BODY,       /**< `$response.body#<pointer>` */
  ARAZZO_EXPR_MESSAGE_HEADER,      /**< `$message.header#<pointer>` */
  ARAZZO_EXPR_MESSAGE_PAYLOAD,     /**< `$message.payload#<pointer>` */
  ARAZZO_EXPR_INPUTS,              /**< `$inputs.<name>` */
  ARAZZO_EXPR_OUTPUTS,             /**< `$outputs.<name>` */
  ARAZZO_EXPR_STEPS,               /**< `$steps.<stepId>.outputs.<name>` */
  ARAZZO_EXPR_WORKFLOWS,           /**< `$workflows.<wfId>.outputs.<name>` */
  ARAZZO_EXPR_SOURCE_DESCRIPTIONS, /**< `$sourceDescriptions.<name>.url` */
  ARAZZO_EXPR_COMPONENTS,          /**< `$components.<type>.<name>` */
  ARAZZO_EXPR_SELF                 /**< `$self` */
};

/**
 * @brief Specification Extension (`^x-`) key-value entry.
 */
struct Arazzo_Extension {
  char *key;              /**< Extension key starting with `x-` */
  struct OpenAPI_Any val; /**< Arbitrary JSON value */
};

/**
 * @brief Collection of Specification Extensions (`^x-`).
 */
struct Arazzo_ExtensionList {
  struct Arazzo_Extension *items; /**< Array of extension entries */
  size_t count;                   /**< Number of extension entries */
};

/**
 * @brief Parsed Arazzo 1.1.0 Runtime Expression AST node.
 */
struct Arazzo_RuntimeExpr {
  enum Arazzo_RuntimeExprKind kind; /**< Root expression classification */
  char *raw;                        /**< Original raw expression string */
  char *source_name;                /**< Source/component group identifier */
  char *step_or_workflow_id;        /**< Referenced stepId or workflowId */
  char *property_name;              /**< Header/query/path/input/output key */
  char *json_pointer;               /**< Optional RFC 6901 `#/...` fragment */
  int is_embedded_template;         /**< Non-zero if embedded `{...}` string */
};

/**
 * @brief Expression Type Object specifying dialect and version.
 */
struct Arazzo_ExpressionType {
  enum Arazzo_ExpressionDialect type; /**< Expression dialect */
  char *version; /**< Dialect version (e.g. draft-goessner-dispatch-jsonpath-00)
                  */
  struct Arazzo_ExtensionList extensions; /**< `^x-` extensions */
};

/**
 * @brief Criterion Object defining context, condition, and evaluation rules.
 */
struct Arazzo_Criterion {
  char *context;                  /**< Runtime expression context */
  char *condition;                /**< Condition assertion string */
  enum Arazzo_CriterionType type; /**< Criterion evaluation type */
  struct Arazzo_ExpressionType expression_type; /**< Extended expression type */
  struct Arazzo_ExtensionList extensions;       /**< `^x-` extensions */
};

/**
 * @brief Selector Object for dynamic value extraction via JSONPath/XPath.
 */
struct Arazzo_Selector {
  char *context;                  /**< Runtime expression context */
  char *selector;                 /**< Query selector expression */
  enum Arazzo_CriterionType type; /**< Selector dialect kind */
  struct Arazzo_ExpressionType expression_type; /**< Extended expression type */
  struct Arazzo_ExtensionList extensions;       /**< `^x-` extensions */
};

/**
 * @brief Reusable Object referencing `$components.*` with optional value
 * override.
 */
struct Arazzo_Reusable {
  int is_reusable;          /**< Non-zero if this entry is a Reusable Object */
  char *reference;          /**< `$components.<group>.<name>` reference */
  struct OpenAPI_Any value; /**< Optional overridden value */
  int has_value_override;   /**< Non-zero if `value` override is present */
};

/**
 * @brief Parameter Object or Reusable Parameter reference.
 */
struct Arazzo_Parameter {
  char *name;               /**< Parameter name */
  enum Arazzo_ParamIn in;   /**< Parameter location */
  struct OpenAPI_Any value; /**< Literal or runtime expression value */
  struct Arazzo_Selector
      selector;    /**< Dynamic selector when `is_selector` is set */
  int is_selector; /**< Non-zero if value is a Selector Object */
  struct Arazzo_RuntimeExpr
      parsed_expr; /**< Parsed runtime expression metadata */
  struct Arazzo_Reusable
      reusable; /**< Reusable `$components.parameters.*` ref */
  struct Arazzo_ExtensionList extensions; /**< `^x-` extensions */
};

/**
 * @brief Payload Replacement Object for deep request body patching.
 */
struct Arazzo_PayloadReplacement {
  char *target; /**< JSON Pointer, JSONPath, or XPath target */
  enum Arazzo_CriterionType target_selector_type; /**< Target selector type */
  struct Arazzo_ExpressionType
      target_expression_type; /**< Extended target dialect */
  struct OpenAPI_Any value;   /**< Replacement literal or expression */
  struct Arazzo_RuntimeExpr
      parsed_expr; /**< Parsed runtime expression for `value` */
  struct Arazzo_ExtensionList extensions; /**< `^x-` extensions */
};

/**
 * @brief Request Body Object for an Arazzo Step.
 */
struct Arazzo_RequestBody {
  char *content_type;         /**< MIME contentType (e.g. application/json) */
  struct OpenAPI_Any payload; /**< Request payload template or expression */
  struct Arazzo_PayloadReplacement
      *replacements;                      /**< Array of payload replacements */
  size_t n_replacements;                  /**< Number of payload replacements */
  struct Arazzo_ExtensionList extensions; /**< `^x-` extensions */
};

/**
 * @brief Success Action Object (`end` or `goto`).
 */
struct Arazzo_SuccessAction {
  char *name; /**< Action name */
  enum Arazzo_ActionType
      type;          /**< `ARAZZO_ACTION_END` or `ARAZZO_ACTION_GOTO` */
  char *workflow_id; /**< Target workflowId when branching to workflow */
  char *step_id;     /**< Target stepId when branching to step */
  struct Arazzo_Parameter *parameters; /**< Parameters passed to target */
  size_t n_parameters;                 /**< Number of parameters */
  struct Arazzo_Criterion *criteria;   /**< Guard criteria for taking action */
  size_t n_criteria;                   /**< Number of guard criteria */
  struct Arazzo_Reusable
      reusable; /**< Reusable `$components.successActions.*` ref */
  struct Arazzo_ExtensionList extensions; /**< `^x-` extensions */
};

/**
 * @brief Failure Action Object (`end`, `retry`, or `goto`).
 */
struct Arazzo_FailureAction {
  char *name;                  /**< Action name */
  enum Arazzo_ActionType type; /**< `end`, `retry`, or `goto` */
  char *workflow_id;           /**< Target workflowId on `goto`/`retry` */
  char *step_id;               /**< Target stepId on `goto`/`retry` */
  struct Arazzo_Parameter
      *parameters;     /**< Parameters override on `goto`/`retry` */
  size_t n_parameters; /**< Number of parameters */
  double retry_after;  /**< Delay in seconds before retry */
  int has_retry_after; /**< Non-zero if `retryAfter` is specified */
  int retry_limit;     /**< Maximum number of retry attempts */
  int has_retry_limit; /**< Non-zero if `retryLimit` is specified */
  struct Arazzo_Criterion
      *criteria;     /**< Guard criteria matching error condition */
  size_t n_criteria; /**< Number of guard criteria */
  struct Arazzo_Reusable
      reusable; /**< Reusable `$components.failureActions.*` ref */
  struct Arazzo_ExtensionList extensions; /**< `^x-` extensions */
};

/**
 * @brief Output mapping entry (`key` -> runtime expression or Selector Object).
 */
struct Arazzo_OutputMapEntry {
  char *key;                 /**< Output variable key (`^[a-zA-Z0-9\.\-_]+$`) */
  char *expression_or_value; /**< Runtime expression string */
  struct Arazzo_Selector
      selector;    /**< Selector Object when `is_selector` is set */
  int is_selector; /**< Non-zero if mapped via Selector Object */
  struct Arazzo_RuntimeExpr parsed_expr; /**< Parsed runtime expression AST */
};

/**
 * @brief Step Object representing a single workflow step.
 */
struct Arazzo_Step {
  char *step_id;     /**< Unique step identifier within workflow */
  char *description; /**< CommonMark description */
  enum Arazzo_StepTargetKind target_kind; /**< Target discriminator */
  char *operation_id;                     /**< Target OpenAPI `operationId` */
  char *operation_path; /**< Target `$sourceDescriptions.<name>.url#/...` */
  char *channel_path;   /**< Target AsyncAPI `channelPath` */
  char *workflow_id;    /**< Target nested `workflowId` */
  struct Arazzo_Parameter *parameters;    /**< Step input parameters */
  size_t n_parameters;                    /**< Number of step parameters */
  struct Arazzo_RequestBody request_body; /**< Step request body */
  int has_request_body; /**< Non-zero if `requestBody` is present */
  struct Arazzo_Criterion
      *success_criteria;     /**< Assertions verifying step success */
  size_t n_success_criteria; /**< Number of success criteria */
  struct Arazzo_SuccessAction *on_success; /**< Step-level success actions */
  size_t n_on_success;                     /**< Number of onSuccess actions */
  struct Arazzo_FailureAction *on_failure; /**< Step-level failure actions */
  size_t n_on_failure;                     /**< Number of onFailure actions */
  struct Arazzo_OutputMapEntry *outputs;   /**< Step outputs map */
  size_t n_outputs;                        /**< Number of step outputs */
  int timeout_ms;                 /**< Maximum execution timeout in ms */
  int has_timeout;                /**< Non-zero if `timeout` is specified */
  char *correlation_id;           /**< Async correlationId expression/token */
  enum Arazzo_AsyncAction action; /**< Async `send` or `receive` action */
  char **depends_on;              /**< Prerequisite stepIds within workflow */
  size_t n_depends_on;            /**< Number of prerequisite stepIds */
  struct Arazzo_ExtensionList extensions; /**< `^x-` extensions */
};

/**
 * @brief Workflow Object representing a complete Arazzo workflow.
 */
struct Arazzo_Workflow {
  char *workflow_id;               /**< Unique workflow identifier */
  char *summary;                   /**< Short summary of workflow */
  char *description;               /**< Detailed CommonMark description */
  struct OpenAPI_SchemaRef inputs; /**< JSON Schema 2020-12 input schema */
  int has_inputs;            /**< Non-zero if `inputs` schema is present */
  char **depends_on;         /**< Prerequisite workflowIds */
  size_t n_depends_on;       /**< Number of prerequisite workflowIds */
  struct Arazzo_Step *steps; /**< Ordered list of workflow steps */
  size_t n_steps;            /**< Number of steps */
  struct Arazzo_SuccessAction
      *success_actions;     /**< Workflow-level default success actions */
  size_t n_success_actions; /**< Number of workflow success actions */
  struct Arazzo_FailureAction
      *failure_actions;     /**< Workflow-level default failure actions */
  size_t n_failure_actions; /**< Number of workflow failure actions */
  struct Arazzo_OutputMapEntry *outputs; /**< Workflow outputs map */
  size_t n_outputs;                      /**< Number of workflow outputs */
  struct Arazzo_Parameter *parameters; /**< Workflow-wide default parameters */
  size_t n_parameters;                 /**< Number of workflow parameters */
  struct Arazzo_ExtensionList extensions; /**< `^x-` extensions */
};

/**
 * @brief Info Object providing metadata about the Arazzo document.
 */
struct Arazzo_Info {
  char *title;       /**< Human-readable title (required) */
  char *summary;     /**< Short summary */
  char *description; /**< CommonMark description */
  char *version;     /**< Document version string (required) */
  struct Arazzo_ExtensionList extensions; /**< `^x-` extensions */
};

/**
 * @brief Source Description Object linking external OpenAPI, AsyncAPI, or
 * Arazzo specs.
 */
struct Arazzo_SourceDescription {
  char *name;                  /**< Source identifier (`^[A-Za-z0-9_\-]+$`) */
  char *url;                   /**< URI reference to target specification */
  enum Arazzo_SourceType type; /**< `openapi`, `asyncapi`, or `arazzo` */
  struct OpenAPI_Spec
      *resolved_openapi; /**< Optional resolved OpenAPI_Spec pointer */
  struct Arazzo_ExtensionList extensions; /**< `^x-` extensions */
};

/**
 * @brief Components Object holding reusable inputs, parameters, and actions.
 */
struct Arazzo_Components {
  char **input_keys;                   /**< Keys for reusable input schemas */
  struct OpenAPI_SchemaRef *inputs;    /**< Array of reusable input schemas */
  size_t n_inputs;                     /**< Number of reusable input schemas */
  char **parameter_keys;               /**< Keys for reusable parameters */
  struct Arazzo_Parameter *parameters; /**< Array of reusable parameters */
  size_t n_parameters;                 /**< Number of reusable parameters */
  char **success_action_keys;          /**< Keys for reusable successActions */
  struct Arazzo_SuccessAction
      *success_actions;       /**< Array of reusable successActions */
  size_t n_success_actions;   /**< Number of reusable successActions */
  char **failure_action_keys; /**< Keys for reusable failureActions */
  struct Arazzo_FailureAction
      *failure_actions;     /**< Array of reusable failureActions */
  size_t n_failure_actions; /**< Number of reusable failureActions */
  struct Arazzo_ExtensionList extensions; /**< `^x-` extensions */
};

/**
 * @brief Root Arazzo Specification Object (v1.1.0).
 */
struct Arazzo_Spec {
  char *arazzo;                /**< Version string (e.g. "1.1.0") */
  enum Arazzo_Version version; /**< Parsed semantic version enum */
  char *self_uri;              /**< Optional `$self` URI */
  struct Arazzo_Info info;     /**< Document metadata */
  struct Arazzo_SourceDescription
      *source_descriptions;            /**< Linked source descriptions */
  size_t n_source_descriptions;        /**< Number of source descriptions */
  struct Arazzo_Workflow *workflows;   /**< Defined workflows */
  size_t n_workflows;                  /**< Number of workflows */
  struct Arazzo_Components components; /**< Reusable components */
  int has_components; /**< Non-zero if `components` is present */
  struct Arazzo_ExtensionList extensions; /**< Root `^x-` extensions */
};

/**
 * @brief Initializes an `Arazzo_Spec` structure to a clean zeroed state with
 * default version `1.1.0`.
 *
 * @param[out] spec Pointer to the `Arazzo_Spec` structure to initialize.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT` if
 * `spec` is NULL.
 */
C_CDD_EXPORT cdd_c_error_t arazzo_spec_init(struct Arazzo_Spec *spec);

/**
 * @brief Releases all dynamically allocated memory owned by an `Arazzo_Spec`
 * structure.
 *
 * Safe to call on a partially-populated or zeroed structure; no-op if `spec` is
 * NULL.
 *
 * @param[in,out] spec Pointer to the `Arazzo_Spec` structure to free.
 */
C_CDD_EXPORT void arazzo_spec_free(struct Arazzo_Spec *spec);

/**
 * @brief Releases all dynamically allocated memory inside an
 * `Arazzo_RuntimeExpr`.
 *
 * @param[in,out] expr Pointer to the runtime expression structure to free.
 */
C_CDD_EXPORT void arazzo_runtime_expr_free(struct Arazzo_RuntimeExpr *expr);

/**
 * @brief Creates a deep copy of an `Arazzo_RuntimeExpr` structure.
 *
 * @param[out] dst Destination runtime expression structure.
 * @param[in] src Source runtime expression structure.
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` if NULL,
 * or `CDD_C_ERROR_MEMORY` on allocation failure.
 */
C_CDD_EXPORT cdd_c_error_t arazzo_runtime_expr_copy(
    struct Arazzo_RuntimeExpr *dst, const struct Arazzo_RuntimeExpr *src);

/**
 * @brief Releases all dynamically allocated memory inside an
 * `Arazzo_ExtensionList`.
 *
 * @param[in,out] list Pointer to the extension list to free.
 */
C_CDD_EXPORT void arazzo_extension_list_free(struct Arazzo_ExtensionList *list);

/**
 * @brief Appends a specification extension (`^x-`) to an
 * `Arazzo_ExtensionList`.
 *
 * @param[in,out] list Target extension list.
 * @param[in] key Extension key (must start with `"x-"`).
 * @param[in] val Extension value to deep-copy.
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` if
 * invalid, or `CDD_C_ERROR_MEMORY` on OOM.
 */
C_CDD_EXPORT cdd_c_error_t
arazzo_extension_list_append(struct Arazzo_ExtensionList *list, const char *key,
                             const struct OpenAPI_Any *val);

/**
 * @brief Converts an `Arazzo_SourceType` enum to its canonical Arazzo string
 * representation.
 *
 * @param[in] type Source description type enum.
 * @param[out] out_str Output pointer receiving static string (`"openapi"`,
 * `"asyncapi"`, `"arazzo"`).
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT` if
 * unknown/NULL.
 */
C_CDD_EXPORT cdd_c_error_t
arazzo_source_type_to_str(enum Arazzo_SourceType type, const char **out_str);

/**
 * @brief Parses a source description type string into an `Arazzo_SourceType`
 * enum.
 *
 * @param[in] str Input string (`"openapi"`, `"asyncapi"`, `"arazzo"`).
 * @param[out] out_type Output enum value.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT` if
 * unrecognized/NULL.
 */
C_CDD_EXPORT cdd_c_error_t
arazzo_source_type_from_str(const char *str, enum Arazzo_SourceType *out_type);

/**
 * @brief Converts an `Arazzo_ParamIn` enum to its canonical Arazzo string
 * representation.
 *
 * @param[in] in_loc Parameter location enum.
 * @param[out] out_str Output pointer receiving static string (`"path"`,
 * `"query"`, `"querystring"`, `"header"`, `"cookie"`).
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT` if
 * unknown/NULL.
 */
C_CDD_EXPORT cdd_c_error_t arazzo_param_in_to_str(enum Arazzo_ParamIn in_loc,
                                                  const char **out_str);

/**
 * @brief Parses a parameter location string into an `Arazzo_ParamIn` enum.
 *
 * @param[in] str Input string (`"path"`, `"query"`, `"querystring"`,
 * `"header"`, `"cookie"`).
 * @param[out] out_in Output enum value.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT` if
 * unrecognized/NULL.
 */
C_CDD_EXPORT cdd_c_error_t
arazzo_param_in_from_str(const char *str, enum Arazzo_ParamIn *out_in);

/**
 * @brief Converts an `Arazzo_ActionType` enum to its canonical Arazzo string
 * representation.
 *
 * @param[in] action Action type enum.
 * @param[out] out_str Output pointer receiving static string (`"end"`,
 * `"goto"`, `"retry"`).
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT` if
 * unknown/NULL.
 */
C_CDD_EXPORT cdd_c_error_t
arazzo_action_type_to_str(enum Arazzo_ActionType action, const char **out_str);

/**
 * @brief Parses an action type string into an `Arazzo_ActionType` enum.
 *
 * @param[in] str Input string (`"end"`, `"goto"`, `"retry"`).
 * @param[out] out_action Output enum value.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT` if
 * unrecognized/NULL.
 */
C_CDD_EXPORT cdd_c_error_t arazzo_action_type_from_str(
    const char *str, enum Arazzo_ActionType *out_action);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_ARAZZO_TYPES_H */
