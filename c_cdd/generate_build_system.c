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
  fprintf(f,
          "cmake_minimum_required(VERSION 3.10)\n"
          "project(%s LANGUAGES C)\n\n"
          "set(CMAKE_C_STANDARD 90)\n"
          "set(CMAKE_C_STANDARD_REQUIRED ON)\n"
          "# Enable strict C90 mode and strict warnings\n"
          "if(MSVC)\n"
          "  add_compile_options(/W4 /Za)\n"
          "else()\n"
          "  add_compile_options(-std=c90 -Wall -Wextra -pedantic)\n"
          "endif()\n\n"
          "add_library(%s %s.c %s.h)\n\n"
          "target_include_directories(%s PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})\n\n",
          basename, basename, basename, basename, basename);

  if (test_file && test_file[0] != '\0') {
    /* Setup greatest.h download */
    fprintf(f,
            "# Download greatest.h if needed\n"
            "include(FetchContent)\n"
            "set(GREATEST_URL \"https://raw.githubusercontent.com/silentbicycle/greatest/master/greatest.h\")\n"
            "set(GREATEST_FILE \"${CMAKE_BINARY_DIR}/greatest.h\")\n"
            "if(NOT EXISTS ${GREATEST_FILE})\n"
            "  file(DOWNLOAD ${GREATEST_URL} ${GREATEST_FILE} SHOW_PROGRESS)\n"
            "endif()\n\n"
            "add_executable(%s_test %s)\n"
            "target_include_directories(%s_test PRIVATE ${CMAKE_BINARY_DIR})\n"
            "target_link_libraries(%s_test PRIVATE %s)\n\n"
            "enable_testing()\n"
            "add_test(NAME %s_test COMMAND %s_test)\n",
            basename, test_file, basename, basename, basename, basename, basename);
  }

  fclose(f);
  printf("Generated CMakeLists.txt\n");
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
          "CFLAGS ?= -Wall -Wextra -Wpedantic -std=c90 -g\n"
          "TARGET = lib%s.a\n"
          "OBJS = %s.o\n"
          "DEPS_DIR = deps\n"
          "GREATEST_H = $(DEPS_DIR)/greatest.h\n"
          "TEST_FILE = %s\n\n",
          basename,
          basename,
          (test_file && test_file[0] != '\0') ? test_file : "");

  fprintf(f,
          ".PHONY: all clean test deps\n\n"
          "all: $(TARGET)\n\n"
          "$(TARGET): $(OBJS)\n"
          "\tar rcs $@ $^\n\n"
          "%%.o: %%.c %%.h\n"
          "\t$(CC) $(CFLAGS) -c $< -o $@\n\n");

  /* rule to download greatest.h if needed */
  fprintf(f,
          "deps:\n"
          "\tmkdir -p $(DEPS_DIR)\n"
          "\t@if [ ! -f $(GREATEST_H) ]; then \\\n"
          "\t  echo Downloading greatest.h...; \\\n"
          "\t  if command -v curl > /dev/null; then \\\n"
          "\t    curl -L -o $(GREATEST_H) https://raw.githubusercontent.com/silentbicycle/greatest/master/greatest.h; \\\n"
          "\t  elif command -v wget > /dev/null; then \\\n"
          "\t    wget -O $(GREATEST_H) https://raw.githubusercontent.com/silentbicycle/greatest/master/greatest.h; \\\n"
          "\t  else \\\n"
          "\t    echo ERROR: Neither curl nor wget found to download greatest.h; exit 1; \\\n"
          "\t  fi; \\\n"
          "\tfi\n\n");

  fprintf(f,
          "test: deps $(TARGET)\n");

  if (test_file && test_file[0] != '\0') {
    fprintf(f,
            "\t$(CC) $(CFLAGS) -I$(DEPS_DIR) -o test_runner $(TEST_FILE) $(TARGET)\n"
            "\t./test_runner\n");
  } else {
    fprintf(f, "\t@echo \"No test file provided\"\n");
  }

  fprintf(f,
          "\nclean:\n"
          "\trm -f $(OBJS) $(TARGET) test_runner\n");

  fclose(f);
  printf("Generated Makefile\n");
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

  fprintf(f,
          "project('%s', 'c', version : '0.1')\n\n"
          "# Strict C90 flags by default\n"
          "cc = meson.get_compiler('c')\n"
          "strict_flags = []\n"
          "if cc.get_id() == 'msvc'\n"
          "  strict_flags = ['/W4', '/Za']\n"
          "else\n"
          "  strict_flags = ['-std=c90', '-Wall', '-Wextra', '-pedantic']\n"
          "endif\n"
          "add_project_arguments(strict_flags, language: 'c')\n\n"
          "lib = static_library(\n"
          "  '%s',\n"
          "  '%s.c',\n"
          "  include_directories: include_directories('.'),\n"
          ")\n\n",
          basename, basename, basename);

  if (test_file && test_file[0] != '\0') {
    fprintf(f,
            "# Download greatest.h for tests\n"
            "greatest_h = run_command(\n"
            "  'curl', \n"
            "  '-fLO', \n"
            "  '-O', \n"
            "  'https://raw.githubusercontent.com/silentbicycle/greatest/master/greatest.h',\n"
            "  check : false\n"
            ")\n\n"
            "test_exe = executable(\n"
            "  '%s_test',\n"
            "  '%s',\n"
            "  link_with: lib,\n"
            "  include_directories: include_directories('.'),\n"
            "  install: false,\n"
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
  errno_t err = fopen_s(&f, "BUILD", "r");
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
            "    copts = [\"-std=c90\", \"-Wall\", \"-Wextra\", \"-pedantic\"],\n"
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

      fprintf(f,
        "\n# NOTE: For greatest.h dependency,\n"
        "# consider adding an http_archive rule in your WORKSPACE file:\n"
        "#\n"
        "# http_archive(\n"
        "#     name = \"greatest\",\n"
        "#     urls = [\"https://github.com/silentbicycle/greatest/archive/master.zip\"],\n"
        "#     strip_prefix = \"greatest-master\",\n"
        "# )\n"
        "#\n"
        "# and then add appropriate deps to test target.\n");
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
