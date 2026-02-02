/**
 * @file openapi_client_gen.c
 * @brief Implementation of the OpenAPI Client Generator.
 *
 * Generates client code including the standard `ApiError` struct and its
 * implementation for RFC 7807 support. Uses tags from the specification
 * combined with optional global namespaces to generate distinct API function
 * groups (`Namespace_Resource_prefix_operation`).
 *
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen_client_body.h"
#include "codegen_client_sig.h"
#include "openapi_client_gen.h"
#include "str_utils.h"

/* Helper macro for I/O checking */
#define CHECK_IO(x)                                                            \
  do {                                                                         \
    if ((x) < 0)                                                               \
      return EIO;                                                              \
  } while (0)

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#endif

/**
 * @brief Generate a sanitized uppercase Include Guard macro.
 */
static char *generate_guard(const char *base) {
  char *g;
  size_t len = strlen(base);
  size_t i;

  g = malloc(len + 3); /* + _H + null */
  if (!g)
    return NULL;

  for (i = 0; i < len; ++i) {
    if (isalnum((unsigned char)base[i])) {
      g[i] = (char)toupper((unsigned char)base[i]);
    } else {
      g[i] = '_';
    }
  }
  g[len] = '_';
  g[len + 1] = 'H';
  g[len + 2] = '\0';
  return g;
}

/**
 * @brief Derive the model header name if not provided.
 */
static char *derive_model_header(const char *base) {
  char *m;
  size_t len = strlen(base) + 10; /* _models.h */
  m = malloc(len + 1);
  if (!m)
    return NULL;
  sprintf(m, "%s_models.h", base);
  return m;
}

/**
 * @brief Sanitize a tag string to be a valid C identifier part.
 * Converts non-alphanumeric characters to underscores.
 * Capitalizes the first letter for style matching (e.g. "pet" -> "Pet").
 *
 * @param tag The tag string from the spec.
 * @return Allocated string with sanitized name, or NULL on error.
 */
static char *sanitize_tag(const char *tag) {
  char *s;
  size_t i;
  if (!tag)
    return NULL;
  s = strdup(tag);
  if (!s)
    return NULL;

  if (s[0] && islower((unsigned char)s[0])) {
    s[0] = (char)toupper((unsigned char)s[0]);
  }

  for (i = 0; s[i]; ++i) {
    if (!isalnum((unsigned char)s[i])) {
      s[i] = '_';
    }
  }
  return s;
}

/**
 * @brief Write standard includes to the header file.
 * Defines `struct ApiError` for standardized error handling.
 */
static int write_header_preamble(FILE *fp, const char *guard,
                                 const char *model_decl) {
  CHECK_IO(fprintf(fp, "#ifndef %s\n", guard));
  CHECK_IO(fprintf(fp, "#define %s\n\n", guard));

  CHECK_IO(fprintf(fp, "#include <stdlib.h>\n"));
  CHECK_IO(fprintf(fp, "#include <stdio.h>\n"));
  CHECK_IO(fprintf(fp, "#include \"http_types.h\"\n"));
  if (model_decl) {
    CHECK_IO(fprintf(fp, "#include \"%s\"\n", model_decl));
  }
  CHECK_IO(fprintf(fp, "\n#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n"));

  /* Define ApiError struct (RFC 7807 inspired) */
  CHECK_IO(fprintf(fp, "/**\n * @brief Standardized API Error structure "
                       "(Problem Details).\n */\n"
                       "struct ApiError {\n"
                       "  char *type;\n"
                       "  char *title;\n"
                       "  int status;\n"
                       "  char *detail;\n"
                       "  char *instance;\n"
                       "  char *raw_body;\n"
                       "};\n\n"
                       "void ApiError_cleanup(struct ApiError *err);\n"
                       "\n"));

  return 0;
}

/**
 * @brief Write standard includes to the implementation file.
 */
