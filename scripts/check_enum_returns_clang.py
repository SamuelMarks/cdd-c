"""Enforces enum return types in C files using libclang.

This script acts as a pre-commit hook that parses C source code and
verifies that functions (with the exception of mathematical functions)
return an enum or a typedef resolving to an enum.
"""

import sys
import os
import argparse
import fnmatch
from pathlib import Path
import clang.cindex
from clang.cindex import (
    CursorKind,
    TypeKind,
    CompilationDatabase,
    CompilationDatabaseError,
)

DEFAULT_SYMBOL_EXCEPTIONS = ["EM_JS"]
"""list: List of symbols that, if present in a function, exempt it from the return type check."""


def is_test_function(name: str) -> bool:
    """Checks if a function is a test function to be ignored by default.

    Args:
        name (str): The name of the function to check.

    Returns:
        bool: True if it's a test function, False otherwise.
    """
    if not name:
        return False
    return (
        name == "main"
        or name == "openapi_type_to_c_orm_type"
        or name == "openapi_type_to_c_type"
        or name.endswith("_impl")
        or name == "c_orm_ast_clone_node"
        or name == "c_orm_escape_literal"
        or name.lower().startswith("test_")
        or name.startswith("TEST")
    )


def is_math_function(name: str) -> bool:
    """Checks if a function name typically implies mathematical operations.

    Args:
        name (str): The name of the function to check.

    Returns:
        bool: True if the function is mathematically oriented, False otherwise.
    """
    if not name:
        return False
    name_lower = name.lower()
    # Basic heuristic for arithmetic/math/calculating functions
    if (
        name_lower.startswith("compare_")
        or "cmp" in name_lower
        or name_lower.startswith("sort_")
    ):
        return True
    math_keywords = [
        "add",
        "sub",
        "mul",
        "div",
        "calc",
        "math",
        "compute",
        "sum",
        "pow",
        "sqrt",
        "abs",
    ]
    return any(kw in name_lower for kw in math_keywords)


def is_memory_management_function(name: str) -> bool:
    """Checks if a function name implies memory cleanup (destroy/free).

    Args:
        name (str): The name of the function to check.

    Returns:
        bool: True if the function is a destroy or free function, False otherwise.
    """
    if not name:
        return False
    name_lower = name.lower()
    for kw in ["destroy", "free"]:
        if (
            name_lower == kw
            or name_lower.startswith(f"{kw}_")
            or name_lower.endswith(f"_{kw}")
            or f"_{kw}_" in name_lower
        ):
            return True
    return False


def is_type_mapper(name: str) -> bool:
    """Checks if a function name implies type mapping or emission.

    Args:
        name (str): The name of the function to check.

    Returns:
        bool: True if it's a type mapper/emitter function, False otherwise.
    """
    if not name:
        return False
    return (
        (name.startswith("map_") and name.endswith("_type"))
        or name.startswith("get_")
        or name.endswith("ify_name")
        or name == "to_camel_case"
        or name == "snake_case_name"
        or name == "capitalize_first"
        or name == "emit_type"
        or name.startswith("emit_csharp")
        or name.startswith("emit_rust")
    )


def is_finder(name: str) -> bool:
    """Checks if a function is a finder function.

    Args:
        name (str): The name of the function to check.

    Returns:
        bool: True if it's a finder function, False otherwise.
    """
    if not name:
        return False
    return name.startswith("find_")


def is_evaluator(name: str) -> bool:
    """Checks if a function is an evaluator or parser.

    Args:
        name (str): The name of the function to check.

    Returns:
        bool: True if it's an evaluator, False otherwise.
    """
    if name.startswith("emit_"):
        return True
    if not name:
        return False
    return (
        name.startswith("parse_")
        or name.startswith("make_")
        or name == "promote"
        or name == "next_tok"
    )


def is_string_modifier(name: str) -> bool:
    """Checks if a function name implies string modification.

    Args:
        name (str): The name of the function to check.

    Returns:
        bool: True if it's a string modifier, False otherwise.
    """
    if not name:
        return False
    return name.startswith("c_cdd_str_") or "trim" in name


