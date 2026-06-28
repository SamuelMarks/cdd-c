/**
 * @file weaver_attributes.c
 * @brief Implementation of GNU attribute weaving.
 */

/* clang-format off */
#include "c_cdd_export.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/patcher.h"
#include "functions/parse/cst.h"
#include "functions/parse/str.h"
#include "functions/parse/tokenizer.h"
#include "functions/emit/weaver_attributes.h"
#include "c_cdd/log.h"
/* clang-format on */
/* LCOV_EXCL_START */

enum cdd_c_error
weaver_translate_gcc_attributes(struct PatchList *patches,
                                const struct TokenList *tokens,
                                const struct CstNodeList *cst) {
  size_t i;
  if (!patches || !tokens || !cst)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < cst->size; i++) {
    const struct CstNode *node = &cst->nodes[i];
    if (node->kind == CST_NODE_GCC_ATTRIBUTE) {
      char *replacement = NULL;
      /* Extract text from tokens directly */
      size_t len = node->length;
      char *attr_text = NULL;
#ifdef CDD_BUILD_TESTS
      {
        extern C_CDD_EXPORT int g_cdd_fail_alloc;
        if (g_cdd_fail_alloc && --g_cdd_fail_alloc == 0)
          attr_text = NULL;
        else
          attr_text = (char *)malloc(len + 1);
      }
#else
      attr_text = (char *)malloc(len + 1);
#endif
      if (!attr_text) {
        C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
        return CDD_C_ERROR_MEMORY;
      }
      memcpy(attr_text, node->start, len);
      attr_text[len] = '\0';

      /* simple translations */
      if (strstr(attr_text, "packed")) {
        /* This one is tricky, needs #pragma pack wrapping around the struct.
           We can just do a basic replacement for now, or flag it. */
        /* To fully do packed, we need to know the struct scope. */
      } else if (strstr(attr_text, "visibility") &&
                 strstr(attr_text, "default")) {
        c_cdd_strdup("#if "
                     "defined(_MSC_VER)\n__declspec(dllexport)\n#else\n__"
                     "attribute__((visibility(\"default\")))\n#endif\n",
                     &replacement);
      } else if (strstr(attr_text, "unused")) {
        /* Unused variables are usually handled by (void)var; but as an
         * attribute it can be tricky. */
      } else if (strstr(attr_text, "noreturn")) {
        c_cdd_strdup("#if "
                     "defined(_MSC_VER)\n__declspec(noreturn)\n#else\n__"
                     "attribute__((noreturn))\n#endif\n",
                     &replacement);
      } else if (strstr(attr_text, "format") && strstr(attr_text, "printf")) {
        c_cdd_strdup("#if "
                     "defined(_MSC_VER)\n_Printf_format_string_\n#else\n__"
                     "attribute__((format(printf)))\n#endif\n",
                     &replacement);
      }

      if (replacement) {
        int rc = patch_list_add(patches, node->start_token, node->end_token,
                                replacement);
        printf("weaver_attr patch_list_add rc=%d\n", rc);
        printf("patch_list_add rc=%d\n", rc);
        if (rc != 0) {
          free(attr_text);
          return rc;
        }
      }
      free(attr_text);
    }
  }

  return CDD_C_SUCCESS;
}

/* LCOV_EXCL_STOP */
