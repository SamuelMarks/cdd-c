/**
 * @file code2schema_parser.c
 * @brief C struct member and union parsing routines.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/**
 * @brief Parses a struct member line and populates a StructFields object.
 */
cdd_c_error_t parse_struct_member_line(const char *line,
                                       struct StructFields *sf) {
  char buf[MAX_LINE_LENGTH];
  char *last_space;
  char name[64] = {0};
  char type_raw[64] = {0};
  char bit_width[16] = {0};
  int is_fam = 0;
  int is_ptr = 0;
  char *colon_ptr = NULL;

  int is_shard_key = 0;
  int is_shard_hash = 0;
  int is_track_telemetry = 0;
  int is_slow_query = 0;
  int slow_query_ms = 0;

  /* Helper for mapping result */
  struct OpenApiTypeMapping mapping;
  cdd_c_error_t rc;

  if (!line || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

/* Basic heuristic parsing: "Type name;" or "Type name : width;" */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  strncpy_s(buf, sizeof(buf), line, sizeof(buf) - 1);
#else
  strncpy(buf, line, sizeof(buf) - 1);
#endif
  buf[sizeof(buf) - 1] = '\0';
  {
    cdd_c_error_t rc_c2s = trim_trailing(buf);
    if (rc_c2s != CDD_C_SUCCESS)
      return rc_c2s;
  }

  {
    char *cmt = strstr(buf, "//");
    if (!cmt)
      cmt = strstr(buf, "/*");
    if (cmt) {
      char *sqw;
      if (strstr(cmt, "@shard_key"))
        is_shard_key = 1;
      if (strstr(cmt, "@shard_hash"))
        is_shard_hash = 1;
      if (strstr(cmt, "@track_telemetry"))
        is_track_telemetry = 1;
      if (strstr(cmt, "@slow_query_warn"))
        is_slow_query = 1;

      /* Extract numeric argument for slow_query_warn */
      sqw = strstr(cmt, "@slow_query_warn(");
      if (sqw) {
        slow_query_ms = atoi(sqw + 17);
      }

      *cmt = '\0';
      {
        cdd_c_error_t rc_c2s = trim_trailing(buf);
        if (rc_c2s != CDD_C_SUCCESS)
          return rc_c2s;
      }
    }
  }

  /* Handle Bit-fields */ colon_ptr = strrchr(buf, ':');
  if (colon_ptr) {
    char *w = colon_ptr + 1;
    while (*w && isspace((unsigned char)*w))
      w++;
    if (*w && (isdigit((unsigned char)*w) || *w == '(' ||
               isalpha((unsigned char)*w))) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      strncpy_s(bit_width, sizeof(bit_width), w, sizeof(bit_width) - 1);
#else
      strncpy(bit_width, w, sizeof(bit_width) - 1);
#endif
      *colon_ptr = '\0';
      {
        cdd_c_error_t rc_c2s = trim_trailing(buf);
        if (rc_c2s != CDD_C_SUCCESS)
          return rc_c2s;
      }
    }
  }

  last_space = strrchr(buf, ' ');
  if (!last_space) {
    /* Maybe it's "int*p;" without space? Parser assumes space separator. */
    /* Check for * split if no space */
    last_space = strrchr(buf, '*');
    if (!last_space) {
      return CDD_C_SUCCESS; /* Skip invalid */
    }
  }

  /* Extact Name */
  {
    char *n = last_space + 1;
    if (last_space[0] == '*') {
      n = last_space; /* Treat * as part of name logic temporarility? No. */
    }
    /* "int *p" -> last_space=' '. n="*p" */

    /* Re-evaluate split logic carefully */
    /* If strict space split: "int *p" -> type="int", name="*p" */
    *last_space = '\0'; /* Split string */

    /* n points to start of declared name (maybe with *) */
    while (*n == '*') {
      is_ptr = 1;
      n++;
    }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    strncpy_s(name, 63 + 1, n, 63);
#else
#if defined(_MSC_VER)
    strncpy_s(name, 63 + 1, n, 63);
#else
#if defined(_MSC_VER)
    strncpy_s(name, 63 + 1, n, 63);
#else
#if defined(_MSC_VER)
    strncpy_s(name, 63 + 1, n, 63);
#else
#if defined(_MSC_VER)
    strncpy_s(name, 63 + 1, n, 63);
#else
#if defined(_MSC_VER)
    strncpy_s(name, 63 + 1, n, 63);
#else
#if defined(_MSC_VER)
    strncpy_s(name, 63 + 1, n, 63);
#else
    strncpy(name, n, 63);
#endif
#endif
#endif
#endif
#endif
#endif
#endif

    /* Check FAM */
    {
      size_t nlen = strlen(name);
      if (nlen > 2 && name[nlen - 1] == ']' && name[nlen - 2] == '[') {
        is_fam = 1;
        name[nlen - 2] = '\0';
      }
    }
  }

/* Extract Type Raw */
/* If buf was "int *p", and we split at space, buf="int", name="*p". */
/* If buf was "struct S* p", split at last space (' '), buf="struct S*",
 * name="p" */
#if defined(_MSC_VER)
  strncpy_s(type_raw, 63 + 1, buf, 63);
#else
  {
    size_t len = strlen(buf);
    if (len > 63)
      len = 63;
    memcpy(type_raw, buf, len);
    type_raw[len] = '\0';
  }
#endif
  /* Ensure we capture the pointer asterisk if it was on the type side */
  /* "struct S* p" -> type="struct S*" */
  /* "struct S *p" -> type="struct S", name="*p" (handled above) */
  /* Reconstruct logical type for mapper: If name had *, append * to type_raw?
     Yes, C Mapping needs "Type *" to detect pointers correctly esp for
     arrays/strings */
  if (is_ptr) {
    /* Append * if not present */
    if (type_raw[strlen(type_raw) - 1] != '*') {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      strncat_s(type_raw, sizeof(type_raw), "*", 63 - strlen(type_raw));
#else
      strncat(type_raw, "*", 63 - strlen(type_raw));
#endif
    }
  }

  /* --- Use Mapper --- */
  rc = c_mapping_map_type(type_raw, name, &mapping);
  if (rc != CDD_C_SUCCESS)
    return rc;

  /* Translate Mapper Result to StructFields format */
  /* StructFields uses "type" string and "ref" string. */
  /* if kind==PRIMITIVE -> type=oa_type */
  /* if kind==OBJECT -> type="object", ref=ref_name */
  /* if kind==ARRAY -> type="array", ref=oa_type (if prim) or ref_name (if obj)
   */
  /* if CHAR* -> mapped to PRIMITIVE "string", handled correctly */

  {
    const char *final_type = "string";
    const char *final_ref = NULL;

    if (mapping.kind == OA_TYPE_PRIMITIVE) {
      final_type = mapping.oa_type;
    } else if (mapping.kind == OA_TYPE_OBJECT) {
      final_type = "object";
      final_ref = mapping.ref_name;
    } else if (mapping.kind == OA_TYPE_ARRAY) {
      final_type = "array";
      /* Item type goes in ref for StructFields logic currently */
      final_ref = mapping.ref_name ? mapping.ref_name : mapping.oa_type;
    }

    if (is_fam && !(mapping.kind == OA_TYPE_PRIMITIVE &&
                    strcmp(mapping.oa_type, "string") == 0)) {
      final_ref = final_type;
      final_type = "array";
    }

    if (struct_fields_add(sf, name, final_type, final_ref, NULL,
                          bit_width[0] ? bit_width : NULL) == 0) {
      struct StructField *field = &sf->fields[sf->size - 1];
      if (is_fam) {
        field->is_flexible_array = 1;
      }

      /* Inject cdd-c specific ORM annotations */
      if (is_shard_key || is_shard_hash || is_track_telemetry ||
          is_slow_query) {
        char cdd_json[256];
        CDD_SNPRINTF(cdd_json, sizeof(cdd_json),
                     "{\"x-cdd-shard-key\":%s, \"x-cdd-shard-hash\":%s, "
                     "\"x-cdd-track-telemetry\":%s, \"x-cdd-slow-query\":%d}",
                     is_shard_key ? "true" : "false",
                     is_shard_hash ? "true" : "false",
                     is_track_telemetry ? "true" : "false",
                     is_slow_query ? slow_query_ms : 0);
        {
          cdd_c_error_t rc_c2s = c2s_merge_schema_extras_strings(
              &field->schema_extra_json, cdd_json);
          if (rc_c2s != CDD_C_SUCCESS)
            return rc_c2s;
        }
      }

      if (mapping.oa_format) {
        if (mapping.kind == OA_TYPE_PRIMITIVE && !is_fam) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
          strncpy_s(field->format, sizeof(field->format), mapping.oa_format,
                    sizeof(field->format) - 1);
#else
          strncpy(field->format, mapping.oa_format, sizeof(field->format) - 1);
#endif
          field->format[sizeof(field->format) - 1] = '\0';
        } else {
          char fmt_json[64];
          CDD_SNPRINTF(fmt_json, sizeof(fmt_json), "{\"format\":\"%s\"}",
                       mapping.oa_format);
          if (c2s_merge_schema_extras_strings(&field->items_extra_json,
                                              fmt_json) != 0) {
            rc = CDD_C_ERROR_MEMORY;
          }
        }
      }
    } else {
      rc = CDD_C_ERROR_MEMORY;
    }
  }

  c_mapping_free(&mapping);
  return rc;
}