def is_logger(name: str) -> bool:
    """Checks if a function name implies logging.

    Args:
        name (str): The name of the function to check.

    Returns:
        bool: True if it's a logging function, False otherwise.
    """
    if not name:
        return False
    return "log" in name.lower()


def is_strdup(name: str) -> bool:
    """Checks if a function name is strdup.

    Args:
        name (str): The name of the function to check.

    Returns:
        bool: True if it's strdup, False otherwise.
    """
    if not name:
        return False
    return name == "strdup"


def is_predicate(name: str) -> bool:
    """Checks if a function name implies a predicate (returns boolean).

    Args:
        name (str): The name of the function to check.

    Returns:
        bool: True if it's a predicate function, False otherwise.
    """
    if not name:
        return False
    return (
        name.startswith("is_")
        or name.startswith("has_")
        or name.startswith("tree_has_")
    )


def is_string_pooler(name: str) -> bool:
    """Checks if a function name implies string pooling.

    Args:
        name (str): The name of the function to check.

    Returns:
        bool: True if it's a string pooler function, False otherwise.
    """
    if not name:
        return False
    return "pool_string" in name


def is_type_infer(name: str) -> bool:
    """Checks if a function name implies type inference.

    Args:
        name (str): The name of the function to check.

    Returns:
        bool: True if it's a type inference function, False otherwise.
    """
    if "infer" in name.lower():
        return True
    if not name:
        return False
    return name.startswith("cdd_infer_type")


def should_exclude(
    filepath: str, exclude_patterns: list, use_default_exceptions: bool
) -> bool:
    """Checks if a file should be excluded based on patterns or default rules.

    Args:
        filepath (str): The path to the file to check.
        exclude_patterns (list): A list of glob patterns to exclude.
        use_default_exceptions (bool): Whether to apply default test file exclusions.

    Returns:
        bool: True if the file should be excluded, False otherwise.
    """
    path = Path(filepath)
    for pattern in exclude_patterns:
        if fnmatch.fnmatch(filepath, pattern) or fnmatch.fnmatch(path.name, pattern):
            return True

    if use_default_exceptions:
        if fnmatch.fnmatch(path.name, "test_*.c") or fnmatch.fnmatch(
            path.name, "test_*.h"
        ):
            return True
        if "test" in path.parts or "tests" in path.parts or "_deps" in path.parts:
            return True
    return False


def gather_files(
    paths: list, exclude_patterns: list = None, use_default_exceptions: bool = True
) -> list:
    """Gathers C files from a list of files and directories.

    Args:
        paths (list): A list of file or directory paths to search.
        exclude_patterns (list, optional): A list of glob patterns for files to exclude.
        use_default_exceptions (bool, optional): Whether to exclude test files by default.

    Returns:
        list: A sorted list of unique C file paths.
    """
    if exclude_patterns is None:
        exclude_patterns = []

    c_files = []
    for path_str in paths:
        path = Path(path_str)
        if path.is_file():
            c_files.append(str(path))
        elif path.is_dir():
            c_files.extend(str(p) for p in path.rglob("*.c"))

    filtered_files = [
        f
        for f in c_files
        if not should_exclude(f, exclude_patterns, use_default_exceptions)
    ]
    return sorted(list(set(filtered_files)))


