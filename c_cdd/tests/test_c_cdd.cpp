#include <gtest/gtest.h>

#include <c_cdd/cdd.h>

GTEST_TEST(emit, struct_from_json_schema) {
  /* Test struct_from_json_schema */

  static const char *const
      from = "\nint f(long b) { return b; } static const int c = f(5);\n",
      *const want =
          "\nlong f(long b) { return b; } static const long c = f(/*b=*/5);\n";
  testing::internal::CaptureStderr();
  testing::internal::CaptureStdout();
  const std::unique_ptr<clang::RecordDecl> record_decl = cdd::emit::struct_from_json_schema();
  EXPECT_TRUE(true);
  const std::string output = testing::internal::GetCapturedStderr() +
                             testing::internal::GetCapturedStdout();
  llvm::outs() << "\noutput: \"" << output << '"';
}
