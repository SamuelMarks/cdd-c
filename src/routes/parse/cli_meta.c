/**
 * @file cli_meta.c
 * @brief Metadata and tag processing routines for C to OpenAPI CLI routes
 * parser.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"
#include "c_cdd/memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/str.h"
#include "openapi/parse/openapi.h"
#include "routes/parse/cli_internal.h"
#include "routes/parse/cli.h"
/* clang-format on */

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif
#endif

/**
 * @brief Checks whether the given path corresponds to a C source or header
 * file.
 *
 * @param[in] path File path to check.
 * @param[out] out_is_source Pointer receiving 1 if source/header file, 0
 * otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL
 * pointers.
 */
cdd_c_error_t is_source_file(const char *path, int *out_is_source) {
  const char *ext;
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cli_fail_is_source_file;
  if (g_cli_fail_is_source_file) {
    g_cli_fail_is_source_file = 0;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  if (!path || !out_is_source)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_source = 0;
  ext = strrchr(path, '.');
  if (!ext)
    return CDD_C_SUCCESS;
  *out_is_source = (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0);
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the spec has tag operation.
 */
cdd_c_error_t spec_has_tag(const struct OpenAPI_Spec *spec, const char *name,
                           int *out_has_tag) {
  size_t i;
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cli_fail_spec_has_tag;
  if (g_cli_fail_spec_has_tag) {
    g_cli_fail_spec_has_tag = 0;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  if (!out_has_tag)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_has_tag = 0;
  if (!spec || !name)
    return CDD_C_SUCCESS;
  for (i = 0; i < spec->n_tags; ++i) {
    if (spec->tags[i].name && strcmp(spec->tags[i].name, name) == 0) {
      *out_has_tag = 1;
      return CDD_C_SUCCESS;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the spec add tag operation.
 */
cdd_c_error_t spec_add_tag(struct OpenAPI_Spec *spec, const char *name) {
  struct OpenAPI_Tag *new_tags;
  struct OpenAPI_Tag *tag;

  if (!spec || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  {
    int has_tag = 0;
    cdd_c_error_t rc_tag = spec_has_tag(spec, name, &has_tag);
    if (rc_tag != CDD_C_SUCCESS)
      return rc_tag;
    if (has_tag)
      return CDD_C_SUCCESS;
  }

  new_tags = (struct OpenAPI_Tag *)C_CDD_REALLOC(
      spec->tags, (spec->n_tags + 1) * sizeof(struct OpenAPI_Tag));
  if (!new_tags)
    return CDD_C_ERROR_MEMORY;
  spec->tags = new_tags;
  tag = &spec->tags[spec->n_tags];
  memset(tag, 0, sizeof(*tag));
  {
    cdd_c_error_t rc = c_cdd_strdup(name, &tag->name);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  spec->n_tags++;

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the spec find tag operation.
 */
cdd_c_error_t spec_find_tag(struct OpenAPI_Spec *spec, const char *name,
                            struct OpenAPI_Tag **_out_val) {
  size_t i;
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cli_fail_spec_find_tag;
  if (g_cli_fail_spec_find_tag) {
    g_cli_fail_spec_find_tag = 0;
    return CDD_C_ERROR_MEMORY;
  }
#endif

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *_out_val = NULL;

  if (!spec || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < spec->n_tags; ++i) {
    if (spec->tags[i].name && strcmp(spec->tags[i].name, name) == 0) {
      *_out_val = &spec->tags[i];
      return CDD_C_SUCCESS;
    }
  }
  *_out_val = NULL;
  return CDD_C_ERROR_UNKNOWN;
}

/**
 * @brief Adds or sets str if missing.
 */
cdd_c_error_t set_str_if_missing(char **dst, const char *src) {
  if (!src || !*src)
    return CDD_C_SUCCESS;
  if (!*dst) {
    cdd_c_error_t rc = c_cdd_strdup(src, dst);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return CDD_C_SUCCESS;
  }
  if (strcmp(*dst, src) != 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  return CDD_C_SUCCESS;
}

/**
 * @brief Frees the memory associated with openapi server variables.
 */
void free_openapi_server_variables(struct OpenAPI_Server *srv) {
  size_t i;
  if (!srv || !srv->variables)
    return;
  for (i = 0; i < srv->n_variables; ++i) {
    size_t e;
    struct OpenAPI_ServerVariable *var = &srv->variables[i];
    if (var->name)
      C_CDD_FREE(var->name);
    if (var->default_value)
      C_CDD_FREE(var->default_value);
    if (var->description)
      C_CDD_FREE(var->description);
    if (var->enum_values) {
      for (e = 0; e < var->n_enum_values; ++e) {
        C_CDD_FREE(var->enum_values[e]);
      }
      C_CDD_FREE(var->enum_values);
    }
  }
  C_CDD_FREE(srv->variables);
  srv->variables = NULL;
  srv->n_variables = 0;
}

/**
 * @brief Creates a deep copy of doc server variables.
 */
cdd_c_error_t copy_doc_server_variables(struct OpenAPI_Server *dst,
                                        const struct DocServer *src) {
  size_t i;
  cdd_c_error_t rc;

  if (!dst || !src)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (src->n_variables == 0) {
    dst->variables = NULL;
    dst->n_variables = 0;
    return CDD_C_SUCCESS;
  }

  dst->variables = (struct OpenAPI_ServerVariable *)C_CDD_CALLOC(
      src->n_variables, sizeof(struct OpenAPI_ServerVariable));
  if (!dst->variables)
    return CDD_C_ERROR_MEMORY;
  dst->n_variables = src->n_variables;

  for (i = 0; i < src->n_variables; ++i) {
    size_t e;
    int found_default = 0;
    const struct DocServerVar *sv = &src->variables[i];
    struct OpenAPI_ServerVariable *dv = &dst->variables[i];

    if (!sv->name || !sv->default_value) {
      free_openapi_server_variables(dst);
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }

    rc = c_cdd_strdup(sv->name, &dv->name);
    if (rc != CDD_C_SUCCESS) {
      free_openapi_server_variables(dst);
      return rc;
    }
    rc = c_cdd_strdup(sv->default_value, &dv->default_value);
    if (rc != CDD_C_SUCCESS) {
      free_openapi_server_variables(dst);
      return rc;
    }
    if (sv->description) {
      rc = c_cdd_strdup(sv->description, &dv->description);
      if (rc != CDD_C_SUCCESS) {
        free_openapi_server_variables(dst);
        return rc;
      }
    }
    if (sv->enum_values && sv->n_enum_values > 0) {
      dv->enum_values =
          (char **)C_CDD_CALLOC(sv->n_enum_values, sizeof(char *));
      if (!dv->enum_values) {
        free_openapi_server_variables(dst);
        return CDD_C_ERROR_MEMORY;
      }
      dv->n_enum_values = sv->n_enum_values;
      for (e = 0; e < sv->n_enum_values; ++e) {
        rc = c_cdd_strdup(sv->enum_values[e], &dv->enum_values[e]);
        if (rc != CDD_C_SUCCESS) {
          free_openapi_server_variables(dst);
          return rc;
        }
        if (strcmp(sv->enum_values[e], sv->default_value) == 0)
          found_default = 1;
      }
      if (!found_default) {
        free_openapi_server_variables(dst);
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the append root servers operation.
 */
cdd_c_error_t append_root_servers(struct OpenAPI_Spec *spec,
                                  const struct DocMetadata *meta) {
  size_t i;
  if (!spec || !meta || meta->n_servers == 0)
    return CDD_C_SUCCESS;
  {
    struct OpenAPI_Server *new_servers = (struct OpenAPI_Server *)C_CDD_REALLOC(
        spec->servers,
        (spec->n_servers + meta->n_servers) * sizeof(struct OpenAPI_Server));
    if (!new_servers)
      return CDD_C_ERROR_MEMORY;
    spec->servers = new_servers;
  }
  for (i = 0; i < meta->n_servers; ++i) {
    const struct DocServer *src = &meta->servers[i];
    struct OpenAPI_Server *dst = &spec->servers[spec->n_servers + i];
    memset(dst, 0, sizeof(*dst));
    if (src->url) {
      cdd_c_error_t rc_str = c_cdd_strdup(src->url, &dst->url);
      if (rc_str != CDD_C_SUCCESS) {
        spec->n_servers += i;
        return rc_str;
      }
    }
    if (src->name) {
      cdd_c_error_t rc_str = c_cdd_strdup(src->name, &dst->name);
      if (rc_str != CDD_C_SUCCESS) {
        C_CDD_FREE(dst->url);
        spec->n_servers += i;
        return rc_str;
      }
    }
    if (src->description) {
      cdd_c_error_t rc_str = c_cdd_strdup(src->description, &dst->description);
      if (rc_str != CDD_C_SUCCESS) {
        C_CDD_FREE(dst->url);
        C_CDD_FREE(dst->name);
        spec->n_servers += i;
        return rc_str;
      }
    }
    if (src->n_variables > 0) {
      cdd_c_error_t vrc = copy_doc_server_variables(dst, src);
      if (vrc != CDD_C_SUCCESS) {
        C_CDD_FREE(dst->url);
        C_CDD_FREE(dst->name);
        C_CDD_FREE(dst->description);
        spec->n_servers += i;
        return vrc;
      }
    }
  }
  spec->n_servers += meta->n_servers;

  return CDD_C_SUCCESS;
}

/**
 * @brief Applies doc global meta.
 */
cdd_c_error_t apply_doc_global_meta(struct OpenAPI_Spec *spec,
                                    const struct DocMetadata *meta) {
  cdd_c_error_t rc;
  if (!spec || !meta)
    return CDD_C_SUCCESS;
  if (meta->json_schema_dialect) {
    rc = set_str_if_missing(&spec->json_schema_dialect,
                            meta->json_schema_dialect);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (meta->info_title) {
    rc = set_str_if_missing(&spec->info.title, meta->info_title);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (meta->info_version) {
    rc = set_str_if_missing(&spec->info.version, meta->info_version);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (meta->info_summary) {
    rc = set_str_if_missing(&spec->info.summary, meta->info_summary);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (meta->info_description) {
    rc = set_str_if_missing(&spec->info.description, meta->info_description);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (meta->terms_of_service) {
    rc = set_str_if_missing(&spec->info.terms_of_service,
                            meta->terms_of_service);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (meta->contact_name || meta->contact_url || meta->contact_email) {
    if (meta->contact_name) {
      rc = set_str_if_missing(&spec->info.contact.name, meta->contact_name);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (meta->contact_url) {
      rc = set_str_if_missing(&spec->info.contact.url, meta->contact_url);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (meta->contact_email) {
      rc = set_str_if_missing(&spec->info.contact.email, meta->contact_email);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (meta->license_name || meta->license_url || meta->license_identifier) {
    if (!meta->license_name && !spec->info.license.name)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    if (meta->license_url && meta->license_identifier)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    if (spec->info.license.url && meta->license_identifier)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    if (spec->info.license.identifier && meta->license_url)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    if (meta->license_name) {
      rc = set_str_if_missing(&spec->info.license.name, meta->license_name);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (meta->license_url) {
      rc = set_str_if_missing(&spec->info.license.url, meta->license_url);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (meta->license_identifier) {
      rc = set_str_if_missing(&spec->info.license.identifier,
                              meta->license_identifier);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (meta->external_docs_url) {
    if (!spec->external_docs.url) {
      rc = c_cdd_strdup(meta->external_docs_url, &spec->external_docs.url);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (meta->external_docs_description) {
        rc = c_cdd_strdup(meta->external_docs_description,
                          &spec->external_docs.description);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    } else if (strcmp(spec->external_docs.url, meta->external_docs_url) != 0) {
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    if (!spec->external_docs.description && meta->external_docs_description) {
      rc = c_cdd_strdup(meta->external_docs_description,
                        &spec->external_docs.description);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  rc = append_root_servers(spec, meta);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = append_root_security(spec, meta);
  if (rc != CDD_C_SUCCESS)
    return rc;

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the spec apply tag meta operation.
 */
cdd_c_error_t spec_apply_tag_meta(struct OpenAPI_Spec *spec,
                                  const struct DocTagMeta *meta) {
  struct OpenAPI_Tag *tag;
  cdd_c_error_t rc;
  if (!spec || !meta || !meta->name || !*meta->name)
    return CDD_C_SUCCESS;
  tag = NULL;
  rc = spec_find_tag(spec, meta->name, &tag);
  if (rc != CDD_C_SUCCESS && rc != CDD_C_ERROR_UNKNOWN)
    return rc;
  if (!tag) {
    rc = spec_add_tag(spec, meta->name);
    if (rc != CDD_C_SUCCESS)
      return rc;
    tag = &spec->tags[spec->n_tags - 1];
  }
  if (meta->summary && !tag->summary) {
    rc = c_cdd_strdup(meta->summary, &tag->summary);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (meta->description && !tag->description) {
    rc = c_cdd_strdup(meta->description, &tag->description);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (meta->parent && !tag->parent) {
    rc = c_cdd_strdup(meta->parent, &tag->parent);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (meta->kind && !tag->kind) {
    rc = c_cdd_strdup(meta->kind, &tag->kind);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (meta->external_docs_url && !tag->external_docs.url) {
    rc = c_cdd_strdup(meta->external_docs_url, &tag->external_docs.url);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (meta->external_docs_description && !tag->external_docs.description &&
      tag->external_docs.url) {
    rc = c_cdd_strdup(meta->external_docs_description,
                      &tag->external_docs.description);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Applies doc tag meta.
 */
cdd_c_error_t apply_doc_tag_meta(struct OpenAPI_Spec *spec,
                                 const struct DocMetadata *meta) {
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (!spec || !meta || !meta->tag_meta || meta->n_tag_meta == 0)
    return CDD_C_SUCCESS;
  for (i = 0; i < meta->n_tag_meta; ++i) {
    rc = spec_apply_tag_meta(spec, &meta->tag_meta[i]);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Collects tags from op.
 */
cdd_c_error_t collect_tags_from_op(struct OpenAPI_Spec *spec,
                                   const struct OpenAPI_Operation *op) {
  size_t i;
  if (!spec || !op || !op->tags)
    return CDD_C_SUCCESS;
  for (i = 0; i < op->n_tags; ++i) {
    cdd_c_error_t rc = spec_add_tag(spec, op->tags[i]);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Collects tags from paths.
 */
cdd_c_error_t collect_tags_from_paths(struct OpenAPI_Spec *spec,
                                      const struct OpenAPI_Path *paths,
                                      size_t n_paths) {
  size_t i;
  if (!spec || !paths)
    return CDD_C_SUCCESS;
  for (i = 0; i < n_paths; ++i) {
    size_t j;
    const struct OpenAPI_Path *path = &paths[i];
    for (j = 0; j < path->n_operations; ++j) {
      cdd_c_error_t rc = collect_tags_from_op(spec, &path->operations[j]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    for (j = 0; j < path->n_additional_operations; ++j) {
      cdd_c_error_t rc =
          collect_tags_from_op(spec, &path->additional_operations[j]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Collects spec tags.
 */
cdd_c_error_t collect_spec_tags(struct OpenAPI_Spec *spec) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cli_fail_collect_spec_tags;
  if (g_cli_fail_collect_spec_tags) {
    g_cli_fail_collect_spec_tags = 0;
    return CDD_C_ERROR_MEMORY;
  }
#endif
  if (!spec)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = collect_tags_from_paths(spec, spec->paths, spec->n_paths);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = collect_tags_from_paths(spec, spec->webhooks, spec->n_webhooks);
  if (rc != CDD_C_SUCCESS)
    return rc;

  return CDD_C_SUCCESS;
}

/**
 * @brief Applies all doc metadata (tags, security schemes, and global).
 */
cdd_c_error_t c2openapi_apply_all_doc_meta(struct OpenAPI_Spec *spec,
                                           const struct DocMetadata *meta) {
  cdd_c_error_t rc = apply_doc_tag_meta(spec, meta);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = apply_doc_security_schemes(spec, meta);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return apply_doc_global_meta(spec, meta);
}
