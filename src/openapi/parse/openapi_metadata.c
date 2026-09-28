/**
 * @file openapi_metadata.c
 * @brief Metadata, tags, discriminator, XML, and server parsing.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Parses info from the given input.
 */
cdd_c_error_t parse_info(const JSON_Object *root_obj,
                         struct OpenAPI_Spec *out) {
  char *_ast_strdup_146 = NULL;
  char *_ast_strdup_147 = NULL;
  char *_ast_strdup_148 = NULL;
  char *_ast_strdup_149 = NULL;
  char *_ast_strdup_150 = NULL;
  char *_ast_strdup_151 = NULL;
  char *_ast_strdup_152 = NULL;
  char *_ast_strdup_153 = NULL;
  char *_ast_strdup_154 = NULL;
  char *_ast_strdup_155 = NULL;
  char *_ast_strdup_156 = NULL;
  const JSON_Object *info_obj;
  const JSON_Object *contact_obj;
  const JSON_Object *license_obj;
  const char *val;

  if (!root_obj || !out)
    return CDD_C_SUCCESS;

  info_obj = json_object_get_object(root_obj, "info");
  if (!info_obj)
    return CDD_C_SUCCESS;

  val = json_object_get_string(info_obj, "title");
  if (val) {
    out->info.title = (c_cdd_strdup(val, &_ast_strdup_146), _ast_strdup_146);
    if (!out->info.title)
      return CDD_C_ERROR_MEMORY;
  }
  val = json_object_get_string(info_obj, "summary");
  if (val) {
    out->info.summary = (c_cdd_strdup(val, &_ast_strdup_147), _ast_strdup_147);
    if (!out->info.summary)
      return CDD_C_ERROR_MEMORY;
  }
  val = json_object_get_string(info_obj, "description");
  if (val) {
    out->info.description =
        (c_cdd_strdup(val, &_ast_strdup_148), _ast_strdup_148);
    if (!out->info.description)
      return CDD_C_ERROR_MEMORY;
  }
  val = json_object_get_string(info_obj, "termsOfService");
  if (val) {
    out->info.terms_of_service =
        (c_cdd_strdup(val, &_ast_strdup_149), _ast_strdup_149);
    if (!out->info.terms_of_service)
      return CDD_C_ERROR_MEMORY;
  }
  val = json_object_get_string(info_obj, "version");
  if (val) {
    out->info.version = (c_cdd_strdup(val, &_ast_strdup_150), _ast_strdup_150);
    if (!out->info.version)
      return CDD_C_ERROR_MEMORY;
  }
  {
    cdd_c_error_t _rc =
        collect_extensions(info_obj, &out->info.extensions_json);
    if (_rc != CDD_C_SUCCESS)
      return _rc;
  }

  contact_obj = json_object_get_object(info_obj, "contact");
  if (contact_obj) {
    val = json_object_get_string(contact_obj, "name");
    if (val) {
      out->info.contact.name =
          (c_cdd_strdup(val, &_ast_strdup_151), _ast_strdup_151);
      if (!out->info.contact.name)
        return CDD_C_ERROR_MEMORY;
    }
    val = json_object_get_string(contact_obj, "url");
    if (val) {
      out->info.contact.url =
          (c_cdd_strdup(val, &_ast_strdup_152), _ast_strdup_152);
      if (!out->info.contact.url)
        return CDD_C_ERROR_MEMORY;
    }
    val = json_object_get_string(contact_obj, "email");
    if (val) {
      out->info.contact.email =
          (c_cdd_strdup(val, &_ast_strdup_153), _ast_strdup_153);
      if (!out->info.contact.email)
        return CDD_C_ERROR_MEMORY;
    }
    {
      cdd_c_error_t _rc =
          collect_extensions(contact_obj, &out->info.contact.extensions_json);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
  }

  license_obj = json_object_get_object(info_obj, "license");
  if (license_obj) {
    const char *lic_name = json_object_get_string(license_obj, "name");
    const char *lic_identifier =
        json_object_get_string(license_obj, "identifier");
    const char *lic_url = json_object_get_string(license_obj, "url");

    if (!lic_name || !*lic_name)
      return CDD_C_ERROR_INVALID_ARGUMENT;

    if (lic_identifier && lic_url)
      return CDD_C_ERROR_INVALID_ARGUMENT;

    out->info.license.name =
        (c_cdd_strdup(lic_name, &_ast_strdup_154), _ast_strdup_154);
    if (!out->info.license.name)
      return CDD_C_ERROR_MEMORY;
    if (lic_identifier) {
      out->info.license.identifier =
          (c_cdd_strdup(lic_identifier, &_ast_strdup_155), _ast_strdup_155);
      if (!out->info.license.identifier)
        return CDD_C_ERROR_MEMORY;
    }
    if (lic_url) {
      out->info.license.url =
          (c_cdd_strdup(lic_url, &_ast_strdup_156), _ast_strdup_156);
      if (!out->info.license.url)
        return CDD_C_ERROR_MEMORY;
    }
    {
      cdd_c_error_t _rc =
          collect_extensions(license_obj, &out->info.license.extensions_json);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses external docs from the given input.
 */
cdd_c_error_t parse_external_docs(const JSON_Object *obj,
                                  struct OpenAPI_ExternalDocs *out) {
  char *_ast_strdup_157 = NULL;
  char *_ast_strdup_158 = NULL;
  const char *desc;
  const char *url;

  if (!obj || !out)
    return CDD_C_SUCCESS;

  desc = json_object_get_string(obj, "description");
  if (desc) {
    out->description = (c_cdd_strdup(desc, &_ast_strdup_157), _ast_strdup_157);
    if (!out->description)
      return CDD_C_ERROR_MEMORY;
  }
  url = json_object_get_string(obj, "url");
  if (!url || !*url)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  out->url = (c_cdd_strdup(url, &_ast_strdup_158), _ast_strdup_158);
  if (!out->url)
    return CDD_C_ERROR_MEMORY;
  {
    cdd_c_error_t _rc = collect_extensions(obj, &out->extensions_json);
    if (_rc != CDD_C_SUCCESS)
      return _rc;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses discriminator object from the given input.
 */
cdd_c_error_t parse_discriminator_object(const JSON_Object *obj,
                                         struct OpenAPI_Discriminator *out) {
  char *_ast_strdup_159 = NULL;
  char *_ast_strdup_160 = NULL;
  char *_ast_strdup_161 = NULL;
  char *_ast_strdup_162 = NULL;
  const char *prop;
  const char *default_mapping;
  const JSON_Object *mapping_obj;
  size_t i, count, used;

  if (!obj || !out)
    return CDD_C_SUCCESS;

  prop = json_object_get_string(obj, "propertyName");
  if (prop) {
    out->property_name =
        (c_cdd_strdup(prop, &_ast_strdup_159), _ast_strdup_159);
    if (!out->property_name)
      return CDD_C_ERROR_MEMORY;
  }

  default_mapping = json_object_get_string(obj, "defaultMapping");
  if (default_mapping) {
    out->default_mapping =
        (c_cdd_strdup(default_mapping, &_ast_strdup_160), _ast_strdup_160);
    if (!out->default_mapping)
      return CDD_C_ERROR_MEMORY;
  }

  mapping_obj = json_object_get_object(obj, "mapping");
  if (mapping_obj) {
    count = json_object_get_count(mapping_obj);
    used = 0;
    for (i = 0; i < count; ++i) {
      const char *name = json_object_get_name(mapping_obj, i);
      if (!name || strncmp(name, "x-", 2) == 0)
        continue;
      used++;
    }
    if (used > 0) {
      out->mapping = (struct OpenAPI_DiscriminatorMap *)calloc(
          used, sizeof(struct OpenAPI_DiscriminatorMap));
      if (!out->mapping)
        return CDD_C_ERROR_MEMORY;
      out->n_mapping = used;
      used = 0;
      for (i = 0; i < count; ++i) {
        const char *name = json_object_get_name(mapping_obj, i);
        const char *val;
        if (!name || strncmp(name, "x-", 2) == 0)
          continue;
        val = json_object_get_string(mapping_obj, name);
        if (!val)
          continue;
        out->mapping[used].value =
            (c_cdd_strdup(name, &_ast_strdup_161), _ast_strdup_161);
        if (!out->mapping[used].value)
          return CDD_C_ERROR_MEMORY;
        out->mapping[used].schema =
            (c_cdd_strdup(val, &_ast_strdup_162), _ast_strdup_162);
        if (!out->mapping[used].schema)
          return CDD_C_ERROR_MEMORY;
        used++;
      }
      out->n_mapping = used;
    }
  }

  {
    cdd_c_error_t _rc = collect_extensions(obj, &out->extensions_json);
    if (_rc != CDD_C_SUCCESS)
      return _rc;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses xml object from the given input.
 */
cdd_c_error_t parse_xml_object(const JSON_Object *obj,
                               struct OpenAPI_Xml *out) {
  enum OpenAPI_XmlNodeType _ast_parse_xml_node_type_40;
  char *_ast_strdup_163 = NULL;
  char *_ast_strdup_164 = NULL;
  char *_ast_strdup_165 = NULL;
  const char *node_type;
  const char *name;
  const char *ns;
  const char *prefix;

  if (!obj || !out)
    return CDD_C_SUCCESS;

  node_type = json_object_get_string(obj, "nodeType");
  if (node_type) {
    out->node_type =
        (parse_xml_node_type(node_type, &_ast_parse_xml_node_type_40),
         _ast_parse_xml_node_type_40);
    out->node_type_set = 1;
  }

  name = json_object_get_string(obj, "name");
  if (name) {
    out->name = (c_cdd_strdup(name, &_ast_strdup_163), _ast_strdup_163);
    if (!out->name)
      return CDD_C_ERROR_MEMORY;
  }

  ns = json_object_get_string(obj, "namespace");
  if (ns) {
    out->namespace_uri = (c_cdd_strdup(ns, &_ast_strdup_164), _ast_strdup_164);
    if (!out->namespace_uri)
      return CDD_C_ERROR_MEMORY;
  }

  prefix = json_object_get_string(obj, "prefix");
  if (prefix) {
    out->prefix = (c_cdd_strdup(prefix, &_ast_strdup_165), _ast_strdup_165);
    if (!out->prefix)
      return CDD_C_ERROR_MEMORY;
  }

  if (json_object_has_value(obj, "attribute")) {
    out->attribute_set = 1;
    out->attribute = json_object_get_boolean(obj, "attribute") == 1;
  }

  if (json_object_has_value(obj, "wrapped")) {
    out->wrapped_set = 1;
    out->wrapped = json_object_get_boolean(obj, "wrapped") == 1;
  }

  {
    cdd_c_error_t _rc = collect_extensions(obj, &out->extensions_json);
    if (_rc != CDD_C_SUCCESS)
      return _rc;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses tags from the given input.
 */
cdd_c_error_t parse_tags(const JSON_Object *root_obj,
                         struct OpenAPI_Spec *out) {
  char *_ast_strdup_166 = NULL;
  char *_ast_strdup_167 = NULL;
  char *_ast_strdup_168 = NULL;
  char *_ast_strdup_169 = NULL;
  char *_ast_strdup_170 = NULL;
  const JSON_Array *tags_arr;
  size_t count, i;

  if (!root_obj || !out)
    return CDD_C_SUCCESS;

  tags_arr = json_object_get_array(root_obj, "tags");
  if (!tags_arr)
    return CDD_C_SUCCESS;

  count = json_array_get_count(tags_arr);
  if (count == 0)
    return CDD_C_SUCCESS;

  out->tags = (struct OpenAPI_Tag *)calloc(count, sizeof(struct OpenAPI_Tag));
  if (!out->tags)
    return CDD_C_ERROR_MEMORY;
  out->n_tags = count;

  for (i = 0; i < count; ++i) {
    const JSON_Object *tag_obj = json_array_get_object(tags_arr, i);
    const char *name = json_object_get_string(tag_obj, "name");
    const char *summary = json_object_get_string(tag_obj, "summary");
    const char *description = json_object_get_string(tag_obj, "description");
    const char *parent = json_object_get_string(tag_obj, "parent");
    const char *kind = json_object_get_string(tag_obj, "kind");
    const JSON_Object *ext = json_object_get_object(tag_obj, "externalDocs");

    if (!name || !*name)
      return CDD_C_ERROR_INVALID_ARGUMENT;

    {
      size_t k;
      for (k = 0; k < i; ++k) {
        if (out->tags[k].name && strcmp(out->tags[k].name, name) == 0)
          return CDD_C_ERROR_INVALID_ARGUMENT;
      }
    }
    out->tags[i].name = (c_cdd_strdup(name, &_ast_strdup_166), _ast_strdup_166);
    if (!out->tags[i].name)
      return CDD_C_ERROR_MEMORY;
    if (summary) {
      out->tags[i].summary =
          (c_cdd_strdup(summary, &_ast_strdup_167), _ast_strdup_167);
      if (!out->tags[i].summary)
        return CDD_C_ERROR_MEMORY;
    }
    if (description) {
      out->tags[i].description =
          (c_cdd_strdup(description, &_ast_strdup_168), _ast_strdup_168);
      if (!out->tags[i].description)
        return CDD_C_ERROR_MEMORY;
    }
    if (parent) {
      out->tags[i].parent =
          (c_cdd_strdup(parent, &_ast_strdup_169), _ast_strdup_169);
      if (!out->tags[i].parent)
        return CDD_C_ERROR_MEMORY;
    }
    if (kind) {
      out->tags[i].kind =
          (c_cdd_strdup(kind, &_ast_strdup_170), _ast_strdup_170);
      if (!out->tags[i].kind)
        return CDD_C_ERROR_MEMORY;
    }
    if (ext) {
      cdd_c_error_t rc = parse_external_docs(ext, &out->tags[i].external_docs);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    {
      cdd_c_error_t _rc =
          collect_extensions(tag_obj, &out->tags[i].extensions_json);
      if (_rc != CDD_C_SUCCESS)
        return _rc;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the tag index by name operation.
 */
cdd_c_error_t tag_index_by_name(const struct OpenAPI_Spec *spec,
                                const char *name, size_t *out_idx) {
  size_t i;
  if (!spec || !name || !out_idx)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  for (i = 0; i < spec->n_tags; ++i) {
    if (spec->tags[i].name && strcmp(spec->tags[i].name, name) == 0) {
      *out_idx = i;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_ERROR_NOT_FOUND;
}

/**
 * @brief Executes the detect tag cycle operation.
 */
cdd_c_error_t detect_tag_cycle(const struct OpenAPI_Spec *spec, size_t idx,
                               int *state) {
  size_t parent_idx = 0;
  const char *parent;
  if (!spec || !state || idx >= spec->n_tags)
    return CDD_C_SUCCESS;
  if (state[idx] == 1)
    return CDD_C_ERROR_UNKNOWN;
  if (state[idx] == 2)
    return CDD_C_SUCCESS;
  state[idx] = 1;
  parent = spec->tags[idx].parent;
  if (parent && *parent) {
    if (tag_index_by_name(spec, parent, &parent_idx) == CDD_C_SUCCESS) {
      if (detect_tag_cycle(spec, parent_idx, state))
        return CDD_C_ERROR_UNKNOWN;
    }
  }
  state[idx] = 2;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the validate tag parents operation.
 */
cdd_c_error_t validate_tag_parents(const struct OpenAPI_Spec *spec) {
  size_t i;
  int *state;
  if (!spec || !spec->tags || spec->n_tags == 0)
    return CDD_C_SUCCESS;

  state = (int *)calloc(spec->n_tags, sizeof(int));
  if (!state)
    return CDD_C_ERROR_MEMORY;
  for (i = 0; i < spec->n_tags; ++i) {
    if (detect_tag_cycle(spec, i, state)) {
      free(state);
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
  }
  free(state);
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the server variable defined operation.
 */
cdd_c_error_t server_variable_defined(const struct OpenAPI_Server *srv,
                                      const char *name) {
  size_t i;
  if (!srv || !name || !srv->variables)
    return CDD_C_SUCCESS;
  for (i = 0; i < srv->n_variables; ++i) {
    if (srv->variables[i].name && strcmp(srv->variables[i].name, name) == 0)
      return CDD_C_ERROR_UNKNOWN;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the server variable seen operation.
 */
cdd_c_error_t server_variable_seen(char **seen, size_t seen_count,
                                   const char *name) {
  size_t i;
  if (!seen || !name)
    return CDD_C_SUCCESS;
  for (i = 0; i < seen_count; ++i) {
    if (seen[i] && strcmp(seen[i], name) == 0)
      return CDD_C_ERROR_UNKNOWN;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the validate server url variables operation.
 */
cdd_c_error_t validate_server_url_variables(const struct OpenAPI_Server *srv) {
  const char *url;
  size_t i;
  char **seen = NULL;
  size_t seen_count = 0;
  size_t seen_cap = 0;

  if (!srv || !srv->url)
    return CDD_C_SUCCESS;

  url = srv->url;
  for (i = 0; url[i]; ++i) {
    if (url[i] == '{') {
      const char *end = strchr(url + i + 1, '}');
      size_t len;
      char *name;
      char **tmp;
      if (!end)
        goto invalid;
      len = (size_t)(end - (url + i + 1));
      if (len == 0)
        goto invalid;
      name = (char *)(size_t)malloc(len + 1);
      if (!name)
        goto oom;
      memcpy(name, url + i + 1, len);
      name[len] = '\0';
      if (!server_variable_defined(srv, name)) {
        free(name);
        goto invalid;
      }
      if (server_variable_seen(seen, seen_count, name)) {
        free(name);
        goto invalid;
      }
      if (seen_count == seen_cap) {
        size_t new_cap = seen_cap ? seen_cap * 2 : 4;
        tmp = (char **)realloc(seen, new_cap * sizeof(char *));
        if (!tmp) {
          free(name);
          goto oom;
        }
        seen = tmp;
        seen_cap = new_cap;
      }
      seen[seen_count++] = name;
      i = (size_t)(end - url);
      continue;
    }
    if (url[i] == '}')
      goto invalid;
  }

  for (i = 0; i < seen_count; ++i)
    free(seen[i]);
  free(seen);
  return CDD_C_SUCCESS;

invalid:
  for (i = 0; i < seen_count; ++i)
    free(seen[i]);
  free(seen);
  return CDD_C_ERROR_INVALID_ARGUMENT;

oom:
  for (i = 0; i < seen_count; ++i)
    free(seen[i]);
  free(seen);
  return CDD_C_ERROR_MEMORY;
}

/**
 * @brief Parses server object from the given input.
 */
cdd_c_error_t parse_server_object(const JSON_Object *srv_obj,
                                  struct OpenAPI_Server *out_srv) {
  char *_ast_strdup_171 = NULL;
  char *_ast_strdup_172 = NULL;
  char *_ast_strdup_173 = NULL;
  char *_ast_strdup_174 = NULL;
  char *_ast_strdup_175 = NULL;
  char *_ast_strdup_176 = NULL;
  char *_ast_strdup_177 = NULL;
  const char *url;
  const char *desc;
  const char *name;
  const JSON_Object *vars;

  if (!srv_obj || !out_srv)
    return CDD_C_SUCCESS;

  url = json_object_get_string(srv_obj, "url");
  desc = json_object_get_string(srv_obj, "description");
  name = json_object_get_string(srv_obj, "name");
  vars = json_object_get_object(srv_obj, "variables");

  if (!url || !*url)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (url && url_has_query_or_fragment(url))
    return CDD_C_ERROR_INVALID_ARGUMENT;

  out_srv->url = (c_cdd_strdup(url, &_ast_strdup_171), _ast_strdup_171);
  if (!out_srv->url)
    return CDD_C_ERROR_MEMORY;

  if (desc) {
    out_srv->description =
        (c_cdd_strdup(desc, &_ast_strdup_172), _ast_strdup_172);
    if (!out_srv->description)
      return CDD_C_ERROR_MEMORY;
  }

  if (name) {
    out_srv->name = (c_cdd_strdup(name, &_ast_strdup_173), _ast_strdup_173);
    if (!out_srv->name)
      return CDD_C_ERROR_MEMORY;
  }
  {
    cdd_c_error_t _rc = collect_extensions(srv_obj, &out_srv->extensions_json);
    if (_rc != CDD_C_SUCCESS)
      return _rc;
  }

  if (vars) {
    size_t vcount = json_object_get_count(vars);
    size_t v;
    if (vcount > 0) {
      out_srv->variables = (struct OpenAPI_ServerVariable *)calloc(
          vcount, sizeof(struct OpenAPI_ServerVariable));
      if (!out_srv->variables)
        return CDD_C_ERROR_MEMORY;
      out_srv->n_variables = vcount;
      for (v = 0; v < vcount; ++v) {
        const char *vname = json_object_get_name(vars, v);
        const JSON_Object *v_obj =
            json_value_get_object(json_object_get_value_at(vars, v));
        struct OpenAPI_ServerVariable *curr = &out_srv->variables[v];
        if (vname) {
          curr->name = (c_cdd_strdup(vname, &_ast_strdup_174), _ast_strdup_174);
          if (!curr->name)
            return CDD_C_ERROR_MEMORY;
        }
        if (v_obj) {
          const char *def_val = json_object_get_string(v_obj, "default");
          const char *v_desc = json_object_get_string(v_obj, "description");
          const JSON_Array *enum_arr = json_object_get_array(v_obj, "enum");
          if (!def_val || !*def_val)
            return CDD_C_ERROR_INVALID_ARGUMENT;
          if (def_val) {
            curr->default_value =
                (c_cdd_strdup(def_val, &_ast_strdup_175), _ast_strdup_175);
            if (!curr->default_value)
              return CDD_C_ERROR_MEMORY;
          }
          if (v_desc) {
            curr->description =
                (c_cdd_strdup(v_desc, &_ast_strdup_176), _ast_strdup_176);
            if (!curr->description)
              return CDD_C_ERROR_MEMORY;
          }
          if (enum_arr) {
            size_t ecount = json_array_get_count(enum_arr);
            size_t e;
            int found_default = 0;
            if (ecount == 0)
              return CDD_C_ERROR_INVALID_ARGUMENT;
            if (ecount > 0) {
              curr->enum_values = (char **)calloc(ecount, sizeof(char *));
              if (!curr->enum_values)
                return CDD_C_ERROR_MEMORY;
              curr->n_enum_values = ecount;
              for (e = 0; e < ecount; ++e) {
                const char *e_val = json_array_get_string(enum_arr, e);
                if (e_val) {
                  curr->enum_values[e] =
                      (c_cdd_strdup(e_val, &_ast_strdup_177), _ast_strdup_177);
                  if (!curr->enum_values[e])
                    return CDD_C_ERROR_MEMORY;
                  if (strcmp(e_val, def_val) == 0)
                    found_default = 1;
                }
              }
              if (!found_default)
                return CDD_C_ERROR_INVALID_ARGUMENT;
            }
          }
          {
            cdd_c_error_t _rc =
                collect_extensions(v_obj, &curr->extensions_json);
            if (_rc != CDD_C_SUCCESS)
              return _rc;
          }
        }
      }
    }
  }

  {
    cdd_c_error_t rc = validate_server_url_variables(out_srv);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses servers array from the given input.
 */
cdd_c_error_t parse_servers_array(const JSON_Object *parent, const char *key,
                                  struct OpenAPI_Server **out_servers,
                                  size_t *out_count) {
  const JSON_Array *servers;
  size_t count, i;

  if (!parent || !key || !out_servers || !out_count)
    return CDD_C_SUCCESS;

  *out_servers = NULL;
  *out_count = 0;

  servers = json_object_get_array(parent, key);
  if (!servers)
    return CDD_C_SUCCESS;

  count = json_array_get_count(servers);
  if (count == 0)
    return CDD_C_SUCCESS;

  *out_servers =
      (struct OpenAPI_Server *)calloc(count, sizeof(struct OpenAPI_Server));
  if (!*out_servers)
    return CDD_C_ERROR_MEMORY;
  *out_count = count;

  for (i = 0; i < count; ++i) {
    const JSON_Object *srv_obj = json_array_get_object(servers, i);
    if (srv_obj) {
      {
        cdd_c_error_t rc = parse_server_object(srv_obj, &(*out_servers)[i]);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
  }

  {
    size_t j;
    for (i = 0; i < count; ++i) {
      const char *name_i = (*out_servers)[i].name;
      if (!name_i || !*name_i)
        continue;
      for (j = i + 1; j < count; ++j) {
        const char *name_j = (*out_servers)[j].name;
        if (!name_j || !*name_j)
          continue;
        if (strcmp(name_i, name_j) == 0)
          return CDD_C_ERROR_INVALID_ARGUMENT;
      }
    }
  }

  return CDD_C_SUCCESS;
}
