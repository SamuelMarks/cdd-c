#include "tokenizer_helpers.h"
#include "c_cdd_utils.h"
#include "tokenizer_types.h"
#include <assert.h>

#ifdef DEBUG_SCANNER
#include "str_includes.h"
#endif /* DEBUG_SCANNER */

size_t eatCComment(const az_span *const source, const size_t start_index,
                   const size_t n,
                   struct tokenizer_az_span_elem *const token_ptr) {
  size_t end_index;
  const uint8_t *const span_ptr = az_span_ptr(*source);
  for (end_index = start_index; end_index < n; end_index++) {
    if (span_ptr[end_index] == '/' && span_ptr[end_index - 1] == '*' &&
        span_ptr[end_index - 2] != '\\')
      break;
  }

  if (end_index > start_index) {
#ifdef DEBUG_SCANNER
    char *s;
    asprintf(&s, "eatCComment[%02" NUM_LONG_FMT "u:%02" NUM_LONG_FMT "u]",
             start_index, end_index);
    print_escaped_span(s, az_span_slice(*source, start_index, end_index + 1));
    free(s);
#endif /* DEBUG_SCANNER */
    token_ptr->kind = C_COMMENT,
    token_ptr->span = az_span_slice(*source, start_index, ++end_index);
  }
  return end_index - 1;
}

size_t eatCppComment(const az_span *const source, const size_t start_index,
                     const size_t n,
                     struct tokenizer_az_span_elem *const token_ptr) {
  size_t end_index;
  const uint8_t *const span_ptr = az_span_ptr(*source);
  for (end_index = start_index; end_index < n; end_index++) {
    const uint8_t ch = span_ptr[end_index], last_ch = span_ptr[end_index - 1];
    if (ch == '\n' && last_ch != '\\')
      break;
  }

  if (end_index > start_index) {
#ifdef DEBUG_SCANNER
    char *s;
    asprintf(&s, "eatCppComment[%02" NUM_LONG_FMT "u:%02" NUM_LONG_FMT "u]",
             start_index, end_index);
    print_escaped_span(s, az_span_slice(*source, start_index, end_index + 1));
    free(s);
#endif /* DEBUG_SCANNER */
    token_ptr->kind = CPP_COMMENT,
    token_ptr->span = az_span_slice(*source, start_index, ++end_index);
  }
  return end_index;
}

size_t eatMacro(const az_span *const source, const size_t start_index,
                const size_t n,
                struct tokenizer_az_span_elem *const token_ptr) {
  size_t end_index;
  const uint8_t *const span_ptr = az_span_ptr(*source);
  for (end_index = start_index; end_index < n; end_index++) {
    if (span_ptr[end_index] == '\n' && span_ptr[end_index - 1] != '\\')
      break;
  }

  if (end_index > start_index) {
#ifdef DEBUG_SCANNER
    char *s;
    asprintf(&s, "eatMacro[%02" NUM_LONG_FMT "u:%02" NUM_LONG_FMT "u]",
             start_index, end_index);
    print_escaped_span(s, az_span_slice(*source, start_index, end_index + 1));
    free(s);
#endif /* DEBUG_SCANNER */
    token_ptr->kind = MACRO,
    token_ptr->span = az_span_slice(*source, start_index, ++end_index);
  }
  return end_index - 1;
}

size_t eatCharLiteral(const az_span *const source, const size_t start_index,
                      const size_t n,
                      struct tokenizer_az_span_elem *const token_ptr) {
  /* Misses encoding prefix */
  size_t end_index;
  const uint8_t *const span_ptr = az_span_ptr(*source);
  for (end_index = start_index + 1; end_index < n; end_index++) {
    const uint8_t ch = span_ptr[end_index], last_ch = span_ptr[end_index - 1],
                  second_last_ch = span_ptr[end_index - 2],
                  third_last_ch = span_ptr[end_index - 3];
    if (ch == '\'' &&
        (last_ch != '\\' || (second_last_ch == '\'' && third_last_ch != '\\')))
      break;
  }

  {
#ifdef DEBUG_SCANNER
    char *s;
    asprintf(&s, "eatStrLiteral[%02" NUM_LONG_FMT "u:%02" NUM_LONG_FMT "u]",
             start_index, end_index);
    print_escaped_span(s, az_span_slice(*source, start_index, end_index + 1));
    free(s);
#endif /* DEBUG_SCANNER */
    token_ptr->kind = SINGLE_QUOTED,
    token_ptr->span = az_span_slice(*source, start_index, ++end_index);
  }
  return end_index - 1;
}

