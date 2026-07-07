/**
 * @file vla_analyzer.h
 * @brief Analysis engine for detecting Variable Length Arrays (VLAs).
 *
 * Scans a C syntax tree for standard VLA declarations and constructs a map
 * to enable AST rewriting using `weaver_vla_to_alloca`.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_VLA_ANALYZER_H
#define C_CDD_VLA_ANALYZER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

/**
 * @brief Represents an identified VLA site in the token stream.
 */
struct VLASite {
  /** @brief field */
  /** @brief field */
  size_t start_token_idx;
  /** @brief field */
  /** @brief field */
  size_t end_token_idx;
  /** @brief field */
  /** @brief field */
  char *type_str;
  /** @brief field */
  /** @brief field */
  char *var_name;
  /** @brief field */
  /** @brief field */
  char *size_expr;
};

/**
 * @brief Collection of identified VLA sites.
 */
struct VLASiteList {
  /** @brief field */
  /** @brief field */
  struct VLASite *sites;
  /** @brief field */
  /** @brief field */
  size_t count;
  /** @brief field */
  /** @brief field */
  size_t capacity;
};

/**
 * @brief Initialize a VLA site list.
 */
extern C_CDD_EXPORT enum cdd_c_error
vla_site_list_init(struct VLASiteList *list);

/**
 * @brief Free resources associated with a VLA site list.
 */
extern C_CDD_EXPORT void vla_site_list_free(struct VLASiteList *list);

/**
 * @brief Scan a token stream to detect VLAs.
 *
 * Scans for patterns like `type var[expr];` where `expr` is not a constant
 * literal. Populates `list` with boundaries and strings.
 *
 * @param[in] tokens The original token stream.
 * @param[out] list The initialized list to populate.
 * @return 0 on success.
 */
extern C_CDD_EXPORT enum cdd_c_error
scan_for_vlas(const struct TokenList *tokens, struct VLASiteList *list);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_VLA_ANALYZER_H */
