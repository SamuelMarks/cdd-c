#!/bin/bash
# Adding noreturn
sed -i.bak -e '/else if (attr->length == 8 && memcmp(attr->start, "noinline", 8) == 0) {/i\
        } else if (attr->length == 8 && memcmp(attr->start, "noreturn", 8) == 0) {\
          tok->start = (const uint8_t *)"__declspec(noreturn)";\
          tok->length = 20;\
          tree->base_tokens->tokens[i + 1].length = 0;\
          tree->base_tokens->tokens[i + 2].length = 0;\
          tree->base_tokens->tokens[i + 3].length = 0;\
          tree->base_tokens->tokens[i + 4].length = 0;\
          tree->base_tokens->tokens[i + 5].length = 0;' src/transformers/gnu_standardizer/gnu_standardizer.c
