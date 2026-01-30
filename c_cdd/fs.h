/**
 * @file fs.h
 * @brief Filesystem utility functions for safe I/O, path manipulation, and
 * directory management.
 * @author Samuel Marks
 */

#ifndef FS_H
#define FS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <errno.h>
#include <stdio.h>

#include <c_cdd_export.h>

/* Windows compatibility macros and types */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define PATH_SEP "\\"
#define PATH_SEP_C '\\'
#define strtok_r strtok_s
typedef struct _stat32 c_stat;

/* Type definitions for Windows HANDLEs to avoid including windows.h globally */
typedef void *PVOID;
typedef PVOID HANDLE;
typedef struct HWND__ *HWND;

#include <direct.h>
#include <io.h>
#include <sys/stat.h>

/* Map POSIX constants/functions to MSVC equivalents */
#ifndef F_OK
#define F_OK 0
#endif /* !F_OK */

#ifndef W_OK
#define W_OK 2
#endif /* !W_OK */

#define access _access
#define rmdir _rmdir

#ifndef PATH_MAX
#define PATH_MAX _MAX_PATH
#endif /* !PATH_MAX */

#define delete_file remove
#define unlink _unlink

/**
 * @brief Check if a path is a UNC path (Windows only).
 *
 * @param[in] path The path to check.
 * @return 1 if UNC, 0 otherwise.
 */
extern C_CDD_EXPORT int path_is_unc(const char *path);

/**
 * @brief Convert ASCII string to Wide string (Windows only).
 *
 * @param[in] s Source ASCII string.
 * @param[out] ws Destination Wide string buffer.
 * @param[in] buf_cap Capacity of the destination buffer in wchar_t elements.
 * @param[out] out_len Pointer to store the number of characters written.
 * @return 0 on success, non-zero error code on failure.
 */
extern C_CDD_EXPORT int ascii_to_wide(const char *s, wchar_t *ws,
                                      size_t buf_cap, size_t *out_len);

/**
 * @brief Convert Wide string to ASCII string (Windows only).
 *
 * @param[in] ws Source Wide string.
 * @param[out] s Destination ASCII string buffer.
 * @param[in] buf_cap Capacity of the destination buffer in bytes.
 * @param[out] out_len Pointer to store the number of bytes written.
 * @return 0 on success, non-zero error code on failure.
 */
extern C_CDD_EXPORT int wide_to_ascii(const wchar_t *ws, char *s,
                                      size_t buf_cap, size_t *out_len);

#else
/* POSIX systems */
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct stat c_stat;

#define PATH_SEP "/"
#define PATH_SEP_C '/'
#define delete_file unlink

#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) */

/**
 * @brief Error codes mapping to standard errno values where applicable,
 * or custom codes for `fopen` specific failures.
 */
enum FopenError {
  FOPEN_OK = 0,                       /**< No error */
  FOPEN_INVALID_PARAMETER = EINVAL,   /**< Invalid input parameters */
  FOPEN_TOO_MANY_OPEN_FILES = EMFILE, /**< Too many open files */
  FOPEN_OUT_OF_MEMORY = ENOMEM,       /**< Memory allocation failed */
  FOPEN_FILE_NOT_FOUND = ENOENT,      /**< File not found */
  FOPEN_PERMISSION_DENIED = EACCES,   /**< Permission denied */
  FOPEN_FILENAME_TOO_LONG = ERANGE,   /**< Filename too long */
  FOPEN_UNKNOWN_ERROR = -1            /**< Unknown error */
};

/**
 * @brief Helper to convert internal standard library errno to FopenError.
 *
 * @param[in] fopen_error The errno value representing an error.
 * @return The corresponding FopenError enum value.
 */
extern C_CDD_EXPORT enum FopenError fopen_error_from(int fopen_error);

/**
 * @brief Struct to hold a file handle and its associated filename.
 * Used to avoid TOCTOU (Time-of-check to time-of-use) race conditions in
 * temporary file creation.
 */
struct FilenameAndPtr {
  FILE *fh;       /**< Open file handle */
  char *filename; /**< Path to the file (malloced) */
};

