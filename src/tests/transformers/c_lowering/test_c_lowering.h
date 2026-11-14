/**
 * @file test_c_lowering.h
 * @brief Unit and OOM tests for C99/C11/C23 to Legacy & Modern MSVC CST
 * lowering passes.
 */
#ifndef TEST_CDD_TRANSFORM_C_LOWERING_H
#define TEST_CDD_TRANSFORM_C_LOWERING_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdlib.h>
#include <string.h>
#include "c_str_span.h"
#include "cdd_cst_transform.h"
#include "classes/emit/cdd_cst_emit.h"
#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_type_eval.h"
#include "transformers/c_lowering/c_lowering_internal.h"
/* clang-format on */

/**
 * @brief Golden regression test for Phase 3.1 Modern & Legacy MSVC passes
 * (VLAs + hybrid mode + `sizeof(vla)`, array param qualifiers `[static N]` /
 * `[restrict N]`, `_Complex`, `_Atomic`, `_Thread_local`, `restrict`, and C23
 * `[[...]]` / `nullptr` / `constexpr` / `1'000` / `<stdckdint.h>`).
 * @return Greatest test result.
 */
TEST test_c_lowering_phase_3_1_modern_and_vla_hybrid(void) {
  cdd_cst_tree_t *tree = NULL;
  char *out = NULL;
  cdd_transform_config_t cfg;
  const char *src1 =
      "#include <complex.h>\n"
      "#include <stdckdint.h>\n"
      "_Thread_local int g_tls = 0;\n"
      "constexpr int k_cap = 1'000;\n"
      "_Atomic(int) g_counter = 0;\n"
      "[[nodiscard]] int proc_arr(int rows, int cols, int buf[static 16],\n"
      "                           char p[restrict 8], int vla_p[rows]) {\n";
  const char *src2 = "  void *np = nullptr;\n"
                     "  int mat[rows][cols + 1];\n"
                     "  size_t msz = sizeof(mat);\n"
                     "  double _Complex z1 = 2.0i;\n"
                     "  double _Complex z2 = z1 + 3.0;\n"
                     "  int ov_out = 0;\n"
                     "  int ov = ckd_add(&ov_out, rows, cols);\n"
                     "  g_counter += 2;\n"
                     "  g_counter++;\n"
                     "  if (np == nullptr && creal(z2) > 0.0) {\n"
                     "    return atomic_load ? (int)msz + ov : g_counter;\n"
                     "  }\n"
                     "  return g_counter;\n"
                     "}\n";
  char full_src[1536];
  CDD_SNPRINTF(full_src, sizeof(full_src), "%s%s", src1, src2);

  memset(&cfg, 0, sizeof(cfg));
  cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;
  cfg.vla_mode = CDD_VLA_MODE_HYBRID;

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str(full_src), &tree));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "__declspec(thread) int g_tls") != NULL);
  ASSERT(strstr(out, "const int k_cap = 1000;") != NULL);
  ASSERT(strstr(out, "volatile int g_counter") != NULL);
  ASSERT(strstr(out, "_Check_return_ int proc_arr") != NULL);
  ASSERT(strstr(out, "int *buf") != NULL);
  ASSERT(strstr(out, "char * __restrict p") != NULL);
  ASSERT(strstr(out, "int *vla_p") != NULL);
  ASSERT(strstr(out, "((void *)0)") != NULL);
  ASSERT(strstr(out, "__cdd_vla_hp_mat") != NULL);
  ASSERT(strstr(out, "((rows) * (cols + 1)) * sizeof(*mat)") != NULL);
  ASSERT(strstr(out, "_Dcomplex") != NULL);
  ASSERT(strstr(out, "z1 = awm_cmake(0.0, (double)(2.0))") != NULL);
  ASSERT(strstr(out, "awm_cadd(") != NULL);
  ASSERT(strstr(out, "awm_creal(") != NULL);
  ASSERT(strstr(out, "__cdd_ckd_add(") != NULL);
  ASSERT(strstr(out, "atomic_fetch_add(&(g_counter), (2))") != NULL);
  ASSERT(strstr(out, "atomic_fetch_add(&(g_counter), 1)") != NULL);
  ASSERT(strstr(out, "atomic_load(&(g_counter))") != NULL);
  ASSERT(strstr(out, "if (__cdd_vla_hp_mat) free(mat);") != NULL);

  free(out);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
  PASS();
}

/**
 * @brief Golden regression test for Phase 3.2 Legacy MSVC (`2005-2017` / C89)
 * passes (mixed declarations & `for`-init hoisting with collision renaming,
 * designated struct & array initializers, compound literals, legacy keywords,
 * `UI64`/`I64` suffixes, `//` line comments, and `%zu`/`%lld` printf strings).
 * @return Greatest test result.
 */
TEST test_c_lowering_phase_3_2_legacy_msvc_c89(void) {
  cdd_cst_tree_t *tree = NULL;
  char *out = NULL;
  cdd_transform_config_t cfg;
  const char *src1 = "// Top-level C99 line comment\n"
                     "struct Vec2 { int x; int y; };\n"
                     "_Static_assert(sizeof(int) >= 4, \"int32\");\n"
                     "static struct Vec2 g_v = { .y = 20, .x = 10 };\n"
                     "static int g_tbl[5] = { [1] = 11, [3] = 33 };\n"
                     "_Noreturn void fatal_stop(void);\n";
  const char *src2 = "static inline int legacy_fn(int n) {\n"
                     "  int acc = 1;\n"
                     "  acc += 2; // mid-line C99 comment\n"
                     "  _Alignas(16) int mid_x = n + 5;\n"
                     "  unsigned long long big = 42ULL;\n"
                     "  struct Vec2 cl_v = (struct Vec2){ .y = 9, .x = 7 };\n"
                     "  int arr_d[4] = { [2] = mid_x };\n"
                     "  for (int i = 0; i < 3; i++) { acc += i; }\n"
                     "  for (int i = 1; i < 4; i++) { acc += i; }\n"
                     "  const char *fn = __func__;\n"
                     "  size_t al = _Alignof(double);\n"
                     "  printf(\"%zu %lld %s\\n\", al, (long long)big, fn);\n"
                     "  return acc + cl_v.x + arr_d[2] + g_v.x + g_tbl[1];\n"
                     "}\n";
  char full_src[2048];
  CDD_SNPRINTF(full_src, sizeof(full_src), "%s%s", src1, src2);

  memset(&cfg, 0, sizeof(cfg));
  cfg.target_profile = CDD_TARGET_PROFILE_MSVC_LEGACY;
  cfg.msvc_ver = 1310;

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str(full_src), &tree));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tree, &out));

  /* Verify zero remaining `//` comments */
  ASSERT(strstr(out, "//") == NULL);
  ASSERT(strstr(out, "/*  Top-level C99 line comment */") != NULL ||
         strstr(out, "/* Top-level C99 line comment */") != NULL);
  /* Static assert & _Noreturn & inline */
  ASSERT(strstr(out, "typedef char __cdd_sa_1[(sizeof(int) >= 4) ? 1 : -1]") !=
         NULL);
  ASSERT(strstr(out, "__declspec(noreturn) void fatal_stop") != NULL);
  ASSERT(strstr(out, "static __inline int legacy_fn") != NULL);
  /* Ordered C89 positional static designated initializers */
  ASSERT(strstr(out, "static struct Vec2 g_v = { 10, 20 }") != NULL);
  ASSERT(strstr(out, "static int g_tbl[5] = { 0, 11, 0, 33 }") != NULL);
  /* Compound literal, collision-safe `for` hoisting, mid-block decl hoisting */
  ASSERT(strstr(out, "__cdd_cl_1") != NULL);
  ASSERT(strstr(out, "__cdd_for_i_1") != NULL);
  ASSERT(strstr(out, "int mid_x;") != NULL);
  ASSERT(strstr(out, "mid_x = n + 5;") != NULL);
  ASSERT(strstr(out, "arr_d[2] = mid_x;") != NULL);
  /* Legacy literals, keywords & printf format specifiers */
  ASSERT(strstr(out, "42UI64") != NULL);
  ASSERT(strstr(out, "\"legacy_fn\"") != NULL);
  ASSERT(strstr(out, "__alignof(double)") != NULL);
  ASSERT(strstr(out, "\"%Iu %I64d %s\\n\"") != NULL);

  free(out);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
  PASS();
}

/**
 * @brief Golden regression test for Phase 3.3 `msvc_port`
 * (`--with-auto-win-msvc`, 2-arg `mkdir` vs 1-arg `setmode` vs 2-arg `setmode`,
 * `--rename-main`, empty TU `typedef int __awm_nonempty_tu;`) and `safe_crt`
 * (`CDD_SAFE_CRT_MSVC_ONLY`).
 * @return Greatest test result.
 */
TEST test_c_lowering_phase_3_3_interop(void) {
  cdd_cst_tree_t *tree = NULL;
  char *out = NULL;
  cdd_transform_config_t cfg;

  /* 1. `--with-auto-win-msvc` + `--rename-main` + 1-arg BSD `setmode` */
  {
    const char *awm_src = "#include <unistd.h>\n"
                          "int main(int argc, char **argv) {\n"
                          "  void *m = setmode(\"0755\");\n"
                          "  mkdir(\"/tmp/d\", 0755);\n"
                          "  read(0, m, 1);\n"
                          "  return argc;\n"
                          "}\n";
    memset(&cfg, 0, sizeof(cfg));
    cfg.with_auto_win_msvc = 1;
    cfg.rename_main = "awm_tool_main";
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(awm_src), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_msvc(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tree, &out));
    ASSERT(strstr(out, "#ifndef _MSC_VER") == NULL);
    ASSERT(strstr(out, "int awm_tool_main(int argc") != NULL);
    ASSERT(strstr(out, "bsd_setmode(\"0755\")") != NULL);
    ASSERT(strstr(out, "mkdir(\"/tmp/d\", 0755)") != NULL);
    ASSERT(strstr(out, "read(0, m, 1)") != NULL);
    free(out);
    out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  /* 2. Without `auto-win-msvc`: 2-arg `mkdir(p, m)` -> `_mkdir(p)` and 2-arg
     `setmode(fd, m)` -> `_setmode(fd, m)` */
  {
    const char *raw_src = "void fs_ops(int fd) {\n"
                          "  mkdir(\"dir\", 0755);\n"
                          "  setmode(fd, 0x8000);\n"
                          "}\n";
    memset(&cfg, 0, sizeof(cfg));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(raw_src), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_msvc(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tree, &out));
    ASSERT(strstr(out, "_mkdir(\"dir\")") != NULL);
    ASSERT(strstr(out, "_setmode(fd, 0x8000)") != NULL);
    free(out);
    out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  /* 3. Empty TU `typedef int __awm_nonempty_tu;` injection */
  {
    const char *empty_src = "/* only comment in translation unit */\n";
    memset(&cfg, 0, sizeof(cfg));
    cfg.with_auto_win_msvc = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(empty_src), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_msvc(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tree, &out));
    ASSERT(strstr(out, "typedef int __awm_nonempty_tu;") != NULL);
    free(out);
    out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  /* 4. `safe_crt` `--safe-crt=msvc-only` (`CDD_SAFE_CRT_MSVC_ONLY`) */
  {
    const char *sc_src = "void copy_s(void) {\n"
                         "  char buf[32];\n"
                         "  strcpy(buf, \"abc\");\n"
                         "}\n";
    memset(&cfg, 0, sizeof(cfg));
    cfg.safe_crt_mode = CDD_SAFE_CRT_MSVC_ONLY;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(sc_src), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_safe_crt(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tree, &out));
    ASSERT(strstr(out, "strcpy_s(") != NULL);
    ASSERT(strstr(out, "#ifdef _MSC_VER") == NULL);
    ASSERT(strstr(out, "#else") == NULL);
    free(out);
    out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  PASS();
}

/**
 * @brief Error-percolation and OOM fault-injection suite for
 * `cdd_transform_c_lowering`.
 * @return Greatest test result.
 */
TEST test_c_lowering_errors_and_oom(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_transform_config_t cfg;
  int fail_at;
  const char *oom_src = "struct P { int a; int b; };\n"
                        "int f(int n, int arr[static 4]) {\n"
                        "  int v[n + 1];\n"
                        "  _Atomic int ac = 0;\n"
                        "  double _Complex z = 1.0i;\n"
                        "  ac += 1;\n"
                        "  struct P p = (struct P){ .b = 2, .a = 1 };\n"
                        "  for (int i = 0; i < 2; i++) { p.a += i; }\n"
                        "  printf(\"%zu\\n\", sizeof(v));\n"
                        "  return p.a + (int)creal(z) + ac + arr[0];\n"
                        "}\n";

  memset(&cfg, 0, sizeof(cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_transform_c_lowering(NULL, &cfg));
  cfg.msvc_ver = -1;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str("int x;\n"), &tree));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_transform_c_lowering(tree, &cfg));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
  tree = NULL;

#ifdef CDD_BUILD_TESTS
  cfg.msvc_ver = 1310;
  cfg.target_profile = CDD_TARGET_PROFILE_MSVC_LEGACY;
  cfg.vla_mode = CDD_VLA_MODE_HYBRID;
  for (fail_at = 1; fail_at <= 60; fail_at++) {
    cdd_c_error_t rc;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(oom_src), &tree));
    g_cdd_alloc_fail = fail_at;
    rc = cdd_transform_c_lowering(tree, &cfg);
    g_cdd_alloc_fail = 0;
    ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_MEMORY);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }
