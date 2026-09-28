/**
 * @file test_operation_common.h
 * @brief Common definitions and helpers for operation generator tests.
 */

#ifndef TEST_OPERATION_COMMON_H
#define TEST_OPERATION_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include "greatest.h"

#include "routes/emit/operation.h"
/* clang-format on */

extern C_CDD_EXPORT int g_cdd_fail_schema_ref_has_data;
extern C_CDD_EXPORT int g_cdd_fail_json_serialize;
extern C_CDD_EXPORT int g_cdd_fail_apply_format;
extern C_CDD_EXPORT int g_cdd_fail_stricmp;
extern C_CDD_EXPORT int g_op_fail_find_media_type_op;
extern C_CDD_EXPORT int g_op_fail_oa_type_is_primitive;
extern C_CDD_EXPORT int g_mapping_fail_init;
extern C_CDD_EXPORT int g_op_fail_find_doc_param;
extern C_CDD_EXPORT int g_op_fail_is_path_param;
extern C_CDD_EXPORT int g_op_fail_is_struct_pointer;
extern C_CDD_EXPORT int g_op_fail_doc_style_to_openapi;
extern C_CDD_EXPORT int g_op_fail_ensure_response_null;
extern C_CDD_EXPORT int g_op_fail_ensure_response_for_code;

