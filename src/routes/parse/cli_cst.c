/**
 * @file cli_cst.c
 * @brief Implementation of CLI CST transformation and multi-pass pipeline
 * handlers.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "routes/parse/cli_cst.h"
#include "c_cddConfig.h"
#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "c_str_span.h"
#include "cdd_cst_transform.h"
#include "classes/emit/cdd_cst_emit.h"
#include "classes/parse/cdd_cst_parser.h"
#include "functions/parse/fs.h"
#include "functions/parse/str.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Parsed command-line options for CST CLI invocations.
 */
struct cli_cst_options_t {
  cdd_transform_config_t config; /**< CST transformation configuration */
  unsigned int pass_mask;        /**< Enabled pipeline pass bitmask */
  int is_audit;                  /**< 1 for --audit mode */
  int is_fix;                    /**< 1 for --fix mode */
  int is_dry_run;                /**< 1 for --dry-run mode */
  int from_stdin;                /**< 1 to read C source from stdin */
  int to_stdout;                 /**< 1 to emit C source to stdout */
  int is_flat;                   /**< 1 to flatten --out-dir output paths */
  const char *out_file;          /**< Explicit -o / --output file path */
  const char *out_dir;           /**< Explicit --out-dir directory path */
  const char *root_dir;          /**< Optional --root-dir prefix to strip */
  const char **inc_dirs_buf;     /**< Heap array of -I include directories */
  size_t inc_dirs_count;         /**< Number of entries in inc_dirs_buf */
};

/** @brief Function signature for a single CST transformation pass. */
typedef cdd_c_error_t (*cdd_pass_fn_t)(cdd_cst_tree_t *,
                                       const cdd_transform_config_t *);

/*
 * @brief Runs a deterministic multi-pass CST transformation pipeline in memory.
 *
 * @param[in,out] tree The CST tree to transform.
 * @param[in] pass_mask Bitmask of `cdd_transform_pass_flags_t` passes to run.
 * @param[in] config Optional configuration for lowering passes.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_transform_pipeline(cdd_cst_tree_t *tree,
                                     unsigned int pass_mask,
                                     const cdd_transform_config_t *config) {
  static const struct {
    unsigned int flag;
    cdd_pass_fn_t fn;
  } passes[7] = {{CDD_PASS_MACROS, cdd_transform_macros},
                 {CDD_PASS_GNU, cdd_transform_gnu},
                 {CDD_PASS_C_LOWERING, cdd_transform_c_lowering},
                 {CDD_PASS_MSVC_PORT, cdd_transform_msvc},
                 {CDD_PASS_SAFE_CRT, cdd_transform_safe_crt},
                 {CDD_PASS_ERROR_PERCOLATOR, cdd_transform_percolate_errors},
                 {CDD_PASS_EXTERN_C, cdd_transform_extern_c}};
  cdd_transform_config_t eff_cfg;
  int ran_any = 0;
  size_t pi;
  cdd_c_error_t rc;

  if (!tree || !tree->root) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  memset(&eff_cfg, 0, sizeof(eff_cfg));
  if (config) {
    eff_cfg = *config;
  }

  if (eff_cfg.vla_mode == CDD_VLA_MODE_MALLOC) {
    eff_cfg.fallback_vla_to_malloc = 1;
  } else if (eff_cfg.fallback_vla_to_malloc) {
    eff_cfg.vla_mode = CDD_VLA_MODE_MALLOC;
  }

  if (eff_cfg.target_profile == CDD_TARGET_PROFILE_MSVC_LEGACY ||
      eff_cfg.target_profile == CDD_TARGET_PROFILE_WATCOM ||
      eff_cfg.target_profile == CDD_TARGET_PROFILE_ISO_C89) {
    eff_cfg.target_c89 = 1;
  } else if (eff_cfg.target_profile == CDD_TARGET_PROFILE_ISO_C99) {
    eff_cfg.target_c99 = 1;
  }

  for (pi = 0; pi < 7; pi++) {
    if (pass_mask & passes[pi].flag) {
      if (ran_any) {
        rc = cdd_cst_reparse_in_place(tree);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      ran_any = 1;
      rc = passes[pi].fn(tree, &eff_cfg);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses a comma-separated list of pass names into a bitmask.
 *
 * @param[in] csv Comma-separated pass list string.
 * @param[out] out_mask Output bitmask receiving `CDD_PASS_*` flags.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
static cdd_c_error_t parse_pass_list(const char *csv, unsigned int *out_mask) {
  const char *p = csv;
  unsigned int mask = 0;

  while (*p) {
    char tok[32];
    const char *comma = strchr(p, ',');
    size_t len = comma ? (size_t)(comma - p) : strlen(p);

    if (len == 0 || len >= sizeof(tok)) {
      fprintf(stderr, "Unknown transform pass in '%s'\n", csv);
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    memcpy(tok, p, len);
    tok[len] = '\0';

    if (strcmp(tok, "gnu") == 0 || strcmp(tok, "gnu_standardizer") == 0) {
      mask |= CDD_PASS_GNU;
    } else if (strcmp(tok, "c_lowering") == 0) {
      mask |= CDD_PASS_C_LOWERING;
    } else if (strcmp(tok, "msvc_port") == 0 || strcmp(tok, "msvc") == 0) {
      mask |= CDD_PASS_MSVC_PORT;
    } else if (strcmp(tok, "safe_crt") == 0) {
      mask |= CDD_PASS_SAFE_CRT;
    } else if (strcmp(tok, "macros") == 0) {
      mask |= CDD_PASS_MACROS;
    } else if (strcmp(tok, "extern_c") == 0) {
      mask |= CDD_PASS_EXTERN_C;
    } else if (strcmp(tok, "error_percolator") == 0 ||
               strcmp(tok, "percolate_errors") == 0) {
      mask |= CDD_PASS_ERROR_PERCOLATOR;
    } else if (strcmp(tok, "all") == 0) {
      mask |= (CDD_PASS_MACROS | CDD_PASS_GNU | CDD_PASS_C_LOWERING |
               CDD_PASS_MSVC_PORT | CDD_PASS_SAFE_CRT |
               CDD_PASS_ERROR_PERCOLATOR | CDD_PASS_EXTERN_C);
    } else if (strcmp(tok, "none") == 0) {
      /* Explicit no-op pass token */
    } else {
      fprintf(stderr, "Unknown transform pass in '%s'\n", csv);
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }

    if (!comma) {
      break;
    }
    p = comma + 1;
  }

  *out_mask = mask;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses `--target-msvc=<profile>` into `cdd_transform_config_t`.
 *
 * @param[in] val Profile string (`legacy`, `modern`, `watcom`, `iso-c89`,
 * `iso-c99`).
 * @param[in,out] cfg Configuration structure to populate.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