static int write_source_preamble(FILE *fp, const char *header_name) {
  CHECK_IO(fprintf(fp, "#include <stdlib.h>\n"));
  CHECK_IO(fprintf(fp, "#include <string.h>\n"));
  CHECK_IO(fprintf(fp, "#include <stdio.h>\n"));
  CHECK_IO(
      fprintf(fp, "#include <parson.h>\n")); /* Needed for ApiError parsing */
  CHECK_IO(fprintf(fp, "#include \"url_utils.h\"\n"));

  /* Backend selection */
  CHECK_IO(fprintf(fp, "#ifdef USE_WININET\n"));
  CHECK_IO(fprintf(fp, "#include \"http_wininet.h\"\n"));
  CHECK_IO(fprintf(fp, "#elif defined(USE_WINHTTP)\n"));
  CHECK_IO(fprintf(fp, "#include \"http_winhttp.h\"\n"));
  CHECK_IO(fprintf(fp, "#else\n"));
  CHECK_IO(fprintf(fp, "#include \"http_curl.h\"\n"));
  CHECK_IO(fprintf(fp, "#endif\n\n"));

  CHECK_IO(fprintf(fp, "#include \"%s\"\n\n", header_name));

  /* Compatibility defines */
  CHECK_IO(fprintf(fp, "#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)\n"
                       "#define strdup _strdup\n"
                       "#endif\n\n"));

  return 0;
}

/**
 * @brief Write the _init and _cleanup factory functions with macro selection.
 * Also writes ApiError implementation.
 */
static int write_lifecycle_funcs(FILE *h, FILE *c, const char *prefix) {
  /* Header */
  CHECK_IO(fprintf(h, "/**\n * @brief Initialize the API Client.\n"
                      " * @param[out] client The client struct to initialize.\n"
                      " * @param[in] base_url The API base URL.\n"
                      " * @return 0 on success.\n */\n"));
  CHECK_IO(fprintf(h,
                   "int %sinit(struct HttpClient *client, const char "
                   "*base_url);\n\n",
                   prefix));

  CHECK_IO(fprintf(h, "/**\n * @brief Cleanup the API Client.\n */\n"));
  CHECK_IO(
      fprintf(h, "void %scleanup(struct HttpClient *client);\n\n", prefix));

  /* Source */

  /* ApiError implementation */
  CHECK_IO(fprintf(c, "void ApiError_cleanup(struct ApiError *err) {\n"
                      "  if (!err) return;\n"
                      "  if(err->type) free(err->type);\n"
                      "  if(err->title) free(err->title);\n"
                      "  if(err->detail) free(err->detail);\n"
                      "  if(err->instance) free(err->instance);\n"
                      "  if(err->raw_body) free(err->raw_body);\n"
                      "  free(err);\n"
                      "}\n\n"));

  /* Helper to parse ApiError (Internal).
     Split large string literal to avoid C90 warnings. */
  CHECK_IO(fprintf(
      c, "static int ApiError_from_json(const char *json, struct ApiError "
         "**out) {\n"
         "  JSON_Value *root;\n"
         "  JSON_Object *obj;\n"
         "  if(!json || !out) return 22; /* EINVAL */\n"
         "  *out = calloc(1, sizeof(struct ApiError));\n"
         "  if(!*out) return 12; /* ENOMEM */\n"
         "  (*out)->raw_body = strdup(json);\n"
         "  root = json_parse_string(json);\n"));
  CHECK_IO(fprintf(
      c, "  if(!root) return 0; /* Not JSON, return strict success but object "
         "only has raw_body */\n"
         "  obj = json_value_get_object(root);\n"
         "  if(obj) {\n"
         "    if(json_object_has_value(obj, \"type\")) (*out)->type = "
         "strdup(json_object_get_string(obj, \"type\"));\n"
         "    if(json_object_has_value(obj, \"title\")) (*out)->title = "
         "strdup(json_object_get_string(obj, \"title\"));\n"
         "    if(json_object_has_value(obj, \"detail\")) (*out)->detail = "
         "strdup(json_object_get_string(obj, \"detail\"));\n"));
  CHECK_IO(fprintf(
      c, "    if(json_object_has_value(obj, \"instance\")) (*out)->instance = "
         "strdup(json_object_get_string(obj, \"instance\"));\n"
         "    if(json_object_has_value(obj, \"status\")) (*out)->status = "
         "(int)json_object_get_number(obj, \"status\");\n"
         "  }\n"
         "  json_value_free(root);\n"
         "  return 0;\n"
         "}\n\n"));

  CHECK_IO(fprintf(c,
                   "int %sinit(struct HttpClient *client, const char "
                   "*base_url) {\n",
                   prefix));
  CHECK_IO(fprintf(c, "  int rc;\n"));
  CHECK_IO(fprintf(c, "  if (!client) return 22; /* EINVAL */\n"));
  CHECK_IO(fprintf(c, "  rc = http_client_init(client);\n"));
  CHECK_IO(fprintf(c, "  if (rc != 0) return rc;\n"));
  CHECK_IO(fprintf(c, "  if (base_url) {\n"));
  CHECK_IO(
      fprintf(c, "    client->base_url = malloc(strlen(base_url) + 1);\n"));
  CHECK_IO(fprintf(c, "    if (!client->base_url) return 12; /* ENOMEM */\n"));
  CHECK_IO(fprintf(c, "    strcpy(client->base_url, base_url);\n"));
  CHECK_IO(fprintf(c, "  }\n"));

  /* Transport selection logic */
  CHECK_IO(fprintf(c, "#ifdef USE_WININET\n"));
  CHECK_IO(
      fprintf(c, "  rc = http_wininet_context_init(&client->transport);\n"));
  CHECK_IO(fprintf(c, "  client->send = http_wininet_send;\n"));
  CHECK_IO(fprintf(c, "#elif defined(USE_WINHTTP)\n"));
  CHECK_IO(
      fprintf(c, "  rc = http_winhttp_context_init(&client->transport);\n"));
  CHECK_IO(fprintf(c, "  client->send = http_winhttp_send;\n"));
  CHECK_IO(fprintf(c, "#else /* Default to Libcurl */\n"));
  CHECK_IO(fprintf(c, "  rc = http_curl_context_init(&client->transport);\n"));
  CHECK_IO(fprintf(c, "  client->send = http_curl_send;\n"));
  CHECK_IO(fprintf(c, "#endif\n"));

  CHECK_IO(fprintf(c, "  return rc;\n}\n\n"));

  CHECK_IO(fprintf(c, "void %scleanup(struct HttpClient *client) {\n", prefix));
  CHECK_IO(fprintf(c, "  if (!client) return;\n"));

  CHECK_IO(fprintf(c, "#ifdef USE_WININET\n"));
  CHECK_IO(fprintf(c, "  http_wininet_context_free(client->transport);\n"));
  CHECK_IO(fprintf(c, "#elif defined(USE_WINHTTP)\n"));
  CHECK_IO(fprintf(c, "  http_winhttp_context_free(client->transport);\n"));
  CHECK_IO(fprintf(c, "#else\n"));
  CHECK_IO(fprintf(c, "  http_curl_context_free(client->transport);\n"));
  CHECK_IO(fprintf(c, "#endif\n"));

  CHECK_IO(fprintf(c, "  http_client_free(client);\n}\n\n"));

  return 0;
}

