#!/bin/bash
sed -i.bak -e '/else if (attr->length == 11 && memcmp(attr->start, "vector_size", 11) == 0) {/i\
        } else if (attr->length == 10 && memcmp(attr->start, "deprecated", 10) == 0) {\
          cdd_token_t *target_val = &tree->base_tokens->tokens[i + 5];\
          char *heap_buf;\
          size_t val_len = target_val->length;\
          heap_buf = (char *)malloc(val_len + 30);\
          if (heap_buf) {\
            strcpy(heap_buf, "__declspec(deprecated(");\
            strncat(heap_buf, (const char *)target_val->start, val_len);\
            strcat(heap_buf, "))");\
            tok->start = (const uint8_t *)heap_buf;\
            tok->length = strlen(heap_buf);\
          }\
          tree->base_tokens->tokens[i + 1].length = 0;\
          tree->base_tokens->tokens[i + 2].length = 0;\
          tree->base_tokens->tokens[i + 3].length = 0;\
          tree->base_tokens->tokens[i + 4].length = 0;\
          tree->base_tokens->tokens[i + 5].length = 0;\
          tree->base_tokens->tokens[i + 6].length = 0;\
          tree->base_tokens->tokens[i + 7].length = 0;\
          tree->base_tokens->tokens[i + 8].length = 0;' src/transformers/gnu_standardizer/gnu_standardizer.c
