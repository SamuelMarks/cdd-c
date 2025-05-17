#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "generate_build_system.h"


static void write_cmake(const char *basename, const char *test_file)
{
    FILE *f;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    errno_t err = fopen_s(&f, "CMakeLists.txt", "w");
    if (err != 0 || f == NULL) {
      fputs("Failed to open CMakeLists.txt for writing", stderr);
      exit(EXIT_FAILURE);
    }
#else
    f = fopen("CMakeLists.txt", "w");
    if (!f) {
        fputs("Failed to open CMakeLists.txt for writing", stderr);
        exit(EXIT_FAILURE);
    }
#endif
    fprintf(f, "cmake_minimum_required(VERSION 3.10)\n");
    fprintf(f, "project(%s C)\n\n", basename);

    fprintf(f, "add_library(%s STATIC %s.c %s.h)\n\n", basename, basename, basename);

    fprintf(f, "target_include_directories(%s PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})\n\n", basename);

    if (test_file && test_file[0] != '\0') {
        fprintf(f, "add_executable(%s_test %s)\n", basename, test_file);
        fprintf(f, "target_link_libraries(%s_test PRIVATE %s)\n\n", basename, basename);
        fprintf(f, "enable_testing()\n");
        fprintf(f, "add_test(NAME %s_test COMMAND %s_test)\n", basename, basename);
    }

    fclose(f);
    puts("Generated CMakeLists.txt");
}

static void write_makefile(const char *basename, const char *test_file)
{
    FILE *f;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  errno_t err = fopen_s(&f, "Makefile", "w");
  if (err != 0 || f == NULL) {
    fputs("Failed to open Makefile for writing", stderr);
    exit(EXIT_FAILURE);
  }
#else
  f = fopen("Makefile", "w");
    if (!f) {
        fprintf(stderr, "Failed to open Makefile for writing\n");
        exit(EXIT_FAILURE);
    }
#endif


    fprintf(f,
            "CC ?= gcc\n"
            "CFLAGS ?= -Wall -Wextra -std=c89 -g\n"
            "TARGET = lib%s.a\n"
            "OBJS = %s.o\n"
            "TEST_FILE = %s\n"
            "\n"
            ".PHONY: all clean test\n"
            "\n"
            "all: $(TARGET)\n"
            "\n"
            "$(TARGET): $(OBJS)\n"
            "\tar rcs $@ $^\n"
            "\n"
            "%%.o: %%.c %%.h\n"
            "\t$(CC) $(CFLAGS) -c $< -o $@\n"
            "\n"
            "test: $(TARGET)\n",
            basename,
            basename,
            (test_file && test_file[0] != '\0') ? test_file : "");

    if (test_file && test_file[0] != '\0') {
        fprintf(f,
                "\t$(CC) $(CFLAGS) -o test_runner $(TEST_FILE) $(TARGET)\n"
                "\t./test_runner\n");
    } else {
        fprintf(f, "\t@echo \"No test file provided\"\n");
    }

    fprintf(f,
            "\nclean:\n"
            "\trm -f $(OBJS) $(TARGET) test_runner\n");

    fclose(f);
    puts("Generated Makefile");
}

static void write_meson(const char *basename, const char *test_file)
{
    FILE *f;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  errno_t err = fopen_s(&f, "meson.build", "w");
  if (err != 0 || f == NULL) {
    fputs("Failed to open Makefile for writing", stderr);
    exit(EXIT_FAILURE);
  }
#else
  f = fopen("meson.build", "w");
    if (!f) {
        fprintf(stderr, "Failed to open meson.build for writing\n");
        exit(EXIT_FAILURE);
    }
#endif

    fprintf(f, "project('%s', 'c', version : '0.1')\n\n", basename);

    fprintf(f,
            "lib = static_library(\n"
            "  '%s',\n"
            "  '%s.c',\n"
            "  include_directories: include_directories('.')\n"
            ")\n\n",
            basename, basename);

    if (test_file && test_file[0] != '\0') {
        fprintf(f,
                "test_exe = executable(\n"
                "  '%s_test',\n"
                "  '%s',\n"
                "  link_with: lib,\n"
                "  install: false\n"
                ")\n\n"
                "test('run_tests', test_exe)\n",
                basename, test_file);
    }

    fclose(f);
    puts("Generated meson.build");
}

static void write_bazel(const char *basename, const char *test_file)
{
    FILE *f;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  errno_t err = fopen_s(&f, header_filename, "r");
  if (err != 0 || f == NULL) {
    fputs("Failed to open CMakeLists.txt for writing", stderr);
    exit(EXIT_FAILURE);
  }
#else
    f = fopen("BUILD", "w");
    if (!f) {
        fprintf(stderr, "Failed to open BUILD for writing\n");
        exit(EXIT_FAILURE);
    }
#endif
    fprintf(f,
            "cc_library(\n"
            "    name = \"%s\",\n"
            "    srcs = [\"%s.c\"],\n"
            "    hdrs = [\"%s.h\"],\n"
            "    visibility = [\"//visibility:public\"],\n"
            ")\n\n",
            basename, basename, basename);

    if (test_file && test_file[0] != '\0') {
        fprintf(f,
                "cc_binary(\n"
                "    name = \"%s_test\",\n"
                "    srcs = [\"%s\"],\n"
                "    deps = [\":%s\"],\n"
                "    visibility = [\"//visibility:public\"],\n"
                ")\n",
                basename, test_file, basename);
    }

    fclose(f);
    puts("Generated BUILD");
}

int generate_build_system_main(int argc, char **argv) {
  if (argc < 2 || argc > 3) {
    fputs("Usage: generate_build <build_system> <basename> [test_file]\n"
          "build_system: cmake | make | meson | bazel\n"
          "basename: base name for .c and .h files\n"
          "test_file: optional .c test file", stderr);
    return EXIT_FAILURE;
  }

  {
    const char *const build_system = argv[0];
    const char *const basename = argv[1];
    const char *const test_file = (argc == 3) ? argv[2] : NULL;

    if (strcmp(build_system, "cmake") == 0) {
      write_cmake(basename, test_file);
    } else if (strcmp(build_system, "make") == 0) {
      write_makefile(basename, test_file);
    } else if (strcmp(build_system, "meson") == 0) {
      write_meson(basename, test_file);
    } else if (strcmp(build_system, "bazel") == 0) {
      write_bazel(basename, test_file);
    } else {
      fprintf(stderr, "Unsupported build system: %s\n", build_system);
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
