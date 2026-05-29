#ifndef GEN_DEF_H
#define GEN_DEF_H
/* clang-format off */
#include "gen_def_models.h"
#include "url_utils.h"
#include <c_abstract_http/http_types.h>
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Standardized API Error structure (Problem Details).
 */
struct ApiError {
  char *type;     /**< type */
  char *title;    /**< title */
  int status;     /**< status */
  char *detail;   /**< detail */
  char *instance; /**< instance */
  char *raw_body; /**< raw_body */
};

/**
 * @brief Auto-generated code from OpenAPI specification
 */
void ApiError_cleanup(struct ApiError *err);

/**
 * @brief Initialize the API Client.
 * @param[out] client The client struct to initialize.
 * @param[in] base_url The API base URL (or NULL to use the default server URL).
 * @return 0 on success.
 */
int init(struct HttpClient *client, const char *base_url);

/**
 * @brief Cleanup the API Client.
 */
void cleanup(struct HttpClient *client);

/**
 * @brief test_op
 * @route GET /test
 * @jsonSchemaDialect test_op
 * @termsOfService test_op
 * @return 200
 */
int test_op(struct HttpClient *ctx, struct ApiError **api_error);

#ifdef __cplusplus
}
#endif
#endif /* GEN_DEF_H */