/**
 * @brief Generate DocBlock for an operation.
 */
static int write_docblock(FILE *fp, const struct OpenAPI_Operation *op) {
  size_t i;
  CHECK_IO(fprintf(fp, "/**\n"));
  if (op->operation_id) {
    CHECK_IO(fprintf(fp, " * @brief %s\n", op->operation_id));
  } else {
    CHECK_IO(fprintf(fp, " * @brief (Unnamed Operation)\n"));
  }

  CHECK_IO(fprintf(fp, " * @param[in] ctx Client context.\n"));

  for (i = 0; i < op->n_parameters; ++i) {
    if (op->parameters[i].name) {
      CHECK_IO(fprintf(
          fp, " * @param[in] %s Parameter (%s).\n", op->parameters[i].name,
          op->parameters[i].in == OA_PARAM_IN_QUERY ? "Query" : "Path"));
    }
  }
  CHECK_IO(fprintf(fp, " * @param[out] api_error Optional pointer to receive "
                       "detailed error info on failure.\n"));

  CHECK_IO(fprintf(fp, " * @return 0 on success, or error code.\n"));
  CHECK_IO(fprintf(fp, " */\n"));
  return 0;
}

int openapi_client_generate(const struct OpenAPI_Spec *const spec,
                            const struct OpenApiClientConfig *const config) {
  FILE *hfile = NULL, *cfile = NULL;
  char *h_name = NULL, *c_name = NULL;
  char *guard = NULL, *model_h = NULL;
  const char *prefix = "";
  int rc = 0;
  size_t i, j;

  if (!spec || !config || !config->filename_base)
    return EINVAL;

  /* Prepare filenames */
  h_name = malloc(strlen(config->filename_base) + 3); /* .h */
  c_name = malloc(strlen(config->filename_base) + 3); /* .c */
  if (!h_name || !c_name) {
    rc = ENOMEM;
    goto cleanup;
  }
  sprintf(h_name, "%s.h", config->filename_base);
  sprintf(c_name, "%s.c", config->filename_base);

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  if (fopen_s(&hfile, h_name, "w") != 0)
    hfile = NULL;
  if (fopen_s(&cfile, c_name, "w") != 0)
    cfile = NULL;
#else
  hfile = fopen(h_name, "w");
  cfile = fopen(c_name, "w");
#endif

  if (!hfile || !cfile) {
    rc = EIO;
    goto cleanup;
  }

  /* Prepare configurations */
  if (config->header_guard)
    guard = strdup(config->header_guard);
  else
    guard = generate_guard(config->filename_base);

  if (config->model_header)
    model_h = strdup(config->model_header);
  else
    model_h = derive_model_header(config->filename_base);

  if (config->func_prefix)
    prefix = config->func_prefix;

  if (!guard || !model_h) {
    rc = ENOMEM;
    goto cleanup;
  }

  /* --- Write Preamble --- */
  if ((rc = write_header_preamble(hfile, guard, model_h)) != 0)
    goto cleanup;
  if ((rc = write_source_preamble(cfile, h_name)) != 0)
    goto cleanup;

  /* --- Write Lifecycle --- */
  if ((rc = write_lifecycle_funcs(hfile, cfile, prefix)) != 0)
    goto cleanup;

  /* --- Iterate Operations --- */
  for (i = 0; i < spec->n_paths; ++i) {
    struct OpenAPI_Path *path = &spec->paths[i];
    for (j = 0; j < path->n_operations; ++j) {
      struct OpenAPI_Operation *op = &path->operations[j];
      struct CodegenSigConfig sig_cfg;
      char *sanitized_group = NULL;
      char *full_group = NULL;

      memset(&sig_cfg, 0, sizeof(sig_cfg));
      sig_cfg.prefix = prefix;

      /* Determine Group Name from Tags and Namespace */
      if (op->n_tags > 0 && op->tags[0]) {
        sanitized_group = sanitize_tag(op->tags[0]);
        if (!sanitized_group) {
          rc = ENOMEM;
          goto cleanup;
        }
      }

      if (config->namespace_prefix && sanitized_group) {
        /* Name: Namespace_Tag */
        full_group = malloc(strlen(config->namespace_prefix) +
                            strlen(sanitized_group) + 2);
        if (!full_group) {
          if (sanitized_group)
            free(sanitized_group);
          rc = ENOMEM;
          goto cleanup;
        }
        sprintf(full_group, "%s_%s", config->namespace_prefix, sanitized_group);
      } else if (config->namespace_prefix) {
        /* Name: Namespace */
        full_group = strdup(config->namespace_prefix);
        if (!full_group) {
          rc = ENOMEM;
          goto cleanup;
        }
      } else if (sanitized_group) {
        /* Name: Tag */
        full_group = strdup(sanitized_group);
        if (!full_group) {
          free(sanitized_group);
          rc = ENOMEM;
          goto cleanup;
        }
      }

      if (full_group) {
        sig_cfg.group_name = full_group;
      }

      /* 1. Header: DocBlock + Prototype */
      if ((rc = write_docblock(hfile, op)) != 0) {
        if (sanitized_group)
          free(sanitized_group);
        if (full_group)
          free(full_group);
        goto cleanup;
      }

      sig_cfg.include_semicolon = 1;
      if ((rc = codegen_client_write_signature(hfile, op, &sig_cfg)) != 0) {
        if (sanitized_group)
          free(sanitized_group);
        if (full_group)
          free(full_group);
        goto cleanup;
      }
      CHECK_IO(fprintf(hfile, "\n"));

      /* 2. Source: Implementation */
      sig_cfg.include_semicolon = 0;
      if ((rc = codegen_client_write_signature(cfile, op, &sig_cfg)) != 0) {
        if (sanitized_group)
          free(sanitized_group);
        if (full_group)
          free(full_group);
        goto cleanup;
      }

      /* Body generation (Passing spec for security lookup) */
      if ((rc = codegen_client_write_body(cfile, op, spec, path->route)) != 0) {
        if (sanitized_group)
          free(sanitized_group);
        if (full_group)
          free(full_group);
        goto cleanup;
      }

      CHECK_IO(fprintf(cfile, "\n"));

      if (sanitized_group)
        free(sanitized_group);
      if (full_group)
        free(full_group);
    }
  }

  CHECK_IO(fprintf(hfile, "#ifdef __cplusplus\n}\n#endif\n"));
  CHECK_IO(fprintf(hfile, "#endif /* %s */\n", guard));

cleanup:
  if (hfile)
    fclose(hfile);
  if (cfile)
    fclose(cfile);
  if (h_name)
    free(h_name);
  if (c_name)
    free(c_name);
  if (guard)
    free(guard);
  if (model_h)
    free(model_h);

  return rc;
}
