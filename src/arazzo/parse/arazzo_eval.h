/**
 * @file arazzo_eval.h
 * @brief Runtime Expression, JSON Pointer, JSONPath, XPath, Criterion, and
 * Payload Replacement Evaluator for Arazzo v1.1.0.
 *
 * Implements strict C89 evaluation of Arazzo v1.1.0 runtime expressions,
 * RFC 6901 JSON Pointers, JSONPath/XPath selectors, `simple`/`regex` criteria,
 * and request body `Payload Replacement Object` mutations.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_ARAZZO_EVAL_H
/** @brief Header guard for arazzo_eval.h */
#define C_CDD_ARAZZO_EVAL_H

/* clang-format off */
#include "arazzo/parse/arazzo_types.h"
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <parson.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Execution state context for evaluating Arazzo 1.1.0 runtime
 * expressions.
 *
 * Holds active HTTP/AsyncAPI transport state (`$url`, `$method`, `$statusCode`,
 * `$self`) and JSON object maps (`JSON_Object *`) for headers, parameters,
 * bodies, `$inputs`, `$outputs`, `$steps`, `$workflows`, `$sourceDescriptions`,
 * and `$components`.
 */
struct Arazzo_ExecutionContext {
  const char *url;                 /**< Current `$url` string */
  const char *method;              /**< Current `$method` string */
  int status_code;                 /**< Current `$statusCode` integer */
  const char *self_uri;            /**< Document `$self` URI */
  const JSON_Object *req_headers;  /**< `$request.header.*` map */
  const JSON_Object *req_query;    /**< `$request.query.*` map */
  const JSON_Object *req_path;     /**< `$request.path.*` map */
  const JSON_Value *req_body;      /**< `$request.body` JSON root */
  const JSON_Object *resp_headers; /**< `$response.header.*` map */
  const JSON_Value *resp_body;     /**< `$response.body` JSON root */
  const JSON_Value *msg_header;    /**< `$message.header` JSON root */
  const JSON_Value *msg_payload;   /**< `$message.payload` JSON root */
  const JSON_Object *inputs;       /**< `$inputs.*` map */
  const JSON_Object *outputs;      /**< `$outputs.*` map */
  const JSON_Object *steps;        /**< `$steps.<stepId>.<prop>` nested map */
  const JSON_Object *workflows;    /**< `$workflows.<wfId>.<prop>` nested map */
  const JSON_Object *sources;    /**< `$sourceDescriptions.<name>.<prop>` map */
  const JSON_Object *components; /**< `$components.<group>.<name>` map */
};

/**
 * @brief Evaluates an RFC 6901 JSON Pointer (`#/...` or `/...`) against a
 * `JSON_Value`.
 *
 * Handles `~1` (`/`) and `~0` (`~`) unescaping and array index traversal.
 *
 * @param[in] root Root `JSON_Value` to query.
 * @param[in] pointer JSON Pointer string (e.g. `"/user/0/id"` or
 * `"#/user/0/id"`).
 * @param[out] out_val Output pointer receiving borrowed `JSON_Value *` inside
 * `root`.
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_NOT_FOUND` if path does not
 * exist, `CDD_C_ERROR_INVALID_ARGUMENT` if malformed/NULL, or
 * `CDD_C_ERROR_MEMORY` on OOM.
 */
C_CDD_EXPORT cdd_c_error_t arazzo_eval_json_pointer(const JSON_Value *root,
                                                    const char *pointer,
                                                    const JSON_Value **out_val);

/**
 * @brief Evaluates a JSONPath query (`$.a.b[0].c`) against a `JSON_Value`.
 *
 * @param[in] root Root `JSON_Value` to query.
 * @param[in] jsonpath JSONPath string starting with `$`.
 * @param[out] out_val Output pointer receiving borrowed `JSON_Value *` inside
 * `root`.
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_NOT_FOUND` if not found,
 * `CDD_C_ERROR_INVALID_ARGUMENT` if malformed/NULL, or `CDD_C_ERROR_MEMORY` on
 * OOM.
 */
