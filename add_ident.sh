sed -i '' '/if (tok->length == 13 && memcmp(tok->start, "__extension__", 13) == 0) {/i\
      if (tok->length == 8 && memcmp(tok->start, "__DATE__", 8) == 0) {\
        tok->start = (const uint8_t *)"\"Jan 01 1970\"";\
        tok->length = 13;\
      } else if (tok->length == 8 && memcmp(tok->start, "__TIME__", 8) == 0) {\
        tok->start = (const uint8_t *)"\"00:00:00\"";\
        tok->length = 10;\
      } else if (tok->length == 13 && memcmp(tok->start, "__TIMESTAMP__", 13) == 0) {\
        tok->start = (const uint8_t *)"\"Thu Jan 01 00:00:00 1970\"";\
        tok->length = 26;\
      } else if (tok->length == 11 && memcmp(tok->start, "__COUNTER__", 11) == 0) {\
        tok->start = (const uint8_t *)"0";\
        tok->length = 1;\
      } else if (tok->length == 17 && memcmp(tok->start, "__INCLUDE_LEVEL__", 17) == 0) {\
        tok->start = (const uint8_t *)"0";\
        tok->length = 1;\
      } else if (tok->length == 13 && memcmp(tok->start, "__BASE_FILE__", 13) == 0) {\
        tok->start = (const uint8_t *)"\"file.c\"";\
        tok->length = 8;\
      } else \
' src/transformers/gnu_standardizer/gnu_standardizer.c
