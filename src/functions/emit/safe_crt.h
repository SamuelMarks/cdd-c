/**
 * @file safe_crt.h
 * @brief CST Transformer for Safe CRT functions (e.g. strcpy_s).
 */

#ifndef C_CDD_FUNCTIONS_EMIT_SAFE_CRT_H
#define C_CDD_FUNCTIONS_EMIT_SAFE_CRT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "functions/parse/cst.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

/**
 * @brief Represents a single Safe CRT injection patch.
 */
struct SafeCrtPatch {
  /** @brief Start token index */
  size_t start_token_idx;
  /** @brief End token index */
  size_t end_token_idx;
  /** @brief Replacement text */
  char *replacement_text;
};

/**
 * @brief List of patches.
 */
struct SafeCrtPatchList {
  /** @brief Dynamic array of patches */
  struct SafeCrtPatch *patches;
  /** @brief Number of active patches */
  size_t size;
  /** @brief Allocated capacity */
  size_t capacity;
};

C_CDD_EXPORT /**
              * @brief Executes the safe crt patch list init operation.
              * @param[out] list The patch list to initialize.
              * @return CDD_C_SUCCESS on success, error code otherwise.
              */
    cdd_c_error_t
    safe_crt_patch_list_init(struct SafeCrtPatchList *list);

C_CDD_EXPORT /**
              * @brief Executes the safe crt patch list free operation.
              * @param[in,out] list The patch list to free.
              * @return CDD_C_SUCCESS on success, error code otherwise.
              */
    cdd_c_error_t
    safe_crt_patch_list_free(struct SafeCrtPatchList *list);

C_CDD_EXPORT /**
              * @brief Adds a patch to the patch list.
              * @param[in,out] list The patch list to append to.
              * @param[in] start Start token index.
              * @param[in] end End token index.
              * @param[in] text Replacement text.
              * @return CDD_C_SUCCESS on success, error code otherwise.
              */
    cdd_c_error_t
    safe_crt_add_patch(struct SafeCrtPatchList *list, size_t start, size_t end,
                       const char *text);

C_CDD_EXPORT /**
              * @brief Extracts concatenated text from a token slice.
              * @param[in] tokens Token list.
              * @param[in] start Start token index.
              * @param[in] end End token index.
              * @param[out] _out_val Pointer to allocated output string.
              * @return CDD_C_SUCCESS on success, error code otherwise.
              */
    cdd_c_error_t
    safe_crt_extract_token_text(const struct TokenList *tokens, size_t start,
                                size_t end, char **_out_val);

C_CDD_EXPORT /**
              * @brief Generates a strcpy safe CRT patch.
              * @param[in] tokens Token list.
              * @param[in] call_start Start token index.
              * @param[in] call_end End token index.
              * @param[in,out] out Output patch list.
              * @return CDD_C_SUCCESS on success, error code otherwise.
              */
    cdd_c_error_t
    safe_crt_generate_strcpy_patch(const struct TokenList *tokens,
                                   size_t call_start, size_t call_end,
                                   struct SafeCrtPatchList *out);

C_CDD_EXPORT /**
              * @brief Generates a fopen safe CRT patch.
              * @param[in] tokens Token list.
              * @param[in] call_start Start token index.
              * @param[in] call_end End token index.
              * @param[in,out] out Output patch list.
              * @return CDD_C_SUCCESS on success, error code otherwise.
              */
    cdd_c_error_t
    safe_crt_generate_fopen_patch(const struct TokenList *tokens,
                                  size_t call_start, size_t call_end,
                                  struct SafeCrtPatchList *out);

C_CDD_EXPORT /**
              * @brief Generates a strncpy safe CRT patch.
              * @param[in] tokens Token list.
              * @param[in] call_start Start token index.
              * @param[in] call_end End token index.
              * @param[in,out] out Output patch list.
              * @return CDD_C_SUCCESS on success, error code otherwise.
              */
    cdd_c_error_t
    safe_crt_generate_strncpy_patch(const struct TokenList *tokens,
                                    size_t call_start, size_t call_end,
                                    struct SafeCrtPatchList *out);

C_CDD_EXPORT /**
              * @brief Generates a sprintf safe CRT patch.
              * @param[in] tokens Token list.
              * @param[in] call_start Start token index.
              * @param[in] call_end End token index.
              * @param[in,out] out Output patch list.
              * @return CDD_C_SUCCESS on success, error code otherwise.
              */
    cdd_c_error_t
    safe_crt_generate_sprintf_patch(const struct TokenList *tokens,
                                    size_t call_start, size_t call_end,
                                    struct SafeCrtPatchList *out);

C_CDD_EXPORT /**
              * @brief Scan CST and generate patches to upgrade standard C calls
              * to MSVC Safe CRT equivalents.
              * @param[in] cst The parsed CST.
              * @param[in] tokens The token stream.
              * @param[out] out_patches The list to populate.
              * @return CDD_C_SUCCESS on success, error code otherwise.
              */
    cdd_c_error_t
    cst_generate_safe_crt_patches(const struct CstNodeList *cst,
                                  const struct TokenList *tokens,
                                  struct SafeCrtPatchList *out_patches);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_FUNCTIONS_EMIT_SAFE_CRT_H */
