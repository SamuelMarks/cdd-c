/**
 * @file project_audit.c
 * @brief Implementation of project auditing.
 * Walks directories, filters for source files, runs static analysis, and
 * generates detailed JSON reports including file locations and line numbers.
 * @author Samuel Marks
 */

#include "project_audit.h"
#include "analysis.h"
#include "fs.h"
#include "tokenizer.h"

#include <c89stringutils_string_extras.h>
#include <ctype.h>
#include <parson.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#define strcasecmp _stricmp
#endif
#else
#include <strings.h>
#endif

void audit_stats_init(struct AuditStats *const stats) {
  if (stats) {
    stats->files_scanned = 0;
    stats->allocations_checked = 0;
    stats->allocations_unchecked = 0;
    stats->functions_returning_alloc = 0;
    stats->violations.items = NULL;
    stats->violations.size = 0;
    stats->violations.capacity = 0;
  }
}

void audit_stats_free(struct AuditStats *const stats) {
  size_t i;
  if (!stats)
    return;
  if (stats->violations.items) {
    for (i = 0; i < stats->violations.size; i++) {
      free(stats->violations.items[i].file_path);
      free(stats->violations.items[i].variable_name);
      free(stats->violations.items[i].allocator_name);
    }
    free(stats->violations.items);
    stats->violations.items = NULL;
  }
  stats->violations.size = 0;
  stats->violations.capacity = 0;
}

static int add_violation(struct AuditStats *stats, const char *file_path,
                         size_t line, size_t col, const char *var_name,
                         const char *allocator) {
  struct AuditViolationList *list = &stats->violations;
  if (list->size >= list->capacity) {
    size_t new_cap = list->capacity == 0 ? 8 : list->capacity * 2;
    struct AuditViolation *new_items = (struct AuditViolation *)realloc(
        list->items, new_cap * sizeof(struct AuditViolation));
    if (!new_items)
      return ENOMEM;
    list->items = new_items;
    list->capacity = new_cap;
  }
  list->items[list->size].file_path = strdup(file_path);
  list->items[list->size].line = line;
  list->items[list->size].col = col;
  list->items[list->size].variable_name = var_name ? strdup(var_name) : NULL;
  list->items[list->size].allocator_name = allocator ? strdup(allocator) : NULL;

  if (!list->items[list->size].file_path ||
      (var_name && !list->items[list->size].variable_name) ||
      (allocator && !list->items[list->size].allocator_name)) {
    /* Handle partial alloc failure */
    free(list->items[list->size].file_path);
    free(list->items[list->size].variable_name);
    free(list->items[list->size].allocator_name);
    return ENOMEM;
  }

  list->size++;
  return 0;
}

/**
 * @brief Calculate line and column number from a pointer into the buffer.
 *
 * @param[in] content Start of the file content buffer.
 * @param[in] token_ptr Pointer to the token within the buffer.
 * @param[out] line 1-based line number.
 * @param[out] col 1-based column number.
 */
static void get_line_col(const char *content, const uint8_t *token_ptr,
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
}

/**
 * @brief Check if filename ends with .c extension.
 */
static int is_c_source(const char *path) {
  const char *dot = strrchr(path, '.');
  if (!dot)
    return 0;
  return (strcasecmp(dot, ".c") == 0);
}

/**
 * @brief Helper to detect functions returning allocations directly.
 */
static int count_returning_allocs(const struct TokenList *tokens) {
  size_t i;
  int count = 0;

  for (i = 0; i < tokens->size - 1; ++i) {
    if (tokens->tokens[i].kind == TOKEN_IDENTIFIER &&
        tokens->tokens[i].length == 6 &&
        strncmp((const char *)tokens->tokens[i].start, "return", 6) == 0) {

      /* Check next non-ws token */
      size_t j = i + 1;
      while (j < tokens->size && tokens->tokens[j].kind == TOKEN_WHITESPACE)
        j++;

      if (j < tokens->size && tokens->tokens[j].kind == TOKEN_IDENTIFIER) {
        /* Check if it's a known allocator */
        const char *name = (const char *)tokens->tokens[j].start;
        size_t len = tokens->tokens[j].length;

        if ((len == 6 && strncmp(name, "malloc", 6) == 0) ||
            (len == 6 && strncmp(name, "calloc", 6) == 0) ||
            (len == 7 && strncmp(name, "realloc", 7) == 0) ||
            (len == 6 && strncmp(name, "strdup", 6) == 0) ||
            (len == 7 && strncmp(name, "strndup", 7) == 0)) {
          count++;
        }
      }
    }
  }
  return count;
}

