#ifndef C_CDD_TOKENIZER_HELPERS_H
#define C_CDD_TOKENIZER_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <c_str_span.h>
#include <stddef.h>

#include "ll.h"
#include <c_cdd_export.h>

extern C_CDD_EXPORT size_t eatCComment(const az_span *, size_t, size_t,
                                       struct scan_az_span_elem ***,
                                       struct scan_az_span_list *);

extern C_CDD_EXPORT size_t eatCppComment(const az_span *, size_t, size_t,
                                         struct scan_az_span_elem ***,
                                         struct scan_az_span_list *);

extern C_CDD_EXPORT size_t eatMacro(const az_span *, size_t, size_t,
                                    struct scan_az_span_elem ***,
                                    struct scan_az_span_list *);

extern C_CDD_EXPORT size_t eatCharLiteral(const az_span *, size_t, size_t,
                                          struct scan_az_span_elem ***,
                                          struct scan_az_span_list *);
extern C_CDD_EXPORT size_t eatStrLiteral(const az_span *, size_t, size_t,
                                         struct scan_az_span_elem ***,
                                         struct scan_az_span_list *);

extern C_CDD_EXPORT size_t eatWhitespace(const az_span *, size_t, size_t,
                                         struct scan_az_span_elem ***,
                                         struct scan_az_span_list *);

extern C_CDD_EXPORT void eatOneChar(const az_span *, size_t,
                                    struct scan_az_span_elem ***,
                                    struct scan_az_span_list *,
                                    enum ScannerKind);

extern C_CDD_EXPORT size_t eatTwoChars(const az_span *, size_t,
                                       struct scan_az_span_elem ***,
                                       struct scan_az_span_list *,
                                       enum ScannerKind);

extern C_CDD_EXPORT size_t eatThreeChars(const az_span *, size_t,
                                         struct scan_az_span_elem ***,
                                         struct scan_az_span_list *,
                                         enum ScannerKind);

extern C_CDD_EXPORT size_t eatWord(const az_span *, size_t, size_t,
                                   struct scan_az_span_elem ***,
                                   struct scan_az_span_list *);

extern C_CDD_EXPORT size_t eatNumber(const az_span *, size_t, size_t,
                                     struct scan_az_span_elem ***,
                                     struct scan_az_span_list *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !C_CDD_TOKENIZER_HELPERS_H */
