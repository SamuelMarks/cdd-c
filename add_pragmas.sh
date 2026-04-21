sed -i '' '/if (tok->kind == CDD_TOKEN_PREPROC_DEFINE) {/i\
    if (tok->kind == CDD_TOKEN_PREPROC_PRAGMA) {\
      const char *p = (const char *)tok->start;\
      size_t len = tok->length;\
      if ((len >= 11 && memcmp(p, "#pragma GCC", 11) == 0) ||\
          (len >= 18 && memcmp(p, "#pragma GCC poison", 18) == 0) ||\
          (len >= 22 && memcmp(p, "#pragma GCC diagnostic", 22) == 0) ||\
          (len >= 25 && memcmp(p, "#pragma GCC system_header", 25) == 0)) {\
        /* Strip the pragma */\
        tok->kind = CDD_TOKEN_OTHER;\
        tok->start = (const uint8_t *)"/* stripped GCC pragma */";\
        tok->length = strlen("/* stripped GCC pragma */");\
      }\
    } else if (tok->kind == CDD_TOKEN_OTHER && tok->length > 0 && tok->start[0] == '\#') {\
      const char *p = (const char *)tok->start;\
      size_t len = tok->length;\
      if (len >= 8 && memcmp(p, "#warning", 8) == 0) {\
        char *buf = (char *)malloc(len + 32);\
        if (buf) {\
          strcpy(buf, "#pragma message(");\
          memcpy(buf + 16, p + 8, len - 8);\
          buf[16 + len - 8] = ')';\
          buf[16 + len - 8 + 1] = '\\0';\
          tree->synthesized_tokens[tree->num_synthesized] = tok;\
          tree->num_synthesized++;\
          tok->start = (const uint8_t *)buf;\
          tok->length = strlen(buf);\
        }\
      } else if (len >= 6 && memcmp(p, "#ident", 6) == 0) {\
        tok->start = (const uint8_t *)"/* stripped #ident */";\
        tok->length = strlen("/* stripped #ident */");\
      } else if (len >= 5 && memcmp(p, "#sccs", 5) == 0) {\
        tok->start = (const uint8_t *)"/* stripped #sccs */";\
        tok->length = strlen("/* stripped #sccs */");\
      }\
    } else if (tok->kind == CDD_TOKEN_PREPROC_INCLUDE) {\
      const char *p = (const char *)tok->start;\
      size_t len = tok->length;\
      if (len >= 13 && memcmp(p, "#include_next", 13) == 0) {\
        char *buf = (char *)malloc(len);\
        if (buf) {\
          strcpy(buf, "#include ");\
          memcpy(buf + 9, p + 13, len - 13);\
          buf[9 + len - 13] = '\\0';\
          tree->synthesized_tokens[tree->num_synthesized] = tok;\
          tree->num_synthesized++;\
          tok->start = (const uint8_t *)buf;\
          tok->length = strlen(buf);\
        }\
      }\
    } else \
' src/transformers/gnu_standardizer/gnu_standardizer.c
