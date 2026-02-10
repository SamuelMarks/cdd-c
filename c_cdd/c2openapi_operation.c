/**
 * @file c2openapi_operation.c
 * @brief Implementation of the Operation Builder.
 *
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c2openapi_operation.h"
#include "c_mapping.h"
#include "str_utils.h"

/* --- Helpers --- */

static const struct DocParam *find_doc_param(const struct DocMetadata *doc,
                                             const char *name) {
  size_t i;
  if (!doc || !name)
    return NULL;
  for (i = 0; i < doc->n_params; ++i) {
    if (doc->params[i].name && strcmp(doc->params[i].name, name) == 0) {
      return &doc->params[i];
    }
  }
  return NULL;
}

static int is_path_param(const char *route, const char *name) {
  char tmpl[128];
  if (!route || !name)
    return 0;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  sprintf_s(tmpl, sizeof(tmpl), "{%s}", name);
#else
  sprintf(tmpl, "{%s}", name);
#endif
  return strstr(route, tmpl) != NULL;
}

static int add_param_to_op(struct OpenAPI_Operation *op,
                           struct OpenAPI_Parameter *p) {
  struct OpenAPI_Parameter *new_arr;
  size_t new_count = op->n_parameters + 1;

  new_arr = (struct OpenAPI_Parameter *)realloc(
      op->parameters, new_count * sizeof(struct OpenAPI_Parameter));
  if (!new_arr)
    return ENOMEM;

  op->parameters = new_arr;
  op->parameters[op->n_parameters] = *p; /* Copy struct */
  op->n_parameters = new_count;
  return 0;
}

/* --- Type Analysis --- */

/**
 * @brief Determine if a type is a struct pointer eligible for Body.
 * Heuristic: Contains "struct", ends with "*" or "**".
 */
static int is_struct_pointer(const char *type, int *is_double_ptr) {
  const char *p;
  if (!type)
    return 0;
  if (!strstr(type, "struct "))
    return 0;

  p = strrchr(type, '*');
  if (!p)
    return 0;

  if (p > type && *(p - 1) == '*')
    *is_double_ptr = 1;
  else
    *is_double_ptr = 0;

  return 1;
}

/* --- Core Logic --- */

