/**
 * @file abstract_struct.h
 * @brief CDD_C generic representation (dynamic dictionary/array)
 *
 * Provides cdd_c_abstract_struct_t and cdd_c_variant_t for generic ORM
 * compatibility.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CLASSES_PARSE_ABSTRACT_STRUCT_H
#define C_CDD_CLASSES_PARSE_ABSTRACT_STRUCT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stddef.h>
#include "c_cdd_export.h"
#include "mocks/c_cdd_stdbool.h"
/* clang-format on */

/**
 * @brief Types of variant values.
 */
typedef enum CddCVariantType {
  CDD_C_VARIANT_TYPE_NULL = 0,
  CDD_C_VARIANT_TYPE_INT,
  CDD_C_VARIANT_TYPE_FLOAT,
  CDD_C_VARIANT_TYPE_STRING,
  CDD_C_VARIANT_TYPE_BLOB
} CddCVariantType;

/**
 * @brief Generic variant type.
 */
typedef struct CddCVariant {
  CddCVariantType type;
  union {
    long long i_val;
    double f_val;
    char *s_val;
    struct {
      unsigned char *data;
      size_t size;
    } b_val;
  } value;
} cdd_c_variant_t;

/**
 * @brief Key-value pair for abstract struct.
 */
typedef struct CddCAbstractStructKV {
  char *key;
  unsigned long key_hash;
  cdd_c_variant_t value;
} cdd_c_abstract_struct_kv_t;

/**
 * @brief Abstract struct containing generic key-value pairs.
 */
typedef struct CddCAbstractStruct {
  cdd_c_abstract_struct_kv_t *kvs;
  size_t count;
  size_t capacity;
} cdd_c_abstract_struct_t;

/**
 * @brief Dynamic array of abstract structs for representing multiple rows.
 */
typedef struct CddCAbstractStructArray {
  cdd_c_abstract_struct_t *items;
  size_t count;
  size_t capacity;
} cdd_c_abstract_struct_array_t;

extern C_CDD_EXPORT /**
                     * @brief Initialize an abstract struct array.
                     * @param arr The array to initialize.
                     * @param capacity Initial capacity.
                     * @return 0 on success.
                     */
    int
    cdd_c_abstract_struct_array_init(cdd_c_abstract_struct_array_t *arr,
                                     size_t capacity);

extern C_CDD_EXPORT /**
                     * @brief Add a populated abstract struct to the array
                     * (takes ownership).
                     * @param arr The array.
                     * @param astruct The struct to append.
                     * @return 0 on success.
                     */
    int
    cdd_c_abstract_struct_array_append(cdd_c_abstract_struct_array_t *arr,
                                       cdd_c_abstract_struct_t *astruct);

extern C_CDD_EXPORT /**
                     * @brief Free an abstract struct array and all nested
                     * items.
                     * @param arr The array to free.
                     * @return 0 on success.
                     */
    int
    cdd_c_abstract_struct_array_free(cdd_c_abstract_struct_array_t *arr);

extern C_CDD_EXPORT int
cdd_c_abstract_struct_array_to_json(const cdd_c_abstract_struct_array_t *arr,
                                    char **out_json);

extern C_CDD_EXPORT /**
                     * @brief Initialize an abstract struct.
                     * @param astruct The abstract struct to initialize.
                     * @return 0 on success, non-zero on error.
                     */
    int
    cdd_c_abstract_struct_init(cdd_c_abstract_struct_t *astruct);

extern C_CDD_EXPORT /**
                     * @brief Initialize an abstract struct with a specific
                     * initial capacity.
                     * @param astruct The abstract struct.
                     * @param capacity The number of keys to reserve.
                     * @return 0 on success, non-zero on error.
                     */
    int
    cdd_c_abstract_struct_init_with_capacity(cdd_c_abstract_struct_t *astruct,
                                             size_t capacity);

extern C_CDD_EXPORT /**
                     * @brief Set a value in the abstract struct.
                     * @param astruct The abstract struct.
                     * @param key The key to set.
                     * @param value The value to set (will be deeply copied).
                     * @return 0 on success, non-zero on error.
                     */
    int
    cdd_c_abstract_set(cdd_c_abstract_struct_t *astruct, const char *key,
                       const cdd_c_variant_t *value);

extern C_CDD_EXPORT /**
                     * @brief Get a value from the abstract struct.
                     * @param astruct The abstract struct.
                     * @param key The key to look up.
                     * @param out_value Pointer to receive the found value.
                     * @return 0 on success, non-zero on error.
                     */
    int
    cdd_c_abstract_get(const cdd_c_abstract_struct_t *astruct, const char *key,
                       cdd_c_variant_t **out_value);

extern C_CDD_EXPORT /**
                     * @brief Deep copy an abstract struct.
                     * @param dest The destination abstract struct.
                     * @param src The source abstract struct.
                     * @return 0 on success, non-zero on error.
                     */
    int
    cdd_c_abstract_struct_deep_copy(cdd_c_abstract_struct_t *dest,
                                    const cdd_c_abstract_struct_t *src);

extern C_CDD_EXPORT /**
                     * @brief Free an abstract struct's contents.
                     * @param astruct The abstract struct to free.
                     * @return 0 on success, non-zero on error.
                     */
    int
    cdd_c_abstract_struct_free(cdd_c_abstract_struct_t *astruct);

extern C_CDD_EXPORT /**
                     * @brief Free a variant's contents.
                     * @param variant The variant to free.
                     * @return 0 on success, non-zero on error.
                     */
    int
    cdd_c_variant_free(cdd_c_variant_t *variant);

