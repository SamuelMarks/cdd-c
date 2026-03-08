#ifndef TEST_CODEGEN_VALIDATION_H
#define TEST_CODEGEN_VALIDATION_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/codegen.h"

/* Helper: generate code and return as buffer */
static int gen_parse_code(const char *name, struct StructFields *sf, char * *_out_val) {
  FILE *tmp = tmpfile();
  long sz;
  char *content;

  if (!tmp)
    { *_out_val = NULL; return 0; }

  if (write_struct_from_jsonObject_func(tmp, name, sf, NULL) != 0) {
    fclose(tmp);
    { *_out_val = NULL; return 0; }
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);

  content = (char *)calloc(1, sz + 1);
  if (sz > 0)
    fread(content, 1, sz, tmp);

  fclose(tmp);
  { *_out_val = content; return 0; }
}

TEST test_int_min_validation(void) { char * _ast_gen_parse_code_0; 
  struct StructFields sf;
  char *code;
  struct StructField *f;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "val", "integer", NULL, NULL, NULL);

  f = &sf.fields[0];
  f->has_min = 1;
  f->min_val = 10.0;

  code = (gen_parse_code("SMin", &sf, &_ast_gen_parse_code_0), _ast_gen_parse_code_0);
  ASSERT(code != NULL);

  /* Check basic logic */
  ASSERT(strstr(code, "if (tmp < 10.000000) { free(ret); return ERANGE; }"));

  free(code);
  struct_fields_free(&sf);
  PASS();
}

TEST test_int_exclusive_min(void) { char * _ast_gen_parse_code_1; 
  struct StructFields sf;
  char *code;
  struct StructField *f;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "val", "integer", NULL, NULL, NULL);

  f = &sf.fields[0];
  f->has_min = 1;
  f->min_val = 5.0;
  f->exclusive_min = 1;

  code = (gen_parse_code("SExcMin", &sf, &_ast_gen_parse_code_1), _ast_gen_parse_code_1);
  ASSERT(code != NULL);

  /* Check exclusive */
  ASSERT(strstr(code, "if (tmp <= 5.000000) { free(ret); return ERANGE; }"));

  free(code);
  struct_fields_free(&sf);
  PASS();
}

TEST test_double_max_validation(void) { char * _ast_gen_parse_code_2; 
  struct StructFields sf;
  char *code;
  struct StructField *f;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "val", "number", NULL, NULL, NULL);

  f = &sf.fields[0];
  f->has_max = 1;
  f->max_val = 100.5;

  code = (gen_parse_code("SMax", &sf, &_ast_gen_parse_code_2), _ast_gen_parse_code_2);
  ASSERT(code != NULL);

  /* Check logic */
  ASSERT(strstr(code, "if (tmp > 100.500000) { free(ret); return ERANGE; }"));

  free(code);
  struct_fields_free(&sf);
  PASS();
}

TEST test_double_exclusive_max(void) { char * _ast_gen_parse_code_3; 
  struct StructFields sf;
  char *code;
  struct StructField *f;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "val", "number", NULL, NULL, NULL);

  f = &sf.fields[0];
  f->has_max = 1;
  f->max_val = 0.0;
  f->exclusive_max = 1;

  code = (gen_parse_code("SExcMax", &sf, &_ast_gen_parse_code_3), _ast_gen_parse_code_3);
  ASSERT(code != NULL);

  /* Check logic */
  ASSERT(strstr(code, "if (tmp >= 0.000000) { free(ret); return ERANGE; }"));

  free(code);
  struct_fields_free(&sf);
  PASS();
}

TEST test_min_and_max(void) { char * _ast_gen_parse_code_4; 
  struct StructFields sf;
  char *code;
  struct StructField *f;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "age", "integer", NULL, NULL, NULL);

  f = &sf.fields[0];
  f->has_min = 1;
  f->min_val = 0;
  f->has_max = 1;
  f->max_val = 120;

  code = (gen_parse_code("Person", &sf, &_ast_gen_parse_code_4), _ast_gen_parse_code_4);
  ASSERT(code != NULL);

  ASSERT(strstr(code, "if (tmp < 0.000000) { free(ret); return ERANGE; }"));
  ASSERT(strstr(code, "if (tmp > 120.000000) { free(ret); return ERANGE; }"));

  free(code);
  struct_fields_free(&sf);
  PASS();
}

TEST test_string_len_validation(void) { char * _ast_gen_parse_code_5; 
  struct StructFields sf;
  char *code;
  struct StructField *f;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "s", "string", NULL, NULL, NULL);

  f = &sf.fields[0];
  f->has_min_len = 1;
  f->min_len = 2;
  f->has_max_len = 1;
  f->max_len = 10;

  code = (gen_parse_code("StrLen", &sf, &_ast_gen_parse_code_5), _ast_gen_parse_code_5);
  ASSERT(code != NULL);

  ASSERT(strstr(code, "strlen(ret->s)"));
  ASSERT(strstr(code, "if (len < 2) { StrLen_cleanup(ret); return ERANGE; }"));
  ASSERT(strstr(code, "if (len > 10) { StrLen_cleanup(ret); return ERANGE; }"));

  free(code);
  struct_fields_free(&sf);
  PASS();
}

