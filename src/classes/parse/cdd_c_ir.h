/**
 * @file cdd_c_ir.h
 * @brief Top-level Intermediate Representation for CDD-C
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CLASSES_PARSE_CDD_C_IR_H
#define C_CDD_CLASSES_PARSE_CDD_C_IR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stddef.h>
#include "c_cdd_export.h"
#include "classes/parse/sql.h"
#include "classes/parse/query_projection.h"
/* clang-format on */

/**
 * @brief Core Intermediate Representation tracking all tables and query
 * projections parsed from a file.
 */
typedef struct CddCIr {
  struct sql_table_t *tables;
  size_t n_tables;
  size_t capacity_tables;

  cdd_c_query_projection_t *projections;
  size_t n_projections;
  size_t capacity_projections;
} cdd_c_ir_t;

extern C_CDD_EXPORT /**
                     * @brief Initialize a new IR structure.
                     * @param ir The IR structure to initialize.
                     * @return 0 on success.
                     */
    int
    cdd_c_ir_init(cdd_c_ir_t *ir);

extern C_CDD_EXPORT /**
                     * @brief Add a table to the IR.
                     * @param ir The IR structure.
                     * @param table The table to add. Note: shallow copy or
                     * takes ownership, must specify. Takes ownership of
                     * pointers within for now.
                     * @return 0 on success.
                     */
    int
    cdd_c_ir_add_table(cdd_c_ir_t *ir, const struct sql_table_t *table);

extern C_CDD_EXPORT /**
                     * @brief Add a projection to the IR.
                     * @param ir The IR structure.
                     * @param proj The projection to add.
                     * @return 0 on success.
                     */
    int
    cdd_c_ir_add_projection(cdd_c_ir_t *ir,
                            const cdd_c_query_projection_t *proj);

extern C_CDD_EXPORT /**
                     * @brief Free all contents inside the IR.
                     * @param ir The IR to free.
                     * @return 0 on success.
                     */
    int
    cdd_c_ir_free(cdd_c_ir_t *ir);

extern C_CDD_EXPORT /**
                     * @brief Parse SQL data into the IR.
                     * @param sql_data The SQL source text.
                     * @param out_ir The IR structure to populate.
                     * @return 0 on success.
                     */
    int
    parse_sql_into_ir(const char *sql_data, cdd_c_ir_t *out_ir);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CLASSES_PARSE_CDD_C_IR_H */