#endif
  PASS();
}

/**
 * @brief Exhaustive branch test covering `float _Complex`, all `_Atomic` ops,
 * VLA `malloc`/`alloca` + loop `break`/`continue`, union designators, array &
 * file-scope compound literals, and full `cdd_transform_pipeline` under strict
 * ISO C89.
 * @return Greatest test result.
 */
TEST test_c_lowering_branches_and_pipeline_c89(void) {
  cdd_cst_tree_t *tree = NULL;
  char *out = NULL;
  cdd_transform_config_t cfg;
  const char *src1 =
      "struct Item { int k; int v; };\n"
      "union Val { int i; double d; };\n"
      "static struct Item *g_item_ptr = &(struct Item){ 3, 4 };\n"
      "float _Complex cf_ops(float _Complex a, float _Complex b) {\n"
      "  float _Complex c = a - b;\n"
      "  float _Complex d = c * 2.0fi;\n"
      "  float _Complex q = d / a;\n"
      "  return (q == c || q != a) ? conjf(q) : awm_cmakef(cimagf(q), 0.0f);\n"
      "}\n";
  const char *src2 =
      "int full_pipeline_fn(int n, const int ro[const 4], int dyn_a[*]) {\n"
      "  _Atomic int st = 0;\n"
      "  ++st;\n"
      "  --st;\n"
      "  st--;\n"
      "  st -= 1;\n"
      "  st |= 2;\n"
      "  st &= 3;\n"
      "  st ^= 1;\n"
      "  st = 9;\n"
      "  for (int k = 0; k < n; k++) {\n"
      "    int vbuf[n + 2];\n"
      "    if (k == 1) continue;\n"
      "    if (k == 2) break;\n"
      "    vbuf[0] = ro[0] + dyn_a[0];\n"
      "  }\n"
      "  union Val u = { .d = 1.5 };\n"
      "  int *arr_cl = (int[]){ 7, 8 };\n"
      "  int ex = ({ int inner = 4; inner + 1; });\n"
      "  return st + (int)u.d + arr_cl[0] + ex + g_item_ptr->k;\n"
      "}\n";
  char full_src[2048];
  CDD_SNPRINTF(full_src, sizeof(full_src), "%s%s", src1, src2);

  memset(&cfg, 0, sizeof(cfg));
  cfg.target_profile = CDD_TARGET_PROFILE_ISO_C89;
  cfg.vla_mode = CDD_VLA_MODE_MALLOC;

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str(full_src), &tree));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_pipeline(
                               tree, CDD_PASS_GNU | CDD_PASS_C_LOWERING, &cfg));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "static struct Item __cdd_cl_1") != NULL);
  ASSERT(strstr(out, "_Fcomplex") != NULL);
  ASSERT(strstr(out, "awm_csubf(") != NULL);
  ASSERT(strstr(out, "awm_cmulf(") != NULL);
  ASSERT(strstr(out, "awm_cdivf(") != NULL);
  ASSERT(strstr(out, "awm_ceqf(") != NULL);
  ASSERT(strstr(out, "const int * const ro") != NULL);
  ASSERT(strstr(out, "int *dyn_a") != NULL);
  ASSERT(strstr(out, "atomic_fetch_sub(") != NULL);
  ASSERT(strstr(out, "atomic_fetch_or(") != NULL);
  ASSERT(strstr(out, "atomic_fetch_and(") != NULL);
  ASSERT(strstr(out, "atomic_fetch_xor(") != NULL);
  ASSERT(strstr(out, "atomic_store(") != NULL);
  ASSERT(strstr(out, "{ free(vbuf); continue; }") != NULL);
  ASSERT(strstr(out, "{ free(vbuf); break; }") != NULL);
  ASSERT(strstr(out, "u.d = 1.5;") != NULL);
  ASSERT(strstr(out, "__cdd_cl_2[0] = 7;") != NULL);
  ASSERT(strstr(out, "__cdd_stmt_expr_") != NULL);

  free(out);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
  PASS();
}

/**
 * @brief Golden CST regression test covering all remaining Legacy MSVC, Modern
 * MSVC, Watcom, and ISO C89 branches in `c_lowering_modern.c` and
 * `c_lowering_legacy.c`.
 * @return Greatest test result.
 */
TEST test_c_lowering_golden_exhaustive_legacy_modern(void) {
  cdd_cst_tree_t *tree = NULL;
  char *out = NULL;
  cdd_transform_config_t cfg;

  /* 1. Modern MSVC: scoped C23 attributes, enum/sizeof non-VLA, void-return &
        hybrid loop VLA unwind, long double complex, _Complex_I, scalar + z */
  {
    const char *m1 =
        "#include <complex.h>\n"
        "enum { K_DIM = 4 };\n"
        "thread_local int g_t1 = 0;\n"
        "__thread int g_t2 = 0;\n"
        "[[gnu::noreturn]] void halt_now(void);\n"
        "[[__deprecated__]] int old_api(int * restrict p) { return *p; }\n";
    const char *m2 =
        "void vla_void_fn(int n) {\n"
        "  int fixed_b[sizeof(int) + K_DIM];\n"
        "  [[maybe_unused]] int v_loc[n + 1];\n"
        "  fixed_b[0] = 1;\n"
        "  if (n <= 0) return;\n"
        "  for (int i = 0; i < n; i++) {\n"
        "    int hv[n + 2];\n"
        "    hv[0] = i;\n"
        "    switch (i) { case 0: [[fallthrough]]; default: break; }\n"
        "    if (i == 1) continue;\n"
        "    if (i == 2) break;\n"
        "  }\n"
        "}\n";
    const char *m3 =
        "double complex cplx_more(long double complex ld, float _Complex fz) "
        "{\n"
        "  double complex z = _Complex_I;\n"
        "  double complex s = 3.0 + z;\n"
        "  int ov = 0;\n"
        "  int r = ckd_sub(&ov, 5, 2) + ckd_mul(&ov, 3, 4);\n"
        "  return (cimag(s) > (double)crealf(fz) + (double)r) ? conj(s) : z;\n"
        "}\n";
    char m_src[2048];
    CDD_SNPRINTF(m_src, sizeof(m_src), "%s%s%s", m1, m2, m3);

    memset(&cfg, 0, sizeof(cfg));
    cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;
    cfg.vla_mode = CDD_VLA_MODE_HYBRID;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(m_src), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tree, &out));
    ASSERT(strstr(out, "__declspec(noreturn) void halt_now") != NULL);
    ASSERT(strstr(out, "__declspec(deprecated) int old_api") != NULL);
    ASSERT(strstr(out, "int * __restrict p") != NULL);
    ASSERT(strstr(out, "fixed_b[sizeof(int) + K_DIM]") != NULL);
    ASSERT(strstr(out, "awm_cimag(") != NULL);
    ASSERT(strstr(out, "awm_crealf(") != NULL);
    ASSERT(strstr(out, "awm_conj(") != NULL);
    ASSERT(strstr(out, "__cdd_ckd_sub(") != NULL);
    ASSERT(strstr(out, "__cdd_ckd_mul(") != NULL);
    free(out);
    out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  /* 2. Legacy MSVC + ISO C99 + Watcom: mixed positional/designated inits,
        multi-var for hoisting, all printf format specifiers, LL -> I64 */
  {
    const char *l1 = "enum { E_TWO = 2 };\n"
                     "typedef int my_item_t;\n"
                     "struct Pair3 { int a; const void *p; int c; };\n"
                     "static struct Pair3 g_p3 = { 1, .c = sizeof(my_item_t) + "
                     "E_TWO, .p = NULL };\n"
                     "static int g_mix[4] = { 9, [2] = E_TWO };\n";
    const char *l2 =
        "int leg_more(int n) {\n"
        "  long long s64 = 12LL;\n"
        "  int plain = 1;\n"
        "  plain++;\n"
        "  struct Pair3 d_p = { n, .c = n + 1 };\n"
        "  int d_arr[4] = { 5, [2] = n };\n"
        "  for (int i = 0, j = 3; i < j; i++) { plain += i; }\n"
        "  printf(\"plain %s\\n\", \"ok\");\n"
        "  printf(\"100%% \\\" %08zx %-4td %ti %llu %llX %lli %jd %ji\\n\",\n"
        "         (size_t)1, (ptrdiff_t)2, (ptrdiff_t)3, 4ULL, 5ULL, 6LL, 7LL, "
        "8LL);\n"
        "  return plain + d_p.c + d_arr[0] + g_p3.a + g_mix[0] + (int)s64;\n"
        "}\n";
    char l_src[2048];
    CDD_SNPRINTF(l_src, sizeof(l_src), "%s%s", l1, l2);

    memset(&cfg, 0, sizeof(cfg));
    cfg.target_profile = CDD_TARGET_PROFILE_MSVC_LEGACY;
    cfg.msvc_ver = 1310;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(l_src), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tree, &out));
    ASSERT(strstr(out, "12I64") != NULL);
    ASSERT(strstr(out, "%08Ix") != NULL);
    ASSERT(strstr(out, "%-4Id") != NULL);
    ASSERT(strstr(out, "%I64u") != NULL);
    ASSERT(strstr(out, "%I64X") != NULL);
    ASSERT(strstr(out, "%I64d") != NULL);
    free(out);
    out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  /* 3. Strict ISO C89 + ISO C99 + Watcom dialect variations */
  {
    const char *iso_src = "static_assert(1, \"ok\");\n"
                          "__thread int tls_v = 0;\n"
                          "_Noreturn inline void c89_die(int * restrict p) {\n"
                          "  _Alignas(8) int x = (int)_Alignof(double);\n"
                          "  *p = x;\n"
                          "}\n";
    memset(&cfg, 0, sizeof(cfg));
    cfg.target_profile = CDD_TARGET_PROFILE_ISO_C89;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(iso_src), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tree, &out));
    ASSERT(strstr(out, "/* thread_local */") != NULL);
    ASSERT(strstr(out, "struct { char _c; double _x; }") != NULL);
    free(out);
    out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    memset(&cfg, 0, sizeof(cfg));
    cfg.target_profile = CDD_TARGET_PROFILE_ISO_C99;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(
                  az_span_create_from_str("[[noreturn]] void c99_die(void);\n"),
                  &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tree, &out));
    ASSERT(strstr(out, "_Noreturn void c99_die") != NULL);
    free(out);
    out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    memset(&cfg, 0, sizeof(cfg));
    cfg.target_profile = CDD_TARGET_PROFILE_WATCOM;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(
                                "inline int wfn(void) { return 0; }\n"),
                            &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tree, &out));
    ASSERT(strstr(out, "__inline int wfn") != NULL);
    free(out);
    out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  PASS();
}

/**
 * @brief Comprehensive coverage test for remaining branches in
 * `c_lowering_modern.c` and `c_lowering_legacy.c`, including VLA modes, array
 * parameter qualifiers, `_Complex` & `_Atomic` forms, C23 features, legacy
 * hoisting/designators/literals/suffixes/formats, and 5-way OOM sweeps.
 * @return Greatest test result.
 */
