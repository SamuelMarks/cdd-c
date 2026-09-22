#include "cdd_test_helpers_export.h"
CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void);
/**
 * @file test_safe_crt.h
 * @brief Unit tests for the Safe CRT transformer.
 */

#ifndef TEST_CDD_TRANSFORM_SAFE_CRT_H
#define TEST_CDD_TRANSFORM_SAFE_CRT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <string.h>
#include <stdlib.h>
#include "cdd_cst_transform.h"
#include "classes/parse/cdd_cst_parser.h"
#include "classes/emit/cdd_cst_emit.h"
#include "c_str_span.h"
/* clang-format on */

/* Moved extern declarations for C89 compliance */
extern C_CDD_EXPORT int g_safe_crt_malloc_fail;
extern C_CDD_EXPORT int g_cdd_cst_alloc_node_fail;
extern C_CDD_EXPORT int g_cdd_query_err_fail;

TEST test_cdd_transform_safe_crt(void) {
  cdd_cst_tree_t *tree = NULL;

  const char code[] = {
      '#',  'd',  'e',  'f',  'i',  'n',  'e',  ' ',  'M',  'Y',  '_',  'M',
      'A',  'C',  'R',  'O',  '_',  'C',  'O',  'N',  'S',  'T',  ' ',  '1',
      '0',  '2',  '4',  '\n', 'v',  'o',  'i',  'd',  ' ',  'f',  'o',  'o',
      '(',  ')',  ' ',  '{',  '\n', ' ',  ' ',  'c',  'h',  'a',  'r',  ' ',
      'd',  'e',  's',  't',  '[',  '1',  '0',  ']',  ';',  '\n', ' ',  ' ',
      'c',  'h',  'a',  'r',  ' ',  'd',  'e',  's',  't',  '2',  '[',  '1',
      '0',  ']',  ';',  '\n', ' ',  ' ',  'c',  'h',  'a',  'r',  ' ',  'd',
      'e',  's',  't',  '3',  '[',  '1',  '0',  ']',  ';',  '\n', ' ',  ' ',
      'w',  'c',  'h',  'a',  'r',  '_',  't',  ' ',  'w',  'd',  'e',  's',
      't',  '[',  '1',  '0',  ']',  ';',  '\n', ' ',  ' ',  'e',  'n',  'u',
      'm',  ' ',  '{',  ' ',  'M',  'Y',  '_',  'C',  'O',  'N',  'S',  'T',
      ' ',  '=',  ' ',  '5',  '1',  '2',  ' ',  '}',  ';',  '\n', ' ',  ' ',
      'c',  'h',  'a',  'r',  ' ',  '*',  'd',  'y',  'n',  '_',  'b',  'u',
      'f',  ' ',  '=',  ' ',  'm',  'a',  'l',  'l',  'o',  'c',  '(',  '2',
      '5',  '6',  ')',  ';',  '\n', ' ',  ' ',  'c',  'h',  'a',  'r',  ' ',
      '*',  'c',  '_',  'b',  'u',  'f',  ' ',  '=',  ' ',  'c',  'a',  'l',
      'l',  'o',  'c',  '(',  '1',  '0',  ',',  ' ',  '2',  ')',  ';',  '\n',
      ' ',  ' ',  'c',  'h',  'a',  'r',  ' ',  '*',  'r',  '_',  'b',  'u',
      'f',  ' ',  '=',  ' ',  'r',  'e',  'a',  'l',  'l',  'o',  'c',  '(',
      'N',  'U',  'L',  'L',  ',',  ' ',  '1',  '0',  '2',  '4',  ')',  ';',
      '\n', ' ',  ' ',  'c',  'h',  'a',  'r',  ' ',  '*',  'e',  'n',  'u',
      'm',  '_',  'b',  'u',  'f',  ' ',  '=',  ' ',  'm',  'a',  'l',  'l',
      'o',  'c',  '(',  'M',  'Y',  '_',  'C',  'O',  'N',  'S',  'T',  ')',
      ';',  '\n', ' ',  ' ',  'c',  'h',  'a',  'r',  ' ',  '*',  'm',  'a',
      'c',  '_',  'b',  'u',  'f',  ' ',  '=',  ' ',  'm',  'a',  'l',  'l',
      'o',  'c',  '(',  'M',  'Y',  '_',  'M',  'A',  'C',  'R',  'O',  '_',
      'C',  'O',  'N',  'S',  'T',  ')',  ';',  '\n', ' ',  ' ',  'c',  'o',
      'n',  's',  't',  ' ',  'i',  'n',  't',  ' ',  'M',  'Y',  '_',  'I',
      'N',  'T',  '_',  'C',  'O',  'N',  'S',  'T',  ' ',  '=',  ' ',  '1',
      '2',  '8',  ';',  '\n', ' ',  ' ',  'c',  'h',  'a',  'r',  ' ',  '*',
      'c',  'o',  'n',  's',  't',  '_',  'b',  'u',  'f',  ' ',  '=',  ' ',
      'm',  'a',  'l',  'l',  'o',  'c',  '(',  'M',  'Y',  '_',  'I',  'N',
      'T',  '_',  'C',  'O',  'N',  'S',  'T',  ')',  ';',  '\n', ' ',  ' ',
      't',  'y',  'p',  'e',  'd',  'e',  'f',  ' ',  'c',  'h',  'a',  'r',
      ' ',  'M',  'y',  'C',  'h',  'a',  'r',  ';',  '\n', ' ',  ' ',  'M',
      'y',  'C',  'h',  'a',  'r',  ' ',  't',  '_',  'b',  'u',  'f',  '[',
      '6',  '4',  ']',  ';',  '\n', ' ',  ' ',  'i',  'n',  't',  ' ',  'i',
      'd',  'x',  ';',  '\n', ' ',  ' ',  'c',  'h',  'a',  'r',  ' ',  'c',
      'h',  ';',  '\n', ' ',  ' ',  's',  't',  'r',  'c',  'p',  'y',  '(',
      'd',  'y',  'n',  '_',  'b',  'u',  'f',  ',',  ' ',  '"',  't',  'e',
      's',  't',  '"',  ')',  ';',  '\n', ' ',  ' ',  's',  't',  'r',  'c',
      'p',  'y',  '(',  'c',  '_',  'b',  'u',  'f',  ',',  ' ',  '"',  't',
      'e',  's',  't',  '"',  ')',  ';',  '\n', ' ',  ' ',  's',  't',  'r',
      'c',  'p',  'y',  '(',  'r',  '_',  'b',  'u',  'f',  ',',  ' ',  '"',
      't',  'e',  's',  't',  '"',  ')',  ';',  '\n', ' ',  ' ',  's',  't',
      'r',  'c',  'p',  'y',  '(',  'e',  'n',  'u',  'm',  '_',  'b',  'u',
      'f',  ',',  ' ',  '"',  't',  'e',  's',  't',  '"',  ')',  ';',  '\n',
      ' ',  ' ',  's',  't',  'r',  'c',  'p',  'y',  '(',  'm',  'a',  'c',
      '_',  'b',  'u',  'f',  ',',  ' ',  '"',  't',  'e',  's',  't',  '"',
      ')',  ';',  '\n', ' ',  ' ',  's',  't',  'r',  'c',  'p',  'y',  '(',
      'c',  'o',  'n',  's',  't',  '_',  'b',  'u',  'f',  ',',  ' ',  '"',
      't',  'e',  's',  't',  '"',  ')',  ';',  '\n', ' ',  ' ',  's',  't',
      'r',  'c',  'p',  'y',  '(',  't',  '_',  'b',  'u',  'f',  ',',  ' ',
      '"',  't',  'e',  's',  't',  '"',  ')',  ';',  '\n', ' ',  ' ',  's',
      't',  'r',  'c',  'p',  'y',  '(',  'd',  'e',  's',  't',  ' ',  '+',
      ' ',  '2',  ',',  ' ',  '"',  't',  'e',  's',  't',  '"',  ')',  ';',
      '\n', ' ',  ' ',  's',  't',  'r',  'c',  'p',  'y',  '(',  '&',  'd',
      'e',  's',  't',  '[',  '3',  ']',  ',',  ' ',  '"',  't',  'e',  's',
      't',  '"',  ')',  ';',  '\n', ' ',  ' ',  'i',  'n',  't',  ' ',  'i',
      'd',  'x',  ';',  '\n', ' ',  ' ',  'c',  'h',  'a',  'r',  ' ',  'c',
      'h',  ';',  '\n', ' ',  ' ',  'd',  'o',  'u',  'b',  'l',  'e',  ' ',
      'd',  ' ',  '=',  ' ',  '1',  '.',  '2',  '3',  ';',  '\n', ' ',  ' ',
      'c',  'h',  'a',  'r',  ' ',  '*',  'p',  ' ',  '=',  ' ',  '_',  'g',
      'c',  'v',  't',  '(',  'd',  ',',  ' ',  '5',  ',',  ' ',  'd',  'e',
      's',  't',  ')',  ';',  '\n', ' ',  ' ',  'm',  'b',  's',  't',  'o',
      'w',  'c',  's',  '(',  'w',  'd',  'e',  's',  't',  ',',  ' ',  '"',
      'a',  'b',  'c',  '"',  ',',  ' ',  '3',  ')',  ';',  '\n', ' ',  ' ',
      'w',  'c',  's',  't',  'o',  'm',  'b',  's',  '(',  'd',  'e',  's',
      't',  ',',  ' ',  'L',  '"',  'a',  'b',  'c',  '"',  ',',  ' ',  '3',
      ')',  ';',  '\n', ' ',  ' ',  'w',  'c',  't',  'o',  'm',  'b',  '(',
      'd',  'e',  's',  't',  ',',  ' ',  'L',  '\'', 'a',  '\'', ')',  ';',
      '\n', ' ',  ' ',  's',  't',  'r',  'c',  'p',  'y',  '(',  'd',  'e',
      's',  't',  ',',  ' ',  '"',  'a',  'b',  'c',  '"',  ')',  ';',  '\n',
      ' ',  ' ',  's',  't',  'r',  'n',  'c',  'p',  'y',  '(',  'd',  'e',
      's',  't',  ',',  ' ',  '"',  'd',  'e',  'f',  '"',  ',',  ' ',  '3',
      ')',  ';',  '\n', ' ',  ' ',  's',  'p',  'r',  'i',  'n',  't',  'f',
      '(',  'd',  'e',  's',  't',  ',',  ' ',  '"',  '%',  'd',  '"',  ',',
      ' ',  '1',  ')',  ';',  '\n', ' ',  ' ',  'F',  'I',  'L',  'E',  ' ',
      '*',  'f',  ';',  '\n', ' ',  ' ',  '#',  'i',  'f',  ' ',  'd',  'e',
      'f',  'i',  'n',  'e',  'd',  '(',  '_',  'M',  'S',  'C',  '_',  'V',
      'E',  'R',  ')',  '\n', ' ',  ' ',  'i',  'f',  '(',  'f',  'o',  'p',
      'e',  'n',  '_',  's',  '(',  '&',  'f',  ',',  ' ',  '"',  'a',  '.',
      't',  'x',  't',  '"',  ',',  ' ',  '"',  'r',  '"',  ')',  ' ',  '!',
      '=',  ' ',  '0',  ')',  ' ',  'f',  ' ',  '=',  ' ',  'N',  'U',  'L',
      'L',  ';',  '\n', '#',  'e',  'l',  's',  'e',  '\n', ' ',  ' ',  'f',
      ' ',  '=',  ' ',  'f',  'o',  'p',  'e',  'n',  '(',  '"',  'a',  '.',
      't',  'x',  't',  '"',  ',',  ' ',  '"',  'r',  '"',  ')',  ';',  '\n',
      '#',  'e',  'n',  'd',  'i',  'f',  ' ',  '\n', ' ',  ' ',  'F',  'I',
      'L',  'E',  ' ',  '*',  'f',  '2',  ';',  '\n', '#',  'i',  'f',  ' ',
      'd',  'e',  'f',  'i',  'n',  'e',  'd',  ' ',  '(',  '_',  'M',  'S',
      'C',  '_',  'V',  'E',  'R',  ')',  '\n', ' ',  ' ',  'i',  'f',  ' ',
      '(',  'f',  'o',  'p',  'e',  'n',  '_',  's',  '(',  '&',  'f',  '2',
      ',',  ' ',  '"',  'b',  '.',  't',  'x',  't',  '"',  ',',  ' ',  '"',
      'r',  '"',  ')',  ' ',  '!',  '=',  ' ',  '0',  ')',  ' ',  'f',  '2',
      ' ',  '=',  ' ',  'N',  'U',  'L',  'L',  ';',  '\n', '#',  'e',  'l',
      's',  'e',  '\n', ' ',  ' ',  'f',  '2',  ' ',  '=',  ' ',  'f',  'o',
      'p',  'e',  'n',  '(',  '"',  'b',  '.',  't',  'x',  't',  '"',  ',',
      ' ',  '"',  'r',  '"',  ')',  ';',  '\n', '#',  'e',  'n',  'd',  'i',
      'f',  ' ',  '\n', ' ',  ' ',  's',  't',  'r',  'c',  'a',  't',  '(',
      'd',  'e',  's',  't',  ',',  ' ',  '"',  'x',  '"',  ')',  ';',  '\n',
      ' ',  ' ',  's',  't',  'r',  'n',  'c',  'a',  't',  '(',  'd',  'e',
      's',  't',  ',',  ' ',  '"',  'y',  '"',  ',',  ' ',  '1',  ')',  ';',
      '\n', ' ',  ' ',  'm',  'e',  'm',  'c',  'p',  'y',  '(',  'd',  'e',
      's',  't',  ',',  ' ',  '"',  'z',  '"',  ',',  ' ',  '1',  ')',  ';',
      '\n', ' ',  ' ',  'm',  'e',  'm',  'm',  'o',  'v',  'e',  '(',  'd',
      'e',  's',  't',  ',',  ' ',  '"',  'w',  '"',  ',',  ' ',  '1',  ')',
      ';',  '\n', ' ',  ' ',  's',  't',  'r',  'c',  'p',  'y',  '(',  'd',
      'e',  's',  't',  ',',  ' ',  's',  't',  'r',  'c',  'p',  'y',  '(',
      'd',  'e',  's',  't',  ',',  ' ',  '"',  'n',  'e',  's',  't',  'e',
      'd',  '"',  ')',  ')',  ';',  '\n', ' ',  ' ',  's',  'n',  'p',  'r',
      'i',  'n',  't',  'f',  '(',  'd',  'e',  's',  't',  ',',  ' ',  '1',
      '0',  ',',  ' ',  '"',  '%',  'd',  '"',  ',',  ' ',  '2',  ')',  ';',
      '\n', ' ',  ' ',  'p',  'r',  'i',  'n',  't',  'f',  '(',  '"',  '%',
      's',  '"',  ',',  ' ',  'd',  'e',  's',  't',  ')',  ';',  '\n', ' ',
      ' ',  'f',  'p',  'r',  'i',  'n',  't',  'f',  '(',  'f',  ',',  ' ',
      '"',  '%',  's',  '"',  ',',  ' ',  'd',  'e',  's',  't',  ')',  ';',
      '\n', ' ',  ' ',  's',  'c',  'a',  'n',  'f',  '(',  '"',  '%',  's',
      ' ',  '%',  'd',  ' ',  '%',  'c',  '"',  ',',  ' ',  'd',  'e',  's',
      't',  ',',  ' ',  '&',  'i',  'd',  'x',  ',',  ' ',  '&',  'c',  'h',
      ')',  ';',  '\n', ' ',  ' ',  'f',  's',  'c',  'a',  'n',  'f',  '(',
      'f',  ',',  ' ',  '"',  '%',  '1',  '0',  's',  '"',  ',',  ' ',  'd',
      'e',  's',  't',  ')',  ';',  '\n', ' ',  ' ',  's',  's',  'c',  'a',
      'n',  'f',  '(',  'd',  'e',  's',  't',  ',',  ' ',  '"',  '%',  '[',
      'a',  '-',  'z',  ']',  ' ',  '%',  '%',  ' ',  '%',  '*',  'd',  ' ',
      '%',  's',  '"',  ',',  ' ',  'd',  'e',  's',  't',  '2',  ',',  ' ',
      'd',  'e',  's',  't',  '3',  ')',  ';',  '\n', ' ',  ' ',  '_',  'i',
      't',  'o',  'a',  '(',  'i',  'd',  'x',  ',',  ' ',  'd',  'e',  's',
      't',  ',',  ' ',  '1',  '0',  ')',  ';',  '\n', ' ',  ' ',  '_',  'm',
      'b',  's',  'c',  'p',  'y',  '(',  'd',  'e',  's',  't',  ',',  ' ',
      '"',  'a',  'b',  'c',  '"',  ')',  ';',  '\n', ' ',  ' ',  '_',  'm',
      'b',  's',  'n',  'c',  'p',  'y',  '(',  'd',  'e',  's',  't',  ',',
      ' ',  '"',  'a',  'b',  'c',  '"',  ',',  ' ',  '2',  ')',  ';',  '\n',
      ' ',  ' ',  '_',  'm',  'b',  's',  'c',  'a',  't',  '(',  'd',  'e',
      's',  't',  ',',  ' ',  '"',  'a',  'b',  'c',  '"',  ')',  ';',  '\n',
      ' ',  ' ',  '_',  'm',  'b',  's',  'n',  'c',  'a',  't',  '(',  'd',
      'e',  's',  't',  ',',  ' ',  '"',  'a',  'b',  'c',  '"',  ',',  ' ',
      '2',  ')',  ';',  '\n', ' ',  ' ',  '_',  'm',  'b',  's',  'l',  'w',
      'r',  '(',  'd',  'e',  's',  't',  ')',  ';',  '\n', ' ',  ' ',  '_',
      'm',  'b',  's',  'u',  'p',  'r',  '(',  'd',  'e',  's',  't',  ')',
      ';',  '\n', ' ',  ' ',  '_',  'm',  'b',  's',  's',  'e',  't',  '(',
      'd',  'e',  's',  't',  ',',  ' ',  '\'', 'a',  '\'', ')',  ';',  '\n',
      ' ',  ' ',  '_',  'm',  'b',  's',  'n',  's',  'e',  't',  '(',  'd',
      'e',  's',  't',  ',',  ' ',  '\'', 'a',  '\'', ',',  ' ',  '2',  ')',
      ';',  '\n', ' ',  ' ',  '_',  'w',  'c',  's',  'l',  'w',  'r',  '(',
      'w',  'd',  'e',  's',  't',  ')',  ';',  '\n', ' ',  ' ',  '_',  'w',
      'c',  's',  'u',  'p',  'r',  '(',  'w',  'd',  'e',  's',  't',  ')',
      ';',  '\n', ' ',  ' ',  'f',  ' ',  '=',  ' ',  'f',  'r',  'e',  'o',
      'p',  'e',  'n',  '(',  '"',  'a',  '.',  't',  'x',  't',  '"',  ',',
      ' ',  '"',  'w',  '"',  ',',  ' ',  'f',  ')',  ';',  '\n', ' ',  ' ',
      'F',  'I',  'L',  'E',  ' ',  '*',  'f',  '3',  ' ',  '=',  ' ',  '_',
      'w',  'f',  'o',  'p',  'e',  'n',  '(',  'L',  '"',  'a',  '.',  't',
      'x',  't',  '"',  ',',  ' ',  'L',  '"',  'r',  '"',  ')',  ';',  '\n',
      ' ',  ' ',  'F',  'I',  'L',  'E',  ' ',  '*',  'f',  '4',  ';',  '\n',
      '#',  'i',  'f',  ' ',  'd',  'e',  'f',  'i',  'n',  'e',  'd',  ' ',
      '(',  '_',  'M',  'S',  'C',  '_',  'V',  'E',  'R',  ')',  '\n', ' ',
      ' ',  'i',  'f',  ' ',  '(',  '(',  '(',  'f',  '4',  ' ',  '=',  ' ',
      'c',  'd',  'd',  '_',  't',  'e',  's',  't',  '_',  't',  'm',  'p',
      'f',  'i',  'l',  'e',  '_',  'g',  'l',  'o',  'b',  'a',  'l',  '(',
      ')',  ')',  ' ',  '=',  '=',  ' ',  'N',  'U',  'L',  'L',  ')',  ')',
      ' ',  'f',  '4',  ' ',  '=',  ' ',  'N',  'U',  'L',  'L',  ';',  '\n',
      '#',  'e',  'l',  's',  'e',  '\n', ' ',  ' ',  'f',  '4',  ' ',  '=',
      ' ',  'c',  'd',  'd',  '_',  't',  'e',  's',  't',  '_',  't',  'm',
      'p',  'f',  'i',  'l',  'e',  '_',  'g',  'l',  'o',  'b',  'a',  'l',
      '(',  ')',  ';',  '\n', '#',  'e',  'n',  'd',  'i',  'f',  ' ',  '\n',
      ' ',  ' ',  '_',  's',  'p',  'l',  'i',  't',  'p',  'a',  't',  'h',
      '(',  'd',  'e',  's',  't',  ',',  ' ',  'd',  'e',  's',  't',  ',',
      ' ',  'd',  'e',  's',  't',  ',',  ' ',  'd',  'e',  's',  't',  ',',
      ' ',  'd',  'e',  's',  't',  ')',  ';',  '\n', ' ',  ' ',  '_',  'm',
      'a',  'k',  'e',  'p',  'a',  't',  'h',  '(',  'd',  'e',  's',  't',
      ',',  ' ',  'd',  'e',  's',  't',  ',',  ' ',  'd',  'e',  's',  't',
      ',',  ' ',  'd',  'e',  's',  't',  ',',  ' ',  'd',  'e',  's',  't',
      ')',  ';',  '\n', ' ',  ' ',  '_',  's',  'e',  'a',  'r',  'c',  'h',
      'e',  'n',  'v',  '(',  '"',  'x',  '"',  ',',  ' ',  '"',  'P',  'A',
      'T',  'H',  '"',  ',',  ' ',  'd',  'e',  's',  't',  ')',  ';',  '\n',
      ' ',  ' ',  '_',  'w',  's',  'e',  'a',  'r',  'c',  'h',  'e',  'n',
      'v',  '(',  'L',  '"',  'x',  '"',  ',',  ' ',  'L',  '"',  'P',  'A',
      'T',  'H',  '"',  ',',  ' ',  'w',  'd',  'e',  's',  't',  ')',  ';',
      '\n', ' ',  ' ',  'p',  ' ',  '=',  ' ',  'g',  'e',  't',  'e',  'n',
      'v',  '(',  '"',  'P',  'A',  'T',  'H',  '"',  ')',  ';',  '\n', ' ',
      ' ',  'w',  'd',  'e',  's',  't',  '[',  '0',  ']',  ' ',  '=',  ' ',
      '(',  'w',  'c',  'h',  'a',  'r',  '_',  't',  ')',  '_',  'w',  'g',
      'e',  't',  'e',  'n',  'v',  '(',  'L',  '"',  'P',  'A',  'T',  'H',
      '"',  ')',  ';',  '\n', ' ',  ' ',  '_',  'p',  'u',  't',  'e',  'n',
      'v',  '(',  '"',  'A',  '=',  'B',  '"',  ')',  ';',  '\n', ' ',  ' ',
      '_',  'w',  'p',  'u',  't',  'e',  'n',  'v',  '(',  'L',  '"',  'A',
      '=',  'B',  '"',  ')',  ';',  '\n', ' ',  ' ',  'q',  's',  'o',  'r',
      't',  '(',  'd',  'e',  's',  't',  ',',  ' ',  '1',  '0',  ',',  ' ',
      '1',  ',',  ' ',  'f',  'o',  'o',  ')',  ';',  '\n', ' ',  ' ',  'b',
      's',  'e',  'a',  'r',  'c',  'h',  '(',  '"',  'a',  '"',  ',',  ' ',
      'd',  'e',  's',  't',  ',',  ' ',  '1',  '0',  ',',  ' ',  '1',  ',',
      ' ',  'f',  'o',  'o',  ')',  ';',  '\n', ' ',  ' ',  '1',  ' ',  '?',
      ' ',  's',  't',  'r',  'c',  'p',  'y',  '(',  'd',  'e',  's',  't',
      ',',  ' ',  '"',  'b',  '"',  ')',  ' ',  ':',  ' ',  's',  't',  'r',
      'c',  'p',  'y',  '(',  'd',  'e',  's',  't',  ',',  ' ',  '"',  'c',
      '"',  ')',  ';',  '\n', ' ',  ' ',  '(',  'v',  'o',  'i',  'd',  ')',
      '0',  ',',  ' ',  's',  't',  'r',  'c',  'p',  'y',  '(',  'd',  'e',
      's',  't',  ',',  ' ',  '"',  'd',  '"',  ')',  ';',  '\n', ' ',  ' ',
      'i',  'f',  ' ',  '(',  '1',  ')',  ' ',  's',  't',  'r',  'c',  'p',
      'y',  '(',  'd',  'e',  's',  't',  ',',  ' ',  '"',  'e',  '"',  ')',
      ';',  '\n', ' ',  ' ',  'i',  'f',  ' ',  '(',  '1',  ')',  ' ',  'i',
      'f',  ' ',  '(',  '1',  ')',  ' ',  's',  't',  'r',  'e',  'r',  'r',
      'o',  'r',  '(',  '1',  ')',  ';',  '\n', ' ',  ' ',  'i',  'f',  ' ',
      '(',  '1',  ')',  ' ',  'i',  'f',  ' ',  '(',  '1',  ')',  ' ',  '_',
      'w',  'c',  's',  'e',  'r',  'r',  'o',  'r',  '(',  '1',  ')',  ';',
      '\n', ' ',  ' ',  'i',  'f',  ' ',  '(',  '1',  ')',  ' ',  's',  't',
      'r',  't',  'o',  'k',  '(',  'd',  'e',  's',  't',  ',',  ' ',  '"',
      'a',  '"',  ')',  ';',  '\n', ' ',  ' ',  'i',  'f',  ' ',  '(',  '1',
      ')',  ' ',  'w',  'c',  's',  't',  'o',  'k',  '(',  'w',  'd',  'e',
      's',  't',  ',',  ' ',  'L',  '"',  'a',  '"',  ')',  ';',  '\n', ' ',
      ' ',  'i',  'f',  ' ',  '(',  '1',  ')',  ' ',  '_',  'm',  'b',  's',
      't',  'o',  'k',  '(',  'd',  'e',  's',  't',  ',',  ' ',  '"',  'a',
      '"',  ')',  ';',  '\n', ' ',  ' ',  'i',  'f',  ' ',  '(',  '1',  ')',
      ' ',  'i',  'f',  ' ',  '(',  '1',  ')',  ' ',  '_',  'e',  'c',  'v',
      't',  '(',  'd',  ',',  ' ',  '1',  ',',  ' ',  '&',  'i',  'd',  'x',
      ',',  ' ',  '&',  'i',  'd',  'x',  ')',  ';',  '\n', ' ',  ' ',  'i',
      'f',  ' ',  '(',  '1',  ')',  ' ',  'i',  'f',  ' ',  '(',  '1',  ')',
      ' ',  '_',  'f',  'c',  'v',  't',  '(',  'd',  ',',  ' ',  '1',  ',',
      ' ',  '&',  'i',  'd',  'x',  ',',  ' ',  '&',  'i',  'd',  'x',  ')',
      ';',  '\n', ' ',  ' ',  'i',  'f',  ' ',  '(',  '1',  ')',  ' ',  'i',
      'f',  ' ',  '(',  '1',  ')',  ' ',  'g',  'e',  't',  'e',  'n',  'v',
      '(',  '"',  'A',  '"',  ')',  ';',  '\n', ' ',  ' ',  'i',  'f',  ' ',
      '(',  '1',  ')',  ' ',  'i',  'f',  ' ',  '(',  '1',  ')',  ' ',  '_',
      'w',  'g',  'e',  't',  'e',  'n',  'v',  '(',  'L',  '"',  'A',  '"',
      ')',  ';',  '\n', ' ',  ' ',  'f',  'o',  'r',  ' ',  '(',  ';',  ';',
      ')',  ' ',  's',  't',  'r',  'c',  'p',  'y',  '(',  'd',  'e',  's',
      't',  ',',  ' ',  '"',  'f',  '"',  ')',  ';',  '\n', ' ',  ' ',  'i',
      'd',  'x',  ' ',  '=',  ' ',  '(',  'i',  'n',  't',  ')',  's',  't',
      'r',  'l',  'e',  'n',  '(',  'd',  'e',  's',  't',  ')',  ';',  '\n',
      ' ',  ' ',  'i',  'f',  ' ',  '(',  '1',  ')',  ' ',  's',  't',  'r',
      'e',  'r',  'r',  'o',  'r',  '(',  '1',  ')',  ';',  '\n', ' ',  ' ',
      'i',  'f',  ' ',  '(',  '1',  ')',  ' ',  '_',  'w',  'c',  's',  'e',
      'r',  'r',  'o',  'r',  '(',  '1',  ')',  ';',  '\n', ' ',  ' ',  's',
      't',  'r',  't',  'o',  'k',  '(',  'd',  'e',  's',  't',  ',',  ' ',
      '"',  'a',  '"',  ')',  ';',  '\n', ' ',  ' ',  'w',  'c',  's',  't',
      'o',  'k',  '(',  'w',  'd',  'e',  's',  't',  ',',  ' ',  'L',  '"',
      'a',  '"',  ')',  ';',  '\n', ' ',  ' ',  '_',  'm',  'b',  's',  't',
      'o',  'k',  '(',  'd',  'e',  's',  't',  ',',  ' ',  '"',  'a',  '"',
      ')',  ';',  '\n', ' ',  ' ',  'i',  'f',  ' ',  '(',  '1',  ')',  ' ',
      '_',  'e',  'c',  'v',  't',  '(',  'd',  ',',  ' ',  '1',  ',',  ' ',
      '&',  'i',  'd',  'x',  ',',  ' ',  '&',  'i',  'd',  'x',  ')',  ';',
      '\n', ' ',  ' ',  'i',  'f',  ' ',  '(',  '1',  ')',  ' ',  '_',  'f',
      'c',  'v',  't',  '(',  'd',  ',',  ' ',  '1',  ',',  ' ',  '&',  'i',
      'd',  'x',  ',',  ' ',  '&',  'i',  'd',  'x',  ')',  ';',  '\n', ' ',
      ' ',  '/',  '*',  ' ',  'p',  'r',  'e',  'f',  'i',  'x',  ' ',  '*',
      '/',  ' ',  's',  't',  'r',  'c',  'p',  'y',  '(',  'd',  'e',  's',
      't',  ',',  ' ',  '/',  '*',  ' ',  's',  'r',  'c',  ' ',  '*',  '/',
      ' ',  '"',  'g',  '"',  ' ',  '/',  '*',  ' ',  's',  'u',  'f',  'f',
      'i',  'x',  ' ',  '*',  '/',  ')',  ';',  '\n', '}',  '\n', '#',  'd',
      'e',  'f',  'i',  'n',  'e',  ' ',  'M',  'Y',  '_',  'C',  'O',  'P',
      'Y',  '_',  'M',  'A',  'C',  'R',  'O',  '(',  'a',  ',',  ' ',  'b',
      ')',  ' ',  's',  't',  'r',  'c',  'p',  'y',  '(',  'a',  ',',  ' ',
      'b',  ')',  '\n', 'v',  'o',  'i',  'd',  ' ',  'b',  'a',  'r',  '(',
      ')',  ' ',  '{',  ' ',  'M',  'Y',  '_',  'C',  'O',  'P',  'Y',  '_',
      'M',  'A',  'C',  'R',  'O',  '(',  'd',  'e',  's',  't',  ',',  ' ',
      '"',  'h',  '"',  ')',  ';',  ' ',  '}',  '\n', '\0'};
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                     &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_transform_safe_crt(tree, &config);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  printf("=== GENERATED CODE ===\n%s\n=== END ===\n", out);

  ASSERT(strstr(out, "strcpy_s(dest, sizeof(dest), \"abc\");") != NULL);
  ASSERT(strstr(out, "strncpy_s(dest, sizeof(dest), \"def\", _TRUNCATE);") !=
         NULL);
  ASSERT(strstr(out, "strcpy_s(dyn_buf, 256, \"test\");") != NULL);
  ASSERT(strstr(out, "strcpy_s(c_buf, (10) * (2), \"test\");") != NULL);
  ASSERT(strstr(out, "strcpy_s(enum_buf, MY_CONST, \"test\");") != NULL);
  ASSERT(strstr(out, "strcpy_s(mac_buf, MY_MACRO_CONST, \"test\");") != NULL);
  ASSERT(strstr(out, "strcpy_s(const_buf, MY_INT_CONST, \"test\");") != NULL);
  ASSERT(strstr(out, "strcpy_s(t_buf, sizeof(t_buf), \"test\");") != NULL);

  ASSERT(strstr(out, "strcpy_s(r_buf, 1024, \"test\");") != NULL);

  ASSERT(strstr(out, "strcpy_s(dest + 2, (sizeof(dest) - (2)), \"test\");") !=
         NULL);
  ASSERT(strstr(out, "strcpy_s(&dest[3], (sizeof(dest) - (3)), \"test\");") !=
         NULL);

  /* Test string conversions */
  ASSERT(strstr(out, "(_gcvt_s(dest, sizeof(dest), d, 5), dest)") != NULL);
  ASSERT(strstr(out, "(mbstowcs_s(NULL, wdest, (sizeof(wdest)) / "
                     "sizeof(wchar_t), \"abc\", 3), wdest)") != NULL);
  ASSERT(
      strstr(out,
             "(wcstombs_s(NULL, dest, (sizeof(dest)), L\"abc\", 3), dest)") !=
      NULL);
  ASSERT(strstr(out, "(wctomb_s(NULL, dest, sizeof(dest), L'a'), dest)") !=
         NULL);

  /* Test file system conversions */
  ASSERT(strstr(out, "_searchenv_s(\"x\", \"PATH\", dest, sizeof(dest))") !=
         NULL);
  ASSERT(strstr(out, "_wsearchenv_s(L\"x\", L\"PATH\", wdest, sizeof(wdest) "
                     "/ sizeof(wchar_t))") != NULL);
  ASSERT(strstr(out, "_wputenv_s(L\"A\", L\"B\")") != NULL);
  ASSERT(strstr(out, "qsort_s(dest, 10, 1, foo, NULL)") != NULL);
  ASSERT(strstr(out, "bsearch_s(\"a\", dest, 10, 1, foo, NULL)") != NULL);

  ASSERT(strstr(out, "sprintf_s(dest, sizeof(dest), \"%d\", 1);") != NULL);
  ASSERT(strstr(out, "fopen_s(&f") != NULL);
  ASSERT(strstr(out, "FILE *f2") != NULL);
  ASSERT(strstr(out, "fopen_s(&f2, \"b.txt\", \"r\")") != NULL);
  ASSERT(strstr(out, "strcat_s(dest, sizeof(dest), \"x\");") != NULL);
  ASSERT(strstr(out, "strncat_s(dest, sizeof(dest), \"y\", _TRUNCATE);") !=
         NULL);
  ASSERT(strstr(out, "memcpy_s(dest, sizeof(dest), \"z\", 1);") != NULL);
  ASSERT(strstr(out, "memmove_s(dest, sizeof(dest), \"w\", 1);") != NULL);

  /* Test nested evaluator */
  ASSERT(strstr(out, "strcpy_s(dest, sizeof(dest), strcpy_s(dest, "
                     "sizeof(dest), \"nested\"));") != NULL);

  /* Test Section 1.1 formatted I/O additions */
  ASSERT(strstr(out, "snprintf_s(dest, 10, _TRUNCATE, \"%d\", 2);") != NULL);
  ASSERT(strstr(out, "printf_s(\"%s\", dest);") != NULL);

  /* Test Section 1.2 scanf additions */
  ASSERT(strstr(out, "scanf_s(\"%s %d %c\", dest, (unsigned)sizeof(dest), "
                     "&idx, &ch, (unsigned)sizeof(ch));") != NULL);
  ASSERT(strstr(out, "fscanf_s(f, \"%10s\", dest, (unsigned)sizeof(dest));") !=
         NULL);
  ASSERT(strstr(out, "sscanf_s(dest, \"%[a-z] %% %*d %s\", dest2, "
                     "(unsigned)sizeof(dest2), "
                     "dest3, (unsigned)sizeof(dest3));") != NULL);

  /* Test _itoa, _mbscpy, freopen, _wfopen additions */
  ASSERT(strstr(out, "_itoa_s(idx, dest, sizeof(dest), 10);") != NULL);
  ASSERT(strstr(out, "_mbscpy_s(dest, sizeof(dest), \"abc\");") != NULL);
  ASSERT(strstr(out, "_mbsncpy_s(dest, sizeof(dest), \"abc\", _TRUNCATE);") !=
         NULL);
  ASSERT(strstr(out, "_mbscat_s(dest, sizeof(dest), \"abc\");") != NULL);
  ASSERT(strstr(out, "_mbsncat_s(dest, sizeof(dest), \"abc\", _TRUNCATE);") !=
         NULL);
  ASSERT(strstr(out, "_mbslwr_s(dest, sizeof(dest));") != NULL);
  ASSERT(strstr(out, "_mbsupr_s(dest, sizeof(dest));") != NULL);
  ASSERT(strstr(out, "_mbsset_s(dest, sizeof(dest), 'a');") != NULL);
  ASSERT(strstr(out, "_mbsnset_s(dest, sizeof(dest), 'a', _TRUNCATE);") !=
         NULL);
  ASSERT(strstr(out, "_wcslwr_s(wdest, sizeof(wdest));") != NULL);
  ASSERT(strstr(out, "_wcsupr_s(wdest, sizeof(wdest));") != NULL);
  ASSERT(strstr(out, "freopen_s") != NULL);
  ASSERT(strstr(out, "FILE *f3") != NULL);
  ASSERT(strstr(out, "_wfopen_s(&f3, L\"a.txt\", L\"r\")") != NULL);
  ASSERT(strstr(out, "FILE *f4") != NULL);
  ASSERT(strstr(out, "f4 = cdd_test_tmpfile_global()") != NULL);
  ASSERT(strstr(out, "_splitpath_s(dest, dest, sizeof(dest), dest, "
                     "sizeof(dest), dest, sizeof(dest), dest, sizeof(dest))") !=
         NULL);
  ASSERT(
      strstr(out, "_makepath_s(dest, sizeof(dest), dest, dest, dest, dest)") !=
      NULL);

  /* Test strlen addition */
  ASSERT(strstr(out, "strnlen_s(dest, sizeof(dest))") != NULL);
  ASSERT(strstr(out, "strtok_s(dest, \"a\", &__strtokctx)") != NULL);
  ASSERT(strstr(out, "wcstok_s(wdest, L\"a\", &__wcstokctx)") != NULL);
  ASSERT(strstr(out, "_mbstok_s(dest, \"a\", &__mbstokctx)") != NULL);
  ASSERT(
      strstr(out, "((_ecvt_s(__ecvtbuf, 128, d, 1, &idx, &idx), __ecvtbuf))") !=
      NULL);
  ASSERT(
      strstr(out, "((_fcvt_s(__fcvtbuf, 128, d, 1, &idx, &idx), __fcvtbuf))") !=
      NULL);
  ASSERT(strstr(out, "((strerror_s(__errbuf, 94, 1), __errbuf))") != NULL);
  ASSERT(strstr(out, "((_wcserror_s(__wcserrbuf, 94, 1), __wcserrbuf))") !=
         NULL);

  /* Test preservation of inline comments */
  ASSERT(strstr(out,
                "strcpy_s(dest, sizeof(dest), /* src */ \"g\" /* suffix */)") !=
         NULL);

  free(out);
  cdd_cst_tree_free(tree);

  {
    const char *code4 =
        "void edge4() { FILE *f; foo(f = fopen(\"a\", \"b\")); }";
    cdd_cst_tree_t *tree4 = NULL;
    ASSERT_EQ(
        0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code4),
                         &tree4));
    ASSERT_EQ(0, cdd_transform_safe_crt(tree4, &config));
    cdd_cst_tree_free(tree4);

    {
      const char *code5 = "void edge5() { char *u; (strcpy(u, \"a\")); }";
      cdd_cst_tree_t *tree5 = NULL;
      ASSERT_EQ(0, cdd_cst_parse(
                       az_span_create_from_str((char *)(size_t)(size_t)code5),
                       &tree5));
      ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_transform_safe_crt(tree5, &config));
      cdd_cst_tree_free(tree5);

      {
        const char *code6 =
            "void edge6() { FILE *f; (f = fopen(\"a\", \"b\")); }";
        cdd_cst_tree_t *tree6 = NULL;
        ASSERT_EQ(0, cdd_cst_parse(
                         az_span_create_from_str((char *)(size_t)(size_t)code6),
                         &tree6));
        ASSERT_EQ(0, cdd_transform_safe_crt(tree6, &config));
        cdd_cst_tree_free(tree6);

        {
          const char *code7 = "void edge7() { FILE *f;\n"
                              "#if defined(_MSC_VER)\n"
                              "  if(fopen_s(&f, \"a\", \"b\") != 0) f = NULL;\n"
                              "#else\n"
                              "  f = fopen(\"a\", \"b\");\n"
                              "#endif\n } ";
          cdd_cst_tree_t *tree7 = NULL;
          ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(
                                         (char *)(size_t)(size_t)code7),
                                     &tree7));
          ASSERT_EQ(0, cdd_transform_safe_crt(tree7, &config));
          cdd_cst_tree_free(tree7);

          {
            const char *code8 =
                "void edge8() { char *u; foo(strcpy(u, \"a\")); }";
            cdd_cst_tree_t *tree8 = NULL;
            ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(
                                           (char *)(size_t)(size_t)code8),
                                       &tree8));
            ASSERT_EQ(CDD_C_ERROR_PARSE,
                      cdd_transform_safe_crt(tree8, &config));
            cdd_cst_tree_free(tree8);

            {
              const char *code9 =
                  "void edge9() { char buf[256]; strcpy(malloc(10), \"a\"); "
                  "str"
                  "cpy(calloc(1, 10), \"a\"); strcpy(realloc(NULL, 10), "
                  "\"a\"); }";
              cdd_cst_tree_t *tree9 = NULL;
              ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(
                                             (char *)(size_t)(size_t)code9),
                                         &tree9));
              ASSERT_EQ(0, cdd_transform_safe_crt(tree9, &config));
              cdd_cst_tree_free(tree9);

              {
                const char *code10 = "void edge10() { scanf(\"%s\", NULL); }";
                cdd_cst_tree_t *tree10 = NULL;
                ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(
                                               (char *)(size_t)(size_t)code10),
                                           &tree10));
                ASSERT_EQ(0, cdd_transform_safe_crt(tree10, &config));
                cdd_cst_tree_free(tree10);

                {
                  const char *code11 = "void edge11() { scanf(\"%s\", 0); }";
                  cdd_cst_tree_t *tree11 = NULL;
                  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((
                                                 char *)(size_t)(size_t)code11),
                                             &tree11));
                  ASSERT_EQ(0, cdd_transform_safe_crt(tree11, &config));
                  cdd_cst_tree_free(tree11);

                  {
                    const char code12[] = {
                        118, 111, 105, 100, 32,  101, 100, 103, 101, 49,  50,
                        40,  41,  32,  123, 32,  99,  104, 97,  114, 32,  98,
                        117, 102, 91,  49,  48,  93,  59,  32,  115, 99,  97,
                        110, 102, 40,  34,  37,  115, 32,  37,  115, 32,  37,
                        115, 32,  37,  115, 32,  37,  115, 32,  37,  115, 32,
                        37,  115, 32,  37,  115, 32,  37,  115, 32,  37,  115,
                        32,  37,  115, 32,  37,  115, 32,  37,  115, 32,  37,
                        115, 32,  37,  115, 32,  37,  115, 32,  37,  115, 32,
                        37,  115, 32,  37,  115, 32,  37,  115, 32,  37,  115,
                        32,  37,  115, 32,  37,  115, 32,  37,  115, 32,  37,
                        115, 32,  37,  115, 32,  37,  115, 32,  37,  115, 32,
                        37,  115, 32,  37,  115, 32,  37,  115, 32,  37,  115,
                        32,  37,  115, 32,  37,  115, 34,  44,  32,  98,  117,
                        102, 44,  32,  98,  117, 102, 44,  32,  98,  117, 102,
                        44,  32,  98,  117, 102, 44,  32,  98,  117, 102, 44,
                        32,  98,  117, 102, 44,  32,  98,  117, 102, 44,  32,
                        98,  117, 102, 44,  32,  98,  117, 102, 44,  32,  98,
                        117, 102, 44,  32,  98,  117, 102, 44,  32,  98,  117,
                        102, 44,  32,  98,  117, 102, 44,  32,  98,  117, 102,
                        44,  32,  98,  117, 102, 44,  32,  98,  117, 102, 44,
                        32,  98,  117, 102, 44,  32,  98,  117, 102, 44,  32,
                        98,  117, 102, 44,  32,  98,  117, 102, 44,  32,  98,
                        117, 102, 44,  32,  98,  117, 102, 44,  32,  98,  117,
                        102, 44,  32,  98,  117, 102, 44,  32,  98,  117, 102,
                        44,  32,  98,  117, 102, 44,  32,  98,  117, 102, 44,
                        32,  98,  117, 102, 44,  32,  98,  117, 102, 44,  32,
                        98,  117, 102, 44,  32,  98,  117, 102, 44,  32,  98,
                        117, 102, 44,  32,  98,  117, 102, 44,  32,  98,  117,
                        102, 41,  59,  32,  125, 0};
                    cdd_cst_tree_t *tree12 = NULL;
                    ASSERT_EQ(0,
                              cdd_cst_parse(az_span_create_from_str(
                                                (char *)(size_t)(size_t)code12),
                                            &tree12));
                    ASSERT_EQ(0, cdd_transform_safe_crt(tree12, &config));
                    cdd_cst_tree_free(tree12);

                    g_fail_io_after = -1;
                    PASS();
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

TEST test_cdd_transform_safe_crt_extended_functions(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};
  char *out = NULL;

  const char code[] = {
      'v',  'o',  'i',  'd',  ' ',  'f',  'o',  'o',  '(',  'v',  'a',  '_',
      'l',  'i',  's',  't',  ' ',  'a',  'r',  'g',  's',  ')',  ' ',  '{',
      '\n', ' ',  ' ',  'c',  'h',  'a',  'r',  ' ',  'd',  'e',  's',  't',
      '[',  '1',  '0',  '0',  ']',  ';',  '\n', ' ',  ' ',  'w',  'c',  'h',
      'a',  'r',  '_',  't',  ' ',  'w',  'd',  'e',  's',  't',  '[',  '1',
      '0',  '0',  ']',  ';',  '\n', ' ',  ' ',  'i',  'n',  't',  ' ',  'i',
      'd',  'x',  ';',  '\n', ' ',  ' ',  'l',  'o',  'n',  'g',  ' ',  'l',
      'v',  'a',  'l',  ';',  '\n', ' ',  ' ',  'u',  'n',  's',  'i',  'g',
      'n',  'e',  'd',  ' ',  'l',  'o',  'n',  'g',  ' ',  'u',  'l',  'v',
      'a',  'l',  ';',  '\n', ' ',  ' ',  'l',  'o',  'n',  'g',  ' ',  'l',
      'o',  'n',  'g',  ' ',  'l',  'l',  'v',  'a',  'l',  ';',  '\n', ' ',
      ' ',  'u',  'n',  's',  'i',  'g',  'n',  'e',  'd',  ' ',  'l',  'o',
      'n',  'g',  ' ',  'l',  'o',  'n',  'g',  ' ',  'u',  'l',  'l',  'v',
      'a',  'l',  ';',  '\n', ' ',  ' ',  'v',  's',  'p',  'r',  'i',  'n',
      't',  'f',  '(',  'd',  'e',  's',  't',  ',',  ' ',  '"',  '%',  's',
      '"',  ',',  ' ',  'a',  'r',  'g',  's',  ')',  ';',  '\n', ' ',  ' ',
      'v',  's',  'n',  'p',  'r',  'i',  'n',  't',  'f',  '(',  'd',  'e',
      's',  't',  ',',  ' ',  '1',  '0',  ',',  ' ',  '"',  '%',  's',  '"',
      ',',  ' ',  'a',  'r',  'g',  's',  ')',  ';',  '\n', ' ',  ' ',  '_',
      'v',  's',  'n',  'p',  'r',  'i',  'n',  't',  'f',  '(',  'd',  'e',
      's',  't',  ',',  ' ',  '1',  '0',  ',',  ' ',  '"',  '%',  's',  '"',
      ',',  ' ',  'a',  'r',  'g',  's',  ')',  ';',  '\n', ' ',  ' ',  'w',
      'm',  'e',  'm',  'c',  'p',  'y',  '(',  'w',  'd',  'e',  's',  't',
      ',',  ' ',  'L',  '"',  'a',  'b',  'c',  '"',  ',',  ' ',  '3',  ')',
      ';',  '\n', ' ',  ' ',  'w',  'm',  'e',  'm',  'm',  'o',  'v',  'e',
      '(',  'w',  'd',  'e',  's',  't',  ',',  ' ',  'L',  '"',  'a',  'b',
      'c',  '"',  ',',  ' ',  '3',  ')',  ';',  '\n', ' ',  ' ',  'w',  'c',
      's',  'c',  'p',  'y',  '(',  'w',  'd',  'e',  's',  't',  ',',  ' ',
      'L',  '"',  'a',  'b',  'c',  '"',  ')',  ';',  '\n', ' ',  ' ',  'w',
      'c',  's',  'n',  'c',  'p',  'y',  '(',  'w',  'd',  'e',  's',  't',
      ',',  ' ',  'L',  '"',  'a',  'b',  'c',  '"',  ',',  ' ',  '2',  ')',
      ';',  '\n', ' ',  ' ',  'w',  'c',  's',  'c',  'a',  't',  '(',  'w',
      'd',  'e',  's',  't',  ',',  ' ',  'L',  '"',  'a',  'b',  'c',  '"',
      ')',  ';',  '\n', ' ',  ' ',  'w',  'c',  's',  'n',  'c',  'a',  't',
      '(',  'w',  'd',  'e',  's',  't',  ',',  ' ',  'L',  '"',  'a',  'b',
      'c',  '"',  ',',  ' ',  '2',  ')',  ';',  '\n', ' ',  ' ',  's',  'w',
      'p',  'r',  'i',  'n',  't',  'f',  '(',  'w',  'd',  'e',  's',  't',
      ',',  ' ',  '1',  '0',  '0',  ',',  ' ',  'L',  '"',  '%',  's',  '"',
      ',',  ' ',  'L',  '"',  't',  'e',  's',  't',  '"',  ')',  ';',  '\n',
      ' ',  ' ',  'v',  's',  'w',  'p',  'r',  'i',  'n',  't',  'f',  '(',
      'w',  'd',  'e',  's',  't',  ',',  ' ',  '1',  '0',  '0',  ',',  ' ',
      'L',  '"',  '%',  's',  '"',  ',',  ' ',  'a',  'r',  'g',  's',  ')',
      ';',  '\n', ' ',  ' ',  '_',  'l',  't',  'o',  'a',  '(',  'l',  'v',
      'a',  'l',  ',',  ' ',  'd',  'e',  's',  't',  ',',  ' ',  '1',  '0',
      ')',  ';',  '\n', ' ',  ' ',  '_',  'u',  'l',  't',  'o',  'a',  '(',
      'u',  'l',  'v',  'a',  'l',  ',',  ' ',  'd',  'e',  's',  't',  ',',
      ' ',  '1',  '0',  ')',  ';',  '\n', ' ',  ' ',  '_',  'i',  '6',  '4',
      't',  'o',  'a',  '(',  'l',  'l',  'v',  'a',  'l',  ',',  ' ',  'd',
      'e',  's',  't',  ',',  ' ',  '1',  '0',  ')',  ';',  '\n', ' ',  ' ',
      '_',  'u',  'i',  '6',  '4',  't',  'o',  'a',  '(',  'u',  'l',  'l',
      'v',  'a',  'l',  ',',  ' ',  'd',  'e',  's',  't',  ',',  ' ',  '1',
      '0',  ')',  ';',  '\n', ' ',  ' ',  '_',  'i',  't',  'o',  'w',  '(',
      'i',  'd',  'x',  ',',  ' ',  'w',  'd',  'e',  's',  't',  ',',  ' ',
      '1',  '0',  ')',  ';',  '\n', ' ',  ' ',  '_',  'l',  't',  'o',  'w',
      '(',  'l',  'v',  'a',  'l',  ',',  ' ',  'w',  'd',  'e',  's',  't',
      ',',  ' ',  '1',  '0',  ')',  ';',  '\n', ' ',  ' ',  '_',  'u',  'l',
      't',  'o',  'w',  '(',  'u',  'l',  'v',  'a',  'l',  ',',  ' ',  'w',
      'd',  'e',  's',  't',  ',',  ' ',  '1',  '0',  ')',  ';',  '\n', ' ',
      ' ',  '_',  's',  't',  'r',  'e',  'r',  'r',  'o',  'r',  '(',  'd',
      'e',  's',  't',  ')',  ';',  '\n', ' ',  ' ',  '_',  's',  't',  'r',
      'i',  'c',  'm',  'p',  '(',  '"',  'a',  '"',  ',',  ' ',  '"',  'b',
      '"',  ')',  ';',  '\n', ' ',  ' ',  '_',  's',  't',  'r',  'n',  'i',
      'c',  'm',  'p',  '(',  '"',  'a',  '"',  ',',  ' ',  '"',  'b',  '"',
      ',',  ' ',  '1',  ')',  ';',  '\n', ' ',  ' ',  '_',  's',  't',  'r',
      'l',  'w',  'r',  '(',  'd',  'e',  's',  't',  ')',  ';',  '\n', ' ',
      ' ',  '_',  's',  't',  'r',  'u',  'p',  'r',  '(',  'd',  'e',  's',
      't',  ')',  ';',  '\n', ' ',  ' ',  '_',  's',  't',  'r',  'n',  's',
      'e',  't',  '(',  'd',  'e',  's',  't',  ',',  ' ',  '\'', 'a',  '\'',
      ',',  ' ',  '2',  ')',  ';',  '\n', ' ',  ' ',  '_',  's',  't',  'r',
      's',  'e',  't',  '(',  'd',  'e',  's',  't',  ',',  ' ',  '\'', 'a',
      '\'', ')',  ';',  '\n', ' ',  ' ',  't',  'm',  'p',  'n',  'a',  'm',
      '(',  'd',  'e',  's',  't',  ')',  ';',  '\n', ' ',  ' ',  '_',  'w',
      's',  'p',  'l',  'i',  't',  'p',  'a',  't',  'h',  '(',  'w',  'd',
      'e',  's',  't',  ',',  ' ',  'w',  'd',  'e',  's',  't',  ',',  ' ',
      'w',  'd',  'e',  's',  't',  ',',  ' ',  'w',  'd',  'e',  's',  't',
      ',',  ' ',  'w',  'd',  'e',  's',  't',  ')',  ';',  '\n', ' ',  ' ',
      '_',  'w',  'm',  'a',  'k',  'e',  'p',  'a',  't',  'h',  '(',  'w',
      'd',  'e',  's',  't',  ',',  ' ',  'w',  'd',  'e',  's',  't',  ',',
      ' ',  'w',  'd',  'e',  's',  't',  ',',  ' ',  'w',  'd',  'e',  's',
      't',  ',',  ' ',  'w',  'd',  'e',  's',  't',  ')',  ';',  '\n', '}',
      '\n', '\0'};

  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  ASSERT_EQ(0, cdd_transform_safe_crt(tree, &config));

  cdd_cst_emit(tree, &out);
  cdd_cst_tree_free(tree);

  printf("EXT OUT:\n%s\n", out);
  printf("\n\nEXT OUT:\n%s\n\n", out);
  ASSERT(strstr(out, "vsprintf_s(dest, sizeof(dest), \"%s\", args);") != NULL);
  ASSERT(strstr(out, "vsnprintf_s(dest, 10, _TRUNCATE, \"%s\", args);") !=
         NULL);
  ASSERT(strstr(out, "_vsnprintf_s(dest, 10, _TRUNCATE, \"%s\", args);") !=
         NULL);
  ASSERT(strstr(out, "wmemcpy_s(wdest, sizeof(wdest), L\"abc\", 3);") != NULL);
  ASSERT(strstr(out, "wmemmove_s(wdest, sizeof(wdest), L\"abc\", 3);") != NULL);
  ASSERT(strstr(out, "wcscpy_s(wdest, sizeof(wdest), L\"abc\");") != NULL);
  ASSERT(strstr(out, "wcsncpy_s(wdest, sizeof(wdest), L\"abc\", _TRUNCATE);") !=
         NULL);
  ASSERT(strstr(out, "wcscat_s(wdest, sizeof(wdest), L\"abc\");") != NULL);
  ASSERT(strstr(out, "wcsncat_s(wdest, sizeof(wdest), L\"abc\", _TRUNCATE);") !=
         NULL);
  ASSERT(strstr(out,
                "swprintf_s(wdest, sizeof(wdest), 100, L\"%s\", L\"test\");") !=
         NULL);
  ASSERT(
      strstr(out, "vswprintf_s(wdest, sizeof(wdest), 100, L\"%s\", args);") !=
      NULL);
  ASSERT(strstr(out, "_ltoa_s(lval, dest, sizeof(dest), 10);") != NULL);
  ASSERT(strstr(out, "_ultoa_s(ulval, dest, sizeof(dest), 10);") != NULL);
  ASSERT(strstr(out, "_i64toa_s(llval, dest, sizeof(dest), 10);") != NULL);
  ASSERT(strstr(out, "_ui64toa_s(ullval, dest, sizeof(dest), 10);") != NULL);
  ASSERT(strstr(out, "_itow_s(idx, wdest, sizeof(wdest), 10);") != NULL);
  ASSERT(strstr(out, "_ltow_s(lval, wdest, sizeof(wdest), 10);") != NULL);
  ASSERT(strstr(out, "_ultow_s(ulval, wdest, sizeof(wdest), 10);") != NULL);
  ASSERT(strstr(out, "((_strerror_s(__errbuf, 94, dest), __errbuf))") != NULL);
  ASSERT(strstr(out, "_stricmp(\"a\", \"b\")") != NULL);
  ASSERT(strstr(out, "_strlwr_s(dest, sizeof(dest));") != NULL);
  ASSERT(strstr(out, "_strupr_s(dest, sizeof(dest));") != NULL);
  ASSERT(strstr(out, "_strnset_s(dest, sizeof(dest), 'a', 2);") != NULL);
  ASSERT(strstr(out, "_strset_s(dest, sizeof(dest), 'a');") != NULL);
  ASSERT(strstr(out, "tmpnam_s(dest, sizeof(dest));") != NULL);
  ASSERT(
      strstr(out,
             "_wsplitpath_s(wdest, wdest, sizeof(wdest), wdest, sizeof(wdest), "
             "wdest, sizeof(wdest), wdest, sizeof(wdest));") != NULL);
  ASSERT(
      strstr(
          out,
          "_wmakepath_s(wdest, sizeof(wdest), wdest, wdest, wdest, wdest);") !=
      NULL);

  free(out);
  PASS();
}

