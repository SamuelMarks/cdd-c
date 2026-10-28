/**
 * @file audit.c
 * @brief Implementation of project auditing.
 * Walks directories, filters for source files, runs static analysis, and
 * generates detailed JSON reports including file locations and line numbers.
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "functions/parse/audit.h"
#include "functions/parse/audit_internal.h"
#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "c_cdd_export.h"
#include "classes/parse/cdd_lexer.h"
#include "functions/parse/analysis.h"
#include "functions/parse/fs.h"
#include "functions/parse/str.h"
#include "functions/parse/tokenizer.h"
#include "openapi/emit/openapi.h"
#include <c89stringutils_string_extras.h>
#include <ctype.h>
#include <parson.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Initializes codebase audit statistics counters.
 *
 * @param[out] stats Pointer to the AuditStats structure to initialize.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t audit_stats_init(struct AuditStats *stats) {
  if (!stats)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  stats->files_scanned = 0;
  stats->allocations_checked = 0;
  stats->allocations_unchecked = 0;
  stats->functions_returning_alloc = 0;
  stats->violations.items = NULL;
  stats->violations.size = 0;
  stats->violations.capacity = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Releases memory held by codebase audit statistics.
 *
 * @param[in,out] stats Pointer to the AuditStats structure to free.
 */
void audit_stats_free(struct AuditStats *stats) {
  size_t i;
  if (!stats)
    return;
  if (stats->violations.items) {
    for (i = 0; i < stats->violations.size; i++) {
      C_CDD_FREE(stats->violations.items[i].file_path);
      C_CDD_FREE(stats->violations.items[i].variable_name);
      C_CDD_FREE(stats->violations.items[i].allocator_name);
    }
    C_CDD_FREE(stats->violations.items);
    stats->violations.items = NULL;
  }
  stats->violations.size = 0;
  stats->violations.capacity = 0;
}

/**
 * @brief Records an unchecked allocation violation in `stats`.
 *
 * @param[in,out] stats Audit statistics structure to update.
 * @param[in] file_path Path to the source file containing the violation.
 * @param[in] line 1-based line number of the violation.
 * @param[in] col 1-based column number of the violation.
 * @param[in] var_name Assigned variable name, or NULL if unassigned.
 * @param[in] allocator Name of the allocator function called.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t add_violation(struct AuditStats *stats,
                                   const char *file_path, size_t line,
                                   size_t col, const char *var_name,
                                   const char *allocator) {
  struct AuditViolationList *list = &stats->violations;
  if (list->size >= list->capacity) {
    size_t new_cap = list->capacity == 0 ? 8 : list->capacity * 2;
    struct AuditViolation *new_items = (struct AuditViolation *)C_CDD_REALLOC(
        list->items, new_cap * sizeof(struct AuditViolation));
    if (!new_items) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    list->items = new_items;
    list->capacity = new_cap;
  }

  list->items[list->size].file_path = C_CDD_STRDUP(file_path);
  list->items[list->size].line = line;
  list->items[list->size].col = col;
  list->items[list->size].variable_name =
      var_name ? C_CDD_STRDUP(var_name) : NULL;
  list->items[list->size].allocator_name = C_CDD_STRDUP(allocator);

  if (!list->items[list->size].file_path ||
      (var_name && !list->items[list->size].variable_name) ||
      !list->items[list->size].allocator_name) {
    C_CDD_FREE(list->items[list->size].file_path);
    C_CDD_FREE(list->items[list->size].variable_name);
    C_CDD_FREE(list->items[list->size].allocator_name);
    return CDD_C_ERROR_MEMORY;
  }

  list->size++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Calculate line and column number from a pointer into the buffer.
 *
 * @param[in] content Start of source buffer.
 * @param[in] token_ptr Pointer to token start within `content`.
 * @param[out] line Output pointer receiving 1-based line number.
 * @param[out] col Output pointer receiving 1-based column number.
 * @return `CDD_C_SUCCESS` on success.
 */
