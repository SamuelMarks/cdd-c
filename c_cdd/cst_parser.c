#include <stdlib.h>
#include <string.h>

#include "cst_parser.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#else
#include <sys/errno.h>
#endif

int add_node(struct CstNodeList *const list, const enum CstNodeKind1 kind,
             const uint8_t *const start, const size_t length) {
  if (list->size >= list->capacity) {
    const size_t new_capacity = list->capacity == 0 ? 64 : list->capacity * 2;
    struct CstNode1 *new_nodes =
        realloc(list->nodes, new_capacity * sizeof(*new_nodes));
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
    const struct Token tok = tokens->tokens[i];
    switch (tok.kind) {
    case TOKEN_KEYWORD_STRUCT:
    case TOKEN_KEYWORD_ENUM:
    case TOKEN_KEYWORD_UNION: {
      enum CstNodeKind1 node_kind = CST_NODE_UNKNOWN;
      size_t end_pos = i;

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
        break;
      }
      end_pos++;

      /* Skip optional identifier */
      if (end_pos < tokens->size &&
          tokens->tokens[end_pos].kind == TOKEN_IDENTIFIER) {
        end_pos++;
      }

      /* Skip whitespace to find '{' or ';' */
      while (end_pos < tokens->size &&
             tokens->tokens[end_pos].kind == TOKEN_WHITESPACE) {
        end_pos++;
      }

      if (end_pos < tokens->size &&
          tokens->tokens[end_pos].kind == TOKEN_LBRACE) {
        size_t brace_count = 1;
        end_pos++;
        while (end_pos < tokens->size && brace_count > 0) {
          if (tokens->tokens[end_pos].kind == TOKEN_LBRACE)
            brace_count++;
          else if (tokens->tokens[end_pos].kind == TOKEN_RBRACE)
            brace_count--;
          end_pos++;
        }
      }

      {
        const size_t length = tokens->tokens[end_pos - 1].start +
                              tokens->tokens[end_pos - 1].length - tok.start;
        if (add_node(out, node_kind, tok.start, length) != 0)
          return -1;
        i = end_pos;
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
