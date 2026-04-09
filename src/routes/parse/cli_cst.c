#include "routes/parse/cli_cst.h"
#include "c_str_span.h"
#include "cdd_cst_transform.h"
#include "classes/emit/cdd_cst_emit.h"
#include "classes/parse/cdd_cst_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int process_file(const char *filepath,
                        int (*transform_fn)(cdd_cst_tree_t *,
                                            const cdd_transform_config_t *),
                        const cdd_transform_config_t *config,
                        int is_audit,
                        int is_dry_run) {
  FILE *f;
  long fsize;
  char *str;
  cdd_cst_tree_t *tree = NULL;
  char *out = NULL;
  int rc;
  FILE *out_f;

  f = fopen(filepath, "rb");
  if (!f) {
    fprintf(stderr, "Error opening %s\n", filepath);
    return 1;
  }
  fseek(f, 0, SEEK_END);
  fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  str = (char *)malloc((size_t)fsize + 1);
  if (!str) {
    fclose(f);
    return 1;
  }
  fread(str, 1, (size_t)fsize, f);
  str[fsize] = '\0';
  fclose(f);

  rc = cdd_cst_parse(az_span_create_from_str(str), &tree);
  if (rc != 0) {
    fprintf(stderr, "Error parsing %s\n", filepath);
    free(str);
    return rc;
  }

  rc = transform_fn(tree, config);
  if (rc != 0) {
    fprintf(stderr, "Error transforming %s\n", filepath);
    cdd_cst_tree_free(tree);
    free(str);
    return rc;
  }

  rc = cdd_cst_emit(tree, &out);
  cdd_cst_tree_free(tree);
  if (rc != 0) {
    fprintf(stderr, "Error emitting %s\n", filepath);
    free(str);
    return rc;
  }

  if (is_audit) {
    if (strcmp(str, out) != 0) {
      /* File needs fixing */
      fprintf(stdout, "%s needs formatting/fixes.\n", filepath);
      rc = 1;
    } else {
      rc = 0;
    }
  } else {
    /* is_fix */
    if (strcmp(str, out) != 0) {
      if (is_dry_run) {
        fprintf(stdout, "Would fix %s (dry run).\n", filepath);
        rc = 0;
      } else {
        out_f = fopen(filepath, "wb");
        if (!out_f) {
          fprintf(stderr, "Error opening %s for writing\n", filepath);
          free(str);
          free(out);
          return 1;
        }
        fwrite(out, 1, strlen(out), out_f);
        fclose(out_f);
        fprintf(stdout, "Fixed %s\n", filepath);
        rc = 0;
      }
    } else {
      rc = 0;
    }
  }

  free(str);
  free(out);
  return rc;
}

int cli_cst_transformer_main(int argc, char **argv) {
  int i;
  int rc = 0;
  int is_audit = 0;
  int is_fix = 0;
  int is_dry_run = 0;
  const char *toolname = NULL;
  cdd_transform_config_t config = {0, 2};
  int (*transform_fn)(cdd_cst_tree_t *, const cdd_transform_config_t *) = NULL;

  if (argc < 1) {
    fprintf(stderr, "Usage: cdd-c transformer <toolname> [--audit | --fix] [--dry-run] <files...>\n");
    return 1;
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
  } else if (strcmp(toolname, "--help") == 0 || strcmp(toolname, "-h") == 0) {
    fprintf(stdout, "Usage: cdd-c transformer <toolname> [--audit | --fix] [--dry-run] <files...>\n");
    fprintf(stdout, "Tools:\n");
    fprintf(stdout, "  extern_c\n");
    fprintf(stdout, "  msvc_port\n");
    fprintf(stdout, "  gnu_standardizer\n");
    fprintf(stdout, "  error_percolator\n");
    return 0;
  } else {
    fprintf(stderr, "Unknown tool: %s\n", toolname);
    return 1;
  }

  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--audit") == 0) {
      is_audit = 1;
    } else if (strcmp(argv[i], "--fix") == 0) {
      is_fix = 1;
    } else if (strcmp(argv[i], "--dry-run") == 0) {
      is_dry_run = 1;
    } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      fprintf(stdout, "Usage: cdd-c transformer %s [--audit | --fix] [--dry-run] <files...>\n", toolname);
      return 0;
    } else {
      /* Assume it's a file */
      if (!is_audit && !is_fix) {
        /* Default to fix if neither specified, to be safe or maybe require it?
           ADD_NEW_TOOLS.md says: my-ts-tool --audit /path */
        fprintf(stderr, "Must specify --audit or --fix.\n");
        return 1;
      }
      
      if (process_file(argv[i], transform_fn, &config, is_audit, is_dry_run) != 0) {
        rc = 1;
      }
    }
  }

  return rc;
}
