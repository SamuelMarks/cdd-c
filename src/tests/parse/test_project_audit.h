/* clang-format off */
#ifndef TEST_PROJECT_AUDIT_H
#define TEST_PROJECT_AUDIT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "functions/parse/audit.h"
#include "functions/parse/fs.h"
/* clang-format on */

TEST test_audit_stats_init(void) {
  struct AuditStats stats;
  /* Set to garbage */
  memset(&stats, 0xFF, sizeof(stats));
  audit_stats_init(&stats);
  ASSERT_EQ(0, stats.files_scanned);
  ASSERT_EQ(0, stats.allocations_checked);
  ASSERT_EQ(0, stats.functions_returning_alloc);
  ASSERT_EQ(0, stats.violations.size);
  ASSERT(stats.violations.items == NULL);
  audit_stats_free(&stats);
  PASS();
}

TEST test_audit_single_file(void) {
  char *sys_tmp = NULL;
  char *root = NULL;
  char *f_unchecked = NULL;
  struct AuditStats stats;
  int rc;

  /* Create explicit subdir to avoid walking /tmp */
  tempdir(&sys_tmp);
  asprintf(&root, "%s%saudit_test_%d", sys_tmp, PATH_SEP, rand());
  makedir(root);

  asprintf(&f_unchecked, "%s%sunchecked.c", root, PATH_SEP);

  /* Create file with 1 unchecked malloc and 1 checked malloc */
  /* Line 1: checked calloc */
  /* Line 2: unchecked malloc */
  write_to_file(f_unchecked,
                ""
                "void f() { char * q = (char *)calloc(1,1); if (!q) return; \n"
                " char * p = (char *)malloc(1); *p = 0; }");

  audit_stats_init(&stats);
  rc = audit_project(root, &stats);

  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, stats.files_scanned);
  ASSERT_EQ(1, stats.allocations_unchecked); /* p */
  ASSERT_EQ(1, stats.allocations_checked);   /* q */

  /* Verify detailed traces */
  ASSERT_EQ(1, stats.violations.size);
  ASSERT_STR_EQ(f_unchecked, stats.violations.items[0].file_path);
  ASSERT_EQ(2, stats.violations.items[0].line); /* Line logic check */
  ASSERT_STR_EQ("p", stats.violations.items[0].variable_name);
  ASSERT_STR_EQ("malloc", stats.violations.items[0].allocator_name);

  audit_stats_free(&stats);

  remove(f_unchecked);
  rmdir(root);
  free(f_unchecked);
  free(root);
  free(sys_tmp);
  PASS();
}

TEST test_audit_ignored_files(void) {
  char *sys_tmp = NULL;
  char *root = NULL;
  char *f_h = NULL;
  struct AuditStats stats;

  tempdir(&sys_tmp);
  asprintf(&root, "%s%saudit_test_ig_%d", sys_tmp, PATH_SEP, rand());
  makedir(root);

  asprintf(&f_h, "%s%signored.h", root, PATH_SEP);
  /* Header file logic currently ignored by audit_project default filter */
  write_to_file(f_h, ""
                     "void f() { char * p = (char *)malloc(1); }");

  audit_stats_init(&stats);
  audit_project(root, &stats);

  /* Should ignore .h files */
  ASSERT_EQ(0, stats.files_scanned);

  audit_stats_free(&stats);

  remove(f_h);
  rmdir(root);
  free(f_h);
  free(root);
  free(sys_tmp);
  PASS();
}

TEST test_audit_return_alloc(void) {
  char *sys_tmp = NULL;
  char *root = NULL;
  char *f_ret = NULL;
  struct AuditStats stats;

  tempdir(&sys_tmp);
  asprintf(&root, "%s%saudit_test_ret_%d", sys_tmp, PATH_SEP, rand());
  makedir(root);

  asprintf(&f_ret, "%s%sret.c", root, PATH_SEP);
  /* Detect return malloc(...) */
  write_to_file(f_ret, "char* f() { return malloc(10); }");

  audit_stats_init(&stats);
  audit_project(root, &stats);

  ASSERT_EQ(1, stats.files_scanned);
  ASSERT_EQ(1, stats.functions_returning_alloc);
  /* return malloc(...) is marked as Checked or ignored check logic depending on
     analysis? Actually find_allocations treats return stmt as alloc site.
     var_name is NULL for return statement.
     In current implementation, return statement allocs are added to sites but
     unchecked.
  */
  ASSERT_EQ(1, stats.allocations_unchecked);
  ASSERT_EQ(1, stats.violations.size);
  ASSERT(stats.violations.items[0].variable_name == NULL);

  audit_stats_free(&stats);

  remove(f_ret);
  rmdir(root);
  free(f_ret);
  free(root);
  free(sys_tmp);
  PASS();
}

