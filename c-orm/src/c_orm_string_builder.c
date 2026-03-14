/**
 * @file c_orm_string_builder.c
 * @brief Implementation of dynamic string builder.
 */

/* clang-format off */
#include "c_orm_string_builder.h"

#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct c_orm_string_builder {
  char *buffer;
  size_t length;
  size_t capacity;
};

int c_orm_string_builder_init(c_orm_string_builder_t **out_builder) {
  c_orm_string_builder_t *sb;

  if (!out_builder)
    return 1;

  sb = (c_orm_string_builder_t *)malloc(sizeof(c_orm_string_builder_t));
  if (!sb)
    return 1;

  sb->capacity = 64;
  sb->length = 0;
  sb->buffer = (char *)malloc(sb->capacity);
  if (!sb->buffer) {
    free(sb);
    return 1;
  }
  sb->buffer[0] = '\0';

  *out_builder = sb;
  return 0;
}

void c_orm_string_builder_free(c_orm_string_builder_t *builder) {
  if (builder) {
    if (builder->buffer) {
      free(builder->buffer);
    }
    free(builder);
  }
}

int c_orm_string_builder_append(c_orm_string_builder_t *builder,
                                const char *str) {
  size_t len;
  size_t required_capacity;

  if (!builder || !str)
    return 1;

  len = strlen(str);
  if (len == 0)
    return 0;

  required_capacity = builder->length + len + 1;
  if (required_capacity > builder->capacity) {
    size_t new_capacity = builder->capacity * 2;
    char *new_buffer;
    while (new_capacity < required_capacity) {
      new_capacity *= 2;
    }
    new_buffer = (char *)realloc(builder->buffer, new_capacity);
    if (!new_buffer)
      return 1;
    builder->buffer = new_buffer;
    builder->capacity = new_capacity;
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  strcpy_s(builder->buffer + builder->length,
           builder->capacity - builder->length, str);
#else
  strcpy(builder->buffer + builder->length, str);
#endif

  builder->length += len;
  return 0;
}

const char *c_orm_string_builder_get(const c_orm_string_builder_t *builder) {
  if (!builder || !builder->buffer)
    return "";
  return builder->buffer;
}

size_t c_orm_string_builder_len(const c_orm_string_builder_t *builder) {
  if (!builder)
    return 0;
  return builder->length;
}