TEST test_c_lowering_complete_coverage(void) {
  cdd_cst_tree_t *tree = NULL;
  char *out = NULL;
  cdd_transform_config_t cfg;

  /* 1. Modern C lowering: VLA modes (alloca, fallback malloc, hybrid) across
        MSVC & ISO_C89, bare sizeof vla, multi-dim VLA, void/value return VLA
        cleanup, array param qualifiers, _Complex, _Atomic, and C23 */
  {
    const char *m_s1 =
        "#include <complex.h>\n"
        "#include <stdckdint.h>\n"
        "/* lead */ constexpr int k_mil = 1'000'000;\n"
        "thread_local int g_tl1 = 0;\n"
        "_Thread_local int g_tl2 = 0;\n"
        "__thread int g_tl3 = 0;\n"
        "[[nodiscard]] [[warn_unused_result]] int mod_arr(\n"
        "    int a[static 4][8], int b[restrict], int c[volatile],\n"
        "    int d[const static 8], int e[sizeof(int[2])]) {\n"
        "  return a[0][0] + b[0] + c[0] + d[0] + e[0] + k_mil;\n"
        "}\n";
    const char *m_s2 =
        "[[noreturn]] void mod_die(void);\n"
        "[[deprecated(\"x\")]] [[vendor::attr(a[0])]] void vla_void(int r, int "
        "c) {\n"
        "  /* c */ [[maybe_unused]] int ext_a[] = { 1, 2 };\n"
        "  int m[r][c];\n"
        "  int v2[ext_a[0] + r];\n"
        "  size_t sz = sizeof m + sizeof v2;\n"
        "  int k;\n"
        "  for (k = 0; k < r; k++) ext_a[0] += (int)sz;\n"
        "  do { m[0][0] = ext_a[0] + v2[0]; } while (0);\n"
        "  if (r == 1) { [[fallthrough]]; return; }\n"
        "}\n";
    const char *m_s3 =
        "int mod_cplx_atom(int r, int c) {\n"
        "  void *np = nullptr;\n"
        "  int vla[r + 1];\n"
        "  size_t vsz = sizeof vla;\n"
        "  float _Complex zf1 = 2.0fi;\n"
        "  float _Complex zf2 = (zf1 - 1.0f) * (2.0fi + zf1) / zf1;\n"
        "  double _Complex z1 = 3.0i;\n"
        "  double _Complex z2 = 4.0j;\n"
        "  double _Complex z3 = (z1 + z2) / (z1 - z2) + (z1 * z2);\n"
        "  long double _Complex zld = 5.0i;\n";
    const char *m_s4 =
        "  _Atomic(int) ac0 = 0;\n"
        "  _Atomic long al0 = 1;\n"
        "  _Atomic int a = 1, b = 2;\n"
        "  int mul_v = r * a;\n"
        "  ++a; a++; --a; a--;\n"
        "  a += (1 + 2); a -= 1; a |= 2; a &= 3; a ^= 4;\n"
        "  b = (a + 1);\n"
        "  int ov = 0;\n"
        "  int ok = ckd_add(&ov, r, c) + ckd_sub(&ov, r, 1) + ckd_mul(&ov, r, "
        "2);\n"
        "  if (np == nullptr && z1 == z2 && zf1 != zf2 && creall(zld) >= 0.0)\n"
        "    return (int)vsz + mul_v + ac0 + (int)al0 + b + ok + "
        "(int)creal(z3);\n"
        "  return a + b;\n"
        "}\n";
    char m_src[2048];
    int m_case;
    CDD_SNPRINTF(m_src, sizeof(m_src), "%s%s%s%s", m_s1, m_s2, m_s3, m_s4);

    for (m_case = 0; m_case < 6; m_case++) {
      memset(&cfg, 0, sizeof(cfg));
      if (m_case == 0) {
        cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;
        cfg.vla_mode = CDD_VLA_MODE_ALLOCA;
      } else if (m_case == 1) {
        cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;
        cfg.vla_mode = CDD_VLA_MODE_ALLOCA;
        cfg.fallback_vla_to_malloc = 1;
      } else if (m_case == 2) {
        cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;
        cfg.vla_mode = CDD_VLA_MODE_HYBRID;
      } else if (m_case == 3) {
        cfg.target_profile = CDD_TARGET_PROFILE_ISO_C89;
        cfg.vla_mode = CDD_VLA_MODE_ALLOCA;
      } else if (m_case == 4) {
        cfg.target_profile = CDD_TARGET_PROFILE_ISO_C89;
        cfg.vla_mode = CDD_VLA_MODE_HYBRID;
      } else {
        cfg.target_profile = CDD_TARGET_PROFILE_WATCOM;
        cfg.vla_mode = CDD_VLA_MODE_MALLOC;
      }
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(m_src), &tree));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tree, &out));
      ASSERT(strstr(out, "1000000") != NULL);
      if (m_case == 0) {
        ASSERT(strstr(out, "_alloca(") != NULL);
        ASSERT(strstr(out, "awm_cmakef(") != NULL);
        ASSERT(strstr(out, "awm_cdiv(") != NULL);
        ASSERT(strstr(out, "atomic_fetch_add(&(b)") == NULL);
        ASSERT(strstr(out, "atomic_store(&(b)") != NULL);
      } else if (m_case == 1) {
        ASSERT(strstr(out, "free(m);") != NULL);
        ASSERT(strstr(out, "free(vla);") != NULL);
      } else if (m_case == 3) {
        ASSERT(strstr(out, "alloca(") != NULL);
      }
      free(out);
      out = NULL;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;
    }
  }

  /* 2. Legacy C89/MSVC lowering: braceless & braced colliding for-init
        hoisting, mid-block pointer/typedef/static/extern/__int128 decls,
        typedef struct & union & sparse array designators, file/func compound
        literals, comments with star-slash, I64 suffixes, Alignas/Alignof, and
        printf formats */
  {
    const char *l_s1 = "// comment with */ inside\r\n"
                       "typedef int MyInt;\n"
                       "typedef struct PtTag { int x; int y; } Pt;\n"
                       "union U { int a; double b; };\n"
                       "_Static_assert(1, \"ok\");\n"
                       "static_assert(1, \"ok2\");\n"
                       "_Noreturn void leg_halt(void);\n"
                       "int *gp = (int[]){ 1, 2, 3 };\n"
                       "static int g_2d[2][2] = { [1] = { 3, 4 } };\n"
                       "static int g_ix[8] = { [sizeof(int[1])] = 9 };\n";
    const char *l_s2 =
        "int leg_cov(int x, int y) {\n"
        "  int sum = 0;\n"
        "  void *c_inits[3] = { NULL, (void *)sizeof(int), (void "
        "*)sizeof(MyInt) };\n"
        "  Pt p = { .y = 2, .x = 1 };\n"
        "  union U u = { .b = 1.5 };\n"
        "  int arr[8] = { [1] = x, [5] = y };\n"
        "  int arr2[4] = { [1] = arr[0], [2] = (x + y) };\n"
        "  Pt p_dyn = { .y = arr[1], .x = (x + 1) };\n"
        "  Pt p2 = (Pt){ 10, 20 };\n";
    const char *l_s3 =
        "  int *ap = (int[ ]){ (1 + 2), 3 };\n"
        "  sum += (1 + 2) + ((Pt){ 5, 6 }).x + ap[0] + (c_inits[0] != NULL);\n"
        "  for (int i = (0); i < 2; i++) sum += i;\n"
        "  for (int i = 0; i < 3; i++) sum += i * 2;\n"
        "  for (int i = 0; i < 3; i++) { int *pi = &i; if (i > 0) { sum += "
        "*pi; } }\n"
        "  for (struct PtTag *ps = &p, *ps2 = 0; ps != 0; ps = ps2) { sum += "
        "ps->x; }\n"
        "  { { sum += 1; } }\n";
    const char *l_s4 = "  if (sum > 0) { if (sum > 1) { sum += 2; } }\n"
                       "  int *p_m = 0, **pp_m = &p_m;\n"
                       "  typedef int LocalT;\n"
                       "  static int s_loc = 7;\n"
                       "  extern int g_ext_v;\n"
                       "  __int128 big_i = 1;\n"
                       "  MyInt ty_v = 4;\n"
                       "  __declspec(align(8)) int ds_v = 2;\n"
                       "  _Alignas((8)) int al_v = (int)_Alignof(int);  \n"
                       "  unsigned long long u1 = 10ull + 10uLL + 10Ull;\n"
                       "  long long s1 = 10ll + 10LL;\n";
    const char *l_s5a =
        "  lbl_done: sum += *pp_m ? **pp_m : 0;\n"
        "  printf(\"%% %zu %zd %zx %zX %lld %lli %llu %llx %llX %jd %ju %td "
        "%tu %-8.2zu\\n\",\n"
        "         (size_t)1, (size_t)2, (size_t)3, (size_t)4, 5LL, 6LL, 7ULL,\n"
        "         8ULL, 9ULL, 10LL, 11ULL, (ptrdiff_t)12, (size_t)13, "
        "(size_t)14);\n";
    const char *l_s5b =
        "  printf(\"%+lld % lli %#llx %llo %*zu %zo %jx %jX\\n\",\n"
        "         1LL, 2LL, 3ULL, 4ULL, 4, (size_t)5, (size_t)6, 7ULL, 8ULL);\n"
        "  return sum + p.x + (int)u.b + arr[1] + arr2[1] + p_dyn.y + p2.x +\n"
        "         s_loc + (LocalT)big_i + ty_v + ds_v + al_v + (int)u1 + "
        "(int)s1 + gp[0] + g_2d[1][0] + g_ix[4];\n"
        "}\n";
    char l_src[3072];
    int l_case;
    CDD_SNPRINTF(l_src, sizeof(l_src), "%s%s%s%s%s%s", l_s1, l_s2, l_s3, l_s4,
                 l_s5a, l_s5b);

    for (l_case = 0; l_case < 3; l_case++) {
      memset(&cfg, 0, sizeof(cfg));
      if (l_case == 0) {
        cfg.target_profile = CDD_TARGET_PROFILE_MSVC_LEGACY;
        cfg.msvc_ver = 1300;
      } else if (l_case == 1) {
        cfg.target_profile = CDD_TARGET_PROFILE_ISO_C89;
      } else {
        cfg.target_profile = CDD_TARGET_PROFILE_DEFAULT;
        cfg.target_c89 = 1;
        cfg.msvc_ver = 1300;
      }
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(l_src), &tree));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tree, &out));
      ASSERT(strstr(out, "static int __cdd_cl_") != NULL);
      if (l_case == 0) {
        ASSERT(strstr(out, "10UI64") != NULL);
        ASSERT(strstr(out, "10I64") != NULL);
        ASSERT(strstr(out, "%-8.2Iu") != NULL);
      }
      free(out);
      out = NULL;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;
    }
  }

  /* 3. Full OOM fault-injection sweeps across MSVC_LEGACY (msvc_ver=1300) and
        MSVC_MODERN for alloc, strdup, cst_realloc, alloc_token, alloc_node */
#ifdef CDD_BUILD_TESTS
  {
    const char *oom_leg =
        "// cmt */ end\n"
        "typedef struct { int x; int y; } Pt;\n"
        "_Static_assert(1, \"ok\");\n"
        "int *gp = (int[]){ 1, 2 };\n"
        "int fl(int x) {\n"
        "  int s = 1;\n"
        "  Pt p = { .y = 2, .x = 1 };\n"
        "  int a[4] = { [1] = x };\n"
        "  Pt p2 = (Pt){ 3, 4 };\n"
        "  for (int i = 0; i < 2; i++) s += i;\n"
        "  for (int i = 0; i < 2; i++) s += i;\n"
        "  int *mp = 0;\n"
        "  _Alignas(8) unsigned long long u = 10ull + (unsigned long "
        "long)_Alignof(int);\n"
        "  printf(\"%zu %lld\\n\", (size_t)s, 10ll);\n"
        "  return s + p.x + a[1] + p2.y + (mp != 0) + (int)u + gp[0];\n"
        "}\n";
    const char *oom_mod =
        "#include <complex.h>\n"
        "#include <stdckdint.h>\n"
        "constexpr int k = 1'000'000;\n"
        "thread_local int gt = 0;\n"
        "[[nodiscard]] int fm(int r, int a[static 4][8], int b[restrict]) {\n"
        "  void *np = nullptr;\n"
        "  int m[r][4];\n"
        "  size_t sz = sizeof m;\n"
        "  float _Complex z = 2.0fi;\n"
        "  float _Complex w = (z + 2.0fi) / z;\n"
        "  _Atomic int ac = 1, bc = 2;\n"
        "  ++ac; ac += 1; bc = 3;\n"
        "  int ov = 0;\n"
        "  int ok = ckd_add(&ov, r, k);\n"
        "  if (np == nullptr) return (int)sz + a[0][0] + b[0] + ac + bc + ok + "
        "(int)crealf(w);\n"
        "  return gt;\n"
        "}\n";
    int fi;
    int prof;
    int fkind;

    for (prof = 0; prof < 2; prof++) {
      const char *src_p = (prof == 0) ? oom_leg : oom_mod;
      memset(&cfg, 0, sizeof(cfg));
      if (prof == 0) {
        cfg.target_profile = CDD_TARGET_PROFILE_MSVC_LEGACY;
        cfg.msvc_ver = 1300;
      } else {
        cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;
        cfg.vla_mode = CDD_VLA_MODE_HYBRID;
      }
      for (fkind = 0; fkind < 5; fkind++) {
        for (fi = 1; fi <= 35; fi++) {
          cdd_c_error_t rc;
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_cst_parse(az_span_create_from_str(src_p), &tree));
          if (fkind == 0)
            g_cdd_alloc_fail = fi;
          else if (fkind == 1)
            g_cdd_strdup_fail = fi;
          else if (fkind == 2)
            g_cdd_cst_realloc_fail = fi;
          else if (fkind == 3)
            g_cdd_cst_alloc_token_fail = fi;
          else
            g_cdd_cst_alloc_node_fail = fi;
          rc = cdd_transform_c_lowering(tree, &cfg);
          g_cdd_alloc_fail = 0;
          g_cdd_strdup_fail = 0;
          g_cdd_cst_realloc_fail = 0;
          g_cdd_cst_alloc_token_fail = 0;
          g_cdd_cst_alloc_node_fail = 0;
          g_cdd_last_alloc_err = rc;
          ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_MEMORY);
          ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
          tree = NULL;
        }
      }
    }
  }