TEST test_cdd_transform_safe_crt_edge_cases(void) {
  cdd_cst_tree_t *tree = NULL;

  const char code[] = {
      'v',  'o',  'i',  'd',  ' ',  'e', 'd',  'g',  'e',  '_',  'c',  'a',
      's',  'e',  's',  '(',  ')',  ' ', '{',  '\n', ' ',  ' ',  'c',  'h',
      'a',  'r',  ' ',  'b',  'u',  'f', '[',  '2',  '5',  '6',  ']',  ';',
      '\n', '\n', '#',  'i',  'f',  ' ', 'd',  'e',  'f',  'i',  'n',  'e',
      'd',  '(',  '_',  'M',  'S',  'C', '_',  'V',  'E',  'R',  ')',  '\n',
      ' ',  ' ',  'i',  'n',  't',  ' ', 'd',  'u',  'm',  'm',  'y',  ' ',
      '=',  ' ',  '1',  ';',  '\n', ' ', ' ',  'i',  'f',  ' ',  '(',  'd',
      'u',  'm',  'm',  'y',  ')',  ' ', 's',  't',  'r',  'c',  'p',  'y',
      '(',  'b',  'u',  'f',  ',',  ' ', '"',  'a',  'b',  'c',  '"',  ')',
      ';',  ' ',  'e',  'l',  's',  'e', ' ',  's',  't',  'r',  'c',  'p',
      'y',  '(',  'b',  'u',  'f',  ',', ' ',  '"',  'd',  'e',  'f',  '"',
      ')',  ';',  '\n', ' ',  ' ',  's', 'w',  'i',  't',  'c',  'h',  ' ',
      '(',  'd',  'u',  'm',  'm',  'y', ')',  ' ',  '{',  ' ',  'c',  'a',
      's',  'e',  ' ',  '1',  ':',  ' ', 's',  't',  'r',  'c',  'p',  'y',
      '(',  'b',  'u',  'f',  ',',  ' ', '"',  'a',  '"',  ')',  ';',  ' ',
      '}',  '\n', ' ',  ' ',  'f',  'o', 'r',  ' ',  '(',  ';',  ' ',  'd',
      'u',  'm',  'm',  'y',  ';',  ' ', 'd',  'u',  'm',  'm',  'y',  '-',
      '-',  ')',  ' ',  's',  't',  'r', 'c',  'p',  'y',  '(',  'b',  'u',
      'f',  ',',  ' ',  '"',  'x',  '"', ')',  ';',  '\n', ' ',  ' ',  'w',
      'h',  'i',  'l',  'e',  ' ',  '(', 'd',  'u',  'm',  'm',  'y',  ')',
      ' ',  's',  't',  'r',  'c',  'p', 'y',  '(',  'b',  'u',  'f',  ',',
      ' ',  '"',  'y',  '"',  ')',  ';', '\n', ' ',  ' ',  'd',  'o',  ' ',
      's',  't',  'r',  'c',  'p',  'y', '(',  'b',  'u',  'f',  ',',  ' ',
      '"',  'z',  '"',  ')',  ';',  ' ', 'w',  'h',  'i',  'l',  'e',  ' ',
      '(',  '0',  ')',  ';',  '\n', ' ', ' ',  '{',  ' ',  's',  't',  'r',
      'c',  'p',  'y',  '(',  'b',  'u', 'f',  ',',  ' ',  '"',  'f',  'o',
      'o',  '"',  ')',  ';',  ' ',  '}', '\n', ' ',  ' ',  'f',  's',  'c',
      'a',  'n',  'f',  '(',  's',  't', 'd',  'o',  'u',  't',  ',',  ' ',
      '"',  '1',  '2',  '3',  '"',  ',', ' ',  '&',  'd',  'u',  'm',  'm',
      'y',  ',',  ' ',  '&',  'd',  'u', 'm',  'm',  'y',  ')',  ';',  '\n',
      ' ',  ' ',  's',  'c',  'a',  'n', 'f',  '(',  '"',  '%',  '2',  '*',
      's',  '"',  ',',  ' ',  '&',  'd', 'u',  'm',  'm',  'y',  ')',  ';',
      '\n', ' ',  ' ',  's',  'c',  'a', 'n',  'f',  '(',  '"',  '%',  '*',
      's',  '"',  ',',  ' ',  '&',  'd', 'u',  'm',  'm',  'y',  ')',  ';',
      '\n', ' ',  ' ',  's',  'c',  'a', 'n',  'f',  '(',  '"',  '%',  '%',
      '"',  ',',  ' ',  '&',  'd',  'u', 'm',  'm',  'y',  ')',  ';',  '\n',
      ' ',  ' ',  's',  'c',  'a',  'n', 'f',  '(',  '"',  '%',  '*',  '[',
      '^',  'a',  '-',  'z',  ']',  '"', ',',  ' ',  '&',  'd',  'u',  'm',
      'm',  'y',  ')',  ';',  '\n', ' ', ' ',  's',  'c',  'a',  'n',  'f',
      '(',  '"',  '%',  '*',  '2',  '[', '^',  'a',  '-',  'z',  ']',  '"',
      ',',  ' ',  '&',  'd',  'u',  'm', 'm',  'y',  ')',  ';',  '\n', ' ',
      ' ',  's',  'c',  'a',  'n',  'f', '(',  '"',  '%',  '*',  '[',  ']',
      ']',  '"',  ',',  ' ',  '&',  'd', 'u',  'm',  'm',  'y',  ')',  ';',
      '\n', ' ',  ' ',  's',  'c',  'a', 'n',  'f',  '(',  '"',  '%',  '*',
      '2',  '[',  ']',  ']',  '"',  ',', ' ',  '&',  'd',  'u',  'm',  'm',
      'y',  ')',  ';',  '\n', ' ',  ' ', 's',  'c',  'a',  'n',  'f',  '(',
      '"',  '%',  '[',  ']',  ']',  '"', ',',  ' ',  'b',  'u',  'f',  ')',
      ';',  '\n', ' ',  ' ',  's',  'c', 'a',  'n',  'f',  '(',  '"',  '%',
      '[',  '^',  ']',  ']',  '"',  ',', ' ',  'b',  'u',  'f',  ')',  ';',
      '\n', ' ',  ' ',  's',  'c',  'a', 'n',  'f',  '(',  '"',  '%',  '2',
      's',  '"',  ',',  ' ',  'b',  'u', 'f',  ')',  ';',  '\n', ' ',  ' ',
      'f',  's',  'c',  'a',  'n',  'f', '(',  's',  't',  'd',  'o',  'u',
      't',  ',',  ' ',  '"',  '1',  '2', '3',  '"',  ',',  ' ',  '&',  'd',
      'u',  'm',  'm',  'y',  ',',  ' ', '&',  'd',  'u',  'm',  'm',  'y',
      ',',  ' ',  '&',  'd',  'u',  'm', 'm',  'y',  ',',  ' ',  '&',  'd',
      'u',  'm',  'm',  'y',  ',',  ' ', '&',  'd',  'u',  'm',  'm',  'y',
      ',',  ' ',  '&',  'd',  'u',  'm', 'm',  'y',  ',',  ' ',  '&',  'd',
      'u',  'm',  'm',  'y',  ',',  ' ', '&',  'd',  'u',  'm',  'm',  'y',
      ',',  ' ',  '&',  'd',  'u',  'm', 'm',  'y',  ',',  ' ',  '&',  'd',
      'u',  'm',  'm',  'y',  ',',  ' ', '&',  'd',  'u',  'm',  'm',  'y',
      ',',  ' ',  '&',  'd',  'u',  'm', 'm',  'y',  ',',  ' ',  '&',  'd',
      'u',  'm',  'm',  'y',  ',',  ' ', '&',  'd',  'u',  'm',  'm',  'y',
      ',',  ' ',  '&',  'd',  'u',  'm', 'm',  'y',  ',',  ' ',  '&',  'd',
      'u',  'm',  'm',  'y',  ',',  ' ', '&',  'd',  'u',  'm',  'm',  'y',
      ',',  ' ',  '&',  'd',  'u',  'm', 'm',  'y',  ')',  ';',  '\n', ' ',
      ' ',  'f',  's',  'c',  'a',  'n', 'f',  '(',  's',  't',  'd',  'o',
      'u',  't',  ',',  ' ',  '"',  '1', '2',  '3',  '"',  ',',  ' ',  '&',
      'd',  'u',  'm',  'm',  'y',  ')', ';',  '\n', ' ',  ' ',  's',  'c',
      'a',  'n',  'f',  '(',  ')',  ';', '\n', ' ',  ' ',  'f',  's',  'c',
      'a',  'n',  'f',  '(',  's',  't', 'd',  'o',  'u',  't',  ')',  ';',
      '\n', ' ',  ' ',  'p',  'r',  'i', 'n',  't',  'f',  '(',  ')',  ';',
      '\n', ' ',  ' ',  's',  'c',  'a', 'n',  'f',  '(',  '"',  '1',  '2',
      '3',  '"',  ',',  ' ',  '&',  'd', 'u',  'm',  'm',  'y',  ',',  ' ',
      '&',  'd',  'u',  'm',  'm',  'y', ')',  ';',  '\n', ' ',  ' ',  'p',
      'r',  'i',  'n',  't',  'f',  '(', '"',  'h',  'e',  'l',  'l',  'o',
      '"',  ')',  ';',  '\n', '"',  ' ', '/',  '*',  ' ',  'n',  'o',  't',
      ' ',  't',  'r',  'a',  'n',  's', 'f',  'o',  'r',  'm',  'e',  'd',
      ' ',  'w',  'i',  't',  'h',  'o', 'u',  't',  ' ',  'a',  'r',  'g',
      's',  ' ',  '*',  '/',  '\n', ' ', ' ',  ' ',  ' ',  ' ',  ' ',  '"',
      '#',  'e',  'n',  'd',  'i',  'f', '\n', '}',  '\n', '\0'};
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_transform_safe_crt(NULL, &config));

  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  ASSERT_EQ(0, cdd_transform_safe_crt(tree, &config));

  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

