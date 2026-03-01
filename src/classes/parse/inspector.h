/**
 * @file c_inspector.h
 * @brief High-level AST Inspector for C Code.
 *
 * Provides utilities to extract semantic information from C source and header
 * files, including:
 * - Scanning for Type Definitions (Structs and Enums).
 * - Extracting Function Signatures (Prototypes).
 *
 * This module is used by synchronization tools to align implementation files
 * with header definitions and to verify consistency.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_C_INSPECTOR_H
#define C_CDD_C_INSPECTOR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>

#include "c_cdd_export.h"
#include "c_cdd_stdbool.h"
#include "classes/emit/enum.h"   /* For EnumMembers */
#include "classes/emit/struct.h" /* For StructFields */
#include "functions/parse/tokenizer.h"

/**
 * @brief Categorization of a detected type definition.
 */
enum TypeDefinitionKind {
  KIND_ENUM,  /**< An Enumeration (enum X { ... }) */
  KIND_STRUCT /**< A Structure (struct X { ... }) */
};

/**
 * @brief Represents a registered type definition found in a file.
 * Owns its string members.
 */
struct TypeDefinition {
  enum TypeDefinitionKind kind; /**< The kind of type definition */
  char *name;                   /**< Name of the type (e.g., "MyStruct") */

  /* Detailed parse info. Union based on kind. */
  union {
    struct EnumMembers *enum_members;   /**< Populated if kind == KIND_ENUM */
    struct StructFields *struct_fields; /**< Populated if kind == KIND_STRUCT */
  } details;
};

/**
 * @brief List of detected type definitions.
 */
struct TypeDefList {
  struct TypeDefinition *items; /**< Array of definitions */
  size_t size;                  /**< Number of items */
  size_t capacity;              /**< Internal capacity */
};

/**
 * @brief Represents a function signature extracted from source code.
 */
struct FuncSignature {
  char *name; /**< Function name (e.g., "my_func") */
  char *sig;  /**< Full signature string (e.g., "int my_func(int x)") */
};

/**
 * @brief List of function signatures.
 */
struct FuncSigList {
  struct FuncSignature *items; /**< Array of signatures */
  size_t size;                 /**< Number of items */
  size_t capacity;             /**< Internal capacity */
};

/* --- Type Definitions API --- */

/**
 * @brief Initialize a TypeDefList.
 * @param[out] list The list to initialize.
 * @return 0 on success, EINVAL if list is NULL.
 */
extern C_CDD_EXPORT int type_def_list_init(struct TypeDefList *list);

/**
 * @brief Free resources in a TypeDefList.
 * Frees all names and nested detail structures (EnumMembers/StructFields).
 * @param[in] list The list to free.
 */
extern C_CDD_EXPORT void type_def_list_free(struct TypeDefList *list);

/**
 * @brief Scan a file (by path) and extract all struct/enum definitions.
 *
 * Parses the file (using line-based heuristics closer to traditional C parsing)
 * to populate the list with any `struct` or `enum` definition blocks found.
 *
 * @param[in] filename Path to the C header or source file.
 * @param[out] out Destination list to populate.
 * @return 0 on success, error code (ENOENT/EIO/ENOMEM) on failure.
 */
extern C_CDD_EXPORT int c_inspector_scan_file_types(const char *filename,
                                                    struct TypeDefList *out);

/* --- Function Signatures API --- */

/**
 * @brief Initialize FuncSigList.
 * @param[out] list The list to initialize.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int func_sig_list_init(struct FuncSigList *list);

/**
 * @brief Free FuncSigList.
 * Frees name and signature strings.
 * @param[in] list The list to free.
 */
extern C_CDD_EXPORT void func_sig_list_free(struct FuncSigList *list);

/**
 * @brief Extract all function signatures from a source string.
 *
 * Uses the CST parser to identify function definitions and extracts their
 * full signature (return type + name + args) up to the opening brace.
 *
 * @param[in] source_code Raw C source string.
 * @param[out] out Destination list to populate.
 * @return 0 on success, ENOMEM on failure.
 */
extern C_CDD_EXPORT int c_inspector_extract_signatures(const char *source_code,
                                                       struct FuncSigList *out);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_C_INSPECTOR_H */