#endif

  /* Focused micro-cases & OOM sweeps for remaining c_lowering lines */
  {
    static const char *const k_low_micro[] = {
        "int f_triv(int a[ /* c */  static 4]);\n"
        "_Thread_local int tl_iso = 0; restrict int *rp_iso = 0;\n"
        "int f_eof(int a[static 4])",
        "int f_vla_ret1(int n) { int a[n]; return a[0]; }\n",
        "int f_vla_brk(int n, int *arr) {\n"
        "  while (n > 0) { int a[n]; if (n == 2) break; if (n == 3) continue; "
        "return sizeof a; }\n"
        "  int x = 0; x = x + arr[n]; int y = sizeof[0]; return x + y;\n"
        "}\n",
        "#include <complex.h>\n"
        "float complex g_fcz;\n",
        "#include <complex.h>\n"
        "void f_cplx(double _Complex z1, double _Complex z2) { "
        "int hx = 0x1i; double _Complex w = 1.0i + _Complex_I; "
        "w = [[nodiscard]] z1 + z1 * z2; }\n",
        "[[nodiscard]] + 1;\n",
        "void f_atom(int *arr) { _Atomic(int (*)(int)) fp; _Atomic int x = 1; "
        "arr[x] = 0; x++; if ((x += 1)) {} if ((x)) {} }\n",
        "typedef int my_td_c;\n"
        "struct In { int v; }; struct Out { struct In in; int y; };\n"
        "int g_big_arr[4] = { [200] = 1 };\n"
        "int g_ok_arr[4] = { [2] = [[maybe_unused]] sizeof(my_td_c) };\n"
        "void f_cl_des(int idx, int d[2]) {\n"
        "  int *p = (int[4]){ 1, 2, 3, 4 };\n"
        "  struct Out o = (struct Out){ .in = { 1 }, .y = 2 };\n"
        "  int a[4] = { [d[0]] = idx };\n"
        "  struct Out o2 = { .y = (idx + 1) };\n"
        "}\n",
        "int g_non_agg = { .a = 1 }; struct { int a; } = { .a = 1 };\n",
        "typedef int my_td_t;\n"
        "inline _Noreturn void f_leg(void) {\n"
        "  for (int i = 0) {}\n"
        "  for (int; ; ) { break; }\n"
        "  for (int i = 0; i < 2; i++) { int k = i; }\n"
        "  for (int i = 0; i < 2; i++) { int k = i; }\n"
        "  { [[fallthrough]] }\n"
        "  my_td_t = 1;\n"
        "  static int st_h = 3;\n"
        "  int __declspec(align((8))) al_h = 4;\n"
        "  int sz_a = (int)_Alignof((int));\n"
        "  const char *fn = __func__;\n"
        "  const char *fmt = \"%zu %lld\";\n"
        "  unsigned long long ull_v = 10ULL;\n"
        "}\n",
        "inline _Noreturn void f_leg2(void) { const char *fn = __func__; "
        "unsigned long long u = 10ULL; const char *s = \"%zu\"; }\n",
        "int g_c89_cmt = 1; // trailing line cmt\n"};
    size_t mi;
    int fi;
    char sbuf[16];

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c_low_prepend_root(NULL, "x"));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c_low_slice_text(NULL, 0, 0, NULL, 0));
    ASSERT_EQ(CDD_C_SUCCESS, c_low_slice_text(NULL, 2, 1, sbuf, sizeof(sbuf)));

    for (mi = 0; mi < sizeof(k_low_micro) / sizeof(k_low_micro[0]); mi++) {
      memset(&cfg, 0, sizeof(cfg));
      if (mi == 0 || mi == 11) {
        cfg.target_profile = CDD_TARGET_PROFILE_ISO_C89;
        cfg.target_c89 = 1;
      } else {
        cfg.target_profile = CDD_TARGET_PROFILE_MSVC_LEGACY;
        cfg.msvc_ver = 1300;
      }
      cfg.vla_mode =
          (mi == 1 || mi == 2) ? CDD_VLA_MODE_MALLOC : CDD_VLA_MODE_HYBRID;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(k_low_micro[mi]), &tree));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;

#ifdef CDD_BUILD_TESTS
      for (fi = 1; fi <= 28; fi++) {
        ASSERT_EQ(
            CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str(k_low_micro[mi]), &tree));
        g_gnu_malloc_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_c_lowering(tree, &cfg);
        g_gnu_malloc_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
        tree = NULL;

        ASSERT_EQ(
            CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str(k_low_micro[mi]), &tree));
        g_gnu_replace_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_c_lowering(tree, &cfg);
        g_gnu_replace_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
        tree = NULL;

        ASSERT_EQ(
            CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str(k_low_micro[mi]), &tree));
        g_gnu_bld_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_c_lowering(tree, &cfg);
        g_gnu_bld_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
        tree = NULL;

        ASSERT_EQ(
            CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str(k_low_micro[mi]), &tree));
        g_cdd_alloc_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_c_lowering(tree, &cfg);
        g_cdd_alloc_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
        tree = NULL;

        ASSERT_EQ(
            CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str(k_low_micro[mi]), &tree));
        g_cdd_cst_alloc_token_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_c_lowering(tree, &cfg);
        g_cdd_cst_alloc_token_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
        tree = NULL;

        ASSERT_EQ(
            CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str(k_low_micro[mi]), &tree));
        g_cdd_cst_realloc_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_c_lowering(tree, &cfg);
        g_cdd_cst_realloc_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
        tree = NULL;

        ASSERT_EQ(
            CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str(k_low_micro[mi]), &tree));
        g_cdd_cst_alloc_node_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_c_lowering(tree, &cfg);
        g_cdd_cst_alloc_node_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
        tree = NULL;
      }
#endif
    }

    /* Empty base_tokens tree */
    {
      cdd_cst_tree_t empty_tr;
      cdd_cst_node_t dummy_root;
      memset(&empty_tr, 0, sizeof(empty_tr));
      memset(&dummy_root, 0, sizeof(dummy_root));
      empty_tr.root = &dummy_root;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(&empty_tr, &cfg));
    }
  }

  PASS();
}

/**
 * @brief Unit test test_c_lowering_modern_branch_completion.
 * @return Greatest test result.
 */