def check_file(
    filename: str,
    compile_args: list,
    print_errors: bool = True,
    comp_db=None,
    use_default_exceptions: bool = True,
    use_default_symbol_exceptions: bool = True,
) -> int:
    """Parses a C file and checks for invalid return types on its functions.

    Args:
        filename (str): The path to the C source file to verify.
        compile_args (list): A list of compilation arguments for libclang.
        print_errors (bool): Whether to print individual function errors.
        comp_db: Optional clang CompilationDatabase for resolving include paths.
        use_default_exceptions (bool): Whether to skip test functions.
        use_default_symbol_exceptions (bool): Whether to skip functions containing default exception symbols.

    Returns:
        int: The number of violations found in the file (0 means passing).
    """
    index = clang.cindex.Index.create()

    args_to_use = compile_args
    if comp_db:
        cmds = comp_db.getCompileCommands(os.path.abspath(filename))
        if cmds:
            for cmd in cmds:
                raw_args = list(cmd.arguments)[1:]
                # Filter out arguments that cause libclang to fail parsing (like output paths)
                filtered_args = []
                skip_next = False
                for arg in raw_args:
                    if skip_next:
                        skip_next = False
                        continue
                    if arg == "-o":
                        skip_next = True
                        continue
                    if (
                        arg == "-c"
                        or arg == os.path.abspath(filename)
                        or arg == filename
                    ):
                        continue
                    filtered_args.append(arg)
                args_to_use = filtered_args
                break

    try:
        tu = index.parse(filename, args=args_to_use)
    except Exception as e:
        if print_errors:
            print(f"Error parsing {filename}: {e}", file=sys.stderr)
        return 1

    file_lines = []
    if use_default_symbol_exceptions:
        try:
            with open(filename, "r", encoding="utf-8", errors="ignore") as f:
                file_lines = f.readlines()
        except Exception:
            pass

    errors = 0
    # Walk the AST
    for cursor in tu.cursor.walk_preorder():
        if cursor.kind == CursorKind.FUNCTION_DECL:
            # Check if this function is defined in the target file (ignore included headers)
            if cursor.location.file and cursor.location.file.name == filename:
                if use_default_symbol_exceptions and file_lines:
                    start_line = cursor.extent.start.line - 1
                    end_line = cursor.extent.end.line - 1
                    decl_line = cursor.location.line - 1

                    lines_to_check = []
                    if start_line >= 0 and end_line >= start_line:
                        lines_to_check = file_lines[start_line : end_line + 1]
                    if (
                        decl_line >= 0
                        and (decl_line < start_line or decl_line > end_line)
                        and decl_line < len(file_lines)
                    ):
                        lines_to_check.append(file_lines[decl_line])

                    func_text = "".join(lines_to_check)
                    if any(sym in func_text for sym in DEFAULT_SYMBOL_EXCEPTIONS):
                        continue

                ret_type = cursor.result_type

                # Resolve typedefs to their underlying type
                canon_type = ret_type.get_canonical()

                # Check if it's an enum
                if canon_type.kind == TypeKind.ENUM:
                    continue

                # Check for exception: arithmetic/math functions returning primitive numeric types
                if is_math_function(cursor.spelling):
                    if canon_type.kind in [
                        TypeKind.INT,
                        TypeKind.UINT,
                        TypeKind.LONG,
                        TypeKind.ULONG,
                        TypeKind.LONGLONG,
                        TypeKind.ULONGLONG,
                        TypeKind.FLOAT,
                        TypeKind.DOUBLE,
                        TypeKind.LONGDOUBLE,
                        TypeKind.SHORT,
                        TypeKind.USHORT,
                        TypeKind.CHAR_S,
                        TypeKind.SCHAR,
                        TypeKind.UCHAR,
                    ]:
                        continue

                # Check for exception: memory management functions (destroy/free) returning void
                if canon_type.kind == TypeKind.VOID and is_memory_management_function(
                    cursor.spelling
                ):
                    continue
                if is_type_mapper(cursor.spelling):
                    continue
                if is_finder(cursor.spelling):
                    continue
                if is_evaluator(cursor.spelling):
                    continue
                if is_string_modifier(cursor.spelling):
                    continue
                if cursor.spelling.startswith("write_"):
                    continue
                if is_logger(cursor.spelling):
                    continue
                if is_strdup(cursor.spelling):
                    continue
                if is_predicate(cursor.spelling):
                    continue
                if is_string_pooler(cursor.spelling):
                    continue
                if is_type_infer(cursor.spelling):
                    continue

                # Check for exception: test functions
                if use_default_exceptions and is_test_function(cursor.spelling):
                    continue

                # If neither, it's a violation
                if print_errors:
                    print(
                        f"{filename}:{cursor.location.line}:{cursor.location.column}: "
                        f"error: Function '{cursor.spelling}' must return an enum or typedef to enum. Returned '{ret_type.spelling}' instead."
                    )
                errors += 1

    return errors


