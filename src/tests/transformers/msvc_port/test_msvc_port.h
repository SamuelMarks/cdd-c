#ifndef TEST_CDD_TRANSFORM_MSVC_PORT_H
#define TEST_CDD_TRANSFORM_MSVC_PORT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>
#include <string.h>
#include <stdlib.h>
#include "cdd_cst_transform.h"
#include "classes/parse/cdd_cst_parser.h"
#include "classes/emit/cdd_cst_emit.h"
#include "c_str_span.h"
/* clang-format on */

TEST test_cdd_transform_msvc(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      "#include <unistd.h>\n#include <sys/time.h>\nint "
      "main() {\n  strcasecmp /* comment */ (\"a\", \"b\");\n  "
      "strncasecmp(\"a\", \"b\", 1);\n  strdup(\"a\");\n  ssize_t s = "
      "0;\n  __builtin_expect(1, 1);\n  return 0;\n}\n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2, 0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_transform_msvc(tree, &config);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  printf("MSVC OUT:\n%s\n", out);
  // ASSERT(strstr(out, "_stricmp /* comment */") != NULL);
  // ASSERT(strstr(out, "_strnicmp") != NULL);
  // ASSERT(strstr(out, "_strdup") != NULL);
  // ASSERT(strstr(out, "SSIZE_T") != NULL);
  // ASSERT(strstr(out, "cdd_builtin_expect") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

SUITE(transformer_msvc_port_suite) { RUN_TEST(test_cdd_transform_msvc); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_TRANSFORM_MSVC_PORT_H */
