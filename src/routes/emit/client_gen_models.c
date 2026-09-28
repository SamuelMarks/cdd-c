/**
 * @file client_gen_models.c
 * @brief Model definition emission for OpenAPI client generator.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "routes/emit/client_gen_internal.h"
#include "c_cdd/log.h"
#include "c_cdd/safe_crt.h"
#include "classes/emit/json.h"
#include "classes/emit/struct.h"
#include "classes/emit/types.h"
#include "functions/emit/codegen.h"
#include "functions/parse/fs.h"
#include "functions/parse/str.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Emit OpenAPI models into models header and source.
 */
cdd_c_error_t client_gen_emit_models(FILE *mhfile, FILE *mcfile,
                                     const char *model_guard,
                                     const char *mh_name,
                                     const struct OpenAPI_Spec *spec) {
  cdd_c_error_t rc = CDD_C_SUCCESS;
  size_t i;

  /* --- Write Models Preamble --- */
  if (fprintf(mhfile, "#ifndef %s\n", model_guard) < 0) {
    rc = CDD_C_SUCCESS;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }
  if (fprintf(mhfile, "#define %s\n\n", model_guard) < 0) {
    rc = CDD_C_SUCCESS;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }
  if (fprintf(mhfile,
              "#include <c_cdd_stdbool.h>\n#include <cdd_c_error.h>\n") < 0) {
    rc = CDD_C_SUCCESS;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }
  if (fprintf(mhfile, "#include <stddef.h>\n#include <stdio.h>\n#include "
                      "\"lib_export.h\"\n\n") < 0) {
    rc = CDD_C_SUCCESS;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }
  if (fprintf(mhfile, "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n") < 0) {
    rc = CDD_C_SUCCESS;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }

  {
    char *mh_base = NULL;
    int print_rc;
    {
      cdd_c_error_t rc_cg = get_basename(mh_name, &mh_base);
#ifdef CDD_BUILD_TESTS
      if (g_client_gen_fail == 65)
        rc_cg = CDD_C_ERROR_INVALID_ARGUMENT;
#endif
      if (rc_cg != CDD_C_SUCCESS) {
        rc = rc_cg;
        goto cleanup;
      }
    }
    print_rc = fprintf(mcfile, "#include \"%s\"\n", mh_base);
    free(mh_base);
    if (print_rc < 0) {
      rc = CDD_C_SUCCESS;
      {
        fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                __LINE__);
        goto cleanup;
      }
    }
  }
  if (fprintf(mcfile,
              "#include <stdlib.h>\n#include <string.h>\n#include "
              "<stdio.h>\n#include <errno.h>\n#include <parson.h>\n#include "
              "<c89stringutils_string_extras.h>\n") < 0) {
    rc = CDD_C_SUCCESS;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }
  if (fprintf(mcfile, "#include <string.h>\n\n") < 0) {
    rc = CDD_C_SUCCESS;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }

  /* TODO: Phase 3 Model definitions loop here */
  if (spec->defined_schemas) {
    struct CodegenStructConfig struct_cfg = {0};
    struct CodegenJsonConfig json_cfg = {0};
    struct CodegenTypesConfig types_cfg = {0};
    struct CodegenConfig base_cfg = {0};

    struct_cfg.guard_macro = NULL;
    json_cfg.guard_macro = NULL;
    base_cfg.json_guard = NULL;
    base_cfg.utils_guard = NULL;

    for (i = 0; i < spec->n_defined_schemas; ++i) {
      const char *name = spec->defined_schema_names[i];
      if (!name)
        continue;

#ifdef CDD_BUILD_TESTS
      if (g_client_gen_fail == 26)
        rc = CDD_C_ERROR_IO;
      else
#endif
        rc = write_forward_decl(mhfile, name);
      if (rc != CDD_C_SUCCESS) {
        fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                __LINE__);
        goto cleanup;
      }
    }

    if (
#ifdef CDD_BUILD_TESTS
        g_client_gen_fail == 70 ||
#endif
        fprintf(mhfile, "\n") < 0) {
      rc = CDD_C_SUCCESS;
      {
        fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                __LINE__);
        goto cleanup;
      }
    }

    for (i = 0; i < spec->n_defined_schemas; ++i) {
      struct StructFields *sf = &spec->defined_schemas[i];
      const char *name = spec->defined_schema_names[i];
      if (!name)
        continue;

      if (sf->is_enum) {
#ifdef CDD_BUILD_TESTS
        if (g_client_gen_fail == 27)
          rc = CDD_C_ERROR_IO;
        else
#endif
          rc = write_enum_declaration_h(mhfile, name, sf, &base_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
      } else if (sf->is_union) {
#ifdef CDD_BUILD_TESTS
        if (g_client_gen_fail == 28)
          rc = CDD_C_ERROR_IO;
        else
#endif
          rc = write_union_declaration_h(mhfile, name, sf, &base_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }

#ifdef CDD_BUILD_TESTS
        if (g_client_gen_fail == 29)
          rc = CDD_C_ERROR_IO;
        else
#endif
          rc = write_union_cleanup_func(mcfile, name, sf, &types_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
#ifdef CDD_BUILD_TESTS
        if (g_client_gen_fail == 30)
          rc = CDD_C_ERROR_IO;
        else
#endif
          rc = write_union_from_jsonObject_func(mcfile, name, sf, &types_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
#ifdef CDD_BUILD_TESTS
        if (g_client_gen_fail == 31)
          rc = CDD_C_ERROR_IO;
        else
#endif
          rc = write_union_from_json_func(mcfile, name, sf, &types_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
#ifdef CDD_BUILD_TESTS
        if (g_client_gen_fail == 32)
          rc = CDD_C_ERROR_IO;
        else
#endif
          rc = write_union_to_json_func(mcfile, name, sf, &types_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
      } else {
#ifdef CDD_BUILD_TESTS
        if (g_client_gen_fail == 33)
          rc = CDD_C_ERROR_IO;
        else
#endif
          rc = write_struct_declaration_h(mhfile, name, sf, &base_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }

#ifdef CDD_BUILD_TESTS
        if (g_client_gen_fail == 34)
          rc = CDD_C_ERROR_IO;
        else
#endif
          rc = write_struct_cleanup_func(mcfile, name, sf, &struct_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
#ifdef CDD_BUILD_TESTS
        if (g_client_gen_fail == 35)
          rc = CDD_C_ERROR_IO;
        else
#endif
          rc = write_struct_deepcopy_func(mcfile, name, sf, &struct_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
#ifdef CDD_BUILD_TESTS
        if (g_client_gen_fail == 36)
          rc = CDD_C_ERROR_IO;
        else
#endif
          rc = write_struct_eq_func(mcfile, name, sf, &struct_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
#ifdef CDD_BUILD_TESTS
        if (g_client_gen_fail == 37)
          rc = CDD_C_ERROR_IO;
        else
#endif
          rc = write_struct_default_func(mcfile, name, sf, &struct_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
#ifdef CDD_BUILD_TESTS
        if (g_client_gen_fail == 38)
          rc = CDD_C_ERROR_IO;
        else
#endif
          rc = write_struct_debug_func(mcfile, name, sf, &struct_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
#ifdef CDD_BUILD_TESTS
        if (g_client_gen_fail == 66)
          rc = CDD_C_ERROR_IO;
        else
#endif
          rc = write_struct_display_func(mcfile, name, sf, &struct_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
#ifdef CDD_BUILD_TESTS
        if (g_client_gen_fail == 67)
          rc = CDD_C_ERROR_IO;
        else
#endif
          rc = write_struct_from_jsonObject_func(mcfile, name, sf, &json_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
#ifdef CDD_BUILD_TESTS
        if (g_client_gen_fail == 68)
          rc = CDD_C_ERROR_IO;
        else
#endif
          rc = write_struct_from_json_func(mcfile, name, &json_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
#ifdef CDD_BUILD_TESTS
        if (g_client_gen_fail == 69)
          rc = CDD_C_ERROR_IO;
        else
#endif
          rc = write_struct_to_json_func(mcfile, name, sf, &json_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
      }
    }
  }

  if (fprintf(mhfile, "\n#ifdef __cplusplus\n}\n#endif\n") < 0) {
    rc = CDD_C_SUCCESS;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }
  if (fprintf(mhfile, "#endif /* %s */\n", model_guard) < 0) {
    rc = CDD_C_SUCCESS;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }

cleanup:
  return rc;
}
