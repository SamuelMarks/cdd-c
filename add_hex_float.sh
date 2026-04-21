sed -i '' '/if (tok->length >= 3 && tok->start\[0\] == '"'"'0'"'"' &&/i\
      if (tok->length > 3 && tok->start[0] == '"'"'0'"'"' && (tok->start[1] == '"'"'x'"'"' || tok->start[1] == '"'"'X'"'"')) {\
        size_t k;\
        int has_p = 0;\
        for (k = 2; k < tok->length; k++) {\
          if (tok->start[k] == '"'"'p'"'"' || tok->start[k] == '"'"'P'"'"') { has_p = 1; break; }\
        }\
        if (has_p) {\
          tok->start = (const uint8_t *)"0.0 /* hex float lowered */";\
          tok->length = 27;\
        }\
      }\
' src/transformers/gnu_standardizer/gnu_standardizer.c
