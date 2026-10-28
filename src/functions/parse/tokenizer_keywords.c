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
 * @brief Checks whether a byte span equals a null-terminated literal string.
 *
 * @param[in] span Byte span to compare.
 * @param[in] str Null-terminated literal string to compare against.
 * @param[out] _out_val Output pointer receiving 1 if equal, 0 otherwise.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT` if
 * NULL.
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
    *_out_val =
        ((int)az_span_is_content_equal(span, az_span_create_from_str(str)));
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Classifies an identifier token span as a C keyword or user identifier.
 *
 * @param[in] start Pointer to the start of the identifier token bytes.
 * @param[in] len Length of the identifier token in bytes.
 * @param[out] _out_val Output pointer receiving the classified `TokenKind`.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT` if
 * NULL.
 */
cdd_c_error_t identify_keyword_or_id(const uint8_t *start, size_t len,
                                     enum TokenKind *_out_val) {
  /** @brief Entry mapping a C keyword string to its `TokenKind`. */
  static const struct {
    /** @brief Keyword literal string */
    const char *word;
    /** @brief Corresponding token kind */
    enum TokenKind kind;
  } keywords[] = {/* C89/C90/C99/C11/C23 Keywords */
                  {"auto", TOKEN_KEYWORD_AUTO},
                  {"break", TOKEN_KEYWORD_BREAK},
                  {"case", TOKEN_KEYWORD_CASE},
                  {"char", TOKEN_KEYWORD_CHAR},
                  {"const", TOKEN_KEYWORD_CONST},
                  {"continue", TOKEN_KEYWORD_CONTINUE},
                  {"default", TOKEN_KEYWORD_DEFAULT},
                  {"do", TOKEN_KEYWORD_DO},
                  {"double", TOKEN_KEYWORD_DOUBLE},
                  {"else", TOKEN_KEYWORD_ELSE},
                  {"enum", TOKEN_KEYWORD_ENUM},
                  {"extern", TOKEN_KEYWORD_EXTERN},
                  {"float", TOKEN_KEYWORD_FLOAT},
                  {"for", TOKEN_KEYWORD_FOR},
                  {"goto", TOKEN_KEYWORD_GOTO},
                  {"if", TOKEN_KEYWORD_IF},
                  {"inline", TOKEN_KEYWORD_INLINE},
                  {"int", TOKEN_KEYWORD_INT},
                  {"long", TOKEN_KEYWORD_LONG},
                  {"register", TOKEN_KEYWORD_REGISTER},
                  {"restrict", TOKEN_KEYWORD_RESTRICT},
                  {"return", TOKEN_KEYWORD_RETURN},
                  {"short", TOKEN_KEYWORD_SHORT},
                  {"signed", TOKEN_KEYWORD_SIGNED},
                  {"sizeof", TOKEN_KEYWORD_SIZEOF},
                  {"static", TOKEN_KEYWORD_STATIC},
                  {"struct", TOKEN_KEYWORD_STRUCT},
                  {"switch", TOKEN_KEYWORD_SWITCH},
                  {"typedef", TOKEN_KEYWORD_TYPEDEF},
                  {"union", TOKEN_KEYWORD_UNION},
                  {"unsigned", TOKEN_KEYWORD_UNSIGNED},
                  {"void", TOKEN_KEYWORD_VOID},
                  {"volatile", TOKEN_KEYWORD_VOLATILE},
                  {"while", TOKEN_KEYWORD_WHILE},
                  {"_Alignas", TOKEN_KEYWORD_ALIGNAS},
                  {"_Alignof", TOKEN_KEYWORD_ALIGNOF},
                  {"_Atomic", TOKEN_KEYWORD_ATOMIC},
                  {"_Bool", TOKEN_KEYWORD_BOOL},
                  {"_Complex", TOKEN_KEYWORD_COMPLEX},
                  {"_Imaginary", TOKEN_KEYWORD_IMAGINARY},
                  {"_Noreturn", TOKEN_KEYWORD_NORETURN},
                  {"_Static_assert", TOKEN_KEYWORD_STATIC_ASSERT},
                  {"_Thread_local", TOKEN_KEYWORD_THREAD_LOCAL},
                  /* Extensions found in common headers */
                  {"__inline", TOKEN_KEYWORD_INLINE},
                  {"__restrict", TOKEN_KEYWORD_RESTRICT},
                  /* C23 standard keywords */
                  {"alignas", TOKEN_KEYWORD_ALIGNAS},
                  {"alignof", TOKEN_KEYWORD_ALIGNOF},
                  {"bool", TOKEN_KEYWORD_BOOL},
                  {"constexpr", TOKEN_KEYWORD_CONSTEXPR},
                  {"false", TOKEN_KEYWORD_FALSE},
                  {"nullptr", TOKEN_KEYWORD_NULLPTR},
                  {"static_assert", TOKEN_KEYWORD_STATIC_ASSERT},
                  {"thread_local", TOKEN_KEYWORD_THREAD_LOCAL},
                  {"true", TOKEN_KEYWORD_TRUE},
                  {"typeof", TOKEN_KEYWORD_TYPEOF},
                  {"embed", TOKEN_KEYWORD_EMBED},
                  {"_Pragma", TOKEN_KEYWORD_PRAGMA_OP},
                  {"__attribute__", TOKEN_KEYWORD_ATTRIBUTE},
                  {"__declspec", TOKEN_KEYWORD_DECLSPEC}};
  size_t i;
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

  for (i = 0; i < sizeof(keywords) / sizeof(keywords[0]); ++i) {
    size_t kw_len = strlen(keywords[i].word);
    if (kw_len == len && memcmp(start, keywords[i].word, len) == 0) {
      *_out_val = keywords[i].kind;
      return CDD_C_SUCCESS;
    }
  }

  *_out_val = TOKEN_IDENTIFIER;
  return CDD_C_SUCCESS;
}
