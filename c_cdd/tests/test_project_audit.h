#ifndef TEST_PROJECT_AUDIT_H
#define TEST_PROJECT_AUDIT_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "fs.h"
#include "project_audit.h"

TEST test_audit_stats_init(void) {
  struct AuditStats stats;
  /* Set to garbage */
  memset(&stats, 0xFF, sizeof(stats));
  audit_stats_init(&stats);
  ASSERT_EQ(0, stats.files_scanned);
  ASSERT_EQ(0, stats.allocations_checked);
  ASSERT_EQ(0, stats.functions_returning_alloc);
  PASS();
}

TEST test_audit_single_file(void) {
  char *root = NULL;
  char *f_unchecked = NULL;
  struct AuditStats stats;
  int rc;

  tempdir(&root);
  asprintf(&f_unchecked, "%s%sunchecked.c", root, PATH_SEP);

  /* Create file with 1 unchecked malloc and 1 checked malloc */
  write_to_file(f_unchecked, "void f() { char *p = malloc(1); *p = 0; "
                             "char *q = calloc(1,1); if (!q) return; }");

  audit_stats_init(&stats);
  rc = audit_project(root, &stats);

  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, stats.files_scanned);
  ASSERT_EQ(1, stats.allocations_unchecked); /* p */
  ASSERT_EQ(1, stats.allocations_checked);   /* q */

  remove(f_unchecked);
  rmdir(root);
  free(f_unchecked);
  free(root);
  PASS();
}

TEST test_audit_ignored_files(void) {
  char *root = NULL;
  char *f_h = NULL;
  struct AuditStats stats;

  tempdir(&root);
  asprintf(&f_h, "%s%signored.h", root, PATH_SEP);
  /* Header file logic currently ignored by audit_project default filter */
  write_to_file(f_h, "void f() { char *p = malloc(1); }");

  audit_stats_init(&stats);
  audit_project(root, &stats);

  ASSERT_EQ(0, stats.files_scanned);

  remove(f_h);
  rmdir(root);
  free(f_h);
  free(root);
  PASS();
}

TEST test_audit_return_alloc(void) {
  char *root = NULL;
  char *f_ret = NULL;
  struct AuditStats stats;

  tempdir(&root);
  asprintf(&f_ret, "%s%sret.c", root, PATH_SEP);
  /* Detect return malloc(...) */
  write_to_file(f_ret, "char* f() { return malloc(10); }");

  audit_stats_init(&stats);
  audit_project(root, &stats);

  ASSERT_EQ(1, stats.files_scanned);
  ASSERT_EQ(1, stats.functions_returning_alloc);

  remove(f_ret);
  rmdir(root);
  free(f_ret);
  free(root);
  PASS();
}

TEST test_audit_json_output(void) {
  struct AuditStats stats;
  char *json;

  audit_stats_init(&stats);
  stats.files_scanned = 10;
  stats.allocations_unchecked = 5;

  json = audit_print_json(&stats);
  ASSERT(json != NULL);
  /* Basic check string contains keys */
  ASSERT(strstr(json, "\"files_scanned\": 10"));
  ASSERT(strstr(json, "\"allocations_unchecked\": 5"));

  free(json);
  PASS();
}

SUITE(project_audit_suite) {
  RUN_TEST(test_audit_stats_init);
  RUN_TEST(test_audit_single_file);
  RUN_TEST(test_audit_ignored_files);
  RUN_TEST(test_audit_return_alloc);
  RUN_TEST(test_audit_json_output);
}

#endif /* !TEST_PROJECT_AUDIT_H */
