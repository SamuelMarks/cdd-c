/**
 * @file project_audit.c
 * @brief Implementation of project auditing.
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
 * Look for "return malloc(...)" patterns.
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
    /* If read fails, maybe permissions, just skip with warning on stderr?
       Or bubble error? Walking typically proceeds. */
    fprintf(stderr, "Warning: Failed to read %s\n", path);
    return 0;
  }

  /* Tokenize */
  /* Cast to char* required by API, but treating const */
  if (tokenize(az_span_create_from_str(content), &tokens) != 0) {
    free(content);
    return 0; /* Tokenization fail - skip */
  }

  /* Analyze */
  if (find_allocations(tokens, &sites) == 0) {
    stats->files_scanned++;

    for (i = 0; i < sites.size; ++i) {
      if (sites.sites[i].is_checked)
        stats->allocations_checked++;
      else
        stats->allocations_unchecked++;
    }
    allocation_site_list_free(&sites);
  }

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

  if (!stats || !root_val)
    return NULL;

  root_obj = json_value_get_object(root_val);
  json_object_set_number(root_obj, "files_scanned",
                         (double)stats->files_scanned);
  json_object_set_number(root_obj, "allocations_checked",
                         (double)stats->allocations_checked);
  json_object_set_number(root_obj, "allocations_unchecked",
                         (double)stats->allocations_unchecked);
  json_object_set_number(root_obj, "functions_returning_alloc",
                         (double)stats->functions_returning_alloc);

  str = json_serialize_to_string_pretty(root_val);
  json_value_free(root_val);
  return str;
}
