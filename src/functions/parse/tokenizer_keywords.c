/**
 * @file tokenizer_keywords.c
 * @brief Keyword recognition helpers for lexical analyzer.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <stddef.h>
#if defined(_MSC_VER) && _MSC_VER < 1600
#include "msvc/stdint.h"
#else
#if defined(_MSC_VER) && _MSC_VER < 1600
#include "c_cdd/msvc/stdint.h"
#else
#include <stdint.h>
#endif
#endif

#include "cdd_c_error.h"
#include "functions/parse/tokenizer.h"
#include "functions/parse/tokenizer_keywords.h"
/* clang-format on */

/**
 * @brief Executes the span equals str operation.
 */
cdd_c_error_t span_equals_str(const az_span span, const char *str,
                              int *_out_val) {
  if (!_out_val) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  *_out_val = 0;
  if (!str) {
    return CDD_C_SUCCESS;
  }

  {
    *_out_val = ((int)az_span_is_content_equal(
        span, az_span_create_from_str((char *)(size_t)str)));
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the identify keyword or id operation.
 */
cdd_c_error_t identify_keyword_or_id(const uint8_t *start, size_t len,
                                     enum TokenKind *_out_val) {
  int _ast_attr = 0;
  int _ast_declspec = 0;
  int _ast_span_equals_str_0 = 0;
  int _ast_span_equals_str_1 = 0;
  int _ast_span_equals_str_2 = 0;
  int _ast_span_equals_str_3 = 0;
  int _ast_span_equals_str_4 = 0;
  int _ast_span_equals_str_5 = 0;
  int _ast_span_equals_str_6 = 0;
  int _ast_span_equals_str_7 = 0;
  int _ast_span_equals_str_8 = 0;
  int _ast_span_equals_str_9 = 0;
  int _ast_span_equals_str_10 = 0;
  int _ast_span_equals_str_11 = 0;
  int _ast_span_equals_str_12 = 0;
  int _ast_span_equals_str_13 = 0;
  int _ast_span_equals_str_14 = 0;
  int _ast_span_equals_str_15 = 0;
  int _ast_span_equals_str_16 = 0;
  int _ast_span_equals_str_17 = 0;
  int _ast_span_equals_str_18 = 0;
  int _ast_span_equals_str_19 = 0;
  int _ast_span_equals_str_20 = 0;
  int _ast_span_equals_str_21 = 0;
  int _ast_span_equals_str_22 = 0;
  int _ast_span_equals_str_23 = 0;
  int _ast_span_equals_str_24 = 0;
  int _ast_span_equals_str_25 = 0;
  int _ast_span_equals_str_26 = 0;
  int _ast_span_equals_str_27 = 0;
  int _ast_span_equals_str_28 = 0;
  int _ast_span_equals_str_29 = 0;
  int _ast_span_equals_str_30 = 0;
  int _ast_span_equals_str_31 = 0;
  int _ast_span_equals_str_32 = 0;
  int _ast_span_equals_str_33 = 0;
  int _ast_span_equals_str_34 = 0;
  int _ast_span_equals_str_35 = 0;
  int _ast_span_equals_str_36 = 0;
  int _ast_span_equals_str_37 = 0;
  int _ast_span_equals_str_38 = 0;
  int _ast_span_equals_str_39 = 0;
  int _ast_span_equals_str_40 = 0;
  int _ast_span_equals_str_41 = 0;
  int _ast_span_equals_str_42 = 0;
  int _ast_span_equals_str_43 = 0;
  int _ast_span_equals_str_44 = 0;
  int _ast_span_equals_str_45 = 0;
  int _ast_span_equals_str_46 = 0;
  int _ast_span_equals_str_47 = 0;
  int _ast_span_equals_str_48 = 0;
  int _ast_span_equals_str_49 = 0;
  int _ast_span_equals_str_50 = 0;
  int _ast_span_equals_str_51 = 0;
  int _ast_span_equals_str_52 = 0;
  int _ast_span_equals_str_53 = 0;
  int _ast_span_equals_str_54 = 0;
  int _ast_span_equals_str_55 = 0;
  int _ast_span_equals_str_56 = 0;
  az_span s;
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cdd_fail_identify_keyword_or_id;
  if (g_cdd_fail_identify_keyword_or_id &&
      --g_cdd_fail_identify_keyword_or_id == 0) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  if (!_out_val) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  *_out_val = TOKEN_IDENTIFIER;
  if (!start) {
    return CDD_C_SUCCESS;
  }
  s = az_span_create((uint8_t *)start, (size_t)len);

  /* C89/C90/C99/C11/C23 Keywords */

  if ((span_equals_str(s, "auto", &_ast_span_equals_str_0),
       _ast_span_equals_str_0)) {
    *_out_val = TOKEN_KEYWORD_AUTO;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "break", &_ast_span_equals_str_1),
       _ast_span_equals_str_1)) {
    *_out_val = TOKEN_KEYWORD_BREAK;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "case", &_ast_span_equals_str_2),
       _ast_span_equals_str_2)) {
    *_out_val = TOKEN_KEYWORD_CASE;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "char", &_ast_span_equals_str_3),
       _ast_span_equals_str_3)) {
    *_out_val = TOKEN_KEYWORD_CHAR;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "const", &_ast_span_equals_str_4),
       _ast_span_equals_str_4)) {
    *_out_val = TOKEN_KEYWORD_CONST;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "continue", &_ast_span_equals_str_5),
       _ast_span_equals_str_5)) {
    *_out_val = TOKEN_KEYWORD_CONTINUE;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "default", &_ast_span_equals_str_6),
       _ast_span_equals_str_6)) {
    *_out_val = TOKEN_KEYWORD_DEFAULT;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "do", &_ast_span_equals_str_7),
       _ast_span_equals_str_7)) {
    *_out_val = TOKEN_KEYWORD_DO;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "double", &_ast_span_equals_str_8),
       _ast_span_equals_str_8)) {
    *_out_val = TOKEN_KEYWORD_DOUBLE;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "else", &_ast_span_equals_str_9),
       _ast_span_equals_str_9)) {
    *_out_val = TOKEN_KEYWORD_ELSE;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "enum", &_ast_span_equals_str_10),
       _ast_span_equals_str_10)) {
    *_out_val = TOKEN_KEYWORD_ENUM;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "extern", &_ast_span_equals_str_11),
       _ast_span_equals_str_11)) {
    *_out_val = TOKEN_KEYWORD_EXTERN;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "float", &_ast_span_equals_str_12),
       _ast_span_equals_str_12)) {
    *_out_val = TOKEN_KEYWORD_FLOAT;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "for", &_ast_span_equals_str_13),
       _ast_span_equals_str_13)) {
    *_out_val = TOKEN_KEYWORD_FOR;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "goto", &_ast_span_equals_str_14),
       _ast_span_equals_str_14)) {
    *_out_val = TOKEN_KEYWORD_GOTO;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "if", &_ast_span_equals_str_15),
       _ast_span_equals_str_15)) {
    *_out_val = TOKEN_KEYWORD_IF;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "inline", &_ast_span_equals_str_16),
       _ast_span_equals_str_16)) {
    *_out_val = TOKEN_KEYWORD_INLINE;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "int", &_ast_span_equals_str_17),
       _ast_span_equals_str_17)) {
    *_out_val = TOKEN_KEYWORD_INT;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "long", &_ast_span_equals_str_18),
       _ast_span_equals_str_18)) {
    *_out_val = TOKEN_KEYWORD_LONG;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "register", &_ast_span_equals_str_19),
       _ast_span_equals_str_19)) {
    *_out_val = TOKEN_KEYWORD_REGISTER;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "restrict", &_ast_span_equals_str_20),
       _ast_span_equals_str_20)) {
    *_out_val = TOKEN_KEYWORD_RESTRICT;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "return", &_ast_span_equals_str_21),
       _ast_span_equals_str_21)) {
    *_out_val = TOKEN_KEYWORD_RETURN;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "short", &_ast_span_equals_str_22),
       _ast_span_equals_str_22)) {
    *_out_val = TOKEN_KEYWORD_SHORT;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "signed", &_ast_span_equals_str_23),
       _ast_span_equals_str_23)) {
    *_out_val = TOKEN_KEYWORD_SIGNED;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "sizeof", &_ast_span_equals_str_24),
       _ast_span_equals_str_24)) {
    *_out_val = TOKEN_KEYWORD_SIZEOF;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "static", &_ast_span_equals_str_25),
       _ast_span_equals_str_25)) {
    *_out_val = TOKEN_KEYWORD_STATIC;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "struct", &_ast_span_equals_str_26),
       _ast_span_equals_str_26)) {
    *_out_val = TOKEN_KEYWORD_STRUCT;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "switch", &_ast_span_equals_str_27),
       _ast_span_equals_str_27)) {
    *_out_val = TOKEN_KEYWORD_SWITCH;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "typedef", &_ast_span_equals_str_28),
       _ast_span_equals_str_28)) {
    *_out_val = TOKEN_KEYWORD_TYPEDEF;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "union", &_ast_span_equals_str_29),
       _ast_span_equals_str_29)) {
    *_out_val = TOKEN_KEYWORD_UNION;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "unsigned", &_ast_span_equals_str_30),
       _ast_span_equals_str_30)) {
    *_out_val = TOKEN_KEYWORD_UNSIGNED;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "void", &_ast_span_equals_str_31),
       _ast_span_equals_str_31)) {
    *_out_val = TOKEN_KEYWORD_VOID;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "volatile", &_ast_span_equals_str_32),
       _ast_span_equals_str_32)) {
    *_out_val = TOKEN_KEYWORD_VOLATILE;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "while", &_ast_span_equals_str_33),
       _ast_span_equals_str_33)) {
    *_out_val = TOKEN_KEYWORD_WHILE;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "_Alignas", &_ast_span_equals_str_34),
       _ast_span_equals_str_34)) {
    *_out_val = TOKEN_KEYWORD_ALIGNAS;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "_Alignof", &_ast_span_equals_str_35),
       _ast_span_equals_str_35)) {
    *_out_val = TOKEN_KEYWORD_ALIGNOF;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "_Atomic", &_ast_span_equals_str_36),
       _ast_span_equals_str_36)) {
    *_out_val = TOKEN_KEYWORD_ATOMIC;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "_Bool", &_ast_span_equals_str_37),
       _ast_span_equals_str_37)) {
    *_out_val = TOKEN_KEYWORD_BOOL;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "_Complex", &_ast_span_equals_str_38),
       _ast_span_equals_str_38)) {
    *_out_val = TOKEN_KEYWORD_COMPLEX;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "_Imaginary", &_ast_span_equals_str_39),
       _ast_span_equals_str_39)) {
    *_out_val = TOKEN_KEYWORD_IMAGINARY;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "_Noreturn", &_ast_span_equals_str_40),
       _ast_span_equals_str_40)) {
    *_out_val = TOKEN_KEYWORD_NORETURN;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "_Static_assert", &_ast_span_equals_str_41),
       _ast_span_equals_str_41)) {
    *_out_val = TOKEN_KEYWORD_STATIC_ASSERT;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "_Thread_local", &_ast_span_equals_str_42),
       _ast_span_equals_str_42)) {
    *_out_val = TOKEN_KEYWORD_THREAD_LOCAL;
    return CDD_C_SUCCESS;
  }

  /* Extensions found in common headers */

  if ((span_equals_str(s, "__inline", &_ast_span_equals_str_43),
       _ast_span_equals_str_43)) {
    *_out_val = TOKEN_KEYWORD_INLINE;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "__restrict", &_ast_span_equals_str_44),
       _ast_span_equals_str_44)) {
    *_out_val = TOKEN_KEYWORD_RESTRICT;
    return CDD_C_SUCCESS;
  }

  /* C23 standard keywords */

  if ((span_equals_str(s, "alignas", &_ast_span_equals_str_45),
       _ast_span_equals_str_45)) {
    *_out_val = TOKEN_KEYWORD_ALIGNAS;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "alignof", &_ast_span_equals_str_46),
       _ast_span_equals_str_46)) {
    *_out_val = TOKEN_KEYWORD_ALIGNOF;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "bool", &_ast_span_equals_str_47),
       _ast_span_equals_str_47)) {
    *_out_val = TOKEN_KEYWORD_BOOL;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "constexpr", &_ast_span_equals_str_48),
       _ast_span_equals_str_48)) {
    *_out_val = TOKEN_KEYWORD_CONSTEXPR;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "false", &_ast_span_equals_str_49),
       _ast_span_equals_str_49)) {
    *_out_val = TOKEN_KEYWORD_FALSE;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "nullptr", &_ast_span_equals_str_50),
       _ast_span_equals_str_50)) {
    *_out_val = TOKEN_KEYWORD_NULLPTR;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "static_assert", &_ast_span_equals_str_51),
       _ast_span_equals_str_51)) {
    *_out_val = TOKEN_KEYWORD_STATIC_ASSERT;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "thread_local", &_ast_span_equals_str_52),
       _ast_span_equals_str_52)) {
    *_out_val = TOKEN_KEYWORD_THREAD_LOCAL;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "true", &_ast_span_equals_str_53),
       _ast_span_equals_str_53)) {
    *_out_val = TOKEN_KEYWORD_TRUE;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "typeof", &_ast_span_equals_str_54),
       _ast_span_equals_str_54)) {
    *_out_val = TOKEN_KEYWORD_TYPEOF;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "embed", &_ast_span_equals_str_55),
       _ast_span_equals_str_55)) {
    *_out_val = TOKEN_KEYWORD_EMBED;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "_Pragma", &_ast_span_equals_str_56),
       _ast_span_equals_str_56)) {
    *_out_val = TOKEN_KEYWORD_PRAGMA_OP;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "__attribute__", &_ast_attr), _ast_attr)) {
    *_out_val = TOKEN_KEYWORD_ATTRIBUTE;
    return CDD_C_SUCCESS;
  }

  if ((span_equals_str(s, "__declspec", &_ast_declspec), _ast_declspec)) {
    *_out_val = TOKEN_KEYWORD_DECLSPEC;
    return CDD_C_SUCCESS;
  }

  {
    *_out_val = TOKEN_IDENTIFIER;
    return CDD_C_SUCCESS;
  }
}
