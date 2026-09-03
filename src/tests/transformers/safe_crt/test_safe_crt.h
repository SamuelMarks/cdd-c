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

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  (void)rc;
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
    ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code4), &tree4));
    ASSERT_EQ(0, cdd_transform_safe_crt(tree4, &config));
    cdd_cst_tree_free(tree4);

    {
      const char *code5 = "void edge5() { char *u; (strcpy(u, \"a\")); }";
      cdd_cst_tree_t *tree5 = NULL;
      ASSERT_EQ(0,
                cdd_cst_parse(az_span_create_from_str((char *)code5), &tree5));
      ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_transform_safe_crt(tree5, &config));
      cdd_cst_tree_free(tree5);

      {
        const char *code6 =
            "void edge6() { FILE *f; (f = fopen(\"a\", \"b\")); }";
        cdd_cst_tree_t *tree6 = NULL;
        ASSERT_EQ(
            0, cdd_cst_parse(az_span_create_from_str((char *)code6), &tree6));
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
          ASSERT_EQ(
              0, cdd_cst_parse(az_span_create_from_str((char *)code7), &tree7));
          ASSERT_EQ(0, cdd_transform_safe_crt(tree7, &config));
          cdd_cst_tree_free(tree7);

          {
            const char *code8 =
                "void edge8() { char *u; foo(strcpy(u, \"a\")); }";
            cdd_cst_tree_t *tree8 = NULL;
            ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code8),
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
              ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code9),
                                         &tree9));
              ASSERT_EQ(0, cdd_transform_safe_crt(tree9, &config));
              cdd_cst_tree_free(tree9);

              {
                const char *code10 = "void edge10() { scanf(\"%s\", NULL); }";
                cdd_cst_tree_t *tree10 = NULL;
                ASSERT_EQ(0,
                          cdd_cst_parse(az_span_create_from_str((char *)code10),
                                        &tree10));
                ASSERT_EQ(0, cdd_transform_safe_crt(tree10, &config));
                cdd_cst_tree_free(tree10);

                {
                  const char *code11 = "void edge11() { scanf(\"%s\", 0); }";
                  cdd_cst_tree_t *tree11 = NULL;
                  ASSERT_EQ(
                      0, cdd_cst_parse(az_span_create_from_str((char *)code11),
                                       &tree11));
                  ASSERT_EQ(0, cdd_transform_safe_crt(tree11, &config));
                  cdd_cst_tree_free(tree11);

                  {
                    const char *code12 =
                        "void edge12() { char buf[10]; scanf(\"%s %s %s %s %s "
                        "%s %s %s %s %s %s "
                        "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s "
                        "%s %s %s %s %s\", "
                        "buf, buf, buf, buf, buf, buf, buf, buf, buf, buf, "
                        "buf, buf, buf, buf, "
                        "buf, buf, buf, buf, buf, buf, buf, buf, buf, buf, "
                        "buf, buf, buf, buf, "
                        "buf, buf, buf, buf, buf, buf); }";
                    cdd_cst_tree_t *tree12 = NULL;
                    ASSERT_EQ(0, cdd_cst_parse(
                                     az_span_create_from_str((char *)code12),
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

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
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

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  ASSERT_EQ(0, cdd_transform_safe_crt(tree, &config));

  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

#ifdef CDD_BUILD_TESTS
/* extern C_CDD_EXPORT int g_safe_crt_malloc_fail; (moved to global) */
/* extern C_CDD_EXPORT int g_cdd_cst_alloc_node_fail; (moved to global) */
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

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));

  {
    int i;
    for (i = 1; i <= 50; i++) {
      cdd_cst_tree_free(tree);
      tree = NULL;
      ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
      g_safe_crt_malloc_fail = i;
      cdd_transform_safe_crt(tree, &config);
      g_safe_crt_malloc_fail = 0;
      cdd_cst_tree_free(tree);
      tree = NULL;
    }
  }

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  g_cdd_cst_alloc_node_fail = 1;
  cdd_transform_safe_crt(tree, &config);
  g_cdd_cst_alloc_node_fail = 0;
  cdd_cst_tree_free(tree);
  tree = NULL;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  g_safe_crt_malloc_fail = 2;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 0;
  cdd_cst_tree_free(tree);
  tree = NULL;
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
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

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  g_safe_crt_malloc_fail = 3;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 0;
  cdd_cst_tree_free(tree);
  tree = NULL;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  g_safe_crt_malloc_fail = 4;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 0;
  cdd_cst_tree_free(tree);
  tree = NULL;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  g_safe_crt_malloc_fail = 5;
  cdd_transform_safe_crt(tree, &config);
  g_safe_crt_malloc_fail = 0;
  cdd_cst_tree_free(tree);
  tree = NULL;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree2));
  cdd_transform_safe_crt(tree2, &config);
  cdd_cst_tree_free(tree2);

  {
    const char *code9 =
        "void edge9() { char buf[256]; strcpy(malloc(10), \"a\"); "
        "str"
        "cpy(calloc(1, 10), \"a\"); strcpy(realloc(NULL, 10), \"a\"); }";
    cdd_cst_tree_t *tree9 = NULL;
    ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code9), &tree9));
    ASSERT_EQ(0, cdd_transform_safe_crt(tree9, &config));
    cdd_cst_tree_free(tree9);

    {
      const char *code10 = "void edge10() { scanf(\"%s\", NULL); }";
      cdd_cst_tree_t *tree10 = NULL;
      ASSERT_EQ(
          0, cdd_cst_parse(az_span_create_from_str((char *)code10), &tree10));
      ASSERT_EQ(0, cdd_transform_safe_crt(tree10, &config));
      cdd_cst_tree_free(tree10);

      {
        const char *code11 = "void edge11() { scanf(\"%s\", 0); }";
        cdd_cst_tree_t *tree11 = NULL;
        ASSERT_EQ(
            0, cdd_cst_parse(az_span_create_from_str((char *)code11), &tree11));
        ASSERT_EQ(0, cdd_transform_safe_crt(tree11, &config));
        cdd_cst_tree_free(tree11);

        {
          const char *code12 = "void edge12() { char buf[10]; scanf(\"%s %s %s "
                               "%s %s %s %s %s %s %s %s "
                               "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s "
                               "%s %s %s %s %s %s %s %s\", "
                               "buf, buf, buf, buf, buf, buf, buf, buf, buf, "
                               "buf, buf, buf, buf, buf, "
                               "buf, buf, buf, buf, buf, buf, buf, buf, buf, "
                               "buf, buf, buf, buf, buf, "
                               "buf, buf, buf, buf, buf, buf); }";
          cdd_cst_tree_t *tree12 = NULL;
          ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code12),
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

SUITE(transformer_safe_crt_suite) {
  RUN_TEST(test_cdd_transform_safe_crt);

  RUN_TEST(test_cdd_transform_safe_crt_extended_functions);

  RUN_TEST(test_cdd_transform_safe_crt_edge_cases);
  RUN_TEST(test_cdd_transform_safe_crt_oom);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_TRANSFORM_SAFE_CRT_H */
