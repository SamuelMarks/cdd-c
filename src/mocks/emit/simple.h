#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
#ifndef C_CDD_TESTS_MOCKS_SIMPLE_H
#define C_CDD_TESTS_MOCKS_SIMPLE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "simple_mocks_export.h"
/* clang-format on */

/** @brief haz */
struct Haz *haz;

extern SIMPLE_MOCKS_EXPORT /**
                            * @brief Executes the Foo cleanup operation.
                            */
    void
    Foo_cleanup(struct Foo *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !C_CDD_TESTS_MOCKS_SIMPLE_H */

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
