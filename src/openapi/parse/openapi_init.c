/**
 * @file openapi_init.c
 * @brief OpenAPI specification and registry lifecycle initialization.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Executes the openapi spec init operation.
 */
cdd_c_error_t openapi_spec_init(struct OpenAPI_Spec *spec) {
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_openapi_spec_init_fail;
  if (g_openapi_spec_init_fail && --g_openapi_spec_init_fail == 0)
    return CDD_C_ERROR_MEMORY;
#endif
  if (!spec)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  memset(spec, 0, sizeof(struct OpenAPI_Spec));
  spec->openapi_version = NULL;
  spec->is_schema_document = 0;
  spec->schema_root_json = NULL;
  spec->self_uri = NULL;
  spec->retrieval_uri = NULL;
  spec->document_uri = NULL;
  spec->doc_registry = NULL;
  spec->json_schema_dialect = NULL;
  spec->extensions_json = NULL;
  memset(&spec->info, 0, sizeof(spec->info));
  memset(&spec->external_docs, 0, sizeof(spec->external_docs));
  spec->paths_extensions_json = NULL;
  spec->webhooks_extensions_json = NULL;
  spec->components_extensions_json = NULL;
  spec->tags = NULL;
  spec->n_tags = 0;
  spec->security = NULL;
  spec->n_security = 0;
  spec->security_set = 0;
  spec->servers = NULL;
  spec->n_servers = 0;
  spec->paths = NULL;
  spec->n_paths = 0;
  spec->webhooks = NULL;
  spec->n_webhooks = 0;
  spec->component_path_items = NULL;
  spec->component_path_item_names = NULL;
  spec->n_component_path_items = 0;
  spec->security_schemes = NULL;
  spec->n_security_schemes = 0;
  spec->component_parameters = NULL;
  spec->component_parameter_names = NULL;
  spec->n_component_parameters = 0;
  spec->component_responses = NULL;
  spec->component_response_names = NULL;
  spec->n_component_responses = 0;
  spec->component_headers = NULL;
  spec->component_header_names = NULL;
  spec->n_component_headers = 0;
  spec->component_request_bodies = NULL;
  spec->component_request_body_names = NULL;
  spec->n_component_request_bodies = 0;
  spec->component_media_types = NULL;
  spec->component_media_type_names = NULL;
  spec->n_component_media_types = 0;
  spec->component_examples = NULL;
  spec->component_example_names = NULL;
  spec->n_component_examples = 0;
  spec->component_links = NULL;
  spec->n_component_links = 0;
  spec->component_callbacks = NULL;
  spec->n_component_callbacks = 0;
  spec->raw_schema_names = NULL;
  spec->raw_schema_json = NULL;
  spec->n_raw_schemas = 0;
  spec->defined_schemas = NULL;
  spec->defined_schema_names = NULL;
  spec->defined_schema_ids = NULL;
  spec->defined_schema_anchors = NULL;
  spec->defined_schema_dynamic_anchors = NULL;
  spec->n_defined_schemas = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the openapi doc registry init operation.
 */
cdd_c_error_t openapi_doc_registry_init(struct OpenAPI_DocRegistry *registry) {
  if (!registry)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  registry->entries = NULL;
  registry->count = 0;
  registry->capacity = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the openapi doc registry free operation.
 */
void openapi_doc_registry_free(struct OpenAPI_DocRegistry *registry) {
  size_t i;
  if (!registry)
    return;
  if (registry->entries) {
    for (i = 0; i < registry->count; ++i) {
      free(registry->entries[i].base_uri);
    }
    free(registry->entries);
  }
  registry->entries = NULL;
  registry->count = 0;
  registry->capacity = 0;
}

/**
 * @brief Executes the openapi doc registry add operation.
 */
cdd_c_error_t openapi_doc_registry_add(struct OpenAPI_DocRegistry *registry,
                                       struct OpenAPI_Spec *spec) {
  size_t _ast_uri_base_len_19 = 0;
  char *_ast_dup_substr_20 = NULL;
  size_t i;
  const char *base_src;
  char *base = NULL;
  struct OpenAPI_DocRegistryEntry *tmp;

  if (!registry || !spec)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  spec->doc_registry = registry;
  base_src = spec->document_uri ? spec->document_uri : spec->self_uri;
  if (!base_src || !*base_src)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  {
    size_t len =
        (uri_base_len(base_src, &_ast_uri_base_len_19), _ast_uri_base_len_19);
    if (len == 0)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    base = (dup_substr(base_src, len, &_ast_dup_substr_20), _ast_dup_substr_20);
  }
  if (!base)
    return CDD_C_ERROR_MEMORY;

  for (i = 0; i < registry->count; ++i) {
    if (registry->entries[i].base_uri &&
        strcmp(registry->entries[i].base_uri, base) == 0) {
      free(base);
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
  }

  if (registry->count == registry->capacity) {
    size_t new_cap = registry->capacity ? registry->capacity * 2 : 4;
    tmp = (struct OpenAPI_DocRegistryEntry *)C_CDD_REALLOC(
        registry->entries, new_cap * sizeof(*registry->entries));
    if (!tmp) {
      free(base);
      return CDD_C_ERROR_MEMORY;
    }
    registry->entries = tmp;
    registry->capacity = new_cap;
  }

  registry->entries[registry->count].base_uri = base;
  registry->entries[registry->count].spec = spec;
  registry->count++;
  return CDD_C_SUCCESS;
}
