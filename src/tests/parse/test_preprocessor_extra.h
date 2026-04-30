static int abort_cb(const struct IncludeInfo *info, void *user_data) {
  struct TestPPCtx *ctx = (struct TestPPCtx *)user_data;
  ctx->count++;
  return 1; /* Abort on first */
}

TEST test_preprocessor_abort(void) {
  struct PPConfig config = {0};
  struct TestPPCtx ctx = {0};
  const char *source = "#include <stdio.h>\n#include <stdlib.h>";
  config.search_paths[0] = "include";
  config.num_search_paths = 1;
  config.sys_search_paths[0] = "sys_include";
  config.num_sys_search_paths = 1;

  cfs_mkdir("sys_include", 0777);
  cfs_write_file("sys_include/stdio.h", "", 0);
  cfs_write_file("sys_include/stdlib.h", "", 0);

  parse_includes_from_source(source, "main.c", &config, abort_cb, &ctx);

  ASSERT_EQ(1, ctx.count);

  cfs_remove("sys_include/stdio.h");
  cfs_remove("sys_include/stdlib.h");
  cfs_remove("sys_include");
  PASS();
}
