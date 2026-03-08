#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/safe_crt.h"

void safe_crt_patch_list_init(struct SafeCrtPatchList *list) {
  if (!list) return;
  list->patches = NULL;
  list->size = 0;
  list->capacity = 0;
}

void safe_crt_patch_list_free(struct SafeCrtPatchList *list) {
  size_t i;
  if (!list) return;
  if (list->patches) {
    for (i = 0; i < list->size; ++i) {
      if (list->patches[i].replacement_text) {
        free(list->patches[i].replacement_text);
      }
    }
    free(list->patches);
  }
  list->patches = NULL;
  list->size = 0;
  list->capacity = 0;
}

static int add_patch(struct SafeCrtPatchList *list, size_t start, size_t end, const char *text) {
  struct SafeCrtPatch *p;
  if (!list || !text) return EINVAL;

  if (list->size >= list->capacity) {
    size_t new_cap = list->capacity == 0 ? 8 : list->capacity * 2;
    struct SafeCrtPatch *new_arr = realloc(list->patches, new_cap * sizeof(struct SafeCrtPatch));
    if (!new_arr) return ENOMEM;
    list->patches = new_arr;
    list->capacity = new_cap;
  }

  p = &list->patches[list->size++];
  p->start_token_idx = start;
  p->end_token_idx = end;
  
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  p->replacement_text = _strdup(text);
#else
  p->replacement_text = strdup(text);
#endif
  
  if (!p->replacement_text) return ENOMEM;
  return 0;
}

static char *extract_token_text(const struct TokenList *tokens, size_t start, size_t end) {
  size_t len = 0;
  size_t i;
  char *str;
  char *ptr;

  for (i = start; i < end; ++i) {
    len += tokens->tokens[i].length;
  }
  
  str = malloc(len + 1);
  if (!str) return NULL;
  
  ptr = str;
  for (i = start; i < end; ++i) {
    memcpy(ptr, tokens->tokens[i].start, tokens->tokens[i].length);
    ptr += tokens->tokens[i].length;
  }
  *ptr = '\0';
  return str;
}

static int generate_strcpy_patch(const struct TokenList *tokens, size_t call_start, size_t call_end, struct SafeCrtPatchList *out) {
  /* Pattern: strcpy(dest, src) */
  size_t lparen = 0;
  size_t comma = 0;
  size_t rparen = 0;
  size_t i;
  char *dest = NULL;
  char *src = NULL;
  char replacement[1024] = {0};

  for (i = call_start; i < call_end; ++i) {
    if (tokens->tokens[i].kind == TOKEN_LPAREN && lparen == 0) lparen = i;
    else if (tokens->tokens[i].kind == TOKEN_COMMA && lparen != 0 && comma == 0) comma = i;
    else if (tokens->tokens[i].kind == TOKEN_RPAREN && comma != 0 && rparen == 0) rparen = i;
  }

  if (lparen != 0 && comma != 0 && rparen != 0) {
    dest = extract_token_text(tokens, lparen + 1, comma);
    src = extract_token_text(tokens, comma + 1, rparen);
    
    if (dest && src) {
      /* Naive data-flow: assume `sizeof(dest)` works. In a real engine, we'd look up the AST node's type. */
      sprintf(replacement, 
        "#if defined(_MSC_VER)\n"
        "  strcpy_s(%s, sizeof(%s), %s);\n"
        "#else\n"
        "  strcpy(%s, %s);\n"
        "#endif", dest, dest, src, dest, src);
      add_patch(out, call_start, call_end, replacement);
    }
    
    if (dest) free(dest);
    if (src) free(src);
  }
  return 0;
}

