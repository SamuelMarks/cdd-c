#ifndef C_CDD_CST_PARSER_TYPES_H
#define C_CDD_CST_PARSER_TYPES_H

#define CST_WITH_BODY

#ifdef __cplusplus
#include <cstdlib>
#include <cstring>
extern "C" {
#else
#include <stdlib.h>
#include <string.h>

#if __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#else
#include <c_cdd_stdbool.h>
#endif /* __STDC_VERSION__ >= 199901L */

#endif /* __cplusplus */

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#include <BaseTsd.h>
#include <errno.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/errno.h>
#endif
#include "c_cdd_export.h"
#include <c_str_span.h>

#define CstNode_base_properties                                                \
  size_t pos_start; /* where in full-source `value` starts */                  \
  struct CstNode *scope;                                                       \
  const az_span value

struct BlockStart {
  CstNode_base_properties;
};

struct BlockEnd {
  CstNode_base_properties;
};

/* mostly from reviewing http://www.quut.com/c/ANSI-C-grammar-y.html */

struct CppComment {
  CstNode_base_properties;
};

struct CComment {
  CstNode_base_properties;
};

enum StorageClass { EXTERN, STATIC, THREAD_LOCAL, AUTO, REGISTER };
static const char *const storage_classes[] = {
    "extern", "static", "thread_local", "auto", "register"};

enum TypeSpecifier /* & TypeQualifier */ {
  VOID,
  CHAR,
  SHORT,
  INT,
  LONG,
  FLOAT,
  DOUBLE,
  SIGNED,
  UNSIGNED,
  BOOL,
  COMPLEX,
  IMAGINARY,
  /*}; enum TypeQualifier {*/
  CONST,
  RESTRICT,
  VOLATILE,
  ATOMIC
};

enum FunctionSpecifier { INLINE, NORETURN };

enum Keywords {
  BREAK,
  CASE,
  CONTINUE,
  DEFAULT,
  DO,
  ELSE,
  ENUM,
  FOR,
  GOTO,
  IF,
  RETURN,
  SIZEOF,
  STRUCT,
  SWITCH,
  TYPEDEF,
  UNION,
  WHILE,
  ALIGNAS,
  ALIGNOF,
  GENERIC,
  STATIC_ASSERT,
  FUNC_NAME
};

/*
static const std::unordered_map<std::string, enum Keywords> keyword2enum {
  {"auto", AUTO},
  {"break", BREAK},
  {"case", CASE},
  {"char", CHAR},
  {"const", CONST},
  {"continue", CONTINUE},
  {"default", DEFAULT},
  {"do", DO},
  {"double", DOUBLE},
  {"else", ELSE},
  {"enum", ENUM},
  {"extern", EXTERN},
  {"float", FLOAT},
  {"for", FOR},
  {"goto", GOTO},
  {"if", IF},
  {"inline", INLINE},
  {"int", INT},
  {"long", LONG},
  {"register", REGISTER},
  {"restrict", RESTRICT},
  {"return", RETURN},
  {"short", SHORT},
  {"signed", SIGNED},
  {"sizeof", SIZEOF},
  {"static", STATIC},
  {"struct", STRUCT},
  {"switch", SWITCH},
  {"typedef", TYPEDEF},
  {"union", UNION},
  {"unsigned", UNSIGNED},
  {"void", VOID},
  {"volatile", VOLATILE},
  {"while", WHILE},
  {"_Alignas", ALIGNAS},
  {"_Alignof", ALIGNOF},
  {"_Atomic", ATOMIC},
  {"_Bool", BOOL},
  {"_Complex", COMPLEX},
  {"_Generic", GENERIC},
  {"_Imaginary", IMAGINARY},
  {"_Noreturn", NORETURN},
  {"_Static_assert", STATIC_ASSERT},
  {"_Thread_local", THREAD_LOCAL},
  {"__func__", FUNC_NAME}
};
*/

/* enum StorageClassSpecifier {
   TYPEDEF, // identifiers must be flagged as TYPEDEF_NAME
   EXTERN,
   STATIC,
   THREAD_LOCAL,
   AUTO,
   REGISTER
 };

 enum TypeSpecifier {
   VOID,
   CHAR,
   SHORT,
   INT,
   LONG,
   FLOAT,
   DOUBLE,
   SIGNED,
   UNSIGNED,
   BOOL,
   COMPLEX,
   IMAGINARY        // non-mandated extension
   ,
   / atomic_type_specifier,
   struct_or_union_specifier,
   enum_specifier,
   TYPEDEF_NAME   /     // after it has been defined as such
 };
*/

struct Expression {
  CstNode_base_properties;
}; /* fallback if nothing else matches */

struct Label {
  CstNode_base_properties;
  const az_span label;
};

struct Case {
  CstNode_base_properties;
  const az_span val;
};

struct Switch {
  CstNode_base_properties;
  const az_span condition;
#ifdef CST_WITH_BODY
  struct CstNode *body;
#endif /* CST_WITH_BODY */
};

struct If {
  CstNode_base_properties;
  const az_span condition;
#ifdef CST_WITH_BODY
  struct CstNode *body;
#endif /* CST_WITH_BODY */
};

struct Else {
  CstNode_base_properties;
#ifdef CST_WITH_BODY
  struct CstNode *body;
#endif /* CST_WITH_BODY */
};

struct ElseIf {
  CstNode_base_properties;
  const az_span condition;
#ifdef CST_WITH_BODY
  struct CstNode *body;
#endif /* CST_WITH_BODY */
};

struct While {
  CstNode_base_properties;
  const az_span condition;
#ifdef CST_WITH_BODY
  struct CstNode *body;
#endif /* CST_WITH_BODY */
};

struct Do {
  CstNode_base_properties;
#ifdef CST_WITH_BODY
  struct CstNode *body;
#endif /* CST_WITH_BODY */
};

/* struct DoWhile { CstNode_base_properties; }; */

struct For {
  CstNode_base_properties;
  const az_span decl_or_expr0, *decl_or_expr1, *expr;
#ifdef CST_WITH_BODY
  struct CstNode *body;
#endif /* CST_WITH_BODY */
};

struct GoTo {
  CstNode_base_properties;
  const az_span label;
};

struct Continue {
  CstNode_base_properties;
};

struct Break {
  CstNode_base_properties;
};

struct Return {
  CstNode_base_properties;
  const az_span val;
};

#define Declaration_properties                                                 \
  enum StorageClass storage_class;                                             \
  enum TypeSpecifier *type_specifier;                                          \
  enum Keywords *specifiers;                                                   \
  const az_span type; /* set to NULL if `specifiers` has the right type [i.e., \
                       builtin type] */                                        \
  const az_span name

struct Declaration {
  CstNode_base_properties;
  Declaration_properties;
};

struct Definition {
  CstNode_base_properties;
  Declaration_properties;
  const az_span value_assigned;
};

struct Struct {
  CstNode_base_properties;
  const az_span name;
  struct Declaration **fields;
  const az_span *declaration_list;
};

struct Union {
  CstNode_base_properties;
  const az_span name;
  struct Declaration **fields;
  const az_span *declaration_list;
};

struct Enum {
  CstNode_base_properties;
  const az_span name;
  struct Declaration **fields;
  const az_span *enumerator_list;
};

struct FunctionPrototype {
  CstNode_base_properties;
  enum FunctionSpecifier function_specifier;
  const az_span name;
  struct Declaration **args;
};

struct FunctionStart {
  CstNode_base_properties;
};

struct Function {
  CstNode_base_properties;
  enum Keywords *specifiers;
  const az_span type; /* set to NULL if `specifiers` has the right type [i.e.,
                       builtin type] */
  az_span name;
  struct Declaration *args;
#ifdef CST_WITH_BODY
  struct CstNode *body;
#endif /* CST_WITH_BODY */
};

struct MacroIf {
  CstNode_base_properties;
  const az_span expr;
#ifdef CST_WITH_BODY
  struct CstNode *body;
#endif /* CST_WITH_BODY */
};

struct MacroElif {
  CstNode_base_properties;
  const az_span expr;
#ifdef CST_WITH_BODY
  struct CstNode *body;
#endif /* CST_WITH_BODY */
};

struct MacroElse {
  CstNode_base_properties;
  const az_span expr;
#ifdef CST_WITH_BODY
  struct CstNode *body;
#endif /* CST_WITH_BODY */
};

struct MacroIfDef {
  CstNode_base_properties;
  const az_span expr;
#ifdef CST_WITH_BODY
  struct CstNode *body;
#endif /* CST_WITH_BODY */
};

struct MacroDefine {
  CstNode_base_properties;
  const az_span expr;
};

struct MacroInclude {
  CstNode_base_properties;
  const az_span val;
};

struct MacroPragma {
  CstNode_base_properties;
  const az_span val;
};

enum CstNodeKind {
  Expression,
  BlockStart,
  BlockEnd,

  Label,
  Case,
  Switch,
  If,
  Else,
  ElseIf,
  While,
  Do,
  For,
  GoTo,
  Continue,
  Break,
  Return,
  Declaration,
  Definition,
  Struct,
  Union,
  Enum,
  FunctionPrototype,
  Function,

  MacroIf,
  MacroElif,
  MacroIfDef,
  MacroElse,
  MacroDefine,
  MacroInclude,
  MacroPragma,

  Sentinel /* NUL value */
};

union CstNodeType {
  struct Expression *expression;
  struct BlockStart *block_start;
  struct BlockEnd *block_end;

  struct Label *label;
  struct Case *case_;
  struct Switch *switch_;
  struct If *if_;
  struct Else *else_;
  struct ElseIf *elseif;
  struct While *while_;
  struct Do *do_;
  struct For *for_;
  struct GoTo *goto_;
  struct Continue *continue_;
  struct Break *break_;
  struct Return *return_;
  struct Declaration *declaration;
  struct Definition *definition;
  struct Struct *struct_;
  struct Union *union_;
  struct Enum *enum_;
  struct FunctionPrototype *function_prototype;
  struct Function *function;

  struct MacroIf *macro_if;
  struct MacroElif *macro_elif;
  struct MacroIfDef *macro_if_def;
  struct MacroElse *macro_else;
  struct MacroDefine *macro_define;
  struct MacroInclude *macro_include;
  struct MacroPragma *macro_pragma;
};

struct CstNode {
  enum CstNodeKind kind;
  void *type;
};
static const struct CstNode CST_NODE_SENTINEL = {Sentinel, NULL};

struct cst_node_arr {
  struct CstNode *elem;
  size_t size;
};

struct Cst_Node {
  enum CstNodeKind kind;
  struct tokenizer_az_span_element **src;
};

extern C_CDD_EXPORT const char *CstNodeKind_to_str(enum CstNodeKind);

extern C_CDD_EXPORT enum CstNodeKind str_to_CstNodeKind(const char *);

extern C_CDD_EXPORT void cst_node_arr_cleanup(const struct cst_node_arr *);

#endif /* !C_CDD_CST_PARSER_TYPES_H */
