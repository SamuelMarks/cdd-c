/**
 * @file simple_json.h
 * @brief Mock JSON models and serialization declarations in simple_json.h.
 */
#ifndef SIMPLE_JSON_H
#define SIMPLE_JSON_H

/* clang-format off */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#else
#include "../c_cdd_stdbool.h"
#endif

#include <stdio.h>
#include "cdd_c_error.h"
#include "c_cdd/json_dom.h"
#include "cdd_src_mocks_export.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @brief Enumeration of mock tank sizes. */
enum Tank {
  /** @brief Large tank variant. */
  Tank_BIG,
  /** @brief Small tank variant. */
  Tank_SMALL,
  /** @brief Unknown tank fallback variant. */
  Tank_UNKNOWN = -1
};

/**
 * @brief Populates the default `Tank` enum variant.
 * @param[out] out Optional pointer receiving `Tank_BIG`.
 * @return `CDD_C_SUCCESS`.
 */
extern CDD_SRC_MOCKS_EXPORT cdd_c_error_t Tank_default(enum Tank *out);

/**
 * @brief Formats a `Tank` enum value as a newly allocated string.
 * @param[in] tank Enum value to format.
 * @param[out] str Output pointer receiving heap-allocated string.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
extern CDD_SRC_MOCKS_EXPORT cdd_c_error_t Tank_to_str(enum Tank tank,
                                                      char **str);

/**
 * @brief Parses a `Tank` enum value from its string representation.
 * @param[in] str Input string (or NULL).
 * @param[out] val Output pointer receiving parsed `Tank` variant.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT`.
 */
extern CDD_SRC_MOCKS_EXPORT cdd_c_error_t Tank_from_str(const char *str,
                                                        enum Tank *val);

/** @brief Mock structure HazE with string and enum fields. */
struct HazE {
  /** @brief Optional heap-allocated string member. */
  char *bzr;
  /** @brief Tank size enumeration member. */
  enum Tank tank;
};

/**
 * @brief Releases a `HazE` struct and its owned members.
 * @param[in,out] haz_e Pointer to `HazE` instance (or NULL).
 * @return `CDD_C_SUCCESS`.
 */
extern CDD_SRC_MOCKS_EXPORT cdd_c_error_t HazE_cleanup(struct HazE *haz_e);

/**
 * @brief Allocates and initializes a default `HazE` instance.
 * @param[out] haz_e Output pointer receiving allocated `HazE`.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
extern CDD_SRC_MOCKS_EXPORT cdd_c_error_t HazE_default(struct HazE **haz_e);

/**
 * @brief Deep-copies a `HazE` instance.
 * @param[in] haz_e_original Source `HazE` instance (or NULL).
 * @param[out] haz_e_dest Output pointer receiving cloned `HazE`.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
extern CDD_SRC_MOCKS_EXPORT cdd_c_error_t
HazE_deepcopy(const struct HazE *haz_e_original, struct HazE **haz_e_dest);

/**
 * @brief Writes a JSON line representation of `haz_e` to `fh`.
 * @param[in] haz_e Source `HazE` instance (or NULL).
 * @param[in,out] fh Output stream.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
extern CDD_SRC_MOCKS_EXPORT cdd_c_error_t HazE_display(const struct HazE *haz_e,
                                                       FILE *fh);

/**
 * @brief Writes a C-struct debug representation of `haz_e` to `fh`.
 * @param[in] haz_e Source `HazE` instance (or NULL).
 * @param[in,out] fh Output stream.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
extern CDD_SRC_MOCKS_EXPORT cdd_c_error_t HazE_debug(const struct HazE *haz_e,
                                                     FILE *fh);

/**
 * @brief Compares two `HazE` instances for structural equality.
 * @param[in] haz_e0 First `HazE` pointer.
 * @param[in] haz_e1 Second `HazE` pointer.
 * @return `CDD_C_SUCCESS` when equal, non-zero otherwise.
 */
extern CDD_SRC_MOCKS_EXPORT cdd_c_error_t HazE_eq(const struct HazE *haz_e0,
                                                  const struct HazE *haz_e1);

