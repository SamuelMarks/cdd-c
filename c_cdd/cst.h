#ifndef C_CDD_CST_H
#define C_CDD_CST_H

#include <cstdlib>
#include <list>
#include <string>
#include <unordered_map>

namespace cdd {
namespace cst {

struct CstNode {
  size_t line_no_start, line_no_end;
  std::list<CstNode *> scope;
  std::string value;
};

/* two phase parser */
const std::vector<std::string> scanner(const std::string &);
std::vector<CstNode> parser(const std::vector<std::string> &);

/* mostly from reviewing http://www.quut.com/c/ANSI-C-grammar-y.html */

struct CppComment : CstNode {};
struct CComment : CstNode {};

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

struct Expression : CstNode {}; /* fallback if nothing else matches */
struct Label : CstNode {
  std::string label;
};
struct Case : CstNode {
  std::string val;
};
struct Switch : CstNode {
  std::string condition;
};
struct If : CstNode {
  std::string condition;
};
struct Else : CstNode {};
struct ElseIf : CstNode {
  std::string condition;
};
struct While : CstNode {
  std::string condition;
};
struct Do : CstNode {};
// struct DoWhile : CstNode {};
struct For : CstNode {
  std::string decl_or_expr0, decl_or_expr1, expr;
};
struct GoTo : CstNode {
  std::string label;
};
struct Continue : CstNode {};
struct Break : CstNode {};
struct Return : CstNode {
  std::string val;
};
struct Declaration : CstNode {
  std::list<Keywords> specifiers;
  std::string name;
};
struct Struct : CstNode {
  std::string name;
  std::list<Declaration> fields;
};
struct Union : CstNode {
  std::string name;
  std::list<Declaration> fields;
};
struct Enum : CstNode {
  std::string name;
  std::list<Declaration> fields;
};
struct FunctionPrototype : CstNode {
  std::string name;
  std::list<Declaration> args;
};
struct Function : CstNode {
  std::list<Keywords> specifiers;
  std::string name;
  std::list<Declaration> args;
  std::list<CstNode> body;
};

namespace macro {
struct MacroIf : CstNode {
  std::string expr;
};
struct MacroElif : CstNode {
  std::string expr;
};
struct MacroIfDef : CstNode {
  std::string expr;
};
struct MacroDefine : CstNode {
  std::string expr;
};
struct MacroInclude : CstNode {
  std::string val;
};
struct MacroPragma : CstNode {
  std::string val;
};
}; // namespace macro
}; // namespace cst
}; // namespace cdd

#endif /* !C_CDD_CST_H */