int c2openapi_build_operation(const struct OpBuilderContext *const ctx,
                              struct OpenAPI_Operation *const out_op) {
  const struct C2OpenAPI_ParsedSig *sig = ctx->sig;
  const struct DocMetadata *doc = ctx->doc;
  size_t i;
  int rc = 0;

  if (!ctx || !out_op || !sig)
    return EINVAL;

  /* 0. Basic Metadata */
  if (doc && doc->verb) {
    /* Map string verb to enum */
    /* Only basic check here, loader does robust parsing */
    if (strcasecmp(doc->verb, "GET") == 0)
      out_op->verb = OA_VERB_GET;
    else if (strcasecmp(doc->verb, "POST") == 0)
      out_op->verb = OA_VERB_POST;
    else if (strcasecmp(doc->verb, "PUT") == 0)
      out_op->verb = OA_VERB_PUT;
    else if (strcasecmp(doc->verb, "DELETE") == 0)
      out_op->verb = OA_VERB_DELETE;
    else if (strcasecmp(doc->verb, "PATCH") == 0)
      out_op->verb = OA_VERB_PATCH;
    else if (strcasecmp(doc->verb, "HEAD") == 0)
      out_op->verb = OA_VERB_HEAD;
    else if (strcasecmp(doc->verb, "OPTIONS") == 0)
      out_op->verb = OA_VERB_OPTIONS;
    else if (strcasecmp(doc->verb, "TRACE") == 0)
      out_op->verb = OA_VERB_TRACE;
    else if (strcasecmp(doc->verb, "QUERY") == 0)
      out_op->verb = OA_VERB_QUERY;
    else
      out_op->verb = OA_VERB_GET; /* Default */
  } else {
    /* Guess from name? e.g. "api_get_..." */
    /* Heuristics:
       api_post_X
       api_X_create  -> _create
       api_X_update  -> _update
       api_X_delete  -> _delete
    */
    if (c_cdd_str_starts_with(ctx->func_name, "api_post_") ||
        strstr(ctx->func_name, "_create"))
      out_op->verb = OA_VERB_POST;
    else if (c_cdd_str_starts_with(ctx->func_name, "api_put_") ||
             strstr(ctx->func_name, "_update"))
      out_op->verb = OA_VERB_PUT;
    else if (c_cdd_str_starts_with(ctx->func_name, "api_delete_") ||
             strstr(ctx->func_name, "_delete"))
      out_op->verb = OA_VERB_DELETE;
    else
      out_op->verb = OA_VERB_GET;
  }

  out_op->operation_id = c_cdd_strdup(ctx->func_name);
  if (!out_op->operation_id)
    return ENOMEM;
  if (doc && doc->summary) {
    out_op->summary = c_cdd_strdup(doc->summary);
    if (!out_op->summary)
      return ENOMEM;
  }

  /* 1. Argument Iteration */
  for (i = 0; i < sig->n_args; ++i) {
    const struct C2OpenAPI_ParsedArg *arg = &sig->args[i];
    const struct DocParam *dp = find_doc_param(doc, arg->name);
    struct OpenAPI_Parameter curr_param;
    struct OpenApiTypeMapping type_map;
    int is_path = 0;
    int is_body = 0;
    int is_out_ptr = 0;
    int is_querystring = 0;

    memset(&curr_param, 0, sizeof(curr_param));
    c_mapping_init(&type_map);

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
    }
    /* B. Implicit Path: Matches {name} in route */
    else if (doc && is_path_param(doc->route, arg->name)) {
      is_path = 1;
    }
    /* C. Implicit Body: "struct *" without const in POST/PUT/PATCH? */
    else {
      int is_double = 0;
      if (is_struct_pointer(arg->type, &is_double)) {
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
      }
    }

    /* Analyze Type using C Mapper */
    rc = c_mapping_map_type(arg->type, arg->name, &type_map);
    if (rc != 0) {
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
        return ENOMEM;
      }
      out_op->responses = new_resps;
      out_op->n_responses++;

      r = &out_op->responses[r_idx];
      memset(r, 0, sizeof(*r));
      r->code = c_cdd_strdup("200"); /* Default success */
      r->description = c_cdd_strdup("Success");
      if (!r->description) {
        c_mapping_free(&type_map);
        return ENOMEM;
      }

      /* Map Schema */
      r->schema.is_array = (type_map.kind == OA_TYPE_ARRAY);
      if (type_map.ref_name) {
        r->schema.ref_name = c_cdd_strdup(type_map.ref_name);
      } else if (type_map.oa_type) {
        r->schema.inline_type = c_cdd_strdup(type_map.oa_type);
      }

      c_mapping_free(&type_map);
      continue; /* Done with this arg */
    }

    if (is_body) {
      /* Request Body Population */
      out_op->req_body.content_type = c_cdd_strdup("application/json");
      out_op->req_body.is_array = (type_map.kind == OA_TYPE_ARRAY);
      /* Use ref_name if object, or type if primitive */
      if (type_map.ref_name) {
        out_op->req_body.ref_name = c_cdd_strdup(type_map.ref_name);
      } else if (type_map.oa_type) {
        out_op->req_body.inline_type = c_cdd_strdup(type_map.oa_type);
      }
      out_op->req_body_required = 1;
      out_op->req_body_required_set = 1;

      c_mapping_free(&type_map);
      continue;
    }

    /* --- Standard Parameter (Query/Path/Header/Cookie) --- */

    curr_param.name = c_cdd_strdup(arg->name);
    curr_param.required = is_path; /* Path params always required */
    if (dp && dp->required)
      curr_param.required = 1;
    if (dp && dp->description) {
      curr_param.description = c_cdd_strdup(dp->description);
      if (!curr_param.description) {
        c_mapping_free(&type_map);
        return ENOMEM;
      }
    }

    curr_param.in = is_path ? OA_PARAM_IN_PATH : OA_PARAM_IN_QUERY;
    if (dp && dp->in_loc && strcmp(dp->in_loc, "header") == 0)
      curr_param.in = OA_PARAM_IN_HEADER;
    else if (dp && dp->in_loc && strcmp(dp->in_loc, "cookie") == 0)
      curr_param.in = OA_PARAM_IN_COOKIE;
    else if (is_querystring)
      curr_param.in = OA_PARAM_IN_QUERYSTRING;

    /* Map Types */
    if (is_querystring) {
      curr_param.type = c_cdd_strdup("string");
      curr_param.content_type =
          c_cdd_strdup("application/x-www-form-urlencoded");
    } else if (type_map.kind == OA_TYPE_ARRAY) {
      curr_param.is_array = 1;
      /* Logic for items_type: c_mapper stores item type in oa_type/ref_name
       * when kind=ARRAY */
      if (type_map.oa_type)
        curr_param.items_type = c_cdd_strdup(type_map.oa_type);
      else if (type_map.ref_name)
        curr_param.items_type = c_cdd_strdup(type_map.ref_name);

      curr_param.type = c_cdd_strdup("array");
    } else {
      /* Primitive / Object (if scalar param is allowed object??) usually string
       */
      /* Spec allows object parameters but they serialize weirdly. Assume string
       * representation unless primitive. */
      if (type_map.oa_type)
        curr_param.type = c_cdd_strdup(type_map.oa_type);
      else
        curr_param.type =
            c_cdd_strdup("string"); /* Fallback for complex types in params */
    }

    rc = add_param_to_op(out_op, &curr_param);
    c_mapping_free(&type_map);
    if (rc != 0)
      return rc;
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
        if (strcmp(out_op->responses[k].code, doc->returns[i].code) == 0) {
          exists = 1;
          break;
        }
      }
      if (!exists) {
        /* Add new response (likely error code) */
        struct OpenAPI_Response *new_resps = (struct OpenAPI_Response *)realloc(
            out_op->responses,
            (out_op->n_responses + 1) * sizeof(struct OpenAPI_Response));
        struct OpenAPI_Response *r;
        if (!new_resps)
          return ENOMEM;
        out_op->responses = new_resps;
        r = &out_op->responses[out_op->n_responses++];
        memset(r, 0, sizeof(*r));
        r->code = c_cdd_strdup(doc->returns[i].code);
        if (doc->returns[i].description) {
          r->description = c_cdd_strdup(doc->returns[i].description);
          if (!r->description)
            return ENOMEM;
        }
        /* Schema for error is usually generic Error struct, logic outside scope
         * here, leaves NULL */
      }
    }
  }

  /* 3. Global Tags */
  /* Heuristic: use first part of function name? e.g. api_pet_get -> "pet" */
  {
    /* Extract resource name if pattern matches prefix_Resource_... */
    if (ctx->func_name) {
      char *dup_name = c_cdd_strdup(ctx->func_name);
      char *token;
      char *ctx_ptr = NULL;
      /* assume snake case */
      token = strtok_r(dup_name, "_", &ctx_ptr); /* prefix */
      token = strtok_r(NULL, "_", &ctx_ptr);     /* resource or next */
      if (token) {
        out_op->tags = (char **)malloc(sizeof(char *));
        if (out_op->tags) {
          out_op->tags[0] = c_cdd_strdup(token);
          if (out_op->tags[0]) {
            if (out_op->tags[0][0])
              out_op->tags[0][0] = (char)toupper(
                  (unsigned char)out_op->tags[0][0]); /* Capitalize */
            out_op->n_tags = 1;
          }
        }
      }
      free(dup_name);
    }
  }

  return 0;
}
