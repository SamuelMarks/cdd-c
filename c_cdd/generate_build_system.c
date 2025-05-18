#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "generate_build_system.h"

#include "fs.h"

#include <sys/stat.h>

static int write_cmake(const char *const output_directory,
                       const char *const basename) {
  FILE *rootCmakeLists, *srcCmakeLists;
  char *rootCmakeListsPath, *srcCmakeListsPath;
  int rc = EXIT_SUCCESS;
  asprintf(&rootCmakeListsPath, "%s" PATH_SEP "%s", output_directory, "CMakeLists.txt");
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  {
    errno_t err = fopen_s(&rootCmakeLists, p, "w");
    if (err != 0 || rootCmakeLists == NULL) {
      fprintf(stderr, "Failed to open %s for writing", p);
      free(p);
      return EXIT_FAILURE;
    }
  }
#else
  rootCmakeLists = fopen(rootCmakeListsPath, "w");
  if (!rootCmakeLists) {
    fprintf(stderr, "Failed to open %s for writing", rootCmakeListsPath);
    free(rootCmakeListsPath);
    return EXIT_FAILURE;
  }
#endif

  fprintf(
      rootCmakeLists,
      "cmake_minimum_required(VERSION 3.10)\n"
      "project(%s LANGUAGES C)\n\n"
      "# Enable strict C90 mode and strict warnings\n"
      "set(CMAKE_C_STANDARD 90)\n"
      "set(CMAKE_C_STANDARD_REQUIRED ON)\n"
      "if(MSVC)\n"
      "  add_compile_options(/W4 /Za)\n"
      "else()\n"
      "  add_compile_options(-Wall -Wextra -pedantic)\n"
      "endif()\n\n"
      "add_subdirectory(\"src\")\n", basename
  );

cleanup_root:
  fclose(rootCmakeLists);
  printf("Generated %s\n", rootCmakeListsPath);
  free(rootCmakeListsPath);

  {
    char *srcTestsPath;
    asprintf(&srcTestsPath, "%s" PATH_SEP "%s" PATH_SEP "%s", output_directory, "src", "test");
    rc = makedirs(srcTestsPath);
    if (rc != 0) {
      fprintf(stderr, "Failed to create src/test directory: %s\n", srcTestsPath);
      free(srcTestsPath);
      return rc;
    }
    free(srcTestsPath);
  }
  {
    asprintf(&srcCmakeListsPath, "%s" PATH_SEP "%s" PATH_SEP "%s", output_directory, "src", "CMakeLists.txt");
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    {
      errno_t err = fopen_s(&srcCmakeLists, srcCmakeListsPath, "w");
      if (err != 0 || srcCmakeListsPath == NULL) {
        fprintf(stderr, "Failed to open %s for writing", srcCmakeListsPath);
        free(srcCmakeListsPath);
        return EXIT_FAILURE;
      }
    }
#else
    srcCmakeLists = fopen(srcCmakeListsPath, "w");
    if (!srcCmakeLists) {
      fprintf(stderr, "Failed to open %s for writing", srcCmakeListsPath);
      free(srcCmakeListsPath);
      return EXIT_FAILURE;
    }
#endif

    fprintf(srcCmakeLists,
        "set(LIBRARY_NAME \"${PROJECT_NAME}\")\n\n"
        "set(Header_Files \"%s.h\" \"lib_export.h\")\n"
        "source_group(\"Header Files\" FILES \"${Header_Files}\")\n\n"
        "set(Source_Files \"%s.c\")\n"
        "source_group(\"Source Files\" FILES \"${Source_Files}\")\n\n",
        basename, basename
        );
    fputs(
        "add_library(\"${LIBRARY_NAME}\" SHARED \"${Header_Files}\" \"${Source_Files}\")\n\n"
        "set_target_properties(\"${LIBRARY_NAME}\" PROPERTIES LINKER_LANGUAGE C)\n\n"
        "include(GNUInstallDirs)\n"
        "target_include_directories(\n"
        "  \"${LIBRARY_NAME}\"\n"
        "  PUBLIC\n"
        "  \"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>\"\n"
        "  \"$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>\"\n"
        "  \"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>\"\n"
        ")\n\n", srcCmakeLists);
    fputs(
    "find_package(parson CONFIG REQUIRED)\n"
    "target_link_libraries(\n"
    "  \"${LIBRARY_NAME}\"\n"
    "  PRIVATE\n"
    "  \"parson::parson\"\n"
    ")\n", srcCmakeLists
    );
    fprintf(
        srcCmakeLists,
        "install(FILES       ${Header_Files}\n"
        "        DESTINATION \"${CMAKE_INSTALL_INCLUDEDIR}\")\n\n"
        "if (EXISTS \"${PROJECT_SOURCE_DIR}/test_%s.h\")\n"
        "  include(CTest)\n"
        "  if (BUILD_TESTING)\n"
        "    add_subdirectory(\"test\")\n"
        "  endif (BUILD_TESTING)\n"
        "endif (EXISTS \"${PROJECT_SOURCE_DIR}/test_%s.h\")\n",
        basename, basename);

    {
      char *testSrcCmakeListsPath;
      asprintf(&testSrcCmakeListsPath, "%s" PATH_SEP "%s" PATH_SEP "%s" PATH_SEP "%s", output_directory, "src", "test", "CMakeLists.txt");
      rc = cp(
        testSrcCmakeListsPath,
        "c_cdd" PATH_SEP "templates" PATH_SEP "CMakeLists.txt_for_tests.cmake"
      );
      free(testSrcCmakeListsPath);
    }
    if (rc == 0) {
      char *p0;
      asprintf(&p0, "%s" PATH_SEP "%s" PATH_SEP "%s", output_directory, "src", "lib_export.h");
      rc = cp(p0, "c_cdd" PATH_SEP "templates" PATH_SEP "lib_export.h");
      free(p0);
    }
    if (rc == 0) {
      char *p1;
      asprintf(&p1, "%s" PATH_SEP "%s" PATH_SEP "%s", output_directory, "src", "vcpkg.json");
      rc = cp(p1, "c_cdd" PATH_SEP "templates" PATH_SEP "vcpkg.json");
      free(p1);
    }
  }

cleanup_src:
  fclose(srcCmakeLists);
  printf("Generated %s\n", srcCmakeListsPath);
  if (rc == 0) {
    char *p2;
    asprintf(&p2, "%s" PATH_SEP "%s", output_directory, "src");
    printf("Copied vcpkg.json & lib_export.h to %s\n", srcCmakeListsPath);
    free(p2);
  }
  free(srcCmakeListsPath);
  return rc;
}

