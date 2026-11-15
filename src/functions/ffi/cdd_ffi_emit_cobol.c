/**
 * @file cdd_ffi_emit_cobol.c
 * @brief Production COBOL 2002+ / GnuCOBOL FFI binding emitter implementation.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_ffi_emit_cobol.h"
#include "c_cdd/format_specifiers.h"
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include "cdd_ffi_emit_common.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/** @brief Maximum COBOL user-defined word length (30 chars satisfies 30/31
 * limits). */
#define COBOL_MAX_WORD_LEN 30

/** @brief Maximum file basename stem length to stay well within MAX_PATH. */
#define COBOL_MAX_FILE_STEM 48

/** @brief COBOL free-form `*>` inline comment style. */
static const cdd_ffi_comment_style_t k_cobol_comment_style = {
    "*> ", NULL, NULL, NULL, NULL, NULL, NULL, 100};

/** @brief Comprehensive table of standard COBOL & GnuCOBOL reserved words. */
static const char *const k_cobol_reserved_words[] = {"ACCEPT",
                                                     "ACCESS",
                                                     "ACTIVE-CLASS",
                                                     "ADD",
                                                     "ADDRESS",
                                                     "ADVANCING",
                                                     "AFTER",
                                                     "ALIGNED",
                                                     "ALL",
                                                     "ALLOCATE",
                                                     "ALPHABET",
                                                     "ALPHABETIC",
                                                     "ALPHABETIC-LOWER",
                                                     "ALPHABETIC-UPPER",
                                                     "ALPHANUMERIC",
                                                     "ALPHANUMERIC-EDITED",
                                                     "ALSO",
                                                     "ALTER",
                                                     "ALTERNATE",
                                                     "AND",
                                                     "ANY",
                                                     "ANYCASE",
                                                     "ARE",
                                                     "AREA",
                                                     "AREAS",
                                                     "ASCENDING",
                                                     "ASSIGN",
                                                     "AT",
                                                     "AUTHOR",
                                                     "B-AND",
                                                     "B-NOT",
                                                     "B-OR",
                                                     "B-XOR",
                                                     "BASED",
                                                     "BEFORE",
                                                     "BINARY",
                                                     "BINARY-CHAR",
                                                     "BINARY-DOUBLE",
                                                     "BINARY-LONG",
                                                     "BINARY-SHORT",
                                                     "BIT",
                                                     "BLANK",
                                                     "BLOCK",
                                                     "BOOLEAN",
                                                     "BOTTOM",
                                                     "BY",
                                                     "BYTE-LENGTH",
                                                     "CALL",
                                                     "CANCEL",
                                                     "CF",
                                                     "CH",
                                                     "CHARACTER",
                                                     "CHARACTERS",
                                                     "CLASS",
                                                     "CLASS-ID",
                                                     "CLOSE",
                                                     "COBOL",
                                                     "CODE",
                                                     "CODE-SET",
                                                     "COL",
                                                     "COLLATING",
                                                     "COLS",
                                                     "COLUMN",
                                                     "COLUMNS",
                                                     "COMMA",
                                                     "COMMON",
                                                     "COMMUNICATION",
                                                     "COMP",
                                                     "COMP-1",
                                                     "COMP-2",
                                                     "COMP-3",
                                                     "COMP-4",
                                                     "COMP-5",
                                                     "COMPUTATIONAL",
                                                     "COMPUTE",
                                                     "CONDITION",
                                                     "CONFIGURATION",
                                                     "CONSTANT",
                                                     "CONTAINS",
                                                     "CONTENT",
                                                     "CONTINUE",
                                                     "CONTROL",
                                                     "CONTROLS",
                                                     "CONVERTING",
                                                     "COPY",
                                                     "CORR",
                                                     "CORRESPONDING",
                                                     "COUNT",
                                                     "CRT",
                                                     "CURRENCY",
                                                     "CURSOR",
                                                     "DATA",
                                                     "DATA-POINTER",
                                                     "DATE",
                                                     "DAY",
                                                     "DAY-OF-WEEK",
                                                     "DE",
                                                     "DEBUGGING",
                                                     "DECIMAL-POINT",
                                                     "DECLARATIVES",
                                                     "DEFAULT",
                                                     "DELETE",
                                                     "DELIMITED",
                                                     "DELIMITER",
                                                     "DEPENDING",
                                                     "DESCENDING",
                                                     "DESTINATION",
                                                     "DETAIL",
                                                     "DISABLE",
                                                     "DISPLAY",
                                                     "DIVIDE",
                                                     "DIVISION",
                                                     "DOWN",
                                                     "DUPLICATES",
                                                     "DYNAMIC",
                                                     "EC",
                                                     "EGI",
                                                     "ELSE",
                                                     "EMI",
                                                     "ENABLE",
                                                     "END",
                                                     "END-ACCEPT",
                                                     "END-ADD",
                                                     "END-CALL",
                                                     "END-COMPUTE",
                                                     "END-DELETE",
                                                     "END-DISPLAY",
                                                     "END-DIVIDE",
                                                     "END-EVALUATE",
                                                     "END-IF",
                                                     "END-MULTIPLY",
                                                     "END-OF-PAGE",
                                                     "END-PERFORM",
                                                     "END-READ",
                                                     "END-RECEIVE",
                                                     "END-RETURN",
                                                     "END-REWRITE",
                                                     "END-SEARCH",
                                                     "END-START",
                                                     "END-STRING",
                                                     "END-SUBTRACT",
                                                     "END-UNSTRING",
                                                     "END-WRITE",
                                                     "ENTER",
                                                     "ENTRY",
                                                     "ENVIRONMENT",
                                                     "EO",
                                                     "EOP",
                                                     "EQUAL",
                                                     "ERROR",
                                                     "ESI",
                                                     "EVALUATE",
                                                     "EVERY",
                                                     "EXCEPTION",
                                                     "EXCEPTION-OBJECT",
                                                     "EXIT",
                                                     "EXTEND",
                                                     "EXTERNAL",
                                                     "FACTORY",
                                                     "FALSE",
                                                     "FD",
                                                     "FILE",
                                                     "FILE-CONTROL",
                                                     "FILLER",
                                                     "FINAL",
                                                     "FIRST",
                                                     "FLOAT-EXTENDED",
                                                     "FLOAT-LONG",
                                                     "FLOAT-SHORT",
                                                     "FOOTING",
                                                     "FOR",
                                                     "FORMAT",
                                                     "FREE",
                                                     "FROM",
                                                     "FUNCTION",
                                                     "FUNCTION-ID",
                                                     "GENERATE",
                                                     "GET",
                                                     "GIVING",
                                                     "GLOBAL",
                                                     "GO",
                                                     "GOBACK",
                                                     "GREATER",
                                                     "GROUP",
                                                     "GROUP-USAGE",
                                                     "HEADING",
                                                     "HIGH-VALUE",
                                                     "HIGH-VALUES",
                                                     "I-O",
                                                     "I-O-CONTROL",
                                                     "ID",
                                                     "IDENTIFICATION",
                                                     "IF",
                                                     "IN",
                                                     "INDEX",
                                                     "INDEXED",
                                                     "INDICATE",
                                                     "INHERITS",
                                                     "INITIAL",
                                                     "INITIALIZE",
                                                     "INITIATE",
                                                     "INPUT",
                                                     "INPUT-OUTPUT",
                                                     "INSPECT",
                                                     "INSTALLATION",
                                                     "INTERFACE",
                                                     "INTERFACE-ID",
                                                     "INTO",
                                                     "INVALID",
                                                     "INVOKE",
                                                     "IS",
                                                     "JUST",
                                                     "JUSTIFIED",
                                                     "KEY",
                                                     "LABEL",
                                                     "LAST",
                                                     "LEADING",
                                                     "LEFT",
                                                     "LENGTH",
                                                     "LESS",
                                                     "LIMIT",
                                                     "LIMITS",
                                                     "LINAGE",
                                                     "LINAGE-COUNTER",
                                                     "LINE",
                                                     "LINE-COUNTER",
                                                     "LINES",
                                                     "LINKAGE",
                                                     "LOCAL-STORAGE",
                                                     "LOCALE",
                                                     "LOCK",
                                                     "LOW-VALUE",
                                                     "LOW-VALUES",
                                                     "MEMORY",
                                                     "MERGE",
                                                     "MESSAGE",
                                                     "METHOD",
                                                     "METHOD-ID",
                                                     "MINUS",
                                                     "MODE",
                                                     "MODULES",
                                                     "MOVE",
                                                     "MULTIPLE",
                                                     "MULTIPLY",
                                                     "NATIONAL",
                                                     "NATIONAL-EDITED",
                                                     "NATIVE",
                                                     "NEGATIVE",
                                                     "NESTED",
                                                     "NEXT",
                                                     "NO",
                                                     "NOT",
                                                     "NULL",
                                                     "NULLS",
                                                     "NUMBER",
                                                     "NUMERIC",
                                                     "NUMERIC-EDITED",
                                                     "OBJECT",
                                                     "OBJECT-COMPUTER",
                                                     "OBJECT-REFERENCE",
                                                     "OCCURS",
                                                     "OF",
                                                     "OFF",
                                                     "OMITTED",
                                                     "ON",
                                                     "OPEN",
                                                     "OPTIONAL",
                                                     "OPTIONS",
                                                     "OR",
                                                     "ORDER",
                                                     "ORGANIZATION",
                                                     "OTHER",
                                                     "OUTPUT",
                                                     "OVERFLOW",
                                                     "OVERRIDE",
                                                     "PACKED-DECIMAL",
                                                     "PADDING",
                                                     "PAGE",
                                                     "PAGE-COUNTER",
                                                     "PERFORM",
                                                     "PF",
                                                     "PH",
                                                     "PIC",
                                                     "PICTURE",
                                                     "PLUS",
                                                     "POINTER",
                                                     "POSITION",
                                                     "POSITIVE",
                                                     "PRESENT",
                                                     "PRINTING",
                                                     "PROCEDURE",
                                                     "PROCEDURES",
                                                     "PROCEED",
                                                     "PROGRAM",
                                                     "PROGRAM-ID",
                                                     "PROGRAM-POINTER",
                                                     "PROPERTY",
                                                     "PROTOTYPE",
                                                     "PURGE",
                                                     "QUEUE",
                                                     "QUOTE",
                                                     "QUOTES",
                                                     "RAISE",
                                                     "RAISING",
                                                     "RANDOM",
                                                     "RD",
                                                     "READ",
                                                     "RECEIVE",
                                                     "RECORD",
                                                     "RECORDS",
                                                     "REDEFINES",
                                                     "REEL",
                                                     "REFERENCE",
                                                     "REFERENCES",
                                                     "RELATIVE",
                                                     "RELEASE",
                                                     "REMAINDER",
                                                     "REMOVAL",
                                                     "RENAMES",
                                                     "REPLACE",
                                                     "REPLACING",
                                                     "REPORT",
                                                     "REPORTING",
                                                     "REPORTS",
                                                     "REPOSITORY",
                                                     "RERUN",
                                                     "RESERVE",
                                                     "RESET",
                                                     "RESUME",
                                                     "RETRY",
                                                     "RETURN",
                                                     "RETURN-CODE",
                                                     "RETURNING",
                                                     "REVERSED",
                                                     "REWIND",
                                                     "REWRITE",
                                                     "RF",
                                                     "RH",
                                                     "RIGHT",
                                                     "ROUNDED",
                                                     "RUN",
                                                     "SAME",
                                                     "SCREEN",
                                                     "SD",
                                                     "SEARCH",
                                                     "SECTION",
                                                     "SECURITY",
                                                     "SEGMENT",
                                                     "SEGMENT-LIMIT",
                                                     "SELECT",
                                                     "SELF",
                                                     "SEND",
                                                     "SENTENCE",
                                                     "SEPARATE",
                                                     "SEQUENCE",
                                                     "SEQUENTIAL",
                                                     "SET",
                                                     "SHARING",
                                                     "SIGN",
                                                     "SIGNED",
                                                     "SIZE",
                                                     "SORT",
                                                     "SORT-MERGE",
                                                     "SOURCE",
                                                     "SOURCE-COMPUTER",
                                                     "SOURCES",
                                                     "SPACE",
                                                     "SPACES",
                                                     "SPECIAL-NAMES",
                                                     "STANDARD",
                                                     "STANDARD-1",
                                                     "STANDARD-2",
                                                     "START",
                                                     "STATUS",
                                                     "STOP",
                                                     "STRING",
                                                     "SUB-QUEUE-1",
                                                     "SUB-QUEUE-2",
                                                     "SUB-QUEUE-3",
                                                     "SUBTRACT",
                                                     "SUM",
                                                     "SUPER",
                                                     "SUPPRESS",
                                                     "SYMBOLIC",
                                                     "SYNC",
                                                     "SYNCHRONIZED",
                                                     "SYSTEM-DEFAULT",
                                                     "TABLE",
                                                     "TALLYING",
                                                     "TAPE",
                                                     "TERMINAL",
                                                     "TERMINATE",
                                                     "TEST",
                                                     "TEXT",
                                                     "THAN",
                                                     "THEN",
                                                     "THROUGH",
                                                     "THRU",
                                                     "TIME",
                                                     "TIMES",
                                                     "TO",
                                                     "TOP",
                                                     "TRAILING",
                                                     "TRUE",
                                                     "TYPE",
                                                     "TYPEDEF",
                                                     "UNIT",
                                                     "UNIVERSAL",
                                                     "UNLOCK",
                                                     "UNSIGNED",
                                                     "UNSTRING",
                                                     "UNTIL",
                                                     "UP",
                                                     "UPON",
                                                     "USAGE",
                                                     "USE",
                                                     "USER-DEFAULT",
                                                     "USING",
                                                     "VAL-STATUS",
                                                     "VALID",
                                                     "VALIDATE",
                                                     "VALIDATE-STATUS",
                                                     "VALUE",
                                                     "VALUES",
                                                     "VARYING",
                                                     "WHEN",
                                                     "WITH",
                                                     "WORDS",
                                                     "WORKING-STORAGE",
                                                     "WRITE",
                                                     "ZERO",
                                                     "ZEROES",
                                                     "ZEROS"};

