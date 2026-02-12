/**
 * @file openapi_loader.c
 * @brief Implementation of OpenAPI extraction logic.
 *
 * Includes logic to parse `explode` and `style` for parameters,
 * `tags` for operation grouping, and full schema definitions.
 *
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "code2schema.h" /* for json_object_to_struct_fields */
#include "openapi_loader.h"
#include "str_utils.h"

/* --- Helper Function Prototypes --- */

static enum OpenAPI_Verb parse_verb(const char *v);
static enum OpenAPI_ParamIn parse_param_in(const char *in);
static enum OpenAPI_Style parse_param_style(const char *s);
static enum OpenAPI_SecurityType parse_security_type(const char *type);
static enum OpenAPI_SecurityIn parse_security_in(const char *in);
static enum OpenAPI_OAuthFlowType parse_oauth_flow_type(const char *flow);
static int parse_any_value(const JSON_Value *val, struct OpenAPI_Any *out);
static void free_any_value(struct OpenAPI_Any *val);
static int parse_any_field(const JSON_Object *obj, const char *key,
                           struct OpenAPI_Any *out, int *out_set);
static int parse_any_array(const JSON_Array *arr, struct OpenAPI_Any **out,
                           size_t *out_count);
static int collect_schema_extras(const JSON_Object *obj,
                                 const char *const *skip_keys,
                                 size_t skip_count, char **out_json);
static int collect_extensions(const JSON_Object *obj, char **out_json);
static int component_key_is_valid(const char *name);
static int validate_component_key_map(const JSON_Object *obj);
static const char *parse_schema_type(const JSON_Object *schema,
                                     int *out_nullable);
struct SchemaConstraintTarget {
  int *has_min;
  double *min_val;
  int *exclusive_min;
  int *has_max;
  double *max_val;
  int *exclusive_max;
  int *has_min_len;
  size_t *min_len;
  int *has_max_len;
  size_t *max_len;
  char **pattern;
  int *has_min_items;
  size_t *min_items;
  int *has_max_items;
  size_t *max_items;
  int *unique_items;
  struct OpenAPI_Any *example;
  int *example_set;
};
static int parse_schema_constraints(const JSON_Object *schema,
                                    struct SchemaConstraintTarget *target);
static int parse_string_enum_array(const JSON_Array *arr, char ***out,
                                   size_t *out_count);
static void free_string_array(char **arr, size_t n);
static int copy_string_array(char ***dst, size_t *dst_count, char *const *src,
                             size_t src_count);
static int parse_example_object(const JSON_Object *ex_obj, const char *name,
                                struct OpenAPI_Example *out,
                                const struct OpenAPI_Spec *spec,
                                int resolve_refs);
static int parse_examples_object(const JSON_Object *examples,
                                 struct OpenAPI_Example **out,
                                 size_t *out_count,
                                 const struct OpenAPI_Spec *spec,
                                 int resolve_refs);
static int parse_media_examples(const JSON_Object *media_obj,
                                struct OpenAPI_Any *example, int *example_set,
                                struct OpenAPI_Example **examples,
                                size_t *n_examples,
                                const struct OpenAPI_Spec *spec,
                                int resolve_refs);
static int parse_oauth_scopes(const JSON_Object *scopes_obj,
                              struct OpenAPI_OAuthScope **out,
                              size_t *out_count);
static int parse_oauth_flows(const JSON_Object *flows_obj,
                             struct OpenAPI_SecurityScheme *out);
static int copy_any_value(struct OpenAPI_Any *dst,
                          const struct OpenAPI_Any *src);
static int copy_schema_ref(struct OpenAPI_SchemaRef *dst,
                           const struct OpenAPI_SchemaRef *src);
static void free_example(struct OpenAPI_Example *ex);
static void free_header(struct OpenAPI_Header *hdr);
static char *clean_ref(const char *full_ref);
static char *json_pointer_unescape(const char *in);
static int parse_info(const JSON_Object *root_obj, struct OpenAPI_Spec *out);
static int parse_external_docs(const JSON_Object *obj,
                               struct OpenAPI_ExternalDocs *out);
static int parse_tags(const JSON_Object *root_obj, struct OpenAPI_Spec *out);
static int parse_server_object(const JSON_Object *srv_obj,
                               struct OpenAPI_Server *out_srv);
static int parse_servers_array(const JSON_Object *parent, const char *key,
                               struct OpenAPI_Server **out_servers,
                               size_t *out_count);
static int parse_security_field(const JSON_Object *obj, const char *key,
                                struct OpenAPI_SecurityRequirementSet **out,
                                size_t *out_count, int *out_set);
static int parse_schema_ref(const JSON_Object *schema,
                            struct OpenAPI_SchemaRef *out);
static int
apply_schema_ref_to_param(struct OpenAPI_Parameter *out_param,
                          const struct OpenAPI_SchemaRef *schema_ref);
static int
apply_schema_ref_to_header(struct OpenAPI_Header *out_hdr,
                           const struct OpenAPI_SchemaRef *schema_ref);
static int parse_header_object(const JSON_Object *hdr_obj,
                               struct OpenAPI_Header *out_hdr,
                               const struct OpenAPI_Spec *spec,
                               int resolve_refs);
static int header_name_is_content_type(const char *name);
static int header_param_is_reserved(const struct OpenAPI_Parameter *param);
static int parse_link_object(const JSON_Object *link_obj,
                             struct OpenAPI_Link *out_link,
                             const struct OpenAPI_Spec *spec, int resolve_refs);
static int parse_headers_object(const JSON_Object *headers,
                                struct OpenAPI_Header **out_headers,
                                size_t *out_count,
                                const struct OpenAPI_Spec *spec,
                                int resolve_refs, int ignore_content_type);
static int parse_parameter_object(const JSON_Object *p_obj,
                                  struct OpenAPI_Parameter *out_param,
                                  const struct OpenAPI_Spec *spec,
                                  int resolve_refs);
static int parse_media_type_object(const JSON_Object *media_obj,
                                   struct OpenAPI_MediaType *out,
                                   const struct OpenAPI_Spec *spec,
                                   int resolve_refs);
static int parse_content_object(const JSON_Object *content,
                                struct OpenAPI_MediaType **out,
                                size_t *out_count,
                                const struct OpenAPI_Spec *spec,
                                int resolve_refs);
static int parse_encoding_object(const JSON_Object *enc_obj,
                                 struct OpenAPI_Encoding *out,
                                 const struct OpenAPI_Spec *spec,
                                 int resolve_refs);
static int parse_encoding_map(const JSON_Object *enc_obj,
                              struct OpenAPI_Encoding **out, size_t *out_count,
                              const struct OpenAPI_Spec *spec,
                              int resolve_refs);
static int parse_encoding_array(const JSON_Array *enc_arr,
                                struct OpenAPI_Encoding **out,
                                size_t *out_count,
                                const struct OpenAPI_Spec *spec,
                                int resolve_refs);
static int param_key_equals(const struct OpenAPI_Parameter *a,
                            const struct OpenAPI_Parameter *b);
static int parse_parameters_array(const JSON_Array *arr,
                                  struct OpenAPI_Parameter **out_params,
                                  size_t *out_count,
                                  const struct OpenAPI_Spec *spec);
static int parse_request_body_object(const JSON_Object *rb_obj,
                                     struct OpenAPI_RequestBody *out_rb,
                                     const struct OpenAPI_Spec *spec,
                                     int resolve_refs);
static int parse_response_object(const JSON_Object *resp_obj,
                                 struct OpenAPI_Response *out_resp,
                                 const struct OpenAPI_Spec *spec,
                                 int resolve_refs);
static int parse_links_object(const JSON_Object *links,
                              struct OpenAPI_Link **out_links,
                              size_t *out_count,
                              const struct OpenAPI_Spec *spec,
                              int resolve_refs);
static int parse_responses(const JSON_Object *responses,
                           struct OpenAPI_Operation *out_op,
                           const struct OpenAPI_Spec *spec);
static int parse_operation(const char *verb_str, const JSON_Object *op_obj,
                           struct OpenAPI_Operation *out_op,
                           const struct OpenAPI_Spec *spec, int is_additional);
static int parse_callback_object(const JSON_Object *cb_obj,
                                 struct OpenAPI_Callback *out_cb,
                                 const struct OpenAPI_Spec *spec,
                                 int resolve_refs);
static int parse_callbacks_object(const JSON_Object *callbacks,
                                  struct OpenAPI_Callback **out_callbacks,
                                  size_t *out_count,
                                  const struct OpenAPI_Spec *spec,
                                  int resolve_refs);
static int parse_servers(const JSON_Object *root_obj, struct OpenAPI_Spec *out);
static int parse_security_schemes(const JSON_Object *components,
                                  struct OpenAPI_Spec *out);
static int parse_component_parameters(const JSON_Object *components,
                                      struct OpenAPI_Spec *out);
static int parse_component_responses(const JSON_Object *components,
                                     struct OpenAPI_Spec *out);
static int parse_component_headers(const JSON_Object *components,
                                   struct OpenAPI_Spec *out);
static int parse_component_request_bodies(const JSON_Object *components,
                                          struct OpenAPI_Spec *out);
static int parse_component_media_types(const JSON_Object *components,
                                       struct OpenAPI_Spec *out);
static int parse_component_examples(const JSON_Object *components,
                                    struct OpenAPI_Spec *out);
static int parse_component_path_items(const JSON_Object *components,
                                      struct OpenAPI_Spec *out);
static int parse_component_links(const JSON_Object *components,
                                 struct OpenAPI_Spec *out);
static int parse_component_callbacks(const JSON_Object *components,
                                     struct OpenAPI_Spec *out);
static int parse_components(const JSON_Object *components,
                            struct OpenAPI_Spec *out);
static int schema_is_string_enum_only(const JSON_Object *schema_obj);
static int schema_is_struct_compatible(const JSON_Value *schema_val,
                                       const JSON_Object *schema_obj);
static int parse_paths_object(const JSON_Object *paths_obj,
                              struct OpenAPI_Path **out_paths,
                              size_t *out_count,
                              const struct OpenAPI_Spec *spec,
                              int require_leading_slash);
static int validate_unique_operation_ids(const struct OpenAPI_Spec *spec);
static int parse_additional_operations(const JSON_Object *path_obj,
                                       struct OpenAPI_Path *path,
                                       const struct OpenAPI_Spec *spec);
static void free_servers_array(struct OpenAPI_Server *servers,
                               size_t n_servers);

/* --- Lifecycle Implementation --- */

void openapi_spec_init(struct OpenAPI_Spec *const spec) {
  if (spec) {
    spec->openapi_version = NULL;
    spec->self_uri = NULL;
    spec->json_schema_dialect = NULL;
    memset(&spec->info, 0, sizeof(spec->info));
    memset(&spec->external_docs, 0, sizeof(spec->external_docs));
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
    spec->n_defined_schemas = 0;
  }
}

static void free_schema_ref_content(struct OpenAPI_SchemaRef *ref) {
  if (!ref)
    return;
  if (ref->ref_name)
    free(ref->ref_name);
  if (ref->ref)
    free(ref->ref);
  if (ref->inline_type)
    free(ref->inline_type);
  if (ref->type_union)
    free_string_array(ref->type_union, ref->n_type_union);
  if (ref->format)
    free(ref->format);
  if (ref->content_type)
    free(ref->content_type);
  if (ref->content_media_type)
    free(ref->content_media_type);
  if (ref->content_encoding)
    free(ref->content_encoding);
  if (ref->items_format)
    free(ref->items_format);
  if (ref->items_type_union)
    free_string_array(ref->items_type_union, ref->n_items_type_union);
  if (ref->items_ref)
    free(ref->items_ref);
  if (ref->items_content_media_type)
    free(ref->items_content_media_type);
  if (ref->items_content_encoding)
    free(ref->items_content_encoding);
  if (ref->summary)
    free(ref->summary);
  if (ref->description)
    free(ref->description);
  if (ref->const_value_set)
    free_any_value(&ref->const_value);
  if (ref->examples) {
    size_t i;
    for (i = 0; i < ref->n_examples; ++i)
      free_any_value(&ref->examples[i]);
    free(ref->examples);
  }
  if (ref->example_set)
    free_any_value(&ref->example);
  if (ref->default_value_set)
    free_any_value(&ref->default_value);
  if (ref->enum_values) {
    size_t i;
    for (i = 0; i < ref->n_enum_values; ++i)
      free_any_value(&ref->enum_values[i]);
    free(ref->enum_values);
  }
  if (ref->schema_extra_json)
    free(ref->schema_extra_json);
  if (ref->items_enum_values) {
    size_t i;
    for (i = 0; i < ref->n_items_enum_values; ++i)
      free_any_value(&ref->items_enum_values[i]);
    free(ref->items_enum_values);
  }
  if (ref->pattern)
    free(ref->pattern);
  if (ref->items_pattern)
    free(ref->items_pattern);
  if (ref->items_example_set)
    free_any_value(&ref->items_example);
  if (ref->items_examples) {
    size_t i;
    for (i = 0; i < ref->n_items_examples; ++i)
      free_any_value(&ref->items_examples[i]);
    free(ref->items_examples);
  }
  if (ref->items_const_value_set)
    free_any_value(&ref->items_const_value);
  if (ref->items_default_value_set)
    free_any_value(&ref->items_default_value);
  if (ref->items_extra_json)
    free(ref->items_extra_json);
  if (ref->multipart_fields) {
    size_t i;
    for (i = 0; i < ref->n_multipart_fields; ++i) {
      if (ref->multipart_fields[i].name)
        free(ref->multipart_fields[i].name);
      if (ref->multipart_fields[i].type)
        free(ref->multipart_fields[i].type);
    }
    free(ref->multipart_fields);
  }
}

static void free_encoding(struct OpenAPI_Encoding *enc) {
  size_t i;
  if (!enc)
    return;
  if (enc->name)
    free(enc->name);
  if (enc->content_type)
    free(enc->content_type);
  if (enc->extensions_json)
    free(enc->extensions_json);
  if (enc->headers) {
    for (i = 0; i < enc->n_headers; ++i) {
      free_header(&enc->headers[i]);
    }
    free(enc->headers);
    enc->headers = NULL;
    enc->n_headers = 0;
  }
  if (enc->encoding) {
    for (i = 0; i < enc->n_encoding; ++i) {
      free_encoding(&enc->encoding[i]);
    }
    free(enc->encoding);
    enc->encoding = NULL;
    enc->n_encoding = 0;
  }
  if (enc->prefix_encoding) {
    for (i = 0; i < enc->n_prefix_encoding; ++i) {
      free_encoding(&enc->prefix_encoding[i]);
    }
    free(enc->prefix_encoding);
    enc->prefix_encoding = NULL;
    enc->n_prefix_encoding = 0;
  }
  if (enc->item_encoding) {
    free_encoding(enc->item_encoding);
    free(enc->item_encoding);
    enc->item_encoding = NULL;
    enc->item_encoding_set = 0;
  }
}

static void free_media_type(struct OpenAPI_MediaType *mt) {
  size_t e;
  if (!mt)
    return;
  if (mt->name)
    free(mt->name);
  if (mt->ref)
    free(mt->ref);
  if (mt->extensions_json)
    free(mt->extensions_json);
  free_schema_ref_content(&mt->schema);
  free_schema_ref_content(&mt->item_schema);
  if (mt->example_set)
    free_any_value(&mt->example);
  if (mt->examples) {
    for (e = 0; e < mt->n_examples; ++e)
      free_example(&mt->examples[e]);
    free(mt->examples);
    mt->examples = NULL;
    mt->n_examples = 0;
  }
  if (mt->encoding) {
    for (e = 0; e < mt->n_encoding; ++e) {
      free_encoding(&mt->encoding[e]);
    }
    free(mt->encoding);
    mt->encoding = NULL;
    mt->n_encoding = 0;
  }
  if (mt->prefix_encoding) {
    for (e = 0; e < mt->n_prefix_encoding; ++e) {
      free_encoding(&mt->prefix_encoding[e]);
    }
    free(mt->prefix_encoding);
    mt->prefix_encoding = NULL;
    mt->n_prefix_encoding = 0;
  }
  if (mt->item_encoding) {
    free_encoding(mt->item_encoding);
    free(mt->item_encoding);
    mt->item_encoding = NULL;
    mt->item_encoding_set = 0;
  }
}

static void free_parameter(struct OpenAPI_Parameter *param) {
  if (!param)
    return;
  if (param->name)
    free(param->name);
  if (param->type)
    free(param->type);
  if (param->content_type)
    free(param->content_type);
  if (param->content_ref)
    free(param->content_ref);
  if (param->content_media_types) {
    size_t i;
    for (i = 0; i < param->n_content_media_types; ++i) {
      free_media_type(&param->content_media_types[i]);
    }
    free(param->content_media_types);
    param->content_media_types = NULL;
    param->n_content_media_types = 0;
  }
  free_schema_ref_content(&param->schema);
  if (param->description)
    free(param->description);
  if (param->extensions_json)
    free(param->extensions_json);
  if (param->items_type)
    free(param->items_type);
  if (param->ref)
    free(param->ref);
  if (param->example_set)
    free_any_value(&param->example);
  if (param->examples) {
    size_t i;
    for (i = 0; i < param->n_examples; ++i) {
      free_example(&param->examples[i]);
    }
    free(param->examples);
    param->examples = NULL;
    param->n_examples = 0;
  }
}

static void free_header(struct OpenAPI_Header *hdr) {
  if (!hdr)
    return;
  if (hdr->name)
    free(hdr->name);
  if (hdr->ref)
    free(hdr->ref);
  if (hdr->description)
    free(hdr->description);
  if (hdr->extensions_json)
    free(hdr->extensions_json);
  if (hdr->content_type)
    free(hdr->content_type);
  if (hdr->content_ref)
    free(hdr->content_ref);
  if (hdr->content_media_types) {
    size_t i;
    for (i = 0; i < hdr->n_content_media_types; ++i) {
      free_media_type(&hdr->content_media_types[i]);
    }
    free(hdr->content_media_types);
    hdr->content_media_types = NULL;
    hdr->n_content_media_types = 0;
  }
  free_schema_ref_content(&hdr->schema);
  if (hdr->type)
    free(hdr->type);
  if (hdr->items_type)
    free(hdr->items_type);
  if (hdr->example_set)
    free_any_value(&hdr->example);
  if (hdr->examples) {
    size_t i;
    for (i = 0; i < hdr->n_examples; ++i) {
      free_example(&hdr->examples[i]);
    }
    free(hdr->examples);
    hdr->examples = NULL;
    hdr->n_examples = 0;
  }
}

static void free_response(struct OpenAPI_Response *resp) {
  size_t i;
  if (!resp)
    return;
  if (resp->code)
    free(resp->code);
  if (resp->summary)
    free(resp->summary);
  if (resp->description)
    free(resp->description);
  if (resp->extensions_json)
    free(resp->extensions_json);
  if (resp->content_type)
    free(resp->content_type);
  if (resp->content_ref)
    free(resp->content_ref);
  if (resp->content_media_types) {
    for (i = 0; i < resp->n_content_media_types; ++i) {
      free_media_type(&resp->content_media_types[i]);
    }
    free(resp->content_media_types);
    resp->content_media_types = NULL;
    resp->n_content_media_types = 0;
  }
  if (resp->ref)
    free(resp->ref);
  if (resp->headers) {
    for (i = 0; i < resp->n_headers; ++i) {
      free_header(&resp->headers[i]);
    }
    free(resp->headers);
    resp->headers = NULL;
    resp->n_headers = 0;
  }
  if (resp->links) {
    for (i = 0; i < resp->n_links; ++i) {
      free_link(&resp->links[i]);
    }
    free(resp->links);
    resp->links = NULL;
    resp->n_links = 0;
  }
  if (resp->example_set)
    free_any_value(&resp->example);
  if (resp->examples) {
    for (i = 0; i < resp->n_examples; ++i) {
      free_example(&resp->examples[i]);
    }
    free(resp->examples);
    resp->examples = NULL;
    resp->n_examples = 0;
  }
  free_schema_ref_content(&resp->schema);
}

static void free_request_body(struct OpenAPI_RequestBody *rb) {
  if (!rb)
    return;
  if (rb->ref)
    free(rb->ref);
  if (rb->description)
    free(rb->description);
  if (rb->extensions_json)
    free(rb->extensions_json);
  if (rb->content_ref)
    free(rb->content_ref);
  if (rb->content_media_types) {
    size_t i;
    for (i = 0; i < rb->n_content_media_types; ++i) {
      free_media_type(&rb->content_media_types[i]);
    }
    free(rb->content_media_types);
    rb->content_media_types = NULL;
    rb->n_content_media_types = 0;
  }
  if (rb->example_set)
    free_any_value(&rb->example);
  if (rb->examples) {
    size_t i;
    for (i = 0; i < rb->n_examples; ++i) {
      free_example(&rb->examples[i]);
    }
    free(rb->examples);
    rb->examples = NULL;
    rb->n_examples = 0;
  }
  free_schema_ref_content(&rb->schema);
}

static void free_any_value(struct OpenAPI_Any *val) {
  if (!val)
    return;
  if (val->type == OA_ANY_STRING && val->string) {
    free(val->string);
  } else if (val->type == OA_ANY_JSON && val->json) {
    free(val->json);
  }
  val->type = OA_ANY_UNSET;
  val->string = NULL;
  val->json = NULL;
}

static void free_link(struct OpenAPI_Link *link) {
  size_t i;
  if (!link)
    return;
  if (link->name)
    free(link->name);
  if (link->ref)
    free(link->ref);
  if (link->summary)
    free(link->summary);
  if (link->description)
    free(link->description);
  if (link->extensions_json)
    free(link->extensions_json);
  if (link->operation_ref)
    free(link->operation_ref);
  if (link->operation_id)
    free(link->operation_id);
  if (link->parameters) {
    for (i = 0; i < link->n_parameters; ++i) {
      if (link->parameters[i].name)
        free(link->parameters[i].name);
      free_any_value(&link->parameters[i].value);
    }
    free(link->parameters);
  }
  if (link->request_body_set)
    free_any_value(&link->request_body);
  if (link->server_set && link->server) {
    free_servers_array(link->server, 1);
    link->server = NULL;
    link->server_set = 0;
  }
}

static void free_operation(struct OpenAPI_Operation *op);
static void free_path_item(struct OpenAPI_Path *p);
static void free_callback(struct OpenAPI_Callback *cb);

static void free_security_requirement(struct OpenAPI_SecurityRequirement *req) {
  size_t i;
  if (!req)
    return;
  if (req->scheme)
    free(req->scheme);
  if (req->scopes) {
    for (i = 0; i < req->n_scopes; ++i) {
      if (req->scopes[i])
        free(req->scopes[i]);
    }
    free(req->scopes);
  }
}

static void
free_security_requirement_set(struct OpenAPI_SecurityRequirementSet *set) {
  size_t i;
  if (!set)
    return;
  if (set->requirements) {
    for (i = 0; i < set->n_requirements; ++i) {
      free_security_requirement(&set->requirements[i]);
    }
    free(set->requirements);
  }
  if (set->extensions_json)
    free(set->extensions_json);
}

static void free_servers_array(struct OpenAPI_Server *servers,
                               size_t n_servers) {
  size_t i;
  if (!servers)
    return;
  for (i = 0; i < n_servers; ++i) {
    if (servers[i].url)
      free(servers[i].url);
    if (servers[i].description)
      free(servers[i].description);
    if (servers[i].name)
      free(servers[i].name);
    if (servers[i].extensions_json)
      free(servers[i].extensions_json);
    if (servers[i].variables) {
      size_t v;
      for (v = 0; v < servers[i].n_variables; ++v) {
        struct OpenAPI_ServerVariable *var = &servers[i].variables[v];
        if (var->name)
          free(var->name);
        if (var->default_value)
          free(var->default_value);
        if (var->description)
          free(var->description);
        if (var->extensions_json)
          free(var->extensions_json);
        if (var->enum_values) {
          size_t e;
          for (e = 0; e < var->n_enum_values; ++e) {
            if (var->enum_values[e])
              free(var->enum_values[e]);
          }
          free(var->enum_values);
        }
      }
      free(servers[i].variables);
    }
  }
  free(servers);
}

static void free_path_item(struct OpenAPI_Path *p) {
  size_t i;
  if (!p)
    return;
  if (p->route)
    free(p->route);
  if (p->ref)
    free(p->ref);
  if (p->summary)
    free(p->summary);
  if (p->description)
    free(p->description);
  if (p->extensions_json)
    free(p->extensions_json);
  if (p->parameters) {
    for (i = 0; i < p->n_parameters; ++i) {
      free_parameter(&p->parameters[i]);
    }
    free(p->parameters);
  }
  if (p->servers) {
    free_servers_array(p->servers, p->n_servers);
    p->servers = NULL;
    p->n_servers = 0;
  }
  if (p->operations) {
    for (i = 0; i < p->n_operations; ++i) {
      free_operation(&p->operations[i]);
    }
    free(p->operations);
  }
  if (p->additional_operations) {
    for (i = 0; i < p->n_additional_operations; ++i) {
      free_operation(&p->additional_operations[i]);
    }
    free(p->additional_operations);
  }
}

static void free_callback(struct OpenAPI_Callback *cb) {
  size_t i;
  if (!cb)
    return;
  if (cb->name)
    free(cb->name);
  if (cb->ref)
    free(cb->ref);
  if (cb->summary)
    free(cb->summary);
  if (cb->description)
    free(cb->description);
  if (cb->extensions_json)
    free(cb->extensions_json);
  if (cb->paths) {
    for (i = 0; i < cb->n_paths; ++i) {
      free_path_item(&cb->paths[i]);
    }
    free(cb->paths);
  }
}

static void free_operation(struct OpenAPI_Operation *op) {
  size_t i;
  if (!op)
    return;
  if (op->method)
    free(op->method);
  if (op->operation_id)
    free(op->operation_id);
  if (op->summary)
    free(op->summary);
  if (op->description)
    free(op->description);
  if (op->extensions_json)
    free(op->extensions_json);

  if (op->tags) {
    for (i = 0; i < op->n_tags; ++i) {
      if (op->tags[i])
        free(op->tags[i]);
    }
    free(op->tags);
  }

  free_schema_ref_content(&op->req_body);
  if (op->req_body_media_types) {
    for (i = 0; i < op->n_req_body_media_types; ++i) {
      free_media_type(&op->req_body_media_types[i]);
    }
    free(op->req_body_media_types);
    op->req_body_media_types = NULL;
    op->n_req_body_media_types = 0;
  }
  if (op->req_body_description)
    free(op->req_body_description);
  if (op->req_body_ref)
    free(op->req_body_ref);
  if (op->external_docs.description)
    free(op->external_docs.description);
  if (op->external_docs.url)
    free(op->external_docs.url);
  if (op->external_docs.extensions_json)
    free(op->external_docs.extensions_json);

  if (op->servers) {
    free_servers_array(op->servers, op->n_servers);
    op->servers = NULL;
    op->n_servers = 0;
  }

  if (op->parameters) {
    for (i = 0; i < op->n_parameters; ++i) {
      free_parameter(&op->parameters[i]);
    }
    free(op->parameters);
  }

  if (op->responses) {
    for (i = 0; i < op->n_responses; ++i) {
      free_response(&op->responses[i]);
    }
    free(op->responses);
  }

  if (op->callbacks) {
    for (i = 0; i < op->n_callbacks; ++i) {
      free_callback(&op->callbacks[i]);
    }
    free(op->callbacks);
    op->callbacks = NULL;
    op->n_callbacks = 0;
  }

  if (op->security) {
    for (i = 0; i < op->n_security; ++i) {
      free_security_requirement_set(&op->security[i]);
    }
    free(op->security);
    op->security = NULL;
    op->n_security = 0;
    op->security_set = 0;
  }
}