static void reset_operation_test(struct OpenAPI_Operation *op) {
  size_t i;
  if (!op)
    return;
  free(op->operation_id);
  free(op->summary);
  free(op->description);
  free(op->method);
  if (op->parameters) {
    for (i = 0; i < op->n_parameters; i++) {
      free(op->parameters[i].name);
      free(op->parameters[i].type);
      free(op->parameters[i].description);
      free(op->parameters[i].content_type);
      free(op->parameters[i].items_type);
      free(op->parameters[i].example.string);
      free(op->parameters[i].example.json);
      free(op->parameters[i].schema.ref_name);
      free(op->parameters[i].schema.ref);
      free(op->parameters[i].schema.inline_type);
      free(op->parameters[i].schema.items_ref);
      free(op->parameters[i].schema.format);
      free(op->parameters[i].schema.items_format);
      free(op->parameters[i].schema.content_media_type);
      free(op->parameters[i].schema.content_encoding);
      free(op->parameters[i].schema.items_content_media_type);
      free(op->parameters[i].schema.items_content_encoding);
    }
    free(op->parameters);
  }
  if (op->tags) {
    for (i = 0; i < op->n_tags; ++i) {
      free(op->tags[i]);
    }
    free(op->tags);
  }
  free(op->external_docs.url);
  free(op->external_docs.description);
  free(op->req_body.ref_name);
  free(op->req_body.inline_type);
  free(op->req_body.content_type);
  free(op->req_body.format);
  free(op->req_body.items_ref);
  free(op->req_body.items_format);
  if (op->req_body_media_types) {
    for (i = 0; i < op->n_req_body_media_types; ++i) {
      struct OpenAPI_MediaType *mt = &op->req_body_media_types[i];
      size_t e;
      free(mt->name);
      free(mt->ref);
      free(mt->extensions_json);
      free(mt->schema.ref_name);
      free(mt->schema.ref);
      free(mt->schema.inline_type);
      free(mt->schema.items_ref);
      free(mt->schema.format);
      free(mt->schema.items_format);
      free(mt->schema.content_media_type);
      free(mt->schema.content_encoding);
      free(mt->schema.items_content_media_type);
      free(mt->schema.items_content_encoding);
      free(mt->example.string);
      free(mt->example.json);
      if (mt->encoding) {
        for (e = 0; e < mt->n_encoding; ++e) {
          free(mt->encoding[e].name);
          free(mt->encoding[e].content_type);
        }
        free(mt->encoding);
      }
      if (mt->prefix_encoding) {
        for (e = 0; e < mt->n_prefix_encoding; ++e) {
          free(mt->prefix_encoding[e].name);
          free(mt->prefix_encoding[e].content_type);
        }
        free(mt->prefix_encoding);
      }
      if (mt->item_encoding) {
        free(mt->item_encoding->name);
        free(mt->item_encoding->content_type);
        free(mt->item_encoding);
      }
    }
    free(op->req_body_media_types);
  }
  free(op->req_body_description);
  free(op->req_body_extensions_json);
  free(op->req_body_ref);
  free(op->req_body.example.string);
  free(op->req_body.example.json);
  if (op->responses) {
    for (i = 0; i < op->n_responses; i++) {
      size_t m;
      free(op->responses[i].code);
      free(op->responses[i].summary);
      free(op->responses[i].description);
      free(op->responses[i].content_type);
      free(op->responses[i].example.string);
      free(op->responses[i].example.json);
      free(op->responses[i].schema.ref_name);
      free(op->responses[i].schema.ref);
      free(op->responses[i].schema.inline_type);
      free(op->responses[i].schema.items_ref);
      free(op->responses[i].schema.format);
      free(op->responses[i].schema.items_format);
      if (op->responses[i].content_media_types) {
        for (m = 0; m < op->responses[i].n_content_media_types; ++m) {
          struct OpenAPI_MediaType *mt =
              &op->responses[i].content_media_types[m];
          free(mt->name);
          free(mt->schema.ref_name);
          free(mt->schema.ref);
          free(mt->schema.inline_type);
          free(mt->schema.items_ref);
          free(mt->schema.format);
          free(mt->schema.items_format);
          free(mt->item_schema.ref_name);
          free(mt->item_schema.ref);
          free(mt->item_schema.inline_type);
          free(mt->item_schema.items_ref);
          free(mt->item_schema.format);
          free(mt->item_schema.items_format);
          free(mt->example.string);
          free(mt->example.json);
        }
        free(op->responses[i].content_media_types);
      }
      if (op->responses[i].headers) {
        size_t h;
        for (h = 0; h < op->responses[i].n_headers; ++h) {
          struct OpenAPI_Header *hdr = &op->responses[i].headers[h];
          free(hdr->name);
          free(hdr->description);
          free(hdr->content_type);
          free(hdr->type);
          free(hdr->schema.inline_type);
          free(hdr->schema.format);
          free(hdr->example.string);
          free(hdr->example.json);
        }
        free(op->responses[i].headers);
      }
      if (op->responses[i].links) {
        size_t l;
        for (l = 0; l < op->responses[i].n_links; ++l) {
          struct OpenAPI_Link *lnk = &op->responses[i].links[l];
          free(lnk->name);
          free(lnk->summary);
          free(lnk->description);
          free(lnk->operation_id);
          free(lnk->operation_ref);
          if (lnk->parameters) {
            size_t p;
            for (p = 0; p < lnk->n_parameters; ++p) {
              free(lnk->parameters[p].name);
              free(lnk->parameters[p].value.string);
              free(lnk->parameters[p].value.json);
            }
            free(lnk->parameters);
          }
          free(lnk->request_body.string);
          free(lnk->request_body.json);
          if (lnk->server) {
            free(lnk->server->url);
            free(lnk->server->name);
            free(lnk->server->description);
            free(lnk->server);
          }
        }
        free(op->responses[i].links);
      }
    }
    free(op->responses);
  }
  if (op->security) {
    for (i = 0; i < op->n_security; ++i) {
      struct OpenAPI_SecurityRequirementSet *set = &op->security[i];
      if (set->requirements) {
        size_t r;
        for (r = 0; r < set->n_requirements; ++r) {
          size_t s;
          free(set->requirements[r].scheme);
          if (set->requirements[r].scopes) {
            for (s = 0; s < set->requirements[r].n_scopes; ++s) {
              free(set->requirements[r].scopes[s]);
            }
            free(set->requirements[r].scopes);
          }
        }
        free(set->requirements);
      }
    }
    free(op->security);
  }
  if (op->servers) {
    for (i = 0; i < op->n_servers; ++i) {
      free_openapi_server_variables_op(&op->servers[i]);
      free(op->servers[i].url);
      free(op->servers[i].name);
      free(op->servers[i].description);
    }
    free(op->servers);
  }
  memset(op, 0, sizeof(*op));
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPERATION_COMMON_H */
