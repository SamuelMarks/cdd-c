#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
/**
 * @file test_mocks.h
 * @brief Mock data structures and factories for tests.
 */
#ifndef TEST_MOCKS_H
#define TEST_MOCKS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "openapi/parse/openapi.h"
#include "cdd_c_error.h"
/* clang-format on */

/*
 * Ensure coverage across OpenAPI models and types to satisfy test dependencies
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_MOCKS_H */

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