TEST test_c_lowering_modern_branch_completion(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_transform_config_t cfg;
  int fi;

  /* 1. Helper edge calls: c_low_prepend_root(empty_root_null, NULL),
        c_low_slice_text(cap==0), detached wiped & unwiped tokens in span */
  {
    cdd_cst_tree_t t_no_root;
    char sb[8];
    memset(&t_no_root, 0, sizeof(t_no_root));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c_low_prepend_root(&t_no_root, "int x;"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("int x;"), &tree));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c_low_prepend_root(tree, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c_low_slice_text(tree->base_tokens->tokens, 0, 1, sb, 0));
    ASSERT_EQ(CDD_C_SUCCESS, c_low_slice_text(tree->base_tokens->tokens, 2, 1,
                                              sb, sizeof(sb)));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* Detached wiped (length == 0) token inside span */
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_cst_parse(az_span_create_from_str("[[ a b c ]] int x;"), &tree));
    /* Detach `a` (wiping length=0) from CST node */
    {
      size_t cidx = 0;
      cdd_cst_node_t *pn = NULL;
      if (cdd_cst_find_node_for_token(tree->root, &tree->base_tokens->tokens[2],
                                      &cidx, &pn) == CDD_C_SUCCESS &&
          pn) {
        tree->base_tokens->tokens[2].length = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_remove_child(pn, cidx));
      }
    }
    memset(&cfg, 0, sizeof(cfg));
    cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  /* 2. C23 attributes, digit separators, thread_local, array params, VLAs */
  {
    const char *m1 =
        "#include <stdckdint.h>\n"
        "enum E { EV_C = 4 };\n"
        "thread_local int id_twelve_ch = 0;\n"
        "[[_abcd]] [[__ab_]] [[__abc__]] [[gnu:one]] [[gnu::123]] [[123]] "
        "int g_at;\n"
        "int f_ap1(int a[static 4], int b[__restrict 4], int c[restrict 4],\n"
        "          int d[const 4], int e[volatile 4], int m[2][3]);\n"
        "int f_ap2(int n, int a[n][n], int b[sizeof(int)], int "
        "c[sizeof((int))],\n"
        "          int d[sizeof n], int e[EV_C], int (fp)[4], [4],\n"
        "          int p5[id5ch], int p6[id_6ch], int p8[id_8char], int "
        "p10[id_10chars]);\n";
    const char *m2 = "void f_void_helper(void) {}\n"
                     "void f_vla_void(int n) {\n"
                     "  int a[n];\n"
                     "  if (n == 1) return f_void_helper();\n"
                     "  return;\n"
                     "}\n"
                     "int f_vla_edges(int n, int **pp, int x) {\n"
                     "  typedef int t6name;\n"
                     "  t6name b_vla[n];\n"
                     "  int a[n];\n"
                     "  int *p_vla[n];\n"
                     "  int fixed_arr[4];\n"
                     "  { int br_vla[n]; } x = *pp[n];\n"
                     "  if (*pp[n]) {}\n"
                     "  x = x + *pp[n] - *pp[n];\n";
    const char *m3 =
        "  int s0 = (int)sizeof fixed_arr[0] + (int)sizeof(x);\n"
        "  int s1 = (int)sizeof(fixed_arr);\n"
        "  int s2 = (int)sizeof(int);\n"
        "  int s3 = (int)sizeof + 1;\n"
        "  for (x = 0; x < 2; x++) ) ;\n"
        "  if (x == 9) break;\n"
        "  switch (x) { default: { int sv[n]; continue; } }\n"
        "  while (x > 0) { int wv[n]; if (x == 1) break + 1; break; }\n"
        "  return s0 + s1 + s2 + s3 + a[0] + b_vla[0] + (p_vla[0] != 0);\n"
        "}\n";
    char m_buf[3072];
    CDD_SNPRINTF(m_buf, sizeof(m_buf), "%s%s%s", m1, m2, m3);

    /* Test across Modern MSVC (HYBRID), ISO C89, and C99/default profiles */
    memset(&cfg, 0, sizeof(cfg));
    cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;
    cfg.vla_mode = CDD_VLA_MODE_HYBRID;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(m_buf), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    memset(&cfg, 0, sizeof(cfg));
    cfg.target_profile = CDD_TARGET_PROFILE_ISO_C89;
    cfg.vla_mode = CDD_VLA_MODE_ALLOCA;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(m_buf), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    memset(&cfg, 0, sizeof(cfg));
    cfg.target_profile = CDD_TARGET_PROFILE_ISO_C99;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(
                                "[[noreturn]] [[fallthrough]] [[__abc_d]] void "
                                "f_nr(int a[restrict 4]);\n"
                                "restrict int *gp;\n"
                                "_Thread_local int gtl;\n"),
                            &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* Void VLA return OOM sweep (hits L804-811!) */
    memset(&cfg, 0, sizeof(cfg));
    cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;
    cfg.vla_mode = CDD_VLA_MODE_MALLOC;
    {
      int saw_rep_err = 0;
      int saw_mal_err = 0;
      for (fi = 1; fi <= 8; fi++) {
        cdd_c_error_t rc_v;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(
                                    "void f_vr(int n) { int a[n]; return; }"),
                                &tree));
        g_gnu_replace_fail = fi;
        rc_v = cdd_transform_c_lowering(tree, &cfg);
        g_gnu_replace_fail = 0;
        if (rc_v == CDD_C_ERROR_MEMORY)
          saw_rep_err = 1;
        else
          ASSERT_EQ(CDD_C_SUCCESS, rc_v);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
        tree = NULL;

        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(
                                    "void f_vr(int n) { int a[n]; return; }"),
                                &tree));
        g_gnu_malloc_fail = fi;
        rc_v = cdd_transform_c_lowering(tree, &cfg);
        g_gnu_malloc_fail = 0;
        if (rc_v == CDD_C_ERROR_MEMORY)
          saw_mal_err = 1;
        else
          ASSERT_EQ(CDD_C_SUCCESS, rc_v);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
        tree = NULL;
      }
      ASSERT_EQ(1, saw_rep_err);
      ASSERT_EQ(1, saw_mal_err);
    }
  }

  /* 3. Complex & Atomic exhaustive operator / boundary / OOM cases */
  {
    const char *ca1a =
        "#include <complex.h>\n"
        "void f_ca(float _Complex f1, float _Complex f2, float _Complex f3,\n"
        "          double _Complex z1, double _Complex z2, double _Complex "
        "z3,\n"
        "          long double _Complex lz, long int_not_long_dbl _Complex "
        "oz,\n"
        "          int cond) {\n"
        "  float _Complex fw = 1.0Fi + 2.0fJ + 3.0I + 4.0j;\n";
    const char *ca1b =
        "  double r = creal(z1) + cimag(z1) + crealf(f1) + cimagf(f1);\n"
        "  fw = conjf(f1); z1 = conj(z2);\n"
        "  fw = 1.0f + 1.0f / f3; z1 = 1.0 + 1.0 / z3;\n"
        "  z1 = 1.0 + 1.0 * z3; cond = (1.0 == 1.0 + z3) + (1.0 == 1.0 - z3);\n"
        "  fw = (f1 + f2) + (f2 - f3);\n"
        "  z1 = z1 + (z2 / z3) - (z1 * z2);\n";
    const char *ca2a =
        "  int ia = 1, ib = 2;\n"
        "  int awm_cmake = 0, awm_cadd = 0, awm_csub = 0, awm_cmul = 0, "
        "awm_cdiv = 0;\n"
        "  z1 = cond ? z1 + z2 : z2 + z3;\n"
        "  cond = (z1 + z2 ? 1 : 0);\n"
        "  { int b0 = 0; } z1 + z2;\n"
        "  z1 = z1 + z2, z2 + z3;\n"
        "  cond = (ia == ib + z2); cond = (ia != ib + z2);\n"
        "  cond = (z1 + ia == ib); cond = (z1 + ia != ib);\n";
    const char *ca2b =
        "  int awm_cmakeX = 0, awm_caddX = 0;\n"
        "  cond = (0 | z1 + z2 | 0) + (1 & z1 + z2 & 1) + (1 % z1 + z2 % 1);\n"
        "  z1 = @ 1 + z1; z1 = z1 + z2 @;\n"
        "  z1 = ia - ib * z2; z1 = z1 + ia - ib;\n"
        "  { z1 + z2 } z1 + ;\n"
        "  z1 = awm_cmake + z1; z1 = z1 + awm_cmake; z1 = awm_cadd + z1;\n"
        "  z1 = awm_cmakeX + z1; z1 = z1 + awm_cmakeX;\n"
        "  z1 = awm_caddX + z1; z1 = z1 + awm_caddX;\n"
        "  fw = f1 + (f2 + f3); fw = f1 + 1.0fJ;\n"
        "  z1 = z1 + awm_csub; z1 = z1 + awm_cmul; z1 = z1 + awm_cdiv;\n"
        "  z1 = 0X1i + 01i + 1i; z1 = z1 + 1.0; z1 = f1 + z2;\n";
    const char *ca3a = "  _Atomic int ax = 1, ay = 2;\n"
                       "  _Atomic(int) *ap = 0;\n"
                       "  struct { int ax; } sa;\n"
                       "  int arr[4]; int nx = 0;\n"
                       "  { *ax; }\n"
                       "  sa.ax = 1; ++nx; -nx; + + ax; ax + +; ax + = 1;\n"
                       "  --ax; ax--;\n"
                       "  ax -= 1; ax |= 2; ax &= 3; ax ^= 4; ax *= 2;\n";
    const char *ca3b =
        "  ax += arr[0]; ax = arr[0];\n"
        "  (ax += 1, ax = 2, nx = 0);\n"
        "  arr[(ax = 1)] = arr[(ax += 1)];\n"
        "  { ax = 1 } { ax += 1 }\n"
        "  if (cond) return (z1 == z2);\n"
        "  return 0;\n"
        "}\n"
        "_Complex double f_creal_ret(double _Complex z1) { return z1 + z1; }\n"
        "void f_atom_param(_Atomic int ap_v) { int y = ap_v; }\n";
    char ca_buf[3584];
    CDD_SNPRINTF(ca_buf, sizeof(ca_buf), "%s%s%s%s%s%s", ca1a, ca1b, ca2a, ca2b,
                 ca3a, ca3b);

    memset(&cfg, 0, sizeof(cfg));
    cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(ca_buf), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* config == NULL on <complex.h> without complex vars (hits L1071 & L1213!)
     */
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_parse(az_span_create_from_str(
                                               "#include <complex.h>\nint x;"),
                                           &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* With --with-auto-win-msvc on <complex.h> */
    cfg.with_auto_win_msvc = 1;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_parse(az_span_create_from_str(
                                               "#include <complex.h>\nint x;"),
                                           &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
    cfg.with_auto_win_msvc = 0;

    /* Wiped leading token `[[maybe_unused]] + z1;` and `[[maybe_unused]] z1 +
       z2;` at token 0 (hits L1248, L1251-1252, L1303-1304!) */
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_cst_parse(
            az_span_create_from_str(
                "[[maybe_unused]] + 1; double _Complex z1, z2; void f(void){ "
                "[[maybe_unused]] z1 + z2; }"),
            &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(
                                "[[maybe_unused]] + z1; double _Complex z1;"),
                            &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* OOM on creal(z1) and atomic rvalue load cdd_pool_string_safe (L1717) */
    {
      int saw_rep_err = 0;
      int saw_alloc_err = 0;
      for (fi = 1; fi <= 32; fi++) {
        cdd_c_error_t rc_c;
        ASSERT_EQ(
            CDD_C_SUCCESS,
            cdd_cst_parse(
                az_span_create_from_str(
                    "double _Complex z1; double f(void){ return creal(z1); }"),
                &tree));
        g_gnu_replace_fail = fi;
        rc_c = cdd_transform_c_lowering(tree, &cfg);
        g_gnu_replace_fail = 0;
        if (rc_c == CDD_C_ERROR_MEMORY)
          saw_rep_err = 1;
        else
          ASSERT_EQ(CDD_C_SUCCESS, rc_c);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
        tree = NULL;

        ASSERT_EQ(
            CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str(
                              "_Atomic int ax; int f(void){ return ax; }"),
                          &tree));
        g_cdd_alloc_fail = fi;
        rc_c = cdd_transform_c_lowering(tree, &cfg);
        g_cdd_alloc_fail = 0;
        if (rc_c == CDD_C_ERROR_MEMORY)
          saw_alloc_err = 1;
        else
          ASSERT_EQ(CDD_C_SUCCESS, rc_c);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
        tree = NULL;

        ASSERT_EQ(
            CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str(
                              "_Atomic int ax; int f(void){ return ax; }"),
                          &tree));
        g_gnu_replace_fail = fi;
        rc_c = cdd_transform_c_lowering(tree, &cfg);
        g_gnu_replace_fail = 0;
        if (rc_c != CDD_C_ERROR_MEMORY)
          ASSERT_EQ(CDD_C_SUCCESS, rc_c);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
        tree = NULL;
      }
      ASSERT_EQ(1, saw_rep_err);
      ASSERT_EQ(1, saw_alloc_err);
    }

    /* () {} at token 0 hits back == 0 in c_lower_vlas */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("() { int x; }"), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  /* 4. Large overflow/cap & EOF boundary snippets (>128 digit sep, >32 VLAs,
        >32 loops, >64 complex/atomic vars, >=95-char names, EOF fragments) */
  {
    char big_m[12288];
    size_t p = 0;
    int k;
    char id125[130];
    char sep140[160];
    memset(id125, 'V', 125);
    id125[125] = '\0';
    sep140[0] = '1';
    sep140[1] = '\'';
    memset(sep140 + 2, '9', 140);
    sep140[142] = '\0';

    p += (size_t)CDD_SNPRINTF(big_m + p, sizeof(big_m) - p,
                              "z1 + z2;\n"
                              "short _Complex z1, z2;\n"
                              "_Atomic int %s;\n"
                              "() { int v_anon[n]; return 1; }\n"
                              "int ) { int v_bad[n]; }\n"
                              "int g_sep = %s;\n"
                              "int f_lng_b(int %s, int a[%s]);\n"
                              "void %s(int n) { int a[n]; }\n"
                              "void f_many(int n) {\n"
                              "  for (({ int q = 1; }); 0; ) {}\n"
                              "  int %s[n];\n"
                              "  int init_vla[n] = { 0 };\n",
                              id125, sep140, id125, id125, id125, id125);
    /* >256-byte spaced multi-dim VLA expression */
    p += (size_t)CDD_SNPRINTF(big_m + p, sizeof(big_m) - p, "  int vbig[");
    for (k = 0; k < 45; k++)
      p += (size_t)CDD_SNPRINTF(big_m + p, sizeof(big_m) - p, "n + ");
    p += (size_t)CDD_SNPRINTF(big_m + p, sizeof(big_m) - p, "1][");
    for (k = 0; k < 45; k++)
      p += (size_t)CDD_SNPRINTF(big_m + p, sizeof(big_m) - p, "n + ");
    p += (size_t)CDD_SNPRINTF(big_m + p, sizeof(big_m) - p, "1];\n");
    for (k = 0; k < 34; k++) {
      p += (size_t)CDD_SNPRINTF(big_m + p, sizeof(big_m) - p, "  int v%d[n];\n",
                                k);
    }
    p += (size_t)CDD_SNPRINTF(big_m + p, sizeof(big_m) - p, "  ");
    for (k = 0; k < 34; k++)
      p += (size_t)CDD_SNPRINTF(big_m + p, sizeof(big_m) - p, "for(;0;){");
    for (k = 0; k < 34; k++)
      p += (size_t)CDD_SNPRINTF(big_m + p, sizeof(big_m) - p, "}");
    p += (size_t)CDD_SNPRINTF(big_m + p, sizeof(big_m) - p, "\n");
    for (k = 0; k < 66; k++) {
      p += (size_t)CDD_SNPRINTF(big_m + p, sizeof(big_m) - p,
                                "  double _Complex cz%d;\n", k);
    }
    p += (size_t)CDD_SNPRINTF(big_m + p, sizeof(big_m) - p,
                              "  double _Complex %s, cz_fn(int);\n", id125);
    for (k = 0; k < 66; k++) {
      p += (size_t)CDD_SNPRINTF(big_m + p, sizeof(big_m) - p,
                                "  _Atomic int av%d, avb%d;\n", k, k);
    }
    p += (size_t)CDD_SNPRINTF(
        big_m + p, sizeof(big_m) - p,
        "  _Atomic int , %s, ;\n"
        "  _Atomic int { int b; };\n"
        "  int sz_other = (int)sizeof(n) + (int)sizeof(n + 1);\n"
        "  cz0 = cz1 + cz2 + cz3 + cz4 + cz5 + cz6 + cz7 + cz8 + cz9 + cz10 + "
        "cz11 + cz12 + cz13 + cz14 + cz15 + cz16 + cz17 + cz18;\n"
        "}\n",
        id125);

    memset(&cfg, 0, sizeof(cfg));
    cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;
    cfg.vla_mode = CDD_VLA_MODE_MALLOC;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(big_m), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* Valid-parse + base_tokens->size truncation by 1 token to exercise EVERY
       unclosed / EOF loop exit cleanly! */
    {
      static const char *const trunc_frags[] = {
          "[ [x]]",
          "[[nodiscard]]",
          "[[gnu::a]]",
          "int f(int a[static 4]);",
          "int f(int a[]);",
          "int f(int a[static 4][3]);",
          "int f(int a[static 4] + 1);",
          "void f(int n){ int a[n]; sizeof a; }",
          "void f(int n){ int a[n]; sizeof(a); }",
          "void f(int n){ int a[n]; return 1; }",
          "void f(int n){ while(1){ int a[n]; break; } }",
          "void f(int n){ int a[n][]; }",
          "void f(int n){ int a[n]; }",
          "{ [0]; }",
          "double _Complex g_top;",
          "signed _Complex g_sc;",
          "char double _Complex g_cdc;",
          "+ _Complex g_pc;",
          "_Complex g_bare_c;",
          "int g_cr0 = creal(1); double _Complex gz; int g_cr1 = creal;",
          "double _Complex gz; int g_cr2 = creal(gz);",
          "_Atomic(int) gx;",
          "_Atomic int gx;",
          "_Atomic int ax; void f(void){ ++ax; }",
          "_Atomic int ax; void f(void){ ax++; }",
          "_Atomic int ax; void f(void){ ax += 1; }",
          "_Atomic int ax; void f(void){ ax = 1; }",
          "_Atomic int ax; void f(struct S *sp){ sp->ax; &ax; int *ax; }",
          "double _Complex z1, z2; int g = z1 + z2;"};
      size_t qi;
      size_t cut;
      for (qi = 0; qi < sizeof(trunc_frags) / sizeof(trunc_frags[0]); qi++) {
        for (cut = 0; cut <= 4; cut++) {
          ASSERT_EQ(
              CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(trunc_frags[qi]), &tree));
          if (cut > 0 && tree->base_tokens->size > cut) {
            tree->base_tokens->size -= cut;
          }
          ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
          tree = NULL;
        }
      }
    }

    /* Mutate ']' inside param lists & ')' inside _Atomic(int) */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(
                                "int f1(int a[4]); int f2(int b[static 4][3]); "
                                "int f3(int c[, int d); _Atomic(int) gx;"),
                            &tree));
    {
      size_t ti;
      int rb_seen = 0;
      for (ti = 0; ti < tree->base_tokens->size; ti++) {
        if (tree->base_tokens->tokens[ti].kind == CDD_TOKEN_RBRACKET) {
          rb_seen++;
          if (rb_seen == 1 || rb_seen == 3) {
            tree->base_tokens->tokens[ti].kind = CDD_TOKEN_IDENTIFIER;
          }
        } else if (tree->base_tokens->tokens[ti].kind == CDD_TOKEN_RPAREN &&
                   ti > 0 &&
                   tree->base_tokens->tokens[ti - 1].kind ==
                       CDD_TOKEN_KEYWORD_INT) {
          tree->base_tokens->tokens[ti].kind = CDD_TOKEN_IDENTIFIER;
        }
      }
    }
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* Stray '}' at cur_depth == 0 / brace_d == 0 and '{' at token 0 +
     * fsym->type_str == NULL */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(
                                "_Atomic int ax; + void f_nt(int n){ int "
                                "a[n]; }"),
                            &tree));
    {
      size_t ti;
      for (ti = 0; ti < tree->base_tokens->size; ti++) {
        if (tree->base_tokens->tokens[ti].kind == CDD_TOKEN_PLUS) {
          tree->base_tokens->tokens[ti].kind = CDD_TOKEN_RBRACE;
          break;
        }
      }
    }
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* int a[n]; at token 0..4 with '{' mutated to ';' after cur_depth = 1 */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("{ int a[n]; }"), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  PASS();
}

/**
 * @brief Exhaustively exercises remaining branches and OOM error paths in
 * `c_lowering_legacy.c`.
 *
 * @return Greatest test result.
 */
