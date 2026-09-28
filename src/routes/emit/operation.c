/**
 * @file operation.c
 * @brief Implementation of operation builder.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "win_compat_sym.h"

#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "classes/parse/mapping.h"
#include "functions/parse/str.h"
#include "routes/emit/operation.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
#undef malloc
#define malloc(sz) C_CDD_MALLOC(sz)
#undef realloc
#define realloc(ptr, sz) C_CDD_REALLOC(ptr, sz)
#undef calloc
#define calloc(n, sz) C_CDD_CALLOC(n, sz)
extern C_CDD_EXPORT int g_cdd_fail_schema_ref_has_data;
extern C_CDD_EXPORT int g_cdd_fail_json_serialize;
extern C_CDD_EXPORT int g_cdd_fail_apply_format;
#endif

/**
 * @brief Executes the c2openapi build operation operation.
 */
cdd_c_error_t c2openapi_build_operation(const struct OpBuilderContext *ctx,
                                        struct OpenAPI_Operation *out_op) {
  const struct C2OpenAPI_ParsedSig *sig;
  const struct DocMetadata *doc;
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!ctx || !out_op || !ctx->sig)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  memset(out_op, 0, sizeof(*out_op));
  sig = ctx->sig;
  doc = ctx->doc;

  /* 0. Basic Metadata */
  if (doc && doc->verb) {
    int diff;
    /* Map string verb to enum */
    /* Only basic check here, loader does robust parsing */
    c_cdd_stricmp(doc->verb, "GET", &diff);
    if (diff == 0)
      out_op->verb = OA_VERB_GET;
    else {
      c_cdd_stricmp(doc->verb, "POST", &diff);
      if (diff == 0)
        out_op->verb = OA_VERB_POST;
      else {
        c_cdd_stricmp(doc->verb, "PUT", &diff);
        if (diff == 0)
          out_op->verb = OA_VERB_PUT;
        else {
          c_cdd_stricmp(doc->verb, "DELETE", &diff);
          if (diff == 0)
            out_op->verb = OA_VERB_DELETE;
          else {
            c_cdd_stricmp(doc->verb, "PATCH", &diff);
            if (diff == 0)
              out_op->verb = OA_VERB_PATCH;
            else {
              c_cdd_stricmp(doc->verb, "HEAD", &diff);
              if (diff == 0)
                out_op->verb = OA_VERB_HEAD;
              else {
                c_cdd_stricmp(doc->verb, "OPTIONS", &diff);
                if (diff == 0)
                  out_op->verb = OA_VERB_OPTIONS;
                else {
                  c_cdd_stricmp(doc->verb, "TRACE", &diff);
                  if (diff == 0)
                    out_op->verb = OA_VERB_TRACE;
                  else {
                    c_cdd_stricmp(doc->verb, "QUERY", &diff);
                    if (diff == 0)
                      out_op->verb = OA_VERB_QUERY;
                    else {
                      out_op->verb = OA_VERB_UNKNOWN;
                      out_op->is_additional = 1;
                      rc = c_cdd_strdup(doc->verb, &out_op->method);
                      if (rc != CDD_C_SUCCESS) {
                        C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
                        return rc;
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  } else {
    /* Guess from name? e.g. "api_get_..." */
    int is_prefix = 0;
    if (ctx->func_name) {
      c_cdd_str_starts_with(ctx->func_name, "api_post_", &is_prefix);
      if (is_prefix || strstr(ctx->func_name, "_create"))
        out_op->verb = OA_VERB_POST;
      else {
        c_cdd_str_starts_with(ctx->func_name, "api_put_", &is_prefix);
        if (is_prefix || strstr(ctx->func_name, "_update"))
          out_op->verb = OA_VERB_PUT;
        else {
          c_cdd_str_starts_with(ctx->func_name, "api_delete_", &is_prefix);
          if (is_prefix || strstr(ctx->func_name, "_delete"))
            out_op->verb = OA_VERB_DELETE;
          else
            out_op->verb = OA_VERB_GET;
        }
      }
    } else {
      out_op->verb = OA_VERB_GET;
    }
  }

  if (doc && doc->operation_id) {
    rc = c_cdd_strdup(doc->operation_id, &out_op->operation_id);
  } else {
    rc = c_cdd_strdup(ctx->func_name, &out_op->operation_id);
  }
  if (rc != CDD_C_SUCCESS) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return rc;
  }
  if (doc && doc->summary) {
    rc = c_cdd_strdup(doc->summary, &out_op->summary);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return rc;
    }
  }
  if (doc && doc->description) {
    rc = c_cdd_strdup(doc->description, &out_op->description);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return rc;
    }
  }
  if (doc && doc->deprecated_set) {
    out_op->deprecated = doc->deprecated ? 1 : 0;
  }
  if (doc && doc->external_docs_url) {
    rc = c_cdd_strdup(doc->external_docs_url, &out_op->external_docs.url);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (doc->external_docs_description) {
      rc = c_cdd_strdup(doc->external_docs_description,
                        &out_op->external_docs.description);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (doc && doc->n_tags > 0) {
    size_t t;
    out_op->tags = (char **)calloc(doc->n_tags, sizeof(char *));
    if (!out_op->tags) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    out_op->n_tags = doc->n_tags;
    for (t = 0; t < doc->n_tags; ++t) {
      rc = c_cdd_strdup(doc->tags[t], &out_op->tags[t]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  if (doc && doc->n_security > 0) {
    size_t s;
    out_op->security = (struct OpenAPI_SecurityRequirementSet *)calloc(
        doc->n_security, sizeof(struct OpenAPI_SecurityRequirementSet));
    if (!out_op->security) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    out_op->n_security = doc->n_security;
    out_op->security_set = 1;
    for (s = 0; s < doc->n_security; ++s) {
      struct OpenAPI_SecurityRequirementSet *set = &out_op->security[s];
      const struct DocSecurityRequirement *src = &doc->security[s];
      set->requirements = (struct OpenAPI_SecurityRequirement *)calloc(
          1, sizeof(struct OpenAPI_SecurityRequirement));
      if (!set->requirements) {
        C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
        return CDD_C_ERROR_MEMORY;
      }
      set->n_requirements = 1;
      rc = c_cdd_strdup(src->scheme, &set->requirements[0].scheme);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (src->n_scopes > 0) {
        size_t k;
        set->requirements[0].scopes =
            (char **)calloc(src->n_scopes, sizeof(char *));
        if (!set->requirements[0].scopes)
          return CDD_C_ERROR_MEMORY;
        set->requirements[0].n_scopes = src->n_scopes;
        for (k = 0; k < src->n_scopes; ++k) {
          rc = c_cdd_strdup(src->scopes[k], &set->requirements[0].scopes[k]);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      }
    }
  }

  if (doc && doc->n_servers > 0) {
    size_t s;
    out_op->servers = (struct OpenAPI_Server *)calloc(
        doc->n_servers, sizeof(struct OpenAPI_Server));
    if (!out_op->servers) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    out_op->n_servers = doc->n_servers;
    for (s = 0; s < doc->n_servers; ++s) {
      const struct DocServer *src = &doc->servers[s];
      if (src->url) {
        rc = c_cdd_strdup(src->url, &out_op->servers[s].url);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      if (src->name) {
        rc = c_cdd_strdup(src->name, &out_op->servers[s].name);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      if (src->description) {
        rc = c_cdd_strdup(src->description, &out_op->servers[s].description);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      if (src->n_variables > 0) {
        cdd_c_error_t vrc =
            copy_doc_server_variables_op(&out_op->servers[s], src);
        if (vrc != CDD_C_SUCCESS)
          return vrc;
      }
    }
  }

  /* 1. Argument Iteration */
  for (i = 0; i < sig->n_args; ++i) {
    const struct C2OpenAPI_ParsedArg *arg = &sig->args[i];
    struct DocParam *dp = NULL;
    struct OpenAPI_Parameter curr_param;
    struct OpenApiTypeMapping type_map;
    int is_path = 0;
    int is_body = 0;
    int is_out_ptr = 0;
    int is_querystring = 0;
    int _is_reserved = 0;
    int _is_path_param = 0;

    memset(&curr_param, 0, sizeof(curr_param));
    rc = c_mapping_init(&type_map);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = find_doc_param(doc, arg->name, &dp);
    if (rc != CDD_C_SUCCESS) {
      c_mapping_free(&type_map);
      return rc;
    }

    /* --- Heuristic: Role Detection --- */

    /* A. Explicit Documentation Override */
    if (dp && dp->in_loc) {
      if (strcmp(dp->in_loc, "path") == 0)
        is_path = 1;
      else if (strcmp(dp->in_loc, "querystring") == 0)
        is_querystring = 1;
      /* "body" isn't a parameter location in OpenAPI 3, but a concept.
         If user says @param [in:body], we treat as Body. */
      else if (strcmp(dp->in_loc, "body") == 0)
        is_body = 1;
    } else {
      /* B. Implicit Path: Matches {name} in route */
      if (doc && doc->route) {
        rc = is_path_param(doc->route, arg->name, &_is_path_param);
        if (rc != CDD_C_SUCCESS) {
          c_mapping_free(&type_map);
          return rc;
        }
        if (_is_path_param)
          is_path = 1;
      }
      /* C. Implicit Body: "struct *" without const in POST/PUT/PATCH? */
      if (!is_path) {
        int is_double = 0;
        int _is_struct_ptr = 0;
        rc = is_struct_pointer(arg->type, &is_double, &_is_struct_ptr);
        if (rc != CDD_C_SUCCESS) {
          c_mapping_free(&type_map);
          return rc;
        }
        if (_is_struct_ptr) {
          if (is_double) {
            /* Double pointer usually `struct X **out` -> Response Body (Output)
             */
            is_out_ptr = 1;
          } else if (strstr(arg->type, "const ")) {
            /* `const struct X *in` -> Request Body */
            if (out_op->verb == OA_VERB_POST || out_op->verb == OA_VERB_PUT ||
                out_op->verb == OA_VERB_PATCH) {
              is_body = 1;
            }
          } else {
            /* `struct X *` (non-const) is ambiguous.
               Could be in-out, or body.
               Default to Request Body for state-changing verbs. */
            if (out_op->verb == OA_VERB_POST || out_op->verb == OA_VERB_PUT) {
              is_body = 1;
            }
          }
        } else if (strstr(arg->type, "**")) {
          is_out_ptr = 1;
        }
      }
    }

    /* Analyze Type using C Mapper */
    rc = c_mapping_map_type(arg->type, arg->name, &type_map);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }

    if (is_out_ptr) {
      /* This is an output parameter (Response Body Schema).
         We store it to populate a "200 OK" response later. */
      struct OpenAPI_Response *r;
      size_t r_idx = out_op->n_responses; /* Add new response */
      struct OpenAPI_Response *new_resps = (struct OpenAPI_Response *)realloc(
          out_op->responses,
          (out_op->n_responses + 1) * sizeof(struct OpenAPI_Response));
      if (!new_resps) {
        c_mapping_free(&type_map);
        return CDD_C_ERROR_MEMORY;
      }
      out_op->responses = new_resps;
      r = &out_op->responses[r_idx];
      memset(r, 0, sizeof(*r));
      rc = c_cdd_strdup("200", &r->code);
      if (rc != CDD_C_SUCCESS) {
        c_mapping_free(&type_map);
        return rc;
      }
      rc = c_cdd_strdup("Success", &r->description);
      if (rc != CDD_C_SUCCESS) {
        free(r->code);
        r->code = NULL;
        c_mapping_free(&type_map);
        return rc;
      }
      out_op->n_responses++;

      /* Map Schema */
      r->schema.is_array = (type_map.kind == OA_TYPE_ARRAY);
      if (type_map.ref_name) {
        rc = c_cdd_strdup(type_map.ref_name, &r->schema.ref_name);
        if (rc != CDD_C_SUCCESS) {
          c_mapping_free(&type_map);
          return rc;
        }
      } else {
        rc = c_cdd_strdup(type_map.oa_type, &r->schema.inline_type);
        if (rc != CDD_C_SUCCESS) {
          c_mapping_free(&type_map);
          return rc;
        }
      }
      {
        int fmt_applied = 0;
        rc = apply_format_to_schema_ref(&r->schema, &type_map, NULL,
                                        &fmt_applied);
        if (rc != CDD_C_SUCCESS) {
          c_mapping_free(&type_map);
          return rc;
        }
      }

      c_mapping_free(&type_map);
      continue; /* Done with this arg */
    }

    if (is_body) {
      /* Request Body Population */
      rc = c_cdd_strdup("application/json", &out_op->req_body.content_type);
      if (rc != CDD_C_SUCCESS) {
        c_mapping_free(&type_map);
        return rc;
      }
      out_op->req_body.is_array = (type_map.kind == OA_TYPE_ARRAY);
      /* Use ref_name if object, or type if primitive */
      if (type_map.ref_name) {
        rc = c_cdd_strdup(type_map.ref_name, &out_op->req_body.ref_name);
        if (rc != CDD_C_SUCCESS) {
          c_mapping_free(&type_map);
          return rc;
        }
      } else {
        rc = c_cdd_strdup(type_map.oa_type, &out_op->req_body.inline_type);
        if (rc != CDD_C_SUCCESS) {
          c_mapping_free(&type_map);
          return rc;
        }
      }
      out_op->req_body_required = 1;
      out_op->req_body_required_set = 1;
      {
        int fmt_applied = 0;
        rc = apply_format_to_schema_ref(&out_op->req_body, &type_map, NULL,
                                        &fmt_applied);
        if (rc != CDD_C_SUCCESS) {
          free(out_op->req_body.content_type);
          out_op->req_body.content_type = NULL;
          if (out_op->req_body.ref_name) {
            free(out_op->req_body.ref_name);
            out_op->req_body.ref_name = NULL;
          }
          if (out_op->req_body.inline_type) {
            free(out_op->req_body.inline_type);
            out_op->req_body.inline_type = NULL;
          }
          c_mapping_free(&type_map);
          return rc;
        }
      }

      c_mapping_free(&type_map);
      continue;
    }

    /* --- Standard Parameter (Query/Path/Header/Cookie) --- */

    rc = c_cdd_strdup(arg->name, &curr_param.name);
    if (rc != CDD_C_SUCCESS) {
      c_mapping_free(&type_map);
      return rc;
    }
    curr_param.required = is_path; /* Path params always required */
    if (dp && dp->required)
      curr_param.required = 1;
    if (dp && dp->description) {
      rc = c_cdd_strdup(dp->description, &curr_param.description);
      if (rc != CDD_C_SUCCESS) {
        free_param_fields(&curr_param);
        c_mapping_free(&type_map);
        return rc;
      }
    }

    curr_param.in = is_path ? OA_PARAM_IN_PATH : OA_PARAM_IN_QUERY;
    if (dp && dp->in_loc && strcmp(dp->in_loc, "header") == 0)
      curr_param.in = OA_PARAM_IN_HEADER;
    else if (dp && dp->in_loc && strcmp(dp->in_loc, "cookie") == 0)
      curr_param.in = OA_PARAM_IN_COOKIE;
    else if (is_querystring)
      curr_param.in = OA_PARAM_IN_QUERYSTRING;

    if (curr_param.in == OA_PARAM_IN_HEADER) {
      rc = is_reserved_header_name(curr_param.name, &_is_reserved);
      if (rc != CDD_C_SUCCESS) {
        free_param_fields(&curr_param);
        c_mapping_free(&type_map);
        return rc;
      }
      if (_is_reserved) {
        free_param_fields(&curr_param);
        c_mapping_free(&type_map);
        continue;
      }
    }

    /* Map Types */
    if (is_querystring) {
      rc = c_cdd_strdup("application/x-www-form-urlencoded",
                        &curr_param.content_type);
      if (rc != CDD_C_SUCCESS) {
        free_param_fields(&curr_param);
        c_mapping_free(&type_map);
        return rc;
      }
      rc = set_querystring_schema_from_type_map(&curr_param, &type_map);
      if (rc != CDD_C_SUCCESS) {
        free_param_fields(&curr_param);
        c_mapping_free(&type_map);
        return rc;
      }
    } else if (type_map.kind == OA_TYPE_ARRAY) {
      curr_param.is_array = 1;
      /* Logic for items_type: c_mapper stores item type in oa_type/ref_name
       * when kind=ARRAY */
      if (type_map.oa_type) {
        rc = c_cdd_strdup(type_map.oa_type, &curr_param.items_type);
        if (rc != CDD_C_SUCCESS) {
          free_param_fields(&curr_param);
          c_mapping_free(&type_map);
          return rc;
        }
      } else {
        rc = c_cdd_strdup(type_map.ref_name, &curr_param.items_type);
        if (rc != CDD_C_SUCCESS) {
          free_param_fields(&curr_param);
          c_mapping_free(&type_map);
          return rc;
        }
      }

      rc = c_cdd_strdup("array", &curr_param.type);
      if (rc != CDD_C_SUCCESS) {
        free_param_fields(&curr_param);
        c_mapping_free(&type_map);
        return rc;
      }
    } else {
      /* Primitive / Object (if scalar param is allowed object??) usually string
       */
      /* Spec allows object parameters but they serialize weirdly. Assume string
       * representation unless primitive. */
      if (type_map.oa_type)
        rc = c_cdd_strdup(type_map.oa_type, &curr_param.type);
      else
        rc = c_cdd_strdup("string", &curr_param.type);
      if (rc != CDD_C_SUCCESS) {
        free_param_fields(&curr_param);
        c_mapping_free(&type_map);
        return rc;
      }
    }

    {
      const char *fmt_override = (dp && dp->format) ? dp->format : NULL;
      int fmt_applied = 0;
      rc = apply_format_to_schema_ref(&curr_param.schema, &type_map,
                                      fmt_override, &fmt_applied);
      if (rc != CDD_C_SUCCESS) {
        free_param_fields(&curr_param);
        c_mapping_free(&type_map);
        return rc;
      }
      if (fmt_applied) {
        if (dp && dp->item_schema)
          curr_param.item_schema_set = 1;
        else
          curr_param.schema_set = 1;
      }
    }

    if (dp) {
      if (dp->content_type) {
        if (curr_param.content_type)
          free(curr_param.content_type);
        rc = c_cdd_strdup(dp->content_type, &curr_param.content_type);
        if (rc != CDD_C_SUCCESS) {
          free_param_fields(&curr_param);
          c_mapping_free(&type_map);
          return rc;
        }
      }
      if (!curr_param.content_type) {
        if (dp->style_set) {
          enum OpenAPI_Style style = OA_STYLE_UNKNOWN;
          rc = doc_style_to_openapi(dp->style, &style);
          if (rc != CDD_C_SUCCESS) {
            free_param_fields(&curr_param);
            c_mapping_free(&type_map);
            return rc;
          }
          if (style != OA_STYLE_UNKNOWN)
            curr_param.style = style;
        }
        if (dp->explode_set) {
          curr_param.explode_set = 1;
          curr_param.explode = dp->explode ? 1 : 0;
        }
        if (dp->allow_reserved_set) {
          curr_param.allow_reserved_set = 1;
          curr_param.allow_reserved = dp->allow_reserved ? 1 : 0;
        }
        if (dp->allow_empty_value_set) {
          curr_param.allow_empty_value_set = 1;
          curr_param.allow_empty_value = dp->allow_empty_value ? 1 : 0;
        }
      }
      if (dp->deprecated_set) {
        curr_param.deprecated_set = 1;
        curr_param.deprecated = dp->deprecated ? 1 : 0;
      }
    }
    if (!curr_param.content_type && (!dp || !dp->style_set)) {
      if (curr_param.in == OA_PARAM_IN_QUERY ||
          curr_param.in == OA_PARAM_IN_COOKIE)
        curr_param.style = OA_STYLE_FORM;
      else
        curr_param.style = OA_STYLE_SIMPLE;
    }

    if (dp && dp->example) {
      rc = parse_example_any(dp->example, &curr_param.example);
      if (rc != CDD_C_SUCCESS) {
        free_param_fields(&curr_param);
        c_mapping_free(&type_map);
        return rc;
      }
      curr_param.example_set = 1;
      if (curr_param.content_type) {
        curr_param.example_location = OA_EXAMPLE_LOC_MEDIA;
      } else {
        curr_param.example_location = OA_EXAMPLE_LOC_OBJECT;
      }
    }

    rc = add_param_to_op(out_op, &curr_param);
    c_mapping_free(&type_map);
    if (rc != CDD_C_SUCCESS) {
      free_param_fields(&curr_param);
      return rc;
    }
  }

  if (doc) {
    if (doc->n_request_bodies > 0) {
      size_t rb_idx;
      for (rb_idx = 0; rb_idx < doc->n_request_bodies; ++rb_idx) {
        const struct DocRequestBody *rb = &doc->request_bodies[rb_idx];
        const char *rb_content_type =
            rb->content_type ? rb->content_type : "application/json";
        if (rb_idx == 0) {
          if (out_op->req_body.content_type)
            free(out_op->req_body.content_type);
          rc = c_cdd_strdup(rb_content_type, &out_op->req_body.content_type);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
        rc = add_request_body_media_type(out_op, rb_content_type,
                                         rb->item_schema);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (rb->example) {
          struct OpenAPI_MediaType *mt = NULL;
          rc = find_media_type_op(out_op->req_body_media_types,
                                  out_op->n_req_body_media_types,
                                  rb_content_type, &mt);
          if (rc != CDD_C_SUCCESS)
            return rc;
          rc = apply_example_to_media_type(mt, rb->example);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }

        if (doc->n_encodings > 0) {
          struct OpenAPI_MediaType *mt = NULL;
          size_t enc_i;
          rc = find_media_type_op(out_op->req_body_media_types,
                                  out_op->n_req_body_media_types,
                                  rb_content_type, &mt);
          if (rc != CDD_C_SUCCESS)
            return rc;
          for (enc_i = 0; enc_i < doc->n_encodings; ++enc_i) {
            const struct DocEncoding *d_enc = &doc->encodings[enc_i];
            struct OpenAPI_Encoding enc;
            memset(&enc, 0, sizeof(enc));

            if (d_enc->name) {
              rc = c_cdd_strdup(d_enc->name, &enc.name);
              if (rc != CDD_C_SUCCESS)
                return rc;
            }
            if (d_enc->content_type) {
              rc = c_cdd_strdup(d_enc->content_type, &enc.content_type);
              if (rc != CDD_C_SUCCESS) {
                free_encoding_fields(&enc);
                return rc;
              }
            }
            if (d_enc->style) {
              rc = doc_style_to_openapi(d_enc->style, &enc.style);
              if (rc != CDD_C_SUCCESS) {
                free_encoding_fields(&enc);
                return rc;
              }
            }

            enc.explode = d_enc->explode;
            enc.explode_set = d_enc->explode_set;
            enc.allow_reserved = d_enc->allow_reserved;
            enc.allow_reserved_set = d_enc->allow_reserved_set;

            if (d_enc->kind == 1) {
              struct OpenAPI_Encoding *new_encs = realloc(
                  mt->prefix_encoding, (mt->n_prefix_encoding + 1) *
                                           sizeof(struct OpenAPI_Encoding));
              if (!new_encs) {
                free_encoding_fields(&enc);
                return CDD_C_ERROR_MEMORY;
              }
              mt->prefix_encoding = new_encs;
              mt->prefix_encoding[mt->n_prefix_encoding++] = enc;
            } else if (d_enc->kind == 2) {
              if (!mt->item_encoding) {
                mt->item_encoding = calloc(1, sizeof(struct OpenAPI_Encoding));
                if (!mt->item_encoding) {
                  free_encoding_fields(&enc);
                  return CDD_C_ERROR_MEMORY;
                }
              }
              *mt->item_encoding = enc;
            } else {
              struct OpenAPI_Encoding *new_encs =
                  realloc(mt->encoding, (mt->n_encoding + 1) *
                                            sizeof(struct OpenAPI_Encoding));
              if (!new_encs) {
                free_encoding_fields(&enc);
                return CDD_C_ERROR_MEMORY;
              }
              mt->encoding = new_encs;
              mt->encoding[mt->n_encoding++] = enc;
            }
          }
        }
      }
    }
    if (doc->request_body_description) {
      rc = c_cdd_strdup(doc->request_body_description,
                        &out_op->req_body_description);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
        return rc;
      }
    }
    if (doc->request_body_required_set) {
      out_op->req_body_required_set = 1;
      out_op->req_body_required = doc->request_body_required ? 1 : 0;
    }
    if (doc->request_body_content_type && doc->n_request_bodies == 0) {
      if (out_op->req_body.content_type)
        free(out_op->req_body.content_type);
      rc = c_cdd_strdup(doc->request_body_content_type,
                        &out_op->req_body.content_type);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  /* 2. Responses (Doc overrides) */
  if (doc && doc->n_returns > 0) {
    /* If doc has specific returns, use them. If we identified an output body
     * earlier (200), we might merge. */
    /* Simple logic: If strict error codes documented, add them. */
    for (i = 0; i < doc->n_returns; ++i) {
      /* Check if response code already exists (e.g. 200 from output param) */
      int exists = 0;
      size_t k;
      for (k = 0; k < out_op->n_responses; ++k) {
        if (doc->returns[i].code &&
            strcmp(out_op->responses[k].code, doc->returns[i].code) == 0) {
          exists = 1;
          if (!out_op->responses[k].summary && doc->returns[i].summary) {
            rc = c_cdd_strdup(doc->returns[i].summary,
                              &out_op->responses[k].summary);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          if (doc->returns[i].description) {
            if (out_op->responses[k].description)
              free(out_op->responses[k].description);
            rc = c_cdd_strdup(doc->returns[i].description,
                              &out_op->responses[k].description);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          if (doc->returns[i].content_type) {
            rc = add_response_media_type(&out_op->responses[k],
                                         doc->returns[i].content_type,
                                         doc->returns[i].item_schema);
            if (rc != CDD_C_SUCCESS)
              return rc;
            if (!out_op->responses[k].content_type) {
              rc = c_cdd_strdup(doc->returns[i].content_type,
                                &out_op->responses[k].content_type);
              if (rc != CDD_C_SUCCESS)
                return rc;
            }
          }
          if (doc->returns[i].example) {
            rc = apply_example_to_response(&out_op->responses[k],
                                           doc->returns[i].example,
                                           doc->returns[i].content_type);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          break;
        }
      }
      if (!exists) {
        /* Add new response (likely error code) */
        struct OpenAPI_Response *new_resps = (struct OpenAPI_Response *)realloc(
            out_op->responses,
            (out_op->n_responses + 1) * sizeof(struct OpenAPI_Response));
        struct OpenAPI_Response *r;
        if (!new_resps) {
          C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
          return CDD_C_ERROR_MEMORY;
        }
        out_op->responses = new_resps;
        r = &out_op->responses[out_op->n_responses++];
        memset(r, 0, sizeof(*r));
        rc = c_cdd_strdup(doc->returns[i].code, &r->code);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (doc->returns[i].summary) {
          rc = c_cdd_strdup(doc->returns[i].summary, &r->summary);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
            return rc;
          }
        }
        if (doc->returns[i].description) {
          rc = c_cdd_strdup(doc->returns[i].description, &r->description);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
            return rc;
          }
        }
        if (doc->returns[i].content_type) {
          rc = add_response_media_type(r, doc->returns[i].content_type,
                                       doc->returns[i].item_schema);
          if (rc != CDD_C_SUCCESS)
            return rc;
          rc = c_cdd_strdup(doc->returns[i].content_type, &r->content_type);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
            return rc;
          }
        }
        if (doc->returns[i].example) {
          rc = apply_example_to_response(r, doc->returns[i].example,
                                         doc->returns[i].content_type);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
        /* Schema for error is usually generic Error struct, logic outside scope
         * here, leaves NULL */
      }
    }
  }

  if (doc && doc->n_response_headers > 0) {
    for (i = 0; i < doc->n_response_headers; ++i) {
      struct OpenAPI_Response *resp = NULL;
      rc = ensure_response_for_code(out_op, doc->response_headers[i].code,
                                    &resp);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (!resp) {
        C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
        return CDD_C_ERROR_MEMORY;
      }
      rc = add_header_to_response(resp, &doc->response_headers[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  if (doc && doc->n_links > 0) {
    for (i = 0; i < doc->n_links; ++i) {
      struct OpenAPI_Response *resp = NULL;
      rc = ensure_response_for_code(out_op, doc->links[i].code, &resp);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (!resp) {
        C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
        return CDD_C_ERROR_MEMORY;
      }
      rc = add_link_to_response(resp, &doc->links[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  if (out_op->n_responses == 0) {
    struct OpenAPI_Response *new_resps = (struct OpenAPI_Response *)realloc(
        out_op->responses, sizeof(struct OpenAPI_Response));
    struct OpenAPI_Response *r;
    if (!new_resps) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    out_op->responses = new_resps;
    r = &out_op->responses[out_op->n_responses++];
    memset(r, 0, sizeof(*r));
    rc = c_cdd_strdup("200", &r->code);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return rc;
    }
    rc = c_cdd_strdup("Success", &r->description);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return rc;
    }
  }

  /* 3. Global Tags */
  /* Heuristic: use first part of function name? e.g. api_pet_get -> "pet" */
  if (ctx->func_name && out_op->n_tags == 0) {
    char *dup_name = NULL;
    rc = c_cdd_strdup(ctx->func_name, &dup_name);
    if (rc != CDD_C_SUCCESS)
      return rc;
    {
      char *token;
      char *ctx_ptr = NULL;
/* assume snake case */
#ifdef _WIN32
      token = strtok_s(dup_name, "_", &ctx_ptr);
#else
      token = strtok_r(dup_name, "_", &ctx_ptr);
#endif /* prefix */
#ifdef _WIN32
      token = strtok_s(NULL, "_", &ctx_ptr);
#else
      token = strtok_r(NULL, "_", &ctx_ptr);
#endif /* resource or next */
      if (token) {
        out_op->tags = (char **)malloc(sizeof(char *));
        if (out_op->tags) {
          rc = c_cdd_strdup(token, &out_op->tags[0]);
          if (rc == CDD_C_SUCCESS) {
            out_op->tags[0][0] = (char)toupper(
                (unsigned char)out_op->tags[0][0]); /* Capitalize */
            out_op->n_tags = 1;
          }
        }
      }
      free(dup_name);
    }
  }

  return CDD_C_SUCCESS;
}