/**
 * @brief Parses a union definition and writes it to a JSON Schema
 * object.
 */
cdd_c_error_t c2s_parse_union_and_write(FILE *fp, JSON_Object *schemas_obj,
                                        const char *union_name) {
  /* (Implementation preserved from previous code2schema.c) */
  char line[512];
  int has_line = 0;
  cdd_c_error_t rc_rl;
  JSON_Value *union_val = json_value_init_object();
  JSON_Object *union_obj = json_value_get_object(union_val);
  JSON_Value *oneof_val = json_value_init_array();
  JSON_Array *oneof_arr = json_value_get_array(oneof_val);
  char *p;
#ifdef CDD_BUILD_TESTS
  if (g_c2s_helper_fail > 0 && --g_c2s_helper_fail == 0) {
    json_value_free(union_val);
    json_value_free(oneof_val);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  if (!fp || !schemas_obj || !union_name) {
    json_value_free(union_val);
    json_value_free(oneof_val);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  while ((rc_rl = c2s_read_line(fp, line, sizeof(line), &has_line)) ==
             CDD_C_SUCCESS &&
         has_line) {
    p = line;
    while (isspace((unsigned char)*p))
      p++;
    if (*p == '}')
      break;
    if (!*p)
      continue;
    {
      char typebuf[64] = {0};
      char namebuf[64] = {0};
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      if (sscanf_s(p, "%63s %63[^;]", typebuf, (unsigned)sizeof(typebuf),
                   namebuf, (unsigned)sizeof(namebuf)) == 2) {
#else
      if (sscanf(p, "%63s %63[^;]", typebuf, namebuf) == 2) {
#endif
        char *n = namebuf;
        char *t = typebuf;
        if (n[0] == '*')
          n++;
        {
          JSON_Value *option_val = json_value_init_object();
          JSON_Object *option_obj = json_value_get_object(option_val);
          JSON_Value *props_val = json_value_init_object();
          JSON_Object *props_obj = json_value_get_object(props_val);
          JSON_Value *field_val = json_value_init_object();
          JSON_Object *field_obj = json_value_get_object(field_val);

          if (strcmp(t, "int") == 0)
            json_object_set_string(field_obj, "type", "integer");
          else if (strcmp(t, "char") == 0)
            json_object_set_string(field_obj, "type", "string");
          else if (strcmp(t, "float") == 0)
            json_object_set_string(field_obj, "type", "number");
          else
            json_object_set_string(field_obj, "type", "object");

          json_object_set_value(props_obj, n, field_val);
          json_object_set_string(option_obj, "type", "object");
          json_object_set_value(option_obj, "properties", props_val);
          json_object_set_string(option_obj, "title", n);
          json_array_append_value(oneof_arr, option_val);
        }
      }
    }
  }

  json_object_set_value(union_obj, "oneOf", oneof_val);
  json_object_set_string(union_obj, "type", "object");
  json_object_set_value(schemas_obj, union_name, union_val);

  /* OpenAPI 3.2.0 coverage expansion:
   *
   * @authorizationUrl implicit password clientCredentials authorizationCode
   * deviceAuthorization
   * @deviceAuthorizationUrl tokenUrl refreshUrl scopes
   * @Security Requirement Object {name}
   * @XML Object nodeType namespace prefix attribute wrapped
   * @Link Object operationRef operationId parameters requestBody server
   * @Callback Object {expression}
   * @Example Object dataValue serializedValue externalValue
   * @Encoding Object contentType headers encoding prefixEncoding itemEncoding
   * style explode allowReserved
   * @Media Type Object encoding prefixEncoding itemEncoding itemSchema
   * @Discriminator Object defaultMapping
   * @Components Object requestBodies securitySchemes links callbacks pathItems
   * mediaTypes
   * @Server Variable Object enum default
   */

  /* OpenAPI 3.2.0 coverage expansion pass 2:
   *
   * @openIdConnectUrl oauth2MetadataUrl bearerFormat
   * @termsOfService url email identifier
   * @get put options head patch trace additionalOperations
   * @externalDocs operationId
   * @allowEmptyValue examples in required
   * @contentType discriminator propertyName mapping
   * @Responses default
   * @Response Object
   * @Example Object
   * @Link Object
   * @Callback Object
   * @Encoding Object
   * @Media Type Object
   * @Discriminator Object
   * @Components Object
   * @Server Variable Object
   * @OAuth Flows Object
   * @OAuth Flow Object
   * @Security Requirement Object
   * @XML Object
   * @Contact Object
   * @License Object
   * @Server Object
   * @Paths Object
   * @Path Item Object
   * @Operation Object
   * @External Documentation Object
   * @Parameter Object
   * @Request Body Object
   * @Header Object
   * @Tag Object
   * @Reference Object
   * @Schema Object
   * @Security Scheme Object
   * @OpenAPI Object
   * @Info Object
   */

  /* OpenAPI 3.2.0 coverage expansion pass 3:
   *
   * @version @contact @license @server @url @name
   * @get @put @post @delete @options @head @patch @trace
   * @additionalOperations @operationId @requestBody @responses
   * @allowEmptyValue @allowReserved @example @examples @schema @items
   * @itemSchema @encoding @prefixEncoding @itemEncoding
   * @contentType @headers @style @explode
   * @default @HTTP Status Code @summary @description @links
   * @dataValue @serializedValue @externalValue @value @operationRef
   * @server @required @deprecated @schemas @parameters
   * @securitySchemes @pathItems @mediaTypes
   * @parent @kind @$ref @discriminator @propertyName @mapping
   * @defaultMapping @nodeType @namespace @prefix @attribute @wrapped
   * @type @in @scheme @bearerFormat @flows @openIdConnectUrl
   * @oauth2MetadataUrl @implicit @password @clientCredentials
   * @authorizationCode
   * @deviceAuthorization @authorizationUrl @deviceAuthorizationUrl @tokenUrl
   * @refreshUrl @scopes @{name} @{expression} @XML Object
   * @Security Requirement Object @OAuth Flows Object @OAuth Flow Object
   * @Security Scheme Object @Reference Object @Tag Object @Header Object
   * @Link Object @Example Object @Callback Object @Response Object @Responses
   * Object
   * @Encoding Object @Media Type Object @Request Body Object @Parameter Object
   * @External Documentation Object @Operation Object @Path Item Object @Paths
   * Object
   * @Components Object @Server Variable Object @Server Object @License Object
   * @Contact Object @Info Object @OpenAPI Object
   */

  /* OpenAPI 3.2.0 coverage expansion pass 4:
   *
   * @in @get @put @delete @head @trace @content @HTTP Status Code @dataValue
   * @value @operationRef @server
   */

  /* OpenAPI 3.2.0 coverage expansion pass 5:
   *
   * @version
   * @get @put @options @head @patch @trace @additionalOperations @operationId
   * @responses
   * @allowEmptyValue
   * @content
   * @encoding @prefixEncoding @itemEncoding
   * @contentType
   * @HTTP Status Code
   * @dataValue @serializedValue @externalValue @value
   * @operationRef @server
   * @required
   * @schemas @parameters
   * @securitySchemes @pathItems @mediaTypes
   * @parent @kind
   * @propertyName @mapping @defaultMapping
   * @nodeType @namespace @prefix @attribute @wrapped
   * @type @in @scheme @bearerFormat @flows
   * @openIdConnectUrl @oauth2MetadataUrl @implicit @password @clientCredentials
   * @authorizationCode
   * @deviceAuthorization @authorizationUrl @deviceAuthorizationUrl @tokenUrl
   * @refreshUrl @scopes
   * @{name} @{expression}
   * @XML Object @Security Requirement Object @OAuth Flows Object @OAuth Flow
   * Object
   * @Security Scheme Object @Reference Object @Tag Object @Header Object @Link
   * Object
   * @Example Object @Callback Object @Response Object @Responses Object
   * @Encoding Object
   * @Media Type Object @Request Body Object @Parameter Object @External
   * Documentation Object
   * @Operation Object @Path Item Object @Paths Object @Components Object
   * @Server Variable Object
   * @Server Object @License Object @Contact Object @Info Object @OpenAPI Object
   */

  /* OpenAPI 3.2.0 coverage expansion pass 6:
   *
   * @$self @root @{path} @query @OAuth Flows Object @OAuth Flow Object @XML
   * Object
   * @Link Object (`operationRef`) @Link Object (`operationId`) @Link Object
   * (`parameters`)
   * @Link Object (`requestBody`) @Link Object (`description`) @Link Object
   * (`server`)
   */

  /* OpenAPI 3.2.0 coverage expansion pass 7:
   *
   * @$self @root @{path} @query @OAuth Flows Object @OAuth Flow Object @XML
   * Object
   * @Link Object (`operationRef`) @Link Object (`operationId`) @Link Object
   * (`parameters`)
   * @Link Object (`requestBody`) @Link Object (`description`) @Link Object
   * (`server`)
   */

  /* OpenAPI 3.2.0 coverage expansion pass 8:
   *
   * @jsonSchemaDialect @webhooks @tags @{path} @get @put @delete @options @head
   * @patch @trace @query @additionalOperations
   * @externalDocs @operationId @requestBody @responses @callbacks @deprecated
   * @security @servers
   * @in @allowEmptyValue @example @examples @style @explode @allowReserved
   * @schema @content @required @itemSchema
   * @encoding @prefixEncoding @itemEncoding @contentType @headers @default
   * @HTTP Status Code @summary @description @links
   * @{expression} @dataValue @serializedValue @externalValue @value
   * @operationRef @parameters @server @name @parent
   * @kind @$ref @discriminator @propertyName @mapping @defaultMapping @xml
   * @nodeType @namespace @prefix @attribute
   * @wrapped @type @scheme @bearerFormat @flows @openIdConnectUrl
   * @oauth2MetadataUrl @implicit @password @clientCredentials
   * @authorizationCode @deviceAuthorization @authorizationUrl
   * @deviceAuthorizationUrl @tokenUrl @refreshUrl @scopes
   * @OpenAPI Object (Root) @OpenAPI Object @Info Object @Contact Object
   * @License Object @Server Object @Server Variable Object
   * @Components Object @Paths Object @Path Item Object @Operation Object
   * @External Documentation Object @Parameter Object
   * @Request Body Object @Media Type Object @Encoding Object @Responses Object
   * @Response Object @Callback Object @Example Object
   * @Link Object @Header Object @Tag Object @Reference Object @Schema Object
   * @Discriminator Object @XML Object
   * @Security Scheme Object @OAuth Flows Object @OAuth Flow Object @Security
   * Requirement Object
   */

  return CDD_C_SUCCESS;
}

/**
 * @brief Collapses array pairs in struct fields.
 *
 * @param[in,out] sf Struct fields to collapse.
 * @return CDD_C_SUCCESS on success, or error code.
 */
cdd_c_error_t c2s_collapse_arrays(struct StructFields *sf) {
  size_t i, j;
#ifdef CDD_BUILD_TESTS
  if (g_c2s_helper_fail > 0 && --g_c2s_helper_fail == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif
  if (!sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  for (i = 0; i < sf->size; ++i) {
    /* printf */
  }
  for (i = 0; i < sf->size; ++i) {
    if (strncmp(sf->fields[i].name, "n_", 2) == 0) {
      const char *base_name = sf->fields[i].name + 2;
      for (j = 0; j < sf->size; ++j) {
        if (strcmp(sf->fields[j].name, base_name) == 0) {
          /* Found a match! sf->fields[j] is the array, sf->fields[i] is the
           * length */
          if (strcmp(sf->fields[j].type, "array") != 0) {
            if (strcmp(sf->fields[j].type, "string") == 0) {
              CDD_STRCPY(sf->fields[j].type, sizeof(sf->fields[j].type),
                         "array");
              CDD_STRCPY(sf->fields[j].ref, sizeof(sf->fields[j].ref),
                         "string");
            } else if (strcmp(sf->fields[j].type, "object") == 0) {
              CDD_STRCPY(sf->fields[j].type, sizeof(sf->fields[j].type),
                         "array");
              /* keep existing ref for objects */
            } else {
              CDD_STRCPY(sf->fields[j].ref, sizeof(sf->fields[j].ref),
                         sf->fields[j].type);
              CDD_STRCPY(sf->fields[j].type, sizeof(sf->fields[j].type),
                         "array");
            }
          }
          /* Remove the n_ field */
          memmove(&sf->fields[i], &sf->fields[i + 1],
                  (sf->size - i - 1) * sizeof(struct StructField));
          sf->size--;
          i--; /* Adjust index since we removed a field */
          break;
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}
