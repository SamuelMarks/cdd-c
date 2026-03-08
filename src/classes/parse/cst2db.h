/**
 * @file cst2db.h
 * @brief Parses Concrete Syntax Tree into a DatabaseSchema.
 */

#ifndef C_CDD_CST2DB_H
#define C_CDD_CST2DB_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "classes/emit/database.h"
#include "functions/parse/cst.h"

/**
 * @brief Parse a CST struct node into a DatabaseTable.
 * Looks for specific annotations or docstrings to mark as DB model.
 * 
 * @param node The CST Node containing the struct definition.
 * @param tokens The full token list.
 * @param table Output parsed table structure.
 * @return 0 on success, non-zero on error or if not a DB model.
 */
C_CDD_EXPORT int cst_parse_table(const struct CstNode *node,
                                 const struct TokenList *tokens,
                                 struct DatabaseTable *table);

/**
 * @brief Extract all DB models from a CST root.
 *
 * @param nodes The CST nodes list.
 * @param tokens The token list.
 * @param schema Output DatabaseSchema.
 * @return 0 on success.
 */
C_CDD_EXPORT int cst_extract_database_schema(const struct CstNodeList *nodes,
                                             const struct TokenList *tokens,
                                             struct DatabaseSchema *schema);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CST2DB_H */