#ifdef CDD_BUILD_TESTS
/* extern C_CDD_EXPORT int g_safe_crt_malloc_fail; (moved to global) */
/* extern C_CDD_EXPORT int g_cdd_cst_alloc_node_fail;
extern C_CDD_EXPORT int g_cdd_query_err_fail; (moved to global) */
#endif

TEST test_cdd_transform_safe_crt_oom(void) {
#ifdef CDD_BUILD_TESTS
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_tree_t *tree2 = NULL;

  const char code[] = {
      'v',  'o', 'i', 'd',  ' ', 'f',  '(', ')', ' ',  '{', ' ', 'c',  'h',
      'a',  'r', ' ', 'b',  'u', 'f',  '[', '1', '0',  ']', ';', ' ',  'c',
      'h',  'a', 'r', ' ',  '*', 'p',  ';', ' ', 'd',  'o', 'u', 'b',  'l',
      'e',  ' ', 'd', ';',  ' ', 'w',  'c', 'h', 'a',  'r', '_', 't',  ' ',
      'w',  'b', 'u', 'f',  '[', '1',  '0', ']', ';',  ' ', 'p', ' ',  '=',
      ' ',  'i', 'f', ' ',  '(', '1',  ')', ' ', 's',  't', 'r', 't',  'o',
      'k',  '(', 'b', 'u',  'f', ',',  ' ', '"', 'a',  '"', ')', ';',  ' ',
      'i',  'f', ' ', '(',  '1', ')',  ' ', 'p', ' ',  '=', ' ', 'w',  'c',
      's',  't', 'o', 'k',  '(', 'w',  'b', 'u', 'f',  ',', ' ', 'L',  '"',
      'a',  '"', ')', ';',  ' ', 'i',  'f', ' ', '(',  '1', ')', ' ',  '_',
      'm',  'b', 's', 't',  'o', 'k',  '(', 'b', 'u',  'f', ',', ' ',  '"',
      'a',  '"', ')', ';',  ' ', 'i',  'f', ' ', '(',  '1', ')', ' ',  's',
      't',  'r', 'e', 'r',  'r', 'o',  'r', '(', '1',  ')', ';', ' ',  'i',
      'f',  ' ', '(', '1',  ')', ' ',  '_', 'w', 'c',  's', 'e', 'r',  'r',
      'o',  'r', '(', '1',  ')', ';',  ' ', 'i', 'f',  ' ', '(', '1',  ')',
      ' ',  '_', 'e', 'c',  'v', 't',  '(', 'd', ',',  ' ', '1', ',',  ' ',
      '0',  ',', ' ', '0',  ')', ';',  ' ', 'i', 'f',  ' ', '(', '1',  ')',
      ' ',  '_', 'f', 'c',  'v', 't',  '(', 'd', ',',  ' ', '1', ',',  ' ',
      '0',  ',', ' ', '0',  ')', ';',  ' ', 'i', 'f',  ' ', '(', '1',  ')',
      ' ',  'c', 't', 'i',  'm', 'e',  '(', 'N', 'U',  'L', 'L', ')',  ';',
      ' ',  'i', 'f', ' ',  '(', '1',  ')', ' ', 'g',  'e', 't', 'e',  'n',
      'v',  '(', '"', 'A',  '"', ')',  ';', ' ', 'F',  'I', 'L', 'E',  ' ',
      '*',  'f', ';', '\n', '#', 'i',  'f', ' ', 'd',  'e', 'f', 'i',  'n',
      'e',  'd', ' ', '(',  '_', 'M',  'S', 'C', '_',  'V', 'E', 'R',  ')',
      '\n', ' ', ' ', 'i',  'f', ' ',  '(', 'f', 'o',  'p', 'e', 'n',  '_',
      's',  '(', '&', 'f',  ',', ' ',  '"', 'A', '"',  ',', ' ', '"',  'B',
      '"',  ')', ' ', '!',  '=', ' ',  '0', ')', ' ',  'f', ' ', '=',  ' ',
      'N',  'U', 'L', 'L',  ';', '\n', '#', 'e', 'l',  's', 'e', '\n', ' ',
      ' ',  'f', ' ', '=',  ' ', 'f',  'o', 'p', 'e',  'n', '(', '"',  'A',
      '"',  ',', ' ', '"',  'B', '"',  ')', ';', '\n', '#', 'e', 'n',  'd',
      'i',  'f', ' ', '\n', 'i', 'f',  ' ', '(', '1',  ')', ' ', '_',  'w',
      'g',  'e', 't', 'e',  'n', 'v',  '(', 'L', '"',  'A', '"', ')',  ';',
      ' ',  's', 't', 'r',  'c', 'p',  'y', '(', 'b',  'u', 'f', ',',  ' ',
      '"',  'a', 'b', 'c',  '"', ')',  ';', ' ', '}',  '\0'};
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));

  {
    int i;
    for (i = 1; i <= 200; i++) {
      cdd_cst_tree_free(tree);
      tree = NULL;
      ASSERT_EQ(
          0, cdd_cst_parse(
                 az_span_create_from_str((char *)(size_t)(size_t)code), &tree));
      g_safe_crt_malloc_fail = i;
      cdd_transform_safe_crt(tree, &config);
      g_safe_crt_malloc_fail = 0;
      cdd_cst_tree_free(tree);
      tree = NULL;
    }
    for (i = 1; i <= 20; i++) {
      cdd_cst_tree_free(tree);
      tree = NULL;
      ASSERT_EQ(
          0, cdd_cst_parse(
                 az_span_create_from_str((char *)(size_t)(size_t)code), &tree));
      g_cdd_cst_alloc_node_fail = i;
      cdd_transform_safe_crt(tree, &config);
      g_cdd_cst_alloc_node_fail = 0;
      cdd_cst_tree_free(tree);
      tree = NULL;
    }
  }

  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  g_cdd_cst_alloc_node_fail = 3;
  cdd_transform_safe_crt(tree, &config);
  g_cdd_cst_alloc_node_fail = 0;
  cdd_cst_tree_free(tree);
  tree = NULL;

  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  g_cdd_query_err_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_transform_safe_crt(tree, &config));
  g_cdd_query_err_fail = 0;
  cdd_cst_tree_free(tree);
  tree = NULL;

  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  g_safe_crt_malloc_fail = 2;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 0;
  cdd_cst_tree_free(tree);
  tree = NULL;
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  g_safe_crt_malloc_fail = 15;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 14;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 13;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 12;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 11;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 10;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 9;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 8;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 7;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 0;
  cdd_cst_tree_free(tree);
  tree = NULL;

  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  g_safe_crt_malloc_fail = 3;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 0;
  cdd_cst_tree_free(tree);
  tree = NULL;

  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  g_safe_crt_malloc_fail = 4;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 0;
  cdd_cst_tree_free(tree);
  tree = NULL;

  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  g_safe_crt_malloc_fail = 5;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 0;
  cdd_cst_tree_free(tree);
  tree = NULL;

  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree2));
  cdd_transform_safe_crt(tree2, &config);
  cdd_cst_tree_free(tree2);

  {
    const char *code9 =
        "void edge9() { char buf[256]; strcpy(malloc(10), \"a\"); "
        "str"
        "cpy(calloc(1, 10), \"a\"); strcpy(realloc(NULL, 10), \"a\"); }";
    cdd_cst_tree_t *tree9 = NULL;
    ASSERT_EQ(
        0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code9),
                         &tree9));
    ASSERT_EQ(0, cdd_transform_safe_crt(tree9, &config));
    cdd_cst_tree_free(tree9);

    {
      const char *code10 = "void edge10() { scanf(\"%s\", NULL); }";
      cdd_cst_tree_t *tree10 = NULL;
      ASSERT_EQ(0, cdd_cst_parse(
                       az_span_create_from_str((char *)(size_t)(size_t)code10),
                       &tree10));
      ASSERT_EQ(0, cdd_transform_safe_crt(tree10, &config));
      cdd_cst_tree_free(tree10);

      {
        const char *code11 = "void edge11() { scanf(\"%s\", 0); }";
        cdd_cst_tree_t *tree11 = NULL;
        ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(
                                       (char *)(size_t)(size_t)code11),
                                   &tree11));
        ASSERT_EQ(0, cdd_transform_safe_crt(tree11, &config));
        cdd_cst_tree_free(tree11);

        {
          const char code12[] = {
              118, 111, 105, 100, 32,  101, 100, 103, 101, 49,  50,  40,  41,
              32,  123, 32,  99,  104, 97,  114, 32,  98,  117, 102, 91,  49,
              48,  93,  59,  32,  115, 99,  97,  110, 102, 40,  34,  37,  115,
              32,  37,  115, 32,  37,  115, 32,  37,  115, 32,  37,  115, 32,
              37,  115, 32,  37,  115, 32,  37,  115, 32,  37,  115, 32,  37,
              115, 32,  37,  115, 32,  37,  115, 32,  37,  115, 32,  37,  115,
              32,  37,  115, 32,  37,  115, 32,  37,  115, 32,  37,  115, 32,
              37,  115, 32,  37,  115, 32,  37,  115, 32,  37,  115, 32,  37,
              115, 32,  37,  115, 32,  37,  115, 32,  37,  115, 32,  37,  115,
              32,  37,  115, 32,  37,  115, 32,  37,  115, 32,  37,  115, 32,
              37,  115, 32,  37,  115, 32,  37,  115, 34,  44,  32,  98,  117,
              102, 44,  32,  98,  117, 102, 44,  32,  98,  117, 102, 44,  32,
              98,  117, 102, 44,  32,  98,  117, 102, 44,  32,  98,  117, 102,
              44,  32,  98,  117, 102, 44,  32,  98,  117, 102, 44,  32,  98,
              117, 102, 44,  32,  98,  117, 102, 44,  32,  98,  117, 102, 44,
              32,  98,  117, 102, 44,  32,  98,  117, 102, 44,  32,  98,  117,
              102, 44,  32,  98,  117, 102, 44,  32,  98,  117, 102, 44,  32,
              98,  117, 102, 44,  32,  98,  117, 102, 44,  32,  98,  117, 102,
              44,  32,  98,  117, 102, 44,  32,  98,  117, 102, 44,  32,  98,
              117, 102, 44,  32,  98,  117, 102, 44,  32,  98,  117, 102, 44,
              32,  98,  117, 102, 44,  32,  98,  117, 102, 44,  32,  98,  117,
              102, 44,  32,  98,  117, 102, 44,  32,  98,  117, 102, 44,  32,
              98,  117, 102, 44,  32,  98,  117, 102, 44,  32,  98,  117, 102,
              44,  32,  98,  117, 102, 44,  32,  98,  117, 102, 41,  59,  32,
              125, 0};
          cdd_cst_tree_t *tree12 = NULL;
          ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(
                                         (char *)(size_t)(size_t)code12),
                                     &tree12));
          ASSERT_EQ(0, cdd_transform_safe_crt(tree12, &config));
          cdd_cst_tree_free(tree12);

