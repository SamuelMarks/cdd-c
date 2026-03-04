#ifndef tokenizer_json_TESTS_H
#define tokenizer_json_TESTS_H
/* Auto-generated test source from JSON Schema tokenizer.json */

#include <stdlib.h>
#include <string.h>

#include <greatest.h>

#include "src/functions/parse/tokenizer.h"

/* Test enum TokenKind to_str/from_str */
TEST test_TokenKind_to_str_from_str(void) {
  char *str = NULL;
  enum TokenKind val;
  int rc;

  rc = TokenKind_to_str(TokenKind_TOKEN_WHITESPACE, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("TOKEN_WHITESPACE", str);
  free(str);

  rc = TokenKind_to_str(TokenKind______Spaces, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< Spaces", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_tabs, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("tabs", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_newlines_____TOKEN_COMMENT, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("newlines */  TOKEN_COMMENT", str);
  free(str);

  rc = TokenKind_to_str(
      TokenKind______Single_line______or_block_comments_____TOKEN_MACRO, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< Single line (//) or block comments */  TOKEN_MACRO", str);
  free(str);

  rc = TokenKind_to_str(
      TokenKind______Preprocessor_directives___________________Keywords_________TOKEN_KEYWORD_STRUCT,
      &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< Preprocessor directives (#...) */  /* --- Keywords --- "
                "*/  TOKEN_KEYWORD_STRUCT",
                str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______struct______TOKEN_KEYWORD_ENUM, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'struct' */  TOKEN_KEYWORD_ENUM", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______enum______TOKEN_KEYWORD_UNION, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'enum' */  TOKEN_KEYWORD_UNION", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______union______TOKEN_KEYWORD_IF, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'union' */  TOKEN_KEYWORD_IF", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______if______TOKEN_KEYWORD_ELSE, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'if' */  TOKEN_KEYWORD_ELSE", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______else______TOKEN_KEYWORD_WHILE, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'else' */  TOKEN_KEYWORD_WHILE", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______while______TOKEN_KEYWORD_DO, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'while' */  TOKEN_KEYWORD_DO", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______do______TOKEN_KEYWORD_FOR, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'do' */  TOKEN_KEYWORD_FOR", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______for______TOKEN_KEYWORD_RETURN, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'for' */  TOKEN_KEYWORD_RETURN", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______return______TOKEN_KEYWORD_SWITCH, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'return' */  TOKEN_KEYWORD_SWITCH", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______switch______TOKEN_KEYWORD_CASE, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'switch' */  TOKEN_KEYWORD_CASE", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______case______TOKEN_KEYWORD_DEFAULT, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'case' */  TOKEN_KEYWORD_DEFAULT", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______default______TOKEN_KEYWORD_BREAK, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'default' */  TOKEN_KEYWORD_BREAK", str);
  free(str);

  rc =
      TokenKind_to_str(TokenKind_______break______TOKEN_KEYWORD_CONTINUE, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'break' */  TOKEN_KEYWORD_CONTINUE", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______continue______TOKEN_KEYWORD_GOTO, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'continue' */  TOKEN_KEYWORD_GOTO", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______goto______TOKEN_KEYWORD_TYPEDEF, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'goto' */  TOKEN_KEYWORD_TYPEDEF", str);
  free(str);

  rc =
      TokenKind_to_str(TokenKind_______typedef______TOKEN_KEYWORD_EXTERN, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'typedef' */  TOKEN_KEYWORD_EXTERN", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______extern______TOKEN_KEYWORD_STATIC, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'extern' */  TOKEN_KEYWORD_STATIC", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______static______TOKEN_KEYWORD_AUTO, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'static' */  TOKEN_KEYWORD_AUTO", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______auto______TOKEN_KEYWORD_REGISTER, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'auto' */  TOKEN_KEYWORD_REGISTER", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______register______TOKEN_KEYWORD_INLINE,
                        &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'register' */  TOKEN_KEYWORD_INLINE", str);
  free(str);

  rc = TokenKind_to_str(
      TokenKind_______inline__or____inline______TOKEN_KEYWORD_CONST, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'inline' or '__inline' */  TOKEN_KEYWORD_CONST", str);
  free(str);

  rc =
      TokenKind_to_str(TokenKind_______const______TOKEN_KEYWORD_VOLATILE, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'const' */  TOKEN_KEYWORD_VOLATILE", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______volatile______TOKEN_KEYWORD_RESTRICT,
                        &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'volatile' */  TOKEN_KEYWORD_RESTRICT", str);
  free(str);

  rc = TokenKind_to_str(
      TokenKind_______restrict__or____restrict______TOKEN_KEYWORD_SIZEOF, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'restrict' or '__restrict' */  TOKEN_KEYWORD_SIZEOF",
                str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______sizeof______TOKEN_KEYWORD_VOID, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'sizeof' */  TOKEN_KEYWORD_VOID", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______void______TOKEN_KEYWORD_CHAR, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'void' */  TOKEN_KEYWORD_CHAR", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______char______TOKEN_KEYWORD_SHORT, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'char' */  TOKEN_KEYWORD_SHORT", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______short______TOKEN_KEYWORD_INT, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'short' */  TOKEN_KEYWORD_INT", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______int______TOKEN_KEYWORD_LONG, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'int' */  TOKEN_KEYWORD_LONG", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______long______TOKEN_KEYWORD_FLOAT, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'long' */  TOKEN_KEYWORD_FLOAT", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______float______TOKEN_KEYWORD_DOUBLE, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'float' */  TOKEN_KEYWORD_DOUBLE", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______double______TOKEN_KEYWORD_SIGNED, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'double' */  TOKEN_KEYWORD_SIGNED", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______signed______TOKEN_KEYWORD_UNSIGNED,
                        &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'signed' */  TOKEN_KEYWORD_UNSIGNED", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______unsigned______TOKEN_KEYWORD_BOOL, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'unsigned' */  TOKEN_KEYWORD_BOOL", str);
  free(str);

  rc = TokenKind_to_str(
      TokenKind________Bool__or__bool______TOKEN_KEYWORD_COMPLEX, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< '_Bool' or 'bool' */  TOKEN_KEYWORD_COMPLEX", str);
  free(str);

  rc = TokenKind_to_str(TokenKind________Complex______TOKEN_KEYWORD_IMAGINARY,
                        &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< '_Complex' */  TOKEN_KEYWORD_IMAGINARY", str);
  free(str);

  rc = TokenKind_to_str(TokenKind________Imaginary______TOKEN_KEYWORD_ATOMIC,
                        &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< '_Imaginary' */  TOKEN_KEYWORD_ATOMIC", str);
  free(str);

  rc = TokenKind_to_str(TokenKind________Atomic______TOKEN_KEYWORD_THREAD_LOCAL,
                        &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< '_Atomic' */  TOKEN_KEYWORD_THREAD_LOCAL", str);
  free(str);

  rc = TokenKind_to_str(
      TokenKind________Thread_local__or__thread_local______TOKEN_KEYWORD_ALIGNAS,
      &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ(
      "/**< '_Thread_local' or 'thread_local' */  TOKEN_KEYWORD_ALIGNAS", str);
  free(str);

  rc = TokenKind_to_str(
      TokenKind________Alignas__or__alignas______TOKEN_KEYWORD_ALIGNOF, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< '_Alignas' or 'alignas' */  TOKEN_KEYWORD_ALIGNOF", str);
  free(str);

  rc = TokenKind_to_str(
      TokenKind________Alignof__or__alignof______TOKEN_KEYWORD_NORETURN, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< '_Alignof' or 'alignof' */  TOKEN_KEYWORD_NORETURN", str);
  free(str);

  rc = TokenKind_to_str(TokenKind________Noreturn______TOKEN_KEYWORD_CONSTEXPR,
                        &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< '_Noreturn' */  TOKEN_KEYWORD_CONSTEXPR", str);
  free(str);

  rc = TokenKind_to_str(
      TokenKind_______constexpr______TOKEN_KEYWORD_STATIC_ASSERT, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'constexpr' */  TOKEN_KEYWORD_STATIC_ASSERT", str);
  free(str);

  rc = TokenKind_to_str(
      TokenKind________Static_assert__or__static_assert______TOKEN_KEYWORD_TYPEOF,
      &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ(
      "/**< '_Static_assert' or 'static_assert' */  TOKEN_KEYWORD_TYPEOF", str);
  free(str);

  rc =
      TokenKind_to_str(TokenKind_______typeof______TOKEN_KEYWORD_NULLPTR, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'typeof' */  TOKEN_KEYWORD_NULLPTR", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______nullptr______TOKEN_KEYWORD_TRUE, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'nullptr' */  TOKEN_KEYWORD_TRUE", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______true______TOKEN_KEYWORD_FALSE, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'true' */  TOKEN_KEYWORD_FALSE", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______false______TOKEN_KEYWORD_EMBED, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'false' */  TOKEN_KEYWORD_EMBED", str);
  free(str);

  rc = TokenKind_to_str(
      TokenKind_______embed___C23_preprocessor______TOKEN_KEYWORD_PRAGMA_OP,
      &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< 'embed' (C23 preprocessor) */  TOKEN_KEYWORD_PRAGMA_OP",
                str);
  free(str);

  rc = TokenKind_to_str(
      TokenKind________Pragma___C99_operator_____________Identifiers___Literals_________TOKEN_IDENTIFIER,
      &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< '_Pragma' (C99 operator) */  /* --- Identifiers & "
                "Literals --- */  TOKEN_IDENTIFIER",
                str);
  free(str);

  rc = TokenKind_to_str(
      TokenKind______Alphanumeric_identifiers_____TOKEN_NUMBER_LITERAL, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< Alphanumeric identifiers */  TOKEN_NUMBER_LITERAL", str);
  free(str);

  rc = TokenKind_to_str(
      TokenKind______Integer_Float_literals_____TOKEN_STRING_LITERAL, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< Integer/Float literals */  TOKEN_STRING_LITERAL", str);
  free(str);

  rc = TokenKind_to_str(TokenKind______Quoted_strings_____TOKEN_CHAR_LITERAL,
                        &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< Quoted strings */  TOKEN_CHAR_LITERAL", str);
  free(str);

  rc = TokenKind_to_str(
      TokenKind______Quoted_characters____________Punctuators__Single___Multi_char__________TOKEN_LBRACE,
      &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< Quoted characters */  /* --- Punctuators (Single & "
                "Multi-char) --- */  TOKEN_LBRACE",
                str);
  free(str);

  rc = TokenKind_to_str(TokenKind______________TOKEN_RBRACE, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< '{' */  TOKEN_RBRACE", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_______, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("/**< '", str);
  free(str);

  rc = TokenKind_to_str(TokenKind_____, &str);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("' */", str);
  free(str);

  rc = TokenKind_from_str("TOKEN_WHITESPACE", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_TOKEN_WHITESPACE, val);

  rc = TokenKind_from_str("/**< Spaces", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind______Spaces, val);

  rc = TokenKind_from_str("tabs", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_tabs, val);

  rc = TokenKind_from_str("newlines */  TOKEN_COMMENT", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_newlines_____TOKEN_COMMENT, val);

  rc = TokenKind_from_str(
      "/**< Single line (//) or block comments */  TOKEN_MACRO", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind______Single_line______or_block_comments_____TOKEN_MACRO,
            val);

  rc = TokenKind_from_str("/**< Preprocessor directives (#...) */  /* --- "
                          "Keywords --- */  TOKEN_KEYWORD_STRUCT",
                          &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(
      TokenKind______Preprocessor_directives___________________Keywords_________TOKEN_KEYWORD_STRUCT,
      val);

  rc = TokenKind_from_str("/**< 'struct' */  TOKEN_KEYWORD_ENUM", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______struct______TOKEN_KEYWORD_ENUM, val);

  rc = TokenKind_from_str("/**< 'enum' */  TOKEN_KEYWORD_UNION", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______enum______TOKEN_KEYWORD_UNION, val);

  rc = TokenKind_from_str("/**< 'union' */  TOKEN_KEYWORD_IF", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______union______TOKEN_KEYWORD_IF, val);

  rc = TokenKind_from_str("/**< 'if' */  TOKEN_KEYWORD_ELSE", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______if______TOKEN_KEYWORD_ELSE, val);

  rc = TokenKind_from_str("/**< 'else' */  TOKEN_KEYWORD_WHILE", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______else______TOKEN_KEYWORD_WHILE, val);

  rc = TokenKind_from_str("/**< 'while' */  TOKEN_KEYWORD_DO", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______while______TOKEN_KEYWORD_DO, val);

  rc = TokenKind_from_str("/**< 'do' */  TOKEN_KEYWORD_FOR", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______do______TOKEN_KEYWORD_FOR, val);

  rc = TokenKind_from_str("/**< 'for' */  TOKEN_KEYWORD_RETURN", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______for______TOKEN_KEYWORD_RETURN, val);

  rc = TokenKind_from_str("/**< 'return' */  TOKEN_KEYWORD_SWITCH", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______return______TOKEN_KEYWORD_SWITCH, val);

  rc = TokenKind_from_str("/**< 'switch' */  TOKEN_KEYWORD_CASE", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______switch______TOKEN_KEYWORD_CASE, val);

  rc = TokenKind_from_str("/**< 'case' */  TOKEN_KEYWORD_DEFAULT", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______case______TOKEN_KEYWORD_DEFAULT, val);

  rc = TokenKind_from_str("/**< 'default' */  TOKEN_KEYWORD_BREAK", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______default______TOKEN_KEYWORD_BREAK, val);

  rc = TokenKind_from_str("/**< 'break' */  TOKEN_KEYWORD_CONTINUE", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______break______TOKEN_KEYWORD_CONTINUE, val);

  rc = TokenKind_from_str("/**< 'continue' */  TOKEN_KEYWORD_GOTO", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______continue______TOKEN_KEYWORD_GOTO, val);

  rc = TokenKind_from_str("/**< 'goto' */  TOKEN_KEYWORD_TYPEDEF", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______goto______TOKEN_KEYWORD_TYPEDEF, val);

  rc = TokenKind_from_str("/**< 'typedef' */  TOKEN_KEYWORD_EXTERN", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______typedef______TOKEN_KEYWORD_EXTERN, val);

  rc = TokenKind_from_str("/**< 'extern' */  TOKEN_KEYWORD_STATIC", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______extern______TOKEN_KEYWORD_STATIC, val);

  rc = TokenKind_from_str("/**< 'static' */  TOKEN_KEYWORD_AUTO", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______static______TOKEN_KEYWORD_AUTO, val);

  rc = TokenKind_from_str("/**< 'auto' */  TOKEN_KEYWORD_REGISTER", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______auto______TOKEN_KEYWORD_REGISTER, val);

  rc = TokenKind_from_str("/**< 'register' */  TOKEN_KEYWORD_INLINE", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______register______TOKEN_KEYWORD_INLINE, val);

  rc = TokenKind_from_str("/**< 'inline' or '__inline' */  TOKEN_KEYWORD_CONST",
                          &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______inline__or____inline______TOKEN_KEYWORD_CONST, val);

  rc = TokenKind_from_str("/**< 'const' */  TOKEN_KEYWORD_VOLATILE", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______const______TOKEN_KEYWORD_VOLATILE, val);

  rc = TokenKind_from_str("/**< 'volatile' */  TOKEN_KEYWORD_RESTRICT", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______volatile______TOKEN_KEYWORD_RESTRICT, val);

  rc = TokenKind_from_str(
      "/**< 'restrict' or '__restrict' */  TOKEN_KEYWORD_SIZEOF", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______restrict__or____restrict______TOKEN_KEYWORD_SIZEOF,
            val);

  rc = TokenKind_from_str("/**< 'sizeof' */  TOKEN_KEYWORD_VOID", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______sizeof______TOKEN_KEYWORD_VOID, val);

  rc = TokenKind_from_str("/**< 'void' */  TOKEN_KEYWORD_CHAR", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______void______TOKEN_KEYWORD_CHAR, val);

  rc = TokenKind_from_str("/**< 'char' */  TOKEN_KEYWORD_SHORT", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______char______TOKEN_KEYWORD_SHORT, val);

  rc = TokenKind_from_str("/**< 'short' */  TOKEN_KEYWORD_INT", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______short______TOKEN_KEYWORD_INT, val);

  rc = TokenKind_from_str("/**< 'int' */  TOKEN_KEYWORD_LONG", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______int______TOKEN_KEYWORD_LONG, val);

  rc = TokenKind_from_str("/**< 'long' */  TOKEN_KEYWORD_FLOAT", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______long______TOKEN_KEYWORD_FLOAT, val);

  rc = TokenKind_from_str("/**< 'float' */  TOKEN_KEYWORD_DOUBLE", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______float______TOKEN_KEYWORD_DOUBLE, val);

  rc = TokenKind_from_str("/**< 'double' */  TOKEN_KEYWORD_SIGNED", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______double______TOKEN_KEYWORD_SIGNED, val);

  rc = TokenKind_from_str("/**< 'signed' */  TOKEN_KEYWORD_UNSIGNED", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______signed______TOKEN_KEYWORD_UNSIGNED, val);

  rc = TokenKind_from_str("/**< 'unsigned' */  TOKEN_KEYWORD_BOOL", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______unsigned______TOKEN_KEYWORD_BOOL, val);

  rc = TokenKind_from_str("/**< '_Bool' or 'bool' */  TOKEN_KEYWORD_COMPLEX",
                          &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind________Bool__or__bool______TOKEN_KEYWORD_COMPLEX, val);

  rc = TokenKind_from_str("/**< '_Complex' */  TOKEN_KEYWORD_IMAGINARY", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind________Complex______TOKEN_KEYWORD_IMAGINARY, val);

  rc = TokenKind_from_str("/**< '_Imaginary' */  TOKEN_KEYWORD_ATOMIC", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind________Imaginary______TOKEN_KEYWORD_ATOMIC, val);

  rc =
      TokenKind_from_str("/**< '_Atomic' */  TOKEN_KEYWORD_THREAD_LOCAL", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind________Atomic______TOKEN_KEYWORD_THREAD_LOCAL, val);

  rc = TokenKind_from_str(
      "/**< '_Thread_local' or 'thread_local' */  TOKEN_KEYWORD_ALIGNAS", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(
      TokenKind________Thread_local__or__thread_local______TOKEN_KEYWORD_ALIGNAS,
      val);

  rc = TokenKind_from_str(
      "/**< '_Alignas' or 'alignas' */  TOKEN_KEYWORD_ALIGNOF", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind________Alignas__or__alignas______TOKEN_KEYWORD_ALIGNOF,
            val);

  rc = TokenKind_from_str(
      "/**< '_Alignof' or 'alignof' */  TOKEN_KEYWORD_NORETURN", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind________Alignof__or__alignof______TOKEN_KEYWORD_NORETURN,
            val);

  rc = TokenKind_from_str("/**< '_Noreturn' */  TOKEN_KEYWORD_CONSTEXPR", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind________Noreturn______TOKEN_KEYWORD_CONSTEXPR, val);

  rc = TokenKind_from_str("/**< 'constexpr' */  TOKEN_KEYWORD_STATIC_ASSERT",
                          &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______constexpr______TOKEN_KEYWORD_STATIC_ASSERT, val);

  rc = TokenKind_from_str(
      "/**< '_Static_assert' or 'static_assert' */  TOKEN_KEYWORD_TYPEOF",
      &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(
      TokenKind________Static_assert__or__static_assert______TOKEN_KEYWORD_TYPEOF,
      val);

  rc = TokenKind_from_str("/**< 'typeof' */  TOKEN_KEYWORD_NULLPTR", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______typeof______TOKEN_KEYWORD_NULLPTR, val);

  rc = TokenKind_from_str("/**< 'nullptr' */  TOKEN_KEYWORD_TRUE", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______nullptr______TOKEN_KEYWORD_TRUE, val);

  rc = TokenKind_from_str("/**< 'true' */  TOKEN_KEYWORD_FALSE", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______true______TOKEN_KEYWORD_FALSE, val);

  rc = TokenKind_from_str("/**< 'false' */  TOKEN_KEYWORD_EMBED", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______false______TOKEN_KEYWORD_EMBED, val);

  rc = TokenKind_from_str(
      "/**< 'embed' (C23 preprocessor) */  TOKEN_KEYWORD_PRAGMA_OP", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(
      TokenKind_______embed___C23_preprocessor______TOKEN_KEYWORD_PRAGMA_OP,
      val);

  rc = TokenKind_from_str("/**< '_Pragma' (C99 operator) */  /* --- "
                          "Identifiers & Literals --- */  TOKEN_IDENTIFIER",
                          &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(
      TokenKind________Pragma___C99_operator_____________Identifiers___Literals_________TOKEN_IDENTIFIER,
      val);

  rc = TokenKind_from_str(
      "/**< Alphanumeric identifiers */  TOKEN_NUMBER_LITERAL", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind______Alphanumeric_identifiers_____TOKEN_NUMBER_LITERAL,
            val);

  rc = TokenKind_from_str(
      "/**< Integer/Float literals */  TOKEN_STRING_LITERAL", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind______Integer_Float_literals_____TOKEN_STRING_LITERAL,
            val);

  rc = TokenKind_from_str("/**< Quoted strings */  TOKEN_CHAR_LITERAL", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind______Quoted_strings_____TOKEN_CHAR_LITERAL, val);

  rc = TokenKind_from_str("/**< Quoted characters */  /* --- Punctuators "
                          "(Single & Multi-char) --- */  TOKEN_LBRACE",
                          &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(
      TokenKind______Quoted_characters____________Punctuators__Single___Multi_char__________TOKEN_LBRACE,
      val);

  rc = TokenKind_from_str("/**< '{' */  TOKEN_RBRACE", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind______________TOKEN_RBRACE, val);

  rc = TokenKind_from_str("/**< '", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_______, val);

  rc = TokenKind_from_str("' */", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_____, val);

  rc = TokenKind_from_str("INVALID", &val);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(TokenKind_UNKNOWN, val);

  PASS();
}
/* Test Token default / deepcopy / eq / cleanup */
TEST test_Token_default_deepcopy_eq_cleanup(void) {
  struct Token *obj0 = NULL;
  struct Token *obj1 = NULL;
  int rc;

  rc = Token_default(&obj0);
  if (rc != 0 || obj0 == NULL)
    FAIL();

  rc = Token_deepcopy(obj0, &obj1);
  if (rc != 0 || obj1 == NULL) {
    Token_cleanup(obj0);
    FAIL();
  }

  ASSERT(Token_eq(obj0, obj1));

  /* Modify obj0 to break equality */
  /* skipping modifying fields because unknown type here */

  Token_cleanup(obj0);
  Token_cleanup(obj1);

  PASS();
}

TEST test_Token_json_roundtrip(void) {
  struct Token *obj_in = NULL;
  struct Token *obj_out = NULL;
  char *json_str = NULL;
  int rc;

  rc = Token_default(&obj_in);
  ASSERT_EQ(0, rc);
  ASSERT(obj_in != NULL);

  rc = Token_to_json(obj_in, &json_str);
  ASSERT_EQ(0, rc);
  ASSERT(json_str != NULL);

  rc = Token_from_json(json_str, &obj_out);
  ASSERT_EQ(0, rc);
  ASSERT(obj_out != NULL);

  ASSERT(Token_eq(obj_in, obj_out));

  free(json_str);
  Token_cleanup(obj_in);
  Token_cleanup(obj_out);

  PASS();
}

/* Test TokenList default / deepcopy / eq / cleanup */
TEST test_TokenList_default_deepcopy_eq_cleanup(void) {
  struct TokenList *obj0 = NULL;
  struct TokenList *obj1 = NULL;
  int rc;

  rc = TokenList_default(&obj0);
  if (rc != 0 || obj0 == NULL)
    FAIL();

  rc = TokenList_deepcopy(obj0, &obj1);
  if (rc != 0 || obj1 == NULL) {
    TokenList_cleanup(obj0);
    FAIL();
  }

  ASSERT(TokenList_eq(obj0, obj1));

  /* Modify obj0 to break equality */
  /* skipping modifying fields because unknown type here */

  TokenList_cleanup(obj0);
  TokenList_cleanup(obj1);

  PASS();
}

TEST test_TokenList_json_roundtrip(void) {
  struct TokenList *obj_in = NULL;
  struct TokenList *obj_out = NULL;
  char *json_str = NULL;
  int rc;

  rc = TokenList_default(&obj_in);
  ASSERT_EQ(0, rc);
  ASSERT(obj_in != NULL);

  rc = TokenList_to_json(obj_in, &json_str);
  ASSERT_EQ(0, rc);
  ASSERT(json_str != NULL);

  rc = TokenList_from_json(json_str, &obj_out);
  ASSERT_EQ(0, rc);
  ASSERT(obj_out != NULL);

  ASSERT(TokenList_eq(obj_in, obj_out));

  free(json_str);
  TokenList_cleanup(obj_in);
  TokenList_cleanup(obj_out);

  PASS();
}

/* Test suites */
SUITE(enums_suite) { RUN_TEST(test_TokenKind_to_str_from_str); }

SUITE(structs_suite) {
  RUN_TEST(test_Token_default_deepcopy_eq_cleanup);
  RUN_TEST(test_Token_json_roundtrip);
  RUN_TEST(test_TokenList_default_deepcopy_eq_cleanup);
  RUN_TEST(test_TokenList_json_roundtrip);
}

#endif /* !tokenizer_json_TESTS_H */