void openapi_spec_free(struct OpenAPI_Spec *const spec) {
  size_t i, j;
  if (!spec)
    return;

  if (spec->openapi_version) {
    free(spec->openapi_version);
    spec->openapi_version = NULL;
  }
  if (spec->self_uri) {
    free(spec->self_uri);
    spec->self_uri = NULL;
  }
  if (spec->json_schema_dialect) {
    free(spec->json_schema_dialect);
    spec->json_schema_dialect = NULL;
  }
  if (spec->extensions_json) {
    free(spec->extensions_json);
    spec->extensions_json = NULL;
  }
  if (spec->info.title)
    free(spec->info.title);
  if (spec->info.summary)
    free(spec->info.summary);
  if (spec->info.description)
    free(spec->info.description);
  if (spec->info.terms_of_service)
    free(spec->info.terms_of_service);
  if (spec->info.version)
    free(spec->info.version);
  if (spec->info.extensions_json)
    free(spec->info.extensions_json);
  if (spec->info.contact.name)
    free(spec->info.contact.name);
  if (spec->info.contact.url)
    free(spec->info.contact.url);
  if (spec->info.contact.email)
    free(spec->info.contact.email);
  if (spec->info.contact.extensions_json)
    free(spec->info.contact.extensions_json);
  if (spec->info.license.name)
    free(spec->info.license.name);
  if (spec->info.license.identifier)
    free(spec->info.license.identifier);
  if (spec->info.license.url)
    free(spec->info.license.url);
  if (spec->info.license.extensions_json)
    free(spec->info.license.extensions_json);
  if (spec->external_docs.description)
    free(spec->external_docs.description);
  if (spec->external_docs.url)
    free(spec->external_docs.url);
  if (spec->external_docs.extensions_json)
    free(spec->external_docs.extensions_json);

  if (spec->tags) {
    for (i = 0; i < spec->n_tags; ++i) {
      if (spec->tags[i].name)
        free(spec->tags[i].name);
      if (spec->tags[i].summary)
        free(spec->tags[i].summary);
      if (spec->tags[i].description)
        free(spec->tags[i].description);
      if (spec->tags[i].parent)
        free(spec->tags[i].parent);
      if (spec->tags[i].kind)
        free(spec->tags[i].kind);
      if (spec->tags[i].extensions_json)
        free(spec->tags[i].extensions_json);
      if (spec->tags[i].external_docs.description)
        free(spec->tags[i].external_docs.description);
      if (spec->tags[i].external_docs.url)
        free(spec->tags[i].external_docs.url);
      if (spec->tags[i].external_docs.extensions_json)
        free(spec->tags[i].external_docs.extensions_json);
    }
    free(spec->tags);
    spec->tags = NULL;
    spec->n_tags = 0;
  }

  if (spec->security) {
    for (i = 0; i < spec->n_security; ++i) {
      free_security_requirement_set(&spec->security[i]);
    }
    free(spec->security);
    spec->security = NULL;
    spec->n_security = 0;
    spec->security_set = 0;
  }

  if (spec->servers) {
    free_servers_array(spec->servers, spec->n_servers);
    spec->servers = NULL;
    spec->n_servers = 0;
  }

  if (spec->paths) {
    for (i = 0; i < spec->n_paths; ++i) {
      free_path_item(&spec->paths[i]);
    }
    free(spec->paths);
    spec->paths = NULL;
  }

  if (spec->webhooks) {
    for (i = 0; i < spec->n_webhooks; ++i) {
      free_path_item(&spec->webhooks[i]);
    }
    free(spec->webhooks);
    spec->webhooks = NULL;
  }

  if (spec->component_path_items) {
    for (i = 0; i < spec->n_component_path_items; ++i) {
      free_path_item(&spec->component_path_items[i]);
      if (spec->component_path_item_names)
        free(spec->component_path_item_names[i]);
    }
    free(spec->component_path_items);
    free(spec->component_path_item_names);
    spec->component_path_items = NULL;
    spec->component_path_item_names = NULL;
    spec->n_component_path_items = 0;
  }

  if (spec->security_schemes) {
    for (i = 0; i < spec->n_security_schemes; ++i) {
      if (spec->security_schemes[i].name)
        free(spec->security_schemes[i].name);
      if (spec->security_schemes[i].description)
        free(spec->security_schemes[i].description);
      if (spec->security_schemes[i].scheme)
        free(spec->security_schemes[i].scheme);
      if (spec->security_schemes[i].bearer_format)
        free(spec->security_schemes[i].bearer_format);
      if (spec->security_schemes[i].key_name)
        free(spec->security_schemes[i].key_name);
      if (spec->security_schemes[i].open_id_connect_url)
        free(spec->security_schemes[i].open_id_connect_url);
      if (spec->security_schemes[i].oauth2_metadata_url)
        free(spec->security_schemes[i].oauth2_metadata_url);
      if (spec->security_schemes[i].extensions_json)
        free(spec->security_schemes[i].extensions_json);
      if (spec->security_schemes[i].flows) {
        size_t f;
        for (f = 0; f < spec->security_schemes[i].n_flows; ++f) {
          struct OpenAPI_OAuthFlow *flow = &spec->security_schemes[i].flows[f];
          size_t s;
          if (flow->authorization_url)
            free(flow->authorization_url);
          if (flow->token_url)
            free(flow->token_url);
          if (flow->refresh_url)
            free(flow->refresh_url);
          if (flow->device_authorization_url)
            free(flow->device_authorization_url);
          if (flow->extensions_json)
            free(flow->extensions_json);
          if (flow->scopes) {
            for (s = 0; s < flow->n_scopes; ++s) {
              if (flow->scopes[s].name)
                free(flow->scopes[s].name);
              if (flow->scopes[s].description)
                free(flow->scopes[s].description);
            }
            free(flow->scopes);
          }
        }
        free(spec->security_schemes[i].flows);
        spec->security_schemes[i].flows = NULL;
        spec->security_schemes[i].n_flows = 0;
      }
    }
    free(spec->security_schemes);
    spec->security_schemes = NULL;
  }

  if (spec->component_parameters) {
    for (i = 0; i < spec->n_component_parameters; ++i) {
      free_parameter(&spec->component_parameters[i]);
      free(spec->component_parameter_names[i]);
    }
    free(spec->component_parameters);
    free(spec->component_parameter_names);
    spec->component_parameters = NULL;
    spec->component_parameter_names = NULL;
    spec->n_component_parameters = 0;
  }

  if (spec->component_responses) {
    for (i = 0; i < spec->n_component_responses; ++i) {
      free_response(&spec->component_responses[i]);
      free(spec->component_response_names[i]);
    }
    free(spec->component_responses);
    free(spec->component_response_names);
    spec->component_responses = NULL;
    spec->component_response_names = NULL;
    spec->n_component_responses = 0;
  }

  if (spec->component_headers) {
    for (i = 0; i < spec->n_component_headers; ++i) {
      free_header(&spec->component_headers[i]);
      free(spec->component_header_names[i]);
    }
    free(spec->component_headers);
    free(spec->component_header_names);
    spec->component_headers = NULL;
    spec->component_header_names = NULL;
    spec->n_component_headers = 0;
  }

  if (spec->component_request_bodies) {
    for (i = 0; i < spec->n_component_request_bodies; ++i) {
      free_request_body(&spec->component_request_bodies[i]);
      free(spec->component_request_body_names[i]);
    }
    free(spec->component_request_bodies);
    free(spec->component_request_body_names);
    spec->component_request_bodies = NULL;
    spec->component_request_body_names = NULL;
    spec->n_component_request_bodies = 0;
  }

  if (spec->component_media_types) {
    for (i = 0; i < spec->n_component_media_types; ++i) {
      struct OpenAPI_MediaType *mt = &spec->component_media_types[i];
      free_media_type(mt);
      if (spec->component_media_type_names)
        free(spec->component_media_type_names[i]);
    }
    free(spec->component_media_types);
    free(spec->component_media_type_names);
    spec->component_media_types = NULL;
    spec->component_media_type_names = NULL;
    spec->n_component_media_types = 0;
  }

  if (spec->component_examples) {
    for (i = 0; i < spec->n_component_examples; ++i) {
      free_example(&spec->component_examples[i]);
      if (spec->component_example_names)
        free(spec->component_example_names[i]);
    }
    free(spec->component_examples);
    free(spec->component_example_names);
    spec->component_examples = NULL;
    spec->component_example_names = NULL;
    spec->n_component_examples = 0;
  }

  if (spec->component_links) {
    for (i = 0; i < spec->n_component_links; ++i) {
      free_link(&spec->component_links[i]);
    }
    free(spec->component_links);
    spec->component_links = NULL;
    spec->n_component_links = 0;
  }

  if (spec->component_callbacks) {
    for (i = 0; i < spec->n_component_callbacks; ++i) {
      free_callback(&spec->component_callbacks[i]);
    }
    free(spec->component_callbacks);
    spec->component_callbacks = NULL;
    spec->n_component_callbacks = 0;
  }

  if (spec->raw_schema_names || spec->raw_schema_json) {
    for (i = 0; i < spec->n_raw_schemas; ++i) {
      if (spec->raw_schema_names)
        free(spec->raw_schema_names[i]);
      if (spec->raw_schema_json)
        free(spec->raw_schema_json[i]);
    }
    free(spec->raw_schema_names);
    free(spec->raw_schema_json);
    spec->raw_schema_names = NULL;
    spec->raw_schema_json = NULL;
    spec->n_raw_schemas = 0;
  }

  if (spec->defined_schemas) {
    for (i = 0; i < spec->n_defined_schemas; ++i) {
      struct_fields_free(&spec->defined_schemas[i]);
      free(spec->defined_schema_names[i]);
    }
    free(spec->defined_schemas);
    free(spec->defined_schema_names);
    spec->defined_schemas = NULL;
    spec->defined_schema_names = NULL;
  }

  spec->n_paths = 0;
  spec->n_webhooks = 0;
  spec->n_component_path_items = 0;
  spec->n_security_schemes = 0;
  spec->n_defined_schemas = 0;
  spec->n_raw_schemas = 0;
  spec->n_component_parameters = 0;
  spec->n_component_responses = 0;
  spec->n_component_headers = 0;
  spec->n_component_request_bodies = 0;
  spec->n_component_media_types = 0;
  spec->n_component_examples = 0;
  spec->n_component_links = 0;
  spec->n_component_callbacks = 0;
  spec->n_security = 0;
  spec->security_set = 0;
}

/* --- Parsing Helpers --- */

static enum OpenAPI_Verb parse_verb(const char *const v) {
  if (strcmp(v, "get") == 0)
    return OA_VERB_GET;
  if (strcmp(v, "post") == 0)
    return OA_VERB_POST;
  if (strcmp(v, "put") == 0)
    return OA_VERB_PUT;
  if (strcmp(v, "delete") == 0)
    return OA_VERB_DELETE;
  if (strcmp(v, "patch") == 0)
    return OA_VERB_PATCH;
  if (strcmp(v, "head") == 0)
    return OA_VERB_HEAD;
  if (strcmp(v, "options") == 0)
    return OA_VERB_OPTIONS;
  if (strcmp(v, "trace") == 0)
    return OA_VERB_TRACE;
  if (strcmp(v, "query") == 0)
    return OA_VERB_QUERY;
  return OA_VERB_UNKNOWN;
}

static enum OpenAPI_ParamIn parse_param_in(const char *const in) {
  if (strcmp(in, "path") == 0)
    return OA_PARAM_IN_PATH;
  if (strcmp(in, "query") == 0)
    return OA_PARAM_IN_QUERY;
  if (strcmp(in, "querystring") == 0)
    return OA_PARAM_IN_QUERYSTRING;
  if (strcmp(in, "header") == 0)
    return OA_PARAM_IN_HEADER;
  if (strcmp(in, "cookie") == 0)
    return OA_PARAM_IN_COOKIE;
  return OA_PARAM_IN_UNKNOWN;
}

static enum OpenAPI_Style parse_param_style(const char *const s) {
  if (!s)
    return OA_STYLE_UNKNOWN;
  if (strcmp(s, "form") == 0)
    return OA_STYLE_FORM;
  if (strcmp(s, "simple") == 0)
    return OA_STYLE_SIMPLE;
  if (strcmp(s, "matrix") == 0)
    return OA_STYLE_MATRIX;
  if (strcmp(s, "label") == 0)
    return OA_STYLE_LABEL;
  if (strcmp(s, "spaceDelimited") == 0)
    return OA_STYLE_SPACE_DELIMITED;
  if (strcmp(s, "pipeDelimited") == 0)
    return OA_STYLE_PIPE_DELIMITED;
  if (strcmp(s, "deepObject") == 0)
    return OA_STYLE_DEEP_OBJECT;
  if (strcmp(s, "cookie") == 0)
    return OA_STYLE_COOKIE;
  return OA_STYLE_UNKNOWN;
}

static int component_key_is_valid(const char *const name) {
  size_t i;
  if (!name || !*name)
    return 0;
  for (i = 0; name[i]; ++i) {
    const unsigned char c = (unsigned char)name[i];
    if (!(isalnum(c) || c == '.' || c == '-' || c == '_'))
      return 0;
  }
  return 1;
}

static int validate_component_key_map(const JSON_Object *const obj) {
  size_t i, count;
  if (!obj)
    return 0;
  count = json_object_get_count(obj);
  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(obj, i);
    if (!component_key_is_valid(name))
      return EINVAL;
  }
  return 0;
}

static int header_name_is_content_type(const char *const name) {
  if (!name)
    return 0;
  return c_cdd_str_iequal(name, "Content-Type");
}

static int header_param_is_reserved(const struct OpenAPI_Parameter *param) {
  if (!param || param->in != OA_PARAM_IN_HEADER || !param->name)
    return 0;
  return c_cdd_str_iequal(param->name, "Accept") ||
         c_cdd_str_iequal(param->name, "Content-Type") ||
         c_cdd_str_iequal(param->name, "Authorization");
}

static enum OpenAPI_SecurityType parse_security_type(const char *const type) {
  if (!type)
    return OA_SEC_UNKNOWN;
  if (strcmp(type, "apiKey") == 0)
    return OA_SEC_APIKEY;
  if (strcmp(type, "http") == 0)
    return OA_SEC_HTTP;
  if (strcmp(type, "mutualTLS") == 0)
    return OA_SEC_MUTUALTLS;
  if (strcmp(type, "oauth2") == 0)
    return OA_SEC_OAUTH2;
  if (strcmp(type, "openIdConnect") == 0)
    return OA_SEC_OPENID;
  return OA_SEC_UNKNOWN;
}

static enum OpenAPI_SecurityIn parse_security_in(const char *const in) {
  if (!in)
    return OA_SEC_IN_UNKNOWN;
  if (strcmp(in, "query") == 0)
    return OA_SEC_IN_QUERY;
  if (strcmp(in, "header") == 0)
    return OA_SEC_IN_HEADER;
  if (strcmp(in, "cookie") == 0)
    return OA_SEC_IN_COOKIE;
  return OA_SEC_IN_UNKNOWN;
}

static enum OpenAPI_OAuthFlowType
parse_oauth_flow_type(const char *const flow) {
  if (!flow)
    return OA_OAUTH_FLOW_UNKNOWN;
  if (strcmp(flow, "implicit") == 0)
    return OA_OAUTH_FLOW_IMPLICIT;
  if (strcmp(flow, "password") == 0)
    return OA_OAUTH_FLOW_PASSWORD;
  if (strcmp(flow, "clientCredentials") == 0)
    return OA_OAUTH_FLOW_CLIENT_CREDENTIALS;
  if (strcmp(flow, "authorizationCode") == 0)
    return OA_OAUTH_FLOW_AUTHORIZATION_CODE;
  if (strcmp(flow, "deviceAuthorization") == 0)
    return OA_OAUTH_FLOW_DEVICE_AUTHORIZATION;
  return OA_OAUTH_FLOW_UNKNOWN;
}

static int parse_any_value(const JSON_Value *const val,
                           struct OpenAPI_Any *const out) {
  JSON_Value_Type t;
  const char *s;
  char *json_str;

  if (!val || !out)
    return 0;

  t = json_value_get_type(val);
  out->type = OA_ANY_UNSET;

  switch (t) {
  case JSONString:
    s = json_value_get_string(val);
    out->type = OA_ANY_STRING;
    out->string = c_cdd_strdup(s ? s : "");
    if (!out->string)
      return ENOMEM;
    break;
  case JSONNumber:
    out->type = OA_ANY_NUMBER;
    out->number = json_value_get_number(val);
    break;
  case JSONBoolean:
    out->type = OA_ANY_BOOL;
    out->boolean = json_value_get_boolean(val);
    break;
  case JSONNull:
    out->type = OA_ANY_NULL;
    break;
  case JSONObject:
  case JSONArray:
    json_str = json_serialize_to_string(val);
    if (!json_str)
      return ENOMEM;
    out->type = OA_ANY_JSON;
    out->json = c_cdd_strdup(json_str);
    json_free_serialized_string(json_str);
    if (!out->json)
      return ENOMEM;
    break;
  default:
    break;
  }

  return 0;
}

static int parse_any_field(const JSON_Object *const obj, const char *const key,
                           struct OpenAPI_Any *const out, int *const out_set) {
  const JSON_Value *val;
  if (!obj || !key || !out || !out_set)
    return 0;
  if (!json_object_has_value(obj, key))
    return 0;
  val = json_object_get_value(obj, key);
  if (!val)
    return 0;
  if (parse_any_value(val, out) != 0)
    return ENOMEM;
  *out_set = 1;
  return 0;
}

static int parse_any_array(const JSON_Array *const arr,
                           struct OpenAPI_Any **const out,
                           size_t *const out_count) {
  size_t i, count;

  if (!out || !out_count)
    return 0;
  *out = NULL;
  *out_count = 0;
  if (!arr)
    return 0;

  count = json_array_get_count(arr);
  if (count == 0)
    return 0;

  *out = (struct OpenAPI_Any *)calloc(count, sizeof(struct OpenAPI_Any));
  if (!*out)
    return ENOMEM;
  *out_count = count;

  for (i = 0; i < count; ++i) {
    const JSON_Value *val = json_array_get_value(arr, i);
    if (parse_any_value(val, &(*out)[i]) != 0) {
      size_t j;
      for (j = 0; j < i; ++j)
        free_any_value(&(*out)[j]);
      free(*out);
      *out = NULL;
      *out_count = 0;
      return ENOMEM;
    }
  }

  return 0;
}

static int key_in_list(const char *key, const char *const *list, size_t count) {
  size_t i;
  if (!key || !list)
    return 0;
  for (i = 0; i < count; ++i) {
    if (list[i] && strcmp(list[i], key) == 0)
      return 1;
  }
  return 0;
}

static int is_extension_key(const char *key) {
  return key && key[0] == 'x' && key[1] == '-';
}

static JSON_Value *clone_json_value(const JSON_Value *val) {
  char *serialized;
  JSON_Value *copy;

  if (!val)
    return NULL;
  serialized = json_serialize_to_string((JSON_Value *)val);
  if (!serialized)
    return NULL;
  copy = json_parse_string(serialized);
  json_free_serialized_string(serialized);
  return copy;
}

static int collect_schema_extras(const JSON_Object *obj,
                                 const char *const *skip_keys,
                                 size_t skip_count, char **out_json) {
  JSON_Value *extras_val;
  JSON_Object *extras_obj;
  size_t i, count;
  char *serialized;

  if (out_json)
    *out_json = NULL;
  if (!obj || !out_json)
    return EINVAL;

  extras_val = json_value_init_object();
  if (!extras_val)
    return ENOMEM;
  extras_obj = json_value_get_object(extras_val);

  count = json_object_get_count(obj);
  for (i = 0; i < count; ++i) {
    const char *key = json_object_get_name(obj, i);
    const JSON_Value *val;
    JSON_Value *copy;

    if (!key || key_in_list(key, skip_keys, skip_count))
      continue;
    val = json_object_get_value(obj, key);
    copy = clone_json_value(val);
    if (!copy) {
      json_value_free(extras_val);
      return ENOMEM;
    }
    if (json_object_set_value(extras_obj, key, copy) != JSONSuccess) {
      json_value_free(copy);
      json_value_free(extras_val);
      return ENOMEM;
    }
  }

  if (json_object_get_count(extras_obj) == 0) {
    json_value_free(extras_val);
    return 0;
  }

  serialized = json_serialize_to_string(extras_val);
  if (!serialized) {
    json_value_free(extras_val);
    return ENOMEM;
  }
  *out_json = c_cdd_strdup(serialized);
  json_free_serialized_string(serialized);
  json_value_free(extras_val);
  return *out_json ? 0 : ENOMEM;
}

static int collect_extensions(const JSON_Object *obj, char **out_json) {
  JSON_Value *extras_val;
  JSON_Object *extras_obj;
  size_t i, count;
  char *serialized;

  if (out_json)
    *out_json = NULL;
  if (!obj || !out_json)
    return EINVAL;

  extras_val = json_value_init_object();
  if (!extras_val)
    return ENOMEM;
  extras_obj = json_value_get_object(extras_val);

  count = json_object_get_count(obj);
  for (i = 0; i < count; ++i) {
    const char *key = json_object_get_name(obj, i);
    const JSON_Value *val;
    JSON_Value *copy;
    if (!is_extension_key(key))
      continue;
    val = json_object_get_value(obj, key);
    copy = clone_json_value(val);
    if (!copy) {
      json_value_free(extras_val);
      return ENOMEM;
    }
    if (json_object_set_value(extras_obj, key, copy) != JSONSuccess) {
      json_value_free(copy);
      json_value_free(extras_val);
      return ENOMEM;
    }
  }

  if (json_object_get_count(extras_obj) == 0) {
    json_value_free(extras_val);
    return 0;
  }

  serialized = json_serialize_to_string(extras_val);
  if (!serialized) {
    json_value_free(extras_val);
    return ENOMEM;
  }
  *out_json = c_cdd_strdup(serialized);
  json_free_serialized_string(serialized);
  json_value_free(extras_val);
  return *out_json ? 0 : ENOMEM;
}

static const char *parse_schema_type(const JSON_Object *const schema,
                                     int *const out_nullable) {
  const char *type;
  const JSON_Array *types;
  size_t i, count;
  const char *chosen = NULL;

  if (out_nullable)
    *out_nullable = 0;
  if (!schema)
    return NULL;

  type = json_object_get_string(schema, "type");
  if (type)
    return type;

  types = json_object_get_array(schema, "type");
  if (!types)
    return NULL;

  count = json_array_get_count(types);
  for (i = 0; i < count; ++i) {
    const char *t = json_array_get_string(types, i);
    if (!t)
      continue;
    if (strcmp(t, "null") == 0) {
      if (out_nullable)
        *out_nullable = 1;
      continue;
    }
    if (!chosen)
      chosen = t;
  }

  if (!chosen && out_nullable && *out_nullable)
    return "null";

  return chosen;
}

static int
parse_schema_constraints(const JSON_Object *const schema,
                         struct SchemaConstraintTarget *const target) {
  if (!schema || !target)
    return 0;

  if (target->example && target->example_set) {
    if (parse_any_field(schema, "example", target->example,
                        target->example_set) != 0)
      return ENOMEM;
  }

  if (target->has_min && target->min_val) {
    if (json_object_has_value_of_type(schema, "minimum", JSONNumber)) {
      *target->has_min = 1;
      *target->min_val = json_object_get_number(schema, "minimum");
    }
    if (json_object_has_value_of_type(schema, "exclusiveMinimum", JSONNumber)) {
      *target->has_min = 1;
      *target->min_val = json_object_get_number(schema, "exclusiveMinimum");
      if (target->exclusive_min)
        *target->exclusive_min = 1;
    } else if (json_object_has_value_of_type(schema, "exclusiveMinimum",
                                             JSONBoolean) &&
               json_object_get_boolean(schema, "exclusiveMinimum")) {
      if (target->exclusive_min)
        *target->exclusive_min = 1;
    }
  }

  if (target->has_max && target->max_val) {
    if (json_object_has_value_of_type(schema, "maximum", JSONNumber)) {
      *target->has_max = 1;
      *target->max_val = json_object_get_number(schema, "maximum");
    }
    if (json_object_has_value_of_type(schema, "exclusiveMaximum", JSONNumber)) {
      *target->has_max = 1;
      *target->max_val = json_object_get_number(schema, "exclusiveMaximum");
      if (target->exclusive_max)
        *target->exclusive_max = 1;
    } else if (json_object_has_value_of_type(schema, "exclusiveMaximum",
                                             JSONBoolean) &&
               json_object_get_boolean(schema, "exclusiveMaximum")) {
      if (target->exclusive_max)
        *target->exclusive_max = 1;
    }
  }

  if (target->has_min_len && target->min_len) {
    if (json_object_has_value_of_type(schema, "minLength", JSONNumber)) {
      *target->has_min_len = 1;
      *target->min_len = (size_t)json_object_get_number(schema, "minLength");
    }
  }

  if (target->has_max_len && target->max_len) {
    if (json_object_has_value_of_type(schema, "maxLength", JSONNumber)) {
      *target->has_max_len = 1;
      *target->max_len = (size_t)json_object_get_number(schema, "maxLength");
    }
  }

  if (target->pattern) {
    if (json_object_has_value_of_type(schema, "pattern", JSONString)) {
      const char *pattern = json_object_get_string(schema, "pattern");
      if (pattern) {
        *target->pattern = c_cdd_strdup(pattern);
        if (!*target->pattern)
          return ENOMEM;
      }
    }
  }

  if (target->has_min_items && target->min_items) {
    if (json_object_has_value_of_type(schema, "minItems", JSONNumber)) {
      *target->has_min_items = 1;
      *target->min_items = (size_t)json_object_get_number(schema, "minItems");
    }
  }

  if (target->has_max_items && target->max_items) {
    if (json_object_has_value_of_type(schema, "maxItems", JSONNumber)) {
      *target->has_max_items = 1;
      *target->max_items = (size_t)json_object_get_number(schema, "maxItems");
    }
  }

  if (target->unique_items) {
    if (json_object_has_value_of_type(schema, "uniqueItems", JSONBoolean)) {
      *target->unique_items = json_object_get_boolean(schema, "uniqueItems");
    }
  }

  return 0;
}

static int parse_string_enum_array(const JSON_Array *const arr, char ***out,
                                   size_t *out_count) {
  size_t i, count;

  if (!out || !out_count)
    return 0;
  *out = NULL;
  *out_count = 0;
  if (!arr)
    return 0;

  count = json_array_get_count(arr);
  if (count == 0)
    return 0;

  for (i = 0; i < count; ++i) {
    if (!json_array_get_string(arr, i))
      return 0;
  }

  *out = (char **)calloc(count, sizeof(char *));
  if (!*out)
    return ENOMEM;
  *out_count = count;

  for (i = 0; i < count; ++i) {
    const char *val = json_array_get_string(arr, i);
    if (!val)
      continue;
    (*out)[i] = c_cdd_strdup(val);
    if (!(*out)[i]) {
      size_t j;
      for (j = 0; j < i; ++j)
        free((*out)[j]);
      free(*out);
      *out = NULL;
      *out_count = 0;
      return ENOMEM;
    }
  }

  return 0;
}

static void free_string_array(char **arr, size_t n) {
  size_t i;
  if (!arr)
    return;
  for (i = 0; i < n; ++i) {
    if (arr[i])
      free(arr[i]);
  }
  free(arr);
}

static int copy_string_array(char ***dst, size_t *dst_count, char *const *src,
                             size_t src_count) {
  size_t i;
  if (!dst || !dst_count)
    return 0;
  *dst = NULL;
  *dst_count = 0;
  if (!src || src_count == 0)
    return 0;
  *dst = (char **)calloc(src_count, sizeof(char *));
  if (!*dst)
    return ENOMEM;
  *dst_count = src_count;
  for (i = 0; i < src_count; ++i) {
    if (!src[i])
      continue;
    (*dst)[i] = c_cdd_strdup(src[i]);
    if (!(*dst)[i]) {
      size_t j;
      for (j = 0; j < i; ++j)
        free((*dst)[j]);
      free(*dst);
      *dst = NULL;
      *dst_count = 0;
      return ENOMEM;
    }
  }
  return 0;
}

