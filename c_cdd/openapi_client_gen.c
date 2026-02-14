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

static const struct OpenAPI_ServerVariable *
find_server_variable(const struct OpenAPI_Server *srv, const char *name) {
  size_t i;
  if (!srv || !name || !srv->variables)
    return NULL;
  for (i = 0; i < srv->n_variables; ++i) {
    const struct OpenAPI_ServerVariable *var = &srv->variables[i];
    if (var->name && strcmp(var->name, name) == 0)
      return var;
  }
  return NULL;
}

static char *render_server_url_default(const struct OpenAPI_Server *srv) {
  const char *url;
  size_t out_len = 0;
  size_t i = 0;
  char *out;

  if (!srv || !srv->url)
    return NULL;
  url = srv->url;

  while (url[i]) {
    if (url[i] == '{') {
      const char *end = strchr(url + i + 1, '}');
      size_t name_len;
      char *name;
      const struct OpenAPI_ServerVariable *var;
      if (!end)
        return NULL;
      name_len = (size_t)(end - (url + i + 1));
      if (name_len == 0)
        return NULL;
      name = (char *)malloc(name_len + 1);
      if (!name)
        return NULL;
      memcpy(name, url + i + 1, name_len);
      name[name_len] = '\0';
      var = find_server_variable(srv, name);
      free(name);
      if (!var || !var->default_value)
        return NULL;
      out_len += strlen(var->default_value);
      i = (size_t)(end - url) + 1;
      continue;
    }
    out_len++;
    i++;
  }

  out = (char *)malloc(out_len + 1);
  if (!out)
    return NULL;

  i = 0;
  {
    size_t out_pos = 0;
    while (url[i]) {
      if (url[i] == '{') {
        const char *end = strchr(url + i + 1, '}');
        size_t name_len;
        char *name;
        const struct OpenAPI_ServerVariable *var;
        if (!end) {
          free(out);
          return NULL;
        }
        name_len = (size_t)(end - (url + i + 1));
        if (name_len == 0) {
          free(out);
          return NULL;
        }
        name = (char *)malloc(name_len + 1);
        if (!name) {
          free(out);
          return NULL;
        }
        memcpy(name, url + i + 1, name_len);
        name[name_len] = '\0';
        var = find_server_variable(srv, name);
        free(name);
        if (!var || !var->default_value) {
          free(out);
          return NULL;
        }
        memcpy(out + out_pos, var->default_value, strlen(var->default_value));
        out_pos += strlen(var->default_value);
        i = (size_t)(end - url) + 1;
        continue;
      }
      out[out_pos++] = url[i++];
    }
    out[out_pos] = '\0';
  }

  return out;
}

static char *escape_c_string_literal(const char *s) {
  size_t i;
  size_t out_len = 0;
  char *out;
  size_t pos = 0;

  if (!s)
    return NULL;
  for (i = 0; s[i]; ++i) {
    switch (s[i]) {
    case '\\':
    case '\"':
      out_len += 2;
      break;
    case '\n':
    case '\r':
    case '\t':
      out_len += 2;
      break;
    default:
      out_len += 1;
      break;
    }
  }
  out = (char *)malloc(out_len + 1);
  if (!out)
    return NULL;
  for (i = 0; s[i]; ++i) {
    switch (s[i]) {
    case '\\':
      out[pos++] = '\\';
      out[pos++] = '\\';
      break;
    case '\"':
      out[pos++] = '\\';
      out[pos++] = '\"';
      break;
    case '\n':
      out[pos++] = '\\';
      out[pos++] = 'n';
      break;
    case '\r':
      out[pos++] = '\\';
      out[pos++] = 'r';
      break;
    case '\t':
      out[pos++] = '\\';
      out[pos++] = 't';
      break;
    default:
      out[pos++] = s[i];
      break;
    }
  }
  out[pos] = '\0';
  return out;
}

static const struct OpenAPI_Server *
select_operation_server(const struct OpenAPI_Path *path,
                        const struct OpenAPI_Operation *op) {
  if (op && op->servers && op->n_servers > 0)
    return &op->servers[0];
  if (path && path->servers && path->n_servers > 0)
    return &path->servers[0];
  return NULL;
}

static char *build_base_url_literal(const char *url) {
  char *escaped = NULL;
  char *literal = NULL;
  size_t len;

  if (!url)
    return NULL;
  escaped = escape_c_string_literal(url);
  if (!escaped)
    return NULL;
  len = strlen(escaped) + 3;
  literal = (char *)malloc(len);
  if (!literal) {
    free(escaped);
    return NULL;
  }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  sprintf_s(literal, len, "\"%s\"", escaped);
#else
  sprintf(literal, "\"%s\"", escaped);
#endif
  free(escaped);
  return literal;
}

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

