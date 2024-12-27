#ifndef C_CDD_SCANNER_TYPES_H
#define C_CDD_SCANNER_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <c_cdd_export.h>
#include <c_str_span.h>

/* print(",\\\n".join(
 *       map("{0}##Suffix".format,
 *           map(lambda _s: _s.partition(" ")[0],s.splitlines()))))
 */

#define KeywordEnumGen(Suffix)                                                 \
        alignas##Suffix,                                                       \
        alignof##Suffix,                                                       \
        auto##Suffix,                                                          \
        bool##Suffix,                                                          \
        break##Suffix,                                                         \
        case##Suffix,                                                          \
        char##Suffix,                                                          \
        const##Suffix,                                                         \
        constexpr##Suffix,                                                     \
        continue##Suffix,                                                      \
        default##Suffix,                                                       \
        do##Suffix,                                                            \
        double##Suffix,                                                        \
        else##Suffix,                                                          \
        enum##Suffix,                                                          \
        extern##Suffix,                                                        \
        false##Suffix,                                                         \
        float##Suffix,                                                         \
        for##Suffix,                                                           \
        goto##Suffix,                                                          \
        if##Suffix,                                                            \
        inline##Suffix,                                                        \
        int##Suffix,                                                           \
        long##Suffix,                                                          \
        nullptr##Suffix,                                                       \
        register##Suffix,                                                      \
        restrict##Suffix,                                                      \
        return##Suffix,                                                        \
        short##Suffix,                                                         \
        signed##Suffix,                                                        \
        sizeof##Suffix,                                                        \
        static##Suffix,                                                        \
        static_assert##Suffix,                                                 \
        struct##Suffix,                                                        \
        switch##Suffix,                                                        \
        thread_local##Suffix,                                                  \
        true##Suffix,                                                          \
        typedef##Suffix,                                                       \
        typeof##Suffix,                                                        \
        typeof_unqual##Suffix,                                                 \
        union##Suffix,                                                         \
        unsigned##Suffix,                                                      \
        void##Suffix,                                                          \
        volatile##Suffix,                                                      \
        while##Suffix,                                                         \
        _Alignas##Suffix,                                                      \
        _Alignof##Suffix,                                                      \
        _Atomic##Suffix,                                                       \
        _BitInt##Suffix,                                                       \
        _Bool##Suffix,                                                         \
        _Complex##Suffix,                                                      \
        _Decimal128##Suffix,                                                   \
        _Decimal32##Suffix,                                                    \
        _Decimal64##Suffix,                                                    \
        _Generic##Suffix,                                                      \
        _Imaginary##Suffix,                                                    \
        _Noreturn##Suffix,                                                     \
        _Static_assert##Suffix,                                                \
        _Thread_local##Suffix

enum TokenizerKind {
  /* except one newline char that terminates CPP_COMMENT or MACRO */
  WHITESPACE,

  MACRO,

  /*Integer,
  Decimal,*/
  DOUBLE_QUOTED,
  SINGLE_QUOTED,

  C_COMMENT,
  CPP_COMMENT,

  LBRACE,
  RBRACE,
  LSQUARE,
  RSQUARE,
  LPAREN,
  RPAREN,

  EQUAL,
  EQ_OP,
  GREATER_THAN,
  GE_OP,
  LESS_THAN,
  LE_OP,
  NE_OP,

  RIGHT_SHIFT,
  LEFT_SHIFT,

  /* unary */
  AND,
  ASTERISK,
  PLUS,
  SUB,
  TILDE,
  EXCLAMATION,
  MODULO,

  INC_OP,
  DEC_OP,

  PTR_OP,

  DIVIDE,
  CARET,
  PIPE,

  AND_OP,
  OR_OP,

  MUL_ASSIGN,
  DIV_ASSIGN,
  MOD_ASSIGN,
  ADD_ASSIGN,
  SUB_ASSIGN,
  LEFT_ASSIGN,
  RIGHT_ASSIGN,
  AND_ASSIGN,
  XOR_ASSIGN,
  OR_ASSIGN,

  QUESTION,
  COLON,

  ELLIPSIS,

  WORD,

  COMMA,
  TERMINATOR, /* ; */

  KeywordEnumGen(Keyword),

  UNKNOWN_SCAN,
};

enum TokenKeyword {
  KeywordEnumGen(Token),

  unknownKeyword
};

struct StrTokenizerKind {
  char *s;
  enum TokenizerKind kind;
};

struct tokenizer_az_span_elem {
  az_span span;
  enum TokenizerKind kind;
};

struct tokenizer_az_span_arr {
  struct tokenizer_az_span_elem *elem;
  size_t size;
};

extern C_CDD_EXPORT const char *TokenizerKind_to_str(enum TokenizerKind);

extern C_CDD_EXPORT enum TokenizerKind str_to_TokenizerKind(const char *);

extern C_CDD_EXPORT const char *TokenKeyword_to_str(enum TokenKeyword);

extern C_CDD_EXPORT enum TokenKeyword str_to_TokenKeyword(const char *);

extern C_CDD_EXPORT void
tokenizer_az_span_elem_arr_cleanup(struct tokenizer_az_span_arr *);

extern C_CDD_EXPORT void
tokenizer_az_span_arr_print(const struct tokenizer_az_span_arr *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#undef KeywordEnumGen

#endif /* !C_CDD_SCANNER_TYPES_H */
