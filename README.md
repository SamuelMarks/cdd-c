====================[ Build | all | Debug (vcpkg,test) ]========================
/Users/samuel/Applications/CLion.app/Contents/bin/cmake/mac/aarch64/bin/cmake --build cdd-c/cmake-build-debug-vcpkgtest --target all -j 10
[13/65] Building C object c_cdd/CMakeFiles/c_cdd.dir/codegen_sdk_tests.c.o
cdd-c/c_cdd/codegen_sdk_tests.c:30:10: warning: mixing declarations and code is a C99 extension [-Wdeclaration-after-statement]
30 |   size_t i;
|          ^
1 warning generated.
[23/65] Building C object c_cdd/CMakeFiles/c_cdd.dir/doc_parser.c.o
cdd-c/c_cdd/doc_parser.c:145:9: warning: unused variable 'token' [-Wunused-variable]
145 |   char *token;
|         ^~~~~
cdd-c/c_cdd/doc_parser.c:268:38: warning: '/*' within block comment [-Wcomment]
268 |   /* Skip initial marker if present (/** or ///) via simple scan loop */
|                                      ^
cdd-c/c_cdd/doc_parser.c:272:17: warning: unused variable 'directive_start' [-Wunused-variable]
272 |     const char *directive_start = NULL;
|                 ^~~~~~~~~~~~~~~
3 warnings generated.
[26/65] Building C object c_cdd/CMakeFiles/c_cdd.dir/http_types.c.o
cdd-c/c_cdd/http_types.c:20:44: warning: variadic macros are a C99 feature [-Wvariadic-macros]
20 | #define sprintf_s_wrapper(buf, start, cap, ...)                                \
|                                            ^
1 warning generated.
[30/65] Building C object c_cdd/CMakeFiles/c_cdd.dir/openapi_client_gen.c.o
cdd-c/c_cdd/openapi_client_gen.c:200:10: warning: string literal of length 1028 exceeds maximum length 509 that C90 compilers are required to support [-Woverlength-strings]
199 |   CHECK_IO(fprintf(
|   ~~~~~~~~~~~~~~~~~
200 |       c, "static int ApiError_from_json(const char *json, struct ApiError "
|       ~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
201 |          "**out) {\n"
|          ~~~~~~~~~~~~
202 |          "  JSON_Value *root;\n"
|          ~~~~~~~~~~~~~~~~~~~~~~~
203 |          "  JSON_Object *obj;\n"
|          ~~~~~~~~~~~~~~~~~~~~~~~
204 |          "  if(!json || !out) return 22; /* EINVAL */\n"
|          ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
205 |          "  *out = calloc(1, sizeof(struct ApiError));\n"
|          ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
206 |          "  if(!*out) return 12; /* ENOMEM */\n"
|          ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
207 |          "  (*out)->raw_body = strdup(json);\n"
|          ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
208 |          "  root = json_parse_string(json);\n"
|          ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
209 |          "  if(!root) return 0; /* Not JSON, return strict success but object "
|          ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
210 |          "only has raw_body */\n"
|          ~~~~~~~~~~~~~~~~~~~~~~~~
211 |          "  obj = json_value_get_object(root);\n"
|          ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
212 |          "  if(obj) {\n"
|          ~~~~~~~~~~~~~~~
213 |          "    if(json_object_has_value(obj, \"type\")) (*out)->type = "
|          ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
214 |          "strdup(json_object_get_string(obj, \"type\"));\n"
|          ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
cdd-c/c_cdd/openapi_client_gen.c:27:10: note: expanded from macro 'CHECK_IO'
27 |     if ((x) < 0)                                                               \
|          ^
1 warning generated.
[52/65] Building C object c_cdd/CMakeFiles/c_cdd_cli.dir/main.c.o
cdd-c/c_cdd/main.c:28:13: warning: unused function 'print_error' [-Wunused-function]
28 | static void print_error(int rc, const char *command_name) {
|             ^~~~~~~~~~~
1 warning generated.
[61/65] Building C object c_cdd/tests/CMakeFiles/test_c_cdd.dir/test_c_cdd.c.o
FAILED: c_cdd/tests/CMakeFiles/test_c_cdd.dir/test_c_cdd.c.o
/usr/bin/cc -DHAVE_ASPRINTF -Icdd-c/cmake-build-debug-vcpkgtest/test_downloads -Icdd-c/c_cdd/tests -Icdd-c/cmake-build-debug-vcpkgtest/c_cdd/tests -Icdd-c/cmake-build-debug-vcpkgtest/c_cdd/tests/mocks -Icdd-c/c_cdd/tests/cdd_test_helpers -Icdd-c/cmake-build-debug-vcpkgtest/c_cdd/tests/cdd_test_helpers -Icdd-c/c_cdd -Icdd-c/cmake-build-debug-vcpkgtest/c_cdd -Icdd-c/cmake-build-debug-vcpkgtest -Icdd-c/c_cdd/tests/mocks -isystem cdd-c/cmake-build-debug-vcpkgtest/vcpkg_installed/arm64-osx/include -g -std=gnu90 -arch arm64 -fvisibility=hidden -fcolor-diagnostics -Wshadow -Wformat=2 -Wall -Wno-missing-braces -Wno-long-long -pedantic -fprofile-arcs -ftest-coverage -MD -MT c_cdd/tests/CMakeFiles/test_c_cdd.dir/test_c_cdd.c.o -MF c_cdd/tests/CMakeFiles/test_c_cdd.dir/test_c_cdd.c.o.d -o c_cdd/tests/CMakeFiles/test_c_cdd.dir/test_c_cdd.c.o -c cdd-c/c_cdd/tests/test_c_cdd.c
In file included from cdd-c/c_cdd/tests/test_c_cdd.c:50:
cdd-c/c_cdd/tests/test_c2openapi_op.h:41:30: error: no member named 'in_loc' in 'struct OpenAPI_Parameter'
41 |       free(op->parameters[i].in_loc); /* Wait, struct mismatch? */
|            ~~~~~~~~~~~~~~~~~ ^
cdd-c/c_cdd/tests/test_c2openapi_op.h:82:20: error: variable has incomplete type 'struct ParsedSig'
82 |   struct ParsedSig sig;
|                    ^
cdd-c/c_cdd/tests/test_c2openapi_op.h:82:10: note: forward declaration of 'struct ParsedSig'
82 |   struct ParsedSig sig;
|          ^
cdd-c/c_cdd/tests/test_c2openapi_op.h:83:24: error: array has incomplete element type 'struct ParsedArg'
83 |   struct ParsedArg args[1];
|                        ^
cdd-c/c_cdd/tests/test_c2openapi_op.h:83:10: note: forward declaration of 'struct ParsedArg'
83 |   struct ParsedArg args[1];
|          ^
cdd-c/c_cdd/tests/test_c2openapi_op.h:130:20: error: variable has incomplete type 'struct ParsedSig'
130 |   struct ParsedSig sig;
|                    ^
cdd-c/c_cdd/tests/test_c2openapi_op.h:130:10: note: forward declaration of 'struct ParsedSig'
130 |   struct ParsedSig sig;
|          ^
cdd-c/c_cdd/tests/test_c2openapi_op.h:131:24: error: array has incomplete element type 'struct ParsedArg'
131 |   struct ParsedArg args[1];
|                        ^
cdd-c/c_cdd/tests/test_c2openapi_op.h:131:10: note: forward declaration of 'struct ParsedArg'
131 |   struct ParsedArg args[1];
|          ^
cdd-c/c_cdd/tests/test_c2openapi_op.h:168:20: error: variable has incomplete type 'struct ParsedSig'
168 |   struct ParsedSig sig;
|                    ^
cdd-c/c_cdd/tests/test_c2openapi_op.h:168:10: note: forward declaration of 'struct ParsedSig'
168 |   struct ParsedSig sig;
|          ^
cdd-c/c_cdd/tests/test_c2openapi_op.h:169:24: error: array has incomplete element type 'struct ParsedArg'
169 |   struct ParsedArg args[1];
|                        ^
cdd-c/c_cdd/tests/test_c2openapi_op.h:169:10: note: forward declaration of 'struct ParsedArg'
169 |   struct ParsedArg args[1];
|          ^
cdd-c/c_cdd/tests/test_c2openapi_op.h:212:20: error: variable has incomplete type 'struct ParsedSig'
212 |   struct ParsedSig sig;
|                    ^
cdd-c/c_cdd/tests/test_c2openapi_op.h:212:10: note: forward declaration of 'struct ParsedSig'
212 |   struct ParsedSig sig;
|          ^
cdd-c/c_cdd/tests/test_c2openapi_op.h:213:24: error: array has incomplete element type 'struct ParsedArg'
213 |   struct ParsedArg args[1];
|                        ^
cdd-c/c_cdd/tests/test_c2openapi_op.h:213:10: note: forward declaration of 'struct ParsedArg'
213 |   struct ParsedArg args[1];
|          ^
In file included from cdd-c/c_cdd/tests/test_c_cdd.c:52:
cdd-c/c_cdd/tests/test_c_mapping.h:37:7: warning: unused variable 'rc' [-Wunused-variable]
37 |   int rc;
|       ^~
cdd-c/c_cdd/tests/test_c_mapping.h:54:7: warning: unused variable 'rc' [-Wunused-variable]
54 |   int rc;
|       ^~
In file included from cdd-c/c_cdd/tests/test_c_cdd.c:53:
cdd-c/c_cdd/tests/test_doc_parser.h:10:26: warning: '/*' within block comment [-Wcomment]
10 |  * - Handling of block (`/**`) and line (`///`) comment styles.
|                          ^
In file included from cdd-c/c_cdd/tests/test_c_cdd.c:54:
cdd-c/c_cdd/tests/test_integration_c2openapi.h:36:50: warning: format specifies type 'char *' but the argument has type 'char' [-Wformat]
36 |   asprintf(&src_dir, "%s%sc2o_test_%d", tmp_dir, PATH_SEP_C, rand());
|                         ~~                       ^~~~~~~~~~
|                         %c
cdd-c/c_cdd/fs.h:96:20: note: expanded from macro 'PATH_SEP_C'
96 | #define PATH_SEP_C '/'
|                    ^~~
In file included from cdd-c/c_cdd/tests/test_c_cdd.c:54:
cdd-c/c_cdd/tests/test_integration_c2openapi.h:38:43: warning: format specifies type 'char *' but the argument has type 'char' [-Wformat]
38 |   asprintf(&c_file, "%s%sapi.c", src_dir, PATH_SEP_C);
|                        ~~                 ^~~~~~~~~~
|                        %c
cdd-c/c_cdd/fs.h:96:20: note: expanded from macro 'PATH_SEP_C'
96 | #define PATH_SEP_C '/'
|                    ^~~
In file included from cdd-c/c_cdd/tests/test_c_cdd.c:54:
cdd-c/c_cdd/tests/test_integration_c2openapi.h:39:46: warning: format specifies type 'char *' but the argument has type 'char' [-Wformat]
39 |   asprintf(&h_file, "%s%smodels.h", src_dir, PATH_SEP_C);
|                        ~~                    ^~~~~~~~~~
|                        %c
cdd-c/c_cdd/fs.h:96:20: note: expanded from macro 'PATH_SEP_C'
96 | #define PATH_SEP_C '/'
|                    ^~~
In file included from cdd-c/c_cdd/tests/test_c_cdd.c:54:
cdd-c/c_cdd/tests/test_integration_c2openapi.h:40:49: warning: format specifies type 'char *' but the argument has type 'char' [-Wformat]
40 |   asprintf(&out_json, "%s%sspec.json", src_dir, PATH_SEP_C);
|                          ~~                     ^~~~~~~~~~
|                          %c
cdd-c/c_cdd/fs.h:96:20: note: expanded from macro 'PATH_SEP_C'
96 | #define PATH_SEP_C '/'
|                    ^~~
In file included from cdd-c/c_cdd/tests/test_c_cdd.c:54:
cdd-c/c_cdd/tests/test_integration_c2openapi.h:67:34: warning: initializer for aggregate is not a compile-time constant [-Wc99-extensions]
67 |     char *argv[] = {"c2openapi", src_dir, out_json};
|                                  ^~~~~~~
8 warnings and 9 errors generated.
[63/65] Linking C executable bin/c_cdd_cli
ninja: build stopped: subcommand failed.