static int param_keys_match(const struct OpenAPI_Parameter *a,
                            const struct OpenAPI_Parameter *b) {
  if (!a || !b || !a->name || !b->name)
    return 0;
  return (a->in == b->in) && (strcmp(a->name, b->name) == 0);
}

static int build_effective_parameters(const struct OpenAPI_Path *path,
                                      const struct OpenAPI_Operation *op,
                                      struct OpenAPI_Parameter **out_params,
                                      size_t *out_count) {
  size_t cap = 0;
  size_t count = 0;
  struct OpenAPI_Parameter *params = NULL;

  if (!out_params || !out_count)
    return EINVAL;

  *out_params = NULL;
  *out_count = 0;

  if (path)
    cap += path->n_parameters;
  if (op)
    cap += op->n_parameters;

  if (cap == 0)
    return 0;

  params = (struct OpenAPI_Parameter *)calloc(cap, sizeof(*params));
  if (!params)
    return ENOMEM;

  if (path && path->parameters) {
    size_t i;
    for (i = 0; i < path->n_parameters; ++i) {
      params[count++] = path->parameters[i];
    }
  }

  if (op && op->parameters) {
    size_t i;
    for (i = 0; i < op->n_parameters; ++i) {
      size_t k;
      int replaced = 0;
      for (k = 0; k < count; ++k) {
        if (param_keys_match(&params[k], &op->parameters[i])) {
          params[k] = op->parameters[i];
          replaced = 1;
          break;
        }
      }
      if (!replaced) {
        params[count++] = op->parameters[i];
      }
    }
  }

  *out_params = params;
  *out_count = count;
  return 0;
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
  CHECK_IO(fprintf(fp, "#include \"url_utils.h\"\n"));
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
static int write_lifecycle_funcs(FILE *h, FILE *c, const char *prefix,
                                 const struct OpenAPI_Spec *spec) {
  char *default_url = NULL;
  char *default_url_escaped = NULL;
  const char *default_url_literal = NULL;

  if (spec && spec->servers && spec->n_servers > 0 && spec->servers[0].url) {
    default_url = render_server_url_default(&spec->servers[0]);
    if (default_url)
      default_url_escaped = escape_c_string_literal(default_url);
    if (default_url_escaped)
      default_url_literal = default_url_escaped;
  } else {
    default_url_literal = "/";
  }
  /* Header */
  CHECK_IO(fprintf(h, "/**\n * @brief Initialize the API Client.\n"
                      " * @param[out] client The client struct to initialize.\n"
                      " * @param[in] base_url The API base URL (or NULL to use"
                      " the default server URL).\n"
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
  if (default_url_literal) {
    CHECK_IO(fprintf(c, "  const char *default_url = \"%s\";\n",
                     default_url_literal));
    CHECK_IO(fprintf(c, "  if (!base_url || base_url[0] == '\\0') {\n"));
    CHECK_IO(fprintf(c, "    base_url = default_url;\n"));
    CHECK_IO(fprintf(c, "  }\n"));
  }
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

  if (default_url)
    free(default_url);
  if (default_url_escaped)
    free(default_url_escaped);
  return 0;
}

/**
 * @brief Generate DocBlock for an operation.
 */
static int write_docblock(FILE *fp, const struct OpenAPI_Operation *op) {
  size_t i;
  CHECK_IO(fprintf(fp, "/**\n"));
  if (op->summary) {
    CHECK_IO(fprintf(fp, " * @brief %s\n", op->summary));
  } else if (op->operation_id) {
    CHECK_IO(fprintf(fp, " * @brief %s\n", op->operation_id));
  } else {
    CHECK_IO(fprintf(fp, " * @brief (Unnamed Operation)\n"));
  }
  if (op->description) {
    CHECK_IO(fprintf(fp, " * @details %s\n", op->description));
  }
  if (op->deprecated) {
    CHECK_IO(fprintf(fp, " * @deprecated\n"));
  }

  CHECK_IO(fprintf(fp, " * @param[in] ctx Client context.\n"));

  for (i = 0; i < op->n_parameters; ++i) {
    if (op->parameters[i].name) {
      const char *loc = "Unknown";
      switch (op->parameters[i].in) {
      case OA_PARAM_IN_QUERY:
        loc = "Query";
        break;
      case OA_PARAM_IN_QUERYSTRING:
        loc = "Querystring";
        break;
      case OA_PARAM_IN_PATH:
        loc = "Path";
        break;
      case OA_PARAM_IN_HEADER:
        loc = "Header";
        break;
      case OA_PARAM_IN_COOKIE:
        loc = "Cookie";
        break;
      default:
        break;
      }
      CHECK_IO(fprintf(fp, " * @param[in] %s Parameter (%s).\n",
                       op->parameters[i].name, loc));
    }
  }
  CHECK_IO(fprintf(fp, " * @param[out] api_error Optional pointer to receive "
                       "detailed error info on failure.\n"));

  CHECK_IO(fprintf(fp, " * @return 0 on success, or error code.\n"));
  CHECK_IO(fprintf(fp, " */\n"));
  return 0;
}

static int emit_operation(FILE *hfile, FILE *cfile,
                          const struct OpenAPI_Path *path,
                          const struct OpenAPI_Operation *op,
                          const struct OpenAPI_Spec *spec,
                          const struct OpenApiClientConfig *config,
                          const char *prefix) {
  struct OpenAPI_Operation effective_op;
  struct OpenAPI_Parameter *effective_params = NULL;
  size_t effective_count = 0;
  struct CodegenSigConfig sig_cfg;
  char *sanitized_group = NULL;
  char *full_group = NULL;
  char *override_url = NULL;
  char *base_url_expr = NULL;
  const struct OpenAPI_Server *server_override = NULL;
  int merge_rc;
  int rc = 0;

#define CHECK_IO_CLEANUP(x)                                                    \
  do {                                                                         \
    if ((x) < 0) {                                                             \
      rc = EIO;                                                                \
      goto cleanup;                                                            \
    }                                                                          \
  } while (0)

  if (!hfile || !cfile || !path || !op || !config || !prefix)
    return EINVAL;

  memset(&sig_cfg, 0, sizeof(sig_cfg));
  sig_cfg.prefix = prefix;

  merge_rc =
      build_effective_parameters(path, op, &effective_params, &effective_count);
  if (merge_rc != 0)
    return merge_rc;

  effective_op = *op;
  effective_op.parameters = effective_params;
  effective_op.n_parameters = effective_count;

  /* Determine Group Name from Tags and Namespace */
  if (effective_op.n_tags > 0 && effective_op.tags[0]) {
    sanitized_group = sanitize_tag(effective_op.tags[0]);
    if (!sanitized_group) {
      rc = ENOMEM;
      goto cleanup;
    }
  }

  if (config->namespace_prefix && sanitized_group) {
    /* Name: Namespace_Tag */
    full_group =
        malloc(strlen(config->namespace_prefix) + strlen(sanitized_group) + 2);
    if (!full_group) {
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
      rc = ENOMEM;
      goto cleanup;
    }
  }

  if (full_group) {
    sig_cfg.group_name = full_group;
  }

  server_override = select_operation_server(path, op);
  if (server_override && server_override->url) {
    override_url = render_server_url_default(server_override);
    if (override_url) {
      base_url_expr = build_base_url_literal(override_url);
      if (!base_url_expr) {
        rc = ENOMEM;
        goto cleanup;
      }
    }
  }

  /* 1. Header: DocBlock + Prototype */
  if ((rc = write_docblock(hfile, &effective_op)) != 0)
    goto cleanup;

  sig_cfg.include_semicolon = 1;
  if ((rc = codegen_client_write_signature(hfile, &effective_op, &sig_cfg)) !=
      0)
    goto cleanup;
  CHECK_IO_CLEANUP(fprintf(hfile, "\n"));

  /* 2. Source: Implementation */
  sig_cfg.include_semicolon = 0;
  if ((rc = codegen_client_write_signature(cfile, &effective_op, &sig_cfg)) !=
      0)
    goto cleanup;

  /* Body generation (Passing spec for security lookup) */
  if ((rc = codegen_client_write_body(cfile, &effective_op, spec, path->route,
                                      base_url_expr)) != 0)
    goto cleanup;

  CHECK_IO_CLEANUP(fprintf(cfile, "\n"));

cleanup:
  if (sanitized_group)
    free(sanitized_group);
  if (full_group)
    free(full_group);
  if (effective_params)
    free(effective_params);
  if (override_url)
    free(override_url);
  if (base_url_expr)
    free(base_url_expr);

#undef CHECK_IO_CLEANUP

  return rc;
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
  if ((rc = write_lifecycle_funcs(hfile, cfile, prefix, spec)) != 0)
    goto cleanup;

  /* --- Iterate Operations --- */
  for (i = 0; i < spec->n_paths; ++i) {
    struct OpenAPI_Path *path = &spec->paths[i];
    for (j = 0; j < path->n_operations; ++j) {
      struct OpenAPI_Operation *op = &path->operations[j];
      rc = emit_operation(hfile, cfile, path, op, spec, config, prefix);
      if (rc != 0)
        goto cleanup;
    }
    for (j = 0; j < path->n_additional_operations; ++j) {
      struct OpenAPI_Operation *op = &path->additional_operations[j];
      rc = emit_operation(hfile, cfile, path, op, spec, config, prefix);
      if (rc != 0)
        goto cleanup;
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
