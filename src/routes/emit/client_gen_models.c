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
    rc = get_basename(mh_name, &mh_base);
    if (rc != CDD_C_SUCCESS) {
      goto cleanup;
    }
    print_rc = fprintf(mcfile, "#include \"%s\"\n", mh_base);
    C_CDD_FREE(mh_base);
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

  /* Phase 3 Model definitions loop implemented */
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

      rc = write_forward_decl(mhfile, name);
      if (rc != CDD_C_SUCCESS) {
        fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                __LINE__);
        goto cleanup;
      }
    }

    if (fprintf(mhfile, "\n") < 0) {
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
        rc = write_enum_declaration_h(mhfile, name, sf, &base_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
      } else if (sf->is_union) {
        rc = write_union_declaration_h(mhfile, name, sf, &base_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }

        rc = write_union_cleanup_func(mcfile, name, sf, &types_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
        rc = write_union_from_jsonObject_func(mcfile, name, sf, &types_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
        rc = write_union_from_json_func(mcfile, name, sf, &types_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
        rc = write_union_to_json_func(mcfile, name, sf, &types_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
      } else {
        rc = write_struct_declaration_h(mhfile, name, sf, &base_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }

        rc = write_struct_cleanup_func(mcfile, name, sf, &struct_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
        rc = write_struct_deepcopy_func(mcfile, name, sf, &struct_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
        rc = write_struct_eq_func(mcfile, name, sf, &struct_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
        rc = write_struct_default_func(mcfile, name, sf, &struct_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
        rc = write_struct_debug_func(mcfile, name, sf, &struct_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
        rc = write_struct_display_func(mcfile, name, sf, &struct_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
        rc = write_struct_from_jsonObject_func(mcfile, name, sf, &json_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
        rc = write_struct_from_json_func(mcfile, name, &json_cfg);
        if (rc != CDD_C_SUCCESS) {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
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