/**
 * @brief Serializes a `HazE` instance to a newly allocated JSON string.
 * @param[in] haz_e Source `HazE` instance (or NULL).
 * @param[out] json Output pointer receiving heap-allocated JSON text.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
extern CDD_SRC_MOCKS_EXPORT cdd_c_error_t HazE_to_json(const struct HazE *haz_e,
                                                       char **json);

/**
 * @brief Deserializes a `HazE` instance from a JSON string.
 * @param[in] json Input JSON string.
 * @param[out] haz_e Output pointer receiving allocated `HazE`.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
extern CDD_SRC_MOCKS_EXPORT cdd_c_error_t HazE_from_json(const char *json,
                                                         struct HazE **haz_e);

/**
 * @brief Deserializes a `HazE` instance from a parsed `JSON_Object`.
 * @param[in] jsonObject Parsed JSON object node.
 * @param[out] haz_e Output pointer receiving allocated `HazE`.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
extern CDD_SRC_MOCKS_EXPORT cdd_c_error_t
HazE_from_jsonObject(const JSON_Object *jsonObject, struct HazE **haz_e);

/** @brief Mock outer structure FooE containing scalar, string, and nested HazE.
 */
struct FooE {
  /** @brief Optional heap-allocated string member. */
  char *bar;
  /** @brief Integer flag/counter member. */
  int can;
  /** @brief Nested `HazE` pointer member. */
  struct HazE *haz;
};

/**
 * @brief Releases a `FooE` struct and its owned members.
 * @param[in,out] foo_e Pointer to `FooE` instance (or NULL).
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
extern CDD_SRC_MOCKS_EXPORT cdd_c_error_t FooE_cleanup(struct FooE *foo_e);

/**
 * @brief Allocates and initializes a default `FooE` instance.
 * @param[out] foo_e Output pointer receiving allocated `FooE`.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
extern CDD_SRC_MOCKS_EXPORT cdd_c_error_t FooE_default(struct FooE **foo_e);

/**
 * @brief Deep-copies a `FooE` instance.
 * @param[in] foo_e_original Source `FooE` instance (or NULL).
 * @param[out] foo_e_dest Output pointer receiving cloned `FooE`.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
extern CDD_SRC_MOCKS_EXPORT cdd_c_error_t
FooE_deepcopy(const struct FooE *foo_e_original, struct FooE **foo_e_dest);

/**
 * @brief Writes a JSON line representation of `foo_e` to `fh`.
 * @param[in] foo_e Source `FooE` instance (or NULL).
 * @param[in,out] fh Output stream.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
extern CDD_SRC_MOCKS_EXPORT cdd_c_error_t FooE_display(const struct FooE *foo_e,
                                                       FILE *fh);

/**
 * @brief Writes a C-struct debug representation of `foo_e` to `fh`.
 * @param[in] foo_e Source `FooE` instance (or NULL).
 * @param[in,out] fh Output stream.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
extern CDD_SRC_MOCKS_EXPORT cdd_c_error_t FooE_debug(const struct FooE *foo_e,
                                                     FILE *fh);

/**
 * @brief Compares two `FooE` instances for structural equality.
 * @param[in] foo_e0 First `FooE` pointer.
 * @param[in] foo_e1 Second `FooE` pointer.
 * @return `CDD_C_SUCCESS` when equal, non-zero otherwise.
 */
extern CDD_SRC_MOCKS_EXPORT cdd_c_error_t FooE_eq(const struct FooE *foo_e0,
                                                  const struct FooE *foo_e1);

/**
 * @brief Serializes a `FooE` instance to a newly allocated JSON string.
 * @param[in] foo_e Source `FooE` instance (or NULL).
 * @param[out] json Output pointer receiving heap-allocated JSON text.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
extern CDD_SRC_MOCKS_EXPORT cdd_c_error_t FooE_to_json(const struct FooE *foo_e,
                                                       char **json);

/**
 * @brief Deserializes a `FooE` instance from a parsed `JSON_Object`.
 * @param[in] jsonObject Parsed JSON object node.
 * @param[out] foo_e Output pointer receiving allocated `FooE`.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
extern CDD_SRC_MOCKS_EXPORT cdd_c_error_t
FooE_from_jsonObject(const JSON_Object *jsonObject, struct FooE **foo_e);

/**
 * @brief Deserializes a `FooE` instance from a JSON string.
 * @param[in] json Input JSON string.
 * @param[out] foo_e Output pointer receiving allocated `FooE`.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
extern CDD_SRC_MOCKS_EXPORT cdd_c_error_t FooE_from_json(const char *json,
                                                         struct FooE **foo_e);

/**
 * @brief Executes the self-contained JSON round-trip mock test.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
extern CDD_SRC_MOCKS_EXPORT cdd_c_error_t run_mocks_test(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SIMPLE_JSON_H */