static int copy_example_fields(struct OpenAPI_Example *dst,
                               const struct OpenAPI_Example *src) {
  if (!dst || !src)
    return 0;
  if (src->name && !dst->name) {
    dst->name = c_cdd_strdup(src->name);
    if (!dst->name)
      return ENOMEM;
  }
  if (src->ref && !dst->ref) {
    dst->ref = c_cdd_strdup(src->ref);
    if (!dst->ref)
      return ENOMEM;
  }
  if (src->extensions_json && !dst->extensions_json) {
    dst->extensions_json = c_cdd_strdup(src->extensions_json);
    if (!dst->extensions_json)
      return ENOMEM;
  }
  if (src->summary && !dst->summary) {
    dst->summary = c_cdd_strdup(src->summary);
    if (!dst->summary)
      return ENOMEM;
  }
  if (src->description && !dst->description) {
    dst->description = c_cdd_strdup(src->description);
    if (!dst->description)
      return ENOMEM;
  }
  if (src->extensions_json && !dst->extensions_json) {
    dst->extensions_json = c_cdd_strdup(src->extensions_json);
    if (!dst->extensions_json)
      return ENOMEM;
  }
  if (src->data_value_set && !dst->data_value_set) {
    if (copy_any_value(&dst->data_value, &src->data_value) != 0)
      return ENOMEM;
    dst->data_value_set = 1;
  }
  if (src->value_set && !dst->value_set) {
    if (copy_any_value(&dst->value, &src->value) != 0)
      return ENOMEM;
    dst->value_set = 1;
  }
  if (src->serialized_value && !dst->serialized_value) {
    dst->serialized_value = c_cdd_strdup(src->serialized_value);
    if (!dst->serialized_value)
      return ENOMEM;
  }
  if (src->external_value && !dst->external_value) {
    dst->external_value = c_cdd_strdup(src->external_value);
    if (!dst->external_value)
      return ENOMEM;
  }
  return 0;
}

static void free_example(struct OpenAPI_Example *ex) {
  if (!ex)
    return;
  if (ex->name)
    free(ex->name);
  if (ex->ref)
    free(ex->ref);
  if (ex->summary)
    free(ex->summary);
  if (ex->description)
    free(ex->description);
  if (ex->extensions_json)
    free(ex->extensions_json);
  if (ex->serialized_value)
    free(ex->serialized_value);
  if (ex->external_value)
    free(ex->external_value);
  if (ex->data_value_set)
    free_any_value(&ex->data_value);
  if (ex->value_set)
    free_any_value(&ex->value);
}

static const struct OpenAPI_Example *
find_component_example(const struct OpenAPI_Spec *spec, const char *ref) {
  size_t i;
  char *name;
  if (!spec || !ref)
    return NULL;
  name = clean_ref(ref);
  if (!name)
    return NULL;
  for (i = 0; i < spec->n_component_examples; ++i) {
    if (spec->component_example_names && spec->component_example_names[i] &&
        strcmp(spec->component_example_names[i], name) == 0) {
      free(name);
      return &spec->component_examples[i];
    }
  }
  free(name);
  return NULL;
}

static int parse_example_object(const JSON_Object *const ex_obj,
                                const char *const name,
                                struct OpenAPI_Example *const out,
                                const struct OpenAPI_Spec *const spec,
                                const int resolve_refs) {
  const char *ref;
  const char *summary;
  const char *desc;
  const char *serialized;
  const char *external;

  if (!ex_obj || !out)
    return 0;

  if (name) {
    out->name = c_cdd_strdup(name);
    if (!out->name)
      return ENOMEM;
  }

  ref = json_object_get_string(ex_obj, "$ref");
  if (ref) {
    out->ref = c_cdd_strdup(ref);
    if (!out->ref)
      return ENOMEM;
    if (resolve_refs && spec) {
      const struct OpenAPI_Example *comp = find_component_example(spec, ref);
      if (comp) {
        if (copy_example_fields(out, comp) != 0)
          return ENOMEM;
      }
    }
  }

  summary = json_object_get_string(ex_obj, "summary");
  if (summary) {
    out->summary = c_cdd_strdup(summary);
    if (!out->summary)
      return ENOMEM;
  }
  desc = json_object_get_string(ex_obj, "description");
  if (desc) {
    out->description = c_cdd_strdup(desc);
    if (!out->description)
      return ENOMEM;
  }
  if (!ref) {
    if (collect_extensions(ex_obj, &out->extensions_json) != 0)
      return ENOMEM;
  }

  if (parse_any_field(ex_obj, "dataValue", &out->data_value,
                      &out->data_value_set) != 0)
    return ENOMEM;
  if (parse_any_field(ex_obj, "value", &out->value, &out->value_set) != 0)
    return ENOMEM;

  serialized = json_object_get_string(ex_obj, "serializedValue");
  if (serialized) {
    out->serialized_value = c_cdd_strdup(serialized);
    if (!out->serialized_value)
      return ENOMEM;
  }
  external = json_object_get_string(ex_obj, "externalValue");
  if (external) {
    out->external_value = c_cdd_strdup(external);
    if (!out->external_value)
      return ENOMEM;
  }

  return 0;
}

static int parse_examples_object(const JSON_Object *const examples,
                                 struct OpenAPI_Example **out,
                                 size_t *out_count,
                                 const struct OpenAPI_Spec *const spec,
                                 const int resolve_refs) {
  size_t count, i;

  if (!out || !out_count) {
    return 0;
  }
  *out = NULL;
  *out_count = 0;

  if (!examples)
    return 0;

  count = json_object_get_count(examples);
  if (count == 0)
    return 0;

  *out =
      (struct OpenAPI_Example *)calloc(count, sizeof(struct OpenAPI_Example));
  if (!*out)
    return ENOMEM;
  *out_count = count;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(examples, i);
    const JSON_Object *ex_obj =
        json_value_get_object(json_object_get_value_at(examples, i));
    if (ex_obj) {
      if (parse_example_object(ex_obj, name, &(*out)[i], spec, resolve_refs) !=
          0)
        return ENOMEM;
    }
  }
  return 0;
}

static int parse_media_examples(const JSON_Object *const media_obj,
                                struct OpenAPI_Any *const example,
                                int *const example_set,
                                struct OpenAPI_Example **examples,
                                size_t *const n_examples,
                                const struct OpenAPI_Spec *const spec,
                                const int resolve_refs) {
  const JSON_Object *examples_obj;

  if (!media_obj)
    return 0;

  examples_obj = json_object_get_object(media_obj, "examples");
  if (examples_obj) {
    return parse_examples_object(examples_obj, examples, n_examples, spec,
                                 resolve_refs);
  }
  return parse_any_field(media_obj, "example", example, example_set);
}

static int parse_oauth_scopes(const JSON_Object *const scopes_obj,
                              struct OpenAPI_OAuthScope **out,
                              size_t *out_count) {
  size_t count, i;
  if (!out || !out_count)
    return 0;
  *out = NULL;
  *out_count = 0;
  if (!scopes_obj)
    return 0;
  count = json_object_get_count(scopes_obj);
  if (count == 0)
    return 0;
  *out = (struct OpenAPI_OAuthScope *)calloc(count,
                                             sizeof(struct OpenAPI_OAuthScope));
  if (!*out)
    return ENOMEM;
  *out_count = count;
  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(scopes_obj, i);
    const char *desc = json_object_get_string(scopes_obj, name);
    if (name) {
      (*out)[i].name = c_cdd_strdup(name);
      if (!(*out)[i].name)
        return ENOMEM;
    }
    if (desc) {
      (*out)[i].description = c_cdd_strdup(desc);
      if (!(*out)[i].description)
        return ENOMEM;
    }
  }
  return 0;
}

static int parse_oauth_flows(const JSON_Object *const flows_obj,
                             struct OpenAPI_SecurityScheme *const out) {
  size_t count, i;
  if (!flows_obj || !out)
    return 0;
  count = json_object_get_count(flows_obj);
  if (count == 0)
    return 0;
  out->flows = (struct OpenAPI_OAuthFlow *)calloc(
      count, sizeof(struct OpenAPI_OAuthFlow));
  if (!out->flows)
    return ENOMEM;
  out->n_flows = count;
  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(flows_obj, i);
    const JSON_Object *flow_obj =
        json_value_get_object(json_object_get_value_at(flows_obj, i));
    struct OpenAPI_OAuthFlow *flow = &out->flows[i];
    if (name)
      flow->type = parse_oauth_flow_type(name);
    if (flow_obj) {
      const char *authorization_url =
          json_object_get_string(flow_obj, "authorizationUrl");
      const char *token_url = json_object_get_string(flow_obj, "tokenUrl");
      const char *refresh_url = json_object_get_string(flow_obj, "refreshUrl");
      const char *device_authorization_url =
          json_object_get_string(flow_obj, "deviceAuthorizationUrl");
      const JSON_Object *scopes_obj =
          json_object_get_object(flow_obj, "scopes");

      if (authorization_url) {
        flow->authorization_url = c_cdd_strdup(authorization_url);
        if (!flow->authorization_url)
          return ENOMEM;
      }
      if (token_url) {
        flow->token_url = c_cdd_strdup(token_url);
        if (!flow->token_url)
          return ENOMEM;
      }
      if (refresh_url) {
        flow->refresh_url = c_cdd_strdup(refresh_url);
        if (!flow->refresh_url)
          return ENOMEM;
      }
      if (device_authorization_url) {
        flow->device_authorization_url = c_cdd_strdup(device_authorization_url);
        if (!flow->device_authorization_url)
          return ENOMEM;
      }
      if (parse_oauth_scopes(scopes_obj, &flow->scopes, &flow->n_scopes) != 0)
        return ENOMEM;
      if (collect_extensions(flow_obj, &flow->extensions_json) != 0)
        return ENOMEM;
    }
  }
  return 0;
}

static char *json_pointer_unescape(const char *in) {
  size_t len, i, j;
  char *out;
  if (!in)
    return NULL;
  len = strlen(in);
  out = (char *)malloc(len + 1);
  if (!out)
    return NULL;
  for (i = 0, j = 0; i < len; ++i) {
    if (in[i] == '~' && i + 1 < len) {
      if (in[i + 1] == '0') {
        out[j++] = '~';
        i++;
        continue;
      }
      if (in[i + 1] == '1') {
        out[j++] = '/';
        i++;
        continue;
      }
    }
    out[j++] = in[i];
  }
  out[j] = '\0';
  return out;
}

static const char *ref_name_from_prefix(const char *ref, const char *prefix) {
  size_t prefix_len;
  if (!ref || !prefix)
    return NULL;
  prefix_len = strlen(prefix);
  if (strncmp(ref, prefix, prefix_len) != 0)
    return NULL;
  return ref + prefix_len;
}

static const struct OpenAPI_Parameter *
find_component_parameter(const struct OpenAPI_Spec *spec, const char *ref) {
  const char *name = ref_name_from_prefix(ref, "#/components/parameters/");
  size_t i;
  if (!spec || !name)
    return NULL;
  for (i = 0; i < spec->n_component_parameters; ++i) {
    if (spec->component_parameter_names[i] &&
        strcmp(spec->component_parameter_names[i], name) == 0) {
      return &spec->component_parameters[i];
    }
  }
  return NULL;
}

static const struct OpenAPI_Response *
find_component_response(const struct OpenAPI_Spec *spec, const char *ref) {
  const char *name = ref_name_from_prefix(ref, "#/components/responses/");
  size_t i;
  if (!spec || !name)
    return NULL;
  for (i = 0; i < spec->n_component_responses; ++i) {
    if (spec->component_response_names[i] &&
        strcmp(spec->component_response_names[i], name) == 0) {
      return &spec->component_responses[i];
    }
  }
  return NULL;
}

static const struct OpenAPI_Header *
find_component_header(const struct OpenAPI_Spec *spec, const char *ref) {
  const char *name = ref_name_from_prefix(ref, "#/components/headers/");
  size_t i;
  if (!spec || !name)
    return NULL;
  for (i = 0; i < spec->n_component_headers; ++i) {
    if (spec->component_header_names[i] &&
        strcmp(spec->component_header_names[i], name) == 0) {
      return &spec->component_headers[i];
    }
  }
  return NULL;
}

static const struct OpenAPI_RequestBody *
find_component_request_body(const struct OpenAPI_Spec *spec, const char *ref) {
  const char *name = ref_name_from_prefix(ref, "#/components/requestBodies/");
  size_t i;
  if (!spec || !name)
    return NULL;
  for (i = 0; i < spec->n_component_request_bodies; ++i) {
    if (spec->component_request_body_names[i] &&
        strcmp(spec->component_request_body_names[i], name) == 0) {
      return &spec->component_request_bodies[i];
    }
  }
  return NULL;
}

static const struct OpenAPI_MediaType *
find_component_media_type(const struct OpenAPI_Spec *spec, const char *ref) {
  const char *name_enc = ref_name_from_prefix(ref, "#/components/mediaTypes/");
  char *name_dec;
  size_t i;
  if (!spec || !name_enc)
    return NULL;
  name_dec = json_pointer_unescape(name_enc);
  if (!name_dec)
    return NULL;
  for (i = 0; i < spec->n_component_media_types; ++i) {
    if (spec->component_media_type_names[i] &&
        strcmp(spec->component_media_type_names[i], name_dec) == 0) {
      free(name_dec);
      return &spec->component_media_types[i];
    }
  }
  free(name_dec);
  return NULL;
}

static const struct OpenAPI_Link *
find_component_link(const struct OpenAPI_Spec *spec, const char *ref) {
  const char *name = ref_name_from_prefix(ref, "#/components/links/");
  size_t i;
  if (!spec || !name)
    return NULL;
  for (i = 0; i < spec->n_component_links; ++i) {
    if (spec->component_links[i].name &&
        strcmp(spec->component_links[i].name, name) == 0) {
      return &spec->component_links[i];
    }
  }
  return NULL;
}

static const struct OpenAPI_Callback *
find_component_callback(const struct OpenAPI_Spec *spec, const char *ref) {
  const char *name = ref_name_from_prefix(ref, "#/components/callbacks/");
  size_t i;
  if (!spec || !name)
    return NULL;
  for (i = 0; i < spec->n_component_callbacks; ++i) {
    if (spec->component_callbacks[i].name &&
        strcmp(spec->component_callbacks[i].name, name) == 0) {
      return &spec->component_callbacks[i];
    }
  }
  return NULL;
}

static int copy_schema_ref(struct OpenAPI_SchemaRef *dst,
                           const struct OpenAPI_SchemaRef *src) {
  size_t i;
  if (!dst || !src)
    return 0;
  dst->schema_is_boolean = src->schema_is_boolean;
  dst->schema_boolean_value = src->schema_boolean_value;
  dst->is_array = src->is_array;
  if (src->ref_name) {
    dst->ref_name = c_cdd_strdup(src->ref_name);
    if (!dst->ref_name)
      return ENOMEM;
  }
  if (src->ref) {
    dst->ref = c_cdd_strdup(src->ref);
    if (!dst->ref)
      return ENOMEM;
  }
  if (src->inline_type) {
    dst->inline_type = c_cdd_strdup(src->inline_type);
    if (!dst->inline_type)
      return ENOMEM;
  }
  if (src->type_union && src->n_type_union > 0) {
    if (copy_string_array(&dst->type_union, &dst->n_type_union, src->type_union,
                          src->n_type_union) != 0)
      return ENOMEM;
  }
  if (src->format) {
    dst->format = c_cdd_strdup(src->format);
    if (!dst->format)
      return ENOMEM;
  }
  if (src->content_type) {
    dst->content_type = c_cdd_strdup(src->content_type);
    if (!dst->content_type)
      return ENOMEM;
  }
  if (src->extensions_json) {
    dst->extensions_json = c_cdd_strdup(src->extensions_json);
    if (!dst->extensions_json)
      return ENOMEM;
  }
  if (src->content_media_type) {
    dst->content_media_type = c_cdd_strdup(src->content_media_type);
    if (!dst->content_media_type)
      return ENOMEM;
  }
  if (src->content_encoding) {
    dst->content_encoding = c_cdd_strdup(src->content_encoding);
    if (!dst->content_encoding)
      return ENOMEM;
  }
  if (src->items_format) {
    dst->items_format = c_cdd_strdup(src->items_format);
    if (!dst->items_format)
      return ENOMEM;
  }
  if (src->items_type_union && src->n_items_type_union > 0) {
    if (copy_string_array(&dst->items_type_union, &dst->n_items_type_union,
                          src->items_type_union, src->n_items_type_union) != 0)
      return ENOMEM;
  }
  if (src->items_ref) {
    dst->items_ref = c_cdd_strdup(src->items_ref);
    if (!dst->items_ref)
      return ENOMEM;
  }
  if (src->items_content_media_type) {
    dst->items_content_media_type = c_cdd_strdup(src->items_content_media_type);
    if (!dst->items_content_media_type)
      return ENOMEM;
  }
  if (src->items_content_encoding) {
    dst->items_content_encoding = c_cdd_strdup(src->items_content_encoding);
    if (!dst->items_content_encoding)
      return ENOMEM;
  }
  dst->nullable = src->nullable;
  dst->items_nullable = src->items_nullable;
  if (src->summary) {
    dst->summary = c_cdd_strdup(src->summary);
    if (!dst->summary)
      return ENOMEM;
  }
  if (src->description) {
    dst->description = c_cdd_strdup(src->description);
    if (!dst->description)
      return ENOMEM;
  }
  if (src->extensions_json) {
    dst->extensions_json = c_cdd_strdup(src->extensions_json);
    if (!dst->extensions_json)
      return ENOMEM;
  }
  if (src->extensions_json) {
    dst->extensions_json = c_cdd_strdup(src->extensions_json);
    if (!dst->extensions_json)
      return ENOMEM;
  }
  if (src->extensions_json) {
    dst->extensions_json = c_cdd_strdup(src->extensions_json);
    if (!dst->extensions_json)
      return ENOMEM;
  }
  if (src->extensions_json) {
    dst->extensions_json = c_cdd_strdup(src->extensions_json);
    if (!dst->extensions_json)
      return ENOMEM;
  }
  if (src->deprecated_set) {
    dst->deprecated_set = 1;
    dst->deprecated = src->deprecated;
  }
  if (src->read_only_set) {
    dst->read_only_set = 1;
    dst->read_only = src->read_only;
  }
  if (src->write_only_set) {
    dst->write_only_set = 1;
    dst->write_only = src->write_only;
  }
  if (src->const_value_set) {
    if (copy_any_value(&dst->const_value, &src->const_value) != 0)
      return ENOMEM;
    dst->const_value_set = 1;
  }
  if (src->examples && src->n_examples > 0) {
    dst->examples = (struct OpenAPI_Any *)calloc(src->n_examples,
                                                 sizeof(struct OpenAPI_Any));
    if (!dst->examples)
      return ENOMEM;
    dst->n_examples = src->n_examples;
    for (i = 0; i < src->n_examples; ++i) {
      if (copy_any_value(&dst->examples[i], &src->examples[i]) != 0)
        return ENOMEM;
    }
  }
  if (src->example_set) {
    if (copy_any_value(&dst->example, &src->example) != 0)
      return ENOMEM;
    dst->example_set = 1;
  }
  if (src->default_value_set) {
    if (copy_any_value(&dst->default_value, &src->default_value) != 0)
      return ENOMEM;
    dst->default_value_set = 1;
  }
  if (src->enum_values && src->n_enum_values > 0) {
    dst->enum_values = (struct OpenAPI_Any *)calloc(src->n_enum_values,
                                                    sizeof(struct OpenAPI_Any));
    if (!dst->enum_values)
      return ENOMEM;
    dst->n_enum_values = src->n_enum_values;
    for (i = 0; i < src->n_enum_values; ++i) {
      if (copy_any_value(&dst->enum_values[i], &src->enum_values[i]) != 0)
        return ENOMEM;
    }
  }
  if (src->schema_extra_json) {
    dst->schema_extra_json = c_cdd_strdup(src->schema_extra_json);
    if (!dst->schema_extra_json)
      return ENOMEM;
  }
  if (src->items_enum_values && src->n_items_enum_values > 0) {
    dst->items_enum_values = (struct OpenAPI_Any *)calloc(
        src->n_items_enum_values, sizeof(struct OpenAPI_Any));
    if (!dst->items_enum_values)
      return ENOMEM;
    dst->n_items_enum_values = src->n_items_enum_values;
    for (i = 0; i < src->n_items_enum_values; ++i) {
      if (copy_any_value(&dst->items_enum_values[i],
                         &src->items_enum_values[i]) != 0)
        return ENOMEM;
    }
  }
  dst->has_min = src->has_min;
  dst->min_val = src->min_val;
  dst->exclusive_min = src->exclusive_min;
  dst->has_max = src->has_max;
  dst->max_val = src->max_val;
  dst->exclusive_max = src->exclusive_max;
  dst->has_min_len = src->has_min_len;
  dst->min_len = src->min_len;
  dst->has_max_len = src->has_max_len;
  dst->max_len = src->max_len;
  if (src->pattern) {
    dst->pattern = c_cdd_strdup(src->pattern);
    if (!dst->pattern)
      return ENOMEM;
  }
  dst->has_min_items = src->has_min_items;
  dst->min_items = src->min_items;
  dst->has_max_items = src->has_max_items;
  dst->max_items = src->max_items;
  dst->unique_items = src->unique_items;
  dst->items_has_min = src->items_has_min;
  dst->items_min_val = src->items_min_val;
  dst->items_exclusive_min = src->items_exclusive_min;
  dst->items_has_max = src->items_has_max;
  dst->items_max_val = src->items_max_val;
  dst->items_exclusive_max = src->items_exclusive_max;
  dst->items_has_min_len = src->items_has_min_len;
  dst->items_min_len = src->items_min_len;
  dst->items_has_max_len = src->items_has_max_len;
  dst->items_max_len = src->items_max_len;
  if (src->items_pattern) {
    dst->items_pattern = c_cdd_strdup(src->items_pattern);
    if (!dst->items_pattern)
      return ENOMEM;
  }
  dst->items_has_min_items = src->items_has_min_items;
  dst->items_min_items = src->items_min_items;
  dst->items_has_max_items = src->items_has_max_items;
  dst->items_max_items = src->items_max_items;
  dst->items_unique_items = src->items_unique_items;
  if (src->items_example_set) {
    if (copy_any_value(&dst->items_example, &src->items_example) != 0)
      return ENOMEM;
    dst->items_example_set = 1;
  }
  if (src->items_examples && src->n_items_examples > 0) {
    dst->items_examples = (struct OpenAPI_Any *)calloc(
        src->n_items_examples, sizeof(struct OpenAPI_Any));
    if (!dst->items_examples)
      return ENOMEM;
    dst->n_items_examples = src->n_items_examples;
    for (i = 0; i < src->n_items_examples; ++i) {
      if (copy_any_value(&dst->items_examples[i], &src->items_examples[i]) != 0)
        return ENOMEM;
    }
  }
  if (src->items_const_value_set) {
    if (copy_any_value(&dst->items_const_value, &src->items_const_value) != 0)
      return ENOMEM;
    dst->items_const_value_set = 1;
  }
  if (src->items_default_value_set) {
    if (copy_any_value(&dst->items_default_value, &src->items_default_value) !=
        0)
      return ENOMEM;
    dst->items_default_value_set = 1;
  }
  if (src->items_extra_json) {
    dst->items_extra_json = c_cdd_strdup(src->items_extra_json);
    if (!dst->items_extra_json)
      return ENOMEM;
  }
  dst->items_schema_is_boolean = src->items_schema_is_boolean;
  dst->items_schema_boolean_value = src->items_schema_boolean_value;
  if (src->n_multipart_fields > 0 && src->multipart_fields) {
    dst->multipart_fields = (struct OpenAPI_MultipartField *)calloc(
        src->n_multipart_fields, sizeof(struct OpenAPI_MultipartField));
    if (!dst->multipart_fields)
      return ENOMEM;
    dst->n_multipart_fields = src->n_multipart_fields;
    for (i = 0; i < src->n_multipart_fields; ++i) {
      const struct OpenAPI_MultipartField *src_field =
          &src->multipart_fields[i];
      struct OpenAPI_MultipartField *dst_field = &dst->multipart_fields[i];
      dst_field->is_binary = src_field->is_binary;
      if (src_field->name) {
        dst_field->name = c_cdd_strdup(src_field->name);
        if (!dst_field->name)
          return ENOMEM;
      }
      if (src_field->type) {
        dst_field->type = c_cdd_strdup(src_field->type);
        if (!dst_field->type)
          return ENOMEM;
      }
    }
  }
  return 0;
}

static int copy_any_value(struct OpenAPI_Any *dst,
                          const struct OpenAPI_Any *src) {
  if (!dst || !src)
    return 0;
  dst->type = src->type;
  dst->number = src->number;
  dst->boolean = src->boolean;
  if (src->type == OA_ANY_STRING && src->string) {
    dst->string = c_cdd_strdup(src->string);
    if (!dst->string)
      return ENOMEM;
  } else if (src->type == OA_ANY_JSON && src->json) {
    dst->json = c_cdd_strdup(src->json);
    if (!dst->json)
      return ENOMEM;
  }
  return 0;
}

static int copy_server_object(struct OpenAPI_Server *dst,
                              const struct OpenAPI_Server *src) {
  size_t v, e;
  if (!dst || !src)
    return 0;
  if (src->url) {
    dst->url = c_cdd_strdup(src->url);
    if (!dst->url)
      return ENOMEM;
  }
  if (src->description) {
    dst->description = c_cdd_strdup(src->description);
    if (!dst->description)
      return ENOMEM;
  }
  if (src->name) {
    dst->name = c_cdd_strdup(src->name);
    if (!dst->name)
      return ENOMEM;
  }
  if (src->extensions_json) {
    dst->extensions_json = c_cdd_strdup(src->extensions_json);
    if (!dst->extensions_json)
      return ENOMEM;
  }
  if (src->n_variables > 0 && src->variables) {
    dst->variables = (struct OpenAPI_ServerVariable *)calloc(
        src->n_variables, sizeof(struct OpenAPI_ServerVariable));
    if (!dst->variables)
      return ENOMEM;
    dst->n_variables = src->n_variables;
    for (v = 0; v < src->n_variables; ++v) {
      const struct OpenAPI_ServerVariable *src_var = &src->variables[v];
      struct OpenAPI_ServerVariable *dst_var = &dst->variables[v];
      if (src_var->name) {
        dst_var->name = c_cdd_strdup(src_var->name);
        if (!dst_var->name)
          return ENOMEM;
      }
      if (src_var->default_value) {
        dst_var->default_value = c_cdd_strdup(src_var->default_value);
        if (!dst_var->default_value)
          return ENOMEM;
      }
      if (src_var->description) {
        dst_var->description = c_cdd_strdup(src_var->description);
        if (!dst_var->description)
          return ENOMEM;
      }
      if (src_var->extensions_json) {
        dst_var->extensions_json = c_cdd_strdup(src_var->extensions_json);
        if (!dst_var->extensions_json)
          return ENOMEM;
      }
      if (src_var->n_enum_values > 0 && src_var->enum_values) {
        dst_var->enum_values =
            (char **)calloc(src_var->n_enum_values, sizeof(char *));
        if (!dst_var->enum_values)
          return ENOMEM;
        dst_var->n_enum_values = src_var->n_enum_values;
        for (e = 0; e < src_var->n_enum_values; ++e) {
          if (src_var->enum_values[e]) {
            dst_var->enum_values[e] = c_cdd_strdup(src_var->enum_values[e]);
            if (!dst_var->enum_values[e])
              return ENOMEM;
          }
        }
      }
    }
  }
  return 0;
}