TEST test_c_lowering_legacy_branch_completion(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_transform_config_t cfg;
  size_t si;
  int fi;

  memset(&cfg, 0, sizeof(cfg));
  cfg.target_profile = CDD_TARGET_PROFILE_MSVC_LEGACY;
  cfg.msvc_ver = 1310;

  /* 1. tree->base_tokens == NULL and size == 0 guards */
  {
    cdd_token_list_t *saved_bt = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("int x;"), &tree));
    saved_bt = tree->base_tokens;
    tree->base_tokens = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    tree->base_tokens = saved_bt;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_parse(az_span_create_from_str(""), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  /* 2. msvc_ver matrix (0, 1310, 1600, 1920) + Watcom + ISO C89 */
  {
    const int vers[] = {0, 1310, 1600, 1920};
    for (si = 0; si < sizeof(vers) / sizeof(vers[0]); si++) {
      cfg.target_profile = CDD_TARGET_PROFILE_MSVC_LEGACY;
      cfg.msvc_ver = vers[si];
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(
                    az_span_create_from_str(
                        "inline void fv(void) { const char *s = \"%zu %lld\"; "
                        "unsigned long long u = 1ULL + 2LL + 100; }"),
                    &tree));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;
    }
    cfg.msvc_ver = 1310;
  }

  /* 3. Deep 66-brace nesting + stray '}' + top-level '{' / '()' */
  {
    char deep[512];
    size_t p = 0;
    int k;
    p += (size_t)CDD_SNPRINTF(deep, sizeof(deep), "void fd(void) ");
    for (k = 0; k < 66; k++)
      deep[p++] = '{';
    p += (size_t)CDD_SNPRINTF(deep + p, sizeof(deep) - p,
                              " for (int i = 0; i < 1; i++) {} ");
    for (k = 0; k < 66; k++)
      deep[p++] = '}';
    deep[p] = '\0';
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(deep), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* Stray '}' at brace_d == 0 via token mutation + () { __func__ } + ) { */
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_cst_parse(
            az_span_create_from_str(
                "+ (void) { const char *f = __func__; } int (*fp(void))(int) { "
                "return 0; }"),
            &tree));
    tree->base_tokens->tokens[0].kind = CDD_TOKEN_RBRACE;
    tree->base_tokens->tokens[2].length = 0;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("+ { int x; }"), &tree));
    tree->base_tokens->tokens[0].kind = CDD_TOKEN_RPAREN;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* `int a ) { int x; }` so i >= 2 and back reaches 0 without '(' */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("int a(void) { int x; }"),
                            &tree));
    tree->base_tokens->tokens[2].kind = CDD_TOKEN_IDENTIFIER;
    tree->base_tokens->tokens[3].kind = CDD_TOKEN_IDENTIFIER;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* `+ () {}` with rp == i + 1 in c_lower_compound_literals */
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_cst_parse(az_span_create_from_str("void f(int x) {}"), &tree));
    tree->base_tokens->tokens[1].kind = CDD_TOKEN_PLUS;
    tree->base_tokens->tokens[3].kind = CDD_TOKEN_RPAREN;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  /* 4. Compound literals & const_init_expr exhaustive edges */
  {
    char id125[130];
    char cl_buf[2048];
    const char *cl_snips[] = {
        "(int){1};",
        "int g0; int *gp = &(int){1}; void f0(void){} int g1 = (int){2};",
        "void f_sz(void) { int s = sizeof(int){1}; sizeox(1); } int fr(void) "
        "{ return(1); }",
        "void f00(void){}",
        "{ int x = 1; }",
        "int x; { int y = 1; }",
        "void f_cl(void) { (int){1}; int s0 = 0; (int){2}; { int s1 = 0; } "
        "(int){3}; if ((1)) (int){4}; }",
        "void f_arr_cl(void) { int *p = (int[]){{1}, (2)}; int *q = "
        "(int[ ]){}; int *r = (int[2]){ , 1 }; int *z = (int[2]){}; }"};
    memset(id125, 'Z', 125);
    id125[125] = '\0';
    for (si = 0; si < sizeof(cl_snips) / sizeof(cl_snips[0]); si++) {
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(cl_snips[si]), &tree));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;
    }

    /* Post-parse token mutations for unclosed '(' / '()' / '(int){1' / 'x {' */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("(int)x;"), &tree));
    tree->base_tokens->tokens[2].kind = CDD_TOKEN_IDENTIFIER;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("(int){1};"), &tree));
    tree->base_tokens->tokens[5].kind = CDD_TOKEN_IDENTIFIER;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("(int){}"), &tree));
    tree->base_tokens->size = 4;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    tree->base_tokens->size = 5;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_cst_parse(az_span_create_from_str("void f(void){int a;}"), &tree));
    /* x { int a; }: make tokens[0]=IDENT "x", tokens[1]=LBRACE '{' */
    tree->base_tokens->tokens[0].kind = CDD_TOKEN_IDENTIFIER;
    tree->base_tokens->tokens[1].kind = CDD_TOKEN_LBRACE;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    CDD_SNPRINTF(cl_buf, sizeof(cl_buf),
                 "typedef int my_td_t;\n"
                 "enum MyE { E_C = 3 };\n"
                 "int g_ord_var = 7;\n"
                 "int %s = 1;\n"
                 "void f_ce(int sizeox) {\n"
                 "  int a1[2] = { sizeof((int)), sizeof 1 };\n"
                 "  int a2[2] = { E_C, (my_td_t)2 };\n"
                 "  int a3[2] = { g_ord_var, sizeox };\n"
                 "  int a4[2] = { sizeox, 1 };\n"
                 "  int a5[2] = { %s, 1 };\n"
                 "}\n",
                 id125, id125);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(cl_buf), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  /* 5. Designated initializers (file/static/local array, struct, union, 65
        fields, long path, bitfields, malformed syntax, and OOM) */
  {
    char big_st[4096];
    char id98[104];
    char fpath_chain[256];
    size_t p = 0;
    int k;
    const char *di_snips[] = {
        "struct Pt { int x; int y; }; const struct Pt cp = { .y = 2 }; "
        "static struct Pt sp = { .x = 1 };",
        "struct UnknownT ut = { .x = 1 }; struct EmptyS {}; struct EmptyS es "
        "= { .x = 1 }; void fu(void) { struct UnknownL ul = { .x = 1 }; }",
        "int dot_num = { .123 };",
        "int multi_d = { .x = 1 }, other_v;",
        "struct Pt { int x; int y; }; void f(int v) { struct Pt p = { 1, 2 }; "
        "struct Pt (p2) = { .x = v }; {} int a[2] = { [0] = 1 }; }",
        "void f(void) { [[maybe_unused]] int a[2] = { [0] = 1 }; }",
        "void f(int v) { static int sa[4] = { [2] = 5, [1] = 3, 4 }; int "
        "la[2] = { v, 1 }; }",
        "int ga[4] = { , [0] = 1 }; int gb[4] = { [0] }; int gc[4] = { [0 1] "
        "= 2 }; int gd[4] = { [0] + 2 };",
        "int ge[200] = { [127] = 1, 2 }; int gf[200] = { [130] = 1 }; int "
        "gg[200] = { [0] = 1, [130] = 2 }; int gh[4] = { [ ] = 1 };",
        "void f(int v) { int b1[3] = { , [0] = {1}, [1] = b1[0] }; int b3[2] "
        "= { [0] + 1 }; int b4[2] = { [[maybe_unused]] , [0] = 1 }; int b5[2] "
        "= { [0] }; }",
        "struct Pt { int x; int y; }; union U { int a; float b; }; union U gu "
        "= { .a = 1 }; void f(void) { static struct Pt s = { .y = 1 }; struct "
        "Pt c = { .y = 2 }; union U lu = { .a = 3 }; }",
        "int g_not_td = 0; struct Bf { int : 4; int x; }; void f(int v) { "
        "struct Bf b = { v, 2, 3 }; struct Bf b2 = { [[maybe_unused]] , .x = v "
        "}; struct Bf b3 = { .bad + v }; g_not_td nv = { .x = v }; }",
        "struct S1 { int a; }; struct S2 { int a[2]; int b; }; int g_arr[2]; "
        "struct S1 gs1 = { .zzz = 1 }; struct S2 gs2 = { , .a = {1}, .b = "
        "g_arr[0], 99 }; struct S2 gs3 = { ., . + 1 = 2, .a + 1 };",
        "struct S2g { int a[2]; int b; }; void f(int v, int *arr) { struct "
        "S2g s = { , .a = {1}, .b = arr[0] + (v), .bad }; }"};

    for (si = 0; si < sizeof(di_snips) / sizeof(di_snips[0]); si++) {
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(di_snips[si]), &tree));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;
    }

    /* Post-parse mutations for unclosed init / dot_eof / !found_id / unclosed [
     */
    {
      size_t ti;
      for (ti = 4; ti <= 9; ti++) {
        size_t orig_n;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str("int u = { .x = 1 };"),
                                &tree));
        orig_n = tree->base_tokens->size;
        tree->base_tokens->size = ti;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
        tree->base_tokens->size = orig_n;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
        tree = NULL;
      }
      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_cst_parse(az_span_create_from_str("int u = { .x = 1 };"), &tree));
      tree->base_tokens->tokens[1].kind = CDD_TOKEN_KEYWORD_INT;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;
    }

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(
                                "void f(int v) { int b2[2] = { [0] = v }; }"),
                            &tree));
    /* Mutate `]` inside `[0] = v` so `rbk` hits `ee` */
    {
      size_t ti;
      int rbc = 0;
      for (ti = 0; ti < tree->base_tokens->size; ti++) {
        if (tree->base_tokens->tokens[ti].kind == CDD_TOKEN_RBRACKET) {
          rbc++;
          if (rbc == 2)
            tree->base_tokens->tokens[ti].kind = CDD_TOKEN_NUMBER;
        }
      }
    }
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    memset(id98, 'm', 98);
    id98[98] = '\0';
    fpath_chain[0] = '\0';
    {
      size_t fp_pos = 0;
      for (k = 0; k < 12; k++) {
        fp_pos += (size_t)CDD_SNPRINTF(
            fpath_chain + fp_pos, sizeof(fpath_chain) - fp_pos, ".abcdefghij");
      }
      fp_pos += (size_t)CDD_SNPRINTF(
          fpath_chain + fp_pos, sizeof(fpath_chain) - fp_pos, ".a.b.c.d.e");
    }

    /* >64 struct fields + >=95-char var name + >128-char multi-segment .path */
    p += (size_t)CDD_SNPRINTF(big_st, sizeof(big_st), "struct Big66 {");
    for (k = 0; k < 66; k++) {
      p += (size_t)CDD_SNPRINTF(big_st + p, sizeof(big_st) - p, " int f%d;", k);
    }
    p += (size_t)CDD_SNPRINTF(
        big_st + p, sizeof(big_st) - p,
        " }; struct Big66 gb66 = { .f0 = 1 };\n"
        "void f_long(int v) { int %s[2] = { [0] = 1 }; struct Big66 lv = { "
        "%s = v }; }\n",
        id98, fpath_chain);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(big_st), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* OOM on file-scope array designator (L628), positional struct (L881),
       and local non-const struct designator (L807) */
    {
      int saw1 = 0, saw2 = 0, saw3 = 0;
      for (fi = 1; fi <= 16; fi++) {
        cdd_c_error_t rc_o;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_parse(az_span_create_from_str(
                                                   "int ga[3] = { [1] = 2 };"),
                                               &tree));
        g_gnu_replace_fail = fi;
        rc_o = cdd_transform_c_lowering(tree, &cfg);
        g_gnu_replace_fail = 0;
        if (rc_o == CDD_C_ERROR_MEMORY)
          saw1 = 1;
        else
          ASSERT_EQ(CDD_C_SUCCESS, rc_o);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
        tree = NULL;

        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(
                      az_span_create_from_str(
                          "struct Pt { int x; int y; }; struct Pt gp = { .y = "
                          "2 };"),
                      &tree));
        g_gnu_replace_fail = fi;
        rc_o = cdd_transform_c_lowering(tree, &cfg);
        g_gnu_replace_fail = 0;
        if (rc_o == CDD_C_ERROR_MEMORY)
          saw2 = 1;
        else
          ASSERT_EQ(CDD_C_SUCCESS, rc_o);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
        tree = NULL;

        ASSERT_EQ(
            CDD_C_SUCCESS,
            cdd_cst_parse(
                az_span_create_from_str(
                    "struct Pt { int x; }; void f(int v) { struct Pt p = { .x "
                    "= v }; }"),
                &tree));
        g_gnu_replace_fail = fi;
        rc_o = cdd_transform_c_lowering(tree, &cfg);
        g_gnu_replace_fail = 0;
        if (rc_o == CDD_C_ERROR_MEMORY)
          saw3 = 1;
        else
          ASSERT_EQ(CDD_C_SUCCESS, rc_o);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
        tree = NULL;
      }
      ASSERT_EQ(1, saw1);
      ASSERT_EQ(1, saw2);
      ASSERT_EQ(1, saw3);
    }
  }

  /* 6. Mixed declarations & for-init hoister exhaustive edges + OOM */
  {
    char loops35[4096];
    char id60[65];
    size_t p = 0;
    int k;
    const char *md_snips[] = {
        "void f(void) { for; int x = 1; int y = 2; }",
        "void f(void) { for (int = 0; ; ) {} for (int , j = 0; ; ) {} }",
        "void f(void) { for (int i, a[2] = {0}, ****************p = 0, (q) = "
        "0, * = 0, k = ; ; ) {} }",
        "void f(void) { for (int * = 0; ; ) {} }",
        "struct S { int i; }; void f(struct S s, struct S *sp) { int j = 0; "
        "for (int i = 0; i < 1; i++) {} for (int i = 0; i < 1; i++) { s.i = "
        "sp->i + i + j; } }",
        "int ga[][2] = { {1, 2}, {3, 4} }; union { int a; } gu; union U { int "
        "a; }; enum E { E0 }; MAC55 fn1 { int a; } MAC4 fn2 { int b; }",
        "void f(void) { int a = { 1 }; if (1) { int c = 0; } __declspec(align("
        "(16))) int dx = 2; }",
        "typedef int mt; void f(void) { int x = 0; mt = 1; mt(1); mt; mt + 1; "
        "mt - 1; mt.f; mt->f; mt: ; }",
        "typedef int mytype10_t; enum E { E0 }; struct St { int x; }; void "
        "f(void) { int a = 1; a++; enum E e = E0; struct St; int [2]; "
        "unsigned int; unsigned int = 1; unsigned int , z = 2; unsigned int "
        "[2]; const int (y) = 3; const mytype10_t (x) = 2; int (*fp)(int) = 0; "
        "int * = 1; int k = ; int w = 1, [[maybe_unused]]; int m = "
        "[[maybe_unused]] 3; }",
        "struct P { int x; }; void f(void) { int a = 1; a++; struct P p1 = "
        "{0}, p2 = {0}; }"};

    for (si = 0; si < sizeof(md_snips) / sizeof(md_snips[0]); si++) {
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(md_snips[si]), &tree));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;
    }

    /* Fresh CST parse per truncation point for colliding `for` & `__declspec`
     */
    {
      const char *trunc_src[] = {
          "void f(void) { for (int i = 0; i < 1; i++) {} for (int i = 0; i < "
          "1; i++) { i++; } }",
          "void f(void) { for (int i = 0; i < 1; i++) {} for (int i = 0; i < "
          "1; i++) i++; }",
          "void f(void) { int a = 1; a++; __declspec(align((16))) int dx = 2; "
          "}"};
      for (si = 0; si < 3; si++) {
        size_t ti, max_n = 0;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(trunc_src[si]), &tree));
        max_n = tree->base_tokens->size;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
        tree = NULL;
        for (ti = 5; ti < max_n; ti++) {
          size_t orig_n;
          ASSERT_EQ(
              CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(trunc_src[si]), &tree));
          orig_n = tree->base_tokens->size;
          tree->base_tokens->size = ti;
          ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
          tree->base_tokens->size = orig_n;
          ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
          tree = NULL;
        }
      }
      /* Unclosed __declspec((16) before `;` in mid-block decl */
      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_cst_parse(
              az_span_create_from_str(
                  "void f(void) { int a = 1; a++; __declspec(align(16)) int dx "
                  "= 2; }"),
              &tree));
      {
        size_t ti;
        for (ti = 0; ti < tree->base_tokens->size; ti++) {
          if (tree->base_tokens->tokens[ti].kind == CDD_TOKEN_RPAREN && ti > 5)
            tree->base_tokens->tokens[ti].kind = CDD_TOKEN_IDENTIFIER;
        }
      }
      ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;
    }

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(
                                "void f(void) { for (int i = 0; ; ) {} }"),
                            &tree));
    /* Mutate `;` inside `for` to identifier so `semi >= rp_for` */
    {
      size_t ti;
      for (ti = 0; ti < tree->base_tokens->size; ti++) {
        if (tree->base_tokens->tokens[ti].kind == CDD_TOKEN_SEMICOLON)
          tree->base_tokens->tokens[ti].kind = CDD_TOKEN_IDENTIFIER;
      }
    }
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_cst_parse(
            az_span_create_from_str("void f(void) { for ( ; ; ) {} }"), &tree));
    tree->base_tokens->tokens[5].kind = CDD_TOKEN_SEMICOLON;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* Raw _Alignas token without '(' leaves ft == "_Alignas" in mixed_decls */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(
                                "void f(void) { int a = 1; _Alignas x; }"),
                            &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* >34 distinct for-loop vars in one block + >=56-char loop var */
    memset(id60, 'v', 60);
    id60[60] = '\0';
    p += (size_t)CDD_SNPRINTF(loops35, sizeof(loops35),
                              "void fl(void) { for (int %s = 0; ; ) break; ",
                              id60);
    for (k = 0; k < 35; k++) {
      p += (size_t)CDD_SNPRINTF(loops35 + p, sizeof(loops35) - p,
                                "for (int v%d = 0; v%d < 1; v%d++) {} ", k, k,
                                k);
    }
    p += (size_t)CDD_SNPRINTF(loops35 + p, sizeof(loops35) - p, "}");
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(loops35), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* OOM on hoisted mid-block static declaration (L1301-1302) */
    {
      int saw_st = 0;
      for (fi = 1; fi <= 10; fi++) {
        cdd_c_error_t rc_s;
        ASSERT_EQ(
            CDD_C_SUCCESS,
            cdd_cst_parse(
                az_span_create_from_str(
                    "void f(void) { int a = 1; a++; static int sx = 2; }"),
                &tree));
        g_gnu_replace_fail = fi;
        rc_s = cdd_transform_c_lowering(tree, &cfg);
        g_gnu_replace_fail = 0;
        if (rc_s == CDD_C_ERROR_MEMORY)
          saw_st = 1;
        else
          ASSERT_EQ(CDD_C_SUCCESS, rc_s);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
        tree = NULL;
      }
      ASSERT_EQ(1, saw_st);
    }
  }

  /* 7. Line comments, printf format strings, and legacy dialect EOF edges */
  {
    const char *ld_snips[] = {
        "//",
        "//only_cr\r",
        "//cr\r\nint x;",
        "// a*b */ *\nint x;",
        "void f(void) { const char *a = \"\"; const char *b = \"abcd\"; "
        "const char *c = \"%-+ #0*.9zu %! %: %zx %zX %zo %zi %tz %llx %llX "
        "%llo %lli %llz %ly %jd %ji %ju %jx %jX %jz %%\"; }",
        "_Alignas x; int p1; _Alignof x; int p2; _Static_assert x; int p3;",
        "_Static_assert(1);"};
    for (si = 0; si < sizeof(ld_snips) / sizeof(ld_snips[0]); si++) {
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(ld_snips[si]), &tree));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;
    }

    /* Truncated string literals ending at '\\', '%', and '%-' */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(
                  az_span_create_from_str(
                      "const char *s1 = \"a%\\\\\"; const char *s2 = \"ab%z\"; "
                      "const char *s3 = \"ab%-z\";"),
                  &tree));
    {
      size_t ti;
      int sc = 0;
      for (ti = 0; ti < tree->base_tokens->size; ti++) {
        if (tree->base_tokens->tokens[ti].kind == CDD_TOKEN_STRING) {
          sc++;
          if (sc == 1)
            tree->base_tokens->tokens[ti].length = 4; /* `"ab\` */
          else if (sc == 2)
            tree->base_tokens->tokens[ti].length = 4; /* `"ab%` */
          else if (sc == 3)
            tree->base_tokens->tokens[ti].length = 5; /* `"ab%-` */
        }
      }
    }
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    {
      const char *kw_trunc[] = {"_Alignas((16)) int x;",
                                "int a = (int)_Alignof((int));",
                                "_Static_assert((1), \"m\");"};
      for (si = 0; si < 3; si++) {
        size_t ti, orig_n;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(kw_trunc[si]), &tree));
        orig_n = tree->base_tokens->size;
        for (ti = 1; ti < orig_n; ti++) {
          tree->base_tokens->size = ti;
          ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
        }
        tree->base_tokens->size = orig_n;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
        tree = NULL;
      }
    }
  }

  PASS();
}

