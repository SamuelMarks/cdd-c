#ifndef C_CDD_SCANNER_H
#define C_CDD_SCANNER_H

#include "c_cdd_stdbool.h"

#include <tree_sitter/api.h>

#include "c_cdd_export.h"

// Declare the `tree_sitter_json` function, which is
// implemented by the `tree-sitter-json` library.
extern C_CDD_EXPORT TSLanguage *tree_sitter_json();

extern C_CDD_EXPORT void run(void);

#endif /* !C_CDD_SCANNER_H */
