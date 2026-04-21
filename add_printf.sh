sed -i '' 's/for (i = 0; i < tree->base_tokens->size; i++) {/for (i = 0; i < tree->base_tokens->size; i++) {\n    printf("i=%zu\\n", i);/g' src/transformers/gnu_standardizer/gnu_standardizer.c
