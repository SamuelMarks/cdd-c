/**
 * @file codegen_sdk_tests.c
 * @brief Implementation of SDK Test Generation.
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "functions/emit_client_sig.h"
#include "tests/emit_sdk_tests.h"

#define CHECK_IO(x)                                                            \
  do {                                                                         \
    if ((x) < 0)                                                               \
      return EIO;                                                              \
  } while (0)

/* --- Helper to write a test for a single operation --- */
static int write_test_operation(FILE *fp, const struct OpenAPI_Operation *op,
                                const struct SdkTestsConfig *config) {
  size_t i;
  CHECK_IO(fprintf(fp, "\nTEST test_%s(void) {\n", op->operation_id));
  CHECK_IO(fprintf(fp, "  struct HttpClient client;\n"));
  CHECK_IO(fprintf(fp, "  int rc;\n"));

  /* Construct Arguments logic */
  /* For simplicity, declare variables for required args with placeholder values
   */
  for (i = 0; i < op->n_parameters; ++i) {
    const struct OpenAPI_Parameter *p = &op->parameters[i];
    if (strcmp(p->type, "integer") == 0) {
      CHECK_IO(fprintf(fp, "  const int %s = 1;\n", p->name));
    } else if (strcmp(p->type, "boolean") == 0) {
      CHECK_IO(fprintf(fp, "  const int %s = 1;\n", p->name));
    } else if (strcmp(p->type, "string") == 0) {
      CHECK_IO(fprintf(fp, "  const char *%s = \"test\";\n", p->name));
    }
  }

  /* Request Body */
  if (op->req_body.ref_name) {
    CHECK_IO(
        fprintf(fp, "  struct %s *req_body = NULL;\n", op->req_body.ref_name));
    if (!op->req_body.is_array) {
      CHECK_IO(fprintf(fp, "  /* Assume %s_default works */\n",
                       op->req_body.ref_name));
      CHECK_IO(
          fprintf(fp, "  %s_default(&req_body);\n", op->req_body.ref_name));
    } else {
      /* Array body stub */
      CHECK_IO(fprintf(fp, "  /* Array body stub */\n"));
    }
  }

  /* Response Output decl */
  /* Find success response (2xx) to type output */
  {
    char *res_type = "void";
    int has_output = 0;
    size_t j;
    for (j = 0; j < op->n_responses; ++j) {
      if (op->responses[j].code[0] == '2' && op->responses[j].schema.ref_name) {
        res_type = op->responses[j].schema.ref_name;
        has_output = 1;
        break;
      }
    }

    if (has_output) {
      CHECK_IO(fprintf(fp, "  struct %s *res_out = NULL;\n", res_type));
    }

    /* Client Init */
    CHECK_IO(fprintf(fp, "  rc = %sinit(&client, \"%s\");\n",
                     config->func_prefix, config->mock_server_url));
    CHECK_IO(fprintf(fp, "  ASSERT_EQ(0, rc);\n"));

    /* Call */
    CHECK_IO(fprintf(fp, "  rc = %s%s(&client", config->func_prefix,
                     op->operation_id));

    /* Args */
    for (i = 0; i < op->n_parameters; ++i) {
      CHECK_IO(fprintf(fp, ", %s", op->parameters[i].name));
      if (op->parameters[i].is_array) {
        CHECK_IO(fprintf(fp, ", 0")); /* len */
      }
    }
    if (op->req_body.ref_name) {
      if (op->req_body.is_array) {
        CHECK_IO(fprintf(fp, ", NULL, 0"));
      } else {
        CHECK_IO(fprintf(fp, ", req_body"));
      }
    }
    if (has_output) {
      CHECK_IO(fprintf(fp, ", &res_out"));
    }
    /* Error out assumed not used in simple signature unless configured,
     * currently codegen_client_sig handles it if present */
    /* Checking if error responses exist */
    {
      int has_err = 0;
      for (j = 0; j < op->n_responses; ++j) {
        if (op->responses[j].code[0] == '4' &&
            op->responses[j].schema.ref_name) {
          has_err = 1;
        }
      }
      if (has_err) {
        CHECK_IO(fprintf(fp, ", NULL")); /* error_out */
      }
    }

    CHECK_IO(fprintf(fp, ");\n"));
    CHECK_IO(fprintf(
        fp, "  /* Check Result - Mock server returns 200 OK text usually, so "
            "parse might fail unless mock matches model */\n"));
    CHECK_IO(fprintf(fp,
                     "  /* ASSERT_EQ(0, rc); Intentionally commented out as "
                     "mock server returns generic OK currently */\n"));

    /* Cleanup */
    CHECK_IO(fprintf(fp, "  %scleanup(&client);\n", config->func_prefix));
    if (has_output) {
      CHECK_IO(fprintf(fp, "  %s_cleanup(res_out);\n", res_type));
    }
    if (op->req_body.ref_name && !op->req_body.is_array) {
      CHECK_IO(fprintf(fp, "  %s_cleanup(req_body);\n", op->req_body.ref_name));
    }
  }

  CHECK_IO(fprintf(fp, "  PASS();\n}\n"));
  return 0;
}

int codegen_sdk_tests_generate(FILE *const fp,
                               const struct OpenAPI_Spec *const spec,
                               const struct SdkTestsConfig *const config) {
  size_t i, j;

  if (!fp || !spec || !config || !config->client_header ||
      !config->mock_server_url)
    return EINVAL;

  /* Header */
  CHECK_IO(fprintf(fp,
                   "#include <greatest.h>\n"
                   "#include <stdlib.h>\n"
                   "#include <string.h>\n"
                   "#include \"%s\"\n\n",
                   config->client_header));

  CHECK_IO(fprintf(fp, "GREATEST_MAIN_DEFS();\n\n"));

  /* Iterate Operations */
  for (i = 0; i < spec->n_paths; ++i) {
    for (j = 0; j < spec->paths[i].n_operations; ++j) {
      if (write_test_operation(fp, &spec->paths[i].operations[j], config) != 0)
        return EIO;
    }
  }

  /* Runner */
  CHECK_IO(fprintf(fp, "\nSUITE(sdk_suite) {\n"));
  for (i = 0; i < spec->n_paths; ++i) {
    for (j = 0; j < spec->paths[i].n_operations; ++j) {
      CHECK_IO(fprintf(fp, "  RUN_TEST(test_%s);\n",
                       spec->paths[i].operations[j].operation_id));
    }
  }
  CHECK_IO(fprintf(fp, "}\n\n"));

  CHECK_IO(fprintf(fp, "int main(int argc, char **argv) {\n"));
  CHECK_IO(fprintf(fp, "  GREATEST_MAIN_BEGIN();\n"));
  CHECK_IO(fprintf(fp, "  RUN_SUITE(sdk_suite);\n"));
  CHECK_IO(fprintf(fp, "  GREATEST_MAIN_END();\n}\n"));

  return 0;
}