C_CDD_EXPORT cdd_c_error_t arazzo_eval_jsonpath(const JSON_Value *root,
                                                const char *jsonpath,
                                                const JSON_Value **out_val);

/**
 * @brief Evaluates an XPath expression (`/order/item/@id` or `/order/status`)
 * against XML text.
 *
 * @param[in] xml_str Null-terminated XML document string.
 * @param[in] xpath XPath query string.
 * @param[out] out_str Output pointer receiving newly allocated extracted
 * string.
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_NOT_FOUND` if not found,
 * `CDD_C_ERROR_INVALID_ARGUMENT` if invalid/NULL, or `CDD_C_ERROR_MEMORY` on
 * OOM.
 */
C_CDD_EXPORT cdd_c_error_t arazzo_eval_xpath(const char *xml_str,
                                             const char *xpath, char **out_str);

/**
 * @brief Resolves an Arazzo runtime expression or embedded template to a
 * string.
 *
 * @param[in] expr_str Runtime expression or template string.
 * @param[in] ctx Active execution context.
 * @param[out] out_str Output pointer receiving newly allocated resolved string.
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_NOT_FOUND` if unresolvable,
 * `CDD_C_ERROR_INVALID_ARGUMENT` if invalid/NULL, or `CDD_C_ERROR_MEMORY` on
 * OOM.
 */
C_CDD_EXPORT cdd_c_error_t arazzo_eval_runtime_expr_to_str(
    const char *expr_str, const struct Arazzo_ExecutionContext *ctx,
    char **out_str);

/**
 * @brief Evaluates an `Arazzo_Criterion` against an `Arazzo_ExecutionContext`.
 *
 * Supports `simple`, `regex`, `jsonpath`, `xpath`, and `expression_type`
 * criteria.
 *
 * @param[in] criterion Criterion to evaluate.
 * @param[in] ctx Active execution context.
 * @param[out] out_passed Receives `1` if criterion passed, `0` otherwise.
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` if
 * invalid/NULL, or `CDD_C_ERROR_MEMORY` on OOM.
 */
C_CDD_EXPORT cdd_c_error_t arazzo_eval_criterion(
    const struct Arazzo_Criterion *criterion,
    const struct Arazzo_ExecutionContext *ctx, int *out_passed);

/**
 * @brief Evaluates an `Arazzo_Selector` to extract a string value from context.
 *
 * @param[in] selector Selector object (`context`, `selector`, `type`).
 * @param[in] ctx Active execution context.
 * @param[out] out_str Output pointer receiving newly allocated extracted
 * string.
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_NOT_FOUND` if not found,
 * `CDD_C_ERROR_INVALID_ARGUMENT` if invalid/NULL, or `CDD_C_ERROR_MEMORY` on
 * OOM.
 */
C_CDD_EXPORT cdd_c_error_t
arazzo_eval_selector(const struct Arazzo_Selector *selector,
                     const struct Arazzo_ExecutionContext *ctx, char **out_str);

/**
 * @brief Applies an array of `Arazzo_PayloadReplacement` rules to a mutable
 * JSON payload.
 *
 * @param[in,out] payload Mutable `JSON_Value *` object/array payload.
 * @param[in] replacements Array of payload replacements.
 * @param[in] n_replacements Number of replacements.
 * @param[in] ctx Active execution context for resolving replacement `value`
 * expressions.
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` if
 * invalid/NULL, `CDD_C_ERROR_NOT_FOUND` if target does not exist, or
 * `CDD_C_ERROR_MEMORY` on OOM.
 */
C_CDD_EXPORT cdd_c_error_t arazzo_apply_payload_replacements(
    JSON_Value *payload, const struct Arazzo_PayloadReplacement *replacements,
    size_t n_replacements, const struct Arazzo_ExecutionContext *ctx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_ARAZZO_EVAL_H */