/**
 * @brief Growable COBOL word registry for collision-free Copybook and
 * subprogram identifiers.
 */
typedef struct cobol_scope_t {
  char **words; /**< Registered uppercase COBOL words */
  size_t count; /**< Active entry count */
  size_t cap;   /**< Allocated capacity */
} cobol_scope_t;

/**
 * @brief Frees all entries in a `cobol_scope_t`.
 *
 * @param[in,out] scope Scope table to clean up.
 * @return CDD_C_SUCCESS.
 */
static NO_DISCARD cdd_c_error_t cobol_scope_free(cobol_scope_t *scope) {
  size_t i;
  for (i = 0; i < scope->count; i++) {
    C_CDD_FREE(scope->words[i]);
  }
  C_CDD_FREE(scope->words);
  scope->words = NULL;
  scope->count = 0;
  scope->cap = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Registers a sanitized COBOL word in `scope`, disambiguating collisions
 * within the 30-character limit.
 *
 * @param[in,out] scope Active COBOL word scope.
 * @param[in] c_ident Source C identifier.
 * @param[in] prefix Optional explicit prefix.
 * @param[out] out_buf Destination buffer (`out_cap >= 8`).
 * @param[in] out_cap Capacity of `out_buf`.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY on OOM.
 */
static NO_DISCARD cdd_c_error_t cobol_scope_make_unique(cobol_scope_t *scope,
                                                        const char *c_ident,
                                                        const char *prefix,
                                                        char *out_buf,
                                                        size_t out_cap) {
  size_t attempt = 0;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  for (;;) {
    size_t i;
    int exists = 0;

    rc = cdd_c_keep_err(rc, cdd_cobol_sanitize_word(c_ident, prefix, attempt,
                                                    out_buf, out_cap));

    for (i = 0; i < scope->count; i++) {
      if (strcmp(scope->words[i], out_buf) == 0) {
        exists = 1;
        break;
      }
    }
    if (!exists)
      break;
    attempt++;
  }

  if (scope->count == scope->cap) {
    size_t new_cap = scope->cap == 0 ? 32 : scope->cap * 2;
    char **new_arr =
        (char **)C_CDD_REALLOC(scope->words, new_cap * sizeof(char *));
    if (!new_arr)
      return CDD_C_ERROR_MEMORY;
    scope->words = new_arr;
    scope->cap = new_cap;
  }
  scope->words[scope->count] = C_CDD_STRDUP(out_buf);
  if (!scope->words[scope->count])
    return CDD_C_ERROR_MEMORY;
  scope->count++;
  return rc;
}

/**
 * @brief Checks if `word` (uppercase) matches a COBOL reserved word.
 *
 * @param[in] word Candidate uppercase COBOL word.
 * @param[out] out_reserved Receives 1 if reserved, 0 otherwise.
 * @return CDD_C_SUCCESS.
 */
static NO_DISCARD cdd_c_error_t cobol_is_reserved(const char *word,
                                                  int *out_reserved) {
  size_t i;
  size_t count;

  *out_reserved = 0;
  count = sizeof(k_cobol_reserved_words) / sizeof(k_cobol_reserved_words[0]);
  for (i = 0; i < count; i++) {
    if (strcmp(word, k_cobol_reserved_words[i]) == 0) {
      *out_reserved = 1;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Computes a 12-bit uppercase hex hash for deterministic truncation
 * suffixes.
 *
 * @param[in] s Input string.
 * @param[out] out_hash Receives 12-bit hash (`0..0xFFF`).
 * @return CDD_C_SUCCESS.
 */
static NO_DISCARD cdd_c_error_t cobol_hash12(const char *s,
                                             unsigned int *out_hash) {
  unsigned int h = 2166136261u;
  while (*s != '\0') {
    h ^= (unsigned int)(unsigned char)(*s++);
    h *= 16777619u;
  }
  *out_hash = (h ^ (h >> 12)) & 0xFFFu;
  return CDD_C_SUCCESS;
}

/*
 * @brief Normalizes a C identifier (`snake_case` or `camelCase`) into a valid
 * uppercase hyphenated COBOL user-defined word (`<= 30` characters).
 *
 * @param[in] c_ident Source C identifier (may be NULL or empty).
 * @param[in] prefix Optional explicit prefix (e.g. `"LS-"` or `"WS-"`), or
 * NULL.
 * @param[in] disambig_idx Disambiguation index (`0` for primary name, `>= 1` on
 * collision).
 * @param[out] out_buf Destination buffer receiving the sanitized COBOL word.
 * @param[in] out_cap Capacity of `out_buf` in bytes (must be >= 8).
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
NO_DISCARD cdd_c_error_t cdd_cobol_sanitize_word(const char *c_ident,
                                                 const char *prefix,
                                                 size_t disambig_idx,
                                                 char *out_buf,
                                                 size_t out_cap) {
  char raw[256];
  char combined[300];
  size_t rpos = 0;
  size_t i;
  size_t eff_max;
  int is_res = 0;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!out_buf || out_cap < 8)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  eff_max = out_cap - 1;
  if (eff_max > COBOL_MAX_WORD_LEN)
    eff_max = COBOL_MAX_WORD_LEN;

  if (c_ident && c_ident[0] != '\0') {
    for (i = 0; c_ident[i] != '\0'; i++) {
      unsigned char ch = (unsigned char)c_ident[i];
      if (rpos + 2 >= sizeof(raw)) {
        break;
      }
      if (isupper(ch)) {
        if (rpos > 0 && raw[rpos - 1] != '-' &&
            (islower((unsigned char)c_ident[i - 1]) ||
             isdigit((unsigned char)c_ident[i - 1]) ||
             islower((unsigned char)c_ident[i + 1]))) {
          raw[rpos++] = '-';
        }
        raw[rpos++] = (char)ch;
      } else if (isalnum(ch)) {
        raw[rpos++] = (char)toupper(ch);
      } else if (rpos > 0 && raw[rpos - 1] != '-') {
        raw[rpos++] = '-';
      }
    }
  }
  while (rpos > 0 && raw[rpos - 1] == '-')
    rpos--;
  raw[rpos] = '\0';

  if (rpos == 0) {
    rc = cdd_c_keep_err(
        rc, cdd_ffi_snprintf_checked(raw, sizeof(raw), "%s", "UNNAMED"));
  }

  if (prefix && prefix[0] != '\0') {
    rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(combined, sizeof(combined),
                                                     "%s%s", prefix, raw));
  } else {
    rc = cdd_c_keep_err(rc, cobol_is_reserved(raw, &is_res));
    rc = cdd_c_keep_err(
        rc,
        cdd_ffi_snprintf_checked(
            combined, sizeof(combined),
            (isdigit((unsigned char)raw[0]) || is_res) ? "CDD-%s" : "%s", raw));
  }

  {
    size_t clen = strlen(combined);
    if (clen > eff_max || disambig_idx > 0) {
      char sfx[24];
      size_t sfx_len;
      size_t keep;
      unsigned int h12 = 0;

      rc = cdd_c_keep_err(
          rc, cobol_hash12((c_ident && c_ident[0] != '\0') ? c_ident : combined,
                           &h12));
      if (disambig_idx > 0) {
        rc = cdd_c_keep_err(
            rc, cdd_ffi_snprintf_checked(sfx, sizeof(sfx), "-%03X%lu", h12,
                                         (unsigned long)disambig_idx));
      } else {
        rc = cdd_c_keep_err(
            rc, cdd_ffi_snprintf_checked(sfx, sizeof(sfx), "-%03X", h12));
      }
      sfx_len = strlen(sfx);
      keep = (eff_max > sfx_len) ? (eff_max - sfx_len) : 1;
      if (keep > clen)
        keep = clen;
      while (keep > 1 && combined[keep - 1] == '-')
        keep--;
      combined[keep] = '\0';
      return cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                    out_buf, out_cap, "%s%s", combined, sfx));
    }
  }

  return cdd_c_keep_err(
      rc, cdd_ffi_snprintf_checked(out_buf, out_cap, "%s", combined));
}

/**
 * @brief Sanitizes a C library/module name into a safe lowercase filename stem
 * (`[a-z0-9_]`, `<= COBOL_MAX_FILE_STEM`).
 *
 * @param[in] raw Raw library or module name.
 * @param[in] fallback Default stem if `raw` is NULL/empty.
 * @param[out] out_buf Destination buffer (`out_cap > COBOL_MAX_FILE_STEM`).
 * @param[in] out_cap Capacity of `out_buf`.
 * @return CDD_C_SUCCESS on success, or error code on truncation.
 */
static NO_DISCARD cdd_c_error_t cobol_sanitize_file_stem(const char *raw,
                                                         const char *fallback,
                                                         char *out_buf,
                                                         size_t out_cap) {
  size_t pos = 0;
  size_t i;
  const char *src = (raw[0] != '\0') ? raw : fallback;

  for (i = 0; src[i] != '\0' && pos < COBOL_MAX_FILE_STEM; i++) {
    unsigned char ch = (unsigned char)src[i];
    if (isupper(ch)) {
      out_buf[pos++] = (char)tolower(ch);
    } else if (isalnum(ch) || ch == '_') {
      out_buf[pos++] = (char)ch;
    } else if (pos > 0 && out_buf[pos - 1] != '_') {
      out_buf[pos++] = '_';
    }
  }
  while (pos > 1 && out_buf[pos - 1] == '_')
    pos--;
  if (pos == 0)
    return cdd_ffi_snprintf_checked(out_buf, out_cap, "%s", fallback);
  out_buf[pos] = '\0';
  return CDD_C_SUCCESS;
}

/**
 * @brief Normalizes a C integer/enum literal into a COBOL 2002 / GnuCOBOL
 * constant literal (`H"..."` for positive hex, decimal digits otherwise, C23
 * `'` stripped).
 *
 * @param[in] raw Raw C literal string.
 * @param[in] allow_hex_lit Non-zero to emit `H"..."` for non-negative hex
 * literals (valid in `CONSTANT AS`, whereas `88 VALUE` prefers decimal).
 * @param[out] out_buf Destination buffer.
 * @param[in] out_cap Capacity of `out_buf`.
 * @return CDD_C_SUCCESS on success, or error code on truncation.
 */
static NO_DISCARD cdd_c_error_t cobol_normalize_int_literal(const char *raw,
                                                            int allow_hex_lit,
                                                            char *out_buf,
                                                            size_t out_cap) {
  const char *p;
  size_t len;
  size_t i;
  int neg = 0;
  unsigned long val = 0;

  if (raw[0] == '\0')
    return cdd_ffi_snprintf_checked(out_buf, out_cap, "%s", "0");

  p = raw;
  while (memchr(" \t(", (unsigned char)*p, 3) != NULL)
    p++;
  if (*p == '-') {
    neg = 1;
    p++;
  } else if (*p == '+') {
    p++;
  }
  while (memchr(" \t", (unsigned char)*p, 2) != NULL)
    p++;

  len = strlen(p);
  while (len > 0 && memchr(" \t)uUlL", (unsigned char)p[len - 1], 7) != NULL) {
    len--;
  }
  if (len == 0)
    return cdd_ffi_snprintf_checked(out_buf, out_cap, "%s", "0");

  if (len > 2 && p[0] == '0' && memchr("xX", (unsigned char)p[1], 2) != NULL) {
    char hex_buf[64];
    size_t hpos = 0;
    for (i = 2; i < len && hpos + 1 < sizeof(hex_buf); i++) {
      if (p[i] == '\'')
        continue;
      if (!isxdigit((unsigned char)p[i]))
        return cdd_ffi_snprintf_checked(out_buf, out_cap, "%s", "0");
      hex_buf[hpos++] = (char)toupper((unsigned char)p[i]);
      val = (val << 4) |
            (unsigned long)(isdigit((unsigned char)p[i])
                                ? (p[i] - '0')
                                : (toupper((unsigned char)p[i]) - 'A' + 10));
    }
    hex_buf[hpos] = '\0';
    if (allow_hex_lit && !neg) {
      return cdd_ffi_snprintf_checked(out_buf, out_cap, "H\"%s\"", hex_buf);
    }
    return cdd_ffi_snprintf_checked(out_buf, out_cap, "%s%lu", neg ? "-" : "",
                                    val);
  }

  if (len > 2 && p[0] == '0' && memchr("bB", (unsigned char)p[1], 2) != NULL) {
    for (i = 2; i < len; i++) {
      if (p[i] == '\'')
        continue;
      if (p[i] != '0' && p[i] != '1')
        return cdd_ffi_snprintf_checked(out_buf, out_cap, "%s", "0");
      val = (val << 1) | (unsigned long)(p[i] - '0');
    }
    return cdd_ffi_snprintf_checked(out_buf, out_cap, "%s%lu", neg ? "-" : "",
                                    val);
  }

  if (len > 1 && p[0] == '0' && p[1] >= '0' && p[1] <= '7') {
    for (i = 1; i < len; i++) {
      if (p[i] == '\'')
        continue;
      if (p[i] < '0' || p[i] > '7')
        return cdd_ffi_snprintf_checked(out_buf, out_cap, "%s", "0");
      val = (val << 3) | (unsigned long)(p[i] - '0');
    }
    return cdd_ffi_snprintf_checked(out_buf, out_cap, "%s%lu", neg ? "-" : "",
                                    val);
  }

  {
    char dec_buf[64];
    size_t dpos = 0;
    for (i = 0; i < len && dpos + 1 < sizeof(dec_buf); i++) {
      if (p[i] == '\'')
        continue;
      if (!isdigit((unsigned char)p[i]))
        return cdd_ffi_snprintf_checked(out_buf, out_cap, "%s", "0");
      dec_buf[dpos++] = p[i];
    }
    dec_buf[dpos] = '\0';
    if (dpos == 0)
      return cdd_ffi_snprintf_checked(out_buf, out_cap, "%s", "0");
    return cdd_ffi_snprintf_checked(out_buf, out_cap, "%s%s", neg ? "-" : "",
                                    dec_buf);
  }
}

/**
 * @brief Looks up a non-function/non-macro IR node by exact C name.
 *
 * @param[in] ir Enclosing FFI IR.
 * @param[in] name C symbol name.
 * @param[out] out_node Receives pointer to matching node, or NULL.
 * @return CDD_C_SUCCESS.
 */
static NO_DISCARD cdd_c_error_t
cobol_find_type_node(const cdd_ffi_ir_t *ir, const char *name,
                     const cdd_ffi_ir_node_t **out_node) {
  size_t i;

  *out_node = NULL;
  if (!name)
    return CDD_C_SUCCESS;

  for (i = 0; i < ir->nodes_count; i++) {
    if (ir->nodes[i].name && strcmp(ir->nodes[i].name, name) == 0 &&
        ir->nodes[i].kind != CDD_FFI_NODE_FUNCTION &&
        ir->nodes[i].kind != CDD_FFI_NODE_MACRO) {
      *out_node = &ir->nodes[i];
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Maps an FFI type (ignoring `array_size`) to its native-endian COBOL
 * 2002+ / GnuCOBOL `USAGE` or `PIC X(n)` clause.
 *
 * @param[in] ir Enclosing FFI IR.
 * @param[in] type FFI type descriptor.
 * @param[in] elem_bytes Known single-element byte size (0 if unknown).
 * @param[out] out_clause Destination buffer for COBOL usage/picture clause.
 * @param[in] clause_cap Capacity of `out_clause`.
 * @return CDD_C_SUCCESS on success, or error code on truncation.
 */
static NO_DISCARD cdd_c_error_t
cobol_map_usage_clause(const cdd_ffi_ir_t *ir, const cdd_ffi_type_t *type,
                       size_t elem_bytes, char *out_clause, size_t clause_cap) {
  cdd_c_error_t rc;

  if (type->pointer_depth > 0 || type->kind == CDD_FFI_KIND_OPAQUE_PTR ||
      type->kind == CDD_FFI_KIND_STD_SHARED_PTR ||
      type->kind == CDD_FFI_KIND_STD_UNIQUE_PTR) {
    return cdd_ffi_snprintf_checked(out_clause, clause_cap, "%s",
                                    "USAGE POINTER");
  }
  if (type->kind == CDD_FFI_KIND_FUNCTION_PTR) {
    return cdd_ffi_snprintf_checked(out_clause, clause_cap, "%s",
                                    "USAGE PROGRAM-POINTER");
  }

  switch (type->kind) {
  case CDD_FFI_KIND_BOOL:
  case CDD_FFI_KIND_INT8:
    return cdd_ffi_snprintf_checked(out_clause, clause_cap, "%s",
                                    "USAGE BINARY-CHAR SIGNED");
  case CDD_FFI_KIND_UINT8:
    return cdd_ffi_snprintf_checked(out_clause, clause_cap, "%s",
                                    "USAGE BINARY-CHAR UNSIGNED");
  case CDD_FFI_KIND_INT16:
    return cdd_ffi_snprintf_checked(out_clause, clause_cap, "%s",
                                    "USAGE BINARY-SHORT SIGNED");
  case CDD_FFI_KIND_UINT16:
    return cdd_ffi_snprintf_checked(out_clause, clause_cap, "%s",
                                    "USAGE BINARY-SHORT UNSIGNED");
  case CDD_FFI_KIND_INT32:
  case CDD_FFI_KIND_ENUM_REF:
    return cdd_ffi_snprintf_checked(out_clause, clause_cap, "%s",
                                    "USAGE BINARY-LONG SIGNED");
  case CDD_FFI_KIND_UINT32:
    return cdd_ffi_snprintf_checked(out_clause, clause_cap, "%s",
                                    "USAGE BINARY-LONG UNSIGNED");
  case CDD_FFI_KIND_INT64:
    return cdd_ffi_snprintf_checked(out_clause, clause_cap, "%s",
                                    "USAGE BINARY-DOUBLE SIGNED");
  case CDD_FFI_KIND_UINT64:
    return cdd_ffi_snprintf_checked(out_clause, clause_cap, "%s",
                                    "USAGE BINARY-DOUBLE UNSIGNED");
  case CDD_FFI_KIND_FLOAT32:
    return cdd_ffi_snprintf_checked(out_clause, clause_cap, "%s",
                                    "USAGE COMP-1");
  case CDD_FFI_KIND_FLOAT64:
    return cdd_ffi_snprintf_checked(out_clause, clause_cap, "%s",
                                    "USAGE COMP-2");
  case CDD_FFI_KIND_TYPEDEF_REF: {
    const cdd_ffi_ir_node_t *rn = NULL;
    if (type->ref_name && (strcmp(type->ref_name, "size_t") == 0 ||
                           strcmp(type->ref_name, "uintptr_t") == 0)) {
      return cdd_ffi_snprintf_checked(out_clause, clause_cap, "%s",
                                      "USAGE BINARY-DOUBLE UNSIGNED");
    }
    rc = cobol_find_type_node(ir, type->ref_name, &rn);
    if (rn && rn->kind == CDD_FFI_NODE_TYPEDEF) {
      return cdd_c_keep_err(
          rc, cobol_map_usage_clause(ir, &rn->return_or_base_type, elem_bytes,
                                     out_clause, clause_cap));
    }
    return cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(out_clause, clause_cap,
                                                       "%s", "USAGE POINTER"));
  }
  case CDD_FFI_KIND_STRUCT_REF:
  case CDD_FFI_KIND_TEMPLATE_STRUCT_REF: {
    const cdd_ffi_ir_node_t *rn = NULL;
    size_t sz = elem_bytes;
    rc = cobol_find_type_node(ir, type->ref_name, &rn);
    if (rn && rn->size_bytes > 0)
      sz = rn->size_bytes;
    if (sz == 0) {
      return cdd_c_keep_err(rc,
                            cdd_ffi_snprintf_checked(out_clause, clause_cap,
                                                     "%s", "USAGE POINTER"));
    }
    return cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(out_clause, clause_cap,
                                                       "PIC X(%lu)",
                                                       (unsigned long)sz));
  }
  default:
    if (elem_bytes > 0) {
      return cdd_ffi_snprintf_checked(out_clause, clause_cap, "PIC X(%lu)",
                                      (unsigned long)elem_bytes);
    }
    return cdd_ffi_snprintf_checked(out_clause, clause_cap, "%s",
                                    "USAGE POINTER");
  }
}

/**
 * @brief Checks if a type is a struct or union passed/returned by value
 * (`pointer_depth == 0`, `array_size <= 0`, non-opaque struct/union in `ir`).
 *
 * @param[in] ir Enclosing FFI IR.
 * @param[in] type FFI type descriptor.
 * @param[out] out_byval Receives 1 if struct/union by value, 0 otherwise.
 * @param[out] out_node Optional pointer receiving matched struct/union node.
 * @return CDD_C_SUCCESS.
 */
static NO_DISCARD cdd_c_error_t
cobol_is_byval_record(const cdd_ffi_ir_t *ir, const cdd_ffi_type_t *type,
                      int *out_byval, const cdd_ffi_ir_node_t **out_node) {
  const cdd_ffi_ir_node_t *rn = NULL;
  cdd_c_error_t rc;

  *out_byval = 0;
  if (out_node)
    *out_node = NULL;

  if (type->pointer_depth > 0 || type->array_size > 0)
    return CDD_C_SUCCESS;
  if (type->kind != CDD_FFI_KIND_STRUCT_REF &&
      type->kind != CDD_FFI_KIND_TEMPLATE_STRUCT_REF) {
    return CDD_C_SUCCESS;
  }

  rc = cobol_find_type_node(ir, type->ref_name, &rn);
  if (rn &&
      (rn->kind == CDD_FFI_NODE_STRUCT || rn->kind == CDD_FFI_NODE_UNION) &&
      rn->fields_count > 0) {
    *out_byval = 1;
    if (out_node)
      *out_node = rn;
  }
  return rc;
}

/**
 * @brief Checks whether a function requires routing through the generated C89
 * ABI shim (`cdd_cobol_<name>`).
 *
 * @param[in] ir Enclosing FFI IR.
 * @param[in] fn Function IR node.
 * @param[out] out_needs_shim Receives 1 if shim is used, 0 for direct C `CALL`.
 * @param[out] out_cstr_ret Receives 1 if function returns `char *` / `const
 * char *`.
 * @param[out] out_rec_ret Receives 1 if function returns a struct/union by
 * value.
 * @return CDD_C_SUCCESS.
 */
static NO_DISCARD cdd_c_error_t cobol_check_fn_shim(const cdd_ffi_ir_t *ir,
                                                    const cdd_ffi_ir_node_t *fn,
                                                    int *out_needs_shim,
                                                    int *out_cstr_ret,
                                                    int *out_rec_ret) {
  size_t j;
  int rec_ret = 0;
  int cstr_ret = 0;
  cdd_c_error_t rc;

  *out_needs_shim = 0;
  *out_cstr_ret = 0;
  *out_rec_ret = 0;

  rc = cobol_is_byval_record(ir, &fn->return_or_base_type, &rec_ret, NULL);

  if (fn->return_or_base_type.pointer_depth == 1 &&
      fn->return_or_base_type.array_size <= 0 &&
      fn->return_or_base_type.kind == CDD_FFI_KIND_INT8) {
    cstr_ret = 1;
  }

  *out_rec_ret = rec_ret;
  *out_cstr_ret = cstr_ret;

  if (fn->is_variadic || rec_ret || cstr_ret) {
    *out_needs_shim = 1;
    return rc;
  }

  for (j = 0; j < fn->fields_count; j++) {
    int byval_p = 0;
    rc = cdd_c_keep_err(
        rc, cobol_is_byval_record(ir, &fn->fields[j].type, &byval_p, NULL));
    if (byval_p) {
      *out_needs_shim = 1;
      return rc;
    }
  }
  return rc;
}

/**
 * @brief Emits `<lib_stem>.cpy` COBOL Data Copybook with exact C ABI struct
 * padding (`FILLER PIC X(n)`), `REDEFINES` unions, `OCCURS N TIMES` arrays, and
 * `CONSTANT AS` / `88`-level enums & macros.
 *
 * @param[in] ir Layout-annotated FFI IR.
 * @param[in] config Generation configuration.
 * @param[in] cpy_file Copybook filename (`<lib_stem>.cpy`).
 * @param[in] node_words Parallel array of unique COBOL words for `ir->nodes`.
 * @return CDD_C_SUCCESS on success, or error code on I/O/OOM failure.
 */
static NO_DISCARD cdd_c_error_t emit_cobol_copybook(
    const cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config,
    const char *cpy_file, char **node_words) {
  FILE *fp = NULL;
  cobol_scope_t cpy_scope;
  size_t i;
  size_t j;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  memset(&cpy_scope, 0, sizeof(cpy_scope));

  for (i = 0; i < ir->nodes_count; i++) {
    if (ir->nodes[i].kind != CDD_FFI_NODE_FUNCTION) {
      char tmp_w[COBOL_MAX_WORD_LEN + 1];
      rc = cobol_scope_make_unique(&cpy_scope, node_words[i], NULL, tmp_w,
                                   sizeof(tmp_w));
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
  }

  rc = cdd_ffi_open_output_file(config->output_dir, cpy_file, &fp);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_ffi_write_fmt(
      fp,
      "*> Auto-generated COBOL 2002+ / GnuCOBOL ABI Copybook (%s)\n"
      "       01 CDD-FFI-ABI-READY CONSTANT AS 1.\n\n",
      cpy_file);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  for (i = 0; i < ir->nodes_count; i++) {
    const cdd_ffi_ir_node_t *node = &ir->nodes[i];

    if (node->kind == CDD_FFI_NODE_FUNCTION)
      continue;

    rc = cdd_ffi_emit_translated_trivia(fp, node->leading_trivia,
                                        &k_cobol_comment_style);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (node->doc) {
      rc = cdd_ffi_emit_translated_doc(fp, node->doc, "       ",
                                       &k_cobol_comment_style);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }

    if (node->kind == CDD_FFI_NODE_MACRO) {
      const char *raw_val =
          node->evaluated_value
              ? node->evaluated_value
              : (node->variants_count > 0 ? node->variants[0].value : NULL);
      if (!raw_val)
        continue;

      if (node->inferred_type == CDD_FFI_MACRO_TYPE_INT) {
        char lit[96];
        rc = cdd_c_keep_err(
            rc, cobol_normalize_int_literal(raw_val, 1, lit, sizeof(lit)));
        rc = cdd_c_keep_err(
            rc, cdd_ffi_write_fmt(fp, "       01 %s CONSTANT AS %s.\n\n",
                                  node_words[i], lit));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      } else if (node->inferred_type == CDD_FFI_MACRO_TYPE_FLOAT) {
        char flit[96];
        size_t fi;
        size_t fpos = 0;
        for (fi = 0; raw_val[fi] != '\0' && fpos + 1 < sizeof(flit); fi++) {
          char c = raw_val[fi];
          if (c == '\'')
            continue;
          if (isdigit((unsigned char)c) ||
              memchr(".-+", (unsigned char)c, 3) != NULL) {
            flit[fpos++] = c;
          }
        }
        flit[fpos] = '\0';
        if (fpos == 0) {
          rc = cdd_c_keep_err(
              rc, cdd_ffi_snprintf_checked(flit, sizeof(flit), "%s", "0"));
        }
        rc = cdd_c_keep_err(
            rc, cdd_ffi_write_fmt(fp, "       01 %s CONSTANT AS %s.\n\n",
                                  node_words[i], flit));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      } else if (node->inferred_type == CDD_FFI_MACRO_TYPE_STRING) {
        rc = cdd_ffi_write_fmt(fp,
                               raw_val[0] == '"'
                                   ? "       01 %s CONSTANT AS %s.\n\n"
                                   : "       01 %s CONSTANT AS \"%s\".\n\n",
                               node_words[i], raw_val);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    } else if (node->kind == CDD_FFI_NODE_ENUM) {
      char item_word[COBOL_MAX_WORD_LEN + 1];
      char raw_item[96];

      for (j = 0; j < node->variants_count; j++) {
        const cdd_ffi_enum_variant_t *var = &node->variants[j];
        char v_word[COBOL_MAX_WORD_LEN + 1];
        char v_const[96];
        const char *v_src;

        if (var->doc) {
          rc = cdd_ffi_emit_translated_doc(fp, var->doc, "       ",
                                           &k_cobol_comment_style);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
        rc = cobol_scope_make_unique(&cpy_scope, var->name, NULL, v_word,
                                     sizeof(v_word));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;

        v_src = (var->number_details && var->number_details->raw_spelling)
                    ? var->number_details->raw_spelling
                    : var->value;
        if (v_src) {
          rc = cdd_c_keep_err(rc, cobol_normalize_int_literal(v_src, 1, v_const,
                                                              sizeof(v_const)));
        } else {
          rc = cdd_c_keep_err(
              rc, cdd_ffi_snprintf_checked(v_const, sizeof(v_const), "%lu",
                                           (unsigned long)j));
        }

        rc = cdd_c_keep_err(
            rc, cdd_ffi_write_fmt(fp, "       01 %s CONSTANT AS %s.\n", v_word,
                                  v_const));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }

      rc = cdd_c_keep_err(rc,
                          cdd_ffi_snprintf_checked(raw_item, sizeof(raw_item),
                                                   "%s-ITEM", node_words[i]));
      rc = cdd_c_keep_err(rc, cobol_scope_make_unique(&cpy_scope, raw_item,
                                                      NULL, item_word,
                                                      sizeof(item_word)));
      if (rc != CDD_C_SUCCESS)
        goto cleanup;

      rc = cdd_ffi_write_fmt(fp, "       01 %s USAGE BINARY-LONG SIGNED.\n",
                             item_word);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;

      for (j = 0; j < node->variants_count; j++) {
        const cdd_ffi_enum_variant_t *var = &node->variants[j];
        char cond_word[COBOL_MAX_WORD_LEN + 1];
        char v_dec[96];
        const char *v_src;

        rc = cobol_scope_make_unique(&cpy_scope, var->name, "IS-", cond_word,
                                     sizeof(cond_word));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;

        v_src = (var->number_details && var->number_details->raw_spelling)
                    ? var->number_details->raw_spelling
                    : var->value;
        if (v_src) {
          rc = cdd_c_keep_err(
              rc, cobol_normalize_int_literal(v_src, 0, v_dec, sizeof(v_dec)));
        } else {
          rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(v_dec, sizeof(v_dec),
                                                           "%lu",
                                                           (unsigned long)j));
        }

        rc = cdd_c_keep_err(rc,
                            cdd_ffi_write_fmt(fp, "          88 %s VALUE %s.\n",
                                              cond_word, v_dec));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
      rc = cdd_ffi_write_str(fp, "\n");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    } else if (node->kind == CDD_FFI_NODE_STRUCT) {
      if (node->fields_count == 0) {
        char h_word[COBOL_MAX_WORD_LEN + 1];
        char raw_h[96];
        rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(raw_h, sizeof(raw_h),
                                                         "%s-HANDLE",
                                                         node_words[i]));
        rc =
            cdd_c_keep_err(rc, cobol_scope_make_unique(&cpy_scope, raw_h, NULL,
                                                       h_word, sizeof(h_word)));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        rc = cdd_ffi_write_fmt(fp, "       01 %s USAGE POINTER.\n\n", h_word);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      } else {
        cobol_scope_t fld_scope;
        memset(&fld_scope, 0, sizeof(fld_scope));

        rc = cdd_ffi_write_fmt(fp, "       01 %s.\n", node_words[i]);
        if (rc == CDD_C_SUCCESS) {
          for (j = 0; j < node->fields_count; j++) {
            const cdd_ffi_field_t *fld = &node->fields[j];
            char f_word[COBOL_MAX_WORD_LEN + 1];
            char usage_str[128];
            char occurs_str[64];
            size_t elem_sz;

            if (fld->doc) {
              rc = cdd_ffi_emit_translated_doc(fp, fld->doc, "          ",
                                               &k_cobol_comment_style);
              if (rc != CDD_C_SUCCESS)
                break;
            }
            if (fld->pad_before_bytes > 0) {
              rc = cdd_ffi_write_fmt(fp, "          05 FILLER PIC X(%lu).\n",
                                     (unsigned long)fld->pad_before_bytes);
              if (rc != CDD_C_SUCCESS)
                break;
            }
            if (fld->bitfield_width > 0 && fld->size_bytes == 0) {
              rc = cdd_ffi_write_fmt(
                  fp,
                  "          *> Packed bitfield %s (%d bits) shares storage\n",
                  fld->name ? fld->name : "bf", fld->bitfield_width);
              if (rc != CDD_C_SUCCESS)
                break;
              continue;
            }

            rc = cobol_scope_make_unique(&fld_scope, fld->name, NULL, f_word,
                                         sizeof(f_word));
            if (rc != CDD_C_SUCCESS)
              break;

            elem_sz = fld->type.array_size > 0
                          ? (fld->size_bytes / (size_t)fld->type.array_size)
                          : fld->size_bytes;
            rc = cdd_c_keep_err(rc, cobol_map_usage_clause(ir, &fld->type,
                                                           elem_sz, usage_str,
                                                           sizeof(usage_str)));

            if (fld->type.array_size > 0) {
              rc = cdd_c_keep_err(
                  rc, cdd_ffi_snprintf_checked(occurs_str, sizeof(occurs_str),
                                               " OCCURS %ld TIMES",
                                               fld->type.array_size));
            } else {
              occurs_str[0] = '\0';
            }

            rc = cdd_c_keep_err(
                rc, cdd_ffi_write_fmt(fp, "          05 %s%s %s.\n", f_word,
                                      occurs_str, usage_str));
            if (rc != CDD_C_SUCCESS)
              break;
          }
        }
        if (rc == CDD_C_SUCCESS && node->trailing_pad_bytes > 0) {
          rc = cdd_ffi_write_fmt(fp, "          05 FILLER PIC X(%lu).\n",
                                 (unsigned long)node->trailing_pad_bytes);
        }
        if (rc == CDD_C_SUCCESS) {
          rc = cdd_ffi_write_str(fp, "\n");
        }
        rc = cdd_c_keep_err(rc, cobol_scope_free(&fld_scope));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    } else if (node->kind == CDD_FFI_NODE_UNION) {
      size_t u_size = node->size_bytes > 0 ? node->size_bytes : 8;
      char raw_word[COBOL_MAX_WORD_LEN + 1];
      char raw_base[96];
      cobol_scope_t var_scope;

      memset(&var_scope, 0, sizeof(var_scope));
      rc = cdd_c_keep_err(rc,
                          cdd_ffi_snprintf_checked(raw_base, sizeof(raw_base),
                                                   "%s-RAW", node_words[i]));
      rc = cdd_c_keep_err(rc,
                          cobol_scope_make_unique(&var_scope, raw_base, NULL,
                                                  raw_word, sizeof(raw_word)));
      if (rc == CDD_C_SUCCESS) {
        rc = cdd_ffi_write_fmt(fp,
                               "       01 %s.\n"
                               "          05 %s PIC X(%lu).\n",
                               node_words[i], raw_word, (unsigned long)u_size);
      }
      if (rc == CDD_C_SUCCESS) {
        for (j = 0; j < node->fields_count; j++) {
          const cdd_ffi_field_t *fld = &node->fields[j];
          char v_word[COBOL_MAX_WORD_LEN + 1];
          char val_word[COBOL_MAX_WORD_LEN + 1];
          char val_raw[96];
          char usage_str[128];
          char occurs_str[64];
          size_t elem_sz;
          size_t rem_pad;

          rc = cobol_scope_make_unique(&var_scope, fld->name, NULL, v_word,
                                       sizeof(v_word));
          if (rc != CDD_C_SUCCESS)
            break;
          rc =
              cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                     val_raw, sizeof(val_raw), "%s-V", v_word));
          rc = cdd_c_keep_err(rc, cobol_scope_make_unique(&var_scope, val_raw,
                                                          NULL, val_word,
                                                          sizeof(val_word)));
          if (rc != CDD_C_SUCCESS)
            break;

          elem_sz = fld->type.array_size > 0
                        ? (fld->size_bytes / (size_t)fld->type.array_size)
                        : fld->size_bytes;
          rc = cdd_c_keep_err(rc, cobol_map_usage_clause(ir, &fld->type,
                                                         elem_sz, usage_str,
                                                         sizeof(usage_str)));

          if (fld->type.array_size > 0) {
            rc = cdd_c_keep_err(
                rc, cdd_ffi_snprintf_checked(occurs_str, sizeof(occurs_str),
                                             " OCCURS %ld TIMES",
                                             fld->type.array_size));
          } else {
            occurs_str[0] = '\0';
          }

          rc = cdd_c_keep_err(
              rc, cdd_ffi_write_fmt(fp,
                                    "          05 %s REDEFINES %s.\n"
                                    "             10 %s%s %s.\n",
                                    v_word, raw_word, val_word, occurs_str,
                                    usage_str));
          if (rc != CDD_C_SUCCESS)
            break;

          rem_pad = (fld->size_bytes > 0 && u_size > fld->size_bytes)
                        ? (u_size - fld->size_bytes)
                        : fld->union_tail_pad_bytes;
          if (rem_pad > 0) {
            rc = cdd_ffi_write_fmt(fp, "             10 FILLER PIC X(%lu).\n",
                                   (unsigned long)rem_pad);
            if (rc != CDD_C_SUCCESS)
              break;
          }
        }
      }
      if (rc == CDD_C_SUCCESS) {
        rc = cdd_ffi_write_str(fp, "\n");
      }
      rc = cdd_c_keep_err(rc, cobol_scope_free(&var_scope));
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    } else {
      /* CDD_FFI_NODE_TYPEDEF */
      char usage_str[128];
      rc = cdd_c_keep_err(rc,
                          cobol_map_usage_clause(ir, &node->return_or_base_type,
                                                 node->size_bytes, usage_str,
                                                 sizeof(usage_str)));
      rc = cdd_c_keep_err(rc, cdd_ffi_write_fmt(fp, "       01 %s %s.\n\n",
                                                node_words[i], usage_str));
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
  }

cleanup:
  if (fp) {
    rc = cdd_c_keep_err(rc, cdd_ffi_close_output_file(&fp));
  }
  return cdd_c_keep_err(rc, cobol_scope_free(&cpy_scope));
}

/**
 * @brief Checks if a C function parameter is an input `const char *` C string
 * eligible for automatic `PIC X(256)` -> null-terminated `WS-C-STR-n`
 * conversion in `<module>.cbl`.
 *
 * @param[in] fld Parameter field descriptor.
 * @param[out] out_is_cstr Receives 1 if input C string, 0 otherwise.
 * @return CDD_C_SUCCESS.
 */
static NO_DISCARD cdd_c_error_t
cobol_is_const_cstr_param(const cdd_ffi_field_t *fld, int *out_is_cstr) {
  *out_is_cstr =
      (fld->type.pointer_depth == 1 && fld->type.array_size <= 0 &&
       fld->type.kind == CDD_FFI_KIND_INT8 &&
       (fld->type.is_const || fld->intent == CDD_FFI_INTENT_IN) &&
       fld->intent != CDD_FFI_INTENT_OUT && fld->intent != CDD_FFI_INTENT_INOUT)
          ? 1
          : 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Formats the `LINKAGE SECTION` usage/picture clause and `CALL` passing
 * mode (`BY VALUE` vs `BY REFERENCE`) for a function parameter.
 *
 * @param[in] ir Enclosing FFI IR.
 * @param[in] fld Function parameter field.
 * @param[out] out_lk_clause Linkage item clause (e.g. `PIC X(256)` or `USAGE
 * BINARY-LONG SIGNED`).
 * @param[in] lk_cap Capacity of `out_lk_clause`.
 * @param[out] out_by_ref Receives 1 for `BY REFERENCE`, 0 for `BY VALUE`.
 * @param[out] out_is_cstr Receives 1 if `const char *` null-terminated buffer
 * is used.
 * @return CDD_C_SUCCESS on success, or error code on truncation.
 */
static NO_DISCARD cdd_c_error_t cobol_classify_param(
    const cdd_ffi_ir_t *ir, const cdd_ffi_field_t *fld, char *out_lk_clause,
    size_t lk_cap, int *out_by_ref, int *out_is_cstr) {
  int byval_rec = 0;
  const cdd_ffi_ir_node_t *rec_node = NULL;
  cdd_c_error_t rc;

  *out_by_ref = 0;
  *out_is_cstr = 0;

  rc = cobol_is_const_cstr_param(fld, out_is_cstr);
  if (*out_is_cstr) {
    *out_by_ref = 1;
    return cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(out_lk_clause, lk_cap,
                                                       "%s", "PIC X(256)"));
  }

  /* Fixed-size array parameter: passed BY REFERENCE */
  if (fld->type.array_size > 0 && fld->type.pointer_depth == 0) {
    char elem_clause[128];
    size_t elem_sz = fld->size_bytes / (size_t)fld->type.array_size;
    *out_by_ref = 1;
    rc = cdd_c_keep_err(rc, cobol_map_usage_clause(ir, &fld->type, elem_sz,
                                                   elem_clause,
                                                   sizeof(elem_clause)));
    return cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(
                                  out_lk_clause, lk_cap, "OCCURS %ld TIMES %s",
                                  fld->type.array_size, elem_clause));
  }

  /* Struct or union passed by value in C -> routed via C89 shim BY REFERENCE */
  rc = cdd_c_keep_err(
      rc, cobol_is_byval_record(ir, &fld->type, &byval_rec, &rec_node));
  if (byval_rec) {
    *out_by_ref = 1;
    return cdd_c_keep_err(
        rc, cdd_ffi_snprintf_checked(out_lk_clause, lk_cap, "PIC X(%lu)",
                                     (unsigned long)rec_node->size_bytes));
  }

  /* Single-indirection typed `[out]` / `[inout]` / `const T *` pointer ->
   * passed BY REFERENCE */
  if (fld->type.pointer_depth == 1 && fld->type.kind != CDD_FFI_KIND_VOID &&
      fld->type.kind != CDD_FFI_KIND_OPAQUE_PTR &&
      fld->type.kind != CDD_FFI_KIND_FUNCTION_PTR &&
      (fld->intent == CDD_FFI_INTENT_OUT ||
       fld->intent == CDD_FFI_INTENT_INOUT ||
       fld->intent == CDD_FFI_INTENT_IN || fld->type.is_const)) {
    cdd_ffi_type_t deref = fld->type;
    const cdd_ffi_ir_node_t *rn = NULL;
    deref.pointer_depth = 0;

    if (deref.kind == CDD_FFI_KIND_STRUCT_REF ||
        deref.kind == CDD_FFI_KIND_TEMPLATE_STRUCT_REF) {
      rc = cdd_c_keep_err(rc, cobol_find_type_node(ir, deref.ref_name, &rn));
      if (rn && rn->size_bytes > 0) {
        *out_by_ref = 1;
        return cdd_c_keep_err(
            rc, cdd_ffi_snprintf_checked(out_lk_clause, lk_cap, "PIC X(%lu)",
                                         (unsigned long)rn->size_bytes));
      }
    } else if (deref.kind != CDD_FFI_KIND_TYPEDEF_REF) {
      *out_by_ref = 1;
      return cdd_c_keep_err(
          rc, cobol_map_usage_clause(ir, &deref, 0, out_lk_clause, lk_cap));
    }
  }

  /* Pass-by-value C scalar, function pointer, or raw `void *` pointer */
  *out_by_ref = 0;
  return cdd_c_keep_err(rc,
                        cobol_map_usage_clause(ir, &fld->type, fld->size_bytes,
                                               out_lk_clause, lk_cap));
}

/**
 * @brief Sanitizes a C identifier into a valid C89 identifier
 * (`[a-zA-Z_][a-zA-Z0-9_]*`) for `<lib>_cobol_shim.h/.c`.
 *
 * @param[in] raw Source identifier string.
 * @param[in] fallback Fallback string when `raw` is NULL/empty (non-empty).
 * @param[out] out_buf Destination buffer (`out_cap > 96`).
 * @param[in] out_cap Capacity of `out_buf`.
 * @return CDD_C_SUCCESS.
 */
static NO_DISCARD cdd_c_error_t cobol_sanitize_c_ident(const char *raw,
                                                       const char *fallback,
                                                       char *out_buf,
                                                       size_t out_cap) {
  size_t pos = 0;
  size_t i;
  const char *src = (raw && raw[0] != '\0') ? raw : fallback;

  if (isdigit((unsigned char)src[0])) {
    out_buf[pos++] = '_';
  }
  for (i = 0; src[i] != '\0' && pos + 1 < out_cap; i++) {
    unsigned char ch = (unsigned char)src[i];
    out_buf[pos++] = (isalnum(ch) || ch == '_') ? (char)ch : '_';
  }
  out_buf[pos] = '\0';
  return CDD_C_SUCCESS;
}

/**
 * @brief Formats a portable C89 type declaration for `<lib>_cobol_shim.h/.c`.
 *
 * @param[in] ir Enclosing FFI IR.
 * @param[in] type FFI type descriptor.
 * @param[in] var_name Optional variable/parameter name (may be NULL).
 * @param[out] out_decl Destination buffer.
 * @param[in] decl_cap Capacity of `out_decl`.
 * @return CDD_C_SUCCESS on success, or error code on truncation.
 */
static NO_DISCARD cdd_c_error_t
cobol_format_c89_decl(const cdd_ffi_ir_t *ir, const cdd_ffi_type_t *type,
                      const char *var_name, char *out_decl, size_t decl_cap) {
  const char *base_c = "void *";
  char base_buf[160];
  char stars[32];
  int p;
  int eff_ptrs = type->pointer_depth;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  switch (type->kind) {
  case CDD_FFI_KIND_VOID:
    base_c = "void";
    break;
  case CDD_FFI_KIND_BOOL:
  case CDD_FFI_KIND_INT8:
    base_c = (type->pointer_depth > 0 && type->kind == CDD_FFI_KIND_INT8)
                 ? "char"
                 : "signed char";
    break;
  case CDD_FFI_KIND_UINT8:
    base_c = "unsigned char";
    break;
  case CDD_FFI_KIND_INT16:
    base_c = "short";
    break;
  case CDD_FFI_KIND_UINT16:
    base_c = "unsigned short";
    break;
  case CDD_FFI_KIND_INT32:
  case CDD_FFI_KIND_ENUM_REF:
    base_c = "int";
    break;
  case CDD_FFI_KIND_UINT32:
    base_c = "unsigned int";
    break;
  case CDD_FFI_KIND_INT64:
    base_c = "cdd_cob_i64";
    break;
  case CDD_FFI_KIND_UINT64:
    base_c = "cdd_cob_u64";
    break;
  case CDD_FFI_KIND_FLOAT32:
    base_c = "float";
    break;
  case CDD_FFI_KIND_FLOAT64:
    base_c = "double";
    break;
  case CDD_FFI_KIND_FUNCTION_PTR:
    if (var_name) {
      return cdd_ffi_snprintf_checked(out_decl, decl_cap, "void (*%s)(void)",
                                      var_name);
    }
    return cdd_ffi_snprintf_checked(out_decl, decl_cap, "%s", "void (*)(void)");
  case CDD_FFI_KIND_STRUCT_REF:
  case CDD_FFI_KIND_TEMPLATE_STRUCT_REF: {
    const cdd_ffi_ir_node_t *rn = NULL;
    rc = cobol_find_type_node(ir, type->ref_name, &rn);
    if (rn &&
        (rn->kind == CDD_FFI_NODE_STRUCT || rn->kind == CDD_FFI_NODE_UNION) &&
        rn->fields_count > 0) {
      char c_tag[128];
      rc = cdd_c_keep_err(rc, cobol_sanitize_c_ident(rn->name, "anon_rec",
                                                     c_tag, sizeof(c_tag)));
      rc = cdd_c_keep_err(
          rc, cdd_ffi_snprintf_checked(
                  base_buf, sizeof(base_buf), "%s %s",
                  rn->kind == CDD_FFI_NODE_UNION ? "union" : "struct", c_tag));
      base_c = base_buf;
    } else {
      base_c = "void";
      if (eff_ptrs == 0)
        eff_ptrs = 1;
    }
    break;
  }
  case CDD_FFI_KIND_TYPEDEF_REF:
    if (type->ref_name && strcmp(type->ref_name, "size_t") == 0) {
      base_c = "size_t";
    } else {
      base_c = "void";
      if (eff_ptrs == 0)
        eff_ptrs = 1;
    }
    break;
  case CDD_FFI_KIND_OPAQUE_PTR:
  default:
    base_c = "void";
    if (eff_ptrs == 0)
      eff_ptrs = 1;
    break;
  }

  for (p = 0; p < eff_ptrs && (size_t)p + 1 < sizeof(stars); p++) {
    stars[p] = '*';
  }
  stars[p] = '\0';

  if (var_name) {
    if (type->array_size > 0 && eff_ptrs == 0) {
      return cdd_c_keep_err(
          rc, cdd_ffi_snprintf_checked(out_decl, decl_cap, "%s%s %s[%ld]",
                                       type->is_const ? "const " : "", base_c,
                                       var_name, type->array_size));
    }
    return cdd_c_keep_err(rc,
                          cdd_ffi_snprintf_checked(
                              out_decl, decl_cap, "%s%s %s%s",
                              (type->is_const && eff_ptrs > 0) ? "const " : "",
                              base_c, stars, var_name));
  }

  return cdd_c_keep_err(
      rc,
      cdd_ffi_snprintf_checked(out_decl, decl_cap, "%s%s%s%s",
                               (type->is_const && eff_ptrs > 0) ? "const " : "",
                               base_c, eff_ptrs > 0 ? " " : "", stars));
}

/**
 * @brief Emits `<module_stem>.cbl` containing reusable COBOL wrapper
 * subprograms for every exported `CDD_FFI_NODE_FUNCTION` in `ir`.
 *
 * @param[in] ir Layout-annotated FFI IR.
 * @param[in] config Generation configuration.
 * @param[in] cbl_file Output `.cbl` filename.
 * @param[in] cpy_file Copybook filename (`<lib_stem>.cpy`).
 * @param[in] mod_word Sanitized COBOL module word.
 * @return CDD_C_SUCCESS on success, or error code on I/O/OOM failure.
 */
static NO_DISCARD cdd_c_error_t emit_cobol_subprograms(
    const cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config,
    const char *cbl_file, const char *cpy_file, const char *mod_word) {
  FILE *fp = NULL;
  cobol_scope_t prog_scope;
  size_t i;
  size_t j;
  size_t emitted_fns = 0;
  cdd_c_error_t rc;

  memset(&prog_scope, 0, sizeof(prog_scope));

  rc = cdd_ffi_open_output_file(config->output_dir, cbl_file, &fp);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = cdd_ffi_write_fmt(
      fp,
      ">>SOURCE FORMAT IS FREE\n"
      "*> Auto-generated COBOL FFI wrapper subprograms (%s)\n\n",
      cbl_file);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  for (i = 0; i < ir->nodes_count; i++) {
    const cdd_ffi_ir_node_t *fn = &ir->nodes[i];
    char prog_id[COBOL_MAX_WORD_LEN + 1];
    char c_sym[128];
    char call_target[160];
    int needs_shim = 0;
    int cstr_ret = 0;
    int rec_ret = 0;
    int has_ret_slot = 0;
    size_t k;

    if (fn->kind != CDD_FFI_NODE_FUNCTION)
      continue;

    emitted_fns++;

    rc = cdd_ffi_emit_translated_trivia(fp, fn->leading_trivia,
                                        &k_cobol_comment_style);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (fn->doc) {
      rc = cdd_ffi_emit_translated_doc(fp, fn->doc, "", &k_cobol_comment_style);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }

    rc = cobol_scope_make_unique(&prog_scope, fn->name, NULL, prog_id,
                                 sizeof(prog_id));
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = cdd_c_keep_err(
        rc, cobol_sanitize_c_ident(fn->name, "anon_fn", c_sym, sizeof(c_sym)));
    rc = cdd_c_keep_err(
        rc, cobol_check_fn_shim(ir, fn, &needs_shim, &cstr_ret, &rec_ret));

    if (needs_shim) {
      rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(call_target,
                                                       sizeof(call_target),
                                                       "cdd_cobol_%s", c_sym));
    } else {
      rc = cdd_c_keep_err(
          rc, cdd_ffi_snprintf_checked(call_target, sizeof(call_target), "%s",
                                       fn->name ? fn->name : c_sym));
    }

    has_ret_slot = !(fn->return_or_base_type.kind == CDD_FFI_KIND_VOID &&
                     fn->return_or_base_type.pointer_depth == 0);

    rc = cdd_c_keep_err(rc, cdd_ffi_write_fmt(fp,
                                              "IDENTIFICATION DIVISION.\n"
                                              "PROGRAM-ID. %s.\n"
                                              "DATA DIVISION.\n"
                                              "WORKING-STORAGE SECTION.\n"
                                              "COPY \"%s\".\n",
                                              prog_id, cpy_file));
    if (rc != CDD_C_SUCCESS)
      goto cleanup;

    for (j = 0; j < fn->fields_count; j++) {
      char lk_c[128];
      int by_ref = 0;
      int is_cstr = 0;
      rc = cdd_c_keep_err(rc, cobol_classify_param(ir, &fn->fields[j], lk_c,
                                                   sizeof(lk_c), &by_ref,
                                                   &is_cstr));
      if (is_cstr) {
        rc = cdd_c_keep_err(
            rc, cdd_ffi_write_fmt(fp, "01 WS-C-STR-%lu PIC X(257).\n",
                                  (unsigned long)j));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    }
    if (fn->is_variadic) {
      rc = cdd_ffi_write_str(fp, "01 WS-C-VAR-STR PIC X(257).\n");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
    if (cstr_ret) {
      rc = cdd_ffi_write_str(
          fp, "01 WS-RET-LEN USAGE BINARY-LONG UNSIGNED VALUE 256.\n");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }

    /* LINKAGE SECTION */
    if (fn->fields_count > 0 || fn->is_variadic || has_ret_slot) {
      rc = cdd_ffi_write_str(fp, "LINKAGE SECTION.\n");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;

      for (j = 0; j < fn->fields_count; j++) {
        char lk_c[128];
        int by_ref = 0;
        int is_cstr = 0;
        rc = cdd_c_keep_err(rc, cobol_classify_param(ir, &fn->fields[j], lk_c,
                                                     sizeof(lk_c), &by_ref,
                                                     &is_cstr));
        rc = cdd_c_keep_err(rc, cdd_ffi_write_fmt(fp, "01 LS-ARG-%lu %s.\n",
                                                  (unsigned long)j, lk_c));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }

      if (fn->is_variadic) {
        rc = cdd_ffi_write_str(fp, "01 LS-VAR-TAIL PIC X(256).\n");
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }

      if (has_ret_slot) {
        if (cstr_ret) {
          rc = cdd_ffi_write_str(fp, "01 LS-RET PIC X(256).\n");
        } else if (rec_ret) {
          const cdd_ffi_ir_node_t *rn = NULL;
          rc = cdd_c_keep_err(
              rc, cobol_is_byval_record(ir, &fn->return_or_base_type, &rec_ret,
                                        &rn));
          rc = cdd_c_keep_err(rc,
                              cdd_ffi_write_fmt(fp, "01 LS-RET PIC X(%lu).\n",
                                                (unsigned long)rn->size_bytes));
        } else {
          char ret_clause[128];
          rc = cdd_c_keep_err(
              rc, cobol_map_usage_clause(ir, &fn->return_or_base_type,
                                         fn->size_bytes, ret_clause,
                                         sizeof(ret_clause)));
          rc = cdd_c_keep_err(
              rc, cdd_ffi_write_fmt(fp, "01 LS-RET %s.\n", ret_clause));
        }
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    }

    /* PROCEDURE DIVISION header */
    rc = cdd_ffi_write_str(fp, "PROCEDURE DIVISION");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (fn->fields_count > 0 || fn->is_variadic || has_ret_slot) {
      rc = cdd_ffi_write_str(fp, " USING");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      for (j = 0; j < fn->fields_count; j++) {
        rc = cdd_ffi_write_fmt(fp, " LS-ARG-%lu", (unsigned long)j);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
      if (fn->is_variadic) {
        rc = cdd_ffi_write_str(fp, " LS-VAR-TAIL");
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
      if (has_ret_slot) {
        rc = cdd_ffi_write_str(fp, " LS-RET");
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    }
    rc = cdd_ffi_write_str(fp, ".\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;

    /* Null-terminate space-padded COBOL PIC X(n) string inputs */
    for (j = 0; j < fn->fields_count; j++) {
      char lk_c[128];
      int by_ref = 0;
      int is_cstr = 0;
      rc = cdd_c_keep_err(rc, cobol_classify_param(ir, &fn->fields[j], lk_c,
                                                   sizeof(lk_c), &by_ref,
                                                   &is_cstr));
      if (is_cstr) {
        rc = cdd_c_keep_err(
            rc,
            cdd_ffi_write_fmt(
                fp,
                "    MOVE LOW-VALUES TO WS-C-STR-%lu\n"
                "    STRING FUNCTION TRIM(LS-ARG-%lu TRAILING) DELIMITED BY "
                "SIZE\n"
                "           X\"00\" DELIMITED BY SIZE\n"
                "           INTO WS-C-STR-%lu\n"
                "    END-STRING\n",
                (unsigned long)j, (unsigned long)j, (unsigned long)j));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    }
    if (fn->is_variadic) {
      rc = cdd_ffi_write_str(
          fp,
          "    MOVE LOW-VALUES TO WS-C-VAR-STR\n"
          "    STRING FUNCTION TRIM(LS-VAR-TAIL TRAILING) DELIMITED BY SIZE\n"
          "           X\"00\" DELIMITED BY SIZE\n"
          "           INTO WS-C-VAR-STR\n"
          "    END-STRING\n");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }

    /* Emit CALL "<exact_c_symbol>" with explicit BY VALUE / BY REFERENCE */
    rc = cdd_ffi_write_fmt(fp, "    CALL \"%s\"\n", call_target);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;

    {
      size_t call_args = fn->fields_count + (fn->is_variadic ? 1u : 0u) +
                         (rec_ret ? 1u : 0u) + (cstr_ret ? 2u : 0u);
      if (call_args > 0) {
        rc = cdd_ffi_write_str(fp, "        USING\n");
        if (rc != CDD_C_SUCCESS)
          goto cleanup;

        for (k = 0; k < fn->fields_count; k++) {
          char lk_c[128];
          int by_ref = 0;
          int is_cstr = 0;
          rc = cdd_c_keep_err(rc, cobol_classify_param(ir, &fn->fields[k], lk_c,
                                                       sizeof(lk_c), &by_ref,
                                                       &is_cstr));
          if (is_cstr) {
            rc = cdd_c_keep_err(
                rc,
                cdd_ffi_write_fmt(fp, "            BY REFERENCE WS-C-STR-%lu\n",
                                  (unsigned long)k));
          } else {
            rc = cdd_c_keep_err(
                rc, cdd_ffi_write_fmt(fp, "            BY %s LS-ARG-%lu\n",
                                      by_ref ? "REFERENCE" : "VALUE",
                                      (unsigned long)k));
          }
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
        if (fn->is_variadic) {
          rc = cdd_ffi_write_str(fp, "            BY REFERENCE WS-C-VAR-STR\n");
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
        if (rec_ret) {
          rc = cdd_ffi_write_str(fp, "            BY REFERENCE LS-RET\n");
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        } else if (cstr_ret) {
          rc = cdd_ffi_write_str(fp, "            BY REFERENCE LS-RET\n"
                                     "            BY VALUE WS-RET-LEN\n");
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
      }
    }

    rc = cdd_ffi_write_str(fp, (has_ret_slot && !rec_ret && !cstr_ret)
                                   ? "        RETURNING LS-RET\n"
                                   : "        RETURNING OMITTED\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;

    rc = cdd_ffi_write_fmt(fp,
                           "    END-CALL\n"
                           "    GOBACK.\n"
                           "END PROGRAM %s.\n\n",
                           prog_id);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  if (emitted_fns == 0) {
    char stub_prog[COBOL_MAX_WORD_LEN + 1];
    rc = cobol_scope_make_unique(&prog_scope, mod_word, NULL, stub_prog,
                                 sizeof(stub_prog));
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = cdd_ffi_write_fmt(fp,
                           "IDENTIFICATION DIVISION.\n"
                           "PROGRAM-ID. %s.\n"
                           "DATA DIVISION.\n"
                           "WORKING-STORAGE SECTION.\n"
                           "COPY \"%s\".\n"
                           "PROCEDURE DIVISION.\n"
                           "    GOBACK.\n"
                           "END PROGRAM %s.\n",
                           stub_prog, cpy_file, stub_prog);
  }

cleanup:
  rc = cdd_c_keep_err(rc, cdd_ffi_close_output_file(&fp));
  return cdd_c_keep_err(rc, cobol_scope_free(&prog_scope));
}

/**
 * @brief Emits strict ISO C89 `<lib_stem>_cobol_shim.h` and
 * `<lib_stem>_cobol_shim.c` providing `cdd_cobol_str_to_pic_x` and pointer
 * ABI shims for by-value structs/unions, `char *` returns, and C variadics.
 *
 * @param[in] ir Layout-annotated FFI IR.
 * @param[in] config Generation configuration.
 * @param[in] lib_stem Sanitized file stem.
 * @return CDD_C_SUCCESS on success, or error code on I/O/formatting failure.
 */
static NO_DISCARD cdd_c_error_t emit_cobol_c89_shim(
    const cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config,
    const char *lib_stem) {
  char hdr_name[256];
  char src_name[256];
  char guard[160];
  FILE *fh = NULL;
  FILE *fc = NULL;
  size_t i;
  size_t j;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  rc =
      cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(hdr_name, sizeof(hdr_name),
                                                  "%s_cobol_shim.h", lib_stem));
  rc =
      cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(src_name, sizeof(src_name),
                                                  "%s_cobol_shim.c", lib_stem));

  {
    size_t gi;
    char up[COBOL_MAX_FILE_STEM + 1];
    for (gi = 0; lib_stem[gi] != '\0'; gi++) {
      up[gi] = (char)toupper((unsigned char)lib_stem[gi]);
    }
    up[gi] = '\0';
    rc =
        cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(guard, sizeof(guard),
                                                    "CDD_COBOL_SHIM_%s_H", up));
  }

  rc = cdd_c_keep_err(
      rc, cdd_ffi_open_output_file(config->output_dir, hdr_name, &fh));
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_ffi_open_output_file(config->output_dir, src_name, &fc);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_ffi_close_output_file(&fh));
  }

  rc = cdd_ffi_write_fmt(
      fh,
      "/**\n"
      " * @file %s\n"
      " * @brief Strict ISO C89 COBOL ABI shim declarations.\n"
      " */\n"
      "#ifndef %s\n"
      "#define %s\n\n"
      "#ifdef __cplusplus\n"
      "extern \"C\" {\n"
      "#endif /* __cplusplus */\n\n"
      "/* "
      "clang-format off */\n"
      "#include <stddef.h>\n"
      "/* "
      "clang-format on */\n\n",
      hdr_name, guard, guard);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_ffi_write_str(
      fh, "#if defined(_MSC_VER)\n"
          "typedef __int64 cdd_cob_i64;\n"
          "typedef unsigned __int64 cdd_cob_u64;\n"
          "#elif defined(__GNUC__) || defined(__clang__)\n"
          "__extension__ typedef long long cdd_cob_i64;\n"
          "__extension__ typedef unsigned long long cdd_cob_u64;\n"
          "#else\n"
          "typedef long cdd_cob_i64;\n"
          "typedef unsigned long cdd_cob_u64;\n"
          "#endif\n\n"
          "int cdd_cobol_str_to_pic_x(const char *c_str, char *cob_buf,\n"
          "                           unsigned int cob_len);\n\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  /* Emit C89 struct/union definitions needed by shims */
  for (i = 0; i < ir->nodes_count; i++) {
    const cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if ((node->kind == CDD_FFI_NODE_STRUCT ||
         node->kind == CDD_FFI_NODE_UNION) &&
        node->fields_count > 0 && node->name) {
      char c_tag[128];
      const char *kw = (node->kind == CDD_FFI_NODE_UNION) ? "union" : "struct";
      rc = cdd_c_keep_err(rc, cobol_sanitize_c_ident(node->name, "anon_rec",
                                                     c_tag, sizeof(c_tag)));
      rc = cdd_c_keep_err(rc, cdd_ffi_write_fmt(fh, "%s %s {\n", kw, c_tag));
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      for (j = 0; j < node->fields_count; j++) {
        char f_c_name[128];
        char f_fb[32];
        char f_decl[256];
        rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(f_fb, sizeof(f_fb),
                                                         "f_%lu",
                                                         (unsigned long)j));
        rc = cdd_c_keep_err(rc,
                            cobol_sanitize_c_ident(node->fields[j].name, f_fb,
                                                   f_c_name, sizeof(f_c_name)));
        rc = cdd_c_keep_err(rc, cobol_format_c89_decl(ir, &node->fields[j].type,
                                                      f_c_name, f_decl,
                                                      sizeof(f_decl)));
        rc = cdd_c_keep_err(rc, cdd_ffi_write_fmt(fh, "  %s;\n", f_decl));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
      rc = cdd_ffi_write_str(fh, "};\n\n");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
  }

  /* Write C89 shim source header + cdd_cobol_str_to_pic_x */
  rc = cdd_ffi_write_fmt(
      fc,
      "/**\n"
      " * @file %s\n"
      " * @brief Strict ISO C89 COBOL ABI shim implementation.\n"
      " */\n"
      "/* "
      "clang-format off */\n"
      "#include \"%s\"\n"
      "#include <string.h>\n"
      "/* "
      "clang-format on */\n\n",
      src_name, hdr_name);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_ffi_write_str(
      fc, "int cdd_cobol_str_to_pic_x(const char *c_str, char *cob_buf,\n"
          "                           unsigned int cob_len) {\n"
          "  unsigned int copy_n;\n"
          "  size_t s_len;\n"
          "  if (!cob_buf)\n"
          "    return -1;\n"
          "  if (cob_len == 0)\n"
          "    return 0;\n"
          "  memset(cob_buf, ' ', (size_t)cob_len);\n"
          "  if (!c_str)\n"
          "    return 0;\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_ffi_write_str(
      fc,
      "  s_len = strlen(c_str);\n"
      "  copy_n = (s_len < (size_t)cob_len) ? (unsigned int)s_len : cob_len;\n"
      "  if (copy_n > 0) {\n"
      "#if defined(_MSC_VER)\n"
      "    memcpy_s(cob_buf, (size_t)cob_len, c_str, (size_t)copy_n);\n"
      "#else\n"
      "    memcpy(cob_buf, c_str, (size_t)copy_n);\n"
      "#endif\n"
      "  }\n"
      "  return 0;\n"
      "}\n\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  /* Emit function shims where needed (`shim_arity >= 1` whenever `needs_shim`)
   */
  for (i = 0; i < ir->nodes_count; i++) {
    const cdd_ffi_ir_node_t *fn = &ir->nodes[i];
    int needs_shim = 0;
    int cstr_ret = 0;
    int rec_ret = 0;
    char c_sym[128];
    char orig_ret_c[160];
    const char *shim_ret_c;
    size_t shim_arity;
    size_t emitted_p = 0;

    if (fn->kind != CDD_FFI_NODE_FUNCTION)
      continue;

    rc = cdd_c_keep_err(
        rc, cobol_check_fn_shim(ir, fn, &needs_shim, &cstr_ret, &rec_ret));
    if (!needs_shim)
      continue;

    rc = cdd_c_keep_err(
        rc, cobol_sanitize_c_ident(fn->name, "anon_fn", c_sym, sizeof(c_sym)));
    rc = cdd_c_keep_err(rc, cobol_format_c89_decl(ir, &fn->return_or_base_type,
                                                  NULL, orig_ret_c,
                                                  sizeof(orig_ret_c)));
    shim_ret_c = (rec_ret || cstr_ret) ? "void" : orig_ret_c;

    /* Extern prototype of underlying C symbol in shim .c */
    rc = cdd_c_keep_err(
        rc, cdd_ffi_write_fmt(fc, "extern %s %s(", orig_ret_c, c_sym));
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (fn->fields_count == 0 && !fn->is_variadic) {
      rc = cdd_ffi_write_str(fc, "void");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    } else {
      for (j = 0; j < fn->fields_count; j++) {
        char p_decl[256];
        char p_nm[32];
        rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(p_nm, sizeof(p_nm),
                                                         "p_%lu",
                                                         (unsigned long)j));
        rc = cdd_c_keep_err(rc,
                            cobol_format_c89_decl(ir, &fn->fields[j].type, p_nm,
                                                  p_decl, sizeof(p_decl)));
        rc = cdd_c_keep_err(
            rc, cdd_ffi_write_fmt(
                    fc, "%s%s", p_decl,
                    (j + 1 < fn->fields_count || fn->is_variadic) ? ", " : ""));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
      if (fn->is_variadic) {
        rc = cdd_ffi_write_str(
            fc, fn->fields_count == 0 ? "const char *fmt, ..." : "...");
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    }
    rc = cdd_ffi_write_str(fc, ");\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;

    /* Shim signature in both .h and .c */
    shim_arity = fn->fields_count + (fn->is_variadic ? 1u : 0u) +
                 (rec_ret ? 1u : 0u) + (cstr_ret ? 2u : 0u);

    rc = cdd_ffi_write_fmt(fh, "%s cdd_cobol_%s(", shim_ret_c, c_sym);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = cdd_ffi_write_fmt(fc, "%s cdd_cobol_%s(", shim_ret_c, c_sym);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;

    for (j = 0; j < fn->fields_count; j++) {
      int byval_p = 0;
      const cdd_ffi_ir_node_t *rn = NULL;
      char p_decl[256];
      char p_nm[32];

      rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(p_nm, sizeof(p_nm),
                                                       "arg_%lu",
                                                       (unsigned long)j));
      rc = cdd_c_keep_err(
          rc, cobol_is_byval_record(ir, &fn->fields[j].type, &byval_p, &rn));
      if (byval_p) {
        char c_tag[128];
        rc = cdd_c_keep_err(rc, cobol_sanitize_c_ident(rn->name, "anon_rec",
                                                       c_tag, sizeof(c_tag)));
        rc = cdd_c_keep_err(
            rc, cdd_ffi_snprintf_checked(
                    p_decl, sizeof(p_decl), "const %s %s *%s",
                    rn->kind == CDD_FFI_NODE_UNION ? "union" : "struct", c_tag,
                    p_nm));
      } else {
        rc = cdd_c_keep_err(rc,
                            cobol_format_c89_decl(ir, &fn->fields[j].type, p_nm,
                                                  p_decl, sizeof(p_decl)));
      }

      emitted_p++;
      rc = cdd_c_keep_err(
          rc, cdd_ffi_write_fmt(fh, "%s%s", p_decl,
                                emitted_p < shim_arity ? ", " : ""));
      if (rc == CDD_C_SUCCESS) {
        rc = cdd_ffi_write_fmt(fc, "%s%s", p_decl,
                               emitted_p < shim_arity ? ", " : "");
      }
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
    if (fn->is_variadic) {
      emitted_p++;
      rc = cdd_ffi_write_fmt(fh, "const char *var_tail%s",
                             emitted_p < shim_arity ? ", " : "");
      if (rc == CDD_C_SUCCESS) {
        rc = cdd_ffi_write_fmt(fc, "const char *var_tail%s",
                               emitted_p < shim_arity ? ", " : "");
      }
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
    if (rec_ret) {
      const cdd_ffi_ir_node_t *rn = NULL;
      char c_tag[128];
      const char *r_kw;
      rc = cdd_c_keep_err(rc, cobol_is_byval_record(
                                  ir, &fn->return_or_base_type, &rec_ret, &rn));
      rc = cdd_c_keep_err(rc, cobol_sanitize_c_ident(rn->name, "anon_rec",
                                                     c_tag, sizeof(c_tag)));
      r_kw = (rn->kind == CDD_FFI_NODE_UNION) ? "union" : "struct";
      rc = cdd_c_keep_err(rc,
                          cdd_ffi_write_fmt(fh, "%s %s *out_ret", r_kw, c_tag));
      if (rc == CDD_C_SUCCESS) {
        rc = cdd_ffi_write_fmt(fc, "%s %s *out_ret", r_kw, c_tag);
      }
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    } else if (cstr_ret) {
      rc = cdd_ffi_write_str(fh, "char *out_pic_x, unsigned int out_len");
      if (rc == CDD_C_SUCCESS) {
        rc = cdd_ffi_write_str(fc, "char *out_pic_x, unsigned int out_len");
      }
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }

    rc = cdd_ffi_write_str(fh, ");\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = cdd_ffi_write_str(fc, ") {\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;

    /* Shim body (strict C89 declarations at top) */
    if (cstr_ret) {
      rc = cdd_ffi_write_str(fc, "  const char *raw_str;\n  int cp_rc;\n");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    } else if (rec_ret) {
      rc = cdd_ffi_write_fmt(fc, "  %s tmp_ret;\n", orig_ret_c);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }

    for (j = 0; j < fn->fields_count; j++) {
      int byval_p = 0;
      const cdd_ffi_ir_node_t *rn = NULL;
      rc = cdd_c_keep_err(
          rc, cobol_is_byval_record(ir, &fn->fields[j].type, &byval_p, &rn));
      if (byval_p) {
        char c_tag[128];
        rc = cdd_c_keep_err(rc, cobol_sanitize_c_ident(rn->name, "anon_rec",
                                                       c_tag, sizeof(c_tag)));
        rc = cdd_c_keep_err(rc, cdd_ffi_write_fmt(fc, "  %s %s zero_%lu;\n",
                                                  rn->kind == CDD_FFI_NODE_UNION
                                                      ? "union"
                                                      : "struct",
                                                  c_tag, (unsigned long)j));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    }
    for (j = 0; j < fn->fields_count; j++) {
      int byval_p = 0;
      rc = cdd_c_keep_err(
          rc, cobol_is_byval_record(ir, &fn->fields[j].type, &byval_p, NULL));
      if (byval_p) {
        rc = cdd_c_keep_err(
            rc,
            cdd_ffi_write_fmt(fc, "  memset(&zero_%lu, 0, sizeof(zero_%lu));\n",
                              (unsigned long)j, (unsigned long)j));
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    }

    if (cstr_ret) {
      rc = cdd_ffi_write_fmt(fc, "  raw_str = %s(", c_sym);
    } else if (rec_ret) {
      rc = cdd_ffi_write_fmt(fc, "  tmp_ret = %s(", c_sym);
    } else if (fn->return_or_base_type.kind == CDD_FFI_KIND_VOID &&
               fn->return_or_base_type.pointer_depth == 0) {
      rc = cdd_ffi_write_fmt(fc, "  %s(", c_sym);
    } else {
      rc = cdd_ffi_write_fmt(fc, "  return %s(", c_sym);
    }
    if (rc != CDD_C_SUCCESS)
      goto cleanup;

    for (j = 0; j < fn->fields_count; j++) {
      int byval_p = 0;
      rc = cdd_c_keep_err(
          rc, cobol_is_byval_record(ir, &fn->fields[j].type, &byval_p, NULL));
      if (byval_p) {
        rc = cdd_c_keep_err(
            rc, cdd_ffi_write_fmt(fc, "arg_%lu ? *arg_%lu : zero_%lu",
                                  (unsigned long)j, (unsigned long)j,
                                  (unsigned long)j));
      } else {
        rc = cdd_c_keep_err(rc,
                            cdd_ffi_write_fmt(fc, "arg_%lu", (unsigned long)j));
      }
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      if (j + 1 < fn->fields_count || fn->is_variadic) {
        rc = cdd_ffi_write_str(fc, ", ");
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    }
    if (fn->is_variadic) {
      rc = cdd_ffi_write_str(fc, fn->fields_count == 0
                                     ? "\"%s\", var_tail ? var_tail : \"\""
                                     : "var_tail ? var_tail : \"\"");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
    rc = cdd_ffi_write_str(fc, ");\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;

    if (cstr_ret) {
      rc = cdd_ffi_write_str(
          fc, "  cp_rc = cdd_cobol_str_to_pic_x(raw_str, out_pic_x, out_len);\n"
              "  if (cp_rc != 0) {\n"
              "    return;\n"
              "  }\n");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    } else if (rec_ret) {
      rc = cdd_ffi_write_str(fc, "  if (out_ret) {\n"
                                 "    *out_ret = tmp_ret;\n"
                                 "  }\n");
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }

    rc = cdd_ffi_write_str(fc, "}\n\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  rc = cdd_ffi_write_fmt(fh,
                         "\n#ifdef __cplusplus\n"
                         "}\n"
                         "#endif /* __cplusplus */\n\n"
                         "#endif /* %s */\n",
                         guard);

cleanup:
  rc = cdd_c_keep_err(rc, cdd_ffi_close_output_file(&fh));
  return cdd_c_keep_err(rc, cdd_ffi_close_output_file(&fc));
}

/**
 * @brief Emits `<module_stem>_test.cbl` exercising Copybook `FUNCTION
 * BYTE-LENGTH` layout assertions and constant values when
 * `config->generate_tests != 0`.
 *
 * @param[in] ir Layout-annotated FFI IR.
 * @param[in] config Generation configuration.
 * @param[in] mod_stem Sanitized file stem for module.
 * @param[in] cpy_file Copybook filename (`<lib_stem>.cpy`).
 * @param[in] mod_word Sanitized COBOL module word.
 * @param[in] node_words Parallel array of unique COBOL words for `ir->nodes`.
 * @return CDD_C_SUCCESS on success, or error code on I/O failure.
 */
static NO_DISCARD cdd_c_error_t emit_cobol_tests(
    const cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config,
    const char *mod_stem, const char *cpy_file, const char *mod_word,
    char **node_words) {
  char test_file[256];
  char test_prog[COBOL_MAX_WORD_LEN + 1];
  FILE *fp = NULL;
  size_t i;
  cdd_c_error_t rc;

  if (!config->generate_tests)
    return CDD_C_SUCCESS;

  rc = cdd_ffi_snprintf_checked(test_file, sizeof(test_file), "%s_test.cbl",
                                mod_stem);
  rc =
      cdd_c_keep_err(rc, cdd_cobol_sanitize_word(mod_word, "TST-", 0, test_prog,
                                                 sizeof(test_prog)));
  rc = cdd_c_keep_err(
      rc, cdd_ffi_open_output_file(config->output_dir, test_file, &fp));
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = cdd_ffi_write_fmt(
      fp,
      ">>SOURCE FORMAT IS FREE\n"
      "*> Auto-generated COBOL Copybook layout verification program\n"
      "IDENTIFICATION DIVISION.\n"
      "PROGRAM-ID. %s.\n"
      "DATA DIVISION.\n"
      "WORKING-STORAGE SECTION.\n"
      "COPY \"%s\".\n"
      "01 WS-ACTUAL-LEN USAGE BINARY-LONG UNSIGNED VALUE 0.\n"
      "PROCEDURE DIVISION.\n"
      "    MOVE CDD-FFI-ABI-READY TO WS-ACTUAL-LEN\n",
      test_prog, cpy_file);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  for (i = 0; i < ir->nodes_count; i++) {
    const cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if ((node->kind == CDD_FFI_NODE_STRUCT ||
         node->kind == CDD_FFI_NODE_UNION) &&
        node->fields_count > 0 && node->size_bytes > 0) {
      rc = cdd_ffi_write_fmt(
          fp,
          "    MOVE FUNCTION BYTE-LENGTH(%s) TO WS-ACTUAL-LEN\n"
          "    IF WS-ACTUAL-LEN NOT = %lu\n"
          "        STOP RUN RETURNING 1\n"
          "    END-IF\n",
          node_words[i], (unsigned long)node->size_bytes);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
  }

  rc = cdd_ffi_write_fmt(fp,
                         "    GOBACK.\n"
                         "END PROGRAM %s.\n",
                         test_prog);

cleanup:
  return cdd_c_keep_err(rc, cdd_ffi_close_output_file(&fp));
}

/*
 * @brief Emits COBOL FFI bindings (`<lib>.cpy` Copybook, `<module>.cbl` wrapper
 * subprograms, `<lib>_cobol_shim.h/.c` C89 ABI shim, and optional
 * `<module>_test.cbl`) for the given FFI IR.
 *
 * @param[in,out] ir Extracted and topologically sorted FFI IR.
 * @param[in] config Bindings generation configuration.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL args,
 * CDD_C_ERROR_MEMORY on OOM/truncation, or CDD_C_ERROR_IO on file I/O failure.
 */
NO_DISCARD cdd_c_error_t cdd_ffi_emit_cobol(
    cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config) {
  const char *raw_lib;
  const char *raw_mod;
  char lib_stem[COBOL_MAX_FILE_STEM + 1];
  char mod_stem[COBOL_MAX_FILE_STEM + 1];
  char mod_word[COBOL_MAX_WORD_LEN + 1];
  char cpy_file[128];
  char cbl_file[128];
  cobol_scope_t top_scope;
  char **node_words = NULL;
  size_t i;
  cdd_c_error_t rc;

  if (!ir || !config || !config->output_dir)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  memset(&top_scope, 0, sizeof(top_scope));

  /* Run ABI layout calculation first (falling back to relaxed refs for fuzz
   * IRs) */
  rc = cdd_ffi_ir_compute_layout(ir, NULL);
  if (rc == CDD_C_ERROR_NOT_FOUND) {
    cdd_ffi_abi_profile_t relaxed_abi;
    rc = cdd_ffi_abi_profile_init(CDD_FFI_ABI_HOST, &relaxed_abi);
    relaxed_abi.allow_unresolved_refs = 1;
    rc = cdd_c_keep_err(rc, cdd_ffi_ir_compute_layout(ir, &relaxed_abi));
  }
  if (rc != CDD_C_SUCCESS)
    return rc;

  raw_lib = config->library_name
                ? config->library_name
                : (ir->module_name ? ir->module_name : "mylib");
  raw_mod = config->module_name ? config->module_name : raw_lib;

  rc = cdd_c_keep_err(rc, cobol_sanitize_file_stem(raw_lib, "mylib", lib_stem,
                                                   sizeof(lib_stem)));
  rc = cdd_c_keep_err(rc, cobol_sanitize_file_stem(raw_mod, lib_stem, mod_stem,
                                                   sizeof(mod_stem)));
  rc = cdd_c_keep_err(rc, cdd_cobol_sanitize_word(raw_mod, NULL, 0, mod_word,
                                                  sizeof(mod_word)));
  rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(cpy_file, sizeof(cpy_file),
                                                   "%s.cpy", lib_stem));
  rc = cdd_c_keep_err(rc, cdd_ffi_snprintf_checked(cbl_file, sizeof(cbl_file),
                                                   "%s.cbl", mod_stem));

  if (ir->nodes_count > 0) {
    node_words = (char **)C_CDD_CALLOC(ir->nodes_count, sizeof(char *));
    if (!node_words) {
      rc = CDD_C_ERROR_MEMORY;
      goto cleanup;
    }
    for (i = 0; i < ir->nodes_count; i++) {
      char wbuf[COBOL_MAX_WORD_LEN + 1];
      rc = cobol_scope_make_unique(&top_scope, ir->nodes[i].name, NULL, wbuf,
                                   sizeof(wbuf));
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      node_words[i] = C_CDD_STRDUP(wbuf);
      if (!node_words[i]) {
        rc = CDD_C_ERROR_MEMORY;
        goto cleanup;
      }
    }
  }

  rc = emit_cobol_copybook(ir, config, cpy_file, node_words);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = emit_cobol_subprograms(ir, config, cbl_file, cpy_file, mod_word);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = emit_cobol_c89_shim(ir, config, lib_stem);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = emit_cobol_tests(ir, config, mod_stem, cpy_file, mod_word, node_words);

cleanup:
  if (node_words) {
    for (i = 0; i < ir->nodes_count; i++) {
      C_CDD_FREE(node_words[i]);
    }
    C_CDD_FREE(node_words);
  }
  return cdd_c_keep_err(rc, cobol_scope_free(&top_scope));
}