extern C_CDD_EXPORT /**
                     * @brief Serialize an abstract struct to a JSON string.
                     * @param astruct The abstract struct.
                     * @param out_json Pointer to receive the allocated JSON
                     * string.
                     * @return 0 on success, non-zero on error.
                     */
    int
    cdd_c_abstract_struct_to_json(const cdd_c_abstract_struct_t *astruct,
                                  char **out_json);

extern C_CDD_EXPORT /**
                     * @brief Deserialize a JSON string to an abstract struct.
                     * @param json The JSON string.
                     * @param out_astruct Pointer to the initialized abstract
                     * struct.
                     * @return 0 on success, non-zero on error.
                     */
    int
    cdd_c_abstract_struct_from_json(const char *json,
                                    cdd_c_abstract_struct_t *out_astruct);

extern C_CDD_EXPORT /**
                     * @brief Print an abstract struct for debugging.
                     * @param astruct The abstract struct.
                     */
    void
    cdd_c_abstract_print(const cdd_c_abstract_struct_t *astruct);

/**
 * @brief Retrieves the total number of bytes allocated for abstract structs
 * globally.
 * @return Byte count.
 */
C_CDD_EXPORT size_t cdd_c_get_allocated_bytes(void);

/**
 * @brief Retrieves the total number of free calls for abstract structs
 * globally.
 * @return Free count.
 */
C_CDD_EXPORT size_t cdd_c_get_freed_calls(void);

/**
 * @brief Represents a driver-agnostic column definition.
 */
typedef struct CddCColumnMeta {
  const char *name;
  int inferred_type; /* Treated as enum SqlDataType, int to avoid deep includes
                        if missing */
} cdd_c_column_meta_t;

/**
 * @brief Provides a generic dynamic builder to hydrate abstract structs from
 * raw generic driver row queries.
 *
 * @param out_astruct Pointer to initialize and populate with the row output.
 * @param row_data Void pointer array representing raw row bytes from a DB
 * driver.
 * @param cols Array of column metadata for the dataset.
 * @param n_cols Number of columns.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT int cdd_c_abstract_hydrate(cdd_c_abstract_struct_t *out_astruct,
                                        void **row_data,
                                        const cdd_c_column_meta_t *cols,
                                        size_t n_cols);

/**
 * @brief SQLite3 specific hydration hook. Extracts generic row types safely
 * using internal SQLite3 type metadata boundaries if native bindings are
 * enabled.
 *
 * @param out_astruct Pointer to populate.
 * @param stmt Opaque pointer to `sqlite3_stmt`.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT int
cdd_c_abstract_hydrate_sqlite3(cdd_c_abstract_struct_t *out_astruct,
                               void *stmt);

/**
 * @brief PostgreSQL libpq specific hydration hook. Extracts generic row types
 * safely using internal PQftype metadata boundaries if native bindings are
 * enabled.
 *
 * @param out_astruct Pointer to populate.
 * @param res Opaque pointer to `PGresult`.
 * @param row_index The current integer row boundary inside the PGresult buffer
 * to decode.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT int
cdd_c_abstract_hydrate_libpq(cdd_c_abstract_struct_t *out_astruct, void *res,
                             int row_index);

/**
 * @brief MySQL/MariaDB specific hydration hook. Extracts generic row types
 * safely using MYSQL_RES field metadata boundaries if native bindings are
 * enabled.
 *
 * @param out_astruct Pointer to populate.
 * @param row Opaque pointer to `MYSQL_ROW`.
 * @param fields Opaque pointer to `MYSQL_FIELD` array containing corresponding
 * length bounds.
 * @param num_fields The integer boundary size.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT int
cdd_c_abstract_hydrate_mysql(cdd_c_abstract_struct_t *out_astruct, void *row,
                             void *fields, unsigned int num_fields);

struct c_orm_meta;

/**
 * @brief Retrieves the offset of a field dynamically from a specific metadata
 * struct.
 * @param meta The reflection metadata.
 * @param field The field name to look up.
 * @return The offset in bytes, or (size_t)-1 if not found.
 */
C_CDD_EXPORT size_t cdd_c_meta_offsetof(const struct c_orm_meta *meta,
                                        const char *field);

/**
 * @brief Provide reflection utility macro helper checking metadata matching
 * dynamic row layouts.
 */
#define CDD_C_TYPEOF(c_orm_meta) ((c_orm_meta)->name)
#define CDD_C_OFFSETOF(c_orm_meta, field)                                      \
  cdd_c_meta_offsetof((c_orm_meta), (field))

/**
 * @brief Converts a hydrated specific C struct back into a dynamic abstract
 * dictionary.
 *
 * @param out_astruct Pointer to initialize and populate.
 * @param in_struct Pointer to populated specific C struct.
 * @param struct_meta Reflection metadata linking struct fields to struct
 * offsets.
 * @return 0 on success.
 */
C_CDD_EXPORT int
cdd_c_specific_to_abstract(cdd_c_abstract_struct_t *out_astruct,
                           const void *in_struct,
                           const struct c_orm_meta *struct_meta);

/**
 * @brief Converts a dynamic abstract dictionary cleanly into a static C struct.
 *
 * @param out_struct Pointer to allocated target struct instance.
 * @param in_astruct Populated abstract database row struct.
 * @param struct_meta Reflection metadata linking struct fields to struct
 * offsets.
 * @param strict_mapping If 1, fails if any field is missing or extraneous.
 * @return 0 on success.
 */
C_CDD_EXPORT int cdd_c_abstract_to_specific(
    void *out_struct, const cdd_c_abstract_struct_t *in_astruct,
    const struct c_orm_meta *struct_meta, int strict_mapping);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CLASSES_PARSE_ABSTRACT_STRUCT_H */
