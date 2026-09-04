/* Auto-generated Node.js N-API bindings for test_Lib_name */

#include "(null)"
#include <node_api.h>
#include <stdlib.h>
#include <string.h>

/* Helper for checking N-API calls */
#define NAPI_CALL(env, call)                                                   \
  do {                                                                         \
    napi_status status = (call);                                               \
    if (status != napi_ok) {                                                   \
      const napi_extended_error_info *error_info = NULL;                       \
      napi_get_last_error_info((env), &error_info);                            \
      int is_pending;                                                          \
      napi_is_exception_pending((env), &is_pending);                           \
      if (!is_pending) {                                                       \
        const char *message = (error_info->error_message == NULL)              \
                                  ? "empty error message"                      \
                                  : error_info->error_message;                 \
        napi_throw_error((env), NULL, message);                                \
      }                                                                        \
      return NULL;                                                             \
    }                                                                          \
  } while (0)

static napi_value Init(napi_env env, napi_value exports) {
  napi_property_descriptor desc[] = {};
  size_t prop_count = sizeof(desc) / sizeof(desc[0]);
  if (prop_count > 0) {
    NAPI_CALL(env, napi_define_properties(env, exports, prop_count, desc));
  }
  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
