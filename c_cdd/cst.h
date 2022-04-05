#ifndef C_CDD_CST_H
#define C_CDD_CST_H

#include <stdlib.h>
#include <string.h>

#include <c_cdd_export.h>

struct CstNode;

#define CstNode_base_properties                                                \
  size_t pos_start; /* where in full-source `value` starts */                  \
  struct CstNode *scope;                                                       \
  const char *value

/* two phase parser */
extern C_CDD_EXPORT const char **scanner(const char *);
extern C_CDD_EXPORT const struct CstNode **parser(const char **);

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

enum Keywords {
  AUTO,
  BREAK,
  CASE,
  CHAR,
  CONST,
  CONTINUE,
  DEFAULT,
  DO,
  DOUBLE,
  ELSE,
  ENUM,
  EXTERN,
  FLOAT,
  FOR,
  GOTO,
  IF,
  INLINE,
  INT,
  LONG,
  REGISTER,
  RESTRICT,
  RETURN,
  SHORT,
  SIGNED,
  SIZEOF,
  STATIC,
  STRUCT,
  SWITCH,
  TYPEDEF,
  UNION,
  UNSIGNED,
  VOID,
  VOLATILE,
  WHILE,
  ALIGNAS,
  ALIGNOF,
  ATOMIC,
  BOOL,
  COMPLEX,
  GENERIC,
  IMAGINARY,
  NORETURN,
  STATIC_ASSERT,
  THREAD_LOCAL,
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

// enum StorageClassSpecifier {
//   TYPEDEF, /* identifiers must be flagged as TYPEDEF_NAME */
//   EXTERN,
//   STATIC,
//   THREAD_LOCAL,
//   AUTO,
//   REGISTER
// };
//
// enum TypeSpecifier {
//   VOID,
//   CHAR,
//   SHORT,
//   INT,
//   LONG,
//   FLOAT,
//   DOUBLE,
//   SIGNED,
//   UNSIGNED,
//   BOOL,
//   COMPLEX,
//   IMAGINARY        /* non-mandated extension */
//   ,
//   /*atomic_type_specifier,
//   struct_or_union_specifier,
//   enum_specifier,
//   TYPEDEF_NAME   */     /* after it has been defined as such */
// };

struct Expression {
  CstNode_base_properties;
}; /* fallback if nothing else matches */

struct Label {
  CstNode_base_properties;
  const char *label;
};

struct Case {
  CstNode_base_properties;
  const char *val;
};

struct Switch {
  CstNode_base_properties;
  const char *condition;
};

struct If {
  CstNode_base_properties;
  const char *condition;
};

struct Else {
  CstNode_base_properties;
};

struct ElseIf {
  CstNode_base_properties;
  const char *condition;
};

struct While {
  CstNode_base_properties;
  const char *condition;
};

struct Do {
  CstNode_base_properties;
};

// struct DoWhile { CstNode_base_properties; };

struct For {
  CstNode_base_properties;
  const char *decl_or_expr0, decl_or_expr1, expr;
};

struct GoTo {
  CstNode_base_properties;
  const char *label;
};

struct Continue {
  CstNode_base_properties;
};

struct Break {
  CstNode_base_properties;
};

struct Return {
  CstNode_base_properties;
  const char *val;
};

struct Declaration {
  CstNode_base_properties;
  enum Keywords **specifiers;
  const char *name;
};

struct Struct {
  CstNode_base_properties;
  const char *name;
  struct Declaration **fields;
};

struct Union {
  CstNode_base_properties;
  const char *name;
  struct Declaration **fields;
};

struct Enum {
  CstNode_base_properties;
  const char *name;
  struct Declaration **fields;
};

struct FunctionPrototype {
  CstNode_base_properties;
  const char *name;
  struct Declaration **args;
};

struct Arg {
  CstNode_base_properties;
  enum Keywords *specifiers;
  const char *name;
};

struct FunctionStart {
  CstNode_base_properties;
};

struct Function {
  CstNode_base_properties;
  enum Keywords *specifiers;
  const char *name;
  struct Arg **args;
  struct CstNode **body;
};

struct MacroIf {
  CstNode_base_properties;
  const char *expr;
};

struct MacroElif {
  CstNode_base_properties;
  const char *expr;
};

struct MacroIfDef {
  CstNode_base_properties;
  const char *expr;
};

struct MacroDefine {
  CstNode_base_properties;
  const char *expr;
};

struct MacroInclude {
  CstNode_base_properties;
  const char *val;
};

struct MacroPragma {
  CstNode_base_properties;
  const char *val;
};

enum CstNodeType {
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
  Struct,
  Union,
  Enum,
  FunctionPrototype,
  Function,

  MacroIf,
  MacroElif,
  MacroIfDef,
  MacroDefine,
  MacroInclude,
  MacroPragma
};

struct CstNode {
  enum CstNodeType kind;
  union {
    struct Expression expression;
    struct BlockStart block_start;
    struct BlockEnd block_end;

    struct Label label;
    struct Case _case;
    struct Switch _switch;
    struct If _if;
    struct Else _else;
    struct ElseIf elseif;
    struct While _while;
    struct Do _do;
    struct For _for;
    struct GoTo _goto;
    struct Continue _continue;
    struct Break _break;
    struct Return _return;
    struct Declaration declaration;
    struct Struct _struct;
    struct Union _union;
    struct Enum _enum;
    struct FunctionPrototype function_prototype;
    struct Function function;

    struct MacroIf macro_if;
    struct MacroElif macro_elif;
    struct MacroIfDef macro_if_def;
    struct MacroDefine macro_define;
    struct MacroInclude macro_include;
    struct MacroPragma macro_pragma;
  };
};

#endif /* !C_CDD_CST_H */
