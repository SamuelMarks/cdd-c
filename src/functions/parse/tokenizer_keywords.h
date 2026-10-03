/**
 * @file tokenizer_keywords.h
 * @brief Keyword recognition helpers for lexical analyzer.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_TOKENIZER_KEYWORDS_H
#define C_CDD_TOKENIZER_KEYWORDS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include <stddef.h>
#if defined(_MSC_VER) && _MSC_VER < 1600
#include "msvc/stdint.h"
#else
#if defined(_MSC_VER) && _MSC_VER < 1600
#include "c_cdd/msvc/stdint.h"
#else
#include <stdint.h>
#endif
#endif

#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !C_CDD_TOKENIZER_KEYWORDS_H */
