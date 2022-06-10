#ifndef C_CDD_SCANNER_TYPES_H
#define C_CDD_SCANNER_TYPES_H

enum ScannerKind {
  UnknownScan,

  Macro,

  Integer,
  Decimal,
  DoubleQuoted,
  SingleQuoted,

  CComment,
  CppComment
};

#endif /* !C_CDD_SCANNER_TYPES_H */
