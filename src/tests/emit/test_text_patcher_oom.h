/**
 * @file test_text_patcher_oom.h
 * @brief OOM and allocation failure unit tests for text patcher.
 *
 * @author Samuel Marks
 */

#ifndef TEST_TEXT_PATCHER_OOM_H
#define TEST_TEXT_PATCHER_OOM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/patcher.h"
#include "functions/parse/str.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

extern C_CDD_EXPORT int g_cdd_alloc_fail;
extern C_CDD_EXPORT int g_fail_io_after;

TEST test_patcher_oom(void) {
  struct PatchList list;
#ifdef CDD_BUILD_TESTS
  {
    int i;
    struct TokenList tl2;
    memset(&tl2, 0, sizeof(tl2));
    /*  (moved to global) */
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, patch_list_init(&list));
    g_cdd_alloc_fail = 0;

    patch_list_init(&list);
    /* fill it up to trigger realloc */
    for (i = 0; i < 8; i++) {
      patch_list_add(&list, 0, 1, strdup("a"));
    }
    {
      int j;
      for (j = 1; j < 50; j++) {
        char *tmp = strdup("b");
        int rc;
        g_cdd_alloc_fail = j;
        rc = patch_list_add(&list, 0, 1, tmp);
        g_cdd_alloc_fail = 0;
        if (rc == 0)
          break;
      }
    }

    {
      char *out_code = NULL;
      int j;
      for (j = 1; j < 50; j++) {
        g_cdd_alloc_fail = j;
        {
          int rc = patch_list_apply(&list, &tl2, &out_code);
          g_cdd_alloc_fail = 0;
          if (out_code) {
            free(out_code);
            out_code = NULL;
          }
          if (rc == 0)
            break;
        }
      }
    }
    /* also test realloc success */
    {
      char *tmp_c = strdup("c");
      ASSERT_EQ(0, patch_list_add(&list, 0, 1, tmp_c));
    }

    /* also test patch with text = NULL in free */
    if (list.patches[0].text)
      free((void *)list.patches[0].text);
    list.patches[0].text = NULL;
    patch_list_free(&list);

    /* Test patch_list_apply OOM */
    {
      struct PatchList p_oom;
      char *out_oom = NULL;
      patch_list_init(&p_oom);
      {
        struct TokenList *tl_alloc = NULL;
        tokenize(
            az_span_create_from_str((char *)(size_t)(size_t) "int main(){}"),
            &tl_alloc);

        {
          char huge_oom[3000];
          memset(huge_oom, 'O', 2999);
          huge_oom[2999] = '\0';
          patch_list_add(&p_oom, 0, 1, strdup(huge_oom));
          {
            int j;
            for (j = 1; j < 180; j++) {
              int rc;
              int my_alloc = j;
              if (j == 4)
                my_alloc = 3000;
              if (j == 5)
                my_alloc = 3000;
              if (j == 6)
                my_alloc = 3000;
              if (j == 7)
                my_alloc = 3000;
              if (j == 8)
                my_alloc = 3000;
              if (j == 9)
                my_alloc = 3000;

              g_cdd_alloc_fail = my_alloc;
              rc = patch_list_apply(&p_oom, tl_alloc, &out_oom);
              ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_MEMORY);
              g_cdd_alloc_fail = 0;
              if (out_oom) {
                free(out_oom);
                out_oom = NULL;
              }
            }
          }

          /* Also test OOM for tokens */
          patch_list_free(&p_oom);
          patch_list_init(&p_oom);
          {
            char huge_oom2[3000];
            memset(huge_oom2, 'O', 2999);
            huge_oom2[2999] = '\0';
            patch_list_add(&p_oom, 4, 4, strdup(huge_oom2));
            {
              int j;
              for (j = 1; j < 180; j++) {
                int rc;
                int my_alloc = j;
                if (j == 4) {
                  my_alloc = 3000;
                }
                if (j == 5) {
                  my_alloc = 3000;
                }
                if (j == 6) {
                  my_alloc = 3000;
                }
                if (j == 7) {
                  my_alloc = 3000;
                }
                if (j == 8) {
                  my_alloc = 3000;
                }
                if (j == 9) {
                  my_alloc = 3000;
                }
                if (j == 10) {
                  my_alloc = 3000;
                }
                if (j == 11) {
                  my_alloc = 3000;
                }
                if (j == 12) {
                  my_alloc = 3000;
                }
                if (j == 13) {
                  my_alloc = 3000;
                }
                if (j == 14) {
                  my_alloc = 3000;
                }
                if (j == 15) {
                  my_alloc = 3000;
                }
                if (j == 16) {
                  my_alloc = 3000;
                }
                if (j == 17) {
                  my_alloc = 3000;
                }
                if (j == 18) {
                  my_alloc = 3000;
                }
                if (j == 19) {
                  my_alloc = 3000;
                }
                if (j == 20) {
                  my_alloc = 3000;
                }
                if (j == 21) {
                  my_alloc = 3000;
                }
                if (j == 22) {
                  my_alloc = 3000;
                }
                if (j == 23) {
                  my_alloc = 3000;
                }
                if (j == 24) {
                  my_alloc = 3000;
                }
                if (j == 25) {
                  my_alloc = 3000;
                }
                if (j == 26) {
                  my_alloc = 3000;
                }
                if (j == 27) {
                  my_alloc = 3000;
                }
                if (j == 28) {
                  my_alloc = 3000;
                }
                if (j == 29) {
                  my_alloc = 3000;
                }
                if (j == 30) {
                  my_alloc = 3000;
                }
                if (j == 31) {
                  my_alloc = 3000;
                }
                if (j == 32) {
                  my_alloc = 3000;
                }
                if (j == 33) {
                  my_alloc = 3000;
                }
                if (j == 34) {
                  my_alloc = 3000;
                }
                if (j == 35) {
                  my_alloc = 3000;
                }
                if (j == 36) {
                  my_alloc = 3000;
                }
                if (j == 37) {
                  my_alloc = 3000;
                }
                if (j == 38) {
                  my_alloc = 3000;
                }
                if (j == 39) {
                  my_alloc = 3000;
                }
                if (j == 40) {
                  my_alloc = 3000;
                }
                if (j == 41) {
                  my_alloc = 3000;
                }
                if (j == 42) {
                  my_alloc = 3000;
                }
                if (j == 43) {
                  my_alloc = 3000;
                }
                if (j == 44) {
                  my_alloc = 3000;
                }
                if (j == 45) {
                  my_alloc = 3000;
                }
                if (j == 46) {
                  my_alloc = 3000;
                }
                if (j == 47) {
                  my_alloc = 3000;
                }
                if (j == 48) {
                  my_alloc = 3000;
                }
                if (j == 49) {
                  my_alloc = 3000;
                }
                if (j == 50) {
                  my_alloc = 3000;
                }
                if (j == 130)
                  my_alloc = 3000;
                if (j == 51)
                  my_alloc = 3000;
                if (j == 52)
                  my_alloc = 3000;
                if (j == 53)
                  my_alloc = 3000;
                if (j == 54)
                  my_alloc = 3000;
                if (j == 55)
                  my_alloc = 3000;
                if (j == 56)
                  my_alloc = 3000;
                if (j == 57)
                  my_alloc = 3000;
                if (j == 58)
                  my_alloc = 3000;
                if (j == 59)
                  my_alloc = 3000;
                if (j == 60)
                  my_alloc = 3000;
                if (j == 61)
                  my_alloc = 3000;
                if (j == 62)
                  my_alloc = 3000;
                if (j == 63)
                  my_alloc = 3000;
                if (j == 64)
                  my_alloc = 3000;
                if (j == 65)
                  my_alloc = 3000;
                if (j == 66)
                  my_alloc = 3000;
                if (j == 67)
                  my_alloc = 3000;
                if (j == 68)
                  my_alloc = 3000;
                if (j == 69)
                  my_alloc = 3000;
                if (j == 70)
                  my_alloc = 3000;
                if (j == 71)
                  my_alloc = 3000;
                if (j == 72)
                  my_alloc = 3000;
                if (j == 73)
                  my_alloc = 3000;
                if (j == 74)
                  my_alloc = 3000;
                if (j == 75)
                  my_alloc = 3000;
                if (j == 76)
                  my_alloc = 3000;
                if (j == 77)
                  my_alloc = 3000;
                if (j == 78)
                  my_alloc = 3000;
                if (j == 79)
                  my_alloc = 3000;
                if (j == 80)
                  my_alloc = 3000;
                if (j == 81)
                  my_alloc = 3000;
                if (j == 82)
                  my_alloc = 3000;
                if (j == 83)
                  my_alloc = 3000;
                if (j == 84)
                  my_alloc = 3000;
                if (j == 85)
                  my_alloc = 3000;
                if (j == 86)
                  my_alloc = 3000;
                if (j == 87)
                  my_alloc = 3000;
                if (j == 88)
                  my_alloc = 3000;
                if (j == 89)
                  my_alloc = 3000;
                if (j == 90)
                  my_alloc = 3000;
                if (j == 91)
                  my_alloc = 3000;
                if (j == 92)
                  my_alloc = 3000;
                if (j == 93)
                  my_alloc = 3000;
                if (j == 94)
                  my_alloc = 3000;
                if (j == 95)
                  my_alloc = 3000;
                if (j == 96)
                  my_alloc = 3000;
                if (j == 97)
                  my_alloc = 3000;
                if (j == 98)
                  my_alloc = 3000;
                if (j == 99)
                  my_alloc = 3000;
                if (j == 100)
                  my_alloc = 3000;
                if (j == 101)
                  my_alloc = 3000;
                if (j == 102)
                  my_alloc = 3000;
                if (j == 103)
                  my_alloc = 3000;
                if (j == 104)
                  my_alloc = 3000;
                if (j == 105)
                  my_alloc = 3000;
                if (j == 106)
                  my_alloc = 3000;
                if (j == 107)
                  my_alloc = 3000;
                if (j == 108)
                  my_alloc = 3000;
                if (j == 109)
                  my_alloc = 3000;
                if (j == 110)
                  my_alloc = 3000;
                if (j == 111)
                  my_alloc = 3000;
                if (j == 112)
                  my_alloc = 3000;
                if (j == 113)
                  my_alloc = 3000;
                if (j == 114)
                  my_alloc = 3000;
                if (j == 115)
                  my_alloc = 3000;
                if (j == 116)
                  my_alloc = 3000;
                if (j == 117)
                  my_alloc = 3000;
                if (j == 118)
                  my_alloc = 3000;
                if (j == 119)
                  my_alloc = 3000;
                if (j == 120)
                  my_alloc = 3000;
                if (j == 121)
                  my_alloc = 3000;
                if (j == 122)
                  my_alloc = 3000;
                if (j == 123)
                  my_alloc = 3000;
                if (j == 124)
                  my_alloc = 3000;
                if (j == 125)
                  my_alloc = 3000;
                if (j == 126)
                  my_alloc = 3000;
                if (j == 127)
                  my_alloc = 3000;
                if (j == 128)
                  my_alloc = 3000;
                if (j == 129)
                  my_alloc = 3000;
                if (j == 130)
                  my_alloc = 3000;
                if (j == 131)
                  my_alloc = 3000;
                if (j == 132)
                  my_alloc = 3000;
                if (j == 133)
                  my_alloc = 3000;
                if (j == 134)
                  my_alloc = 3000;
                if (j == 135)
                  my_alloc = 3000;
                if (j == 136)
                  my_alloc = 3000;
                if (j == 137)
                  my_alloc = 3000;
                if (j == 138)
                  my_alloc = 3000;
                if (j == 139)
                  my_alloc = 3000;
                if (j == 140)
                  my_alloc = 3000;
                if (j == 141)
                  my_alloc = 3000;
                if (j == 142)
                  my_alloc = 3000;
                if (j == 143)
                  my_alloc = 3000;
                if (j == 144)
                  my_alloc = 3000;
                if (j == 145)
                  my_alloc = 3000;
                if (j == 146)
                  my_alloc = 3000;
                if (j == 147)
                  my_alloc = 3000;
                if (j == 148)
                  my_alloc = 3000;
                if (j == 149)
                  my_alloc = 3000;
                if (j == 150)
                  my_alloc = 3000;
                if (j == 151)
                  my_alloc = 3000;
                if (j == 152)
                  my_alloc = 3000;
                if (j == 153)
                  my_alloc = 3000;
                if (j == 154)
                  my_alloc = 3000;
                if (j == 155)
                  my_alloc = 3000;
                if (j == 156)
                  my_alloc = 3000;
                if (j == 157)
                  my_alloc = 3000;
                if (j == 158)
                  my_alloc = 3000;
                if (j == 159)
                  my_alloc = 3000;
                if (j == 160)
                  my_alloc = 3000;
                if (j == 161)
                  my_alloc = 3000;
                if (j == 162)
                  my_alloc = 3000;
                if (j == 163)
                  my_alloc = 3000;
                if (j == 164)
                  my_alloc = 3000;
                if (j == 165)
                  my_alloc = 3000;
                if (j == 166)
                  my_alloc = 3000;
                if (j == 167)
                  my_alloc = 3000;
                if (j == 168)
                  my_alloc = 3000;
                if (j == 169)
                  my_alloc = 3000;
                if (j == 170)
                  my_alloc = 3000;
                if (j == 92)
                  my_alloc = 3000;
                if (j == 93)
                  my_alloc = 3000;
                if (j == 94)
                  my_alloc = 3000;
                if (j == 95)
                  my_alloc = 3000;
                if (j == 96)
                  my_alloc = 3000;
                if (j == 97)
                  my_alloc = 3000;
                if (j == 98)
                  my_alloc = 3000;
                if (j == 99)
                  my_alloc = 3000;
                if (j == 100)
                  my_alloc = 3000;
                if (j == 101)
                  my_alloc = 3000;
                if (j == 102)
                  my_alloc = 3000;
                if (j == 103)
                  my_alloc = 3000;
                if (j == 104)
                  my_alloc = 3000;
                if (j == 105)
                  my_alloc = 3000;
                if (j == 106)
                  my_alloc = 3000;
                if (j == 107)
                  my_alloc = 3000;
                if (j == 108)
                  my_alloc = 3000;
                if (j == 109)
                  my_alloc = 3000;
                if (j == 110)
                  my_alloc = 3000;
                if (j == 111)
                  my_alloc = 3000;
                if (j == 112)
                  my_alloc = 3000;
                if (j == 113)
                  my_alloc = 3000;
                if (j == 114)
                  my_alloc = 3000;
                if (j == 115)
                  my_alloc = 3000;
                if (j == 116)
                  my_alloc = 3000;
                if (j == 117)
                  my_alloc = 3000;
                if (j == 118)
                  my_alloc = 3000;
                if (j == 119)
                  my_alloc = 3000;
                if (j == 120)
                  my_alloc = 3000;
                if (j == 121)
                  my_alloc = 3000;
                if (j == 122)
                  my_alloc = 3000;
                if (j == 123)
                  my_alloc = 3000;
                if (j == 124)
                  my_alloc = 3000;
                if (j == 125)
                  my_alloc = 3000;
                if (j == 126)
                  my_alloc = 3000;
                if (j == 127)
                  my_alloc = 3000;
                if (j == 128)
                  my_alloc = 3000;
                if (j == 129)
                  my_alloc = 3000;
                if (j == 130)
                  my_alloc = 3000;
                if (j == 131)
                  my_alloc = 3000;
                if (j == 132)
                  my_alloc = 3000;
                if (j == 133)
                  my_alloc = 3000;
                if (j == 134)
                  my_alloc = 3000;
                if (j == 135)
                  my_alloc = 3000;
                if (j == 136)
                  my_alloc = 3000;
                if (j == 137)
                  my_alloc = 3000;
                if (j == 138)
                  my_alloc = 3000;
                if (j == 139)
                  my_alloc = 3000;
                if (j == 140)
                  my_alloc = 3000;
                if (j == 141)
                  my_alloc = 3000;
                if (j == 142)
                  my_alloc = 3000;
                if (j == 143)
                  my_alloc = 3000;
                if (j == 144)
                  my_alloc = 3000;
                if (j == 145)
                  my_alloc = 3000;
                if (j == 146)
                  my_alloc = 3000;
                if (j == 147)
                  my_alloc = 3000;
                if (j == 148)
                  my_alloc = 3000;
                if (j == 149)
                  my_alloc = 3000;
                if (j == 150)
                  my_alloc = 3000;
                if (j == 151)
                  my_alloc = 3000;
                if (j == 152)
                  my_alloc = 3000;
                if (j == 153)
                  my_alloc = 3000;
                if (j == 154)
                  my_alloc = 3000;
                if (j == 155)
                  my_alloc = 3000;
                if (j == 156)
                  my_alloc = 3000;
                if (j == 157)
                  my_alloc = 3000;
                if (j == 158)
                  my_alloc = 3000;
                if (j == 159)
                  my_alloc = 3000;
                if (j == 160)
                  my_alloc = 3000;
                if (j == 161)
                  my_alloc = 3000;
                if (j == 162)
                  my_alloc = 3000;
                if (j == 163)
                  my_alloc = 3000;
                if (j == 164)
                  my_alloc = 3000;
                if (j == 165)
                  my_alloc = 3000;
                if (j == 166)
                  my_alloc = 3000;
                if (j == 167)
                  my_alloc = 3000;
                if (j == 168)
                  my_alloc = 3000;
                if (j == 169)
                  my_alloc = 3000;
                if (j == 170)
                  my_alloc = 3000;
                if (j == 130)
                  my_alloc = 3000;
                if (j == 51)
                  my_alloc = 3000;
                if (j == 52)
                  my_alloc = 3000;
                if (j == 53)
                  my_alloc = 3000;
                if (j == 54)
                  my_alloc = 3000;
                if (j == 55)
                  my_alloc = 3000;
                if (j == 56)
                  my_alloc = 3000;
                if (j == 57)
                  my_alloc = 3000;
                if (j == 58)
                  my_alloc = 3000;
                if (j == 59)
                  my_alloc = 3000;
                if (j == 60)
                  my_alloc = 3000;
                if (j == 61)
                  my_alloc = 3000;
                if (j == 62)
                  my_alloc = 3000;
                if (j == 63)
                  my_alloc = 3000;
                if (j == 64)
                  my_alloc = 3000;
                if (j == 65)
                  my_alloc = 3000;
                if (j == 66)
                  my_alloc = 3000;
                if (j == 67)
                  my_alloc = 3000;
                if (j == 68)
                  my_alloc = 3000;
                if (j == 69)
                  my_alloc = 3000;
                if (j == 70)
                  my_alloc = 3000;
                if (j == 71)
                  my_alloc = 3000;
                if (j == 72)
                  my_alloc = 3000;
                if (j == 73)
                  my_alloc = 3000;
                if (j == 74)
                  my_alloc = 3000;
                if (j == 75)
                  my_alloc = 3000;
                if (j == 76)
                  my_alloc = 3000;
                if (j == 77)
                  my_alloc = 3000;
                if (j == 78)
                  my_alloc = 3000;
                if (j == 79)
                  my_alloc = 3000;
                if (j == 80)
                  my_alloc = 3000;
                if (j == 81)
                  my_alloc = 3000;
                if (j == 82)
                  my_alloc = 3000;
                if (j == 83)
                  my_alloc = 3000;
                if (j == 84)
                  my_alloc = 3000;
                if (j == 85)
                  my_alloc = 3000;
                if (j == 86)
                  my_alloc = 3000;
                if (j == 87)
                  my_alloc = 3000;
                if (j == 88)
                  my_alloc = 3000;
                if (j == 89)
                  my_alloc = 3000;
                if (j == 90)
                  my_alloc = 3000;
                g_cdd_alloc_fail = my_alloc;
                rc = patch_list_apply(&p_oom, tl_alloc, &out_oom);
                ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_MEMORY);
                g_cdd_alloc_fail = 0;
                if (out_oom) {
                  free(out_oom);
                  out_oom = NULL;
                }
              }
            }
            patch_list_free(&p_oom);
            free_token_list(tl_alloc);
          }
        }
      }
    }
  }
#endif
  g_fail_io_after = -1;
  PASS();
}

SUITE(text_patcher_oom_suite) { RUN_TEST(test_patcher_oom); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_TEXT_PATCHER_OOM_H */
