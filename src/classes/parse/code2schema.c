/**
 * @file code2schema.c
 * @brief CLI entry point and top-level orchestration for code2schema.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/**
 * @brief CLI entry point for code2schema command.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return 0 on success, EXIT_FAILURE on error.
 */
cdd_c_error_t code2schema_main(int argc, char **argv) {
  FILE *fp;
  char line[MAX_LINE_LENGTH];
  JSON_Value *root = json_value_init_object();
  JSON_Object *root_obj = json_value_get_object(root);
  JSON_Value *schemas_val = json_value_init_object();
  JSON_Object *schemas_obj = json_value_get_object(schemas_val);
  JSON_Value *comp_val = json_value_init_object();
  JSON_Object *comp_obj = json_value_get_object(comp_val);
  cdd_c_error_t ret_rc = CDD_C_SUCCESS;

  if (argc != 2 || !argv || !argv[0] || !argv[1]) {
    json_value_free(root);
    json_value_free(schemas_val);
    json_value_free(comp_val);
    return CDD_C_ERROR_UNKNOWN;
  }

#if defined(_MSC_VER)
  if (fopen_s(&fp, argv[0], "r") != 0)
    fp = NULL;
#else
  fp = fopen(argv[0], "r");
#endif
  if (!fp) {
    json_value_free(root);
    json_value_free(schemas_val);
    json_value_free(comp_val);
    return CDD_C_ERROR_UNKNOWN;
  }

  json_object_set_value(comp_obj, "schemas", schemas_val);
  json_object_set_value(root_obj, "components", comp_val);

  {
    int has_line = 0;
    cdd_c_error_t rc_rl;
    while ((rc_rl = c2s_read_line(fp, line, sizeof(line), &has_line)) ==
               CDD_C_SUCCESS &&
           has_line) {
      char *p = line;
      while (isspace((unsigned char)*p))
        p++;

      if (strncmp(p, "union ", 6) == 0) {
        char union_name[64] = {0};
        char *brace = strchr(p, '{');
        if (brace) {
          *brace = 0;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
          if (sscanf_s(p + 6, "%63s", union_name,
                       (unsigned)sizeof(union_name)) == 1) {
#else
          if (sscanf(p + 6, "%63s", union_name) == 1) {
#endif
            {
              cdd_c_error_t rc_c2s =
                  c2s_parse_union_and_write(fp, schemas_obj, union_name);
              if (rc_c2s != CDD_C_SUCCESS) {
                ret_rc = rc_c2s;
                goto cleanup;
              }
            }
          }
        }
      } else if (strncmp(p, "struct ", 7) == 0) {
        char struct_name[64] = {0};
        char *brace = strchr(p, '{');
        if (brace) {
          struct StructFields sf;
          *brace = 0;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
          if (sscanf_s(p + 7, "%63s", struct_name,
                       (unsigned)sizeof(struct_name)) == 1) {
#else
          if (sscanf(p + 7, "%63s", struct_name) == 1) {
#endif
            if (struct_fields_init(&sf) == 0) {
              char subline[MAX_LINE_LENGTH];
              int has_subline = 0;
              while ((rc_rl = c2s_read_line(fp, subline, sizeof(subline),
                                            &has_subline)) == CDD_C_SUCCESS &&
                     has_subline) {
                char *sp = subline;
                while (isspace((unsigned char)*sp))
                  sp++;
                if (*sp == '}')
                  break;
                if (strncmp(sp, "struct {", 8) == 0 ||
                    strncmp(sp, "struct{", 7) == 0) {
                  /* Start nested struct */
                  struct StructFields nested_sf;
                  char nested_name[128];
                  char nested_prop_name[64] = {0};
                  int has_nested = 0;
                  {
                    cdd_c_error_t rc_c2s = struct_fields_init(&nested_sf);
                    if (rc_c2s != CDD_C_SUCCESS) {
                      struct_fields_free(&sf);
                      ret_rc = rc_c2s;
                      goto cleanup;
                    }
                  }
                  while ((rc_rl = c2s_read_line(fp, subline, sizeof(subline),
                                                &has_nested)) ==
                             CDD_C_SUCCESS &&
                         has_nested) {
                    char *nsp = subline;
                    while (isspace((unsigned char)*nsp))
                      nsp++;
                    if (*nsp == '}') {
                      /* extract name */
                      char *semi = strchr(nsp, ';');
                      if (semi)
                        *semi = 0;
                      nsp++;
                      while (isspace((unsigned char)*nsp))
                        nsp++;
                      CDD_STRCPY(nested_prop_name, sizeof(nested_prop_name),
                                 nsp);
                      break;
                    }
                    if (*nsp) {
                      cdd_c_error_t rc_c2s =
                          parse_struct_member_line(nsp, &nested_sf);
                      if (rc_c2s != CDD_C_SUCCESS) {
                        struct_fields_free(&nested_sf);
                        struct_fields_free(&sf);
                        ret_rc = rc_c2s;
                        goto cleanup;
                      }
                    }
                  }
                  CDD_SNPRINTF(nested_name, sizeof(nested_name), "%s_%s",
                               struct_name, nested_prop_name);
                  {
                    cdd_c_error_t rc_coll = c2s_collapse_arrays(&nested_sf);
                    if (rc_coll != CDD_C_SUCCESS) {
                      struct_fields_free(&nested_sf);
                      struct_fields_free(&sf);
                      ret_rc = rc_coll;
                      goto cleanup;
                    }
                  }
                  write_struct_to_json_schema(schemas_obj, nested_name,
                                              &nested_sf);
                  struct_fields_add(&sf, nested_prop_name, "object",
                                    nested_name, NULL, NULL);
                  struct_fields_free(&nested_sf);
                  continue;
                }
                if (*sp) {
                  cdd_c_error_t rc_c2s = parse_struct_member_line(sp, &sf);
                  if (rc_c2s != CDD_C_SUCCESS) {
                    struct_fields_free(&sf);
                    ret_rc = rc_c2s;
                    goto cleanup;
                  }
                }
              }
              {
                cdd_c_error_t rc_coll = c2s_collapse_arrays(&sf);
                if (rc_coll != CDD_C_SUCCESS) {
                  struct_fields_free(&sf);
                  ret_rc = rc_coll;
                  goto cleanup;
                }
              }
              {
                cdd_c_error_t rc_c2s =
                    write_struct_to_json_schema(schemas_obj, struct_name, &sf);
                if (rc_c2s != CDD_C_SUCCESS) {
                  struct_fields_free(&sf);
                  ret_rc = rc_c2s;
                  goto cleanup;
                }
              }
              struct_fields_free(&sf);
            }
          }
        }
      } else if (strncmp(p, "enum ", 5) == 0) {
        char enum_name[64] = {0};
        char *brace = strchr(p, '{');
        if (brace) {
          JSON_Value *eval = json_value_init_object();
          JSON_Object *eobj = json_value_get_object(eval);
          JSON_Value *arrval = json_value_init_array();
          JSON_Array *earr = json_value_get_array(arrval);

          *brace = 0;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
          if (sscanf_s(p + 5, "%63s", enum_name, (unsigned)sizeof(enum_name)) ==
              1) {
#else
          if (sscanf(p + 5, "%63s", enum_name) == 1) {
#endif
            const char *delim = ",}";
            char *token;
            char *ctx = NULL;
            char *rest = brace + 1;
            char full_body[4096] = {0};

            /* Accumulate body */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
            strcat_s(full_body, sizeof(full_body), rest);
            while (!strchr(full_body, '}')) {
              int has_el = 0;
              if (c2s_read_line(fp, line, sizeof(line), &has_el) !=
                      CDD_C_SUCCESS ||
                  !has_el)
                break;
              strcat_s(full_body, sizeof(full_body), line);
            }
#else
            strcat(full_body, rest);
            while (!strchr(full_body, '}')) {
              int has_el = 0;
              if (c2s_read_line(fp, line, sizeof(line), &has_el) !=
                      CDD_C_SUCCESS ||
                  !has_el)
                break;
              strcat(full_body, line);
            }
#endif

#ifdef _WIN32
            token = strtok_s(full_body, delim, &ctx);
#else
            token = strtok_r(full_body, delim, &ctx);
#endif
            while (token) {
              char *tm = token;
              while (isspace((unsigned char)*tm))
                tm++;
              c_cdd_str_trim_trailing_whitespace(tm);
              {
                char *eq = strchr(tm, '=');
                if (eq)
                  *eq = 0;
                c_cdd_str_trim_trailing_whitespace(tm);
              }
              if (*tm)
                json_array_append_string(earr, tm);
#ifdef _WIN32
              token = strtok_s(NULL, delim, &ctx);
#else
              token = strtok_r(NULL, delim, &ctx);
#endif
            }
            json_object_set_string(eobj, "type", "string");
            json_object_set_value(eobj, "enum", arrval);
            json_object_set_value(schemas_obj, enum_name, eval);
          } else {
            json_value_free(eval);
            json_value_free(arrval);
          }
        }
      }
    }
  }

  if (json_serialize_to_file_pretty(root, argv[1]) != JSONSuccess) {
    ret_rc = CDD_C_ERROR_IO;
    goto cleanup;
  }
  ret_rc = CDD_C_SUCCESS;

cleanup:
  fclose(fp);
  json_value_free(root);
  return ret_rc;
}
