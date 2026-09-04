#ifndef TEST_ORCHESTRATOR_COVERAGE_H
#define TEST_ORCHESTRATOR_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "functions/parse/orchestrator.h"
#include <greatest.h>
/* clang-format on */

/* Moved extern declarations for C89 compliance */
extern C_CDD_EXPORT int g_cdd_strdup_fail;

extern C_CDD_EXPORT cdd_c_error_t fix_code_main(int argc, char **argv);

TEST test_orchestrator_coverage_fix_code_main(void) {
  char *argv_missing[] = {(char *)(size_t)"does_not_exist_dir",
                          (char *)(size_t)"--in-place"};
  char *argv_missing2[] = {(char *)(size_t)"does_not_exist.c",
                           (char *)(size_t)"out.c"};

  /* Will fail to walk directory */
  int rc_2_argv_missing = fix_code_main(2, argv_missing);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc_2_argv_missing);

  /* Missing input file */
  {
    int rc_2_argv_missing2 = fix_code_main(2, argv_missing2);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc_2_argv_missing2);

    PASS();
  }
}

TEST test_orchestrator_coverage_oom(void) {
  char *out = NULL;
  /*  (moved to global) */

  int i;

  for (i = 1; i < 20; ++i) {
    g_cdd_alloc_fail = i;
    orchestrate_fix("void test() { char *p = malloc(1); }", &out);
    C_CDD_FREE(out);
    out = NULL;
    g_cdd_alloc_fail = 0;
  }

  PASS();
}

TEST test_orchestrator_coverage_oom_deep(void) {
  char *out = NULL;
  /*  (moved to global) */
  /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */

  int i;
  const char *code = "void test() { char *p = malloc(1); }\n"
                     "void test2() { char *p = malloc(1); }\n";

  for (i = 1; i < 50; ++i) {
    g_cdd_alloc_fail = i;
    orchestrate_fix(code, &out);
    C_CDD_FREE(out);
    out = NULL;
    g_cdd_alloc_fail = 0;

    g_cdd_strdup_fail = i;
    orchestrate_fix(code, &out);
    C_CDD_FREE(out);
    out = NULL;
    g_cdd_strdup_fail = 0;
  }

  PASS();
}

TEST test_orchestrator_coverage_fix_file(void) {
  FILE *f;
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_empty.txt", "w") != 0)
    f = NULL;
#else
  f = fopen("test_empty.txt", "w");
#endif
  if (f)
    if (f)
      fclose(f);
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_empty.c", "w") != 0)
    f = NULL;
#else
  f = fopen("test_empty.c", "w");
#endif
  if (f) {
    fputs("int main(){}", f);
    if (f)
      fclose(f);
  }
  makedir("my_empty_dir");
  {
    char *argv_txt[] = {(char *)(size_t)"test_empty.txt",
                        (char *)(size_t)"out.txt"};
    char *argv_c[] = {(char *)(size_t)"test_empty.c", (char *)(size_t)"out.c"};
    char *argv_dir[] = {(char *)(size_t)"my_empty_dir",
                        (char *)(size_t)"--in-place"};

    int rc_txt = fix_code_main(2, argv_txt);
    ASSERT_EQ(EXIT_SUCCESS, rc_txt);

    /* c file -> is_c_source true -> parses -> valid c file, refactors, success
     */
    {
      int rc_c = fix_code_main(2, argv_c);
      ASSERT_EQ(EXIT_SUCCESS, rc_c);

      /* directory -> parses all c files, refactors, success */
      {
        int rc_dir = fix_code_main(2, argv_dir);
        ASSERT_EQ(EXIT_SUCCESS, rc_dir);
        PASS();
      }
    }
  }
}
TEST test_orchestrator_coverage_fix_dir_no_inplace_1arg(void) {
  char *argv[] = {(char *)(size_t)"my_empty_dir"};
  int rc = fix_code_main(1, argv);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  PASS();
}

TEST test_orchestrator_coverage_fix_file_1arg(void) {
  /* Single missing file implicitly */
  char *argv_single[] = {(char *)(size_t)"does_not_exist.c"};
  int rc_1_argv_single = fix_code_main(1, argv_single);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc_1_argv_single);

  PASS();
}

TEST test_orchestrator_coverage_fix_dir_errors(void) {
  char *argv_argc0[] = {
      (char *)(size_t)"dummy"}; /* argv isn't used for argc 0 */
  char *argv_argc3[] = {(char *)(size_t)"dir", (char *)(size_t)"out1",
                        (char *)(size_t)"out2"};

  /* Invalid argc */
  int rc_0_argv_argc0 = fix_code_main(0, argv_argc0);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc_0_argv_argc0);
  {
    int rc_3_argv_argc3 = fix_code_main(3, argv_argc3);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc_3_argv_argc3);

    PASS();
  }
}

TEST test_orchestrator_coverage_fix_file_failures(void) {
  char *argv_c[] = {(char *)(size_t)"test_empty.c", (char *)(size_t)"out.c"};
  /*  (moved to global) */
  int rc;

  /* Trigger orchestrate_fix failure inside fix_file_callback */
  g_cdd_alloc_fail = 1;
  rc = fix_code_main(2, argv_c);
  g_cdd_alloc_fail = 0;
  (void)rc;
  printf("DEBUG: fix_file_failures rc=%d\n", rc);
  ASSERT_EQ((int)EXIT_FAILURE, rc);

  PASS();
}

TEST test_orchestrator_coverage_fix_file_failures_2(void) {
  char *argv_c[] = {(char *)(size_t)"test_empty.c", (char *)(size_t)"out.c"};
  /*  (moved to global) */
  int rc;

  /* Trigger orchestrate_fix failure inside fix_file_callback by skipping first
   * alloc */
  g_cdd_alloc_fail = 2;
  rc = fix_code_main(2, argv_c);
  g_cdd_alloc_fail = 0;

  (void)rc;
  ASSERT_EQ((int)EXIT_FAILURE, rc);

  PASS();
}

TEST test_orchestrator_coverage_fix_file_write_fail(void) {
  char *argv_c[] = {(char *)(size_t)"test_empty.c",
                    (char *)(size_t)"my_empty_dir/unwritable/out.c"};
  int rc = fix_code_main(2, argv_c);
  ASSERT_EQ((int)EXIT_FAILURE, rc);
  PASS();
}
SUITE(orchestrator_coverage_suite) {
  RUN_TEST(test_orchestrator_coverage_fix_code_main);
  RUN_TEST(test_orchestrator_coverage_oom);
  RUN_TEST(test_orchestrator_coverage_fix_file);
  RUN_TEST(test_orchestrator_coverage_oom_deep);
  RUN_TEST(test_orchestrator_coverage_fix_dir_no_inplace_1arg);
  RUN_TEST(test_orchestrator_coverage_fix_file_1arg);
  RUN_TEST(test_orchestrator_coverage_fix_dir_errors);
  RUN_TEST(test_orchestrator_coverage_fix_file_failures);
  RUN_TEST(test_orchestrator_coverage_fix_file_failures_2);
  RUN_TEST(test_orchestrator_coverage_fix_file_write_fail);
}

#ifdef __cplusplus
}
#endif
#endif
