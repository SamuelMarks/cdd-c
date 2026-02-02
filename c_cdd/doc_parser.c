/**
 * @file doc_parser.c
 * @brief Implementation of the documentation comment parser.
 *
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doc_parser.h"
#include "str_utils.h"

/* --- Helpers --- */

/**
 * @brief Check if a character is a newline.
 */
static int is_eol(char c) { return c == '\n' || c == '\r'; }

/**
 * @brief Skip whitespace in a string buffer.
 */
static const char *skip_ws(const char *p) {
  while (*p && isspace((unsigned char)*p) && !is_eol(*p)) {
    p++;
  }
  return p;
}

/**
 * @brief Extract the next word from the string.
 *
 * @param str Input string pointer.
 * @param end Pointer to end of string (or newline).
 * @param next_out [out] Pointer to where the scan stopped.
 * @return Allocated string containing the word, or NULL on failure.
 */
static char *extract_word(const char *str, const char *end,
                          const char **next_out) {
  const char *p = skip_ws(str);
  const char *word_start = p;
  size_t len;
  char *res;

  while (p < end && !isspace((unsigned char)*p)) {
    p++;
  }

  len = (size_t)(p - word_start);
  if (len == 0) {
    if (next_out)
      *next_out = p;
    return NULL;
  }

  res = (char *)malloc(len + 1);
  if (!res)
    return NULL;

  memcpy(res, word_start, len);
  res[len] = '\0';

  if (next_out)
    *next_out = p;
  return res;
}

/**
 * @brief Extract the remainder of the line as text.
 * Trims leading/trailing whitespace.
 */
static char *extract_rest(const char *str, const char *end) {
  const char *p = skip_ws(str);
  const char *e = end;
  size_t len;
  char *res;

  /* Trim trailing */
  while (e > p && isspace((unsigned char)*(e - 1))) {
    e--;
  }

  len = (size_t)(e - p);
  if (len == 0)
    return NULL;

  res = (char *)malloc(len + 1);
  if (!res)
    return NULL;

  memcpy(res, p, len);
  res[len] = '\0';
  return res;
}

/* --- Core Logic --- */

int doc_metadata_init(struct DocMetadata *const meta) {
  if (!meta)
    return EINVAL;
  memset(meta, 0, sizeof(*meta));
  return 0;
}

void doc_metadata_free(struct DocMetadata *const meta) {
  size_t i;
  if (!meta)
    return;

  if (meta->route)
    free(meta->route);
  if (meta->verb)
    free(meta->verb);
  if (meta->summary)
    free(meta->summary);

  if (meta->params) {
    for (i = 0; i < meta->n_params; ++i) {
      free(meta->params[i].name);
      free(meta->params[i].in_loc);
      free(meta->params[i].description);
    }
    free(meta->params);
  }

  if (meta->returns) {
    for (i = 0; i < meta->n_returns; ++i) {
      free(meta->returns[i].code);
      free(meta->returns[i].description);
    }
    free(meta->returns);
  }

  memset(meta, 0, sizeof(*meta));
}

static int parse_param_line(const char *line, const char *end,
                            struct DocMetadata *out) {
  struct DocParam *new_params;
  struct DocParam *p;
  const char *cur = line;

  /* Realloc array */
  new_params = (struct DocParam *)realloc(
      out->params, (out->n_params + 1) * sizeof(struct DocParam));
  if (!new_params)
    return ENOMEM;
  out->params = new_params;
  p = &out->params[out->n_params];
  memset(p, 0, sizeof(*p));

  /* 1. Name */
  p->name = extract_word(cur, end, &cur);
  if (!p->name) {
    /* Malformed param line, ignore but don't crash */
    return 0;
  }

  /* 2. Check for Attributes [key:val] or [required] */
  cur = skip_ws(cur);
  while (cur < end && *cur == '[') {
    const char *close_bracket = cur;
    while (close_bracket < end && *close_bracket != ']')
      close_bracket++;

    if (close_bracket < end) {
      /* Extract content inside [] */
      const char *inner_start = cur + 1;
      size_t inner_len = (size_t)(close_bracket - inner_start);
      char *attr = (char *)malloc(inner_len + 1);
      if (attr) {
        memcpy(attr, inner_start, inner_len);
        attr[inner_len] = '\0';

        if (strncmp(attr, "in:", 3) == 0) {
          p->in_loc = c_cdd_strdup(attr + 3);
        } else if (strcmp(attr, "required") == 0) {
          p->required = 1;
        }
        free(attr);
      }
      cur = close_bracket + 1;
      cur = skip_ws(cur);
    } else {
      break; /* Unbalanced */
    }
  }

  /* 3. Description */
  p->description = extract_rest(cur, end);

  out->n_params++;
  return 0;
}

