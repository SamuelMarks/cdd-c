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
#include "c_cdd/memory.h"
#include "c_cdd/log.h"
/* clang-format on */

cdd_c_error_t weaver_translate_gcc_attributes(struct PatchList *patches,
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
      char *attr_text = (char *)C_CDD_MALLOC(len + 1);
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
        int r =
            c_cdd_strdup("#if "
                         "defined(_MSC_VER)\n__declspec(dllexport)\n#else\n__"
                         "attribute__((visibility(\"default\")))\n#endif\n",
                         &replacement);
        /* LCOV_EXCL_START */
        if (r != CDD_C_SUCCESS) {
          C_CDD_FREE(attr_text);
          return r;
        }
        /* LCOV_EXCL_STOP */
      } else if (strstr(attr_text, "unused")) {
        /* Unused variables are usually handled by (void)var; but as an
         * attribute it can be tricky. */
      } else if (strstr(attr_text, "noreturn")) {
        int r =
            c_cdd_strdup("#if "
                         "defined(_MSC_VER)\n__declspec(noreturn)\n#else\n__"
                         "attribute__((noreturn))\n#endif\n",
                         &replacement);
        /* LCOV_EXCL_START */
        if (r != CDD_C_SUCCESS) {
          C_CDD_FREE(attr_text);
          return r;
        }
        /* LCOV_EXCL_STOP */
      } else if (strstr(attr_text, "format") && strstr(attr_text, "printf")) {
        int r =
            c_cdd_strdup("#if "
                         "defined(_MSC_VER)\n_Printf_format_string_\n#else\n__"
                         "attribute__((format(printf)))\n#endif\n",
                         &replacement);
        /* LCOV_EXCL_START */
        if (r != CDD_C_SUCCESS) {
          C_CDD_FREE(attr_text);
          return r;
        }
        /* LCOV_EXCL_STOP */
      }

      if (replacement) {
        int rc = patch_list_add(patches, node->start_token, node->end_token,
                                replacement);
        /* LCOV_EXCL_START */
        if (rc != 0) {
          C_CDD_FREE(attr_text);
          return rc;
        }
        /* LCOV_EXCL_STOP */
      }
      C_CDD_FREE(attr_text);
    }
  }

  return CDD_C_SUCCESS;
}
