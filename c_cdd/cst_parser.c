#include <stdlib.h>
#include <string.h>

#include "cst_parser.h"

#include <sys/errno.h>

int add_node(struct CstNodeList *const list, const enum CstNodeKind1 kind,
             const char *const start, const size_t length) {
  if (list->size >= list->capacity) {
    const size_t new_capacity = list->capacity == 0 ? 64 : list->capacity * 2;
    struct CstNode1 *new_nodes = realloc(
        list->nodes, new_capacity * sizeof(*new_nodes));
    if (!new_nodes)
      return ENOMEM;
    list->nodes = new_nodes;
    list->capacity = new_capacity;
  }
  list->nodes[list->size].kind = kind;
  list->nodes[list->size].start = start;
  list->nodes[list->size].length = length;
  list->size++;
  return 0;
}

int parse_tokens(const struct TokenList *const tokens,
                 struct CstNodeList *const out) {
  size_t i = 0;
  while (i < tokens->size) {
    struct Token tok = tokens->tokens[i];
    const size_t start = i;
    switch (tok.kind) {
    case TOKEN_KEYWORD_STRUCT:
    case TOKEN_KEYWORD_ENUM:
    case TOKEN_KEYWORD_UNION: {
      enum CstNodeKind1 node_kind;
      size_t brace_count;
      switch (tok.kind) {
      case TOKEN_KEYWORD_STRUCT:
        node_kind = CST_NODE_STRUCT;
        break;
      case TOKEN_KEYWORD_ENUM:
        node_kind = CST_NODE_ENUM;
        break;
      case TOKEN_KEYWORD_UNION:
      default:
        node_kind = CST_NODE_UNION;
      }

      i++;

      if (i < tokens->size && tokens->tokens[i].kind == TOKEN_IDENTIFIER)
        i++;

      if (i >= tokens->size || tokens->tokens[i].kind != TOKEN_LBRACE) {
        const size_t end = i;
        const size_t length = tokens->tokens[end - 1].start +
                              tokens->tokens[end - 1].length - tok.start;
        if (add_node(out, node_kind, tok.start, length) != 0)
          return -1;
        continue;
      }

      brace_count = 1;
      i++;

      while (i < tokens->size && brace_count > 0) {
        if (tokens->tokens[i].kind == TOKEN_LBRACE)
          brace_count++;
        else if (tokens->tokens[i].kind == TOKEN_RBRACE)
          brace_count--;
        i++;
      }
      {
        const size_t length =
          tokens->tokens[i - 1].start + tokens->tokens[i - 1].length - tok.start;
        if (add_node(out, node_kind, tok.start, length) != 0)
          return -1;
      }
      break;
    }
    case TOKEN_COMMENT:
      if (add_node(out, CST_NODE_COMMENT, tok.start, tok.length) != 0)
        return -1;
      i++;
      break;
    case TOKEN_MACRO:
      if (add_node(out, CST_NODE_MACRO, tok.start, tok.length) != 0)
        return -1;
      i++;
      break;
    case TOKEN_WHITESPACE:
      if (add_node(out, CST_NODE_WHITESPACE, tok.start, tok.length) != 0)
        return -1;
      i++;
      break;
    default:
      if (add_node(out, CST_NODE_OTHER, tok.start, tok.length) != 0)
        return -1;
      i++;
      break;
    }
  }
  return 0;
}

void free_cst_node_list(struct CstNodeList *const list) {
  free(list->nodes);
  list->nodes = NULL;
  list->size = 0;
  list->capacity = 0;
}