static int parse_return_line(const char *line, const char *end,
                             struct DocMetadata *out) {
  struct DocResponse *new_resps;
  struct DocResponse *r;
  const char *cur = line;

  new_resps = (struct DocResponse *)realloc(
      out->returns, (out->n_returns + 1) * sizeof(struct DocResponse));
  if (!new_resps)
    return ENOMEM;
  out->returns = new_resps;
  r = &out->returns[out->n_returns];
  memset(r, 0, sizeof(*r));

  /* 1. Code */
  r->code = extract_word(cur, end, &cur);
  if (!r->code) {
    return 0;
  }

  /* 2. Description */
  r->description = extract_rest(cur, end);

  out->n_returns++;
  return 0;
}

static int parse_route_line(const char *line, const char *end,
                            struct DocMetadata *out) {
  const char *cur = line;
  char *word1 = extract_word(cur, end, &cur);
  char *word2 = NULL;

  if (!word1)
    return 0;

  /* Check if word1 is a Verb or Path */
  /* Heuristic: Starts with / is path. Uppercase is verb. */
  if (word1[0] == '/') {
    /* No verb specified */
    if (out->route)
      free(out->route);
    out->route = word1;
  } else {
    /* Assume verb */
    if (out->verb)
      free(out->verb);
    out->verb = word1;

    /* Next word should be path */
    word2 = extract_word(cur, end, &cur);
    if (word2) {
      if (out->route)
        free(out->route);
      out->route = word2;
    }
  }
  return 0;
}

int doc_parse_block(const char *const comment, struct DocMetadata *const out) {
  const char *p = comment;
  int rc = 0;

  if (!comment || !out)
    return EINVAL;

  /* Skip initial marker if present (e.g. "/**" or "///") via simple scan loop
   */
  while (*p) {
    const char *line_end;
    const char *scan;

    /* Find end of current line */
    line_end = p;
    while (*line_end && !is_eol(*line_end)) {
      line_end++;
    }

    /* Analyze line content */
    scan = p;

    /* Skip leading whitespace */
    while (scan < line_end && isspace((unsigned char)*scan))
      scan++;

    /* Skip Comment Decorators */
    if (scan < line_end && *scan == '/') {
      scan++;
      if (scan < line_end && *scan == '*') {
        /* Block comment start */
        scan++;
        while (scan < line_end && *scan == '*')
          scan++;
      } else if (scan < line_end && *scan == '/') {
        /* Line comment start */
        scan++;
        while (scan < line_end && *scan == '/')
          scan++;
      }
    } else if (scan < line_end && *scan == '*') {
      /* Continuation line start */
      scan++;
      /* Also skip closing slash if present */
      if (scan < line_end && *scan == '/')
        scan++;
    }

    /* Skip whitespace again after decorators */
    while (scan < line_end && isspace((unsigned char)*scan))
      scan++;

    /* Check for Directive (@ or \) */
    if (scan < line_end && (*scan == '@' || *scan == '\\')) {
      const char *cmd_start = scan + 1;
      const char *cmd_end = cmd_start;
      char *cmd = NULL;

      while (cmd_end < line_end && isalpha((unsigned char)*cmd_end)) {
        cmd_end++;
      }

      {
        size_t cmd_len = (size_t)(cmd_end - cmd_start);
        cmd = (char *)malloc(cmd_len + 1);
        if (!cmd) {
          rc = ENOMEM;
          goto cleanup;
        }
        memcpy(cmd, cmd_start, cmd_len);
        cmd[cmd_len] = '\0';

        /* Dispatch */
        if (strcmp(cmd, "route") == 0) {
          rc = parse_route_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "param") == 0) {
          rc = parse_param_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "return") == 0 || strcmp(cmd, "returns") == 0) {
          rc = parse_return_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "summary") == 0 || strcmp(cmd, "brief") == 0) {
          if (out->summary)
            free(out->summary);
          out->summary = extract_rest(cmd_end, line_end);
        }

        free(cmd);
        if (rc != 0)
          goto cleanup;
      }
    } else {
      /* Continuation / Description lines not handled in this basic pass */
    }

    /* Advance to next line */
    p = line_end;
    while (*p && is_eol(*p))
      p++;
  }

cleanup:
  return rc;
}