/**
 * @brief Callback for directory walker.
 * Parses file and updates stats.
 */
static int audit_file_callback(const char *path, void *user_data) {
  struct AuditStats *stats = (struct AuditStats *)user_data;
  struct TokenList *tokens = NULL;
  struct AllocationSiteList sites = {0};
  char *content = NULL;
  size_t sz = 0;
  int rc;
  size_t i;

  /* Filter only .c files */
  if (!is_c_source(path))
    return 0;

  /* Read */
  rc = read_to_file(path, "r", &content, &sz);
  if (rc != 0) {
    fprintf(stderr, "Warning: Failed to read %s\n", path);
    return 0;
  }

  /* Tokenize */
  if (tokenize(az_span_create_from_str(content), &tokens) != 0) {
    free(content);
    return 0; /* Tokenization fail - skip */
  }

  /* Analyze */
  if (find_allocations(tokens, &sites) == 0) {
    stats->files_scanned++;

    for (i = 0; i < sites.size; ++i) {
      if (sites.sites[i].is_checked) {
        stats->allocations_checked++;
      } else {
        size_t line, col;
        /* Calculate line/col for the violation */
        const struct Token *tok = &tokens->tokens[sites.sites[i].token_index];
        get_line_col(content, tok->start, &line, &col);

        /* Add to details */
        add_violation(stats, path, line, col, sites.sites[i].var_name,
                      sites.sites[i].spec->name);

        stats->allocations_unchecked++;
      }
    }
  }

  allocation_site_list_free(&sites);

  /* Custom analysis for return-alloc patterns */
  stats->functions_returning_alloc += count_returning_allocs(tokens);

  free_token_list(tokens);
  free(content);

  return 0;
}

int audit_project(const char *const root_path, struct AuditStats *const stats) {
  if (!root_path || !stats)
    return EINVAL;

  return walk_directory(root_path, audit_file_callback, stats);
}

char *audit_print_json(const struct AuditStats *const stats) {
  JSON_Value *root_val = json_value_init_object();
  JSON_Object *root_obj;
  char *str = NULL;
  size_t i;

  if (!stats || !root_val)
    return NULL;

  root_obj = json_value_get_object(root_val);

  {
    JSON_Value *summary_val = json_value_init_object();
    JSON_Object *summary_obj = json_value_get_object(summary_val);

    json_object_set_number(summary_obj, "files_scanned",
                           (double)stats->files_scanned);
    json_object_set_number(summary_obj, "allocations_checked",
                           (double)stats->allocations_checked);
    json_object_set_number(summary_obj, "allocations_unchecked",
                           (double)stats->allocations_unchecked);
    json_object_set_number(summary_obj, "functions_returning_alloc",
                           (double)stats->functions_returning_alloc);

    json_object_set_value(root_obj, "summary", summary_val);
  }

  {
    JSON_Value *violations_val = json_value_init_array();
    JSON_Array *violations_arr = json_value_get_array(violations_val);

    for (i = 0; i < stats->violations.size; ++i) {
      struct AuditViolation *v = &stats->violations.items[i];
      JSON_Value *v_val = json_value_init_object();
      JSON_Object *v_obj = json_value_get_object(v_val);

      json_object_set_string(v_obj, "file", v->file_path);
      json_object_set_number(v_obj, "line", (double)v->line);
      json_object_set_number(v_obj, "col", (double)v->col);
      if (v->variable_name)
        json_object_set_string(v_obj, "variable", v->variable_name);
      else
        json_object_set_null(v_obj, "variable");
      json_object_set_string(v_obj, "allocator", v->allocator_name);

      json_array_append_value(violations_arr, v_val);
    }
    json_object_set_value(root_obj, "violations", violations_val);
  }

  str = json_serialize_to_string_pretty(root_val);
  json_value_free(root_val);
  return str;
}