TEST test_audit_json_output(void) {
  struct AuditStats stats;
  char *json;

  audit_stats_init(&stats);
  stats.files_scanned = 10;
  stats.allocations_checked = 20;
  stats.allocations_unchecked = 1;

  /* Manually inject a violation to test JSON serialization mechanics
   * independent of FS */
  /* Replicating add_violation logic manually or assume init is clean */
  stats.violations.items =
      (struct AuditViolation *)malloc(2 * sizeof(struct AuditViolation));
  stats.violations.capacity = 2;
  stats.violations.size = 2;
  stats.violations.items[0].file_path = strdup("test.c");
  stats.violations.items[0].line = 12;
  stats.violations.items[0].col = 4;
  stats.violations.items[0].variable_name = strdup("x");
  stats.violations.items[0].allocator_name = strdup("malloc");
  stats.violations.items[1].file_path = strdup("test2.c");
  stats.violations.items[1].line = 13;
  stats.violations.items[1].col = 5;
  stats.violations.items[1].variable_name = NULL;
  stats.violations.items[1].allocator_name = strdup("calloc");

  audit_print_json(&stats, &json);
  ASSERT(json != NULL);
  /* Check JSON structure */
  ASSERT(strstr(json, "\"files_scanned\": 10"));
  ASSERT(strstr(json, "\"allocations_unchecked\": 1"));

  /* Check violations array */
  ASSERT(strstr(json, "\"violations\": ["));
  ASSERT(strstr(json, "\"file\": \"test.c\""));
  ASSERT(strstr(json, "\"line\": 12"));
  ASSERT(strstr(json, "\"variable\": \"x\""));

  free(json);
  audit_stats_free(&stats);
  PASS();
}

TEST test_audit_stats_null(void) {
  struct AuditStats stats;
  char *_test_json = (char *)1;
  audit_stats_init(NULL); /* Should do nothing safely */
  audit_stats_free(NULL); /* Should return safely */

  ASSERT_EQ(EINVAL, audit_project(NULL, &stats));
  ASSERT_EQ(EINVAL, audit_project("dummy", NULL));
  audit_print_json(NULL, &_test_json);
  ASSERT(_test_json == NULL);

  PASS();
}

TEST test_audit_edge_cases(void) {
  char *sys_tmp = NULL;
  char *root = NULL;
  char *f_noext = NULL;
  char *d_dir_c = NULL;
  char *f_strndup = NULL;
  char *f_bad_token = NULL;
  char *f_unreadable = NULL;
  struct AuditStats stats;

  tempdir(&sys_tmp);
  asprintf(&root, "%s%saudit_edge_%d", sys_tmp, PATH_SEP, rand());
  makedir(root);

  /* No extension file */
  asprintf(&f_noext, "%s%snoextension", root, PATH_SEP);
  write_to_file(f_noext, "int x = 0;");

  /* Directory ending in .c */
  asprintf(&d_dir_c, "%s%sdir.c", root, PATH_SEP);
  makedir(d_dir_c);

  /* return strndup */
  asprintf(&f_strndup, "%s%sstrndup_test.c", root, PATH_SEP);
  write_to_file(f_strndup, "char* f() { return strndup(\"a\", 1); }");

  /* Unreadable file */
  asprintf(&f_unreadable, "%s%sunreadable.c", root, PATH_SEP);
  write_to_file(f_unreadable, "int unreadable = 1;");
  chmod(f_unreadable, 0000);

  /* Tokenize failure: Unclosed string literal. */
  asprintf(&f_bad_token, "%s%sbad.c", root, PATH_SEP);
  write_to_file(f_bad_token, "char *s = \"unclosed");

  audit_stats_init(&stats);
  audit_project(root, &stats);

  /* strndup should be counted in functions_returning_alloc */
  ASSERT(stats.functions_returning_alloc >= 1);

  audit_stats_free(&stats);

  remove(f_noext);
  rmdir(d_dir_c);
  remove(f_strndup);

  chmod(f_unreadable, 0644);
  remove(f_unreadable);
  free(f_unreadable);
  remove(f_bad_token);
  rmdir(root);
  free(f_noext);
  free(d_dir_c);
  free(f_strndup);
  free(f_bad_token);
  free(root);
  free(sys_tmp);
  PASS();
}

SUITE(project_audit_suite) {
  RUN_TEST(test_audit_stats_null);
  RUN_TEST(test_audit_edge_cases);
  RUN_TEST(test_audit_stats_init);
  RUN_TEST(test_audit_single_file);
  RUN_TEST(test_audit_ignored_files);
  RUN_TEST(test_audit_return_alloc);
  RUN_TEST(test_audit_json_output);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_PROJECT_AUDIT_H */
