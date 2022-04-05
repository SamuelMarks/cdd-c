#ifndef C_CDD_CST_H
#define C_CDD_CST_H

#include <cstdlib>
#include <string>
#include <unordered_map>

namespace cdd {
namespace cst {

struct CstNode {
  size_t line_no_start, line_no_end;
  const char *value;
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
struct Label : CstNode {};
struct Switch : CstNode {};
struct If : CstNode {};
struct Else : CstNode {};
struct ElseIf : CstNode {};
struct While : CstNode {};
struct Do : CstNode {};
struct DoWhile : CstNode {};
struct For : CstNode {};
struct GoTo : CstNode {};
struct Continue : CstNode {};
struct Break : CstNode {};
struct Return : CstNode {};
struct Declaration : CstNode {};
struct Struct : CstNode {};
struct Union : CstNode {};
struct Emum : CstNode {};
struct FunctionPrototype : CstNode {};
struct Function : CstNode {};
struct FunctionArgs : CstNode {};

namespace macro {
struct MacroIf : CstNode {};
struct MacroElif : CstNode {};
struct MacroIfDef : CstNode {};
struct MacroDefine : CstNode {};
struct MacroInclude : CstNode {};
struct MacroPragma : CstNode {};
}; // namespace macro
}; // namespace cst
}; // namespace cdd

#endif /* !C_CDD_CST_H */
