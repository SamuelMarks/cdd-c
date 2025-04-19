#ifndef CST_PARSER_H
#define CST_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <c_cdd_export.h>

#include "tokenizer.h"

C_CDD_EXPORT enum CstNodeKind1 {
  CST_NODE_STRUCT,
  CST_NODE_ENUM,
  CST_NODE_UNION,
  CST_NODE_COMMENT,
  CST_NODE_MACRO,
  CST_NODE_WHITESPACE,
  CST_NODE_OTHER,
  CST_NODE_UNKNOWN
};

C_CDD_EXPORT struct CstNode1 {
  enum CstNodeKind1 kind;
  const uint8_t *start;
  size_t length;
  /* For struct/enum/union: member list etc could be added here */
};

C_CDD_EXPORT struct CstNodeList {
  struct CstNode1 *nodes;
  size_t size;
  size_t capacity;
};

extern C_CDD_EXPORT int add_node(struct CstNodeList *, enum CstNodeKind1,
                                 const uint8_t *, size_t);

extern C_CDD_EXPORT int parse_tokens(const struct TokenList *,
                                     struct CstNodeList *);

extern C_CDD_EXPORT void free_cst_node_list(struct CstNodeList *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CST_PARSER_H */