static int write_makefile(const char *const output_directory,
                          const char *const basename,
                          const char *const test_file) {
  FILE *f;
  char *p;
  asprintf(&p, "%s" PATH_SEP "%s", output_directory, "Makefile");
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  {
    errno_t err = fopen_s(&f, p, "w");
    if (err != 0 || f == NULL) {
      fprintf(stderr, "Failed to open %s for writing\n", p);
      free(p);
      return EXIT_FAILURE;
    }
  }
#else
  f = fopen(p, "w");
  if (!f) {
    fprintf(stderr, "Failed to open %s for writing\n", p);
    free(p);
    return EXIT_FAILURE;
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
          basename, basename,
          (test_file && test_file[0] != '\0') ? test_file : "");

  fprintf(f, ".PHONY: all clean test deps\n\n"
             "all: $(TARGET)\n\n"
             "$(TARGET): $(OBJS)\n"
             "\tar rcs $@ $^\n\n"
             "%%.o: %%.c %%.h\n"
             "\t$(CC) $(CFLAGS) -c $< -o $@\n\n");

  /* rule to download greatest.h if needed */
  fprintf(f, "deps:\n"
             "\tmkdir -p $(DEPS_DIR)\n"
             "\t@if [ ! -f $(GREATEST_H) ]; then \\\n"
             "\t  echo Downloading greatest.h...; \\\n"
             "\t  if command -v curl > /dev/null; then \\\n"
             "\t    curl -L -o $(GREATEST_H) "
             "https://raw.githubusercontent.com/silentbicycle/greatest/master/"
             "greatest.h; \\\n"
             "\t  elif command -v wget > /dev/null; then \\\n"
             "\t    wget -O $(GREATEST_H) "
             "https://raw.githubusercontent.com/silentbicycle/greatest/master/"
             "greatest.h; \\\n"
             "\t  else \\\n"
             "\t    echo ERROR: Neither curl nor wget found to download "
             "greatest.h; exit 1; \\\n"
             "\t  fi; \\\n"
             "\tfi\n\n");

  fprintf(f, "test: deps $(TARGET)\n");

  if (test_file && test_file[0] != '\0') {
    fprintf(f, "\t$(CC) $(CFLAGS) -I$(DEPS_DIR) -o test_runner $(TEST_FILE) "
               "$(TARGET)\n"
               "\t./test_runner\n");
  } else {
    fprintf(f, "\t@echo \"No test file provided\"\n");
  }

  fprintf(f, "\nclean:\n"
             "\trm -f $(OBJS) $(TARGET) test_runner\n");

  fclose(f);
  printf("Generated %s\n", p);
  free(p);
  return EXIT_SUCCESS;
}

