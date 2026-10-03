/**
 * @file doc_internal.h
 * @brief Internal declarations for documentation comment parsing components.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_DOC_INTERNAL_H
#define C_CDD_DOC_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "docstrings/parse/doc.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Test if character is an end-of-line character.
 */
#define DOC_IS_EOL(c) ((c) == '\n' || (c) == '\r')

/**
 * @brief Skip whitespace characters in buffer.
 *
 * @param[in] p Pointer to buffer.
 * @param[out] out_pos Pointer to receive advanced buffer position.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_skip_ws(const char *p,
                                              const char **out_pos);

/**
 * @brief Extract the next word from the buffer.
 *
 * @param[in] str Input string pointer.
 * @param[in] end Pointer to end of buffer or line.
 * @param[out] next_out Pointer to receive where scan stopped.
 * @param[out] out_val Pointer to receive allocated word string, or NULL if
 * empty.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_extract_word(const char *str,
                                                   const char *end,
                                                   const char **next_out,
                                                   char **out_val);

/**
 * @brief Extract remaining text on the line, trimming whitespace.
 *
 * @param[in] str Input string pointer.
 * @param[in] end Pointer to end of line.
 * @param[out] out_val Pointer to receive allocated remainder string, or NULL if
 * empty.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_extract_rest(const char *str,
                                                   const char *end,
                                                   char **out_val);

/**
 * @brief Trim leading and trailing whitespace in-place.
 *
 * @param[in,out] s String to trim.
 * @param[out] out_val Pointer to receive start of trimmed string.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_trim_segment(char *s, char **out_val);

/**
 * @brief Parse boolean representation from string.
 *
 * @param[in] s String to parse.
 * @param[out] out Pointer to receive parsed integer boolean (0 or 1).
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on bad input.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_parse_bool_text(const char *s, int *out);

/**
 * @brief Parse serialization style enum from text.
 *
 * @param[in] s Style name string.
 * @param[out] out Pointer to receive style enum.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_parse_style_text(const char *s,
                                                       enum DocParamStyle *out);

/**
 * @brief Parse an optional boolean attribute like [required] or
 * [required:true].
 *
 * @param[in] attr Attribute string.
 * @param[in] key Key name to match.
 * @param[out] out_set Set to 1 if matched.
 * @param[out] out_val Set to parsed boolean value.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_parse_optional_bool_attr(const char *attr,
                                                               const char *key,
                                                               int *out_set,
                                                               int *out_val);

/**
 * @brief Parse optional example attribute like [example:...].
 *
 * @param[in] attr Attribute string.
 * @param[out] out_example Pointer to receive allocated example string.
 * @return CDD_C_SUCCESS on success (1 if matched, 0 if not), error code on
 * failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
doc_parse_optional_example_attr(const char *attr, char **out_example);

/**
 * @brief Find key token in string buffer.
 *
 * @param[in] s String buffer.
 * @param[in] key Key to search for.
 * @param[out] key_len Pointer to receive key length + delimiter.
 * @param[out] out_val Pointer to receive matched position.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_find_key_token(char *s, const char *key,
                                                     size_t *key_len,
                                                     char **out_val);

/**
 * @brief Split comma-delimited scopes into string array.
 *
 * @param[in] input Comma-delimited scopes string.
 * @param[out] out_scopes Pointer to receive allocated array of strings.
 * @param[out] out_count Pointer to receive array size.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_split_scopes(const char *input,
                                                   char ***out_scopes,
                                                   size_t *out_count);

/**
 * @brief Split pipe- or comma-delimited enum values into string array.
 *
 * @param[in] input Enum values string.
 * @param[out] out_vals Pointer to receive allocated array of strings.
 * @param[out] out_count Pointer to receive array size.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_split_enum_values(const char *input,
                                                        char ***out_vals,
                                                        size_t *out_count);

/**
 * @brief Add tag to metadata.
 *
 * @param[in,out] out Metadata to receive tag.
 * @param[in] tag Tag string.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_add_tag(struct DocMetadata *out,
                                              const char *tag);

/**
 * @brief Add tag metadata entry to metadata.
 *
 * @param[in,out] out Metadata to receive tag meta.
 * @param[in] meta Tag meta entry.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_add_tag_meta(struct DocMetadata *out,
                                                   struct DocTagMeta *meta);

/**
 * @brief Parse tag directive line.
 *
 * @param[in] line Line start.
 * @param[in] end Line end.
 * @param[in,out] out Metadata.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_parse_tags_line(const char *line,
                                                      const char *end,
                                                      struct DocMetadata *out);

/**
 * @brief Parse tagMeta directive line.
 *
 * @param[in] line Line start.
 * @param[in] end Line end.
 * @param[in,out] out Metadata.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_parse_tag_meta_line(
    const char *line, const char *end, struct DocMetadata *out);

/**
 * @brief Parse deprecated directive line.
 *
 * @param[in] line Line start.
 * @param[in] end Line end.
 * @param[in,out] out Metadata.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_parse_deprecated_line(
    const char *line, const char *end, struct DocMetadata *out);

/**
 * @brief Parse externalDocs directive line.
 *
 * @param[in] line Line start.
 * @param[in] end Line end.
 * @param[in,out] out Metadata.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_parse_external_docs_line(
    const char *line, const char *end, struct DocMetadata *out);

/**
 * @brief Parse contact directive line.
 *
 * @param[in] line Line start.
 * @param[in] end Line end.
 * @param[in,out] out Metadata.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_parse_contact_line(
    const char *line, const char *end, struct DocMetadata *out);

/**
 * @brief Parse license directive line.
 *
 * @param[in] line Line start.
 * @param[in] end Line end.
 * @param[in,out] out Metadata.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_parse_license_line(
    const char *line, const char *end, struct DocMetadata *out);

/**
 * @brief Parse server directive line.
 *
 * @param[in] line Line start.
 * @param[in] end Line end.
 * @param[in,out] out Metadata.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_parse_server_line(
    const char *line, const char *end, struct DocMetadata *out);

/**
 * @brief Parse serverVar directive line.
 *
 * @param[in] line Line start.
 * @param[in] end Line end.
 * @param[in,out] out Metadata.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_parse_server_var_line(
    const char *line, const char *end, struct DocMetadata *out);

/**
 * @brief Parse encoding directive line.
 *
 * @param[in] line Line start.
 * @param[in] end Line end.
 * @param[in,out] out Metadata.
 * @param[in] kind Encoding kind (0=encoding, 1=prefixEncoding, 2=itemEncoding).
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_parse_encoding_line(
    const char *line, const char *end, struct DocMetadata *out, int kind);

/**
 * @brief Parse route directive line.
 *
 * @param[in] line Line start.
 * @param[in] end Line end.
 * @param[in,out] out Metadata.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_parse_route_line(const char *line,
                                                       const char *end,
                                                       struct DocMetadata *out);

/**
 * @brief Parse param directive line.
 *
 * @param[in] line Line start.
 * @param[in] end Line end.
 * @param[in,out] out Metadata.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_parse_param_line(const char *line,
                                                       const char *end,
                                                       struct DocMetadata *out);

/**
 * @brief Parse return directive line.
 *
 * @param[in] line Line start.
 * @param[in] end Line end.
 * @param[in,out] out Metadata.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_parse_return_line(
    const char *line, const char *end, struct DocMetadata *out);

/**
 * @brief Parse responseHeader directive line.
 *
 * @param[in] line Line start.
 * @param[in] end Line end.
 * @param[in,out] out Metadata.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_parse_response_header_line(
    const char *line, const char *end, struct DocMetadata *out);

/**
 * @brief Parse link directive line.
 *
 * @param[in] line Line start.
 * @param[in] end Line end.
 * @param[in,out] out Metadata.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_parse_link_line(const char *line,
                                                      const char *end,
                                                      struct DocMetadata *out);

/**
 * @brief Parse requestBody directive line.
 *
 * @param[in] line Line start.
 * @param[in] end Line end.
 * @param[in,out] out Metadata.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_parse_request_body_line(
    const char *line, const char *end, struct DocMetadata *out);

/**
 * @brief Parse security type enum from text.
 *
 * @param[in] text Type name string.
 * @param[out] out_val Pointer to receive security type enum.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t
doc_parse_security_type_text(const char *text, enum DocSecurityType *out_val);

/**
 * @brief Parse security in location enum from text.
 *
 * @param[in] text Location string.
 * @param[out] out_val Pointer to receive security in enum.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t
doc_parse_security_in_text(const char *text, enum DocSecurityIn *out_val);

/**
 * @brief Parse OAuth flow type enum from text.
 *
 * @param[in] text Flow type string.
 * @param[out] out_val Pointer to receive flow type enum.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_parse_oauth_flow_type_text(
    const char *text, enum DocOAuthFlowType *out_val);

/**
 * @brief Parse OAuth scopes list.
 *
 * @param[in] input Comma-separated scopes string.
 * @param[out] out Pointer to receive allocated array of OAuth scopes.
 * @param[out] out_count Pointer to receive number of scopes.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_parse_oauth_scopes(
    const char *input, struct DocOAuthScope **out, size_t *out_count);

/**
 * @brief Parse security requirement directive line.
 *
 * @param[in] line Line start.
 * @param[in] end Line end.
 * @param[in,out] out Metadata.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_parse_security_line(
    const char *line, const char *end, struct DocMetadata *out);

/**
 * @brief Parse securityScheme directive line.
 *
 * @param[in] line Line start.
 * @param[in] end Line end.
 * @param[in,out] out Metadata.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t doc_parse_security_scheme_line(
    const char *line, const char *end, struct DocMetadata *out);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_DOC_INTERNAL_H */
