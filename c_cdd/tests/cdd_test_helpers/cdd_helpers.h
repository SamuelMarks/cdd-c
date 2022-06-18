#ifndef C_CDD_TESTS_CDD_HELPERS_H
#define C_CDD_TESTS_CDD_HELPERS_H

#include "cdd_test_helpers_export.h"
#include "scanner_types.h"

extern CDD_TEST_HELPERS_EXPORT void cdd_precondition_failed(void);

extern CDD_TEST_HELPERS_EXPORT void
debug_scanned_with_mock(const struct scan_az_span_list *scanned, size_t *i,
                        const struct StrScannerKind *scanned_l, size_t);

#endif /* !C_CDD_TESTS_CDD_HELPERS_H */
