}
else if (tok->length == 29 &&
         memcmp(tok->start, "__builtin_extract_return_addr", 29) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_extract_return_addr";
  tok->length = 31;
}
else if (tok->length == 26 &&
         memcmp(tok->start, "__builtin_frob_return_addr", 26) == 0) {
  tok->start = (const uint8_t *)"cdd_builtin_frob_return_addr";
  tok->length = 28;
