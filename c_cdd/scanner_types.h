#ifndef C_CDD_SCANNER_TYPES_H
#define C_CDD_SCANNER_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "c_cdd_export.h"

enum ScannerKind {
  UnknownScan,

  Macro,

  /*Integer,
  Decimal,*/
  DoubleQuoted,
  SingleQuoted,

  CComment,
  CppComment
};

extern C_CDD_EXPORT const char *ScannerKind_to_str(enum ScannerKind);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !C_CDD_SCANNER_TYPES_H */