#endif
          g_fail_io_after = -1;
          PASS();
        }
      }
    }
  }
}

TEST test_cdd_transform_safe_crt_needs_buffers(void) {
  cdd_transform_config_t config;
  cdd_cst_tree_t *tree = NULL;
  const char *snippets[] = {
      "void test_err(void) {\n  char *msg = strerror(1);\n}\n",
      "void test_wcserr(void) {\n  wchar_t *msg = _wcserror(1);\n}\n",
      "void test_strtok(void) {\n  char buf[32];\n  char *tok = strtok(buf, "
      "\",\");\n}\n",
      "void test_wcstok(void) {\n  wchar_t buf[32];\n  wchar_t *tok = "
      "wcstok(buf, L\",\");\n}\n",
      "void test_mbstok(void) {\n  unsigned char buf[32];\n  unsigned char "
      "*tok = _mbstok(buf, (unsigned char *)\",\");\n}\n",
      "void test_ecvt(void) {\n  int dec, sign;\n  char *res = _ecvt(3.14, 2, "
      "&dec, &sign);\n}\n",
      "void test_fcvt(void) {\n  int dec, sign;\n  char *res = _fcvt(3.14, 2, "
      "&dec, &sign);\n}\n",
      "void test_getenv(void) {\n  char *val = getenv(\"PATH\");\n}\n",
      "void test_wgetenv(void) {\n  wchar_t *val = _wgetenv(L\"PATH\");\n}\n",
      "void test_putenv(void) {\n  _putenv(\"KEY=VAL\");\n}\n",
      "void test_wputenv(void) {\n  _wputenv(L\"KEY=VAL\");\n}\n",
      "void test_qsort(void) {\n  int arr[4];\n  qsort(arr, 4, sizeof(int), "
      "NULL);\n}\n"};
  size_t i;
  memset(&config, 0, sizeof(config));

  for (i = 0; i < sizeof(snippets) / sizeof(snippets[0]); ++i) {
    cdd_c_error_t rc_t;
    ASSERT_EQ(
        0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)snippets[i]),
                         &tree));
    rc_t = cdd_transform_safe_crt(tree, &config);
    if (rc_t != 0) {
      fprintf(stderr, "Snippet %d failed with rc=%d: %s\n", (int)i, (int)rc_t,
              snippets[i]);
    }
    ASSERT_EQ(0, rc_t);
    cdd_cst_tree_free(tree);
    tree = NULL;
  }
  PASS();
}

