#ifdef _MSC_VER
#ifndef strdup
#define strdup _strdup
#endif
#endif
/**
 * @file test_c2openapi_op_helpers.h
 * @brief Helper fixtures for OpenAPI operation builder unit tests.
 */

#ifndef TEST_C2OPENAPI_OP_HELPERS_H
#define TEST_C2OPENAPI_OP_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/parse/inspector.h"
#include "docstrings/parse/doc.h"
#include "openapi/parse/openapi.h"
#include "routes/emit/operation.h"
/* clang-format on */

static void reset_op(struct OpenAPI_Operation *op) {
  size_t i;
  /* Use openapi_spec_free or manual free?
     The op inside spec relies on arrays. Loader's free logic is complex.
     We use a simplified free here since we only populate one op.
  */
  /* Call module internal logic if available or reimplement basic cleanup */
  /* Reimplementing basics for test safety */
  if (op->operation_id)
    free(op->operation_id);
  if (op->summary)
    free(op->summary);
  if (op->description)
    free(op->description);
  if (op->method)
    free(op->method);
  if (op->parameters) {
    for (i = 0; i < op->n_parameters; i++) {
      free(op->parameters[i].name);
      /* op->parameters[i].in is an enum, nothing to free */
      free(op->parameters[i].type);
      if (op->parameters[i].description)
        free(op->parameters[i].description);
      if (op->parameters[i].content_type)
        free(op->parameters[i].content_type);
      if (op->parameters[i].items_type)
        free(op->parameters[i].items_type);
      if (op->parameters[i].example.type == OA_ANY_STRING &&
          op->parameters[i].example.string)
        free(op->parameters[i].example.string);
      if (op->parameters[i].example.type == OA_ANY_JSON &&
          op->parameters[i].example.json)
        free(op->parameters[i].example.json);
      if (op->parameters[i].schema.ref_name)
        free(op->parameters[i].schema.ref_name);
      if (op->parameters[i].schema.ref)
        free(op->parameters[i].schema.ref);
      if (op->parameters[i].schema.inline_type)
        free(op->parameters[i].schema.inline_type);
      if (op->parameters[i].schema.items_ref)
        free(op->parameters[i].schema.items_ref);
      if (op->parameters[i].schema.format)
        free(op->parameters[i].schema.format);
      if (op->parameters[i].schema.items_format)
        free(op->parameters[i].schema.items_format);
      if (op->parameters[i].schema.content_media_type)
        free(op->parameters[i].schema.content_media_type);
      if (op->parameters[i].schema.content_encoding)
        free(op->parameters[i].schema.content_encoding);
      if (op->parameters[i].schema.items_content_media_type)
        free(op->parameters[i].schema.items_content_media_type);
      if (op->parameters[i].schema.items_content_encoding)
        free(op->parameters[i].schema.items_content_encoding);
    }
    free(op->parameters);
  }
  if (op->tags) {
    if (op->n_tags > 0) {

      for (i = 0; i < op->n_tags; ++i) {
        if (op->tags[i])
          free(op->tags[i]);
      }
    }
    free(op->tags);
  }
  if (op->external_docs.url)
    free(op->external_docs.url);
  if (op->external_docs.description)
    free(op->external_docs.description);
  if (op->req_body.ref_name)
    free(op->req_body.ref_name);
  if (op->req_body.inline_type)
    free(op->req_body.inline_type);
  if (op->req_body.content_type)
    free(op->req_body.content_type);
  if (op->req_body_media_types) {
    for (i = 0; i < op->n_req_body_media_types; ++i) {
      struct OpenAPI_MediaType *mt = &op->req_body_media_types[i];
      if (mt->name)
        free(mt->name);
      if (mt->ref)
        free(mt->ref);
      if (mt->extensions_json)
        free(mt->extensions_json);
      if (mt->schema.ref_name)
        free(mt->schema.ref_name);
      if (mt->schema.ref)
        free(mt->schema.ref);
      if (mt->schema.inline_type)
        free(mt->schema.inline_type);
      if (mt->schema.items_ref)
        free(mt->schema.items_ref);
      if (mt->schema.format)
        free(mt->schema.format);
      if (mt->schema.items_format)
        free(mt->schema.items_format);
      if (mt->schema.content_media_type)
        free(mt->schema.content_media_type);
      if (mt->schema.content_encoding)
        free(mt->schema.content_encoding);
      if (mt->schema.items_content_media_type)
        free(mt->schema.items_content_media_type);
      if (mt->schema.items_content_encoding)
        free(mt->schema.items_content_encoding);
      if (mt->example.type == OA_ANY_STRING && mt->example.string)
        free(mt->example.string);
      if (mt->example.type == OA_ANY_JSON && mt->example.json)
        free(mt->example.json);
    }
    free(op->req_body_media_types);
  }
  if (op->req_body_description)
    free(op->req_body_description);
  if (op->req_body_extensions_json)
    free(op->req_body_extensions_json);
  if (op->req_body_ref)
    free(op->req_body_ref);
  if (op->req_body.example.type == OA_ANY_STRING && op->req_body.example.string)
    free(op->req_body.example.string);
  if (op->req_body.example.type == OA_ANY_JSON && op->req_body.example.json)
    free(op->req_body.example.json);
  if (op->responses) {
    for (i = 0; i < op->n_responses; i++) {
      free(op->responses[i].code);
      if (op->responses[i].summary)
        free(op->responses[i].summary);
      if (op->responses[i].description)
        free(op->responses[i].description);
      if (op->responses[i].content_type)
        free(op->responses[i].content_type);
      if (op->responses[i].example.type == OA_ANY_STRING &&
          op->responses[i].example.string)
        free(op->responses[i].example.string);
      if (op->responses[i].example.type == OA_ANY_JSON &&
          op->responses[i].example.json)
        free(op->responses[i].example.json);
      if (op->responses[i].schema.ref_name)
        free(op->responses[i].schema.ref_name);
      if (op->responses[i].schema.inline_type)
        free(op->responses[i].schema.inline_type);
      if (op->responses[i].content_media_types) {
        size_t j;
        for (j = 0; j < op->responses[i].n_content_media_types; ++j) {
          struct OpenAPI_MediaType *mt =
              &op->responses[i].content_media_types[j];
          if (mt->name)
            free(mt->name);
          if (mt->ref)
            free(mt->ref);
          if (mt->extensions_json)
            free(mt->extensions_json);
          if (mt->schema.ref_name)
            free(mt->schema.ref_name);
          if (mt->schema.ref)
            free(mt->schema.ref);
          if (mt->schema.inline_type)
            free(mt->schema.inline_type);
          if (mt->schema.items_ref)
            free(mt->schema.items_ref);
          if (mt->schema.format)
            free(mt->schema.format);
          if (mt->schema.items_format)
            free(mt->schema.items_format);
          if (mt->schema.content_media_type)
            free(mt->schema.content_media_type);
          if (mt->schema.content_encoding)
            free(mt->schema.content_encoding);
          if (mt->schema.items_content_media_type)
            free(mt->schema.items_content_media_type);
          if (mt->schema.items_content_encoding)
            free(mt->schema.items_content_encoding);
          if (mt->example.type == OA_ANY_STRING && mt->example.string)
            free(mt->example.string);
          if (mt->example.type == OA_ANY_JSON && mt->example.json)
            free(mt->example.json);
        }
        free(op->responses[i].content_media_types);
      }
      if (op->responses[i].headers) {
        size_t h;
        for (h = 0; h < op->responses[i].n_headers; ++h) {
          struct OpenAPI_Header *hdr = &op->responses[i].headers[h];
          free(hdr->name);
          if (hdr->ref)
            free(hdr->ref);
          if (hdr->description)
            free(hdr->description);
          if (hdr->content_type)
            free(hdr->content_type);
          if (hdr->content_ref)
            free(hdr->content_ref);
          if (hdr->type)
            free(hdr->type);
          if (hdr->items_type)
            free(hdr->items_type);
          if (hdr->schema.ref_name)
            free(hdr->schema.ref_name);
          if (hdr->schema.ref)
            free(hdr->schema.ref);
          if (hdr->schema.inline_type)
            free(hdr->schema.inline_type);
          if (hdr->schema.items_ref)
            free(hdr->schema.items_ref);
          if (hdr->schema.format)
            free(hdr->schema.format);
          if (hdr->schema.items_format)
            free(hdr->schema.items_format);
          if (hdr->schema.content_media_type)
            free(hdr->schema.content_media_type);
          if (hdr->schema.content_encoding)
            free(hdr->schema.content_encoding);
          if (hdr->schema.items_content_media_type)
            free(hdr->schema.items_content_media_type);
          if (hdr->schema.items_content_encoding)
            free(hdr->schema.items_content_encoding);
          if (hdr->example.type == OA_ANY_STRING && hdr->example.string)
            free(hdr->example.string);
          if (hdr->example.type == OA_ANY_JSON && hdr->example.json)
            free(hdr->example.json);
        }
        free(op->responses[i].headers);
      }
      if (op->responses[i].links) {
        size_t l;
        for (l = 0; l < op->responses[i].n_links; ++l) {
          struct OpenAPI_Link *link = &op->responses[i].links[l];
          size_t p;
          free(link->name);
          if (link->ref)
            free(link->ref);
          if (link->summary)
            free(link->summary);
          if (link->description)
            free(link->description);
          if (link->operation_ref)
            free(link->operation_ref);
          if (link->operation_id)
            free(link->operation_id);
          if (link->parameters) {
            for (p = 0; p < link->n_parameters; ++p) {
              struct OpenAPI_LinkParam *lp = &link->parameters[p];
              free(lp->name);
              if (lp->value.type == OA_ANY_STRING && lp->value.string)
                free(lp->value.string);
              if (lp->value.type == OA_ANY_JSON && lp->value.json)
                free(lp->value.json);
            }
            free(link->parameters);
          }
          if (link->request_body_set) {
            if (link->request_body.type == OA_ANY_STRING &&
                link->request_body.string)
              free(link->request_body.string);
            if (link->request_body.type == OA_ANY_JSON &&
                link->request_body.json)
              free(link->request_body.json);
          }
          if (link->server_set && link->server) {
            if (link->server->url)
              free(link->server->url);
            if (link->server->name)
              free(link->server->name);
            if (link->server->description)
              free(link->server->description);
            free(link->server);
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
      size_t v;
      free(op->servers[i].url);
      free(op->servers[i].name);
      free(op->servers[i].description);
      if (op->servers[i].variables) {
        for (v = 0; v < op->servers[i].n_variables; ++v) {
          size_t e;
          struct OpenAPI_ServerVariable *var = &op->servers[i].variables[v];
          free(var->name);
          free(var->default_value);
          free(var->description);
          if (var->enum_values) {
            for (e = 0; e < var->n_enum_values; ++e) {
              free(var->enum_values[e]);
            }
            free(var->enum_values);
          }
        }
        free(op->servers[i].variables);
      }
    }
    free(op->servers);
  }
  memset(op, 0, sizeof(*op));
}

static cdd_c_error_t
find_response_media_type(const struct OpenAPI_Response *resp, const char *name,
                         const struct OpenAPI_MediaType **_out_val) {
  size_t i;
  if (!resp || !name || !resp->content_media_types) {
    *_out_val = NULL;
    return 0;
  }
  for (i = 0; i < resp->n_content_media_types; ++i) {
    const struct OpenAPI_MediaType *mt = &resp->content_media_types[i];
    if (mt->name && strcmp(mt->name, name) == 0) {
      *_out_val = mt;
      return 0;
    }
  }
  {
    *_out_val = NULL;
    return 0;
  }
}

/* --- Tests --- */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_C2OPENAPI_OP_HELPERS_H */