static int generate_fopen_patch(const struct TokenList *tokens, size_t call_start, size_t call_end, struct SafeCrtPatchList *out) {
  /* Pattern: FILE *f = fopen(path, mode); 
     or f = fopen(path, mode); 
     Find the assignment target to rewrite it as fopen_s(&f, path, mode); 
  */
  size_t lparen = 0, comma1 = 0, rparen = 0, i;
  char *path = NULL;
  char *mode = NULL;
  char *dest = NULL;
  char replacement[1024] = {0};
  
  /* Look backwards for '=' to find assignment target */
  size_t assign_idx = 0;
  for (i = call_start; i > 0; --i) {
    if (tokens->tokens[i].kind == TOKEN_ASSIGN) {
      assign_idx = i;
      break;
    }
    if (tokens->tokens[i].kind == TOKEN_SEMICOLON || tokens->tokens[i].kind == TOKEN_LBRACE || tokens->tokens[i].kind == TOKEN_RBRACE) {
      break; /* Stop looking if we cross statement boundaries */
    }
  }

  for (i = call_start; i < call_end; ++i) {
    if (tokens->tokens[i].kind == TOKEN_LPAREN && lparen == 0) lparen = i;
    else if (tokens->tokens[i].kind == TOKEN_COMMA && comma1 == 0) comma1 = i;
    else if (tokens->tokens[i].kind == TOKEN_RPAREN) rparen = i;
  }

  if (assign_idx != 0 && lparen != 0 && comma1 != 0 && rparen != 0) {
    size_t id_idx = assign_idx;
    path = extract_token_text(tokens, lparen + 1, comma1);
    mode = extract_token_text(tokens, comma1 + 1, rparen);

    /* Go left of '=' to find the identifier */
    while (id_idx > 0) {
      id_idx--;
      if (tokens->tokens[id_idx].kind == TOKEN_IDENTIFIER) {
        break;
      }
    }

    dest = extract_token_text(tokens, id_idx, id_idx + 1);

    if (path && mode && dest) {
      sprintf(replacement,
        "#if defined(_MSC_VER)\n"
        "  fopen_s(&%s, %s, %s);\n"
        "#else\n"
        "  %s = fopen(%s, %s);\n"
        "#endif", dest, path, mode, dest, path, mode);
      add_patch(out, assign_idx - 1, call_end, replacement);
    }    
    if (path) free(path);
    if (mode) free(mode);
    if (dest) free(dest);
  }
  return 0;
}

static int generate_strncpy_patch(const struct TokenList *tokens, size_t call_start, size_t call_end, struct SafeCrtPatchList *out) {
  /* Pattern: strncpy(dest, src, count) */
  size_t lparen = 0, comma1 = 0, comma2 = 0, rparen = 0, i;
  char *dest = NULL, *src = NULL, *count = NULL;
  char replacement[1024] = {0};

  for (i = call_start; i < call_end; ++i) {
    if (tokens->tokens[i].kind == TOKEN_LPAREN && lparen == 0) lparen = i;
    else if (tokens->tokens[i].kind == TOKEN_COMMA) {
      if (comma1 == 0) comma1 = i;
      else if (comma2 == 0) comma2 = i;
    }
    else if (tokens->tokens[i].kind == TOKEN_RPAREN && rparen == 0) rparen = i;
  }

  if (lparen != 0 && comma1 != 0 && comma2 != 0 && rparen != 0) {
    dest = extract_token_text(tokens, lparen + 1, comma1);
    src = extract_token_text(tokens, comma1 + 1, comma2);
    count = extract_token_text(tokens, comma2 + 1, rparen);
    
    if (dest && src && count) {
      sprintf(replacement, 
        "#if defined(_MSC_VER)\n"
        "  strncpy_s(%s, sizeof(%s), %s, %s);\n"
        "#else\n"
        "  strncpy(%s, %s, %s);\n"
        "#endif", dest, dest, src, count, dest, src, count);
      add_patch(out, call_start, call_end, replacement);
    }
    
    if (dest) free(dest);
    if (src) free(src);
    if (count) free(count);
  }
  return 0;
}

static int generate_sprintf_patch(const struct TokenList *tokens, size_t call_start, size_t call_end, struct SafeCrtPatchList *out) {
  /* Pattern: sprintf(dest, format, ...) */
  size_t lparen = 0, comma1 = 0, rparen = 0, i;
  char *dest = NULL;
  char *args = NULL;
  char replacement[2048] = {0};

  for (i = call_start; i < call_end; ++i) {
    if (tokens->tokens[i].kind == TOKEN_LPAREN && lparen == 0) lparen = i;
    else if (tokens->tokens[i].kind == TOKEN_COMMA && comma1 == 0) comma1 = i;
    else if (tokens->tokens[i].kind == TOKEN_RPAREN) rparen = i; /* Use the last one for ... */
  }

  if (lparen != 0 && comma1 != 0 && rparen != 0) {
    dest = extract_token_text(tokens, lparen + 1, comma1);
    args = extract_token_text(tokens, comma1 + 1, rparen);
    
    if (dest && args) {
      sprintf(replacement, 
        "#if defined(_MSC_VER)\n"
        "  sprintf_s(%s, sizeof(%s), %s);\n"
        "#else\n"
        "  sprintf(%s, %s);\n"
        "#endif", dest, dest, args, dest, args);
      add_patch(out, call_start, call_end, replacement);
    }
    
    if (dest) free(dest);
    if (args) free(args);
  }
  return 0;
}

