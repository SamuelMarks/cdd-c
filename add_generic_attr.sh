#!/bin/bash
sed -i.bak -e '/          tree->base_tokens->tokens\[i + 8\].length = 0;/!b;n;c\
        }\
      } else if (i + 3 < tree->base_tokens->size &&\
                 tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LPAREN &&\
                 tree->base_tokens->tokens[i + 2].kind == CDD_TOKEN_LPAREN) {\
        /* Generic multi-arg attribute parser: find the matching )) */\
        int lparen_count = 2;\
        size_t k;\
        for (k = i + 3; k < tree->base_tokens->size; k++) {\
          if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_LPAREN)\
            lparen_count++;\
          else if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_RPAREN)\
            lparen_count--;\
          if (lparen_count == 0) {\
            /* Strip it */\
            size_t wipe;\
            tok->start = (const uint8_t *)"/* attribute */";\
            tok->length = 15;\
            for (wipe = i + 1; wipe <= k; wipe++) {\
              tree->base_tokens->tokens[wipe].length = 0;\
            }\
            break;\
          }\
        }' src/transformers/gnu_standardizer/gnu_standardizer.c
