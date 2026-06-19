#ifndef C_CDD_C2OPENAPI_CLI_H
#define C_CDD_C2OPENAPI_CLI_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
/* clang-format on */

extern C_CDD_EXPORT int c2openapi_cli_main(int argc, char **argv);
extern C_CDD_EXPORT int to_docs_json_cli_main(int argc, char **argv);
extern C_CDD_EXPORT int to_openapi_cli_main(int argc, char **argv);
extern C_CDD_EXPORT int from_openapi_cli_main(int argc, char **argv);
extern C_CDD_EXPORT int generate_bindings_cli_main(int argc, char **argv);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_C2OPENAPI_CLI_H */