TEST test_cdd_transform_safe_crt_more_cases(void) {
  cdd_transform_config_t config;
  cdd_cst_tree_t *tree = NULL;
  const char *snippets[] = {
      "void test_bare_fopen(void) {\n  fopen(\"a\", \"r\");\n}\n",
      "void test_splitpath_null(void) {\n  char drive[3];\n  char dir[256];\n  "
      "char fname[256];\n  char ext[256];\n  _splitpath(\"path\", drive, dir, "
      "NULL, 0);\n}\n",
      "void test_putenv_var(void) {\n  char *var = \"KEY=VAL\";\n  "
      "_putenv(var);\n}\n",
      "void test_decl_fopen(void) {\n  FILE *f = fopen(\"a\", \"r\");\n}\n",
      "void test_decl_fopen_ptr(void) {\n  FILE **fp;\n  *fp = fopen(\"a\", "
      "\"r\");\n}\n",
      "void test_struct_fopen(void) {\n  struct S { FILE *f; } s;\n  s.f = "
      "fopen(\"a\", \"r\");\n}\n",
      "void test_scanf_brackets(void) {\n  char buf[32];\n  sscanf(\"str\", "
      "\"%*10[a-z] %*s %10[^]a] %10[]a] %10[a-z] %*c %10C %10S\", buf, buf, "
      "buf, buf, buf);\n}\n",
      "void test_inferred_trivia(void) {\n  char dest[10];\n  /*c1*/ "
      "strcpy(/*c2*/ dest /*c3*/, \"test\");\n}\n",
      "void test_equal_trivia(void) {\n  FILE *f;\n  f /*c1*/ = /*c2*/ "
      "fopen(\"a\", \"r\");\n}\n",
      "void test_already_safe(void) {\n  /*CDD_SAFE_CRT*/ strcpy(dest, "
      "\"a\");\n}\n",
      "void test_scanf_no_args(void) {\n  scanf();\n}\n",
      "void test_sscanf_no_args(void) {\n  sscanf(s);\n}\n",

      "void test_assign_dot_fopen(void) {\n  struct S { FILE *f; } s;\n  s.f = "
      "fopen(\"a\", \"r\");\n}\n",
      "void test_assign_star_fopen(void) {\n  FILE *f; FILE **fp = &f;\n  *fp "
      "= fopen(\"a\", \"r\");\n}\n",

      "void test_semicolon_in_call(void) {\n  foo(a; b);\n}\n",
      "void test_call_comma_stop(void) {\n  (a, b);\n}\n",

      "void test_synthesized(void) {\n\n#if defined(_MSC_VER)\n  int x = "
      "1;\n#endif\n}\n",

      "void test_bare_w_fopen(void) {\n  _wfopen(\"a\", \"r\");\n}\n",
      "void test_bare_freopen(void) {\n  FILE *f = NULL;\n  freopen(\"a\", "
      "\"r\", f);\n}\n",
      "void test_bare_tmpfile(void) {\n  tmpfile();\n}\n",
      "void test_arrow_fopen(void) {\n  struct S { FILE *f; } *s;\n  s->f = "
      "fopen(\"a\", \"r\");\n}\n",
      "void test_bracket_fopen(void) {\n  FILE *arr[2];\n  arr[0] = "
      "fopen(\"a\", \"r\");\n}\n",
      "void test_arrow_tmpfile(void) {\n  struct S { FILE *f; } *s;\n  s->f = "
      "tmpfile();\n}\n",
      "void test_bracket_tmpfile(void) {\n  FILE *arr[2];\n  arr[0] = "
      "tmpfile();\n}\n",
      "void test_nospace_fopen(void) {\n  FILE *f; f=fopen(\"a\",\"r\");\n}\n",
      "void test_zero_args(void) {\n"
      "  strcpy();\n  strncpy();\n  snprintf();\n  printf();\n  memcpy();\n"
      "  _itoa();\n  gets();\n  _splitpath();\n  _makepath();\n  _gcvt();\n"
      "  mbstowcs();\n  wctomb();\n  _searchenv();\n  getenv();\n  _putenv();\n"
      "  qsort();\n  strtok();\n}\n",
      "void test_extra_funcs1(void) {\n"
      "  char buf[32]; wchar_t wbuf[32]; va_list va; FILE *f = NULL;\n"
      "  _snprintf(buf, 32, \"%s\", \"a\");\n  _vsnprintf(buf, 32, \"%s\", "
      "va);\n"
      "  vprintf(\"%s\", va);\n  vfprintf(f, \"%s\", va);\n"
      "  vfscanf(f, \"%s\", buf);\n  vsscanf(\"s\", \"%s\", buf);\n"
      "  wcscpy(wbuf, L\"a\");\n  wcscat(wbuf, L\"b\");\n"
      "  swprintf(wbuf, 32, L\"%s\", L\"a\");\n  vswprintf(wbuf, 32, L\"%s\", "
      "va);\n"
      "}\n",
      "void test_sscanf_variants(void) {\n"
      "  char s2[16]; char s3[16]; char *fmt = \"%s\"; int d;\n"
      "  sscanf(\"str\", \"%d %% %s\", &d, s2);\n"
      "  sscanf(\"str\", \"%*d %s\", s2);\n"
      "  sscanf(\"str\", \"%*\", s2);\n"
      "  sscanf(\"str\", \"%\", s2);\n"
      "  sscanf(\"str\", fmt, s2, s3, &d);\n"
      "}\n",
      "void test_extra_funcs2(void) {\n"
      "  char buf[32]; wchar_t wbuf[32]; unsigned char mbuf[32]; char s[5];\n"
      "  _mbscpy(mbuf, \"a\");\n  _mbscat(mbuf, \"b\");\n"
      "  _strnset(buf, 'a', 5);\n  _strset(buf, 'a');\n  _mbsset(mbuf, 'a');\n"
      "  _strlwr(buf);\n  _strupr(buf);\n  _mbslwr(mbuf);\n  _mbsupr(mbuf);\n"
      "  _wcslwr(wbuf);\n  _wcsupr(wbuf);\n  tmpnam(buf);\n  strlen(s);\n"
      "  wcsncpy(wbuf, L\"a\", 5);\n  wcsncat(wbuf, L\"b\", 5);\n"
      "}\n",
      "void test_extra_funcs3(void) {\n"
      "  char buf[32]; wchar_t wbuf[32]; unsigned char mbuf[32];\n"
      "  _mbsncpy(mbuf, \"a\", 5);\n  _mbsncat(mbuf, \"b\", 5);\n  "
      "_mbsnset(mbuf, 'a', 5);\n"
      "  memmove(buf, buf+1, 5);\n  wmemcpy(wbuf, wbuf+1, 5);\n  "
      "wmemmove(wbuf, wbuf+1, 5);\n"
      "  _ltoa(1, buf, 10);\n  _ultoa(1, buf, 10);\n  _i64toa(1, buf, 10);\n  "
      "_ui64toa(1, buf, 10);\n"
      "}\n",
      "void test_extra_funcs4(void) {\n"
      "  char buf[32]; wchar_t wbuf[32];\n"
      "  _itow(1, wbuf, 10);\n  _ltow(1, wbuf, 10);\n  _ultow(1, wbuf, 10);\n"
      "  _wmakepath(wbuf, L\"c\", L\"dir\", L\"f\", L\"ext\");\n  "
      "wcstombs(buf, wbuf, 32);\n"
      "  _wsearchenv(L\"f\", L\"PATH\", wbuf);\n  _strerror(\"msg\");\n"
      "}\n",
      "void test_putenv_no_eq(void) {\n  _putenv(\"NOEQUALS\");\n  "
      "_putenv(\"=VAL\");\n  _wputenv(L\"KEY=VAL\");\n}\n",
      "void test_long_comment(void) {\n  /* a comment with length >= 16 */\n  "
      "char dest[10];\n  strcpy(dest, \"val\");\n}\n",
      "void test_unclosed_scanset(void) {\n  char buf[32];\n  sscanf(\"str\", "
      "\"%[abc\", buf);\n  sscanf(\"str\", \"%[^abc]\", buf);\n}\n",
      "void test_infer_patterns(void) {\n"
      "  char buf[32];\n"
      "  char *p1 = malloc(64);\n"
      "  char *p2 = calloc(10, 4);\n"
      "  char *p3 = realloc(p1, 128);\n"
      "  strcpy(&buf[2], \"a\");\n"
      "  strcpy(buf + 2, \"b\");\n"
      "  strcpy(p1, \"c\");\n"
      "  strcpy(p2, \"d\");\n"
      "  strcpy(p3, \"e\");\n"
      "}\n",
      "void test_pool_expand(void){\n"
      "char b[1];\n"
      "strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");"
      "strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");"
      "strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");"
      "strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");"
      "strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");"
      "strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");"
      "strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");"
      "strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");strcpy(b,\"\");"
      "strcpy(b,\"\");strcpy(b,\"\");\n}\n"

  };
  size_t i;
  memset(&config, 0, sizeof(config));

  for (i = 0; i < sizeof(snippets) / sizeof(snippets[0]); ++i) {
    int p_rc = cdd_cst_parse(
        az_span_create_from_str((char *)(size_t)snippets[i]), &tree);
    int t_rc = 0;
    if (p_rc == 0) {
      t_rc = cdd_transform_safe_crt(tree, &config);
      cdd_cst_tree_free(tree);
      tree = NULL;
    }
    if (p_rc != 0 || t_rc != 0) {
      fprintf(stderr, "Snippet %d failed: parse=%d, trans=%d: %s\n", (int)i,
              p_rc, t_rc, snippets[i]);
    }
    ASSERT_EQ(0, p_rc);
    ASSERT_EQ(0, t_rc);
  }
  PASS();
}

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT cdd_c_error_t arena_alloc(size_t len, void **out_ptr);
extern C_CDD_EXPORT cdd_c_error_t parse_expr_ast(void *stmt, size_t *idx,
                                                 int stop_at_comma,
                                                 void *out_ast);