/**
 * @brief Extract the base name (filename component) from a path.
 * Allocates a new string which the caller must free.
 *
 * @param[in] path The full path.
 * @param[out] out Pointer to char* where the allocated string will be stored.
 * @return 0 on success, ENOMEM on allocation failure, EINVAL if args invalid.
 */
extern C_CDD_EXPORT int get_basename(const char *path, char **out);

/**
 * @brief Extract the directory component from a path.
 * Allocates a new string which the caller must free.
 *
 * @param[in] path The full path.
 * @param[out] out Pointer to char* where the allocated string will be stored.
 * @return 0 on success, ENOMEM on allocation failure, EINVAL if args invalid.
 */
extern C_CDD_EXPORT int get_dirname(const char *path, char **out);

/**
 * @brief Read entire file content into a buffer.
 * Allocates the buffer which the caller must free.
 *
 * @param[in] path Path to the file to open.
 * @param[in] mode Mode string for fopen (e.g. "r", "rb").
 * @param[out] out_data Pointer to char* where allocated data will be stored.
 * @param[out] out_size Pointer to size_t where data length will be stored.
 * @return 0 on success, or an error code (errno) on failure.
 */
extern C_CDD_EXPORT int read_to_file(const char *path, const char *mode,
                                     char **out_data, size_t *out_size);

/**
 * @brief Read entire content from an open file stream.
 * Allocates the buffer which the caller must free. Does not close the stream.
 *
 * @param[in] fh Open file handle.
 * @param[out] out_data Pointer to char* where allocated data will be stored.
 * @param[out] out_size Pointer to size_t where data length will be stored.
 * @return 0 on success, or an error code (errno) on failure.
 */
extern C_CDD_EXPORT int read_from_fh(FILE *fh, char **out_data,
                                     size_t *out_size);

/**
 * @brief Copy a file from source to destination.
 * Fails if destination already exists.
 *
 * @param[in] dst Destination path.
 * @param[in] src Source path.
 * @return 0 on success, non-zero error code on failure.
 */
extern C_CDD_EXPORT int cp(const char *dst, const char *src);

/**
 * @brief Create a directory.
 * Fails if the directory already exists (unless implementation specific,
 * generally wraps mkdir).
 *
 * @param[in] path Path of directory to create.
 * @return 0 on success, non-zero error code on failure.
 */
extern C_CDD_EXPORT int makedir(const char *path);

/**
 * @brief Create a directory recursively (like `mkdir -p`).
 *
 * @param[in] path Path of directory tree to create.
 * @return 0 on success, non-zero error code from `mkdir` or `stat` on failure.
 */
extern C_CDD_EXPORT int makedirs(const char *path);

/**
 * @brief Get a temporary directory path.
 * Allocates a new string which the caller must free.
 *
 * @param[out] out_path Pointer to char* where the path string will be stored.
 * @return 0 on success, ENOMEM or other error code on failure.
 */
extern C_CDD_EXPORT int tempdir(char **out_path);

/**
 * @brief Cleanup FilenameAndPtr struct (close file and free filename).
 *
 * @param[in] file Pointer to struct to clean.
 */
extern C_CDD_EXPORT void FilenameAndPtr_cleanup(struct FilenameAndPtr *file);

/**
 * @brief Cleanup struct and delete the file from the filesystem.
 *
 * @param[in] file Pointer to struct.
 */
extern C_CDD_EXPORT void
FilenameAndPtr_delete_and_cleanup(struct FilenameAndPtr *file);

/**
 * @brief Create a temporary file with a random name, open it, and return handle
 * + name.
 *
 * @param[in] prefix Optional prefix for the filename (can be NULL).
 * @param[in] suffix Optional suffix for the filename (can be NULL).
 * @param[in] mode Open mode (e.g. "w+", "wb").
 * @param[out] file Output struct containing FILE* and filename string.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int mktmpfilegetnameandfile(const char *prefix,
                                                const char *suffix,
                                                const char *mode,
                                                struct FilenameAndPtr *file);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !FS_H */