size_t eatStrLiteral(const az_span *const source, const size_t start_index,
                     const size_t n,
                     struct tokenizer_az_span_elem *const token_ptr) {
  /* Misses encoding prefix */
  size_t end_index;
  const uint8_t *const span_ptr = az_span_ptr(*source);
  for (end_index = start_index + 1; end_index < n; end_index++) {
    const uint8_t ch = span_ptr[end_index], last_ch = span_ptr[end_index - 1];
    if (ch == '"' && last_ch != '\\')
      break;
  }

  {
#ifdef DEBUG_SCANNER
    char *s;
    asprintf(&s, "eatStrLiteral[%02" NUM_LONG_FMT "u:%02" NUM_LONG_FMT "u]",
             start_index, end_index);
    print_escaped_span(s, az_span_slice(*source, start_index, end_index + 1));
    free(s);
#endif /* DEBUG_SCANNER */
    token_ptr->kind = DOUBLE_QUOTED,
    token_ptr->span = az_span_slice(*source, start_index, ++end_index);
  }
  return end_index - 1;
}

size_t eatWhitespace(const az_span *const source, const size_t start_index,
                     const size_t n,
                     struct tokenizer_az_span_elem *const token_ptr) {
  size_t end_index;
  const uint8_t *const span_ptr = az_span_ptr(*source);
  /*uint8_t ch;
  for (end_index = start_index + 1, ch = az_span_ptr(*source)[end_index];
       end_index < n &&
       (ch == '\n' || ch == '\r' || ch == '\t' ||
        ch == ' ') //isspace(az_span_ptr(*source)[end_index]);
       end_index++, ch = az_span_ptr(*source)[end_index]) {
  }*/
  for (end_index = start_index + 1; end_index < n; end_index++) {
    const uint8_t ch = span_ptr[end_index];
    switch (ch) {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
    case '\v':
      break;
    default:
      goto end;
    }
  }

end: {
#ifdef DEBUG_SCANNER
  {
    char *s;
    asprintf(&s, "eatWhitespace[%02" NUM_LONG_FMT "u:%02" NUM_LONG_FMT "u]",
             start_index, end_index);
    print_escaped_span(s, az_span_slice(*source, start_index, end_index));
    assert(s != NULL);
    assert(strlen(s) > 0);
    free(s);
  }
#endif /* DEBUG_SCANNER */
  token_ptr->kind = WHITESPACE,
  token_ptr->span = az_span_slice(*source, start_index, end_index);
}
  return end_index - 1;
}

void eatOneChar(const az_span *const source, const size_t start_index,
                struct tokenizer_az_span_elem *const token_ptr,
                const enum TokenizerKind kind) {
#ifdef DEBUG_SCANNER
  char *s;
  asprintf(&s, "eat%s[%02" NUM_LONG_FMT "u:%02" NUM_LONG_FMT "u]",
           TokenizerKind_to_str(kind), start_index, start_index + 1);
  print_escaped_span(s, az_span_slice(*source, start_index, start_index + 1));
  free(s);
#endif /* DEBUG_SCANNER */
  token_ptr->kind = kind,
  token_ptr->span = az_span_slice(*source, start_index, start_index + 1);
}

size_t eatSlice(const az_span *const source, const size_t start_index,
                const off_t offset,
                struct tokenizer_az_span_elem *const token_ptr,
                const enum TokenizerKind kind) {
  const size_t end_index = start_index + offset;
#ifdef DEBUG_SCANNER
  char *s;
  asprintf(&s, "eat%s[%02" NUM_LONG_FMT "u:%02" NUM_LONG_FMT "u]",
           TokenizerKind_to_str(kind), start_index, end_index);
  print_escaped_span(s, az_span_slice(*source, start_index, end_index));
  free(s);
#endif /* DEBUG_SCANNER */
  token_ptr->kind = kind,
  token_ptr->span = az_span_slice(*source, start_index, end_index);
  return end_index;
}