static int copy_link_fields(struct OpenAPI_Link *dst,
                            const struct OpenAPI_Link *src) {
  size_t i;
  if (!dst || !src)
    return 0;
  if (src->summary) {
    dst->summary = c_cdd_strdup(src->summary);
    if (!dst->summary)
      return ENOMEM;
  }
  if (src->description) {
    dst->description = c_cdd_strdup(src->description);
    if (!dst->description)
      return ENOMEM;
  }
  if (src->extensions_json) {
    dst->extensions_json = c_cdd_strdup(src->extensions_json);
    if (!dst->extensions_json)
      return ENOMEM;
  }
  if (src->operation_ref) {
    dst->operation_ref = c_cdd_strdup(src->operation_ref);
    if (!dst->operation_ref)
      return ENOMEM;
  }
  if (src->operation_id) {
    dst->operation_id = c_cdd_strdup(src->operation_id);
    if (!dst->operation_id)
      return ENOMEM;
  }
  if (src->n_parameters > 0 && src->parameters) {
    dst->parameters = (struct OpenAPI_LinkParam *)calloc(
        src->n_parameters, sizeof(struct OpenAPI_LinkParam));
    if (!dst->parameters)
      return ENOMEM;
    dst->n_parameters = src->n_parameters;
    for (i = 0; i < src->n_parameters; ++i) {
      if (src->parameters[i].name) {
        dst->parameters[i].name = c_cdd_strdup(src->parameters[i].name);
        if (!dst->parameters[i].name)
          return ENOMEM;
      }
      if (copy_any_value(&dst->parameters[i].value,
                         &src->parameters[i].value) != 0)
        return ENOMEM;
    }
  }
  if (src->request_body_set) {
    dst->request_body_set = 1;
    if (copy_any_value(&dst->request_body, &src->request_body) != 0)
      return ENOMEM;
  }
  if (src->server_set && src->server) {
    dst->server =
        (struct OpenAPI_Server *)calloc(1, sizeof(struct OpenAPI_Server));
    if (!dst->server)
      return ENOMEM;
    dst->server_set = 1;
    if (copy_server_object(dst->server, src->server) != 0)
      return ENOMEM;
  }
  return 0;
}

static int copy_parameter_fields(struct OpenAPI_Parameter *dst,
                                 const struct OpenAPI_Parameter *src) {
  if (!dst || !src)
    return 0;
  dst->in = src->in;
  dst->required = src->required;
  dst->deprecated = src->deprecated;
  dst->deprecated_set = src->deprecated_set;
  dst->is_array = src->is_array;
  dst->style = src->style;
  dst->explode = src->explode;
  dst->explode_set = src->explode_set;
  dst->allow_reserved = src->allow_reserved;
  dst->allow_reserved_set = src->allow_reserved_set;
  dst->allow_empty_value = src->allow_empty_value;
  dst->allow_empty_value_set = src->allow_empty_value_set;
  dst->example_location = src->example_location;
  if (src->name) {
    dst->name = c_cdd_strdup(src->name);
    if (!dst->name)
      return ENOMEM;
  }
  if (src->type) {
    dst->type = c_cdd_strdup(src->type);
    if (!dst->type)
      return ENOMEM;
  }
  if (src->description) {
    dst->description = c_cdd_strdup(src->description);
    if (!dst->description)
      return ENOMEM;
  }
  if (src->content_type) {
    dst->content_type = c_cdd_strdup(src->content_type);
    if (!dst->content_type)
      return ENOMEM;
  }
  if (src->content_ref) {
    dst->content_ref = c_cdd_strdup(src->content_ref);
    if (!dst->content_ref)
      return ENOMEM;
  }
  if (src->content_media_types && src->n_content_media_types > 0) {
    if (copy_media_type_array(
            &dst->content_media_types, &dst->n_content_media_types,
            src->content_media_types, src->n_content_media_types) != 0)
      return ENOMEM;
  }
  if (src->schema_set) {
    dst->schema_set = 1;
    if (copy_schema_ref(&dst->schema, &src->schema) != 0)
      return ENOMEM;
  }
  if (src->items_type) {
    dst->items_type = c_cdd_strdup(src->items_type);
    if (!dst->items_type)
      return ENOMEM;
  }
  if (src->example_set) {
    if (copy_any_value(&dst->example, &src->example) != 0)
      return ENOMEM;
    dst->example_set = 1;
  }
  if (src->examples && src->n_examples > 0) {
    size_t i;
    dst->examples = (struct OpenAPI_Example *)calloc(
        src->n_examples, sizeof(struct OpenAPI_Example));
    if (!dst->examples)
      return ENOMEM;
    dst->n_examples = src->n_examples;
    for (i = 0; i < src->n_examples; ++i) {
      if (copy_example_fields(&dst->examples[i], &src->examples[i]) != 0)
        return ENOMEM;
    }
  }
  return 0;
}

static int copy_header_fields(struct OpenAPI_Header *dst,
                              const struct OpenAPI_Header *src) {
  if (!dst || !src)
    return 0;
  dst->required = src->required;
  dst->deprecated = src->deprecated;
  dst->deprecated_set = src->deprecated_set;
  dst->style = src->style;
  dst->style_set = src->style_set;
  dst->explode = src->explode;
  dst->explode_set = src->explode_set;
  dst->is_array = src->is_array;
  dst->example_location = src->example_location;
  if (src->description) {
    dst->description = c_cdd_strdup(src->description);
    if (!dst->description)
      return ENOMEM;
  }
  if (src->content_type) {
    dst->content_type = c_cdd_strdup(src->content_type);
    if (!dst->content_type)
      return ENOMEM;
  }
  if (src->content_ref) {
    dst->content_ref = c_cdd_strdup(src->content_ref);
    if (!dst->content_ref)
      return ENOMEM;
  }
  if (src->content_media_types && src->n_content_media_types > 0) {
    if (copy_media_type_array(
            &dst->content_media_types, &dst->n_content_media_types,
            src->content_media_types, src->n_content_media_types) != 0)
      return ENOMEM;
  }
  if (src->schema_set) {
    dst->schema_set = 1;
    if (copy_schema_ref(&dst->schema, &src->schema) != 0)
      return ENOMEM;
  }
  if (src->type) {
    dst->type = c_cdd_strdup(src->type);
    if (!dst->type)
      return ENOMEM;
  }
  if (src->items_type) {
    dst->items_type = c_cdd_strdup(src->items_type);
    if (!dst->items_type)
      return ENOMEM;
  }
  if (src->example_set) {
    if (copy_any_value(&dst->example, &src->example) != 0)
      return ENOMEM;
    dst->example_set = 1;
  }
  if (src->examples && src->n_examples > 0) {
    size_t i;
    dst->examples = (struct OpenAPI_Example *)calloc(
        src->n_examples, sizeof(struct OpenAPI_Example));
    if (!dst->examples)
      return ENOMEM;
    dst->n_examples = src->n_examples;
    for (i = 0; i < src->n_examples; ++i) {
      if (copy_example_fields(&dst->examples[i], &src->examples[i]) != 0)
        return ENOMEM;
    }
  }
  return 0;
}