static int write_meson(const char *const output_directory,
                       const char *const basename,
                       const char *const test_file) {
  FILE *f;
  char *p;
  asprintf(&p, "%s" PATH_SEP "%s", output_directory, "meson.build");
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  {
    errno_t err = fopen_s(&f, p, "w");
    if (err != 0 || f == NULL) {
      fprintf(stderr, "Failed to open %s for writing\n", p);
      free(p);
      return EXIT_FAILURE;
    }
  }
#else
  f = fopen(p, "w");
  if (!f) {
    fprintf(stderr, "Failed to open %s for writing\n", p);
    free(p);
    return EXIT_FAILURE;
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
            "  "
            "'https://raw.githubusercontent.com/silentbicycle/greatest/master/"
            "greatest.h',\n"
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
  printf("Generated %s\n", p);
  free(p);
  return EXIT_SUCCESS;
}

static int write_bazel(const char *const output_directory,
                       const char *const basename,
                       const char *const test_file) {
  FILE *f;
  char *p;
  asprintf(&p, "%s" PATH_SEP "%s", output_directory, "BUILD");
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  {
    errno_t err = fopen_s(&f, p, "w");
    if (err != 0 || f == NULL) {
      fprintf(stderr, "Failed to open %s for writing\n", p);
      free(p);
      return EXIT_FAILURE;
    }
  }
#else
  f = fopen(p, "w");
  if (!f) {
    fprintf(stderr, "Failed to open %s for writing\n", p);
    free(p);
    return EXIT_FAILURE;
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

    fprintf(
        f,
        "\n# NOTE: For greatest.h dependency,\n"
        "# consider adding an http_archive rule in your WORKSPACE file:\n"
        "#\n"
        "# http_archive(\n"
        "#     name = \"greatest\",\n"
        "#     urls = "
        "[\"https://github.com/silentbicycle/greatest/archive/master.zip\"],\n"
        "#     strip_prefix = \"greatest-master\",\n"
        "# )\n"
        "#\n"
        "# and then add appropriate deps to test target.\n");
  }

  fclose(f);
  printf("Generated %s\n", p);
  free(p);
  return EXIT_SUCCESS;
}

int generate_build_system_main(int argc, char **argv) {
  if (argc < 3 || argc > 4) {
    fputs("Usage: generate_build <build_system> <output_directory> <basename> "
          "[test_file]\n"
          "build_system: cmake | make | meson | bazel\n"
          "basename: base name for .c and .h files\n"
          "test_file: optional .c test file",
          stderr);
    return EXIT_FAILURE;
  }

  {
    const char *const build_system = argv[0];
    const char *const output_directory = argv[1];
    const char *const basename = argv[2];
    const char *const test_file = (argc == 4) ? argv[3] : NULL;

    if (access(output_directory, F_OK) != 0) {
      const int rc = makedirs(output_directory);
      if (rc != 0) {
        fprintf(stderr, "Failed to create output directory: %s\n",
                output_directory);
        return rc;
      }
    }

    if (strcmp(build_system, "cmake") == 0) {
      return write_cmake(output_directory, basename);
    } else if (strcmp(build_system, "make") == 0) {
      return write_makefile(output_directory, basename, test_file);
    } else if (strcmp(build_system, "meson") == 0) {
      return write_meson(output_directory, basename, test_file);
    } else if (strcmp(build_system, "bazel") == 0) {
      return write_bazel(output_directory, basename, test_file);
    } else {
      fprintf(stderr, "Unsupported build system: %s\n", build_system);
      return EXIT_FAILURE;
    }
  }
}
