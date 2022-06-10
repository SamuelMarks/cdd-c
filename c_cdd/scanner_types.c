#include "scanner_types.h"

const char *ScannerKind_to_str(const enum ScannerKind kind) {
  switch (kind) {
  case Macro:
    return "Macro";

  case DoubleQuoted:
    return "DoubleQuoted";

  case SingleQuoted:
    return "SingleQuoted";

  case CComment:
    return "CComment";

  case CppComment:
    return "CppComment";

  case UnknownScan:
  default:
    return "UnknownScan";
  }
}