static int copy_encoding_fields(struct OpenAPI_Encoding *dst,
                                const struct OpenAPI_Encoding *src) {
  size_t i;
  if (!dst || !src)
    return 0;
  dst->style = src->style;
  dst->style_set = src->style_set;
  dst->explode = src->explode;
  dst->explode_set = src->explode_set;
  dst->allow_reserved = src->allow_reserved;
  dst->allow_reserved_set = src->allow_reserved_set;
  if (src->name && !dst->name) {
    dst->name = c_cdd_strdup(src->name);
    if (!dst->name)
      return ENOMEM;
  }
  if (src->content_type) {
    dst->content_type = c_cdd_strdup(src->content_type);
    if (!dst->content_type)
      return ENOMEM;
  }
  if (src->headers && src->n_headers > 0) {
    dst->headers = (struct OpenAPI_Header *)calloc(
        src->n_headers, sizeof(struct OpenAPI_Header));
    if (!dst->headers)
      return ENOMEM;
    dst->n_headers = src->n_headers;
    for (i = 0; i < src->n_headers; ++i) {
      struct OpenAPI_Header *dst_hdr = &dst->headers[i];
      const struct OpenAPI_Header *src_hdr = &src->headers[i];
      if (src_hdr->name) {
        dst_hdr->name = c_cdd_strdup(src_hdr->name);
        if (!dst_hdr->name)
          return ENOMEM;
      }
      if (copy_header_fields(dst_hdr, src_hdr) != 0)
        return ENOMEM;
    }
  }
  if (src->encoding && src->n_encoding > 0) {
    dst->encoding = (struct OpenAPI_Encoding *)calloc(
        src->n_encoding, sizeof(struct OpenAPI_Encoding));
    if (!dst->encoding)
      return ENOMEM;
    dst->n_encoding = src->n_encoding;
    for (i = 0; i < src->n_encoding; ++i) {
      if (copy_encoding_fields(&dst->encoding[i], &src->encoding[i]) != 0)
        return ENOMEM;
    }
  }
  if (src->prefix_encoding && src->n_prefix_encoding > 0) {
    dst->prefix_encoding = (struct OpenAPI_Encoding *)calloc(
        src->n_prefix_encoding, sizeof(struct OpenAPI_Encoding));
    if (!dst->prefix_encoding)
      return ENOMEM;
    dst->n_prefix_encoding = src->n_prefix_encoding;
    for (i = 0; i < src->n_prefix_encoding; ++i) {
      if (copy_encoding_fields(&dst->prefix_encoding[i],
                               &src->prefix_encoding[i]) != 0)
        return ENOMEM;
    }
  }
  if (src->item_encoding) {
    dst->item_encoding =
        (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
    if (!dst->item_encoding)
      return ENOMEM;
    dst->item_encoding_set = 1;
    if (copy_encoding_fields(dst->item_encoding, src->item_encoding) != 0)
      return ENOMEM;
  }
  return 0;
}

static int copy_media_type_fields(struct OpenAPI_MediaType *dst,
                                  const struct OpenAPI_MediaType *src) {
  size_t i;
  if (!dst || !src)
    return 0;
  if (src->name && !dst->name) {
    dst->name = c_cdd_strdup(src->name);
    if (!dst->name)
      return ENOMEM;
  }
  if (src->ref && !dst->ref) {
    dst->ref = c_cdd_strdup(src->ref);
    if (!dst->ref)
      return ENOMEM;
  }
  if (src->schema_set || src->schema.ref_name || src->schema.inline_type ||
      src->schema.is_array || src->schema.n_multipart_fields > 0) {
    if (copy_schema_ref(&dst->schema, &src->schema) != 0)
      return ENOMEM;
    dst->schema_set = 1;
  }
  if (src->item_schema_set || src->item_schema.ref_name ||
      src->item_schema.inline_type || src->item_schema.is_array ||
      src->item_schema.n_multipart_fields > 0) {
    if (copy_schema_ref(&dst->item_schema, &src->item_schema) != 0)
      return ENOMEM;
    dst->item_schema_set = 1;
  }
  if (src->example_set) {
    if (copy_any_value(&dst->example, &src->example) != 0)
      return ENOMEM;
    dst->example_set = 1;
  }
  if (src->examples && src->n_examples > 0) {
    dst->examples = (struct OpenAPI_Example *)calloc(
        src->n_examples, sizeof(struct OpenAPI_Example));
    if (!dst->examples)
      return ENOMEM;
    dst->n_examples = src->n_examples;
    for (i = 0; i < src->n_examples; ++i) {
      if (copy_example_fields(&dst->examples[i], &src->examples[i]) != 0)
        return ENOMEM;
    }
  }
  if (src->encoding && src->n_encoding > 0) {
    dst->encoding = (struct OpenAPI_Encoding *)calloc(
        src->n_encoding, sizeof(struct OpenAPI_Encoding));
    if (!dst->encoding)
      return ENOMEM;
    dst->n_encoding = src->n_encoding;
    for (i = 0; i < src->n_encoding; ++i) {
      if (copy_encoding_fields(&dst->encoding[i], &src->encoding[i]) != 0)
        return ENOMEM;
    }
  }
  if (src->prefix_encoding && src->n_prefix_encoding > 0) {
    dst->prefix_encoding = (struct OpenAPI_Encoding *)calloc(
        src->n_prefix_encoding, sizeof(struct OpenAPI_Encoding));
    if (!dst->prefix_encoding)
      return ENOMEM;
    dst->n_prefix_encoding = src->n_prefix_encoding;
    for (i = 0; i < src->n_prefix_encoding; ++i) {
      if (copy_encoding_fields(&dst->prefix_encoding[i],
                               &src->prefix_encoding[i]) != 0)
        return ENOMEM;
    }
  }
  if (src->item_encoding) {
    dst->item_encoding =
        (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
    if (!dst->item_encoding)
      return ENOMEM;
    dst->item_encoding_set = 1;
    if (copy_encoding_fields(dst->item_encoding, src->item_encoding) != 0)
      return ENOMEM;
  }
  return 0;
}

static int copy_media_type_array(struct OpenAPI_MediaType **dst,
                                 size_t *dst_count,
                                 const struct OpenAPI_MediaType *src,
                                 size_t src_count) {
  size_t i;
  if (!dst || !dst_count)
    return 0;
  *dst = NULL;
  *dst_count = 0;
  if (!src || src_count == 0)
    return 0;
  *dst = (struct OpenAPI_MediaType *)calloc(src_count,
                                            sizeof(struct OpenAPI_MediaType));
  if (!*dst)
    return ENOMEM;
  *dst_count = src_count;
  for (i = 0; i < src_count; ++i) {
    if (copy_media_type_fields(&(*dst)[i], &src[i]) != 0)
      return ENOMEM;
  }
  return 0;
}

static int copy_response_fields(struct OpenAPI_Response *dst,
                                const struct OpenAPI_Response *src) {
  size_t i;
  if (!dst || !src)
    return 0;
  if (src->summary) {
    dst->summary = c_cdd_strdup(src->summary);
    if (!dst->summary)
      return ENOMEM;
  }
  if (src->description) {
    dst->description = c_cdd_strdup(src->description);
    if (!dst->description)
      return ENOMEM;
  }
  if (src->content_type) {
    dst->content_type = c_cdd_strdup(src->content_type);
    if (!dst->content_type)
      return ENOMEM;
  }
  if (src->content_ref) {
    dst->content_ref = c_cdd_strdup(src->content_ref);
    if (!dst->content_ref)
      return ENOMEM;
  }
  if (src->content_media_types && src->n_content_media_types > 0) {
    if (copy_media_type_array(
            &dst->content_media_types, &dst->n_content_media_types,
            src->content_media_types, src->n_content_media_types) != 0)
      return ENOMEM;
  }
  if (src->example_set) {
    if (copy_any_value(&dst->example, &src->example) != 0)
      return ENOMEM;
    dst->example_set = 1;
  }
  if (src->examples && src->n_examples > 0) {
    dst->examples = (struct OpenAPI_Example *)calloc(
        src->n_examples, sizeof(struct OpenAPI_Example));
    if (!dst->examples)
      return ENOMEM;
    dst->n_examples = src->n_examples;
    for (i = 0; i < src->n_examples; ++i) {
      if (copy_example_fields(&dst->examples[i], &src->examples[i]) != 0)
        return ENOMEM;
    }
  }
  if (src->n_headers > 0 && src->headers) {
    dst->headers = (struct OpenAPI_Header *)calloc(
        src->n_headers, sizeof(struct OpenAPI_Header));
    if (!dst->headers)
      return ENOMEM;
    dst->n_headers = src->n_headers;
    for (i = 0; i < src->n_headers; ++i) {
      struct OpenAPI_Header *dst_hdr = &dst->headers[i];
      const struct OpenAPI_Header *src_hdr = &src->headers[i];
      if (src_hdr->name) {
        dst_hdr->name = c_cdd_strdup(src_hdr->name);
        if (!dst_hdr->name)
          return ENOMEM;
      }
      if (copy_header_fields(dst_hdr, src_hdr) != 0)
        return ENOMEM;
    }
  }
  if (src->n_links > 0 && src->links) {
    dst->links = (struct OpenAPI_Link *)calloc(src->n_links,
                                               sizeof(struct OpenAPI_Link));
    if (!dst->links)
      return ENOMEM;
    dst->n_links = src->n_links;
    for (i = 0; i < src->n_links; ++i) {
      struct OpenAPI_Link *dst_link = &dst->links[i];
      const struct OpenAPI_Link *src_link = &src->links[i];
      if (src_link->name) {
        dst_link->name = c_cdd_strdup(src_link->name);
        if (!dst_link->name)
          return ENOMEM;
      }
      if (src_link->ref) {
        dst_link->ref = c_cdd_strdup(src_link->ref);
        if (!dst_link->ref)
          return ENOMEM;
      }
      if (copy_link_fields(dst_link, src_link) != 0)
        return ENOMEM;
    }
  }
  return copy_schema_ref(&dst->schema, &src->schema);
}

static int parse_info(const JSON_Object *const root_obj,
                      struct OpenAPI_Spec *const out) {
  const JSON_Object *info_obj;
  const JSON_Object *contact_obj;
  const JSON_Object *license_obj;
  const char *val;

  if (!root_obj || !out)
    return 0;

  info_obj = json_object_get_object(root_obj, "info");
  if (!info_obj)
    return 0;

  val = json_object_get_string(info_obj, "title");
  if (val) {
    out->info.title = c_cdd_strdup(val);
    if (!out->info.title)
      return ENOMEM;
  }
  val = json_object_get_string(info_obj, "summary");
  if (val) {
    out->info.summary = c_cdd_strdup(val);
    if (!out->info.summary)
      return ENOMEM;
  }
  val = json_object_get_string(info_obj, "description");
  if (val) {
    out->info.description = c_cdd_strdup(val);
    if (!out->info.description)
      return ENOMEM;
  }
  val = json_object_get_string(info_obj, "termsOfService");
  if (val) {
    out->info.terms_of_service = c_cdd_strdup(val);
    if (!out->info.terms_of_service)
      return ENOMEM;
  }
  val = json_object_get_string(info_obj, "version");
  if (val) {
    out->info.version = c_cdd_strdup(val);
    if (!out->info.version)
      return ENOMEM;
  }
  if (collect_extensions(info_obj, &out->info.extensions_json) != 0)
    return ENOMEM;

  contact_obj = json_object_get_object(info_obj, "contact");
  if (contact_obj) {
    val = json_object_get_string(contact_obj, "name");
    if (val) {
      out->info.contact.name = c_cdd_strdup(val);
      if (!out->info.contact.name)
        return ENOMEM;
    }
    val = json_object_get_string(contact_obj, "url");
    if (val) {
      out->info.contact.url = c_cdd_strdup(val);
      if (!out->info.contact.url)
        return ENOMEM;
    }
    val = json_object_get_string(contact_obj, "email");
    if (val) {
      out->info.contact.email = c_cdd_strdup(val);
      if (!out->info.contact.email)
        return ENOMEM;
    }
    if (collect_extensions(contact_obj, &out->info.contact.extensions_json) !=
        0)
      return ENOMEM;
  }

  license_obj = json_object_get_object(info_obj, "license");
  if (license_obj) {
    val = json_object_get_string(license_obj, "name");
    if (val) {
      out->info.license.name = c_cdd_strdup(val);
      if (!out->info.license.name)
        return ENOMEM;
    }
    val = json_object_get_string(license_obj, "identifier");
    if (val) {
      out->info.license.identifier = c_cdd_strdup(val);
      if (!out->info.license.identifier)
        return ENOMEM;
    }
    val = json_object_get_string(license_obj, "url");
    if (val) {
      out->info.license.url = c_cdd_strdup(val);
      if (!out->info.license.url)
        return ENOMEM;
    }
    if (collect_extensions(license_obj, &out->info.license.extensions_json) !=
        0)
      return ENOMEM;
  }

  return 0;
}

static int parse_external_docs(const JSON_Object *const obj,
                               struct OpenAPI_ExternalDocs *const out) {
  const char *desc;
  const char *url;

  if (!obj || !out)
    return 0;

  desc = json_object_get_string(obj, "description");
  if (desc) {
    out->description = c_cdd_strdup(desc);
    if (!out->description)
      return ENOMEM;
  }
  url = json_object_get_string(obj, "url");
  if (url) {
    out->url = c_cdd_strdup(url);
    if (!out->url)
      return ENOMEM;
  }
  if (collect_extensions(obj, &out->extensions_json) != 0)
    return ENOMEM;

  return 0;
}

static int parse_tags(const JSON_Object *const root_obj,
                      struct OpenAPI_Spec *const out) {
  const JSON_Array *tags_arr;
  size_t count, i;

  if (!root_obj || !out)
    return 0;

  tags_arr = json_object_get_array(root_obj, "tags");
  if (!tags_arr)
    return 0;

  count = json_array_get_count(tags_arr);
  if (count == 0)
    return 0;

  out->tags = (struct OpenAPI_Tag *)calloc(count, sizeof(struct OpenAPI_Tag));
  if (!out->tags)
    return ENOMEM;
  out->n_tags = count;

  for (i = 0; i < count; ++i) {
    const JSON_Object *tag_obj = json_array_get_object(tags_arr, i);
    const char *name = json_object_get_string(tag_obj, "name");
    const char *summary = json_object_get_string(tag_obj, "summary");
    const char *description = json_object_get_string(tag_obj, "description");
    const char *parent = json_object_get_string(tag_obj, "parent");
    const char *kind = json_object_get_string(tag_obj, "kind");
    const JSON_Object *ext = json_object_get_object(tag_obj, "externalDocs");

    if (name) {
      size_t k;
      for (k = 0; k < i; ++k) {
        if (out->tags[k].name && strcmp(out->tags[k].name, name) == 0)
          return EINVAL;
      }
      out->tags[i].name = c_cdd_strdup(name);
      if (!out->tags[i].name)
        return ENOMEM;
    }
    if (summary) {
      out->tags[i].summary = c_cdd_strdup(summary);
      if (!out->tags[i].summary)
        return ENOMEM;
    }
    if (description) {
      out->tags[i].description = c_cdd_strdup(description);
      if (!out->tags[i].description)
        return ENOMEM;
    }
    if (parent) {
      out->tags[i].parent = c_cdd_strdup(parent);
      if (!out->tags[i].parent)
        return ENOMEM;
    }
    if (kind) {
      out->tags[i].kind = c_cdd_strdup(kind);
      if (!out->tags[i].kind)
        return ENOMEM;
    }
    if (ext) {
      if (parse_external_docs(ext, &out->tags[i].external_docs) != 0)
        return ENOMEM;
    }
    if (collect_extensions(tag_obj, &out->tags[i].extensions_json) != 0)
      return ENOMEM;
  }

  return 0;
}

static int parse_server_object(const JSON_Object *const srv_obj,
                               struct OpenAPI_Server *const out_srv) {
  const char *url;
  const char *desc;
  const char *name;
  const JSON_Object *vars;

  if (!srv_obj || !out_srv)
    return 0;

  url = json_object_get_string(srv_obj, "url");
  desc = json_object_get_string(srv_obj, "description");
  name = json_object_get_string(srv_obj, "name");
  vars = json_object_get_object(srv_obj, "variables");

  out_srv->url = c_cdd_strdup(url ? url : "/");
  if (!out_srv->url)
    return ENOMEM;

  if (desc) {
    out_srv->description = c_cdd_strdup(desc);
    if (!out_srv->description)
      return ENOMEM;
  }
  if (name) {
    out_srv->name = c_cdd_strdup(name);
    if (!out_srv->name)
      return ENOMEM;
  }
  if (collect_extensions(srv_obj, &out_srv->extensions_json) != 0)
    return ENOMEM;

  if (vars) {
    size_t vcount = json_object_get_count(vars);
    size_t v;
    if (vcount > 0) {
      out_srv->variables = (struct OpenAPI_ServerVariable *)calloc(
          vcount, sizeof(struct OpenAPI_ServerVariable));
      if (!out_srv->variables)
        return ENOMEM;
      out_srv->n_variables = vcount;
      for (v = 0; v < vcount; ++v) {
        const char *vname = json_object_get_name(vars, v);
        const JSON_Object *v_obj =
            json_value_get_object(json_object_get_value_at(vars, v));
        struct OpenAPI_ServerVariable *curr = &out_srv->variables[v];
        if (vname) {
          curr->name = c_cdd_strdup(vname);
          if (!curr->name)
            return ENOMEM;
        }
        if (v_obj) {
          const char *def_val = json_object_get_string(v_obj, "default");
          const char *v_desc = json_object_get_string(v_obj, "description");
          const JSON_Array *enum_arr = json_object_get_array(v_obj, "enum");
          if (def_val) {
            curr->default_value = c_cdd_strdup(def_val);
            if (!curr->default_value)
              return ENOMEM;
          }
          if (v_desc) {
            curr->description = c_cdd_strdup(v_desc);
            if (!curr->description)
              return ENOMEM;
          }
          if (enum_arr) {
            size_t ecount = json_array_get_count(enum_arr);
            size_t e;
            if (ecount > 0) {
              curr->enum_values = (char **)calloc(ecount, sizeof(char *));
              if (!curr->enum_values)
                return ENOMEM;
              curr->n_enum_values = ecount;
              for (e = 0; e < ecount; ++e) {
                const char *e_val = json_array_get_string(enum_arr, e);
                if (e_val) {
                  curr->enum_values[e] = c_cdd_strdup(e_val);
                  if (!curr->enum_values[e])
                    return ENOMEM;
                }
              }
            }
          }
          if (collect_extensions(v_obj, &curr->extensions_json) != 0)
            return ENOMEM;
        }
      }
    }
  }

  return 0;
}

static int parse_servers_array(const JSON_Object *const parent,
                               const char *const key,
                               struct OpenAPI_Server **out_servers,
                               size_t *out_count) {
  const JSON_Array *servers;
  size_t count, i;

  if (!parent || !key || !out_servers || !out_count)
    return 0;

  *out_servers = NULL;
  *out_count = 0;

  servers = json_object_get_array(parent, key);
  if (!servers)
    return 0;

  count = json_array_get_count(servers);
  if (count == 0)
    return 0;

  *out_servers =
      (struct OpenAPI_Server *)calloc(count, sizeof(struct OpenAPI_Server));
  if (!*out_servers)
    return ENOMEM;
  *out_count = count;

  for (i = 0; i < count; ++i) {
    const JSON_Object *srv_obj = json_array_get_object(servers, i);
    if (srv_obj) {
      if (parse_server_object(srv_obj, &(*out_servers)[i]) != 0)
        return ENOMEM;
    }
  }

  return 0;
}

static int
parse_security_requirements(const JSON_Array *const arr,
                            struct OpenAPI_SecurityRequirementSet **out,
                            size_t *out_count) {
  size_t i, count;
  int rc = 0;

  if (!arr || !out || !out_count)
    return 0;

  count = json_array_get_count(arr);
  if (count == 0) {
    *out = NULL;
    *out_count = 0;
    return 0;
  }

  *out = (struct OpenAPI_SecurityRequirementSet *)calloc(
      count, sizeof(struct OpenAPI_SecurityRequirementSet));
  if (!*out)
    return ENOMEM;

  *out_count = count;

  for (i = 0; i < count; ++i) {
    const JSON_Object *sec_obj = json_array_get_object(arr, i);
    struct OpenAPI_SecurityRequirementSet *set = &(*out)[i];
    size_t j, req_count = 0;
    size_t key_count = 0;

    if (sec_obj) {
      key_count = json_object_get_count(sec_obj);
      for (j = 0; j < key_count; ++j) {
        const char *name = json_object_get_name(sec_obj, j);
        if (name && strncmp(name, "x-", 2) != 0)
          req_count++;
      }
      if (collect_extensions(sec_obj, &set->extensions_json) != 0) {
        rc = ENOMEM;
        goto fail;
      }
    }

    if (req_count == 0) {
      set->requirements = NULL;
      set->n_requirements = 0;
      continue;
    }

    set->requirements = (struct OpenAPI_SecurityRequirement *)calloc(
        req_count, sizeof(struct OpenAPI_SecurityRequirement));
    if (!set->requirements) {
      rc = ENOMEM;
      goto fail;
    }
    set->n_requirements = req_count;
    {
      size_t req_idx = 0;
      for (j = 0; j < key_count; ++j) {
        size_t k, n_scopes = 0;
        const char *scheme = json_object_get_name(sec_obj, j);
        const JSON_Array *scopes_arr = json_object_get_array(sec_obj, scheme);
        struct OpenAPI_SecurityRequirement *req;
        if (!scheme || strncmp(scheme, "x-", 2) == 0)
          continue;
        req = &set->requirements[req_idx++];

        req->scheme = c_cdd_strdup(scheme ? scheme : "");
        if (!req->scheme) {
          rc = ENOMEM;
          goto fail;
        }

        if (scopes_arr)
          n_scopes = json_array_get_count(scopes_arr);

        if (n_scopes == 0) {
          req->scopes = NULL;
          req->n_scopes = 0;
          continue;
        }

        req->scopes = (char **)calloc(n_scopes, sizeof(char *));
        if (!req->scopes) {
          rc = ENOMEM;
          goto fail;
        }
        req->n_scopes = n_scopes;

        for (k = 0; k < n_scopes; ++k) {
          const char *scope = json_array_get_string(scopes_arr, k);
          req->scopes[k] = c_cdd_strdup(scope ? scope : "");
          if (!req->scopes[k]) {
            rc = ENOMEM;
            goto fail;
          }
        }
      }
    }
  }

  return 0;

fail:
  if (*out) {
    for (i = 0; i < count; ++i) {
      free_security_requirement_set(&(*out)[i]);
    }
    free(*out);
    *out = NULL;
  }
  *out_count = 0;
  return rc;
}

static int parse_security_field(const JSON_Object *const obj,
                                const char *const key,
                                struct OpenAPI_SecurityRequirementSet **out,
                                size_t *out_count, int *out_set) {
  const JSON_Array *arr;
  if (!obj || !key || !out || !out_count || !out_set)
    return 0;

  if (!json_object_has_value(obj, key)) {
    *out_set = 0;
    return 0;
  }

  *out_set = 1;
  arr = json_object_get_array(obj, key);
  if (!arr) {
    *out = NULL;
    *out_count = 0;
    return 0;
  }

  return parse_security_requirements(arr, out, out_count);
}

static char *clean_ref(const char *full_ref) {
  const char *slash;
  if (!full_ref)
    return NULL;
  slash = strrchr(full_ref, '/');
  if (slash) {
    return c_cdd_strdup(slash + 1);
  }
  return c_cdd_strdup(full_ref);
}

static int schema_is_string_enum_only(const JSON_Object *schema_obj) {
  const JSON_Array *enum_arr;
  size_t i, count;
  const char *type;

  if (!schema_obj)
    return 0;

  enum_arr = json_object_get_array(schema_obj, "enum");
  if (!enum_arr)
    return 0;

  count = json_array_get_count(enum_arr);
  if (count == 0)
    return 0;

  type = json_object_get_string(schema_obj, "type");
  if (type && strcmp(type, "string") != 0)
    return 0;

  for (i = 0; i < count; ++i) {
    if (!json_array_get_string(enum_arr, i))
      return 0;
  }

  return 1;
}

static int schema_is_struct_compatible(const JSON_Value *schema_val,
                                       const JSON_Object *schema_obj) {
  const char *type;
  if (!schema_val || !schema_obj)
    return 0;
  if (json_value_get_type(schema_val) == JSONBoolean)
    return 0;
  if (schema_is_string_enum_only(schema_obj))
    return 1;
  type = json_object_get_string(schema_obj, "type");
  if (type)
    return strcmp(type, "object") == 0;
  if (json_object_get_object(schema_obj, "properties"))
    return 1;
  if (json_object_get_array(schema_obj, "allOf") ||
      json_object_get_array(schema_obj, "anyOf") ||
      json_object_get_array(schema_obj, "oneOf"))
    return 1;
  return 0;
}

static const char *const k_schema_skip_keys[] = {"$ref",
                                                 "type",
                                                 "items",
                                                 "format",
                                                 "contentMediaType",
                                                 "contentEncoding",
                                                 "enum",
                                                 "const",
                                                 "default",
                                                 "examples",
                                                 "example",
                                                 "minimum",
                                                 "maximum",
                                                 "exclusiveMinimum",
                                                 "exclusiveMaximum",
                                                 "minLength",
                                                 "maxLength",
                                                 "pattern",
                                                 "minItems",
                                                 "maxItems",
                                                 "uniqueItems",
                                                 "summary",
                                                 "description",
                                                 "deprecated",
                                                 "readOnly",
                                                 "writeOnly"};

static const char *const k_items_skip_keys[] = {"$ref",
                                                "type",
                                                "format",
                                                "contentMediaType",
                                                "contentEncoding",
                                                "enum",
                                                "const",
                                                "default",
                                                "examples",
                                                "example",
                                                "minimum",
                                                "maximum",
                                                "exclusiveMinimum",
                                                "exclusiveMaximum",
                                                "minLength",
                                                "maxLength",
                                                "pattern",
                                                "minItems",
                                                "maxItems",
                                                "uniqueItems",
                                                "summary",
                                                "description",
                                                "deprecated",
                                                "readOnly",
                                                "writeOnly"};

static int parse_schema_ref(const JSON_Object *const schema,
                            struct OpenAPI_SchemaRef *const out) {
  const char *ref = json_object_get_string(schema, "$ref");
  const char *summary = json_object_get_string(schema, "summary");
  const char *desc = json_object_get_string(schema, "description");
  const char *format = json_object_get_string(schema, "format");
  const char *content_media_type =
      json_object_get_string(schema, "contentMediaType");
  const char *content_encoding =
      json_object_get_string(schema, "contentEncoding");
  const JSON_Array *type_arr;
  const JSON_Array *examples_arr;
  const JSON_Array *enum_arr;
  const char *type;
  int nullable = 0;
  int deprecated_present;
  int deprecated_val;
  int read_only_present;
  int read_only_val;
  int write_only_present;
  int write_only_val;

  if (!out)
    return EINVAL;
  if (!schema)
    return 0;

  out->schema_is_boolean = 0;
  out->schema_boolean_value = 0;
  out->is_array = 0;
  out->ref_name = NULL;
  out->ref = NULL;
  out->inline_type = NULL;
  out->type_union = NULL;
  out->n_type_union = 0;
  out->format = NULL;
  out->items_format = NULL;
  out->items_ref = NULL;
  out->items_type_union = NULL;
  out->n_items_type_union = 0;
  out->content_type = NULL;
  out->content_media_type = NULL;
  out->content_encoding = NULL;
  out->items_content_media_type = NULL;
  out->items_content_encoding = NULL;
  out->nullable = 0;
  out->items_nullable = 0;
  out->summary = NULL;
  out->description = NULL;
  out->deprecated = 0;
  out->deprecated_set = 0;
  out->read_only = 0;
  out->read_only_set = 0;
  out->write_only = 0;
  out->write_only_set = 0;
  out->const_value_set = 0;
  memset(&out->const_value, 0, sizeof(out->const_value));
  out->examples = NULL;
  out->n_examples = 0;
  out->example_set = 0;
  memset(&out->example, 0, sizeof(out->example));
  out->default_value_set = 0;
  memset(&out->default_value, 0, sizeof(out->default_value));
  out->enum_values = NULL;
  out->n_enum_values = 0;
  out->schema_extra_json = NULL;
  out->items_enum_values = NULL;
  out->n_items_enum_values = 0;
  out->has_min = 0;
  out->min_val = 0;
  out->exclusive_min = 0;
  out->has_max = 0;
  out->max_val = 0;
  out->exclusive_max = 0;
  out->has_min_len = 0;
  out->min_len = 0;
  out->has_max_len = 0;
  out->max_len = 0;
  out->pattern = NULL;
  out->has_min_items = 0;
  out->min_items = 0;
  out->has_max_items = 0;
  out->max_items = 0;
  out->unique_items = 0;
  out->items_has_min = 0;
  out->items_min_val = 0;
  out->items_exclusive_min = 0;
  out->items_has_max = 0;
  out->items_max_val = 0;
  out->items_exclusive_max = 0;
  out->items_has_min_len = 0;
  out->items_min_len = 0;
  out->items_has_max_len = 0;
  out->items_max_len = 0;
  out->items_pattern = NULL;
  out->items_has_min_items = 0;
  out->items_min_items = 0;
  out->items_has_max_items = 0;
  out->items_max_items = 0;
  out->items_unique_items = 0;
  out->items_example_set = 0;
  memset(&out->items_example, 0, sizeof(out->items_example));
  out->items_examples = NULL;
  out->n_items_examples = 0;
  out->items_const_value_set = 0;
  memset(&out->items_const_value, 0, sizeof(out->items_const_value));
  out->items_default_value_set = 0;
  memset(&out->items_default_value, 0, sizeof(out->items_default_value));
  out->items_extra_json = NULL;
  out->items_schema_is_boolean = 0;
  out->items_schema_boolean_value = 0;
  out->multipart_fields = NULL;
  out->n_multipart_fields = 0;

  type_arr = json_object_get_array(schema, "type");
  if (type_arr) {
    if (parse_string_enum_array(type_arr, &out->type_union,
                                &out->n_type_union) != 0)
      return ENOMEM;
  }

  type = parse_schema_type(schema, &nullable);
  out->nullable = nullable;

  if (parse_any_field(schema, "default", &out->default_value,
                      &out->default_value_set) != 0)
    return ENOMEM;
  if (collect_schema_extras(schema, k_schema_skip_keys,
                            sizeof(k_schema_skip_keys) /
                                sizeof(k_schema_skip_keys[0]),
                            &out->schema_extra_json) != 0)
    return ENOMEM;
  {
    struct SchemaConstraintTarget target = {
        &out->has_min,      &out->min_val,       &out->exclusive_min,
        &out->has_max,      &out->max_val,       &out->exclusive_max,
        &out->has_min_len,  &out->min_len,       &out->has_max_len,
        &out->max_len,      &out->pattern,       &out->has_min_items,
        &out->min_items,    &out->has_max_items, &out->max_items,
        &out->unique_items, &out->example,       &out->example_set,
    };
    if (parse_schema_constraints(schema, &target) != 0)
      return ENOMEM;
  }

  enum_arr = json_object_get_array(schema, "enum");
  if (enum_arr) {
    if (parse_any_array(enum_arr, &out->enum_values, &out->n_enum_values) != 0)
      return ENOMEM;
  }

  if (format) {
    out->format = c_cdd_strdup(format);
    if (!out->format)
      return ENOMEM;
  }
  if (content_media_type) {
    out->content_media_type = c_cdd_strdup(content_media_type);
    if (!out->content_media_type)
      return ENOMEM;
  }
  if (content_encoding) {
    out->content_encoding = c_cdd_strdup(content_encoding);
    if (!out->content_encoding)
      return ENOMEM;
  }

  if (ref && summary) {
    out->summary = c_cdd_strdup(summary);
    if (!out->summary)
      return ENOMEM;
  }
  if (desc) {
    out->description = c_cdd_strdup(desc);
    if (!out->description)
      return ENOMEM;
  }

  deprecated_present = json_object_has_value(schema, "deprecated");
  deprecated_val = json_object_get_boolean(schema, "deprecated");
  if (deprecated_present) {
    out->deprecated_set = 1;
    out->deprecated = (deprecated_val == 1);
  }
  read_only_present = json_object_has_value(schema, "readOnly");
  read_only_val = json_object_get_boolean(schema, "readOnly");
  if (read_only_present) {
    out->read_only_set = 1;
    out->read_only = (read_only_val == 1);
  }
  write_only_present = json_object_has_value(schema, "writeOnly");
  write_only_val = json_object_get_boolean(schema, "writeOnly");
  if (write_only_present) {
    out->write_only_set = 1;
    out->write_only = (write_only_val == 1);
  }

  if (parse_any_field(schema, "const", &out->const_value,
                      &out->const_value_set) != 0)
    return ENOMEM;

  examples_arr = json_object_get_array(schema, "examples");
  if (examples_arr) {
    if (parse_any_array(examples_arr, &out->examples, &out->n_examples) != 0)
      return ENOMEM;
  }

  if (ref) {
    const char *name_enc = ref_name_from_prefix(ref, "#/components/schemas/");
    char *name_dec = NULL;
    out->ref = c_cdd_strdup(ref);
    if (!out->ref)
      return ENOMEM;
    if (name_enc) {
      name_dec = json_pointer_unescape(name_enc);
      if (!name_dec)
        return ENOMEM;
      out->ref_name = name_dec;
    }
    return 0;
  }

  if (type && strcmp(type, "array") == 0) {
    out->is_array = 1;
    const JSON_Value *items_val = json_object_get_value(schema, "items");
    const JSON_Object *items =
        items_val ? json_value_get_object(items_val) : NULL;
    if (items_val && json_value_get_type(items_val) == JSONBoolean) {
      out->items_schema_is_boolean = 1;
      out->items_schema_boolean_value = json_value_get_boolean(items_val);
      return 0;
    }
    if (items) {
      const char *item_ref = json_object_get_string(items, "$ref");
      const char *item_format = json_object_get_string(items, "format");
      const char *item_type = NULL;
      const JSON_Array *item_types_arr = NULL;
      const JSON_Array *item_enum = NULL;
      const JSON_Array *item_examples = NULL;
      int items_nullable = 0;
      const char *item_content_media_type =
          json_object_get_string(items, "contentMediaType");
      const char *item_content_encoding =
          json_object_get_string(items, "contentEncoding");
      {
        struct SchemaConstraintTarget target = {
            &out->items_has_min,       &out->items_min_val,
            &out->items_exclusive_min, &out->items_has_max,
            &out->items_max_val,       &out->items_exclusive_max,
            &out->items_has_min_len,   &out->items_min_len,
            &out->items_has_max_len,   &out->items_max_len,
            &out->items_pattern,       &out->items_has_min_items,
            &out->items_min_items,     &out->items_has_max_items,
            &out->items_max_items,     &out->items_unique_items,
            &out->items_example,       &out->items_example_set,
        };
        if (parse_schema_constraints(items, &target) != 0)
          return ENOMEM;
      }
      item_type = parse_schema_type(items, &items_nullable);
      out->items_nullable = items_nullable;
      item_types_arr = json_object_get_array(items, "type");
      if (item_types_arr) {
        if (parse_string_enum_array(item_types_arr, &out->items_type_union,
                                    &out->n_items_type_union) != 0)
          return ENOMEM;
      }
      if (item_format) {
        out->items_format = c_cdd_strdup(item_format);
        if (!out->items_format)
          return ENOMEM;
      }
      if (item_content_media_type) {
        out->items_content_media_type = c_cdd_strdup(item_content_media_type);
        if (!out->items_content_media_type)
          return ENOMEM;
      }
      if (item_content_encoding) {
        out->items_content_encoding = c_cdd_strdup(item_content_encoding);
        if (!out->items_content_encoding)
          return ENOMEM;
      }
      item_enum = json_object_get_array(items, "enum");
      if (item_enum) {
        if (parse_any_array(item_enum, &out->items_enum_values,
                            &out->n_items_enum_values) != 0)
          return ENOMEM;
      }
      item_examples = json_object_get_array(items, "examples");
      if (item_examples) {
        if (parse_any_array(item_examples, &out->items_examples,
                            &out->n_items_examples) != 0)
          return ENOMEM;
      }
      if (parse_any_field(items, "const", &out->items_const_value,
                          &out->items_const_value_set) != 0)
        return ENOMEM;
      if (parse_any_field(items, "default", &out->items_default_value,
                          &out->items_default_value_set) != 0)
        return ENOMEM;
      if (collect_schema_extras(items, k_items_skip_keys,
                                sizeof(k_items_skip_keys) /
                                    sizeof(k_items_skip_keys[0]),
                                &out->items_extra_json) != 0)
        return ENOMEM;
      if (item_ref) {
        const char *name_enc =
            ref_name_from_prefix(item_ref, "#/components/schemas/");
        char *name_dec = NULL;
        out->items_ref = c_cdd_strdup(item_ref);
        if (!out->items_ref)
          return ENOMEM;
        if (name_enc) {
          name_dec = json_pointer_unescape(name_enc);
          if (!name_dec)
            return ENOMEM;
          out->ref_name = name_dec;
        }
        return 0;
      }
      if (item_type) {
        out->inline_type = c_cdd_strdup(item_type);
        return out->inline_type ? 0 : ENOMEM;
      }
    }
    return 0;
  }

  if (type) {
    out->inline_type = c_cdd_strdup(type);
    return out->inline_type ? 0 : ENOMEM;
  }

  return 0;
}

static int
apply_schema_ref_to_param(struct OpenAPI_Parameter *const out_param,
                          const struct OpenAPI_SchemaRef *const schema_ref) {
  if (!out_param || !schema_ref)
    return 0;
  if (!schema_ref->ref_name && !schema_ref->inline_type &&
      !schema_ref->is_array)
    return 0;

  if (out_param->type) {
    free(out_param->type);
    out_param->type = NULL;
  }
  if (out_param->items_type) {
    free(out_param->items_type);
    out_param->items_type = NULL;
  }

  if (schema_ref->is_array) {
    out_param->is_array = 1;
    out_param->type = c_cdd_strdup("array");
    if (!out_param->type)
      return ENOMEM;
    if (schema_ref->inline_type) {
      out_param->items_type = c_cdd_strdup(schema_ref->inline_type);
      if (!out_param->items_type)
        return ENOMEM;
    } else if (schema_ref->ref_name) {
      out_param->items_type = c_cdd_strdup(schema_ref->ref_name);
      if (!out_param->items_type)
        return ENOMEM;
    }
    return 0;
  }

  out_param->is_array = 0;
  if (schema_ref->inline_type) {
    out_param->type = c_cdd_strdup(schema_ref->inline_type);
  } else if (schema_ref->ref_name) {
    out_param->type = c_cdd_strdup(schema_ref->ref_name);
  }
  if (!out_param->type)
    return ENOMEM;

  return 0;
}

static int
apply_schema_ref_to_header(struct OpenAPI_Header *const out_hdr,
                           const struct OpenAPI_SchemaRef *const schema_ref) {
  if (!out_hdr || !schema_ref)
    return 0;
  if (!schema_ref->ref_name && !schema_ref->inline_type &&
      !schema_ref->is_array)
    return 0;

  if (out_hdr->type) {
    free(out_hdr->type);
    out_hdr->type = NULL;
  }
  if (out_hdr->items_type) {
    free(out_hdr->items_type);
    out_hdr->items_type = NULL;
  }

  if (schema_ref->is_array) {
    out_hdr->is_array = 1;
    out_hdr->type = c_cdd_strdup("array");
    if (!out_hdr->type)
      return ENOMEM;
    if (schema_ref->inline_type) {
      out_hdr->items_type = c_cdd_strdup(schema_ref->inline_type);
      if (!out_hdr->items_type)
        return ENOMEM;
    } else if (schema_ref->ref_name) {
      out_hdr->items_type = c_cdd_strdup(schema_ref->ref_name);
      if (!out_hdr->items_type)
        return ENOMEM;
    }
    return 0;
  }

  out_hdr->is_array = 0;
  if (schema_ref->inline_type) {
    out_hdr->type = c_cdd_strdup(schema_ref->inline_type);
  } else if (schema_ref->ref_name) {
    out_hdr->type = c_cdd_strdup(schema_ref->ref_name);
  }
  if (!out_hdr->type)
    return ENOMEM;

  return 0;
}

static int parse_servers(const JSON_Object *const root_obj,
                         struct OpenAPI_Spec *const out) {
  return parse_servers_array(root_obj, "servers", &out->servers,
                             &out->n_servers);
}

static int parse_security_schemes(const JSON_Object *const components,
                                  struct OpenAPI_Spec *const out) {
  const JSON_Object *schemes;
  size_t count, i;

  if (!components || !out)
    return 0;

  schemes = json_object_get_object(components, "securitySchemes");
  if (!schemes)
    return 0;

  if (validate_component_key_map(schemes) != 0)
    return EINVAL;

  count = json_object_get_count(schemes);
  if (count == 0)
    return 0;

  out->security_schemes = (struct OpenAPI_SecurityScheme *)calloc(
      count, sizeof(struct OpenAPI_SecurityScheme));
  if (!out->security_schemes)
    return ENOMEM;
  out->n_security_schemes = count;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(schemes, i);
    const JSON_Object *sec_obj =
        json_value_get_object(json_object_get_value_at(schemes, i));
    if (!component_key_is_valid(name))
      return EINVAL;
    out->security_schemes[i].name = c_cdd_strdup(name ? name : "");
    if (!out->security_schemes[i].name)
      return ENOMEM;
    out->security_schemes[i].type = OA_SEC_UNKNOWN;

    if (!sec_obj)
      continue;

    {
      const char *type = json_object_get_string(sec_obj, "type");
      out->security_schemes[i].type = parse_security_type(type);
    }

    {
      const char *desc = json_object_get_string(sec_obj, "description");
      if (desc) {
        out->security_schemes[i].description = c_cdd_strdup(desc);
        if (!out->security_schemes[i].description)
          return ENOMEM;
      }
    }
    if (json_object_has_value(sec_obj, "deprecated")) {
      out->security_schemes[i].deprecated_set = 1;
      out->security_schemes[i].deprecated =
          json_object_get_boolean(sec_obj, "deprecated") == 1;
    }
    if (collect_extensions(sec_obj,
                           &out->security_schemes[i].extensions_json) != 0)
      return ENOMEM;

    if (out->security_schemes[i].type == OA_SEC_APIKEY) {
      const char *in = json_object_get_string(sec_obj, "in");
      const char *key_name = json_object_get_string(sec_obj, "name");
      out->security_schemes[i].in = parse_security_in(in);
      if (key_name) {
        out->security_schemes[i].key_name = c_cdd_strdup(key_name);
        if (!out->security_schemes[i].key_name)
          return ENOMEM;
      }
    } else if (out->security_schemes[i].type == OA_SEC_HTTP) {
      const char *scheme = json_object_get_string(sec_obj, "scheme");
      const char *bearer_format =
          json_object_get_string(sec_obj, "bearerFormat");
      if (scheme) {
        out->security_schemes[i].scheme = c_cdd_strdup(scheme);
        if (!out->security_schemes[i].scheme)
          return ENOMEM;
      }
      if (bearer_format) {
        out->security_schemes[i].bearer_format = c_cdd_strdup(bearer_format);
        if (!out->security_schemes[i].bearer_format)
          return ENOMEM;
      }
    } else if (out->security_schemes[i].type == OA_SEC_OPENID) {
      const char *oid_url = json_object_get_string(sec_obj, "openIdConnectUrl");
      if (oid_url) {
        out->security_schemes[i].open_id_connect_url = c_cdd_strdup(oid_url);
        if (!out->security_schemes[i].open_id_connect_url)
          return ENOMEM;
      }
    } else if (out->security_schemes[i].type == OA_SEC_OAUTH2) {
      const char *meta_url =
          json_object_get_string(sec_obj, "oauth2MetadataUrl");
      const JSON_Object *flows_obj = json_object_get_object(sec_obj, "flows");
      if (meta_url) {
        out->security_schemes[i].oauth2_metadata_url = c_cdd_strdup(meta_url);
        if (!out->security_schemes[i].oauth2_metadata_url)
          return ENOMEM;
      }
      if (flows_obj) {
        if (parse_oauth_flows(flows_obj, &out->security_schemes[i]) != 0)
          return ENOMEM;
      }
    }
  }

  return 0;
}

static int parse_header_object(const JSON_Object *const hdr_obj,
                               struct OpenAPI_Header *const out_hdr,
                               const struct OpenAPI_Spec *const spec,
                               const int resolve_refs) {
  const char *ref;
  const char *desc;
  const char *style_str;
  int required_present;
  int required_val;
  int deprecated_present;
  int deprecated_val;
  int explode_present;
  int explode_val;
  const JSON_Value *schema_val;
  const JSON_Object *schema;
  const JSON_Object *content;
  const JSON_Object *effective_schema;
  const JSON_Value *effective_schema_val;
  const JSON_Object *media_obj;
  const JSON_Value *media_schema_val;
  const char *media_type;
  const char *media_ref;
  const struct OpenAPI_SchemaRef *resolved_schema;
  struct OpenAPI_SchemaRef parsed_schema;
  int parsed_schema_set;
  const char *type;

  if (!hdr_obj || !out_hdr)
    return 0;

  ref = json_object_get_string(hdr_obj, "$ref");
  if (ref) {
    out_hdr->ref = c_cdd_strdup(ref);
    if (!out_hdr->ref)
      return ENOMEM;
    if (resolve_refs && spec) {
      const struct OpenAPI_Header *comp = find_component_header(spec, ref);
      if (comp) {
        if (copy_header_fields(out_hdr, comp) != 0)
          return ENOMEM;
      }
    }
    desc = json_object_get_string(hdr_obj, "description");
    if (desc) {
      out_hdr->description = c_cdd_strdup(desc);
      if (!out_hdr->description)
        return ENOMEM;
    }
    return 0;
  }

  desc = json_object_get_string(hdr_obj, "description");
  if (desc) {
    out_hdr->description = c_cdd_strdup(desc);
    if (!out_hdr->description)
      return ENOMEM;
  }

  required_present = json_object_has_value(hdr_obj, "required");
  required_val = json_object_get_boolean(hdr_obj, "required");
  if (required_present)
    out_hdr->required = (required_val == 1);

  deprecated_present = json_object_has_value(hdr_obj, "deprecated");
  deprecated_val = json_object_get_boolean(hdr_obj, "deprecated");
  if (deprecated_present) {
    out_hdr->deprecated_set = 1;
    out_hdr->deprecated = (deprecated_val == 1);
  }

  style_str = json_object_get_string(hdr_obj, "style");
  if (style_str) {
    enum OpenAPI_Style parsed_style = parse_param_style(style_str);
    if (parsed_style != OA_STYLE_SIMPLE)
      return EINVAL;
    out_hdr->style_set = 1;
    out_hdr->style = parsed_style;
  } else {
    out_hdr->style = OA_STYLE_SIMPLE;
  }

  explode_present = json_object_has_value(hdr_obj, "explode");
  explode_val = json_object_get_boolean(hdr_obj, "explode");
  if (explode_present) {
    out_hdr->explode_set = 1;
    out_hdr->explode = explode_val;
  }

  schema_val = json_object_get_value(hdr_obj, "schema");
  schema = schema_val ? json_value_get_object(schema_val) : NULL;
  content = json_object_get_object(hdr_obj, "content");
  {
    const int has_schema = (schema_val != NULL);
    const int has_content = (content != NULL);
    if (has_schema && has_content)
      return EINVAL;
    if (!has_schema && !has_content)
      return EINVAL;
  }
  if (content) {
    if (json_object_get_count(content) != 1)
      return EINVAL;
    if (parse_content_object(content, &out_hdr->content_media_types,
                             &out_hdr->n_content_media_types, spec,
                             resolve_refs) != 0)
      return ENOMEM;
  }
  effective_schema = schema;
  effective_schema_val = schema_val;
  media_obj = NULL;
  media_schema_val = NULL;
  media_type = NULL;
  media_ref = NULL;
  resolved_schema = NULL;
  parsed_schema_set = 0;

  if (content) {
    size_t ccount = json_object_get_count(content);
    if (ccount > 0) {
      media_type = json_object_get_name(content, 0);
      if (media_type) {
        media_obj = json_object_get_object(content, media_type);
      }
    }
    if (media_type) {
      out_hdr->content_type = c_cdd_strdup(media_type);
      if (!out_hdr->content_type)
        return ENOMEM;
    }
    if (media_obj) {
      media_ref = json_object_get_string(media_obj, "$ref");
      if (media_ref) {
        out_hdr->content_ref = c_cdd_strdup(media_ref);
        if (!out_hdr->content_ref)
          return ENOMEM;
        if (resolve_refs && spec) {
          const struct OpenAPI_MediaType *mt =
              find_component_media_type(spec, media_ref);
          if (mt) {
            if (mt->schema_set)
              resolved_schema = &mt->schema;
            else if (mt->item_schema_set)
              resolved_schema = &mt->item_schema;
          }
        }
      } else {
        media_schema_val = json_object_get_value(media_obj, "schema");
        effective_schema_val = media_schema_val;
        effective_schema =
            media_schema_val ? json_value_get_object(media_schema_val) : NULL;
      }
    }
  }

  type = effective_schema ? json_object_get_string(effective_schema, "type")
                          : NULL;

  if (resolved_schema) {
    if (apply_schema_ref_to_header(out_hdr, resolved_schema) != 0)
      return ENOMEM;
    if (copy_schema_ref(&out_hdr->schema, resolved_schema) != 0)
      return ENOMEM;
    out_hdr->schema_set = 1;
  } else if (effective_schema_val &&
             json_value_get_type(effective_schema_val) == JSONBoolean) {
    out_hdr->schema.schema_is_boolean = 1;
    out_hdr->schema.schema_boolean_value =
        json_value_get_boolean(effective_schema_val);
    out_hdr->schema_set = 1;
  } else if (effective_schema) {
    if (parse_schema_ref(effective_schema, &parsed_schema) != 0)
      return ENOMEM;
    parsed_schema_set = 1;
    if (apply_schema_ref_to_header(out_hdr, &parsed_schema) != 0) {
      free_schema_ref_content(&parsed_schema);
      return ENOMEM;
    }
    if (copy_schema_ref(&out_hdr->schema, &parsed_schema) != 0) {
      free_schema_ref_content(&parsed_schema);
      return ENOMEM;
    }
    out_hdr->schema_set = 1;
  }

  if (!out_hdr->type) {
    out_hdr->type = c_cdd_strdup(type ? type : "string");
    if (!out_hdr->type) {
      if (parsed_schema_set)
        free_schema_ref_content(&parsed_schema);
      return ENOMEM;
    }
  }

  if (parse_examples_object(json_object_get_object(hdr_obj, "examples"),
                            &out_hdr->examples, &out_hdr->n_examples, spec,
                            resolve_refs) != 0)
    return ENOMEM;
  if (out_hdr->n_examples == 0) {
    if (parse_any_field(hdr_obj, "example", &out_hdr->example,
                        &out_hdr->example_set) != 0)
      return ENOMEM;
  }
  if (out_hdr->example_set || out_hdr->n_examples > 0) {
    out_hdr->example_location = OA_EXAMPLE_LOC_OBJECT;
  } else if (media_obj && !media_ref) {
    if (parse_media_examples(media_obj, &out_hdr->example,
                             &out_hdr->example_set, &out_hdr->examples,
                             &out_hdr->n_examples, spec, resolve_refs) != 0)
      return ENOMEM;
    if (out_hdr->example_set || out_hdr->n_examples > 0) {
      out_hdr->example_location = OA_EXAMPLE_LOC_MEDIA;
    }
  }

  if (collect_extensions(hdr_obj, &out_hdr->extensions_json) != 0)
    return ENOMEM;

  if (parsed_schema_set)
    free_schema_ref_content(&parsed_schema);

  return 0;
}

static int parse_link_parameters(const JSON_Object *const params_obj,
                                 struct OpenAPI_LinkParam **out_params,
                                 size_t *out_count) {
  size_t count, i;
  if (!out_params || !out_count)
    return 0;
  *out_params = NULL;
  *out_count = 0;
  if (!params_obj)
    return 0;

  count = json_object_get_count(params_obj);
  if (count == 0)
    return 0;

  *out_params = (struct OpenAPI_LinkParam *)calloc(
      count, sizeof(struct OpenAPI_LinkParam));
  if (!*out_params)
    return ENOMEM;
  *out_count = count;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(params_obj, i);
    const JSON_Value *val = json_object_get_value_at(params_obj, i);
    if (name) {
      (*out_params)[i].name = c_cdd_strdup(name);
      if (!(*out_params)[i].name)
        return ENOMEM;
    }
    if (val) {
      if (parse_any_value(val, &(*out_params)[i].value) != 0)
        return ENOMEM;
    }
  }

  return 0;
}