static cdd_c_error_t parse_target_profile(const char *val,
                                          cdd_transform_config_t *cfg) {
  if (strcmp(val, "legacy") == 0) {
    cfg->target_profile = CDD_TARGET_PROFILE_MSVC_LEGACY;
    cfg->target_c89 = 1;
    if (cfg->msvc_ver == 0)
      cfg->msvc_ver = 1400;
  } else if (strcmp(val, "modern") == 0) {
    cfg->target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;
    if (cfg->msvc_ver == 0)
      cfg->msvc_ver = 1928;
  } else if (strcmp(val, "watcom") == 0) {
    cfg->target_profile = CDD_TARGET_PROFILE_WATCOM;
    cfg->target_c89 = 1;
  } else if (strcmp(val, "iso-c89") == 0) {
    cfg->target_profile = CDD_TARGET_PROFILE_ISO_C89;
    cfg->target_c89 = 1;
  } else if (strcmp(val, "iso-c99") == 0) {
    cfg->target_profile = CDD_TARGET_PROFILE_ISO_C99;
    cfg->target_c99 = 1;
  } else {
    fprintf(stderr, "Unknown --target-msvc profile: %s\n", val);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses `--vla-mode=<alloca|malloc|hybrid>` into config.
 *
 * @param[in] val VLA mode string.
 * @param[in,out] cfg Configuration structure to update.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
static cdd_c_error_t parse_vla_mode(const char *val,
                                    cdd_transform_config_t *cfg) {
  if (strcmp(val, "alloca") == 0) {
    cfg->vla_mode = CDD_VLA_MODE_ALLOCA;
    cfg->fallback_vla_to_malloc = 0;
  } else if (strcmp(val, "malloc") == 0) {
    cfg->vla_mode = CDD_VLA_MODE_MALLOC;
    cfg->fallback_vla_to_malloc = 1;
  } else if (strcmp(val, "hybrid") == 0) {
    cfg->vla_mode = CDD_VLA_MODE_HYBRID;
    cfg->fallback_vla_to_malloc = 0;
  } else {
    fprintf(stderr, "Unknown --vla-mode: %s\n", val);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses `--safe-crt=<off|guarded|msvc-only>` into options.
 *
 * @param[in] val Safe CRT mode string.
 * @param[in,out] opts Options structure to update.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
static cdd_c_error_t parse_safe_crt_mode(const char *val,
                                         struct cli_cst_options_t *opts) {
  if (strcmp(val, "off") == 0) {
    opts->config.safe_crt_mode = CDD_SAFE_CRT_OFF;
    opts->pass_mask &= ~(unsigned int)CDD_PASS_SAFE_CRT;
  } else if (strcmp(val, "guarded") == 0) {
    opts->config.safe_crt_mode = CDD_SAFE_CRT_GUARDED;
    opts->pass_mask |= CDD_PASS_SAFE_CRT;
  } else if (strcmp(val, "msvc-only") == 0) {
    opts->config.safe_crt_mode = CDD_SAFE_CRT_MSVC_ONLY;
    opts->pass_mask |= CDD_PASS_SAFE_CRT;
  } else {
    fprintf(stderr, "Unknown --safe-crt mode: %s\n", val);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Appends an include search directory to `opts->inc_dirs_buf`.
 *
 * @param[in,out] opts Options structure owning `inc_dirs_buf`.
 * @param[in] dir Directory path string.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY on OOM.
 */
static cdd_c_error_t append_include_dir(struct cli_cst_options_t *opts,
                                        const char *dir) {
  const char **new_buf;
  size_t next_count = opts->inc_dirs_count + 1;

  new_buf = (const char **)C_CDD_REALLOC((void *)opts->inc_dirs_buf,
                                         next_count * sizeof(const char *));
  if (!new_buf) {
    return CDD_C_ERROR_MEMORY;
  }
  new_buf[opts->inc_dirs_count] = dir;
  opts->inc_dirs_buf = new_buf;
  opts->inc_dirs_count = next_count;
  opts->config.include_dirs = opts->inc_dirs_buf;
  opts->config.num_include_dirs = opts->inc_dirs_count;
  return CDD_C_SUCCESS;
}

/**
 * @brief Frees heap allocations inside `cli_cst_options_t`.
 *
 * @param[in,out] opts Options structure to clean up.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cli_cst_options_free(struct cli_cst_options_t *opts) {
  if (opts->inc_dirs_buf) {
    C_CDD_FREE((void *)opts->inc_dirs_buf);
    opts->inc_dirs_buf = NULL;
    opts->inc_dirs_count = 0;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Resolves the destination path for `-o <file>` or `--out-dir <dir>`.
 *
 * @param[in] filepath Input file path (or `"-"` for stdin).
 * @param[in] opts Parsed CLI options.
 * @param[out] out_resolved Newly allocated output file path string.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t
resolve_output_filepath(const char *filepath,
                        const struct cli_cst_options_t *opts,
                        char **out_resolved) {
  char *base = NULL;
  cdd_c_error_t rc;

  if (opts->out_file) {
    return c_cdd_strdup(opts->out_file, out_resolved);
  }

  if (!opts->is_flat && opts->root_dir) {
    size_t rlen = strlen(opts->root_dir);
    while (rlen > 0 && (opts->root_dir[rlen - 1] == '/' ||
                        opts->root_dir[rlen - 1] == '\\')) {
      rlen--;
    }
    if (rlen > 0 && strncmp(filepath, opts->root_dir, rlen) == 0 &&
        (filepath[rlen] == '/' || filepath[rlen] == '\\')) {
      return fs_path_join(opts->out_dir, filepath + rlen + 1, out_resolved);
    }
  }

  if (!opts->is_flat && filepath[0] != '/' && filepath[0] != '\\' &&
      strstr(filepath, "..") == NULL && strchr(filepath, ':') == NULL) {
    return fs_path_join(opts->out_dir, filepath, out_resolved);
  }

  rc = get_basename(filepath, &base);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  rc = fs_path_join(opts->out_dir, base, out_resolved);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(base);
    return rc;
  }
  C_CDD_FREE(base);
  return CDD_C_SUCCESS;
}

/**
 * @brief Writes transformed output to an explicit out-of-tree file path,
 * creating parent directories if needed.
 *
 * @param[in] dest_path Target file path to write.
 * @param[in] content Null-terminated emitted C source.
 * @param[in] is_dry_run Non-zero to print dry-run notice without writing.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t write_out_of_tree_file(const char *dest_path,
                                            const char *content,
                                            int is_dry_run) {
  char *parent_dir = NULL;
  FILE *out_f = NULL;
  cdd_c_error_t rc;

  if (is_dry_run) {
    fprintf(stdout, "Would write %s (dry run).\n", dest_path);
    return CDD_C_SUCCESS;
  }

  rc = get_dirname(dest_path, &parent_dir);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  if (strcmp(parent_dir, ".") != 0) {
    rc = makedirs(parent_dir);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(parent_dir);
      return rc;
    }
  }
  C_CDD_FREE(parent_dir);

#if defined(_MSC_VER)
  if (fopen_s(&out_f, dest_path, "wb") != 0)
    out_f = NULL;
#else
  out_f = fopen(dest_path, "wb");
#endif
  if (!out_f) {
    fprintf(stderr, "Error opening %s for writing\n", dest_path);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  fwrite(content, 1, strlen(content), out_f);
  fclose(out_f);
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses, transforms (via single pass or multi-pass pipeline), and
 * emits or audits a single C input (file or stdin).
 *
 * @param[in] filepath Path to C file, or `"-"` when reading from `stdin`.
 * @param[in] transform_fn Optional single-pass callback (NULL uses pipeline).
 * @param[in] opts CLI options and transformation configuration.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t
process_file_ex(const char *filepath,
                cdd_c_error_t (*transform_fn)(cdd_cst_tree_t *,
                                              const cdd_transform_config_t *),
                const struct cli_cst_options_t *opts) {
  char *str = NULL;
  cdd_cst_tree_t *tree = NULL;
  char *out = NULL;
  cdd_c_error_t rc;
  int read_stdin = (opts->from_stdin || strcmp(filepath, "-") == 0);

  if (read_stdin) {
    size_t stdin_sz = 0;
    rc = read_from_fh(stdin, &str, &stdin_sz);
    if (rc != CDD_C_SUCCESS) {
      fprintf(stderr, "Error reading from stdin\n");
      return rc;
    }
  } else {
    FILE *f;
    long fsize;
#if defined(_MSC_VER)
    if (fopen_s(&f, filepath, "rb") != 0)
      f = NULL;
#else
    f = fopen(filepath, "rb");
#endif
    if (!f) {
      fprintf(stderr, "Error opening %s\n", filepath);
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    str = (char *)C_CDD_MALLOC((size_t)fsize + 1);
    if (!str) {
      fclose(f);
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    {
      size_t bytes = fread(str, 1, (size_t)fsize, f);
      str[bytes] = '\0';
    }
    fclose(f);
  }

  rc = cdd_cst_parse(az_span_create_from_str(str), &tree);
  if (rc != CDD_C_SUCCESS) {
    fprintf(stderr, "Error parsing %s\n", filepath);
    C_CDD_FREE(str);
    return rc;
  }

  if (transform_fn) {
    rc = transform_fn(tree, &opts->config);
    if (rc != CDD_C_SUCCESS) {
      fprintf(stderr, "Error transforming %s\n", filepath);
      C_CDD_FREE(str);
      return cdd_c_keep_err(rc, cdd_cst_tree_free(tree));
    }
  } else {
    rc = cdd_transform_pipeline(tree, opts->pass_mask, &opts->config);
    if (rc != CDD_C_SUCCESS) {
      fprintf(stderr, "Error transforming %s\n", filepath);
      C_CDD_FREE(str);
      return cdd_c_keep_err(rc, cdd_cst_tree_free(tree));
    }
  }

  rc = cdd_cst_emit(tree, &out);
  if (rc != CDD_C_SUCCESS) {
    fprintf(stderr, "Error emitting %s\n", filepath);
    C_CDD_FREE(str);
    return cdd_c_keep_err(rc, cdd_cst_tree_free(tree));
  }
  rc = cdd_cst_tree_free(tree);

  if (opts->to_stdout || (read_stdin && !opts->out_file && !opts->out_dir)) {
    fwrite(out, 1, strlen(out), stdout);
  } else if (opts->out_file || opts->out_dir) {
    char *dest_path = NULL;
    rc = resolve_output_filepath(filepath, opts, &dest_path);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(str);
      C_CDD_FREE(out);
      return rc;
    }
    rc = write_out_of_tree_file(dest_path, out, opts->is_dry_run);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(dest_path);
      C_CDD_FREE(str);
      C_CDD_FREE(out);
      return rc;
    }
    C_CDD_FREE(dest_path);
  } else if (opts->is_audit) {
    if (strcmp(str, out) != 0) {
      fprintf(stdout, "%s needs formatting/fixes.\n", filepath);
      rc = CDD_C_ERROR_UNKNOWN;
    } else {
      rc = CDD_C_SUCCESS;
    }
  } else {
    if (strcmp(str, out) != 0) {
      if (opts->is_dry_run) {
        fprintf(stdout, "Would fix %s (dry run).\n", filepath);
        rc = CDD_C_SUCCESS;
      } else {
        FILE *out_f;
#if defined(_MSC_VER)
        if (fopen_s(&out_f, filepath, "wb") != 0)
          out_f = NULL;
#else
        out_f = fopen(filepath, "wb");
#endif
        if (!out_f) {
          fprintf(stderr, "Error opening %s for writing\n", filepath);
          C_CDD_FREE(str);
          C_CDD_FREE(out);
          return CDD_C_ERROR_INVALID_ARGUMENT;
        }
        fwrite(out, 1, strlen(out), out_f);
        fclose(out_f);
        fprintf(stdout, "Fixed %s\n", filepath);
        rc = CDD_C_SUCCESS;
      }
    } else {
      rc = CDD_C_SUCCESS;
    }
  }

  C_CDD_FREE(str);
  C_CDD_FREE(out);
  return rc;
}

/**
 * @brief Extracts the value for a recognized `--key=val` or `--key val` CLI
 * option.
 *
 * @param[in] argc Total argument count.
 * @param[in] argv Argument vector.
 * @param[in,out] idx Current argument index (incremented if next token used).
 * @param[in] plen Length of matched `--key` prefix.
 * @param[out] out_val Extracted value string.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if missing
 * or empty value.
 */
static cdd_c_error_t extract_opt_value(int argc, char **argv, int *idx,
                                       size_t plen, const char **out_val) {
  const char *arg = argv[*idx];
  *out_val = NULL;

  if (arg[plen] == '\0') {
    if (*idx + 1 >= argc) {
      fprintf(stderr, "Missing value for %s\n", arg);
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    *idx += 1;
    *out_val = argv[*idx];
    return CDD_C_SUCCESS;
  }

  if (arg[plen + 1] == '\0') {
    fprintf(stderr, "Empty value for %.*s\n", (int)plen, arg);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  *out_val = arg + plen + 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses a single common CLI flag into `cli_cst_options_t`.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @param[in,out] idx Current index in `argv`.
 * @param[in,out] opts Options struct to populate.
 * @param[out] out_consumed Set to 1 if `argv[*idx]` was a recognized flag.
 * @return CDD_C_SUCCESS on success, or error code on invalid option value.
 */
static cdd_c_error_t parse_common_cst_flag(int argc, char **argv, int *idx,
                                           struct cli_cst_options_t *opts,
                                           int *out_consumed) {
  static const char *const k_kv_keys[] = {
      "--output",   "--out-dir",     "--root-dir",   "--indent-width",
      "--passes",   "--target-msvc", "--msvc-ver",   "--vla-mode",
      "--safe-crt", "--rename-main", "--include-dir"};
  const char *arg = argv[*idx];
  const char *val = NULL;
  size_t k;
  cdd_c_error_t rc;

  *out_consumed = 1;

  if (strcmp(arg, "--audit") == 0) {
    opts->is_audit = 1;
    return CDD_C_SUCCESS;
  }
  if (strcmp(arg, "--fix") == 0) {
    opts->is_fix = 1;
    return CDD_C_SUCCESS;
  }
  if (strcmp(arg, "--dry-run") == 0) {
    opts->is_dry_run = 1;
    return CDD_C_SUCCESS;
  }
  if (strcmp(arg, "--stdin") == 0) {
    opts->from_stdin = 1;
    return CDD_C_SUCCESS;
  }
  if (strcmp(arg, "--stdout") == 0) {
    opts->to_stdout = 1;
    return CDD_C_SUCCESS;
  }
  if (strcmp(arg, "--flat") == 0) {
    opts->is_flat = 1;
    return CDD_C_SUCCESS;
  }
  if (strcmp(arg, "--use-tabs") == 0) {
    opts->config.use_tabs = 1;
    return CDD_C_SUCCESS;
  }
  if (strcmp(arg, "--target-c89") == 0) {
    opts->config.target_c89 = 1;
    return CDD_C_SUCCESS;
  }
  if (strcmp(arg, "--target-c99") == 0) {
    opts->config.target_c99 = 1;
    return CDD_C_SUCCESS;
  }
  if (strcmp(arg, "--with-auto-win-msvc") == 0) {
    opts->config.with_auto_win_msvc = 1;
    return CDD_C_SUCCESS;
  }
  if (strcmp(arg, "--fallback-alloca") == 0 ||
      strcmp(arg, "--fallback-malloc") == 0) {
    opts->config.vla_mode = CDD_VLA_MODE_MALLOC;
    opts->config.fallback_vla_to_malloc = 1;
    return CDD_C_SUCCESS;
  }

  if (strcmp(arg, "-o") == 0) {
    if (*idx + 1 >= argc) {
      fprintf(stderr, "Missing value for -o\n");
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    *idx += 1;
    opts->out_file = argv[*idx];
    return CDD_C_SUCCESS;
  }

  if (arg[0] == '-' && arg[1] == 'I') {
    if (arg[2] != '\0') {
      return append_include_dir(opts, arg + 2);
    }
    if (*idx + 1 >= argc) {
      fprintf(stderr, "Missing value for -I\n");
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    *idx += 1;
    return append_include_dir(opts, argv[*idx]);
  }

  for (k = 0; k < sizeof(k_kv_keys) / sizeof(k_kv_keys[0]); k++) {
    size_t plen = strlen(k_kv_keys[k]);
    if (strncmp(arg, k_kv_keys[k], plen) == 0 &&
        (arg[plen] == '\0' || arg[plen] == '=')) {
      rc = extract_opt_value(argc, argv, idx, plen, &val);
      if (rc != CDD_C_SUCCESS) {
        return rc;
      }
      switch (k) {
      case 0:
        opts->out_file = val;
        return CDD_C_SUCCESS;
      case 1:
        opts->out_dir = val;
        return CDD_C_SUCCESS;
      case 2:
        opts->root_dir = val;
        return CDD_C_SUCCESS;
      case 3:
        opts->config.indent_width = atoi(val);
        return CDD_C_SUCCESS;
      case 4: {
        return parse_pass_list(val, &opts->pass_mask);
      }
      case 5: {
        return parse_target_profile(val, &opts->config);
      }
      case 6: {
        int ver = atoi(val);
        if (ver <= 0) {
          fprintf(stderr, "Invalid --msvc-ver value: %s\n", val);
          return CDD_C_ERROR_INVALID_ARGUMENT;
        }
        opts->config.msvc_ver = ver;
        return CDD_C_SUCCESS;
      }
      case 7: {
        return parse_vla_mode(val, &opts->config);
      }
      case 8: {
        return parse_safe_crt_mode(val, opts);
      }
      case 9:
        opts->config.rename_main = val;
        return CDD_C_SUCCESS;
      default: {
        return append_include_dir(opts, val);
      }
      }
    }
  }

  *out_consumed = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Scans `argv[start_idx..argc)` for flags and executes file/stdin
 * inputs.
 *
 * @param[in] argc Total argument count.
 * @param[in] argv Argument vector.
 * @param[in] start_idx Starting index in `argv`.
 * @param[in] tool_help_name Optional tool name for `--help` inside
 * `transformer <tool>`, or NULL.
 * @param[in] transform_fn Optional single-pass callback (NULL uses pipeline).
 * @param[in,out] opts Options structure.
 * @param[in] allow_zero_files_noop 1 if zero files with no `--stdin` returns
 * success (legacy `transformer <tool>` / `standardize-gnu`), 0 to error.
 * @param[in] fail_fast 1 to abort on first file error, 0 to continue.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t run_cst_cli_args(
    int argc, char **argv, int start_idx, const char *tool_help_name,
    cdd_c_error_t (*transform_fn)(cdd_cst_tree_t *,
                                  const cdd_transform_config_t *),
    struct cli_cst_options_t *opts, int allow_zero_files_noop, int fail_fast) {
  int i;
  size_t num_files = 0;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  int is_out_of_tree;

  for (i = start_idx; i < argc; i++) {
    int consumed = 0;
    if (tool_help_name &&
        (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)) {
      fprintf(stdout,
              "Usage: cdd-c transformer %s [--audit | --fix] [--dry-run] "
              "[-o <out> | --out-dir <dir> | --stdin | --stdout] <files...>\n",
              tool_help_name);
      return cli_cst_options_free(opts);
    }
    rc = parse_common_cst_flag(argc, argv, &i, opts, &consumed);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cli_cst_options_free(opts));
    }
    if (!consumed) {
      if (argv[i][0] == '-' && argv[i][1] != '\0') {
        if (!fail_fast) {
          continue;
        }
        fprintf(stderr, "Unknown option: %s\n", argv[i]);
        return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                              cli_cst_options_free(opts));
      }
      num_files++;
    }
  }

  is_out_of_tree =
      (opts->out_file || opts->out_dir || opts->to_stdout || opts->from_stdin);

  if (opts->out_file && num_files > 1) {
    fprintf(stderr, "Cannot use -o with multiple input files; use --out-dir\n");
    return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                          cli_cst_options_free(opts));
  }

  if (opts->from_stdin && num_files == 0) {
    rc = process_file_ex("-", transform_fn, opts);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cli_cst_options_free(opts));
    }
    return cli_cst_options_free(opts);
  }

  if (num_files == 0) {
    if (allow_zero_files_noop) {
      return cli_cst_options_free(opts);
    }
    fprintf(stderr, "No input files specified.\n");
    return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                          cli_cst_options_free(opts));
  }

  if (!is_out_of_tree && !opts->is_audit && !opts->is_fix) {
    fprintf(stderr, "Must specify --audit or --fix.\n");
    return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                          cli_cst_options_free(opts));
  }

  for (i = start_idx; i < argc; i++) {
    int consumed = 0;
    struct cli_cst_options_t skip_opts;
    cdd_c_error_t skip_rc;
    memset(&skip_opts, 0, sizeof(skip_opts));
    skip_rc = parse_common_cst_flag(argc, argv, &i, &skip_opts, &consumed);
    if (skip_rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(skip_rc,
                            cdd_c_keep_err(cli_cst_options_free(&skip_opts),
                                           cli_cst_options_free(opts)));
    }
    skip_rc = cli_cst_options_free(&skip_opts);
    if (!consumed) {
      if (!fail_fast && argv[i][0] == '-' && argv[i][1] != '\0') {
        continue;
      }
      {
        cdd_c_error_t p_rc = process_file_ex(argv[i], transform_fn, opts);
        if (p_rc != CDD_C_SUCCESS) {
          if (fail_fast) {
            return cdd_c_keep_err(p_rc, cli_cst_options_free(opts));
          }
          rc = CDD_C_ERROR_UNKNOWN;
        }
      }
    }
  }

  return cdd_c_keep_err(rc, cli_cst_options_free(opts));
}

/*
 * @brief Subcommand handler for cdd-c transformer.
 *
 * @param[in] argc Argument count.
 * @param[out] argv Argument values.
 * @return 0 on success.
 */
cdd_c_error_t cli_cst_transformer_main(int argc, char **argv) {
  const char *toolname = NULL;
  struct cli_cst_options_t opts;
  cdd_c_error_t (*transform_fn)(cdd_cst_tree_t *,
                                const cdd_transform_config_t *) = NULL;

  memset(&opts, 0, sizeof(opts));

  if (argc < 1) {
    fprintf(stderr, "Usage: cdd-c transformer <toolname> [--audit | --fix] "
                    "[--dry-run] <files...>\n");
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  toolname = argv[0];
  if (strcmp(toolname, "extern_c") == 0) {
    transform_fn = cdd_transform_extern_c;
  } else if (strcmp(toolname, "msvc_port") == 0) {
    transform_fn = cdd_transform_msvc;
  } else if (strcmp(toolname, "gnu_standardizer") == 0) {
    transform_fn = cdd_transform_gnu;
  } else if (strcmp(toolname, "error_percolator") == 0) {
    transform_fn = cdd_transform_percolate_errors;
  } else if (strcmp(toolname, "safe_crt") == 0) {
    transform_fn = cdd_transform_safe_crt;
  } else if (strcmp(toolname, "macros") == 0) {
    transform_fn = cdd_transform_macros;
  } else if (strcmp(toolname, "--help") == 0 || strcmp(toolname, "-h") == 0) {
    fprintf(stdout, "Usage: cdd-c transformer <toolname> [--audit | --fix] "
                    "[--dry-run] <files...>\n");
    fprintf(stdout, "Tools:\n");
    fprintf(stdout, "  extern_c\n");
    fprintf(stdout, "  msvc_port\n");
    fprintf(stdout, "  gnu_standardizer\n");
    fprintf(stdout, "  error_percolator\n");
    fprintf(stdout, "  safe_crt\n");
    fprintf(stdout, "  macros\n");
    return CDD_C_SUCCESS;
  } else {
    fprintf(stderr, "Unknown tool: %s\n", toolname);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  return run_cst_cli_args(argc, argv, 1, toolname, transform_fn, &opts, 1, 1);
}

/*
 * @brief Subcommand handler for standardize-gnu.
 *
 * @param[in] argc Argument count.
 * @param[out] argv Argument values.
 * @return 0 on success.
 */
cdd_c_error_t cli_standardize_gnu_main(int argc, char **argv) {
  struct cli_cst_options_t opts;
  memset(&opts, 0, sizeof(opts));

  if (argc < 1) {
    fprintf(stderr, "Usage: cdd-c standardize-gnu [OPTIONS] <files...>\n");
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (strcmp(argv[0], "--help") == 0 || strcmp(argv[0], "-h") == 0) {
    fprintf(stdout, "Usage: cdd-c standardize-gnu [OPTIONS] <files...>\n");
    fprintf(stdout, "Options:\n");
    fprintf(stdout, "  --target-c89       Strictly target C89 semantics\n");
    fprintf(stdout, "  --target-c99       Target C99 semantics\n");
    fprintf(stdout, "  --vla-mode=<mode>  alloca | malloc | hybrid\n");
    fprintf(stdout, "  --fallback-alloca  Alias for --vla-mode=malloc\n");
    fprintf(stdout, "  -o, --output <f>   Write output to file\n");
    fprintf(stdout, "  --out-dir <dir>    Write outputs into directory\n");
    fprintf(stdout, "  --stdin/--stdout   Stream C source via stdio\n");
    fprintf(stdout,
            "  --audit            Check if files need formatting/fixes\n");
    fprintf(stdout, "  --fix              Apply fixes in-place\n");
    fprintf(stdout, "  --dry-run          Show what would be fixed without "
                    "modifying files\n");
    return CDD_C_SUCCESS;
  }

  return run_cst_cli_args(argc, argv, 0, NULL, cdd_transform_gnu, &opts, 1, 0);
}

/*
 * @brief Subcommand handler for `cdd-c transform` multi-pass pipeline.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument values.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cli_cst_transform_pipeline_main(int argc, char **argv) {
  int i;
  struct cli_cst_options_t opts;

  memset(&opts, 0, sizeof(opts));
  opts.pass_mask = (CDD_PASS_GNU | CDD_PASS_C_LOWERING | CDD_PASS_MSVC_PORT);

  if (argc < 1) {
    fprintf(stderr,
            "Usage: cdd-c transform [--passes=gnu,c_lowering,msvc_port] "
            "[--target-msvc=<legacy|modern|watcom|iso-c89|iso-c99>] "
            "<in.c> -o <out.c>\n");
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      fprintf(stdout, "Usage: cdd-c transform [OPTIONS] <in.c> [-o <out.c> | "
                      "--out-dir <dir> | --stdout]\n");
      fprintf(stdout, "Options:\n");
      fprintf(stdout, "  --passes=<list>             Comma-separated passes\n");
      fprintf(stdout,
              "  --target-msvc=<profile>     legacy|modern|watcom|iso-c89|"
              "iso-c99\n");
      fprintf(stdout,
              "  --msvc-ver=<1200..1999>     Explicit _MSC_VER target\n");
      fprintf(stdout,
              "  --with-auto-win-msvc        Target auto-win-msvc headers\n");
      fprintf(stdout,
              "  --vla-mode=<mode>           alloca | malloc | hybrid\n");
      fprintf(stdout,
              "  --safe-crt=<mode>           off | guarded | msvc-only\n");
      fprintf(stdout,
              "  --rename-main=<sym>         Rename main entry symbol\n");
      fprintf(stdout, "  -I, --include-dir <dir>     Header search path\n");
      fprintf(stdout, "  -o, --output <file>         Output C source path\n");
      fprintf(stdout,
              "  --out-dir <dir>             Output directory for sources\n");
      fprintf(
          stdout,
          "  --root-dir <dir>            Strip root prefix for --out-dir\n");
      fprintf(stdout, "  --flat                      Flatten subdirectories\n");
      fprintf(stdout,
              "  --stdin / --stdout          Stream C source via stdio\n");
      return CDD_C_SUCCESS;
    }
  }

  return run_cst_cli_args(argc, argv, 0, NULL, NULL, &opts, 0, 1);
}

/*
 * @brief Standalone CLI dispatcher for `cdd-c-transform`
 * (`CDD_C_TRANSFORMER_ONLY`).
 *
 * @param[in] argc Argument count (including `argv[0]` program name).
 * @param[in] argv Argument vector.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cli_transform_dispatch_main(int argc, char **argv) {
  const char *cmd;

  if (argc < 2 || !argv) {
    fprintf(stderr,
            "Usage: cdd-c-transform <transform|transformer|standardize-gnu> "
            "[OPTIONS] <files...>\n");
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  cmd = argv[1];
  if (strcmp(cmd, "--help") == 0 || strcmp(cmd, "-h") == 0) {
    fprintf(stdout,
            "Usage: cdd-c-transform <transform|transformer|standardize-gnu> "
            "[OPTIONS] <files...>\n");
    fprintf(stdout, "Commands:\n");
    fprintf(stdout,
            "  transform         Multi-pass out-of-tree CST pipeline\n");
    fprintf(stdout, "  transformer       Single-pass CST transformer\n");
    fprintf(stdout, "  standardize-gnu   GNU C to ISO C standardizer\n");
    return CDD_C_SUCCESS;
  }

  if (strcmp(cmd, "--version") == 0 || strcmp(cmd, "-v") == 0) {
    fprintf(stdout, "cdd-c-transform version %s\n", C_CDD_VERSION);
    return CDD_C_SUCCESS;
  }

  if (strcmp(cmd, "transform") == 0) {
    return cli_cst_transform_pipeline_main(argc - 2, argv + 2);
  }
  if (strcmp(cmd, "transformer") == 0) {
    return cli_cst_transformer_main(argc - 2, argv + 2);
  }
  if (strcmp(cmd, "standardize-gnu") == 0) {
    return cli_standardize_gnu_main(argc - 2, argv + 2);
  }
  if (cmd[0] == '-') {
    return cli_cst_transform_pipeline_main(argc - 1, argv + 1);
  }

  fprintf(stderr, "Unknown command: %s\n", cmd);
  return CDD_C_ERROR_INVALID_ARGUMENT;
}