/**
 * @brief Covers cross-pass CST synchronization, `cdd_cst_reparse_in_place`,
 * `__int128` scalar literal initializers, mid-block tag/aggregate hoisting,
 * and OOM/branch paths across `gnu_standardizer` and `c_lowering`.
 *
 * @return Greatest test result.
 */
TEST test_cross_pass_pipeline_and_int128_coverage(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_transform_config_t cfg;
  int fi;

  /* 1. Direct cdd_cst_reparse_in_place NULL + OOM + success */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_reparse_in_place(NULL));
  for (fi = 1; fi <= 25; fi++) {
    cdd_c_error_t rc_rp;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("int x = 1;\n"), &tree));
    g_cdd_alloc_fail = fi;
    rc_rp = cdd_cst_reparse_in_place(tree);
    g_cdd_alloc_fail = 0;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
    if (rc_rp == CDD_C_SUCCESS)
      break;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc_rp);
  }

  /* 2. Multi-pass pipeline synchronization + OOM across passes */
  {
    char pipe_src[1536];
    const char *p1 = "int g_vla_bound = 4;\n"
                     "int gv1 = 1, gv2 = 2;\n"
                     "static void cl_fn(int *p) { if (p) *p = 0; }\n"
                     "int pipe_fn(int n) {\n"
                     "  int outer_t = n + 1;\n"
                     "  int se = ({ outer_t * 3; });\n"
                     "  int sep = 1'000 + 0x1'00uLL;\n"
                     "  unsigned __int128 u_pos = 10, u_neg = -5, u_expr = se "
                     "+ 1;\n"
                     "  __int128 s_neg = -7, s_pos = 8;\n";
    const char *p2 =
        "  unsigned __int128 u_big = 0x1'0000'0000'0000'0000ULL;\n"
        "  unsigned __int128 u_sum = u_pos + -u_big;\n"
        "  int tc = __builtin_types_compatible_p(gv1, gv2);\n"
        "  int vla[g_vla_bound];\n"
        "  vla[0] = se + sep + tc + (int)u_sum.low + (int)s_neg.low + "
        "(int)s_pos.low + (int)u_neg.low + (int)u_expr.low;\n"
        "  struct MidS { int x; int y; };\n"
        "  union MidU { int a; };\n"
        "  enum MidE { ME_1 = 1 };\n";
    const char *p3 = "  typedef int mid_td_t;\n"
                     "  static int mid_stat = 3;\n"
                     "  struct MidS ms = { .y = 20, .x = 10 };\n"
                     "  struct MidS ms2 = (struct MidS){ 5, 6 };\n"
                     "  union MidU mu = (union MidU){ 9 };\n"
                     "  int arr[3] = { 1, 2, 3 };\n"
                     "  mid_td_t sc_cl = (int){ n + 2 };\n"
                     "  int emp_cl = (int){};\n"
                     "  double _Complex z = 1.0 + 2.0i;\n"
                     "  int __attribute__((cleanup(cl_fn))) g = 5;\n"
                     "  if (n > 99) {}\n"
                     "  return vla[0] + ms.x + ms2.y + mu.a + arr[0] + sc_cl + "
                     "emp_cl + mid_stat + ME_1 + (int)awm_creal(z) + g;\n"
                     "}\n";
    CDD_SNPRINTF(pipe_src, sizeof(pipe_src), "%s%s%s", p1, p2, p3);
    memset(&cfg, 0, sizeof(cfg));
    cfg.target_profile = CDD_TARGET_PROFILE_MSVC_LEGACY;
    cfg.target_c89 = 1;
    cfg.with_auto_win_msvc = 1;

    for (fi = 1; fi <= 160; fi++) {
      cdd_c_error_t rc_p;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(pipe_src), &tree));
      g_cdd_alloc_fail = fi;
      rc_p = cdd_transform_pipeline(
          tree, CDD_PASS_GNU | CDD_PASS_C_LOWERING | CDD_PASS_MSVC_PORT, &cfg);
      g_cdd_alloc_fail = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;
      if (rc_p == CDD_C_SUCCESS)
        break;
    }

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(pipe_src), &tree));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_transform_pipeline(
                  tree, CDD_PASS_GNU | CDD_PASS_C_LOWERING | CDD_PASS_MSVC_PORT,
                  &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  /* 3. Edge branches in __int128 initializers, cleanup returns, and stmt_0 CL
   */
  {
    const char *edges[] = {
        "int x = 1;\n",
        "typedef unk_nm alias_t;\n"
        "int unk_nm = 1;\n"
        "int c_tc = __builtin_types_compatible_p(alias_t, alias_t);\n",
        "__int128 fn128(__int128 p);\n"
        "__int128 g_br = { 1, 0 };\n"
        "__int128 g_fl = 1.5;\n"
        "float g_10f = 10f;\n"
        "__int128 g_pexpr = 1 + 2;\n"
        "__int128 g_nsemi = -7;\n"
        "__int128 g_huge = 9999999999999999999999999999999999999;\n"
        "__int128 g_nhuge = -999999999999999999999999999;\n"
        "__int128 g_nexpr = -1 + 2;\n"
        "int hex_bad = 0x;\n"
        "unsigned __int128 f_chain(unsigned __int128 a, unsigned __int128 b, "
        "unsigned __int128 c) { return a + b - c; }\n",
        "int shadow_t = 0;\n"
        "int f_sh(void) { typedef int shadow_t; return "
        "__builtin_types_compatible_p(shadow_t, shadow_t); }\n",
        "__int128 g_eof = 10",
        "__int128 g_neof = -5",
        "__int128 g_meof = -",
        "__int128 g_aeof =",
        "void cl(int *p) {}\n"
        "void f_BraceRet(int n) {\n"
        "  int __attribute__((cleanup(cl))) x = 1;\n"
        "  int v[n];\n"
        "  { return; }\n"
        "  { int __attribute__((cleanup(cl))) y = 2; return; }\n"
        "  if (n) {} return;\n"
        "}\n",
        "void cl(int *p) {}\n"
        "void f_InnerOnly(int n) {\n"
        "  { int __attribute__((cleanup(cl))) y = 2; int v2[n]; return; }\n"
        "}\n"
        "void f_OuterVlaInnerRet(int n) {\n"
        "  int v1[n];\n"
        "  { return; }\n"
        "}\n"
        "void f_CondEndRet(int n) {\n"
        "  int __attribute__((cleanup(cl))) x = 1;\n"
        "  if (n) return;\n"
        "}\n"
        "void f_AfterRet(void) {\n"
        "  int __attribute__((cleanup(cl))) x = 1;\n"
        "  return; x = 2;\n"
        "}\n",
        "int f_stmt0_cl(int n) {\n"
        "  n++;\n"
        "  lbl7chr: { n++; }\n"
        "  struct S { int v; };\n"
        "  union U { int v; };\n"
        "  typedef struct { int q; } td_agg_t;\n"
        "  struct { int anon_q; };\n"
        "  struct S s_init = { 1 };\n"
        "  int m1 = 1, arr2[2] = { 3, 4 };\n"
        "  return s_init.v + m1 + arr2[0];\n"
        "}\n"
        "void f_tag_var(void) { int a = 1; a++; struct S { int v; } sv; sv.v = "
        "a; }\n"};
    const char *cl_direct =
        "int f_cld(int n) {\n"
        "  struct S { int v; };\n"
        "  union U { int v; };\n"
        "  n++;\n"
        "  lbl7chr: { n++; }\n"
        "  typedef struct { int q; } td_agg_t;\n"
        "  struct { int anon_q; };\n"
        "  struct MidT { int z; };\n"
        "  int m1 = 1, arr2[2] = { 3, 4 };\n"
        "  return m1 + arr2[0] + (int){} + (struct S){ n }.v + "
        "(union U){ n }.v;\n"
        "}\n";
    size_t ei;
    memset(&cfg, 0, sizeof(cfg));
    cfg.target_profile = CDD_TARGET_PROFILE_MSVC_LEGACY;
    cfg.target_c89 = 1;
    cfg.fallback_vla_to_malloc = 1;
    cfg.vla_mode = CDD_VLA_MODE_MALLOC;
    for (ei = 0; ei < sizeof(edges) / sizeof(edges[0]); ei++) {
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(edges[ei]), &tree));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_transform_pipeline(tree, CDD_PASS_GNU | CDD_PASS_C_LOWERING,
                                       &cfg));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;
      for (fi = 1; fi <= 300; fi++) {
        cdd_c_error_t rc_e;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(edges[ei]), &tree));
        g_cdd_alloc_fail = fi;
        rc_e = cdd_transform_pipeline(tree, CDD_PASS_GNU | CDD_PASS_C_LOWERING,
                                      &cfg);
        g_cdd_alloc_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
        tree = NULL;
        if (rc_e == CDD_C_SUCCESS)
          break;
      }
    }
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(cl_direct), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
    for (fi = 1; fi <= 200; fi++) {
      cdd_c_error_t rc_d;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(cl_direct), &tree));
      g_cdd_alloc_fail = fi;
      rc_d = cdd_transform_c_lowering(tree, &cfg);
      g_cdd_alloc_fail = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;
      if (rc_d == CDD_C_SUCCESS)
        break;
    }

    /* Truncated return without trailing `}` in gnu_standardizer_unroll */
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_cst_parse(az_span_create_from_str(
                          "void cl(int *p) {} void f_RetNoBrace(void) { int "
                          "__attribute__((cleanup(cl))) x = 1; return; }"),
                      &tree));
    tree->base_tokens->size--;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tree, &cfg));
    tree->base_tokens->size++;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* Direct C_LOWERING global-var VLA bound + NULL-config GNU unroll + "u" */
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_cst_parse(
            az_span_create_from_str(
                "int g_vb = 4; void fv(void) { int a[g_vb]; a[0] = 1; }\n"),
            &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_cst_parse(az_span_create_from_str(
                          "void cl(int *p){} void fn(void){ int "
                          "__attribute__((cleanup(cl))) x = 1; return; }\n"),
                      &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tree, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* Synthetic suffix-only CDD_TOKEN_NUMBER "u" for cdd_cst_type_eval.c */
    {
      cdd_token_t su_tok;
      char tbuf[32];
      memset(&su_tok, 0, sizeof(su_tok));
      su_tok.kind = CDD_TOKEN_NUMBER;
      su_tok.start = (const uint8_t *)"u";
      su_tok.length = 1;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_eval_tokens_type(NULL, NULL, &su_tok, 1,
                                         CDD_CST_TYPE_MODE_RVALUE, tbuf,
                                         sizeof(tbuf), NULL));
    }
  }

  PASS();
}