cdd_c_error_t audit_get_line_col(const char *content, const uint8_t *token_ptr,
                                 size_t *line, size_t *col) {
  const char *p = content;
  *line = 1;
  *col = 1;

  while ((const uint8_t *)p < token_ptr && *p != '\0') {
    if (*p == '\n') {
      (*line)++;
      *col = 1;
    } else {
      (*col)++;
    }
    p++;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Check if filename ends with `.c` extension.
 *
 * @param[in] path File path string to check.
 * @param[out] out_is_source Pointer to receive 1 if path ends with `.c`, 0
 * otherwise.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static cdd_c_error_t is_c_source(const char *path, int *out_is_source) {
  const char *dot;
  int diff;
  *out_is_source = 0;
  dot = strrchr(path, '.');
  if (!dot)
    return CDD_C_SUCCESS;
  {
    cdd_c_error_t rc_au = c_cdd_stricmp(dot, ".c", &diff);
    if (rc_au != CDD_C_SUCCESS)
      return rc_au;
  }
  *out_is_source = (diff == 0);
  return CDD_C_SUCCESS;
}

/**
 * @brief Helper to detect functions returning allocations directly.
 *
 * @param[in] tokens Lexical token list to scan.
 * @param[out] out_count Output pointer receiving the count of directly returned
 * allocations.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t count_returning_allocs(const struct TokenList *tokens,
                                     int *out_count) {
  size_t i;
  int count = 0;
  if (!out_count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_count = 0;

  if (!tokens || !tokens->tokens || tokens->size == 0)
    return CDD_C_SUCCESS;

  for (i = 0; i < tokens->size - 1; ++i) {
    if (tokens->tokens[i].kind == TOKEN_KEYWORD_RETURN) {
      size_t j = i + 1;
      while (j < tokens->size && tokens->tokens[j].kind == TOKEN_WHITESPACE)
        j++;

      if (j < tokens->size && tokens->tokens[j].kind == TOKEN_IDENTIFIER) {
        const char *name = (const char *)tokens->tokens[j].start;
        size_t len = tokens->tokens[j].length;

        if (len == 6) {
          if (strncmp(name, "malloc", 6) == 0) {
            count++;
          }
          if (strncmp(name, "calloc", 6) == 0) {
            count++;
          }
          if (strncmp(name, "strdup", 6) == 0) {
            count++;
          }
        } else if (len == 7) {
          if (strncmp(name, "realloc", 7) == 0) {
            count++;
          }
          if (strncmp(name, "strndup", 7) == 0) {
            count++;
          }
        }
      }
    }
  }
  *out_count = count;
  return CDD_C_SUCCESS;
}

/**
 * @brief Callback for directory walker that parses a file and updates audit
 * stats.
 *
 * @param[in] path Path to the visited file.
 * @param[in,out] user_data Pointer to `struct AuditStats`.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static cdd_c_error_t audit_file_callback(const char *path, void *user_data) {
  struct AuditStats *stats = (struct AuditStats *)user_data;
  struct TokenList *tokens = NULL;
  struct AllocationSiteList sites = {0};
  char *content = NULL;
  size_t sz = 0;
  cdd_c_error_t rc;
  size_t i;

  /* Filter only .c files */
  {
    int is_src = 0;
    cdd_c_error_t rc_au = is_c_source(path, &is_src);
    if (rc_au != CDD_C_SUCCESS)
      return rc_au;
    if (!is_src)
      return CDD_C_SUCCESS;
  }

  /* Read */
  rc = read_to_file(path, "r", &content, &sz);
  if (rc != CDD_C_SUCCESS) {
    fprintf(stderr, "Warning: Failed to read %s\n", path);
    return CDD_C_SUCCESS;
  }

  /* Tokenize */
  {
    cdd_c_error_t tok_rc = tokenize(az_span_create_from_str(content), &tokens);
    if (tok_rc != CDD_C_SUCCESS) {
      free_token_list(tokens);
      C_CDD_FREE(content);
      return CDD_C_SUCCESS; /* Tokenization fail - skip */
    }
  }

  /* Analyze */
  {
    cdd_c_error_t find_rc = find_allocations(tokens, &sites);
    if (find_rc == CDD_C_SUCCESS) {
      stats->files_scanned++;

      for (i = 0; i < sites.size; ++i) {
        if (sites.sites[i].is_checked) {
          stats->allocations_checked++;
        } else {
          size_t line, col;
          const struct Token *tok = &tokens->tokens[sites.sites[i].token_index];
          audit_get_line_col(content, tok->start, &line, &col);

          add_violation(stats, path, line, col, sites.sites[i].var_name,
                        sites.sites[i].spec->name);

          stats->allocations_unchecked++;
        }
      }
    }
  }

  allocation_site_list_free(&sites);

  /* Custom analysis for return-alloc patterns */
  {
    int count = 0;
    count_returning_allocs(tokens, &count);
    stats->functions_returning_alloc += (size_t)count;
  }

  free_token_list(tokens);
  C_CDD_FREE(content);

  return CDD_C_SUCCESS;
}

/**
 * @brief Scans a project directory to collect safety, allocation, and macro
 * audit metrics.
 *
 * @param[in] root_path Root directory or file path to audit.
 * @param[in,out] stats Target AuditStats structure to populate.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t audit_project(const char *root_path, struct AuditStats *stats) {
  if (!root_path || !stats)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  return walk_directory(root_path, audit_file_callback, stats);
}

/**
 * @brief Accumulates a Parson `JSON_Status` into a sticky `cdd_c_error_t`
 * state.
 *
 * @param[in] st Parson status (`JSONSuccess` or `JSONFailure`).
 * @param[in,out] rc Sticky error code updated to `CDD_C_ERROR_MEMORY` on
 * failure.
 * @return The updated `*rc`.
 */
static cdd_c_error_t audit_check_json(JSON_Status st, cdd_c_error_t *rc) {
  if (st != JSONSuccess)
    *rc = CDD_C_ERROR_MEMORY;
  return *rc;
}

/**
 * @brief Safely attaches `val` to `obj[name]`, freeing `val` and recording
 * `CDD_C_ERROR_MEMORY` into `rc` on failure.
 *
 * @param[in,out] obj Target JSON object.
 * @param[in] name Property key.
 * @param[in] val Value to attach (ownership transferred or freed).
 * @param[in,out] rc Sticky error code.
 * @return The updated `*rc`.
 */
static cdd_c_error_t audit_set_val(JSON_Object *obj, const char *name,
                                   JSON_Value *val, cdd_c_error_t *rc) {
  if (!val || cdd_json_object_set_value_safe(obj, name, val) != CDD_C_SUCCESS)
    *rc = CDD_C_ERROR_MEMORY;
  return *rc;
}

/**
 * @brief Safely appends `val` to `arr`, freeing `val` and recording
 * `CDD_C_ERROR_MEMORY` into `rc` on failure.
 *
 * @param[in,out] arr Target JSON array.
 * @param[in] val Value to append (ownership transferred or freed).
 * @param[in,out] rc Sticky error code.
 * @return The updated `*rc`.
 */
static cdd_c_error_t audit_append_val(JSON_Array *arr, JSON_Value *val,
                                      cdd_c_error_t *rc) {
  if (!val || cdd_json_array_append_value_safe(arr, val) != CDD_C_SUCCESS)
    *rc = CDD_C_ERROR_MEMORY;
  return *rc;
}

/**
 * @brief Serializes collected audit statistics as a JSON report.
 *
 * @param[in] stats AuditStats structure to serialize.
 * @param[out] out_json Output pointer receiving the newly allocated JSON report
 * string.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t audit_print_json(const struct AuditStats *stats,
                               char **out_json) {
  JSON_Value *root_val = NULL;
  JSON_Object *root_obj;
  char *str = NULL;
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (!stats || !out_json) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  root_val = json_value_init_object();
  if (!root_val) {
    *out_json = NULL;
    return CDD_C_ERROR_MEMORY;
  }

  root_obj = json_value_get_object(root_val);

  {
    JSON_Value *summary_val = json_value_init_object();
    JSON_Object *summary_obj = json_value_get_object(summary_val);

    audit_check_json(json_object_set_number(summary_obj, "files_scanned",
                                            (double)stats->files_scanned),
                     &rc);
    audit_check_json(json_object_set_number(summary_obj, "allocations_checked",
                                            (double)stats->allocations_checked),
                     &rc);
    audit_check_json(
        json_object_set_number(summary_obj, "allocations_unchecked",
                               (double)stats->allocations_unchecked),
        &rc);
    audit_check_json(
        json_object_set_number(summary_obj, "functions_returning_alloc",
                               (double)stats->functions_returning_alloc),
        &rc);

    audit_set_val(root_obj, "summary", summary_val, &rc);
  }

  {
    JSON_Value *violations_val = json_value_init_array();
    JSON_Array *violations_arr = json_value_get_array(violations_val);

    for (i = 0; i < stats->violations.size; ++i) {
      struct AuditViolation *v = &stats->violations.items[i];
      JSON_Value *v_val = json_value_init_object();
      JSON_Object *v_obj = json_value_get_object(v_val);

      audit_check_json(json_object_set_string(v_obj, "file", v->file_path),
                       &rc);
      audit_check_json(json_object_set_number(v_obj, "line", (double)v->line),
                       &rc);
      audit_check_json(json_object_set_number(v_obj, "col", (double)v->col),
                       &rc);
      if (v->variable_name)
        audit_check_json(
            json_object_set_string(v_obj, "variable", v->variable_name), &rc);
      else
        audit_check_json(json_object_set_null(v_obj, "variable"), &rc);
      audit_check_json(
          json_object_set_string(v_obj, "allocator", v->allocator_name), &rc);

      audit_append_val(violations_arr, v_val, &rc);
    }
    audit_set_val(root_obj, "violations", violations_val, &rc);
  }

  if (rc == CDD_C_SUCCESS)
    str = json_serialize_to_string_pretty(root_val);
  json_value_free(root_val);
  if (!str) {
    *out_json = NULL;
    return CDD_C_ERROR_MEMORY;
  }
  *out_json = str;
  return CDD_C_SUCCESS;
}