static int parse_link_object(const JSON_Object *const link_obj,
                             struct OpenAPI_Link *const out_link,
                             const struct OpenAPI_Spec *const spec,
                             const int resolve_refs) {
  const char *ref;
  const char *summary;
  const char *desc;
  const char *op_ref;
  const char *op_id;
  const JSON_Object *params_obj;
  const JSON_Value *req_body_val;
  const JSON_Object *server_obj;

  if (!link_obj || !out_link)
    return 0;

  ref = json_object_get_string(link_obj, "$ref");
  if (ref) {
    out_link->ref = c_cdd_strdup(ref);
    if (!out_link->ref)
      return ENOMEM;
    if (resolve_refs && spec) {
      const struct OpenAPI_Link *comp = find_component_link(spec, ref);
      if (comp) {
        if (copy_link_fields(out_link, comp) != 0)
          return ENOMEM;
      }
    }
    summary = json_object_get_string(link_obj, "summary");
    if (summary) {
      out_link->summary = c_cdd_strdup(summary);
      if (!out_link->summary)
        return ENOMEM;
    }
    desc = json_object_get_string(link_obj, "description");
    if (desc) {
      out_link->description = c_cdd_strdup(desc);
      if (!out_link->description)
        return ENOMEM;
    }
    return 0;
  }

  summary = json_object_get_string(link_obj, "summary");
  if (summary) {
    out_link->summary = c_cdd_strdup(summary);
    if (!out_link->summary)
      return ENOMEM;
  }
  desc = json_object_get_string(link_obj, "description");
  if (desc) {
    out_link->description = c_cdd_strdup(desc);
    if (!out_link->description)
      return ENOMEM;
  }

  op_ref = json_object_get_string(link_obj, "operationRef");
  if (op_ref) {
    out_link->operation_ref = c_cdd_strdup(op_ref);
    if (!out_link->operation_ref)
      return ENOMEM;
  }
  op_id = json_object_get_string(link_obj, "operationId");
  if (op_id) {
    out_link->operation_id = c_cdd_strdup(op_id);
    if (!out_link->operation_id)
      return ENOMEM;
  }
  if (collect_extensions(link_obj, &out_link->extensions_json) != 0)
    return ENOMEM;

  params_obj = json_object_get_object(link_obj, "parameters");
  if (parse_link_parameters(params_obj, &out_link->parameters,
                            &out_link->n_parameters) != 0)
    return ENOMEM;

  req_body_val = json_object_get_value(link_obj, "requestBody");
  if (req_body_val) {
    out_link->request_body_set = 1;
    if (parse_any_value(req_body_val, &out_link->request_body) != 0)
      return ENOMEM;
  }

  server_obj = json_object_get_object(link_obj, "server");
  if (server_obj) {
    out_link->server =
        (struct OpenAPI_Server *)calloc(1, sizeof(struct OpenAPI_Server));
    if (!out_link->server)
      return ENOMEM;
    out_link->server_set = 1;
    if (parse_server_object(server_obj, out_link->server) != 0)
      return ENOMEM;
  }

  return 0;
}

static int parse_links_object(const JSON_Object *const links,
                              struct OpenAPI_Link **out_links,
                              size_t *out_count,
                              const struct OpenAPI_Spec *const spec,
                              const int resolve_refs) {
  size_t i, count;
  if (!out_links || !out_count)
    return 0;

  *out_links = NULL;
  *out_count = 0;

  if (!links)
    return 0;

  count = json_object_get_count(links);
  if (count == 0)
    return 0;

  *out_links =
      (struct OpenAPI_Link *)calloc(count, sizeof(struct OpenAPI_Link));
  if (!*out_links)
    return ENOMEM;
  *out_count = count;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(links, i);
    const JSON_Object *link_obj =
        json_value_get_object(json_object_get_value_at(links, i));
    struct OpenAPI_Link *curr = &(*out_links)[i];
    if (name) {
      curr->name = c_cdd_strdup(name);
      if (!curr->name)
        return ENOMEM;
    }
    if (link_obj) {
      if (parse_link_object(link_obj, curr, spec, resolve_refs) != 0)
        return ENOMEM;
    }
  }

  return 0;
}

static int parse_headers_object(const JSON_Object *const headers,
                                struct OpenAPI_Header **out_headers,
                                size_t *out_count,
                                const struct OpenAPI_Spec *const spec,
                                const int resolve_refs,
                                const int ignore_content_type) {
  size_t i, count, valid = 0;
  if (!out_headers || !out_count)
    return 0;

  *out_headers = NULL;
  *out_count = 0;

  if (!headers)
    return 0;

  count = json_object_get_count(headers);
  if (count == 0)
    return 0;

  *out_headers =
      (struct OpenAPI_Header *)calloc(count, sizeof(struct OpenAPI_Header));
  if (!*out_headers)
    return ENOMEM;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(headers, i);
    const JSON_Object *h_obj =
        json_value_get_object(json_object_get_value_at(headers, i));
    struct OpenAPI_Header *curr = &(*out_headers)[valid];
    if (ignore_content_type && header_name_is_content_type(name))
      continue;
    if (name) {
      curr->name = c_cdd_strdup(name);
      if (!curr->name)
        return ENOMEM;
    }
    if (h_obj) {
      int rc = parse_header_object(h_obj, curr, spec, resolve_refs);
      if (rc != 0)
        return rc;
    }
    valid++;
  }

  if (valid == 0) {
    free(*out_headers);
    *out_headers = NULL;
    *out_count = 0;
    return 0;
  }
  if (valid < count) {
    struct OpenAPI_Header *tmp = (struct OpenAPI_Header *)realloc(
        *out_headers, valid * sizeof(struct OpenAPI_Header));
    if (tmp)
      *out_headers = tmp;
  }
  *out_count = valid;
  return 0;
}

static int parse_encoding_object(const JSON_Object *const enc_obj,
                                 struct OpenAPI_Encoding *const out,
                                 const struct OpenAPI_Spec *const spec,
                                 const int resolve_refs) {
  const char *content_type;
  const char *style_str;
  int explode_present;
  int explode_val;
  int allow_reserved_present;
  int allow_reserved_val;
  const JSON_Object *headers_obj;
  const JSON_Object *nested_encoding_obj;
  const JSON_Array *prefix_encoding_arr;
  const JSON_Object *item_encoding_obj;

  if (!enc_obj || !out)
    return 0;

  {
    const int has_encoding = json_object_has_value(enc_obj, "encoding");
    const int has_prefix = json_object_has_value(enc_obj, "prefixEncoding");
    const int has_item = json_object_has_value(enc_obj, "itemEncoding");
    if (has_encoding && (has_prefix || has_item))
      return EINVAL;
  }

  content_type = json_object_get_string(enc_obj, "contentType");
  if (content_type) {
    out->content_type = c_cdd_strdup(content_type);
    if (!out->content_type)
      return ENOMEM;
  }

  style_str = json_object_get_string(enc_obj, "style");
  if (style_str) {
    out->style = parse_param_style(style_str);
    out->style_set = 1;
  }
  explode_present = json_object_has_value(enc_obj, "explode");
  explode_val = json_object_get_boolean(enc_obj, "explode");
  if (explode_present) {
    out->explode_set = 1;
    out->explode = (explode_val == 1);
  }
  allow_reserved_present = json_object_has_value(enc_obj, "allowReserved");
  allow_reserved_val = json_object_get_boolean(enc_obj, "allowReserved");
  if (allow_reserved_present) {
    out->allow_reserved_set = 1;
    out->allow_reserved = (allow_reserved_val == 1);
  }

  headers_obj = json_object_get_object(enc_obj, "headers");
  if (headers_obj) {
    int rc = parse_headers_object(headers_obj, &out->headers, &out->n_headers,
                                  spec, resolve_refs, 1);
    if (rc != 0)
      return rc;
  }

  nested_encoding_obj = json_object_get_object(enc_obj, "encoding");
  if (nested_encoding_obj) {
    if (parse_encoding_map(nested_encoding_obj, &out->encoding,
                           &out->n_encoding, spec, resolve_refs) != 0)
      return ENOMEM;
  }

  prefix_encoding_arr = json_object_get_array(enc_obj, "prefixEncoding");
  if (prefix_encoding_arr) {
    if (parse_encoding_array(prefix_encoding_arr, &out->prefix_encoding,
                             &out->n_prefix_encoding, spec, resolve_refs) != 0)
      return ENOMEM;
  }

  item_encoding_obj = json_object_get_object(enc_obj, "itemEncoding");
  if (item_encoding_obj) {
    out->item_encoding =
        (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
    if (!out->item_encoding)
      return ENOMEM;
    out->item_encoding_set = 1;
    if (parse_encoding_object(item_encoding_obj, out->item_encoding, spec,
                              resolve_refs) != 0)
      return ENOMEM;
  }

  if (collect_extensions(enc_obj, &out->extensions_json) != 0)
    return ENOMEM;

  return 0;
}

static int parse_encoding_map(const JSON_Object *const enc_obj,
                              struct OpenAPI_Encoding **out, size_t *out_count,
                              const struct OpenAPI_Spec *const spec,
                              const int resolve_refs) {
  size_t i, count, valid = 0;
  if (!out || !out_count)
    return 0;
  *out = NULL;
  *out_count = 0;
  if (!enc_obj)
    return 0;
  count = json_object_get_count(enc_obj);
  if (count == 0)
    return 0;

  *out =
      (struct OpenAPI_Encoding *)calloc(count, sizeof(struct OpenAPI_Encoding));
  if (!*out)
    return ENOMEM;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(enc_obj, i);
    const JSON_Object *enc_def =
        json_value_get_object(json_object_get_value_at(enc_obj, i));
    struct OpenAPI_Encoding *curr = &(*out)[valid];
    if (!name || !enc_def)
      continue;
    curr->name = c_cdd_strdup(name);
    if (!curr->name)
      return ENOMEM;
    {
      int rc = parse_encoding_object(enc_def, curr, spec, resolve_refs);
      if (rc != 0)
        return rc;
    }
    valid++;
  }

  if (valid == 0) {
    free(*out);
    *out = NULL;
    *out_count = 0;
    return 0;
  }
  *out_count = valid;
  return 0;
}

static int parse_encoding_array(const JSON_Array *const enc_arr,
                                struct OpenAPI_Encoding **out,
                                size_t *out_count,
                                const struct OpenAPI_Spec *const spec,
                                const int resolve_refs) {
  size_t i, count, valid = 0;
  if (!out || !out_count)
    return 0;
  *out = NULL;
  *out_count = 0;
  if (!enc_arr)
    return 0;
  count = json_array_get_count(enc_arr);
  if (count == 0)
    return 0;

  *out =
      (struct OpenAPI_Encoding *)calloc(count, sizeof(struct OpenAPI_Encoding));
  if (!*out)
    return ENOMEM;

  for (i = 0; i < count; ++i) {
    const JSON_Object *enc_def = json_array_get_object(enc_arr, i);
    struct OpenAPI_Encoding *curr = &(*out)[valid];
    if (!enc_def)
      continue;
    {
      int rc = parse_encoding_object(enc_def, curr, spec, resolve_refs);
      if (rc != 0)
        return rc;
    }
    valid++;
  }

  if (valid == 0) {
    free(*out);
    *out = NULL;
    *out_count = 0;
    return 0;
  }
  *out_count = valid;
  return 0;
}

static int parse_parameter_object(const JSON_Object *const p_obj,
                                  struct OpenAPI_Parameter *const out_param,
                                  const struct OpenAPI_Spec *const spec,
                                  const int resolve_refs) {
  const char *ref;
  const char *name;
  const char *in;
  const char *desc;
  int req;
  int deprecated_present;
  int deprecated_val;
  int allow_reserved_present;
  int allow_reserved_val;
  int allow_empty_present;
  int allow_empty_val;
  const JSON_Value *schema_val;
  const JSON_Object *schema;
  const JSON_Object *content;
  const JSON_Object *effective_schema;
  const JSON_Value *effective_schema_val;
  const JSON_Object *media_obj;
  const JSON_Value *media_schema_val;
  const char *media_type;
  const char *media_ref;
  const struct OpenAPI_SchemaRef *resolved_schema;
  struct OpenAPI_SchemaRef parsed_schema;
  int parsed_schema_set;
  const char *type;
  const char *style_str;
  int explode_present;
  int explode_val;

  if (!p_obj || !out_param)
    return 0;

  ref = json_object_get_string(p_obj, "$ref");
  if (ref) {
    out_param->ref = c_cdd_strdup(ref);
    if (!out_param->ref)
      return ENOMEM;
    if (resolve_refs && spec) {
      const struct OpenAPI_Parameter *comp =
          find_component_parameter(spec, ref);
      if (comp) {
        if (copy_parameter_fields(out_param, comp) != 0)
          return ENOMEM;
      }
    }
    desc = json_object_get_string(p_obj, "description");
    if (desc) {
      out_param->description = c_cdd_strdup(desc);
      if (!out_param->description)
        return ENOMEM;
    }
    return 0;
  }

  name = json_object_get_string(p_obj, "name");
  in = json_object_get_string(p_obj, "in");
  if (!name || !*name || !in)
    return EINVAL;
  desc = json_object_get_string(p_obj, "description");
  req = json_object_get_boolean(p_obj, "required");
  deprecated_present = json_object_has_value(p_obj, "deprecated");
  deprecated_val = json_object_get_boolean(p_obj, "deprecated");
  allow_reserved_present = json_object_has_value(p_obj, "allowReserved");
  allow_reserved_val = json_object_get_boolean(p_obj, "allowReserved");
  allow_empty_present = json_object_has_value(p_obj, "allowEmptyValue");
  allow_empty_val = json_object_get_boolean(p_obj, "allowEmptyValue");
  schema_val = json_object_get_value(p_obj, "schema");
  schema = schema_val ? json_value_get_object(schema_val) : NULL;
  content = json_object_get_object(p_obj, "content");
  if (content) {
    if (json_object_get_count(content) != 1)
      return EINVAL;
    if (parse_content_object(content, &out_param->content_media_types,
                             &out_param->n_content_media_types, spec,
                             resolve_refs) != 0)
      return ENOMEM;
  }
  effective_schema = schema;
  effective_schema_val = schema_val;
  media_obj = NULL;
  media_schema_val = NULL;
  media_type = NULL;
  media_ref = NULL;
  resolved_schema = NULL;
  type = NULL;
  parsed_schema_set = 0;
  style_str = json_object_get_string(p_obj, "style");
  explode_present = json_object_has_value(p_obj, "explode");
  explode_val = json_object_get_boolean(p_obj, "explode");

  out_param->name = c_cdd_strdup(name ? name : "");
  out_param->in = in ? parse_param_in(in) : OA_PARAM_IN_UNKNOWN;
  if (out_param->in == OA_PARAM_IN_UNKNOWN)
    return EINVAL;
  out_param->required = (req == 1);

  {
    const int has_schema = (schema_val != NULL);
    const int has_content = (content != NULL);
    if (has_schema && has_content)
      return EINVAL;
    if (!has_schema && !has_content)
      return EINVAL;
    if (out_param->in == OA_PARAM_IN_QUERYSTRING && !has_content)
      return EINVAL;
  }

  if (content) {
    if (out_param->in == OA_PARAM_IN_QUERYSTRING) {
      media_obj =
          json_object_get_object(content, "application/x-www-form-urlencoded");
      if (media_obj) {
        media_type = "application/x-www-form-urlencoded";
      }
    }
    if (!media_obj) {
      size_t ccount = json_object_get_count(content);
      if (ccount > 0) {
        media_type = json_object_get_name(content, 0);
        if (media_type) {
          media_obj = json_object_get_object(content, media_type);
        }
      }
    }
    if (media_type) {
      out_param->content_type = c_cdd_strdup(media_type);
      if (!out_param->content_type)
        return ENOMEM;
    }
    if (media_obj) {
      media_ref = json_object_get_string(media_obj, "$ref");
      if (media_ref) {
        out_param->content_ref = c_cdd_strdup(media_ref);
        if (!out_param->content_ref)
          return ENOMEM;
        if (resolve_refs && spec) {
          const struct OpenAPI_MediaType *mt =
              find_component_media_type(spec, media_ref);
          if (mt) {
            if (mt->schema_set)
              resolved_schema = &mt->schema;
            else if (mt->item_schema_set)
              resolved_schema = &mt->item_schema;
          }
        }
      } else {
        media_schema_val = json_object_get_value(media_obj, "schema");
        effective_schema_val = media_schema_val;
        effective_schema =
            media_schema_val ? json_value_get_object(media_schema_val) : NULL;
      }
    }
  }

  if (effective_schema)
    type = json_object_get_string(effective_schema, "type");

  if (desc) {
    out_param->description = c_cdd_strdup(desc);
    if (!out_param->description)
      return ENOMEM;
  }
  if (deprecated_present) {
    out_param->deprecated_set = 1;
    out_param->deprecated = (deprecated_val == 1);
  }
  if (allow_reserved_present) {
    out_param->allow_reserved_set = 1;
    out_param->allow_reserved = (allow_reserved_val == 1);
  }
  if (allow_empty_present) {
    out_param->allow_empty_value_set = 1;
    out_param->allow_empty_value = (allow_empty_val == 1);
  }
  if (resolved_schema) {
    if (out_param->in != OA_PARAM_IN_QUERYSTRING) {
      if (apply_schema_ref_to_param(out_param, resolved_schema) != 0)
        return ENOMEM;
    }
    if (copy_schema_ref(&out_param->schema, resolved_schema) != 0)
      return ENOMEM;
    out_param->schema_set = 1;
  } else if (effective_schema_val &&
             json_value_get_type(effective_schema_val) == JSONBoolean) {
    out_param->schema.schema_is_boolean = 1;
    out_param->schema.schema_boolean_value =
        json_value_get_boolean(effective_schema_val);
    out_param->schema_set = 1;
  } else if (effective_schema) {
    if (parse_schema_ref(effective_schema, &parsed_schema) != 0)
      return ENOMEM;
    parsed_schema_set = 1;
    if (out_param->in != OA_PARAM_IN_QUERYSTRING) {
      if (apply_schema_ref_to_param(out_param, &parsed_schema) != 0) {
        free_schema_ref_content(&parsed_schema);
        return ENOMEM;
      }
    }
    if (copy_schema_ref(&out_param->schema, &parsed_schema) != 0) {
      free_schema_ref_content(&parsed_schema);
      return ENOMEM;
    }
    out_param->schema_set = 1;
  }

  if (!out_param->type) {
    out_param->type = c_cdd_strdup(type ? type : "string");
    if (!out_param->type) {
      if (parsed_schema_set)
        free_schema_ref_content(&parsed_schema);
      return ENOMEM;
    }
  }

  if (style_str) {
    out_param->style = parse_param_style(style_str);
  } else {
    if (out_param->in == OA_PARAM_IN_QUERY)
      out_param->style = OA_STYLE_FORM;
    else if (out_param->in == OA_PARAM_IN_PATH)
      out_param->style = OA_STYLE_SIMPLE;
    else if (out_param->in == OA_PARAM_IN_COOKIE)
      out_param->style = OA_STYLE_FORM;
    else
      out_param->style = OA_STYLE_SIMPLE;
  }

  if (explode_present) {
    out_param->explode_set = 1;
    out_param->explode = explode_val;
  } else {
    if (out_param->style == OA_STYLE_FORM)
      out_param->explode = 1;
    else
      out_param->explode = 0;
  }

  if (parse_examples_object(json_object_get_object(p_obj, "examples"),
                            &out_param->examples, &out_param->n_examples, spec,
                            resolve_refs) != 0)
    return ENOMEM;
  if (out_param->n_examples == 0) {
    if (parse_any_field(p_obj, "example", &out_param->example,
                        &out_param->example_set) != 0)
      return ENOMEM;
  }
  if (out_param->example_set || out_param->n_examples > 0) {
    out_param->example_location = OA_EXAMPLE_LOC_OBJECT;
  } else if (media_obj && !media_ref) {
    if (parse_media_examples(media_obj, &out_param->example,
                             &out_param->example_set, &out_param->examples,
                             &out_param->n_examples, spec, resolve_refs) != 0)
      return ENOMEM;
    if (out_param->example_set || out_param->n_examples > 0) {
      out_param->example_location = OA_EXAMPLE_LOC_MEDIA;
    }
  }

  if (collect_extensions(p_obj, &out_param->extensions_json) != 0)
    return ENOMEM;

  if (parsed_schema_set)
    free_schema_ref_content(&parsed_schema);

  return 0;
}

static int parse_media_type_object(const JSON_Object *const media_obj,
                                   struct OpenAPI_MediaType *const out,
                                   const struct OpenAPI_Spec *const spec,
                                   const int resolve_refs) {
  const JSON_Value *schema_val;
  const JSON_Object *schema_obj;
  const JSON_Value *item_schema_val;
  const JSON_Object *item_schema_obj;
  const JSON_Object *encoding_obj;
  const JSON_Array *prefix_encoding_arr;
  const JSON_Object *item_encoding_obj;
  const char *ref;

  if (!media_obj || !out)
    return 0;

  {
    const int has_encoding = json_object_has_value(media_obj, "encoding");
    const int has_prefix = json_object_has_value(media_obj, "prefixEncoding");
    const int has_item = json_object_has_value(media_obj, "itemEncoding");
    if (has_encoding && (has_prefix || has_item))
      return EINVAL;
  }

  ref = json_object_get_string(media_obj, "$ref");
  if (ref) {
    out->ref = c_cdd_strdup(ref);
    if (!out->ref)
      return ENOMEM;
    if (resolve_refs && spec) {
      const struct OpenAPI_MediaType *mt = find_component_media_type(spec, ref);
      if (mt) {
        if (copy_media_type_fields(out, mt) != 0)
          return ENOMEM;
      }
    }
    return 0;
  }

  schema_val = json_object_get_value(media_obj, "schema");
  schema_obj = schema_val ? json_value_get_object(schema_val) : NULL;
  if (schema_val) {
    if (json_value_get_type(schema_val) == JSONBoolean) {
      out->schema.schema_is_boolean = 1;
      out->schema.schema_boolean_value = json_value_get_boolean(schema_val);
      out->schema_set = 1;
    } else if (schema_obj) {
      if (parse_schema_ref(schema_obj, &out->schema) != 0)
        return ENOMEM;
      out->schema_set = 1;
    }
  }

  item_schema_val = json_object_get_value(media_obj, "itemSchema");
  item_schema_obj =
      item_schema_val ? json_value_get_object(item_schema_val) : NULL;
  if (item_schema_val) {
    if (json_value_get_type(item_schema_val) == JSONBoolean) {
      out->item_schema.schema_is_boolean = 1;
      out->item_schema.schema_boolean_value =
          json_value_get_boolean(item_schema_val);
      out->item_schema_set = 1;
    } else if (item_schema_obj) {
      if (parse_schema_ref(item_schema_obj, &out->item_schema) != 0)
        return ENOMEM;
      out->item_schema_set = 1;
    }
  }

  encoding_obj = json_object_get_object(media_obj, "encoding");
  if (encoding_obj) {
    int rc = parse_encoding_map(encoding_obj, &out->encoding, &out->n_encoding,
                                spec, resolve_refs);
    if (rc != 0)
      return rc;
  }

  prefix_encoding_arr = json_object_get_array(media_obj, "prefixEncoding");
  if (prefix_encoding_arr) {
    int rc = parse_encoding_array(prefix_encoding_arr, &out->prefix_encoding,
                                  &out->n_prefix_encoding, spec, resolve_refs);
    if (rc != 0)
      return rc;
  }

  item_encoding_obj = json_object_get_object(media_obj, "itemEncoding");
  if (item_encoding_obj) {
    out->item_encoding =
        (struct OpenAPI_Encoding *)calloc(1, sizeof(struct OpenAPI_Encoding));
    if (!out->item_encoding)
      return ENOMEM;
    out->item_encoding_set = 1;
    {
      int rc = parse_encoding_object(item_encoding_obj, out->item_encoding,
                                     spec, resolve_refs);
      if (rc != 0)
        return rc;
    }
  }

  if (parse_media_examples(media_obj, &out->example, &out->example_set,
                           &out->examples, &out->n_examples, spec,
                           resolve_refs) != 0)
    return ENOMEM;

  if (collect_extensions(media_obj, &out->extensions_json) != 0)
    return ENOMEM;

  return 0;
}

static size_t media_type_base_len(const char *name) {
  size_t len = 0;
  if (!name)
    return 0;
  while (name[len] && name[len] != ';')
    ++len;
  return len;
}

static int media_type_base_equal(const char *a, const char *b) {
  size_t alen;
  size_t blen;
  if (!a || !b)
    return 0;
  alen = media_type_base_len(a);
  blen = media_type_base_len(b);
  if (alen != blen)
    return 0;
  return strncmp(a, b, alen) == 0;
}