size_t eatWord(const az_span *const source, const size_t start_index,
               const size_t n, struct tokenizer_az_span_elem *const token_ptr) {
  size_t end_index;
  char *word;
  enum TokenizerKind kind = WORD;
  const uint8_t *const span_ptr = az_span_ptr(*source);
  for (end_index = start_index + 1; end_index < n; end_index++) {
    const uint8_t ch = span_ptr[end_index];
    switch (ch) {
    case '_':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
    case 'i':
    case 'j':
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 's':
    case 't':
    case 'u':
    case 'v':
    case 'w':
    case 'x':
    case 'y':
    case 'z':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
    case 'H':
    case 'I':
    case 'J':
    case 'K':
    case 'L':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
    case 'Q':
    case 'R':
    case 'S':
    case 'T':
    case 'U':
    case 'V':
    case 'W':
    case 'X':
    case 'Y':
    case 'Z':
      break;
    default:
      goto end;
    }
  }
end: {
  const az_span word_span = az_span_slice(*source, start_index, end_index);

  {
    const size_t word_span_n = az_span_size(word_span) + 1;
    word = malloc(word_span_n);
    if (word == NULL)
      exit(ENOMEM);
    az_span_to_str(word, word_span_n, word_span);
    /* The `str_to` functions should be O(1) so this shouldn't be *that*
     * inefficient */
    if (str_to_TokenKeyword(word) == unknownKeyword)
      word =
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
          _strdup
#else
          strdup
#endif
          ("Word");
    else
      kind = str_to_TokenizerKind(word);
  }
  {
#ifdef DEBUG_SCANNER
    char *s;
    asprintf(&s, "eat%s[%02" NUM_LONG_FMT "u:%02" NUM_LONG_FMT "u]", word,
             start_index, end_index);
    print_escaped_span(s, word_span);
    free(s);
#endif /* DEBUG_SCANNER */
    free(word);
  }
  token_ptr->kind = kind,
  token_ptr->span = az_span_slice(*source, start_index, end_index);
}
  return end_index - 1;
}

size_t eatNumber(const az_span *const source, const size_t start_index,
                 const size_t n,
                 struct tokenizer_az_span_elem *const token_ptr) {
  /* doesn't handle type suffix */
  size_t end_index;
  const uint8_t *const span_ptr = az_span_ptr(*source);
  for (end_index = start_index + 1; end_index < n; end_index++) {
    const uint8_t ch = span_ptr[end_index], last_ch = span_ptr[end_index - 1];
    switch (ch) {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
    case '\v':
      goto end;
      /* Handle comments, as there is only one digit in: "5//5" and one in
       * "6/\*5"
       */
    case '/':
    case '*':
      if (last_ch == '/') {
        --end_index;
        goto end;
      }
      /* TODO: Scientific notation, hexadecimal, octal */
    default:
      break;
    }
  }

end: {
#ifdef DEBUG_SCANNER
  char *s;
  asprintf(&s, "eatWord[%02" NUM_LONG_FMT "u:%02" NUM_LONG_FMT "u]",
           start_index, end_index);
  print_escaped_span(s, az_span_slice(*source, start_index, end_index));
  free(s);
#endif /* DEBUG_SCANNER */
  token_ptr->kind = NUMERIC,
  token_ptr->span = az_span_slice(*source, start_index, end_index);
}
  return end_index - 1;
}

void tokenizer_az_span_elem_arr_cleanup(
    struct tokenizer_az_span_arr *const token_arr) {
  if (token_arr == NULL || token_arr->elem == NULL ||
      az_span_ptr(token_arr->elem->span) == NULL)
    return;
  free(token_arr->elem);
  token_arr->elem = NULL, token_arr->size = 0;
}
