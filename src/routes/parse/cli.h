#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
#ifndef C_CDD_C2OPENAPI_CLI_H
#define C_CDD_C2OPENAPI_CLI_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
struct OpenAPI_Spec;
struct TypeDefList;
/* clang-format on */

extern C_CDD_EXPORT cdd_c_error_t c2openapi_register_types(
    struct OpenAPI_Spec *spec, const struct TypeDefList *types);
extern C_CDD_EXPORT cdd_c_error_t c2openapi_cli_main(int argc, char **argv);
extern C_CDD_EXPORT cdd_c_error_t to_docs_json_cli_main(int argc, char **argv);
extern C_CDD_EXPORT cdd_c_error_t to_openapi_cli_main(int argc, char **argv);
extern C_CDD_EXPORT cdd_c_error_t from_openapi_cli_main(int argc, char **argv);
extern C_CDD_EXPORT cdd_c_error_t generate_bindings_cli_main(int argc,
                                                             char **argv);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_C2OPENAPI_CLI_H */

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