static const struct OpenAPI_MediaType *
find_media_type_by_name(const struct OpenAPI_MediaType *mts, size_t n,
                        const char *name) {
  size_t i;
  if (!mts || !name)
    return NULL;
  for (i = 0; i < n; ++i) {
    if (mts[i].name && media_type_base_equal(mts[i].name, name))
      return &mts[i];
  }
  return NULL;
}

static int media_type_is_json(const char *name) {
  size_t len;
  if (!name)
    return 0;
  len = media_type_base_len(name);
  if (len == strlen("application/json") &&
      strncmp(name, "application/json", len) == 0)
    return 1;
  if (len >= 5 && strncmp(name + (len - 5), "+json", 5) == 0)
    return 1;
  return 0;
}

static int media_type_specificity(const char *name) {
  const char *slash;
  size_t len;
  size_t type_len;
  size_t sub_len;
  if (!name)
    return -1;
  len = media_type_base_len(name);
  if (len == 0)
    return -1;
  slash = (const char *)memchr(name, '/', len);
  if (!slash)
    return 2; /* Treat unknown as exact */
  type_len = (size_t)(slash - name);
  if (type_len == 0 || type_len >= len)
    return -1;
  sub_len = len - type_len - 1;
  if (type_len == 1 && name[0] == '*' && sub_len == 1 && slash[1] == '*')
    return 0;                                       /* */
  */ if (sub_len == 1 && slash[1] == '*') return 1; /* type/* */
  return 2;                                         /* type/subtype */
}

static int media_type_preference_rank(const char *name) {
  if (!name)
    return 0;
  if (media_type_is_json(name))
    return 3;
  if (media_type_base_equal(name, "application/x-www-form-urlencoded"))
    return 2;
  if (media_type_base_equal(name, "multipart/form-data"))
    return 1;
  return 0;
}

static const struct OpenAPI_MediaType *
select_primary_media_type(const struct OpenAPI_MediaType *mts, size_t n) {
  size_t i;
  const struct OpenAPI_MediaType *best = NULL;
  int best_spec = -1;
  int best_rank = -1;

  for (i = 0; i < n; ++i) {
    int spec = media_type_specificity(mts[i].name);
    int rank = media_type_preference_rank(mts[i].name);
    if (spec > best_spec) {
      best_spec = spec;
      best_rank = rank;
      best = &mts[i];
      continue;
    }
    if (spec == best_spec && rank > best_rank) {
      best_rank = rank;
      best = &mts[i];
    }
  }

  return best;
}

static int parse_content_object(const JSON_Object *const content,
                                struct OpenAPI_MediaType **out,
                                size_t *out_count,
                                const struct OpenAPI_Spec *const spec,
                                const int resolve_refs) {
  size_t i, count, valid = 0;
  if (!out || !out_count)
    return 0;
  *out = NULL;
  *out_count = 0;
  if (!content)
    return 0;
  count = json_object_get_count(content);
  if (count == 0)
    return 0;

  *out = (struct OpenAPI_MediaType *)calloc(count,
                                            sizeof(struct OpenAPI_MediaType));
  if (!*out)
    return ENOMEM;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(content, i);
    const JSON_Object *media_obj =
        json_value_get_object(json_object_get_value_at(content, i));
    struct OpenAPI_MediaType *curr = &(*out)[valid];
    if (!name || !media_obj)
      continue;
    curr->name = c_cdd_strdup(name);
    if (!curr->name)
      return ENOMEM;
    {
      int rc = parse_media_type_object(media_obj, curr, spec, resolve_refs);
      if (rc != 0)
        return rc;
    }
    valid++;
  }

  if (valid == 0) {
    free(*out);
    *out = NULL;
    *out_count = 0;
    return 0;
  }
  *out_count = valid;
  return 0;
}

static int param_key_equals(const struct OpenAPI_Parameter *const a,
                            const struct OpenAPI_Parameter *const b) {
  if (!a || !b || !a->name || !b->name)
    return 0;
  return (a->in == b->in) && (strcmp(a->name, b->name) == 0);
}

static int parse_parameters_array(const JSON_Array *const arr,
                                  struct OpenAPI_Parameter **out_params,
                                  size_t *out_count,
                                  const struct OpenAPI_Spec *const spec) {
  size_t i, count, valid = 0;
  if (!out_params || !out_count)
    return 0;

  *out_params = NULL;
  *out_count = 0;

  if (!arr)
    return 0;

  count = json_array_get_count(arr);
  if (count == 0)
    return 0;

  *out_params = (struct OpenAPI_Parameter *)calloc(
      count, sizeof(struct OpenAPI_Parameter));
  if (!*out_params)
    return ENOMEM;

  for (i = 0; i < count; ++i) {
    const JSON_Object *p_obj = json_array_get_object(arr, i);
    if (p_obj) {
      struct OpenAPI_Parameter tmp = {0};
      int rc = parse_parameter_object(p_obj, &tmp, spec, 1);
      if (rc != 0) {
        free_parameter(&tmp);
        return rc;
      }
      if (header_param_is_reserved(&tmp)) {
        free_parameter(&tmp);
        continue;
      }
      if (tmp.name && *tmp.name && tmp.in != OA_PARAM_IN_UNKNOWN) {
        size_t k;
        for (k = 0; k < valid; ++k) {
          if (param_key_equals(&tmp, &(*out_params)[k])) {
            free_parameter(&tmp);
            return EINVAL;
          }
        }
      }
      (*out_params)[valid++] = tmp;
    }
  }
  if (valid == 0) {
    free(*out_params);
    *out_params = NULL;
    *out_count = 0;
    return 0;
  }
  if (valid < count) {
    struct OpenAPI_Parameter *tmp = (struct OpenAPI_Parameter *)realloc(
        *out_params, valid * sizeof(struct OpenAPI_Parameter));
    if (tmp)
      *out_params = tmp;
  }
  *out_count = valid;
  return 0;
}

static int parse_request_body_object(const JSON_Object *const rb_obj,
                                     struct OpenAPI_RequestBody *const out_rb,
                                     const struct OpenAPI_Spec *const spec,
                                     const int resolve_refs) {
  const char *ref;
  const char *desc;
  int required_present;
  int required_val;
  const JSON_Object *content;
  const struct OpenAPI_MediaType *primary = NULL;

  if (!rb_obj || !out_rb)
    return 0;

  ref = json_object_get_string(rb_obj, "$ref");
  if (ref) {
    out_rb->ref = c_cdd_strdup(ref);
    if (!out_rb->ref)
      return ENOMEM;
    if (resolve_refs && spec) {
      const struct OpenAPI_RequestBody *comp =
          find_component_request_body(spec, ref);
      if (comp) {
        if (copy_request_body_fields(out_rb, comp) != 0)
          return ENOMEM;
      }
    }
    desc = json_object_get_string(rb_obj, "description");
    if (desc) {
      out_rb->description = c_cdd_strdup(desc);
      if (!out_rb->description)
        return ENOMEM;
    }
    return 0;
  }

  desc = json_object_get_string(rb_obj, "description");
  if (desc) {
    out_rb->description = c_cdd_strdup(desc);
    if (!out_rb->description)
      return ENOMEM;
  }

  required_present = json_object_has_value(rb_obj, "required");
  required_val = json_object_get_boolean(rb_obj, "required");
  if (required_present) {
    out_rb->required_set = 1;
    out_rb->required = (required_val == 1);
  }

  content = json_object_get_object(rb_obj, "content");
  if (content) {
    if (parse_content_object(content, &out_rb->content_media_types,
                             &out_rb->n_content_media_types, spec,
                             resolve_refs) != 0)
      return ENOMEM;
    primary = select_primary_media_type(out_rb->content_media_types,
                                        out_rb->n_content_media_types);
    if (primary) {
      if (primary->ref) {
        out_rb->content_ref = c_cdd_strdup(primary->ref);
        if (!out_rb->content_ref)
          return ENOMEM;
      }
      if (primary->schema_set) {
        if (copy_schema_ref(&out_rb->schema, &primary->schema) != 0)
          return ENOMEM;
      } else if (primary->item_schema_set) {
        if (copy_schema_ref(&out_rb->schema, &primary->item_schema) != 0)
          return ENOMEM;
      }
      if (primary->examples && primary->n_examples > 0) {
        out_rb->examples = (struct OpenAPI_Example *)calloc(
            primary->n_examples, sizeof(struct OpenAPI_Example));
        if (!out_rb->examples)
          return ENOMEM;
        out_rb->n_examples = primary->n_examples;
        {
          size_t i;
          for (i = 0; i < primary->n_examples; ++i) {
            if (copy_example_fields(&out_rb->examples[i],
                                    &primary->examples[i]) != 0)
              return ENOMEM;
          }
        }
      } else if (primary->example_set) {
        if (copy_any_value(&out_rb->example, &primary->example) != 0)
          return ENOMEM;
        out_rb->example_set = 1;
      }
      if (primary->name) {
        out_rb->schema.content_type = c_cdd_strdup(primary->name);
        if (!out_rb->schema.content_type)
          return ENOMEM;
      }
    }
  }

  if (collect_extensions(rb_obj, &out_rb->extensions_json) != 0)
    return ENOMEM;

  return 0;
}

static int copy_request_body_fields(struct OpenAPI_RequestBody *dst,
                                    const struct OpenAPI_RequestBody *src) {
  if (!dst || !src)
    return 0;
  if (src->ref && !dst->ref) {
    dst->ref = c_cdd_strdup(src->ref);
    if (!dst->ref)
      return ENOMEM;
  }
  if (src->description) {
    dst->description = c_cdd_strdup(src->description);
    if (!dst->description)
      return ENOMEM;
  }
  if (src->content_ref) {
    dst->content_ref = c_cdd_strdup(src->content_ref);
    if (!dst->content_ref)
      return ENOMEM;
  }
  if (src->extensions_json) {
    dst->extensions_json = c_cdd_strdup(src->extensions_json);
    if (!dst->extensions_json)
      return ENOMEM;
  }
  if (src->content_media_types && src->n_content_media_types > 0) {
    if (copy_media_type_array(
            &dst->content_media_types, &dst->n_content_media_types,
            src->content_media_types, src->n_content_media_types) != 0)
      return ENOMEM;
  }
  if (src->example_set) {
    if (copy_any_value(&dst->example, &src->example) != 0)
      return ENOMEM;
    dst->example_set = 1;
  }
  if (src->examples && src->n_examples > 0) {
    size_t i;
    dst->examples = (struct OpenAPI_Example *)calloc(
        src->n_examples, sizeof(struct OpenAPI_Example));
    if (!dst->examples)
      return ENOMEM;
    dst->n_examples = src->n_examples;
    for (i = 0; i < src->n_examples; ++i) {
      if (copy_example_fields(&dst->examples[i], &src->examples[i]) != 0)
        return ENOMEM;
    }
  }
  dst->required = src->required;
  dst->required_set = src->required_set;
  if (copy_schema_ref(&dst->schema, &src->schema) != 0)
    return ENOMEM;
  return 0;
}

static int parse_response_object(const JSON_Object *const resp_obj,
                                 struct OpenAPI_Response *const out_resp,
                                 const struct OpenAPI_Spec *const spec,
                                 const int resolve_refs) {
  const char *ref;
  const char *summary;
  const char *desc;
  const JSON_Object *content;
  const JSON_Object *headers;
  const JSON_Object *links;
  if (!resp_obj || !out_resp)
    return 0;

  ref = json_object_get_string(resp_obj, "$ref");
  if (ref) {
    out_resp->ref = c_cdd_strdup(ref);
    if (!out_resp->ref)
      return ENOMEM;
    if (resolve_refs && spec) {
      const struct OpenAPI_Response *comp = find_component_response(spec, ref);
      if (comp) {
        if (copy_response_fields(out_resp, comp) != 0)
          return ENOMEM;
      }
    }
  }

  summary = json_object_get_string(resp_obj, "summary");
  if (summary) {
    out_resp->summary = c_cdd_strdup(summary);
    if (!out_resp->summary)
      return ENOMEM;
  }

  desc = json_object_get_string(resp_obj, "description");
  if (desc) {
    out_resp->description = c_cdd_strdup(desc);
    if (!out_resp->description)
      return ENOMEM;
  }

  if (!ref) {
    if (collect_extensions(resp_obj, &out_resp->extensions_json) != 0)
      return ENOMEM;
  }

  if (ref)
    return 0;

  headers = json_object_get_object(resp_obj, "headers");
  if (headers) {
    int rc = parse_headers_object(headers, &out_resp->headers,
                                  &out_resp->n_headers, spec, resolve_refs, 1);
    if (rc != 0)
      return rc;
  }

  links = json_object_get_object(resp_obj, "links");
  if (links) {
    if (parse_links_object(links, &out_resp->links, &out_resp->n_links, spec,
                           resolve_refs) != 0)
      return ENOMEM;
  }

  content = json_object_get_object(resp_obj, "content");
  if (content) {
    const struct OpenAPI_MediaType *primary = NULL;
    if (parse_content_object(content, &out_resp->content_media_types,
                             &out_resp->n_content_media_types, spec,
                             resolve_refs) != 0)
      return ENOMEM;
    primary = select_primary_media_type(out_resp->content_media_types,
                                        out_resp->n_content_media_types);
    if (primary) {
      if (primary->name) {
        out_resp->content_type = c_cdd_strdup(primary->name);
        if (!out_resp->content_type)
          return ENOMEM;
      }
      if (primary->ref) {
        out_resp->content_ref = c_cdd_strdup(primary->ref);
        if (!out_resp->content_ref)
          return ENOMEM;
      }
      if (primary->schema_set) {
        if (copy_schema_ref(&out_resp->schema, &primary->schema) != 0)
          return ENOMEM;
      } else if (primary->item_schema_set) {
        if (copy_schema_ref(&out_resp->schema, &primary->item_schema) != 0)
          return ENOMEM;
      }
      if (primary->examples && primary->n_examples > 0) {
        out_resp->examples = (struct OpenAPI_Example *)calloc(
            primary->n_examples, sizeof(struct OpenAPI_Example));
        if (!out_resp->examples)
          return ENOMEM;
        out_resp->n_examples = primary->n_examples;
        {
          size_t i;
          for (i = 0; i < primary->n_examples; ++i) {
            if (copy_example_fields(&out_resp->examples[i],
                                    &primary->examples[i]) != 0)
              return ENOMEM;
          }
        }
      } else if (primary->example_set) {
        if (copy_any_value(&out_resp->example, &primary->example) != 0)
          return ENOMEM;
        out_resp->example_set = 1;
      }
    }
  }

  return 0;
}

static int parse_responses(const JSON_Object *responses,
                           struct OpenAPI_Operation *out_op,
                           const struct OpenAPI_Spec *spec) {
  size_t i, count;
  if (!responses || !out_op)
    return 0;

  count = json_object_get_count(responses);
  if (count == 0)
    return 0;

  out_op->responses =
      (struct OpenAPI_Response *)calloc(count, sizeof(struct OpenAPI_Response));
  if (!out_op->responses)
    return ENOMEM;
  out_op->n_responses = count;

  for (i = 0; i < count; ++i) {
    const char *code = json_object_get_name(responses, i);
    const JSON_Value *val = json_object_get_value_at(responses, i);
    const JSON_Object *resp_obj = json_value_get_object(val);
    struct OpenAPI_Response *curr = &out_op->responses[i];

    curr->code = c_cdd_strdup(code);
    if (!curr->code)
      return ENOMEM;

    if (resp_obj) {
      if (parse_response_object(resp_obj, curr, spec, 1) != 0)
        return ENOMEM;
    }
  }
  return 0;
}

static int parse_callback_object(const JSON_Object *const cb_obj,
                                 struct OpenAPI_Callback *const out_cb,
                                 const struct OpenAPI_Spec *const spec,
                                 const int resolve_refs) {
  const char *ref;
  const char *summary;
  const char *desc;

  if (!cb_obj || !out_cb)
    return 0;

  ref = json_object_get_string(cb_obj, "$ref");
  if (ref) {
    out_cb->ref = c_cdd_strdup(ref);
    if (!out_cb->ref)
      return ENOMEM;
    if (resolve_refs && spec) {
      const struct OpenAPI_Callback *comp = find_component_callback(spec, ref);
      if (comp) {
        /* Callback refs are preserved; inline paths are not resolved here. */
        (void)comp;
      }
    }
    summary = json_object_get_string(cb_obj, "summary");
    if (summary) {
      out_cb->summary = c_cdd_strdup(summary);
      if (!out_cb->summary)
        return ENOMEM;
    }
    desc = json_object_get_string(cb_obj, "description");
    if (desc) {
      out_cb->description = c_cdd_strdup(desc);
      if (!out_cb->description)
        return ENOMEM;
    }
    return 0;
  }

  if (collect_extensions(cb_obj, &out_cb->extensions_json) != 0)
    return ENOMEM;

  return parse_paths_object(cb_obj, &out_cb->paths, &out_cb->n_paths, spec, 0);
}

static int parse_callbacks_object(const JSON_Object *const callbacks,
                                  struct OpenAPI_Callback **out_callbacks,
                                  size_t *out_count,
                                  const struct OpenAPI_Spec *const spec,
                                  const int resolve_refs) {
  size_t i, count;
  if (!out_callbacks || !out_count)
    return 0;

  *out_callbacks = NULL;
  *out_count = 0;

  if (!callbacks)
    return 0;

  count = json_object_get_count(callbacks);
  if (count == 0)
    return 0;

  *out_callbacks =
      (struct OpenAPI_Callback *)calloc(count, sizeof(struct OpenAPI_Callback));
  if (!*out_callbacks)
    return ENOMEM;
  *out_count = count;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(callbacks, i);
    const JSON_Object *cb_obj =
        json_value_get_object(json_object_get_value_at(callbacks, i));
    struct OpenAPI_Callback *curr = &(*out_callbacks)[i];
    if (name) {
      curr->name = c_cdd_strdup(name);
      if (!curr->name)
        return ENOMEM;
    }
    if (cb_obj) {
      if (parse_callback_object(cb_obj, curr, spec, resolve_refs) != 0)
        return ENOMEM;
    }
  }

  return 0;
}

static int parse_operation(const char *const verb_str,
                           const JSON_Object *const op_obj,
                           struct OpenAPI_Operation *const out_op,
                           const struct OpenAPI_Spec *const spec,
                           const int is_additional) {
  const char *op_id;
  const JSON_Array *params;
  const JSON_Array *tags;
  const JSON_Object *req_body, *responses;
  const char *summary;
  const char *description;
  const JSON_Object *ext_docs;
  int deprecated_present;
  int deprecated_val;

  if (!verb_str || !op_obj || !out_op)
    return EINVAL;

  out_op->verb = parse_verb(verb_str);
  out_op->is_additional = is_additional;
  if (verb_str) {
    out_op->method = c_cdd_strdup(verb_str);
    if (!out_op->method)
      return ENOMEM;
  }
  if (out_op->verb == OA_VERB_UNKNOWN && !is_additional)
    return 0;

  op_id = json_object_get_string(op_obj, "operationId");
  out_op->operation_id = c_cdd_strdup(op_id ? op_id : "unnamed");
  summary = json_object_get_string(op_obj, "summary");
  if (summary) {
    out_op->summary = c_cdd_strdup(summary);
    if (!out_op->summary)
      return ENOMEM;
  }
  description = json_object_get_string(op_obj, "description");
  if (description) {
    out_op->description = c_cdd_strdup(description);
    if (!out_op->description)
      return ENOMEM;
  }
  ext_docs = json_object_get_object(op_obj, "externalDocs");
  if (ext_docs) {
    if (parse_external_docs(ext_docs, &out_op->external_docs) != 0)
      return ENOMEM;
  }
  deprecated_present = json_object_has_value(op_obj, "deprecated");
  deprecated_val = json_object_get_boolean(op_obj, "deprecated");
  if (deprecated_present)
    out_op->deprecated = (deprecated_val == 1);
  if (parse_security_field(op_obj, "security", &out_op->security,
                           &out_op->n_security, &out_op->security_set) != 0)
    return ENOMEM;
  if (collect_extensions(op_obj, &out_op->extensions_json) != 0)
    return ENOMEM;

  /* 1. Parameters */
  params = json_object_get_array(op_obj, "parameters");
  {
    int rc = parse_parameters_array(params, &out_op->parameters,
                                    &out_op->n_parameters, spec);
    if (rc != 0)
      return rc;
  }

  /* 2. Request Body */
  req_body = json_object_get_object(op_obj, "requestBody");
  if (req_body) {
    struct OpenAPI_RequestBody rb;
    memset(&rb, 0, sizeof(rb));
    if (parse_request_body_object(req_body, &rb, spec, 1) != 0)
      return ENOMEM;
    if (rb.ref) {
      out_op->req_body_ref = c_cdd_strdup(rb.ref);
      if (!out_op->req_body_ref) {
        free_request_body(&rb);
        return ENOMEM;
      }
    }
    if (rb.description) {
      out_op->req_body_description = c_cdd_strdup(rb.description);
      if (!out_op->req_body_description) {
        free_request_body(&rb);
        return ENOMEM;
      }
    }
    if (rb.required_set) {
      out_op->req_body_required_set = 1;
      out_op->req_body_required = rb.required;
    }
    if (copy_schema_ref(&out_op->req_body, &rb.schema) != 0) {
      free_request_body(&rb);
      return ENOMEM;
    }
    if (rb.content_media_types && rb.n_content_media_types > 0) {
      if (copy_media_type_array(
              &out_op->req_body_media_types, &out_op->n_req_body_media_types,
              rb.content_media_types, rb.n_content_media_types) != 0) {
        free_request_body(&rb);
        return ENOMEM;
      }
    }
    free_request_body(&rb);
  }

  /* 3. Responses */
  responses = json_object_get_object(op_obj, "responses");
  if (parse_responses(responses, out_op, spec) != 0)
    return ENOMEM;

  /* 4. Callbacks */
  {
    const JSON_Object *callbacks_obj =
        json_object_get_object(op_obj, "callbacks");
    if (parse_callbacks_object(callbacks_obj, &out_op->callbacks,
                               &out_op->n_callbacks, spec, 1) != 0)
      return ENOMEM;
  }

  /* 5. Tags */
  tags = json_object_get_array(op_obj, "tags");
  if (tags) {
    size_t t_count = json_array_get_count(tags);
    out_op->n_tags = t_count;
    if (t_count > 0) {
      size_t k;
      out_op->tags = (char **)calloc(t_count, sizeof(char *));
      if (!out_op->tags)
        return ENOMEM;
      for (k = 0; k < t_count; ++k) {
        const char *t_val = json_array_get_string(tags, k);
        out_op->tags[k] = c_cdd_strdup(t_val ? t_val : "");
        if (!out_op->tags[k])
          return ENOMEM;
      }
    }
  }

  if (parse_servers_array(op_obj, "servers", &out_op->servers,
                          &out_op->n_servers) != 0)
    return ENOMEM;

  return 0;
}

static int parse_component_parameters(const JSON_Object *const components,
                                      struct OpenAPI_Spec *const out) {
  const JSON_Object *params;
  size_t count, i;

  if (!components || !out)
    return 0;

  params = json_object_get_object(components, "parameters");
  if (!params)
    return 0;

  if (validate_component_key_map(params) != 0)
    return EINVAL;

  count = json_object_get_count(params);
  if (count == 0)
    return 0;

  out->component_parameters = (struct OpenAPI_Parameter *)calloc(
      count, sizeof(struct OpenAPI_Parameter));
  out->component_parameter_names = (char **)calloc(count, sizeof(char *));
  if (!out->component_parameters || !out->component_parameter_names)
    return ENOMEM;
  out->n_component_parameters = count;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(params, i);
    const JSON_Object *p_obj =
        json_value_get_object(json_object_get_value_at(params, i));
    if (name) {
      if (!component_key_is_valid(name))
        return EINVAL;
      out->component_parameter_names[i] = c_cdd_strdup(name);
      if (!out->component_parameter_names[i])
        return ENOMEM;
    }
    if (p_obj) {
      if (parse_parameter_object(p_obj, &out->component_parameters[i], NULL,
                                 0) != 0)
        return ENOMEM;
    }
  }

  return 0;
}

static int parse_component_responses(const JSON_Object *const components,
                                     struct OpenAPI_Spec *const out) {
  const JSON_Object *responses;
  size_t count, i;

  if (!components || !out)
    return 0;

  responses = json_object_get_object(components, "responses");
  if (!responses)
    return 0;

  if (validate_component_key_map(responses) != 0)
    return EINVAL;

  count = json_object_get_count(responses);
  if (count == 0)
    return 0;

  out->component_responses =
      (struct OpenAPI_Response *)calloc(count, sizeof(struct OpenAPI_Response));
  out->component_response_names = (char **)calloc(count, sizeof(char *));
  if (!out->component_responses || !out->component_response_names)
    return ENOMEM;
  out->n_component_responses = count;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(responses, i);
    const JSON_Object *r_obj =
        json_value_get_object(json_object_get_value_at(responses, i));
    if (name) {
      if (!component_key_is_valid(name))
        return EINVAL;
      out->component_response_names[i] = c_cdd_strdup(name);
      if (!out->component_response_names[i])
        return ENOMEM;
    }
    if (r_obj) {
      if (parse_response_object(r_obj, &out->component_responses[i], NULL, 0) !=
          0)
        return ENOMEM;
    }
  }

  return 0;
}

static int parse_component_headers(const JSON_Object *const components,
                                   struct OpenAPI_Spec *const out) {
  const JSON_Object *headers;
  size_t count, i;

  if (!components || !out)
    return 0;

  headers = json_object_get_object(components, "headers");
  if (!headers)
    return 0;

  if (validate_component_key_map(headers) != 0)
    return EINVAL;

  count = json_object_get_count(headers);
  if (count == 0)
    return 0;

  out->component_headers =
      (struct OpenAPI_Header *)calloc(count, sizeof(struct OpenAPI_Header));
  out->component_header_names = (char **)calloc(count, sizeof(char *));
  if (!out->component_headers || !out->component_header_names)
    return ENOMEM;
  out->n_component_headers = count;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(headers, i);
    const JSON_Object *h_obj =
        json_value_get_object(json_object_get_value_at(headers, i));
    if (name) {
      if (!component_key_is_valid(name))
        return EINVAL;
      out->component_header_names[i] = c_cdd_strdup(name);
      if (!out->component_header_names[i])
        return ENOMEM;
    }
    if (h_obj) {
      if (parse_header_object(h_obj, &out->component_headers[i], NULL, 0) != 0)
        return ENOMEM;
    }
  }

  return 0;
}

static int parse_component_request_bodies(const JSON_Object *const components,
                                          struct OpenAPI_Spec *const out) {
  const JSON_Object *bodies;
  size_t count, i;

  if (!components || !out)
    return 0;

  bodies = json_object_get_object(components, "requestBodies");
  if (!bodies)
    return 0;

  if (validate_component_key_map(bodies) != 0)
    return EINVAL;

  count = json_object_get_count(bodies);
  if (count == 0)
    return 0;

  out->component_request_bodies = (struct OpenAPI_RequestBody *)calloc(
      count, sizeof(struct OpenAPI_RequestBody));
  out->component_request_body_names = (char **)calloc(count, sizeof(char *));
  if (!out->component_request_bodies || !out->component_request_body_names)
    return ENOMEM;
  out->n_component_request_bodies = count;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(bodies, i);
    const JSON_Object *rb_obj =
        json_value_get_object(json_object_get_value_at(bodies, i));
    if (name) {
      if (!component_key_is_valid(name))
        return EINVAL;
      out->component_request_body_names[i] = c_cdd_strdup(name);
      if (!out->component_request_body_names[i])
        return ENOMEM;
    }
    if (rb_obj) {
      if (parse_request_body_object(rb_obj, &out->component_request_bodies[i],
                                    NULL, 0) != 0)
        return ENOMEM;
    }
  }

  return 0;
}