int cst_generate_safe_crt_patches(const struct CstNodeList *cst,
                                  const struct TokenList *tokens,
                                  struct SafeCrtPatchList *out_patches) {
  size_t i, j;
  if (!cst || !tokens || !out_patches) return EINVAL;

  for (i = 0; i < cst->size; ++i) {
    const struct CstNode *n = &cst->nodes[i];
    
    /* Search for function calls inside OTHER nodes */
    for (j = n->start_token; j < n->end_token; ++j) {
      if (tokens->tokens[j].kind == TOKEN_IDENTIFIER) {
        size_t len = tokens->tokens[j].length;
        const char *str = (const char *)tokens->tokens[j].start;
        
        if (len == 6 && strncmp(str, "strcpy", 6) == 0) {
          size_t end = j;
          while (end < n->end_token && tokens->tokens[end].kind != TOKEN_SEMICOLON) {
            end++;
          }
          if (end < n->end_token) {
            generate_strcpy_patch(tokens, j, end + 1, out_patches);
            j = end;
          }
        } else if (len == 7 && strncmp(str, "strncpy", 7) == 0) {
          size_t end = j;
          while (end < n->end_token && tokens->tokens[end].kind != TOKEN_SEMICOLON) {
            end++;
          }
          if (end < n->end_token) {
            generate_strncpy_patch(tokens, j, end + 1, out_patches);
            j = end;
          }
        } else if (len == 7 && strncmp(str, "sprintf", 7) == 0) {
          size_t end = j;
          while (end < n->end_token && tokens->tokens[end].kind != TOKEN_SEMICOLON) {
            end++;
          }
          if (end < n->end_token) {
            generate_sprintf_patch(tokens, j, end + 1, out_patches);
            j = end;
          }
        } else if (len == 5 && strncmp(str, "fopen", 5) == 0) {
           size_t end = j;
          while (end < n->end_token && tokens->tokens[end].kind != TOKEN_SEMICOLON) {
            end++;
          }
          if (end < n->end_token) {
            generate_fopen_patch(tokens, j, end + 1, out_patches);
            j = end;
          }
        }
      }
    }
    
    /* VLA Scan: Look for `type id[expr];` where expr is not just an integer literal */
    for (j = n->start_token; j < n->end_token; ++j) {
      if ((tokens->tokens[j].kind == TOKEN_KEYWORD_CHAR || 
           tokens->tokens[j].kind == TOKEN_KEYWORD_INT || 
           tokens->tokens[j].kind == TOKEN_KEYWORD_DOUBLE) && j + 3 < n->end_token) {
           
        if (tokens->tokens[j+1].kind == TOKEN_IDENTIFIER && tokens->tokens[j+2].kind == TOKEN_LBRACKET) {
           /* potential array declaration */
           size_t end_bracket = j + 3;
           int is_vla = 0;
           while (end_bracket < n->end_token && tokens->tokens[end_bracket].kind != TOKEN_RBRACKET) {
             if (tokens->tokens[end_bracket].kind == TOKEN_IDENTIFIER) {
                is_vla = 1; /* Expression involves variable -> VLA */
             }
             end_bracket++;
           }
           
           if (is_vla && end_bracket < n->end_token) {
             /* found a VLA */
             char *type_name = extract_token_text(tokens, j, j + 1);
             char *var_name = extract_token_text(tokens, j + 1, j + 2);
             char *expr = extract_token_text(tokens, j + 3, end_bracket);
             char replacement[1024] = {0};
             
             if (type_name && var_name && expr) {
               size_t end_stmt = end_bracket;
               sprintf(replacement, 
                 "#if defined(_MSC_VER)\n"
                 "  %s *%s = (%s*)_alloca((%s) * sizeof(%s));\n"
                 "#else\n"
                 "  %s %s[%s];\n"
                 "#endif", type_name, var_name, type_name, expr, type_name, type_name, var_name, expr);
               
               /* statement usually ends with ; */
               while (end_stmt < n->end_token && tokens->tokens[end_stmt].kind != TOKEN_SEMICOLON) {
                 end_stmt++;
               }
               if (end_stmt < n->end_token) {
                 add_patch(out_patches, j, end_stmt + 1, replacement);
                 /* Only skip to end_bracket, so we don't skip the rest of the node loop processing */
                 j = end_bracket;
               }
             }
             if (type_name) free(type_name);
             if (var_name) free(var_name);
             if (expr) free(expr);
           }
        }
      }
    }
  }

  return 0;
}