extern C_CDD_EXPORT cdd_c_error_t find_and_mark_fopen(void *head,
                                                      int *out_found);
extern C_CDD_EXPORT cdd_c_error_t check_unsupported_calls(void *head);
extern C_CDD_EXPORT cdd_c_error_t check_needs_transform(void *head);
extern C_CDD_EXPORT cdd_c_error_t expr_is_null_or_zero(void *node);
extern C_CDD_EXPORT cdd_c_error_t clone_trivia(void *head, void **out_trivia);
extern C_CDD_EXPORT cdd_c_error_t clone_token(void *tree, void *tok,
                                              void **out_tok);
extern C_CDD_EXPORT void emit_inferred_size(cdd_cst_builder_t *bld, void *dest);
extern C_CDD_EXPORT const char *safe_crt_pool_string_safe(void *tree,
                                                          const char *str);
struct safe_crt_expr_test {
  int type;
  cdd_token_t *tok;
  cdd_token_t *close_tok;
  void *args[16];
  size_t num_args;
  void *next;
};
extern C_CDD_EXPORT cdd_c_error_t emit_ast_bld_strip(void *node,
                                                     cdd_cst_builder_t *bld,
                                                     int is_msc);
extern C_CDD_EXPORT cdd_c_error_t
emit_ast_bld_strip_ampersand(void *node, cdd_cst_builder_t *bld, int is_msc);
extern C_CDD_EXPORT void get_indent_string(cdd_token_t *tok, char *out_indent);
extern C_CDD_EXPORT int g_cdd_cst_alloc_token_fail;
#endif