static int parse_component_media_types(const JSON_Object *const components,
                                       struct OpenAPI_Spec *const out) {
  const JSON_Object *media_types;
  size_t count, i;

  if (!components || !out)
    return 0;

  media_types = json_object_get_object(components, "mediaTypes");
  if (!media_types)
    return 0;

  if (validate_component_key_map(media_types) != 0)
    return EINVAL;

  count = json_object_get_count(media_types);
  if (count == 0)
    return 0;

  out->component_media_types = (struct OpenAPI_MediaType *)calloc(
      count, sizeof(struct OpenAPI_MediaType));
  out->component_media_type_names = (char **)calloc(count, sizeof(char *));
  if (!out->component_media_types || !out->component_media_type_names)
    return ENOMEM;
  out->n_component_media_types = count;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(media_types, i);
    const JSON_Object *mt_obj =
        json_value_get_object(json_object_get_value_at(media_types, i));
    struct OpenAPI_MediaType *curr = &out->component_media_types[i];

    if (name) {
      if (!component_key_is_valid(name))
        return EINVAL;
      out->component_media_type_names[i] = c_cdd_strdup(name);
      if (!out->component_media_type_names[i])
        return ENOMEM;
      curr->name = c_cdd_strdup(name);
      if (!curr->name)
        return ENOMEM;
    }
    if (mt_obj) {
      if (parse_media_type_object(mt_obj, curr, out, 0) != 0)
        return ENOMEM;
    }
  }

  return 0;
}

static int parse_component_examples(const JSON_Object *const components,
                                    struct OpenAPI_Spec *const out) {
  const JSON_Object *examples;
  size_t count, i;

  if (!components || !out)
    return 0;

  examples = json_object_get_object(components, "examples");
  if (!examples)
    return 0;

  if (validate_component_key_map(examples) != 0)
    return EINVAL;

  count = json_object_get_count(examples);
  if (count == 0)
    return 0;

  out->component_examples =
      (struct OpenAPI_Example *)calloc(count, sizeof(struct OpenAPI_Example));
  out->component_example_names = (char **)calloc(count, sizeof(char *));
  if (!out->component_examples || !out->component_example_names)
    return ENOMEM;
  out->n_component_examples = count;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(examples, i);
    const JSON_Object *ex_obj =
        json_value_get_object(json_object_get_value_at(examples, i));
    if (name) {
      if (!component_key_is_valid(name))
        return EINVAL;
      out->component_example_names[i] = c_cdd_strdup(name);
      if (!out->component_example_names[i])
        return ENOMEM;
    }
    if (ex_obj) {
      if (parse_example_object(ex_obj, name, &out->component_examples[i], out,
                               0) != 0)
        return ENOMEM;
    }
  }

  return 0;
}

static int parse_component_links(const JSON_Object *const components,
                                 struct OpenAPI_Spec *const out) {
  const JSON_Object *links;
  size_t count, i;

  if (!components || !out)
    return 0;

  links = json_object_get_object(components, "links");
  if (!links)
    return 0;

  if (validate_component_key_map(links) != 0)
    return EINVAL;

  count = json_object_get_count(links);
  if (count == 0)
    return 0;

  out->component_links =
      (struct OpenAPI_Link *)calloc(count, sizeof(struct OpenAPI_Link));
  if (!out->component_links)
    return ENOMEM;
  out->n_component_links = count;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(links, i);
    const JSON_Object *link_obj =
        json_value_get_object(json_object_get_value_at(links, i));
    struct OpenAPI_Link *curr = &out->component_links[i];
    if (name) {
      if (!component_key_is_valid(name))
        return EINVAL;
      curr->name = c_cdd_strdup(name);
      if (!curr->name)
        return ENOMEM;
    }
    if (link_obj) {
      if (parse_link_object(link_obj, curr, out, 0) != 0)
        return ENOMEM;
    }
  }

  return 0;
}

static int parse_component_callbacks(const JSON_Object *const components,
                                     struct OpenAPI_Spec *const out) {
  const JSON_Object *callbacks;
  size_t count, i;

  if (!components || !out)
    return 0;

  callbacks = json_object_get_object(components, "callbacks");
  if (!callbacks)
    return 0;

  if (validate_component_key_map(callbacks) != 0)
    return EINVAL;

  count = json_object_get_count(callbacks);
  if (count == 0)
    return 0;

  out->component_callbacks =
      (struct OpenAPI_Callback *)calloc(count, sizeof(struct OpenAPI_Callback));
  if (!out->component_callbacks)
    return ENOMEM;
  out->n_component_callbacks = count;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(callbacks, i);
    const JSON_Object *cb_obj =
        json_value_get_object(json_object_get_value_at(callbacks, i));
    struct OpenAPI_Callback *curr = &out->component_callbacks[i];
    if (name) {
      if (!component_key_is_valid(name))
        return EINVAL;
      curr->name = c_cdd_strdup(name);
      if (!curr->name)
        return ENOMEM;
    }
    if (cb_obj) {
      if (parse_callback_object(cb_obj, curr, out, 0) != 0)
        return ENOMEM;
    }
  }

  return 0;
}

static int parse_component_path_items(const JSON_Object *const components,
                                      struct OpenAPI_Spec *const out) {
  const JSON_Object *path_items;
  size_t i;
  int rc;

  if (!components || !out)
    return 0;

  path_items = json_object_get_object(components, "pathItems");
  if (!path_items)
    return 0;

  if (validate_component_key_map(path_items) != 0)
    return EINVAL;

  rc = parse_paths_object(path_items, &out->component_path_items,
                          &out->n_component_path_items, out, 0);
  if (rc != 0)
    return rc;

  if (out->n_component_path_items == 0)
    return 0;

  out->component_path_item_names =
      (char **)calloc(out->n_component_path_items, sizeof(char *));
  if (!out->component_path_item_names)
    return ENOMEM;

  for (i = 0; i < out->n_component_path_items; ++i) {
    const char *name = out->component_path_items[i].route;
    if (name) {
      out->component_path_item_names[i] = c_cdd_strdup(name);
      if (!out->component_path_item_names[i])
        return ENOMEM;
    }
  }

  return 0;
}

static int parse_components(const JSON_Object *components,
                            struct OpenAPI_Spec *out) {
  const JSON_Object *schemas;
  size_t i, count;

  if (!components || !out)
    return 0;

  {
    int rc = parse_security_schemes(components, out);
    if (rc != 0)
      return rc;
    rc = parse_component_parameters(components, out);
    if (rc != 0)
      return rc;
    rc = parse_component_responses(components, out);
    if (rc != 0)
      return rc;
    rc = parse_component_headers(components, out);
    if (rc != 0)
      return rc;
    rc = parse_component_request_bodies(components, out);
    if (rc != 0)
      return rc;
    rc = parse_component_media_types(components, out);
    if (rc != 0)
      return rc;
    rc = parse_component_examples(components, out);
    if (rc != 0)
      return rc;
    rc = parse_component_links(components, out);
    if (rc != 0)
      return rc;
    rc = parse_component_callbacks(components, out);
    if (rc != 0)
      return rc;
    rc = parse_component_path_items(components, out);
    if (rc != 0)
      return rc;
  }

  schemas = json_object_get_object(components, "schemas");
  if (!schemas)
    return 0;

  if (validate_component_key_map(schemas) != 0)
    return EINVAL;

  count = json_object_get_count(schemas);
  if (count == 0)
    return 0;

  {
    size_t struct_count = 0;
    size_t raw_count = 0;
    for (i = 0; i < count; ++i) {
      const JSON_Value *schema_val = json_object_get_value_at(schemas, i);
      const JSON_Object *schema_obj = json_value_get_object(schema_val);
      if (schema_is_struct_compatible(schema_val, schema_obj))
        struct_count++;
      else
        raw_count++;
    }

    if (struct_count > 0) {
      out->defined_schemas = (struct StructFields *)calloc(
          struct_count, sizeof(struct StructFields));
      out->defined_schema_names = (char **)calloc(struct_count, sizeof(char *));
      if (!out->defined_schemas || !out->defined_schema_names)
        return ENOMEM;
      out->n_defined_schemas = struct_count;
    }

    if (raw_count > 0) {
      out->raw_schema_names = (char **)calloc(raw_count, sizeof(char *));
      out->raw_schema_json = (char **)calloc(raw_count, sizeof(char *));
      if (!out->raw_schema_names || !out->raw_schema_json)
        return ENOMEM;
      out->n_raw_schemas = raw_count;
    }
  }

  {
    size_t struct_idx = 0;
    size_t raw_idx = 0;
    for (i = 0; i < count; ++i) {
      const char *name = json_object_get_name(schemas, i);
      const JSON_Value *schema_val = json_object_get_value_at(schemas, i);
      const JSON_Object *schema_obj = json_value_get_object(schema_val);

      if (!component_key_is_valid(name))
        return EINVAL;

      if (schema_is_struct_compatible(schema_val, schema_obj)) {
        out->defined_schema_names[struct_idx] = c_cdd_strdup(name);
        if (!out->defined_schema_names[struct_idx])
          return ENOMEM;
        struct_fields_init(&out->defined_schemas[struct_idx]);
        if (json_object_to_struct_fields(
                schema_obj, &out->defined_schemas[struct_idx], schemas) != 0) {
          return ENOMEM;
        }
        struct_idx++;
      } else {
        char *raw_json = NULL;
        char *dup_json = NULL;
        out->raw_schema_names[raw_idx] = c_cdd_strdup(name);
        if (!out->raw_schema_names[raw_idx])
          return ENOMEM;
        raw_json = json_serialize_to_string(schema_val);
        if (!raw_json)
          return ENOMEM;
        dup_json = c_cdd_strdup(raw_json);
        json_free_serialized_string(raw_json);
        if (!dup_json)
          return ENOMEM;
        out->raw_schema_json[raw_idx] = dup_json;
        raw_idx++;
      }
    }
  }

  return 0;
}

static int parse_additional_operations(const JSON_Object *const path_obj,
                                       struct OpenAPI_Path *const path,
                                       const struct OpenAPI_Spec *const spec) {
  const JSON_Object *add_ops;
  size_t count, i;

  if (!path_obj || !path)
    return 0;

  add_ops = json_object_get_object(path_obj, "additionalOperations");
  if (!add_ops)
    return 0;

  count = json_object_get_count(add_ops);
  if (count == 0)
    return 0;

  path->additional_operations = (struct OpenAPI_Operation *)calloc(
      count, sizeof(struct OpenAPI_Operation));
  if (!path->additional_operations)
    return ENOMEM;
  path->n_additional_operations = count;

  for (i = 0; i < count; ++i) {
    const char *method = json_object_get_name(add_ops, i);
    const JSON_Object *op_obj =
        json_value_get_object(json_object_get_value_at(add_ops, i));
    struct OpenAPI_Operation *curr = &path->additional_operations[i];
    if (parse_operation(method, op_obj, curr, spec, 1) != 0)
      return ENOMEM;
  }

  return 0;
}

static int parse_paths_object(const JSON_Object *const paths_obj,
                              struct OpenAPI_Path **out_paths,
                              size_t *out_count,
                              const struct OpenAPI_Spec *const spec,
                              int require_leading_slash) {
  size_t i, n_paths;

  if (!paths_obj || !out_paths || !out_count)
    return 0;

  n_paths = json_object_get_count(paths_obj);
  if (n_paths == 0) {
    *out_paths = NULL;
    *out_count = 0;
    return 0;
  }

  *out_paths =
      (struct OpenAPI_Path *)calloc(n_paths, sizeof(struct OpenAPI_Path));
  if (!*out_paths)
    return ENOMEM;
  *out_count = n_paths;

  for (i = 0; i < n_paths; ++i) {
    const char *route = json_object_get_name(paths_obj, i);
    const JSON_Value *p_val = json_object_get_value_at(paths_obj, i);
    const JSON_Object *p_obj = json_value_get_object(p_val);
    struct OpenAPI_Path *curr_path = &(*out_paths)[i];
    size_t n_ops_in_obj = p_obj ? json_object_get_count(p_obj) : 0;
    size_t k, valid_ops = 0;

    if (require_leading_slash && (!route || route[0] != '/'))
      return EINVAL;
    if (route) {
      curr_path->route = c_cdd_strdup(route);
      if (!curr_path->route)
        return ENOMEM;
    }

    if (p_obj) {
      const char *path_ref = json_object_get_string(p_obj, "$ref");
      const char *path_summary = json_object_get_string(p_obj, "summary");
      const char *path_description =
          json_object_get_string(p_obj, "description");
      const JSON_Array *path_params =
          json_object_get_array(p_obj, "parameters");

      if (path_ref) {
        curr_path->ref = c_cdd_strdup(path_ref);
        if (!curr_path->ref)
          return ENOMEM;
      }
      if (path_summary) {
        curr_path->summary = c_cdd_strdup(path_summary);
        if (!curr_path->summary)
          return ENOMEM;
      }
      if (path_description) {
        curr_path->description = c_cdd_strdup(path_description);
        if (!curr_path->description)
          return ENOMEM;
      }
      if (!path_ref) {
        if (collect_extensions(p_obj, &curr_path->extensions_json) != 0)
          return ENOMEM;
      }
      {
        int rc = parse_parameters_array(path_params, &curr_path->parameters,
                                        &curr_path->n_parameters, spec);
        if (rc != 0)
          return rc;
      }
      if (parse_servers_array(p_obj, "servers", &curr_path->servers,
                              &curr_path->n_servers) != 0) {
        return ENOMEM;
      }
      if (parse_additional_operations(p_obj, curr_path, spec) != 0) {
        return ENOMEM;
      }
    }

    if (n_ops_in_obj > 0) {
      curr_path->operations = (struct OpenAPI_Operation *)calloc(
          n_ops_in_obj, sizeof(struct OpenAPI_Operation));

      if (!curr_path->operations) {
        return ENOMEM;
      }

      if (p_obj) {
        for (k = 0; k < n_ops_in_obj; ++k) {
          const char *verb = json_object_get_name(p_obj, k);
          const JSON_Object *op_obj =
              json_value_get_object(json_object_get_value_at(p_obj, k));
          if (!verb)
            continue;
          if (strcmp(verb, "parameters") == 0 || strcmp(verb, "servers") == 0 ||
              strcmp(verb, "summary") == 0 ||
              strcmp(verb, "description") == 0 || strcmp(verb, "$ref") == 0 ||
              strcmp(verb, "additionalOperations") == 0) {
            continue;
          }
          if (parse_operation(verb, op_obj, &curr_path->operations[valid_ops],
                              spec, 0) == 0) {
            if (curr_path->operations[valid_ops].verb != OA_VERB_UNKNOWN) {
              valid_ops++;
            }
          }
        }
      }
      curr_path->n_operations = valid_ops;
    }
  }

  return 0;
}

static int name_in_list(const char *name, char **names, size_t count) {
  size_t i;
  if (!name || !names)
    return 0;
  for (i = 0; i < count; ++i) {
    if (names[i] && strcmp(names[i], name) == 0)
      return 1;
  }
  return 0;
}

static void free_name_list(char **names, size_t count) {
  size_t i;
  if (!names)
    return;
  for (i = 0; i < count; ++i)
    free(names[i]);
  free(names);
}

static int collect_path_template_names(const char *route, char ***out_names,
                                       size_t *out_count) {
  size_t i;
  size_t cap = 0;
  size_t count = 0;
  char **names = NULL;

  if (!out_names || !out_count)
    return EINVAL;
  *out_names = NULL;
  *out_count = 0;

  if (!route)
    return 0;

  for (i = 0; route[i]; ++i) {
    if (route[i] == '{') {
      size_t start = i + 1;
      size_t end = start;
      while (route[end] && route[end] != '}')
        ++end;
      if (!route[end]) {
        free_name_list(names, count);
        return EINVAL;
      }
      if (end == start) {
        free_name_list(names, count);
        return EINVAL;
      }
      {
        size_t len = end - start;
        char *name = (char *)malloc(len + 1);
        if (!name) {
          free_name_list(names, count);
          return ENOMEM;
        }
        memcpy(name, route + start, len);
        name[len] = '\0';
        if (name_in_list(name, names, count)) {
          free(name);
          free_name_list(names, count);
          return EINVAL;
        }
        if (count == cap) {
          size_t new_cap = cap ? cap * 2 : 4;
          char **tmp = (char **)realloc(names, new_cap * sizeof(char *));
          if (!tmp) {
            free(name);
            free_name_list(names, count);
            return ENOMEM;
          }
          names = tmp;
          cap = new_cap;
        }
        names[count++] = name;
      }
      i = end;
    } else if (route[i] == '}') {
      free_name_list(names, count);
      return EINVAL;
    }
  }

  *out_names = names;
  *out_count = count;
  return 0;
}

static const struct OpenAPI_Parameter *
find_path_param(const struct OpenAPI_Parameter *params, size_t n,
                const char *name) {
  size_t i;
  if (!params || !name)
    return NULL;
  for (i = 0; i < n; ++i) {
    if (params[i].in != OA_PARAM_IN_PATH)
      continue;
    if (!params[i].name)
      continue;
    if (strcmp(params[i].name, name) == 0)
      return &params[i];
  }
  return NULL;
}

static int validate_path_params_list(const struct OpenAPI_Parameter *params,
                                     size_t n_params, char **template_names,
                                     size_t n_template_names) {
  size_t i;
  if (!params)
    return 0;
  for (i = 0; i < n_params; ++i) {
    const struct OpenAPI_Parameter *p = &params[i];
    if (p->in != OA_PARAM_IN_PATH)
      continue;
    if (!p->name || !name_in_list(p->name, template_names, n_template_names))
      return EINVAL;
    if (!p->required)
      return EINVAL;
  }
  return 0;
}

static int validate_path_template_for_operation(
    const struct OpenAPI_Path *path, const struct OpenAPI_Operation *op,
    char **template_names, size_t n_template_names) {
  size_t i;
  if (!path || !template_names || n_template_names == 0)
    return 0;

  if (validate_path_params_list(op ? op->parameters : NULL,
                                op ? op->n_parameters : 0, template_names,
                                n_template_names) != 0)
    return EINVAL;

  for (i = 0; i < n_template_names; ++i) {
    const struct OpenAPI_Parameter *p =
        op ? find_path_param(op->parameters, op->n_parameters,
                             template_names[i])
           : NULL;
    if (!p) {
      p = find_path_param(path->parameters, path->n_parameters,
                          template_names[i]);
    }
    if (!p)
      return EINVAL;
  }

  return 0;
}

static int validate_path_templates(const struct OpenAPI_Path *paths,
                                   size_t n_paths) {
  size_t i;
  for (i = 0; i < n_paths; ++i) {
    const struct OpenAPI_Path *path = &paths[i];
    char **template_names = NULL;
    size_t n_template_names = 0;
    size_t op_idx;
    int rc;
    int has_ops;

    if (!path->route)
      continue;
    if (path->ref)
      continue;

    rc = collect_path_template_names(path->route, &template_names,
                                     &n_template_names);
    if (rc != 0)
      return rc;

    rc = validate_path_params_list(path->parameters, path->n_parameters,
                                   template_names, n_template_names);
    if (rc != 0) {
      free_name_list(template_names, n_template_names);
      return rc;
    }

    has_ops = (path->n_operations + path->n_additional_operations) > 0;
    if (!has_ops) {
      free_name_list(template_names, n_template_names);
      continue;
    }

    for (op_idx = 0; op_idx < path->n_operations; ++op_idx) {
      if (validate_path_template_for_operation(path, &path->operations[op_idx],
                                               template_names,
                                               n_template_names) != 0) {
        free_name_list(template_names, n_template_names);
        return EINVAL;
      }
    }

    for (op_idx = 0; op_idx < path->n_additional_operations; ++op_idx) {
      if (validate_path_template_for_operation(
              path, &path->additional_operations[op_idx], template_names,
              n_template_names) != 0) {
        free_name_list(template_names, n_template_names);
        return EINVAL;
      }
    }

    free_name_list(template_names, n_template_names);
  }

  return 0;
}

static void scan_querystring_usage(const struct OpenAPI_Parameter *params,
                                   size_t n_params, size_t *qs_count,
                                   int *has_query) {
  size_t i;
  if (!qs_count || !has_query)
    return;
  for (i = 0; i < n_params; ++i) {
    const struct OpenAPI_Parameter *p = &params[i];
    if (p->in == OA_PARAM_IN_QUERYSTRING) {
      (*qs_count)++;
    } else if (p->in == OA_PARAM_IN_QUERY) {
      *has_query = 1;
    }
  }
}

static int validate_querystring_usage(const struct OpenAPI_Path *paths,
                                      size_t n_paths) {
  size_t i;
  for (i = 0; i < n_paths; ++i) {
    const struct OpenAPI_Path *path = &paths[i];
    size_t path_qs = 0;
    int path_has_query = 0;
    size_t op_idx;

    scan_querystring_usage(path->parameters, path->n_parameters, &path_qs,
                           &path_has_query);
    if (path_qs > 1)
      return EINVAL;
    if (path_qs > 0 && path_has_query)
      return EINVAL;

    for (op_idx = 0; op_idx < path->n_operations; ++op_idx) {
      size_t op_qs = 0;
      int op_has_query = 0;
      size_t total_qs;
      int has_query;
      scan_querystring_usage(path->operations[op_idx].parameters,
                             path->operations[op_idx].n_parameters, &op_qs,
                             &op_has_query);
      total_qs = path_qs + op_qs;
      has_query = path_has_query || op_has_query;
      if (total_qs > 1)
        return EINVAL;
      if (total_qs > 0 && has_query)
        return EINVAL;
    }

    for (op_idx = 0; op_idx < path->n_additional_operations; ++op_idx) {
      size_t op_qs = 0;
      int op_has_query = 0;
      size_t total_qs;
      int has_query;
      scan_querystring_usage(path->additional_operations[op_idx].parameters,
                             path->additional_operations[op_idx].n_parameters,
                             &op_qs, &op_has_query);
      total_qs = path_qs + op_qs;
      has_query = path_has_query || op_has_query;
      if (total_qs > 1)
        return EINVAL;
      if (total_qs > 0 && has_query)
        return EINVAL;
    }
  }
  return 0;
}

static int add_unique_operation_id(char ***ids, size_t *count, size_t *cap,
                                   const char *op_id) {
  size_t i;
  char **tmp;
  if (!op_id || !*op_id)
    return 0;
  for (i = 0; i < *count; ++i) {
    if ((*ids)[i] && strcmp((*ids)[i], op_id) == 0)
      return EINVAL;
  }
  if (*count == *cap) {
    size_t new_cap = *cap ? (*cap * 2) : 8;
    tmp = (char **)realloc(*ids, new_cap * sizeof(char *));
    if (!tmp)
      return ENOMEM;
    *ids = tmp;
    *cap = new_cap;
  }
  (*ids)[*count] = c_cdd_strdup(op_id);
  if (!(*ids)[*count])
    return ENOMEM;
  (*count)++;
  return 0;
}

static int collect_operation_ids(const struct OpenAPI_Path *paths,
                                 size_t n_paths, char ***ids, size_t *count,
                                 size_t *cap) {
  size_t i, j;
  int rc;
  if (!paths)
    return 0;
  for (i = 0; i < n_paths; ++i) {
    const struct OpenAPI_Path *p = &paths[i];
    for (j = 0; j < p->n_operations; ++j) {
      rc = add_unique_operation_id(ids, count, cap,
                                   p->operations[j].operation_id);
      if (rc != 0)
        return rc;
    }
    for (j = 0; j < p->n_additional_operations; ++j) {
      rc = add_unique_operation_id(ids, count, cap,
                                   p->additional_operations[j].operation_id);
      if (rc != 0)
        return rc;
    }
  }
  return 0;
}

static int validate_unique_operation_ids(const struct OpenAPI_Spec *spec) {
  char **ids = NULL;
  size_t count = 0;
  size_t cap = 0;
  int rc = 0;
  size_t i;

  if (!spec)
    return 0;

  rc = collect_operation_ids(spec->paths, spec->n_paths, &ids, &count, &cap);
  if (rc != 0)
    goto cleanup;

  rc = collect_operation_ids(spec->webhooks, spec->n_webhooks, &ids, &count,
                             &cap);
  if (rc != 0)
    goto cleanup;

  rc = collect_operation_ids(spec->component_path_items,
                             spec->n_component_path_items, &ids, &count, &cap);
  if (rc != 0)
    goto cleanup;

cleanup:
  for (i = 0; i < count; ++i) {
    free(ids[i]);
  }
  free(ids);
  return rc;
}

int openapi_load_from_json(const JSON_Value *const root,
                           struct OpenAPI_Spec *const out) {
  const JSON_Object *root_obj;
  const JSON_Object *paths_obj;
  const JSON_Object *webhooks_obj;
  const JSON_Object *comps_obj;
  int rc;

  if (!root || !out)
    return EINVAL;

  root_obj = json_value_get_object(root);
  if (!root_obj)
    return EINVAL;

  {
    const char *version = json_object_get_string(root_obj, "openapi");
    if (version) {
      out->openapi_version = c_cdd_strdup(version);
      if (!out->openapi_version)
        return ENOMEM;
    }
  }
  {
    const char *self_uri = json_object_get_string(root_obj, "$self");
    if (self_uri) {
      out->self_uri = c_cdd_strdup(self_uri);
      if (!out->self_uri)
        return ENOMEM;
    }
  }
  {
    const char *dialect = json_object_get_string(root_obj, "jsonSchemaDialect");
    if (dialect) {
      out->json_schema_dialect = c_cdd_strdup(dialect);
      if (!out->json_schema_dialect)
        return ENOMEM;
    }
  }
  if (collect_extensions(root_obj, &out->extensions_json) != 0)
    return ENOMEM;

  if (parse_info(root_obj, out) != 0) {
    openapi_spec_free(out);
    return ENOMEM;
  }
  {
    const JSON_Object *ext_docs =
        json_object_get_object(root_obj, "externalDocs");
    if (ext_docs) {
      if (parse_external_docs(ext_docs, &out->external_docs) != 0) {
        openapi_spec_free(out);
        return ENOMEM;
      }
    }
  }
  if (parse_tags(root_obj, out) != 0) {
    openapi_spec_free(out);
    return ENOMEM;
  }

  if (parse_security_field(root_obj, "security", &out->security,
                           &out->n_security, &out->security_set) != 0) {
    openapi_spec_free(out);
    return ENOMEM;
  }

  if (parse_servers(root_obj, out) != 0) {
    openapi_spec_free(out);
    return ENOMEM;
  }

  paths_obj = json_object_get_object(root_obj, "paths");
  webhooks_obj = json_object_get_object(root_obj, "webhooks");
  comps_obj = json_object_get_object(root_obj, "components");

  /* Load Schemas First */
  if (comps_obj) {
    if (parse_components(comps_obj, out) != 0) {
      openapi_spec_free(out);
      return ENOMEM;
    }
  }

  if (paths_obj) {
    rc = parse_paths_object(paths_obj, &out->paths, &out->n_paths, out, 1);
    if (rc != 0) {
      openapi_spec_free(out);
      return rc;
    }
    rc = validate_path_templates(out->paths, out->n_paths);
    if (rc != 0) {
      openapi_spec_free(out);
      return rc;
    }
    rc = validate_querystring_usage(out->paths, out->n_paths);
    if (rc != 0) {
      openapi_spec_free(out);
      return rc;
    }
  }

  if (webhooks_obj) {
    rc = parse_paths_object(webhooks_obj, &out->webhooks, &out->n_webhooks, out,
                            0);
    if (rc != 0) {
      openapi_spec_free(out);
      return rc;
    }
    rc = validate_querystring_usage(out->webhooks, out->n_webhooks);
    if (rc != 0) {
      openapi_spec_free(out);
      return rc;
    }
  }

  if (out->component_path_items && out->n_component_path_items > 0) {
    rc = validate_querystring_usage(out->component_path_items,
                                    out->n_component_path_items);
    if (rc != 0) {
      openapi_spec_free(out);
      return rc;
    }
  }

  rc = validate_unique_operation_ids(out);
  if (rc != 0) {
    openapi_spec_free(out);
    return rc;
  }

  return 0;
}

const struct StructFields *
openapi_spec_find_schema(const struct OpenAPI_Spec *spec, const char *name) {
  size_t i;
  if (!spec || !name)
    return NULL;
  for (i = 0; i < spec->n_defined_schemas; ++i) {
    if (spec->defined_schema_names[i] &&
        strcmp(spec->defined_schema_names[i], name) == 0) {
      return &spec->defined_schemas[i];
    }
  }
  return NULL;
}