/**
 * @brief Regression and OOM test for multi-pass `_Atomic(T)` + `default: }` +
 * multi-dimensional VLA indexing + designated compound literals + non-returning
 * VLA scope exit.
 * @return Greatest test result.
 */
TEST test_c_lowering_multidim_vla_cl_desig_and_gnu_pipeline(void) {
  cdd_cst_tree_t *tree = NULL;
  char *out = NULL;
  cdd_transform_config_t cfg;

  /* 1. Multi-pass GNU + C89 pipeline: `_Atomic(int)`, `default: }`, `else {`,
        nested-bracket 2D VLA `m[a[0]][1]`, 1D slice `m[0]`, and designated
        compound literal with nested `{ ... }` and `( ... )` */
  {
    const char *s1 = "#include <stdio.h>\n"
                     "#include <stdckdint.h>\n"
                     "struct Inner { int k; };\n"
                     "struct Outer { struct Inner in; int v; int w; };\n"
                     "_Atomic(int) g_at = 0;\n";
    const char *s2 =
        "int pipeline_fn(int n, int *a) {\n"
        "  struct abcd { int z; };\n"
        "  int v1[n];\n"
        "  int m[n][2];\n"
        "  int *slice;\n"
        "  struct Outer o = (struct Outer){ 0, (1 + 2), .in = { 1 }, .v = (2 + "
        "3), .in.k };\n"
        "  v1[0] = o.v;\n"
        "  a[0] = v1[0];\n"
        "  m[a[0]][1] = v1[0];\n"
        "  slice = m[0];\n"
        "  switch (n) { case 1: g_at += 1; break; default: }\n"
        "  if (n > 2) { return m[0][1] + (slice != 0); } else { return g_at; "
        "}\n"
        "}\n";
    char full_s[1536];
    CDD_SNPRINTF(full_s, sizeof(full_s), "%s%s", s1, s2);
    memset(&cfg, 0, sizeof(cfg));
    cfg.target_profile = CDD_TARGET_PROFILE_ISO_C89;
    cfg.vla_mode = CDD_VLA_MODE_MALLOC;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(full_s), &tree));
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_transform_pipeline(tree, CDD_PASS_GNU | CDD_PASS_C_LOWERING, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tree, &out));

    ASSERT(strstr(out, "#include <stdio.h>") != NULL);
    ASSERT(strstr(out, "volatile int g_at = 0;") != NULL);
    ASSERT(strstr(out, "default: ;") != NULL);
    ASSERT(strstr(out, "m[(((a[0])) * (2) + (1))]") != NULL);
    ASSERT(strstr(out, "__cdd_ci_1.in = { 1 };") != NULL);
    ASSERT(strstr(out, "__cdd_ci_1.v = (2 + 3);") != NULL);

    free(out);
    out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  /* 2. Truncated multi-dim VLA subscript token-boundary guards */
  {
    size_t saved_sz;
    size_t trim;
    memset(&cfg, 0, sizeof(cfg));
    cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;
    for (trim = 5; trim <= 12; trim++) {
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(
                                  "void tf1(int n){int m[n][2];m[0+1][1]=1;}"),
                              &tree));
      saved_sz = tree->base_tokens->size;
      tree->base_tokens->size = saved_sz - trim;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
      tree->base_tokens->size = saved_sz;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;
    }
  }

#ifdef CDD_BUILD_TESTS
  /* 3. OOM sweeps for `default: }`, 2D VLA subscript rewrite, and non-returning
        VLA scope-exit `free(...)` */
  {
    int fail_at;
    int fk;
    const char *gnu_lbl_src = "void f(int x) { switch (x) { default: } }\n";
    const char *vla_oom_src =
        "void vf(int n) { int v[n]; int m[n][2]; m[0][1] = 5; v[0] = m[0][1]; "
        "}\n";
    const char *vla_min_src = "void vf_min(int n) { int v[n]; }\n";
    for (fk = 0; fk < 2; fk++) {
      for (fail_at = 1; fail_at <= 20; fail_at++) {
        cdd_c_error_t rc;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(gnu_lbl_src), &tree));
        if (fk == 0)
          g_cdd_alloc_fail = fail_at;
        else
          g_cdd_cst_alloc_token_fail = fail_at;
        rc = cdd_transform_gnu(tree, NULL);
        g_cdd_alloc_fail = 0;
        g_cdd_cst_alloc_token_fail = 0;
        ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_MEMORY);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
        tree = NULL;
      }
    }
    memset(&cfg, 0, sizeof(cfg));
    cfg.target_profile = CDD_TARGET_PROFILE_ISO_C89;
    cfg.vla_mode = CDD_VLA_MODE_MALLOC;
    for (fk = 0; fk < 2; fk++) {
      for (fail_at = 1; fail_at <= 60; fail_at++) {
        cdd_c_error_t rc;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(vla_oom_src), &tree));
        if (fk == 0)
          g_cdd_alloc_fail = fail_at;
        else
          g_cdd_cst_alloc_token_fail = fail_at;
        rc = cdd_transform_c_lowering(tree, &cfg);
        g_cdd_alloc_fail = 0;
        g_cdd_cst_alloc_token_fail = 0;
        ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_MEMORY);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
        tree = NULL;

        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(vla_min_src), &tree));
        if (fk == 0)
          g_cdd_alloc_fail = fail_at;
        else
          g_cdd_cst_alloc_token_fail = fail_at;
        rc = cdd_transform_c_lowering(tree, &cfg);
        g_cdd_alloc_fail = 0;
        g_cdd_cst_alloc_token_fail = 0;
        ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_MEMORY);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
        tree = NULL;
      }
    }
  }
#endif

  PASS();
}

/**
 * @brief Covers >=31-char designated array index, >=127-char VLA function name,
 * and >=63-char C23 attribute identifier truncation bounds.
 * @return Greatest test result.
 */
TEST test_c_lowering_long_tokens_and_attr_caps(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_transform_config_t cfg;
  char id132[136];
  char attr70[72];
  char src[1024];

  memset(id132, 'f', 132);
  id132[132] = '\0';
  memset(attr70, 'a', 70);
  attr70[70] = '\0';

  CDD_SNPRINTF(
      src, sizeof(src),
      "[[%s]] [[gnu::%s]] void %s(int n) {\n"
      "  int v[n];\n"
      "  static int arr[4] = { [00000000000000000000000000000000001] = 7 };\n"
      "  v[0] = arr[1];\n"
      "}\n",
      attr70, attr70, id132);

  memset(&cfg, 0, sizeof(cfg));
  cfg.target_profile = CDD_TARGET_PROFILE_ISO_C89;
  cfg.target_c89 = 1;
  cfg.vla_mode = CDD_VLA_MODE_MALLOC;

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_parse(az_span_create_from_str(src), &tree));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, &cfg));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
  PASS();
}

/**
 * @brief Suite registration for `transformer_c_lowering_suite`.
 */
SUITE(transformer_c_lowering_suite) {
  RUN_TEST(test_c_lowering_phase_3_1_modern_and_vla_hybrid);
  RUN_TEST(test_c_lowering_phase_3_2_legacy_msvc_c89);
  RUN_TEST(test_c_lowering_phase_3_3_interop);
  RUN_TEST(test_c_lowering_errors_and_oom);
  RUN_TEST(test_c_lowering_branches_and_pipeline_c89);
  RUN_TEST(test_c_lowering_golden_exhaustive_legacy_modern);
  RUN_TEST(test_c_lowering_complete_coverage);
  RUN_TEST(test_c_lowering_modern_branch_completion);
  RUN_TEST(test_c_lowering_legacy_branch_completion);
  RUN_TEST(test_cross_pass_pipeline_and_int128_coverage);
  RUN_TEST(test_c_lowering_multidim_vla_cl_desig_and_gnu_pipeline);
  RUN_TEST(test_c_lowering_long_tokens_and_attr_caps);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_TRANSFORM_C_LOWERING_H */