TEST test_string_simple_pattern_prefix(void) { char * _ast_gen_parse_code_6; 
  struct StructFields sf;
  char *code;
  struct StructField *f;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "p", "string", NULL, NULL, NULL);

  f = &sf.fields[0];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  strcpy_s(f->pattern, sizeof(f->pattern), "^prefix");
#else
#if defined(_MSC_VER)
  strcpy_s(f->pattern, sizeof(f->pattern), "^prefix");
#else
#if defined(_MSC_VER)
  strcpy_s(f->pattern, sizeof(f->pattern), "^prefix");
#else
  strcpy(f->pattern, "^prefix");
#endif
#endif
#endif

  code = (gen_parse_code("SPat", &sf, &_ast_gen_parse_code_6), _ast_gen_parse_code_6);
  ASSERT(code != NULL);

  ASSERT(strstr(code, "strncmp(ret->p, \"prefix\", 6) != 0"));

  free(code);
  struct_fields_free(&sf);
  PASS();
}

TEST test_string_simple_pattern_suffix(void) { char * _ast_gen_parse_code_7; 
  struct StructFields sf;
  char *code;
  struct StructField *f;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "p", "string", NULL, NULL, NULL);

  f = &sf.fields[0];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  strcpy_s(f->pattern, sizeof(f->pattern), "suffix$");
#else
#if defined(_MSC_VER)
  strcpy_s(f->pattern, sizeof(f->pattern), "suffix$");
#else
#if defined(_MSC_VER)
  strcpy_s(f->pattern, sizeof(f->pattern), "suffix$");
#else
  strcpy(f->pattern, "suffix$");
#endif
#endif
#endif

  code = (gen_parse_code("SSuf", &sf, &_ast_gen_parse_code_7), _ast_gen_parse_code_7);
  ASSERT(code != NULL);

  ASSERT(strstr(code, "strcmp(ret->p + len - 6, \"suffix\")"));

  free(code);
  struct_fields_free(&sf);
  PASS();
}

TEST test_string_simple_pattern_exact(void) { char * _ast_gen_parse_code_8; 
  struct StructFields sf;
  char *code;
  struct StructField *f;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "p", "string", NULL, NULL, NULL);

  f = &sf.fields[0];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  strcpy_s(f->pattern, sizeof(f->pattern), "^exact$");
#else
#if defined(_MSC_VER)
  strcpy_s(f->pattern, sizeof(f->pattern), "^exact$");
#else
#if defined(_MSC_VER)
  strcpy_s(f->pattern, sizeof(f->pattern), "^exact$");
#else
  strcpy(f->pattern, "^exact$");
#endif
#endif
#endif

  code = (gen_parse_code("SExact", &sf, &_ast_gen_parse_code_8), _ast_gen_parse_code_8);
  ASSERT(code != NULL);

  ASSERT(strstr(code, "strcmp(ret->p, \"exact\") != 0"));

  free(code);
  struct_fields_free(&sf);
  PASS();
}

TEST test_string_simple_pattern_contains(void) { char * _ast_gen_parse_code_9; 
  struct StructFields sf;
  char *code;
  struct StructField *f;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "p", "string", NULL, NULL, NULL);

  f = &sf.fields[0];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  strcpy_s(f->pattern, sizeof(f->pattern), "sub");
#else
#if defined(_MSC_VER)
  strcpy_s(f->pattern, sizeof(f->pattern), "sub");
#else
#if defined(_MSC_VER)
  strcpy_s(f->pattern, sizeof(f->pattern), "sub");
#else
  strcpy(f->pattern, "sub");
#endif
#endif
#endif

  code = (gen_parse_code("SSub", &sf, &_ast_gen_parse_code_9), _ast_gen_parse_code_9);
  ASSERT(code != NULL);

  ASSERT(strstr(code, "strstr(ret->p, \"sub\") == NULL"));

  free(code);
  struct_fields_free(&sf);
  PASS();
}

SUITE(codegen_validation_suite) {
  RUN_TEST(test_int_min_validation);
  RUN_TEST(test_int_exclusive_min);
  RUN_TEST(test_double_max_validation);
  RUN_TEST(test_double_exclusive_max);
  RUN_TEST(test_min_and_max);
  RUN_TEST(test_string_len_validation);
  RUN_TEST(test_string_simple_pattern_prefix);
  RUN_TEST(test_string_simple_pattern_suffix);
  RUN_TEST(test_string_simple_pattern_exact);
  RUN_TEST(test_string_simple_pattern_contains);
}

#endif /* TEST_CODEGEN_VALIDATION_H */