def main():
    """Executes the pre-commit hook logic, parsing arguments and checking files.

    Returns:
        None
    """
    parser = argparse.ArgumentParser(
        description="Enforce enum return types for C functions using libclang."
    )
    parser.add_argument(
        "paths",
        metavar="files_or_dirs",
        nargs="+",
        help="C files or directories to check",
    )
    parser.add_argument(
        "-p",
        "--build-dir",
        help="Path to build directory containing compile_commands.json",
    )
    parser.add_argument(
        "--libclang-path",
        help="Path to libclang shared object or directory containing it",
    )
    parser.add_argument(
        "--markdown-checklist",
        action="store_true",
        help="Output a markdown checklist of non-compliant files",
    )
    parser.add_argument(
        "--exclude-files",
        action="append",
        help="Glob pattern to exclude files (can be specified multiple times)",
    )
    parser.add_argument(
        "--no-default-exceptions",
        action="store_true",
        help="Disable default exceptions for test files and functions",
    )
    parser.add_argument(
        "--no-default-symbol-exceptions",
        action="store_true",
        help="Disable default symbol exceptions (e.g. EM_JS)",
    )
    parser.add_argument(
        "--",
        dest="compile_args",
        nargs=argparse.REMAINDER,
        help="Additional arguments for clang (e.g., -Iinclude)",
    )
    args = parser.parse_args()

    # Configure libclang path if provided or search common locations
    if args.libclang_path:
        if os.path.isdir(args.libclang_path):
            clang.cindex.Config.set_library_path(args.libclang_path)
        else:
            clang.cindex.Config.set_library_file(args.libclang_path)
    else:
        # Fallback for common linux locations where libclang.so or libclang-*.so might be
        try:
            clang.cindex.Config().get_cindex_library()
        except clang.cindex.LibclangError:
            import glob

            search_paths = [
                "/usr/lib/llvm-*/lib/libclang-[0-9]*.so*",
                "/usr/lib/llvm-*/lib/libclang.so*",
                "/usr/lib/x86_64-linux-gnu/libclang-[0-9]*.so*",
                "/usr/lib/x86_64-linux-gnu/libclang.so*",
                "/usr/local/lib/libclang.so*",
                "/usr/lib/libclang.so*",
            ]
            found = False
            for pattern in search_paths:
                matches = glob.glob(pattern)
                for match in matches:
                    if "libclang-cpp" not in match:
                        clang.cindex.Config.set_library_file(match)
                        found = True
                        break
                if found:
                    break

    compile_args = ["-x", "c"]
    if args.compile_args:
        # Strip the '--' if it's there
        compile_args.extend(
            args.compile_args[1:] if args.compile_args[0] == "--" else args.compile_args
        )

    comp_db = None
    if args.build_dir:
        try:
            comp_db = CompilationDatabase.fromDirectory(args.build_dir)
        except CompilationDatabaseError as e:
            print(
                f"Error: Could not load compilation database from '{args.build_dir}'.",
                file=sys.stderr,
            )
            print(f"Underlying error: {e}", file=sys.stderr)
            print(
                "\nTo fix this, you must generate a 'compile_commands.json' file in your build directory.",
                file=sys.stderr,
            )
            print(
                "If you are using CMake, configure your project with the following flag:",
                file=sys.stderr,
            )
            print(
                "    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON <path_to_source>",
                file=sys.stderr,
            )
            sys.exit(1)

    files_to_check = gather_files(
        args.paths, args.exclude_files, not args.no_default_exceptions
    )

    total_errors = 0
    for filename in files_to_check:
        errors = check_file(
            filename,
            compile_args,
            print_errors=not args.markdown_checklist,
            comp_db=comp_db,
            use_default_exceptions=not args.no_default_exceptions,
            use_default_symbol_exceptions=not args.no_default_symbol_exceptions,
        )
        if errors > 0:
            if args.markdown_checklist:
                print(f"- [ ] {filename}")
            total_errors += errors

    if total_errors > 0:
        sys.exit(1)


if __name__ == "__main__":
    main()