TEST test_cdd_transform_safe_crt_direct_internals(void) {
  void *ptr = NULL;
  size_t idx = 0;
  int found = 0;

  /* arena_alloc */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arena_alloc(10, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, arena_alloc(10, &ptr));
  ASSERT(ptr != NULL);

  /* parse_expr_ast */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_expr_ast(NULL, &idx, 0, NULL));

  /* find_and_mark_fopen */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, find_and_mark_fopen(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, find_and_mark_fopen(NULL, &found));
  ASSERT_EQ(0, found);

  /* check_unsupported_calls */
  ASSERT_EQ(CDD_C_SUCCESS, check_unsupported_calls(NULL));

  /* check_needs_transform */
  ASSERT_EQ(CDD_C_SUCCESS, check_needs_transform(NULL));
  {
    /* emit_inferred_size with unknown token (name remains NULL) and with
     * cdd_query_err_fail */
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    struct safe_crt_expr_test unknown_node;
    cdd_token_t t_num;
    memset(&unknown_node, 0, sizeof(unknown_node));
    memset(&t_num, 0, sizeof(t_num));
    t_num.kind = CDD_TOKEN_STRING;
    t_num.start = (const uint8_t *)"\"hello\"";
    t_num.length = 7;
    unknown_node.tok = &t_num;

    if (cdd_cst_parse(az_span_create_from_str("void f() { int x; }"),
                      &dummy_tree) == 0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);
        emit_inferred_size(&dummy_bld, (void *)&unknown_node);

        /* now with identifier and g_cdd_query_err_fail */
        t_num.kind = CDD_TOKEN_IDENTIFIER;
        t_num.start = (const uint8_t *)"x";
        t_num.length = 1;
        g_cdd_query_err_fail = 1;
        emit_inferred_size(&dummy_bld, (void *)&unknown_node);
        g_cdd_query_err_fail = 0;
      }
      cdd_cst_tree_free(dummy_tree);
    }
  }
  {
    /* check_needs_transform with type 2 node wrapping type 3 */
    struct safe_crt_expr_test n2, n3;
    memset(&n2, 0, sizeof(n2));
    memset(&n3, 0, sizeof(n3));
    n3.type = 3;
    n2.type = 2;
    n2.args[0] = &n3;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, check_needs_transform(&n2));
  }
  {
    /* emit_ast_bld directly on call node with NULL close_tok */
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    struct safe_crt_expr_test call_ast;
    memset(&call_ast, 0, sizeof(call_ast));
    call_ast.type = 1; /* type = 1 (call) */
    {
      cdd_token_t t;
      memset(&t, 0, sizeof(t));
      t.kind = CDD_TOKEN_IDENTIFIER;
      t.start = (const uint8_t *)"strlen";
      t.length = 6;
      call_ast.tok = &t;
      /* close_tok remains NULL */
      if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &dummy_tree) ==
          0) {
        if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
          cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);
          emit_ast_bld_strip((void *)&call_ast, &dummy_bld, 0);
        }
        cdd_cst_tree_free(dummy_tree);
      }
    }
  }
  {
    /* check_needs_transform with type 3 node */
    int n3[32];
    memset(n3, 0, sizeof(n3));
    n3[0] = 3; /* type = 3 */
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, check_needs_transform(n3));
  }

  /* expr_is_null_or_zero */
  ASSERT_EQ(CDD_C_SUCCESS, expr_is_null_or_zero(NULL));

  /* clone_trivia */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, clone_trivia(NULL, NULL));

  /* clone_token */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, clone_token(NULL, NULL, NULL));

  {
    /* emit_ast_bld_strip with leading trivia on first token */
    cdd_cst_tree_t *t = NULL;
    cdd_cst_node_t *n = NULL;
    cdd_cst_builder_t b;
    size_t s_idx = 0;
    void *ast = NULL;
    if (cdd_cst_parse(az_span_create_from_str("/*hello*/ foo;"), &t) == 0) {
      if (t->root && t->root->num_children > 0) {
        parse_expr_ast(t->root->children[0].val.node, &s_idx, 0, (void *)&ast);
        if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &n) == 0) {
          cdd_cst_builder_init(&b, t, n);
          emit_ast_bld_strip(ast, &b, 0);
        }
      }
      cdd_cst_tree_free(t);
    }
  }

  {
    /* safe_crt_pool_string_safe tests */
    cdd_cst_tree_t *dummy_tree = NULL;
    ASSERT_EQ(NULL, safe_crt_pool_string_safe(NULL, "test"));
    ASSERT_EQ(NULL, safe_crt_pool_string_safe((void *)1, NULL));
    if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &dummy_tree) ==
        0) {
      g_safe_crt_malloc_fail = 1;
      ASSERT_EQ(NULL, safe_crt_pool_string_safe(dummy_tree, "dup_fail"));
      g_safe_crt_malloc_fail = 2;
      /* strdup succeeds, then pool realloc fails because new_cap becomes 0 */
      ASSERT_EQ(NULL, safe_crt_pool_string_safe(dummy_tree, "pool_fail"));
      g_safe_crt_malloc_fail = 3;
      safe_crt_pool_string_safe(dummy_tree, "pool_fail3");
      g_safe_crt_malloc_fail = 0;
      cdd_cst_tree_free(dummy_tree);
    }
  }

  {
    /* clone_trivia with multiple elements and failure on second element */
    cdd_trivia_t t1, t2;
    cdd_trivia_t *out_triv = NULL;
    memset(&t1, 0, sizeof(t1));
    memset(&t2, 0, sizeof(t2));
    t1.next = &t2;
    g_safe_crt_malloc_fail = 2;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, clone_trivia(&t1, (void **)&out_triv));
    g_safe_crt_malloc_fail = 0;
  }
  {
    /* clone_token with failed token creation */
    cdd_token_t tok_in;
    cdd_token_t *tok_out = NULL;
    memset(&tok_in, 0, sizeof(tok_in));
    /* NULL tree causes cdd_cst_create_token_len to fail */
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              clone_token(NULL, &tok_in, (void **)&tok_out));
  }

  {
    /* emit_inferred_size with invalid size expression */
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    emit_inferred_size(NULL, NULL);
    memset(&dummy_bld, 0, sizeof(dummy_bld));
    emit_inferred_size(&dummy_bld, NULL);
    emit_ast_bld_strip(NULL, &dummy_bld, 0);

    if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &dummy_tree) ==
        0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);
        emit_inferred_size(&dummy_bld, NULL);
      }
      cdd_cst_tree_free(dummy_tree);
    }
  }

  {
    /* expr_is_null_or_zero variants */
    struct safe_crt_expr_test n_test, n_next;
    cdd_token_t t_zero, t_one, t_null;
    memset(&n_test, 0, sizeof(n_test));
    memset(&n_next, 0, sizeof(n_next));
    memset(&t_zero, 0, sizeof(t_zero));
    memset(&t_one, 0, sizeof(t_one));
    memset(&t_null, 0, sizeof(t_null));

    t_zero.kind = CDD_TOKEN_NUMBER;
    t_zero.start = (const uint8_t *)"0";
    t_zero.length = 1;

    t_one.kind = CDD_TOKEN_NUMBER;
    t_one.start = (const uint8_t *)"1";
    t_one.length = 1;

    t_null.kind = CDD_TOKEN_IDENTIFIER;
    t_null.start = (const uint8_t *)"NULL";
    t_null.length = 4;

    n_test.type = 1;
    ASSERT_EQ(CDD_C_SUCCESS, expr_is_null_or_zero((void *)&n_test));

    n_test.type = 0;
    n_test.tok = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, expr_is_null_or_zero((void *)&n_test));

    n_test.tok = &t_zero;
    n_test.next = &n_next;
    ASSERT_EQ(CDD_C_SUCCESS, expr_is_null_or_zero((void *)&n_test));

    n_test.next = NULL;
    n_test.tok = &t_one;
    ASSERT_EQ(CDD_C_SUCCESS, expr_is_null_or_zero((void *)&n_test));

    n_test.tok = &t_null;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, expr_is_null_or_zero((void *)&n_test));

    n_test.tok = &t_zero;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, expr_is_null_or_zero((void *)&n_test));
  }

  {
    /* check_unsupported_calls with long identifier */
    struct safe_crt_expr_test long_call;
    cdd_token_t t_long;
    char long_name[140];
    memset(long_name, 'a', 139);
    long_name[139] = '\0';
    memset(&t_long, 0, sizeof(t_long));
    t_long.kind = CDD_TOKEN_IDENTIFIER;
    t_long.start = (const uint8_t *)long_name;
    t_long.length = 139;
    memset(&long_call, 0, sizeof(long_call));
    long_call.type = 1;
    long_call.tok = &t_long;
    ASSERT_EQ(CDD_C_SUCCESS, check_unsupported_calls((void *)&long_call));
  }

  {
    /* infer_buffer_size edge cases */
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    struct safe_crt_expr_test n1, n2, n3, n_call_test;
    cdd_token_t t_amp, t_buf, t_lbrk, t_star, t_not_ident, t_plus, t_call_name;

    memset(&n1, 0, sizeof(n1));
    memset(&n2, 0, sizeof(n2));
    memset(&n3, 0, sizeof(n3));
    memset(&n_call_test, 0, sizeof(n_call_test));
    memset(&t_amp, 0, sizeof(t_amp));
    memset(&t_buf, 0, sizeof(t_buf));
    memset(&t_lbrk, 0, sizeof(t_lbrk));
    memset(&t_star, 0, sizeof(t_star));
    memset(&t_not_ident, 0, sizeof(t_not_ident));
    memset(&t_plus, 0, sizeof(t_plus));
    memset(&t_call_name, 0, sizeof(t_call_name));

    t_amp.kind = CDD_TOKEN_OTHER;
    t_amp.start = (const uint8_t *)"&";
    t_amp.length = 1;

    t_star.kind = CDD_TOKEN_STAR;
    t_star.start = (const uint8_t *)"*";
    t_star.length = 1;

    t_buf.kind = CDD_TOKEN_IDENTIFIER;
    t_buf.start = (const uint8_t *)"buf";
    t_buf.length = 3;

    t_not_ident.kind = CDD_TOKEN_NUMBER;
    t_not_ident.start = (const uint8_t *)"1";
    t_not_ident.length = 1;

    t_lbrk.kind = CDD_TOKEN_LBRACKET;
    t_lbrk.start = (const uint8_t *)"[";
    t_lbrk.length = 1;

    t_plus.kind = CDD_TOKEN_PLUS;
    t_plus.start = (const uint8_t *)"+";
    t_plus.length = 1;

    if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &dummy_tree) ==
        0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);

        /* length != 1 */
        t_amp.length = 2;
        n1.tok = &t_amp;
        emit_inferred_size(&dummy_bld, (void *)&n1);
        t_amp.length = 1;

        /* start[0] != '&' */
        n1.tok = &t_star;
        emit_inferred_size(&dummy_bld, (void *)&n1);
        n1.tok = &t_amp;

        /* n1.next == NULL */
        n1.next = NULL;
        emit_inferred_size(&dummy_bld, (void *)&n1);

        /* n1.next->type != 0 */
        n1.next = &n2;
        n2.type = 1;
        n2.tok = &t_buf;
        emit_inferred_size(&dummy_bld, (void *)&n1);
        n2.type = 0;

        /* n2.tok->kind != CDD_TOKEN_IDENTIFIER */
        n2.tok = &t_not_ident;
        emit_inferred_size(&dummy_bld, (void *)&n1);
        n2.tok = &t_buf;

        /* n2.next == NULL */
        n2.next = NULL;
        emit_inferred_size(&dummy_bld, (void *)&n1);

        /* n2.next->type != 2 */
        n2.next = &n3;
        n3.type = 0;
        emit_inferred_size(&dummy_bld, (void *)&n1);
        n3.type = 2;

        /* n3.tok->kind != CDD_TOKEN_LBRACKET */
        n3.tok = &t_star;
        emit_inferred_size(&dummy_bld, (void *)&n1);

        /* buf + expr sub-conditions */
        n1.tok = &t_buf;
        n1.next = NULL;
        emit_inferred_size(&dummy_bld, (void *)&n1);

        n1.next = &n2;
        n2.type = 1;
        n2.tok = &t_buf;
        emit_inferred_size(&dummy_bld, (void *)&n1);
        n2.type = 0;

        n2.tok = &t_star;
        emit_inferred_size(&dummy_bld, (void *)&n1);

        n2.tok = &t_plus;
        n2.next = NULL;
        emit_inferred_size(&dummy_bld, (void *)&n1);

        /* malloc/calloc/realloc sub-conditions */
        n_call_test.type = 1;
        t_call_name.kind = CDD_TOKEN_IDENTIFIER;
        n_call_test.tok = &t_call_name;

        n_call_test.num_args = 1;
        t_call_name.start = (const uint8_t *)"f";
        t_call_name.length = 1;
        emit_inferred_size(&dummy_bld, (void *)&n_call_test);

        t_call_name.start = (const uint8_t *)"malloX";
        t_call_name.length = 6;
        emit_inferred_size(&dummy_bld, (void *)&n_call_test);

        n_call_test.num_args = 2;
        t_call_name.start = (const uint8_t *)"f";
        t_call_name.length = 1;
        emit_inferred_size(&dummy_bld, (void *)&n_call_test);

        t_call_name.start = (const uint8_t *)"calloX";
        t_call_name.length = 6;
        emit_inferred_size(&dummy_bld, (void *)&n_call_test);

        t_call_name.start = (const uint8_t *)"realloX";
        t_call_name.length = 7;
        emit_inferred_size(&dummy_bld, (void *)&n_call_test);
      }
      cdd_cst_tree_free(dummy_tree);
    }
  }

  {
    /* parse_expr_ast edge cases: unclosed and brace */
    cdd_cst_tree_t *t_brace = NULL;
    size_t s_idx = 0;
    void *ast_out = NULL;
    cdd_cst_node_t stmt;
    cdd_cst_child_t children[3];
    cdd_token_t tok_rbrace, tok_lbrace, tok_ident, tok_lparen;
    size_t test_idx = 0;

    memset(&stmt, 0, sizeof(stmt));
    memset(children, 0, sizeof(children));
    memset(&tok_rbrace, 0, sizeof(tok_rbrace));
    memset(&tok_lbrace, 0, sizeof(tok_lbrace));
    memset(&tok_ident, 0, sizeof(tok_ident));
    memset(&tok_lparen, 0, sizeof(tok_lparen));

    tok_rbrace.kind = CDD_TOKEN_RBRACE;
    tok_lbrace.kind = CDD_TOKEN_LBRACE;
    tok_ident.kind = CDD_TOKEN_IDENTIFIER;
    tok_lparen.kind = CDD_TOKEN_LPAREN;

    children[0].kind = CDD_CST_CHILD_TOKEN;
    children[0].val.token = &tok_rbrace;
    stmt.children = children;
    stmt.num_children = 1;

    test_idx = 0;
    parse_expr_ast(&stmt, &test_idx, 0, &ast_out);

    children[0].val.token = &tok_lbrace;
    test_idx = 0;
    parse_expr_ast(&stmt, &test_idx, 0, &ast_out);

    children[0].val.token = &tok_ident;
    test_idx = 0;
    parse_expr_ast(&stmt, &test_idx, 0, &ast_out);

    children[0].val.token = &tok_ident;
    children[1].kind = CDD_CST_CHILD_TOKEN;
    children[1].val.token = &tok_lparen;
    stmt.num_children = 2;
    test_idx = 0;
    parse_expr_ast(&stmt, &test_idx, 0, &ast_out);

    if (cdd_cst_parse(az_span_create_from_str("void f() { { int x; } }"),
                      &t_brace) == 0) {
      if (t_brace->root && t_brace->root->num_children > 0) {
        parse_expr_ast(t_brace->root->children[0].val.node, &s_idx, 0,
                       &ast_out);
      }
      cdd_cst_tree_free(t_brace);
    }
  }

  {
    /* check_needs_transform branch coverage */
    struct safe_crt_expr_test n_chk;
    cdd_token_t t_chk;
    const char *names[] = {"vprintf", "_vsnprintf", "vscanf", "vfscanf",
                           "vsscanf", "strlen",     "tmpfile"};
    size_t k;
    memset(&n_chk, 0, sizeof(n_chk));
    memset(&t_chk, 0, sizeof(t_chk));
    t_chk.kind = CDD_TOKEN_IDENTIFIER;
    n_chk.tok = &t_chk;

    /* type = 5 */
    n_chk.type = 5;
    t_chk.start = (const uint8_t *)"strcpy";
    t_chk.length = 6;
    check_needs_transform((void *)&n_chk);

    /* length >= 127 */
    t_chk.length = 130;
    check_needs_transform((void *)&n_chk);

    n_chk.type = 1;
    for (k = 0; k < sizeof(names) / sizeof(names[0]); k++) {
      t_chk.start = (const uint8_t *)names[k];
      t_chk.length = strlen(names[k]);
      check_needs_transform((void *)&n_chk);
    }
  }

  {
    /* get_indent_string branch coverage */
    char out_ind[64];
    cdd_token_t tok_ind;
    cdd_trivia_t triv_ind;
    char long_spaces[70];
    memset(long_spaces, ' ', 68);
    long_spaces[68] = '\0';
    memset(&tok_ind, 0, sizeof(tok_ind));
    memset(&triv_ind, 0, sizeof(triv_ind));

    /* tok == NULL */
    get_indent_string(NULL, out_ind);

    /* last_ws->length >= 63 */
    triv_ind.kind = TRIVIA_WHITESPACE;
    triv_ind.start = (const uint8_t *)long_spaces;
    triv_ind.length = 68;
    tok_ind.leading_trivia = &triv_ind;
    get_indent_string(&tok_ind, out_ind);
  }

  {
    /* transform with NULL root */
    cdd_cst_tree_t empty_tree;
    cdd_transform_config_t cfg;
    memset(&empty_tree, 0, sizeof(empty_tree));
    memset(&cfg, 0, sizeof(cfg));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_transform_safe_crt(&empty_tree, &cfg));
  }

  {
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    struct safe_crt_expr_test n_emit;
    cdd_token_t t_emit;
    const char *emit_names[] = {"vfprintf", "vscanf", "_stricmp"};
    size_t k;

    memset(&n_emit, 0, sizeof(n_emit));
    memset(&t_emit, 0, sizeof(t_emit));
    t_emit.kind = CDD_TOKEN_IDENTIFIER;
    n_emit.type = 1;

    if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &dummy_tree) ==
        0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);

        /* node->tok == NULL */
        n_emit.tok = NULL;
        emit_ast_bld_strip((void *)&n_emit, &dummy_bld, 0);

        n_emit.tok = &t_emit;
        for (k = 0; k < sizeof(emit_names) / sizeof(emit_names[0]); k++) {
          t_emit.start = (const uint8_t *)emit_names[k];
          t_emit.length = strlen(emit_names[k]);
          emit_ast_bld_strip((void *)&n_emit, &dummy_bld, 1);
        }
      }
      cdd_cst_tree_free(dummy_tree);
    }
  }

  {
    cdd_cst_node_t stmt;
    cdd_cst_child_t children[4];
    cdd_token_t t_ident, t_lpar, t_comma, t_rpar, t_semi;
    size_t test_idx = 0;
    void *out_expr = NULL;

    memset(&stmt, 0, sizeof(stmt));
    memset(children, 0, sizeof(children));
    memset(&t_ident, 0, sizeof(t_ident));
    memset(&t_lpar, 0, sizeof(t_lpar));
    memset(&t_comma, 0, sizeof(t_comma));
    memset(&t_rpar, 0, sizeof(t_rpar));
    memset(&t_semi, 0, sizeof(t_semi));

    t_ident.kind = CDD_TOKEN_IDENTIFIER;
    t_lpar.kind = CDD_TOKEN_LPAREN;
    t_comma.kind = CDD_TOKEN_COMMA;
    t_rpar.kind = CDD_TOKEN_RPAREN;
    t_semi.kind = CDD_TOKEN_SEMICOLON;

    /* foo(,) */
    children[0].kind = CDD_CST_CHILD_TOKEN;
    children[0].val.token = &t_ident;
    children[1].kind = CDD_CST_CHILD_TOKEN;
    children[1].val.token = &t_lpar;
    children[2].kind = CDD_CST_CHILD_TOKEN;
    children[2].val.token = &t_comma;
    children[3].kind = CDD_CST_CHILD_TOKEN;
    children[3].val.token = &t_rpar;
    stmt.children = children;
    stmt.num_children = 4;
    test_idx = 0;
    parse_expr_ast(&stmt, &test_idx, 0, &out_expr);

    /* foo(;) */
    children[2].val.token = &t_semi;
    test_idx = 0;
    parse_expr_ast(&stmt, &test_idx, 0, &out_expr);

    /* foo(a) ending at num_children */
    stmt.num_children = 3;
    children[2].val.token = &t_ident;
    test_idx = 0;
    parse_expr_ast(&stmt, &test_idx, 0, &out_expr);
  }

  {
    /* find_and_mark_fopen with long call name and lhs == curr */
    struct safe_crt_expr_test n_assign, n_call, n_semi;
    cdd_token_t t_assign, t_long_call, t_semi;
    int found_test = 0;
    char long_call_name[20];
    memset(long_call_name, 'x', 19);
    long_call_name[19] = '\0';

    memset(&n_assign, 0, sizeof(n_assign));
    memset(&n_call, 0, sizeof(n_call));
    memset(&n_semi, 0, sizeof(n_semi));
    memset(&t_assign, 0, sizeof(t_assign));
    memset(&t_long_call, 0, sizeof(t_long_call));
    memset(&t_semi, 0, sizeof(t_semi));

    t_semi.kind = CDD_TOKEN_SEMICOLON;
    t_assign.kind = CDD_TOKEN_ASSIGN;
    t_long_call.kind = CDD_TOKEN_IDENTIFIER;
    t_long_call.start = (const uint8_t *)long_call_name;
    t_long_call.length = 19;

    n_assign.type = 0;
    n_assign.tok = &t_assign;
    n_assign.next = &n_call;

    n_call.type = 1;
    n_call.tok = &t_long_call;

    find_and_mark_fopen((void *)&n_assign, &found_test);

    /* fopen with semicolon before assign so lhs_start is NULL and t is NULL */
    n_semi.type = 0;
    n_semi.tok = &t_semi;
    n_semi.next = &n_assign;
    t_long_call.start = (const uint8_t *)"fopen";
    t_long_call.length = 5;
    find_and_mark_fopen((void *)&n_semi, &found_test);
  }

  {
    /* infer_buffer_size with NULL current_tree->root and NULL dest */
    cdd_cst_tree_t tree_null_root;
    cdd_cst_builder_t bld_null_root;
    cdd_cst_node_t dummy_tgt;
    struct safe_crt_expr_test n_dest;
    cdd_token_t t_dest;
    memset(&tree_null_root, 0, sizeof(tree_null_root));
    memset(&bld_null_root, 0, sizeof(bld_null_root));
    memset(&dummy_tgt, 0, sizeof(dummy_tgt));
    memset(&n_dest, 0, sizeof(n_dest));
    memset(&t_dest, 0, sizeof(t_dest));

    t_dest.kind = CDD_TOKEN_IDENTIFIER;
    t_dest.start = (const uint8_t *)"buf";
    t_dest.length = 3;
    n_dest.type = 0;
    n_dest.tok = &t_dest;

    cdd_cst_builder_init(&bld_null_root, &tree_null_root, &dummy_tgt);

    /* current_tree->root is NULL */
    emit_inferred_size(&bld_null_root, (void *)&n_dest);

    /* dest is NULL */
    tree_null_root.root = &dummy_tgt;
    emit_inferred_size(&bld_null_root, NULL);
  }

  {
    /* infer_buffer_size Pattern 1 & 2 comprehensive branches */
    cdd_cst_tree_t tree_pat;
    cdd_cst_builder_t bld_pat;
    cdd_cst_node_t *root_node = NULL, *stmt_node = NULL,
                   *dummy_child_node = NULL;
    cdd_cst_child_t pat_children[12];
    cdd_token_t t_buf_id, t_lbrk_tok, t_type_id, t_assign_tok, t_malloc_tok,
        t_other_tok;
    cdd_token_t t_lpar, t_rpar, t_comma, t_arg;
    struct safe_crt_expr_test n_buf_dest;

    memset(&tree_pat, 0, sizeof(tree_pat));
    memset(&bld_pat, 0, sizeof(bld_pat));
    memset(pat_children, 0, sizeof(pat_children));
    memset(&t_buf_id, 0, sizeof(t_buf_id));
    memset(&t_lbrk_tok, 0, sizeof(t_lbrk_tok));
    memset(&t_type_id, 0, sizeof(t_type_id));
    memset(&t_assign_tok, 0, sizeof(t_assign_tok));
    memset(&t_malloc_tok, 0, sizeof(t_malloc_tok));
    memset(&t_other_tok, 0, sizeof(t_other_tok));
    memset(&t_lpar, 0, sizeof(t_lpar));
    memset(&t_rpar, 0, sizeof(t_rpar));
    memset(&t_comma, 0, sizeof(t_comma));
    memset(&t_arg, 0, sizeof(t_arg));
    memset(&n_buf_dest, 0, sizeof(n_buf_dest));

    t_lpar.kind = CDD_TOKEN_LPAREN;
    t_rpar.kind = CDD_TOKEN_RPAREN;
    t_comma.kind = CDD_TOKEN_COMMA;
    t_arg.kind = CDD_TOKEN_NUMBER;
    t_arg.start = (const uint8_t *)"1";
    t_arg.length = 1;

    t_buf_id.kind = CDD_TOKEN_IDENTIFIER;
    t_buf_id.start = (const uint8_t *)"buf";
    t_buf_id.length = 3;

    t_type_id.kind = CDD_TOKEN_IDENTIFIER;
    t_type_id.start = (const uint8_t *)"char";
    t_type_id.length = 4;

    t_lbrk_tok.kind = CDD_TOKEN_LBRACKET;
    t_assign_tok.kind = CDD_TOKEN_ASSIGN;
    t_other_tok.kind = CDD_TOKEN_NUMBER;

    t_malloc_tok.kind = CDD_TOKEN_IDENTIFIER;
    t_malloc_tok.start = (const uint8_t *)"malloc";
    t_malloc_tok.length = 6;

    n_buf_dest.type = 0;
    n_buf_dest.tok = &t_buf_id;

    if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &root_node) == 0 &&
        cdd_cst_alloc_node(CDD_CST_UNKNOWN, &stmt_node) == 0 &&
        cdd_cst_alloc_node(CDD_CST_EXPRESSION, &dummy_child_node) == 0) {
      tree_pat.root = root_node;
      cdd_cst_append_child_node(root_node, stmt_node);
      cdd_cst_builder_init(&bld_pat, &tree_pat, root_node);

      /* Case 1: j == 0 (t_buf_id is first child) -> j > 0 is false! */
      pat_children[0].kind = CDD_CST_CHILD_TOKEN;
      pat_children[0].val.token = &t_buf_id;
      pat_children[1].kind = CDD_CST_CHILD_TOKEN;
      pat_children[1].val.token = &t_type_id;
      stmt_node->children = pat_children;
      stmt_node->num_children = 2;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 2: j + 1 < stmt->num_children is false (j is last child) */
      pat_children[0].val.token = &t_type_id;
      pat_children[1].val.token = &t_buf_id;
      stmt_node->num_children = 2;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 3: children[j + 1].kind != CDD_CST_CHILD_TOKEN */
      pat_children[2].kind = CDD_CST_CHILD_NODE;
      pat_children[2].val.node = dummy_child_node;
      stmt_node->num_children = 3;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 4: children[j - 1].kind != CDD_CST_CHILD_TOKEN */
      pat_children[0].kind = CDD_CST_CHILD_NODE;
      pat_children[0].val.node = dummy_child_node;
      pat_children[2].kind = CDD_CST_CHILD_TOKEN;
      pat_children[2].val.token = &t_lbrk_tok;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 5: children[j - 1].val.token->kind != CDD_TOKEN_IDENTIFIER */
      pat_children[0].kind = CDD_CST_CHILD_TOKEN;
      pat_children[0].val.token = &t_other_tok;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 6: Pattern 2: children[j + 1] is node */
      pat_children[0].val.token = &t_buf_id;
      pat_children[1].kind = CDD_CST_CHILD_NODE;
      pat_children[1].val.node = dummy_child_node;
      pat_children[2].kind = CDD_CST_CHILD_TOKEN;
      pat_children[2].val.token = &t_other_tok;
      stmt_node->num_children = 3;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 7: Pattern 2: children[j + 1] is token, but not ASSIGN */
      pat_children[1].kind = CDD_CST_CHILD_TOKEN;
      pat_children[1].val.token = &t_other_tok;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 7b: Pattern 2: children[j + 1] is ASSIGN, children[j + 2] is node
       */
      pat_children[1].val.token = &t_assign_tok;
      pat_children[2].kind = CDD_CST_CHILD_NODE;
      pat_children[2].val.node = dummy_child_node;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 8: Pattern 2: children[j + 1] is ASSIGN, children[j + 2] is not
       * identifier */
      pat_children[2].kind = CDD_CST_CHILD_TOKEN;
      pat_children[2].val.token = &t_other_tok;
      pat_children[3].kind = CDD_CST_CHILD_TOKEN;
      pat_children[3].val.token = &t_other_tok;
      stmt_node->num_children = 4;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 9: m_tok->length < 6 */
      t_other_tok.kind = CDD_TOKEN_IDENTIFIER;
      t_other_tok.start = (const uint8_t *)"f";
      t_other_tok.length = 1;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 10: m_tok->length >= 6 but name is foobar */
      t_other_tok.start = (const uint8_t *)"foobar";
      t_other_tok.length = 6;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 11: m_expr with type != 1 */
      pat_children[2].val.token = &t_malloc_tok;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 12: calloc with num_args != 2 */
      t_malloc_tok.start = (const uint8_t *)"calloc";
      t_malloc_tok.length = 6;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 13: realloc with num_args != 2 */
      t_malloc_tok.start = (const uint8_t *)"realloc";
      t_malloc_tok.length = 7;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 14: Pattern 2: malloc with 2 args -> malloc(1, 2) */
      t_malloc_tok.start = (const uint8_t *)"malloc";
      t_malloc_tok.length = 6;
      pat_children[0].val.token = &t_buf_id;
      pat_children[1].kind = CDD_CST_CHILD_TOKEN;
      pat_children[1].val.token = &t_assign_tok;
      pat_children[2].kind = CDD_CST_CHILD_TOKEN;
      pat_children[2].val.token = &t_malloc_tok;
      pat_children[3].kind = CDD_CST_CHILD_TOKEN;
      pat_children[3].val.token = &t_lpar;
      pat_children[4].kind = CDD_CST_CHILD_TOKEN;
      pat_children[4].val.token = &t_arg;
      pat_children[5].kind = CDD_CST_CHILD_TOKEN;
      pat_children[5].val.token = &t_comma;
      pat_children[6].kind = CDD_CST_CHILD_TOKEN;
      pat_children[6].val.token = &t_arg;
      pat_children[7].kind = CDD_CST_CHILD_TOKEN;
      pat_children[7].val.token = &t_rpar;
      stmt_node->num_children = 8;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 15: Pattern 2: calloc with 3 args -> calloc(1, 2, 3) */
      t_malloc_tok.start = (const uint8_t *)"calloc";
      t_malloc_tok.length = 6;
      pat_children[7].val.token = &t_comma;
      pat_children[8].kind = CDD_CST_CHILD_TOKEN;
      pat_children[8].val.token = &t_arg;
      pat_children[9].kind = CDD_CST_CHILD_TOKEN;
      pat_children[9].val.token = &t_rpar;
      stmt_node->num_children = 10;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 16: Pattern 2: realloc with 3 args -> realloc(1, 2, 3) */
      t_malloc_tok.start = (const uint8_t *)"realloc";
      t_malloc_tok.length = 7;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      stmt_node->num_children = 0;
      stmt_node->children = NULL;
      free(dummy_child_node);
      free(stmt_node);
      free(root_node);
    }
  }

  {
    /* infer_buffer_size with NULL tok */
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    struct safe_crt_expr_test n_null_tok;
    memset(&n_null_tok, 0, sizeof(n_null_tok));

    if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &dummy_tree) ==
        0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);

        n_null_tok.type = 0;
        n_null_tok.tok = NULL;
        emit_inferred_size(&dummy_bld, (void *)&n_null_tok);

        n_null_tok.type = 1;
        emit_inferred_size(&dummy_bld, (void *)&n_null_tok);

        /* node->type == 1 with tok->kind != CDD_TOKEN_IDENTIFIER */
        {
          cdd_token_t t_num;
          memset(&t_num, 0, sizeof(t_num));
          t_num.kind = CDD_TOKEN_NUMBER;
          t_num.start = (const uint8_t *)"1";
          t_num.length = 1;
          n_null_tok.tok = &t_num;
          emit_inferred_size(&dummy_bld, (void *)&n_null_tok);
        }
      }
      cdd_cst_tree_free(dummy_tree);
    }
  }

  {
    /* _putenv literal and non-literal branches in emit_ast_bld */
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    struct safe_crt_expr_test n_putenv, n_str;
    cdd_token_t t_putenv, t_str;

    memset(&n_putenv, 0, sizeof(n_putenv));
    memset(&n_str, 0, sizeof(n_str));
    memset(&t_putenv, 0, sizeof(t_putenv));
    memset(&t_str, 0, sizeof(t_str));

    t_putenv.kind = CDD_TOKEN_IDENTIFIER;
    t_putenv.start = (const uint8_t *)"_putenv";
    t_putenv.length = 7;

    n_putenv.type = 1;
    n_putenv.tok = &t_putenv;
    n_putenv.num_args = 1;
    n_putenv.args[0] = &n_str;

    if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &dummy_tree) ==
        0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);

        /* Case 0: str_node == NULL */
        n_putenv.args[0] = NULL;
        emit_ast_bld_strip((void *)&n_putenv, &dummy_bld, 1);
        n_putenv.args[0] = &n_str;

        /* Case 1: str_node->type != 0 */
        n_str.type = 2;
        emit_ast_bld_strip((void *)&n_putenv, &dummy_bld, 1);

        /* Case 2: str_node->tok == NULL */
        n_str.type = 0;
        n_str.tok = NULL;
        emit_ast_bld_strip((void *)&n_putenv, &dummy_bld, 1);

        /* Case 3: str_node->tok->length == 2 */
        n_str.tok = &t_str;
        t_str.kind = CDD_TOKEN_IDENTIFIER;
        t_str.start = (const uint8_t *)"LL";
        t_str.length = 2;
        emit_ast_bld_strip((void *)&n_putenv, &dummy_bld, 1);

        /* Case 4: str_node->tok->start[0] != 'L' */
        t_str.start = (const uint8_t *)"M";
        t_str.length = 1;
        emit_ast_bld_strip((void *)&n_putenv, &dummy_bld, 1);

        /* Case 4b: str_node->tok->start[0] == 'L' but next is NULL */
        t_str.start = (const uint8_t *)"L";
        t_str.length = 1;
        n_str.next = NULL;
        emit_ast_bld_strip((void *)&n_putenv, &dummy_bld, 1);

        /* Case 4c: str_node->tok->start[0] == 'L', next has type != 0 */
        {
          struct safe_crt_expr_test n_next_str;
          memset(&n_next_str, 0, sizeof(n_next_str));
          n_str.next = &n_next_str;
          n_next_str.type = 2;
          emit_ast_bld_strip((void *)&n_putenv, &dummy_bld, 1);

          /* Case 4d: str_node->tok->start[0] == 'L', next has type == 0 and tok
           * == NULL */
          n_next_str.type = 0;
          n_next_str.tok = NULL;
          emit_ast_bld_strip((void *)&n_putenv, &dummy_bld, 1);
          n_str.next = NULL;
        }

        /* Case 5: string starting with L\" */
        t_str.kind = CDD_TOKEN_STRING;
        t_str.start = (const uint8_t *)"L\"A=B\"";
        t_str.length = 6;
        emit_ast_bld_strip((void *)&n_putenv, &dummy_bld, 1);

        /* Case 6: string starting with = */
        t_str.start = (const uint8_t *)"\"=B\"";
        t_str.length = 4;
        emit_ast_bld_strip((void *)&n_putenv, &dummy_bld, 1);

        /* Case 7: _wputenv with L\"A=B\" */
        t_putenv.start = (const uint8_t *)"_wputenv";
        t_putenv.length = 8;
        t_str.start = (const uint8_t *)"L\"A=B\"";
        t_str.length = 6;
        emit_ast_bld_strip((void *)&n_putenv, &dummy_bld, 1);

        /* Case 8: string starting with ' */
        t_str.start = (const uint8_t *)"'A=B'";
        t_str.length = 5;
        emit_ast_bld_strip((void *)&n_putenv, &dummy_bld, 1);

        /* Case 9: string starting with LX=Y */
        t_str.start = (const uint8_t *)"LX=Y";
        t_str.length = 4;
        emit_ast_bld_strip((void *)&n_putenv, &dummy_bld, 1);
      }
      cdd_cst_tree_free(dummy_tree);
    }
  }

  {
    /* sscanf format argument variations */
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    struct safe_crt_expr_test n_scanf, n_arg0, n_fmt;
    cdd_token_t t_scanf_id;

    memset(&n_scanf, 0, sizeof(n_scanf));
    memset(&n_arg0, 0, sizeof(n_arg0));
    memset(&n_fmt, 0, sizeof(n_fmt));
    memset(&t_scanf_id, 0, sizeof(t_scanf_id));

    t_scanf_id.kind = CDD_TOKEN_IDENTIFIER;
    t_scanf_id.start = (const uint8_t *)"sscanf";
    t_scanf_id.length = 6;

    n_scanf.type = 1;
    n_scanf.tok = &t_scanf_id;
    n_scanf.num_args = 2;
    n_scanf.args[0] = &n_arg0;
    n_scanf.args[1] = NULL; /* args[1] is NULL */

    if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &dummy_tree) ==
        0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);

        /* args[format_idx] is NULL */
        emit_ast_bld_strip((void *)&n_scanf, &dummy_bld, 1);

        /* args[format_idx]->type != 0 */
        n_scanf.args[1] = &n_fmt;
        n_fmt.type = 2;
        emit_ast_bld_strip((void *)&n_scanf, &dummy_bld, 1);

        /* args[format_idx]->type == 0, tok == NULL */
        n_fmt.type = 0;
        n_fmt.tok = NULL;
        emit_ast_bld_strip((void *)&n_scanf, &dummy_bld, 1);
      }
      cdd_cst_tree_free(dummy_tree);
    }
  }

  {
    /* node->type == 2 and is_safe == 1 with clone_token failure */
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    struct safe_crt_expr_test n_paren, n_inner;
    cdd_token_t t_paren;

    memset(&n_paren, 0, sizeof(n_paren));
    memset(&n_inner, 0, sizeof(n_inner));
    memset(&t_paren, 0, sizeof(t_paren));

    t_paren.kind = CDD_TOKEN_LPAREN;
    t_paren.start = (const uint8_t *)"(";
    t_paren.length = 1;

    n_paren.type = 2;
    n_paren.tok = &t_paren;
    n_paren.args[0] = &n_inner;

    if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &dummy_tree) ==
        0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);

        g_safe_crt_malloc_fail = 1;
        emit_ast_bld_strip((void *)&n_paren, &dummy_bld, 0);
        g_safe_crt_malloc_fail = 0;
      }
      cdd_cst_tree_free(dummy_tree);
    }
  }

  {
    /* node->type == 3 branches */
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    struct safe_crt_expr_test n_type3, n_lhs_t3, n_call_t3;
    cdd_token_t t_fopen;

    memset(&n_type3, 0, sizeof(n_type3));
    memset(&n_lhs_t3, 0, sizeof(n_lhs_t3));
    memset(&n_call_t3, 0, sizeof(n_call_t3));
    memset(&t_fopen, 0, sizeof(t_fopen));

    t_fopen.kind = CDD_TOKEN_IDENTIFIER;
    t_fopen.start = (const uint8_t *)"fopen";
    t_fopen.length = 5;

    n_type3.type = 3;
    n_type3.args[0] = &n_lhs_t3;
    n_type3.args[1] = &n_call_t3;

    n_call_t3.type = 1;
    n_call_t3.tok = &t_fopen;

    n_lhs_t3.type = 0;
    n_lhs_t3.tok = NULL;
    n_lhs_t3.next = &n_type3;

    if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &dummy_tree) ==
        0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);
        emit_ast_bld_strip((void *)&n_type3, &dummy_bld, 1);
        emit_ast_bld_strip((void *)&n_type3, &dummy_bld, 0);

        /* call with name length >= 127 */
        {
          struct safe_crt_expr_test n_long_call;
          cdd_token_t t_long;
          char long_name[140];
          memset(long_name, 'a', 139);
          long_name[139] = '\0';
          memset(&t_long, 0, sizeof(t_long));
          t_long.kind = CDD_TOKEN_IDENTIFIER;
          t_long.start = (const uint8_t *)long_name;
          t_long.length = 139;
          memset(&n_long_call, 0, sizeof(n_long_call));
          n_long_call.type = 1;
          n_long_call.tok = &t_long;
          emit_ast_bld_strip((void *)&n_long_call, &dummy_bld, 0);
        }
      }
      cdd_cst_tree_free(dummy_tree);
    }
  }

  {
    /* clone_token fail in emit_inferred_size */
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    struct safe_crt_expr_test n_char_arr;
    cdd_token_t t_char_arr;
    memset(&n_char_arr, 0, sizeof(n_char_arr));
    memset(&t_char_arr, 0, sizeof(t_char_arr));
    t_char_arr.kind = CDD_TOKEN_IDENTIFIER;
    t_char_arr.start = (const uint8_t *)"buf";
    t_char_arr.length = 3;
    n_char_arr.type = 0;
    n_char_arr.tok = &t_char_arr;

    if (cdd_cst_parse(az_span_create_from_str("void f() { char buf[10]; }"),
                      &dummy_tree) == 0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);
        /* sizeof (1st), ( (2nd), clone_token (3rd) */
        g_cdd_cst_alloc_token_fail = 3;
        emit_inferred_size(&dummy_bld, (void *)&n_char_arr);
        g_cdd_cst_alloc_token_fail = 0;
      }
      cdd_cst_tree_free(dummy_tree);
    }
  }

  {
    /* clone_token failure in emit_ast_bld */
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    struct safe_crt_expr_test n_strcpy, n_paren, n_func, n_assign_test,
        n_lhs_var, n_lhs_type, n_call_fn;
    cdd_token_t t_scpy, t_paren_close, t_func_id, t_assign_op, t_var, t_fn;
    int k;

    memset(&n_strcpy, 0, sizeof(n_strcpy));
    memset(&n_paren, 0, sizeof(n_paren));
    memset(&n_func, 0, sizeof(n_func));
    memset(&n_assign_test, 0, sizeof(n_assign_test));
    memset(&n_lhs_var, 0, sizeof(n_lhs_var));
    memset(&n_lhs_type, 0, sizeof(n_lhs_type));
    memset(&n_call_fn, 0, sizeof(n_call_fn));

    memset(&t_scpy, 0, sizeof(t_scpy));
    memset(&t_paren_close, 0, sizeof(t_paren_close));
    memset(&t_func_id, 0, sizeof(t_func_id));
    memset(&t_assign_op, 0, sizeof(t_assign_op));
    memset(&t_var, 0, sizeof(t_var));
    memset(&t_fn, 0, sizeof(t_fn));

    t_scpy.kind = CDD_TOKEN_IDENTIFIER;
    t_scpy.start = (const uint8_t *)"strcpy";
    t_scpy.length = 6;

    t_paren_close.kind = CDD_TOKEN_RPAREN;
    t_paren_close.start = (const uint8_t *)")";
    t_paren_close.length = 1;

    t_func_id.kind = CDD_TOKEN_IDENTIFIER;
    t_func_id.start = (const uint8_t *)"getenv";
    t_func_id.length = 6;

    t_assign_op.kind = CDD_TOKEN_ASSIGN;
    t_var.kind = CDD_TOKEN_IDENTIFIER;
    t_var.start = (const uint8_t *)"f";
    t_var.length = 1;

    t_fn.kind = CDD_TOKEN_IDENTIFIER;
    t_fn.start = (const uint8_t *)"fopen";
    t_fn.length = 5;

    n_strcpy.type = 1;
    n_strcpy.tok = &t_scpy;
    n_strcpy.close_tok = &t_paren_close;

    n_paren.type = 2;
    n_paren.tok = &t_paren_close;
    n_paren.close_tok = &t_paren_close;

    n_func.type = 1;
    n_func.tok = &t_func_id;

    n_assign_test.type = 3;
    n_assign_test.tok = &t_assign_op;
    n_assign_test.args[0] = &n_lhs_type;
    n_assign_test.args[1] = &n_call_fn;
    n_lhs_type.type = 0;
    n_lhs_type.tok = &t_var;
    n_lhs_type.next = &n_lhs_var;
    n_lhs_var.type = 0;
    n_lhs_var.tok = &t_var;
    n_lhs_var.next = &n_assign_test;
    n_call_fn.type = 1;
    n_call_fn.tok = &t_fn;

    if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &dummy_tree) ==
        0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);

        /* Fail on strcpy (is_safe = 1, line 984) */
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);
        g_cdd_cst_alloc_token_fail = 1;
        emit_ast_bld_strip((void *)&n_strcpy, &dummy_bld, 1);
        g_cdd_cst_alloc_token_fail = 0;

        /* Fail on close_tok (line 1470) */
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);
        g_cdd_cst_alloc_token_fail = 3;
        emit_ast_bld_strip((void *)&n_strcpy, &dummy_bld, 1);
        g_cdd_cst_alloc_token_fail = 0;

        /* Fail on type 2 close_tok (line 879) */
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);
        g_cdd_cst_alloc_token_fail = 2;
        emit_ast_bld_strip((void *)&n_paren, &dummy_bld, 0);
        g_cdd_cst_alloc_token_fail = 0;

        /* Fail on getenv (line 1012) */
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);
        g_cdd_cst_alloc_token_fail = 1;
        emit_ast_bld_strip((void *)&n_func, &dummy_bld, 1);
        g_cdd_cst_alloc_token_fail = 0;

        /* Fail on node->type == 3 tokens */
        for (k = 1; k <= 10; k++) {
          cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);
          g_cdd_cst_alloc_token_fail = k;
          emit_ast_bld_strip((void *)&n_assign_test, &dummy_bld, 1);
          g_cdd_cst_alloc_token_fail = 0;
        }
      }
      cdd_cst_tree_free(dummy_tree);
    }
  }

  {
    /* clone_token with NULL out_tok */
    cdd_token_t t_clone;
    memset(&t_clone, 0, sizeof(t_clone));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, clone_token(NULL, &t_clone, NULL));
  }

  {
    /* emit_ast_bld_strip_ampersand branches */
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    struct safe_crt_expr_test n_amp;
    cdd_token_t t_amp;

    memset(&n_amp, 0, sizeof(n_amp));
    memset(&t_amp, 0, sizeof(t_amp));
    t_amp.kind = CDD_TOKEN_OTHER;
    t_amp.start = (const uint8_t *)"&";
    t_amp.length = 1;

    if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &dummy_tree) ==
        0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);

        /* node == NULL */
        emit_ast_bld_strip_ampersand(NULL, &dummy_bld, 0);

        /* node.type = 1 */
        n_amp.type = 1;
        emit_ast_bld_strip_ampersand(&n_amp, &dummy_bld, 0);

        /* node.type = 0, tok = NULL */
        n_amp.type = 0;
        n_amp.tok = NULL;
        emit_ast_bld_strip_ampersand(&n_amp, &dummy_bld, 0);

        /* node.type = 0, tok->length = 2 */
        n_amp.tok = &t_amp;
        t_amp.length = 2;
        emit_ast_bld_strip_ampersand(&n_amp, &dummy_bld, 0);
        t_amp.length = 1;

        /* node.type = 0, length = 1, start[0] = '&' */
        emit_ast_bld_strip_ampersand(&n_amp, &dummy_bld, 0);
      }
      cdd_cst_tree_free(dummy_tree);
    }
  }

  {
    /* cdd_transform_safe_crt with empty statement and statement with node child
     */
    cdd_cst_tree_t *t_empty_stmt = NULL;
    cdd_cst_node_t *empty_node = NULL, *node_child_stmt = NULL,
                   *sub_node = NULL;
    cdd_transform_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));

    if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &t_empty_stmt) ==
        0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &empty_node) == 0 &&
          cdd_cst_alloc_node(CDD_CST_UNKNOWN, &node_child_stmt) == 0 &&
          cdd_cst_alloc_node(CDD_CST_EXPRESSION, &sub_node) == 0) {
        /* empty_node has 0 children */
        cdd_cst_append_child_node(t_empty_stmt->root, empty_node);

        /* node_child_stmt has a node child */
        cdd_cst_append_child_node(node_child_stmt, sub_node);
        cdd_cst_append_child_node(t_empty_stmt->root, node_child_stmt);

        cdd_transform_safe_crt(t_empty_stmt, &cfg);
      }
      cdd_cst_tree_free(t_empty_stmt);
    }
  }

  {
    /* node->close_tok failure in emit_ast_bld */
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    struct safe_crt_expr_test n_call_close;
    cdd_token_t t_call_id, t_close_id;

    memset(&n_call_close, 0, sizeof(n_call_close));
    memset(&t_call_id, 0, sizeof(t_call_id));
    memset(&t_close_id, 0, sizeof(t_close_id));

    t_call_id.kind = CDD_TOKEN_IDENTIFIER;
    t_call_id.start = (const uint8_t *)"printf";
    t_call_id.length = 6;

    t_close_id.kind = CDD_TOKEN_RPAREN;
    t_close_id.start = (const uint8_t *)")";
    t_close_id.length = 1;

    n_call_close.type = 1;
    n_call_close.tok = &t_call_id;
    n_call_close.close_tok = &t_close_id;

    if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &dummy_tree) ==
        0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);

        g_safe_crt_malloc_fail = 2;
        emit_ast_bld_strip((void *)&n_call_close, &dummy_bld, 0);
        g_safe_crt_malloc_fail = 0;
      }
      cdd_cst_tree_free(dummy_tree);
    }
  }

  PASS();
}

SUITE(transformer_safe_crt_suite) {
  RUN_TEST(test_cdd_transform_safe_crt_direct_internals);
  RUN_TEST(test_cdd_transform_safe_crt_more_cases);
  RUN_TEST(test_cdd_transform_safe_crt_needs_buffers);
  RUN_TEST(test_cdd_transform_safe_crt);

  RUN_TEST(test_cdd_transform_safe_crt_extended_functions);

  RUN_TEST(test_cdd_transform_safe_crt_edge_cases);
  RUN_TEST(test_cdd_transform_safe_crt_oom);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_TRANSFORM_SAFE_CRT_H */
