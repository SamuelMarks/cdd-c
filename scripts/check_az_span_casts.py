#!/usr/bin/env python3
"""Pre-commit hook to forbid C-style casts and wrappers in az_span_create* calls.

Ensures that `az_span_create`, `az_span_create_from_str`,
`az_span_create_from_str_of_size`, and `az_span_create_from_const_u8` are called
directly with properly typed arguments without `(uint8_t *)(size_t)`,
`(char *)(size_t)`, `(uint8_t *)`, or `cdd_test_mut_str` workarounds.
"""

import os
import re
import sys
from typing import Iterable, List, Tuple

SPAN_CALL_RE = re.compile(
    r"\b(az_span_create(?:_from_str(?:_of_size)?|_from_const_u8)?)\s*\("
)

FORBIDDEN_ARG_PATTERNS = [
    (
        re.compile(r"\(\s*(?:const\s+)?(?:uint8_t|char|void)\s*\*\s*\)\s*\(\s*size_t\s*\)"),
        "Forbidden `(ptr *)(size_t)` cast in az_span constructor; use "
        "`az_span_create_from_str`, `az_span_create_from_str_of_size`, or "
        "`az_span_create_from_const_u8` without casting.",
    ),
    (
        re.compile(r"^\s*\(\s*(?:const\s+)?(?:uint8_t|char|void)\s*\*\s*\)"),
        "Forbidden pointer cast in az_span constructor; use "
        "`az_span_create_from_str`, `az_span_create_from_str_of_size`, or "
        "`az_span_create_from_const_u8` matching the source pointer type.",
    ),
    (
        re.compile(r"\bcdd_test_mut_str\s*\("),
        "Unnecessary `cdd_test_mut_str` wrapper inside az_span constructor; "
        "pass the `const char *` or string literal directly to "
        "`az_span_create_from_str`.",
    ),
    (
        re.compile(r"\bstrlen\s*\("),
        "Avoid `strlen(...)` inside `az_span_create*`; use "
        "`az_span_create_from_str(str)` instead.",
    ),
]


def iter_source_files(paths: Iterable[str]) -> Iterable[str]:
    """Yields `.c` and `.h` files from the provided file or directory paths.

    Args:
        paths: File or directory paths to scan.

    Yields:
        Paths to `.c` and `.h` source files.
    """
    for path in paths:
        if os.path.isfile(path):
            if path.endswith((".c", ".h")):
                yield path
        elif os.path.isdir(path):
            for root, _, files in os.walk(path):
                for fname in sorted(files):
                    if fname.endswith((".c", ".h")):
                        yield os.path.join(root, fname)


def extract_call_args(text: str, open_paren_idx: int) -> str:
    """Extracts the balanced parenthesized argument substring starting at `open_paren_idx`.

    Args:
        text: Full file content.
        open_paren_idx: Index of the opening `(` of the function call.

    Returns:
        The substring inside the outer parentheses of the call.
    """
    depth = 0
    in_str = False
    in_chr = False
    escape = False
    i = open_paren_idx
    n = len(text)
    while i < n:
        ch = text[i]
        if escape:
            escape = False
        elif ch == chr(92):
            escape = True
        elif in_str:
            if ch == '"':
                in_str = False
        elif in_chr:
            if ch == "'":
                in_chr = False
        else:
            if ch == '"':
                in_str = True
            elif ch == "'":
                in_chr = True
            elif ch == "(":
                depth += 1
            elif ch == ")":
                depth -= 1
                if depth == 0:
                    return text[open_paren_idx + 1 : i]
        i += 1
    return text[open_paren_idx + 1 :]


def check_file(filepath: str) -> List[Tuple[int, str]]:
    """Scans a single C/header file for forbidden casts in `az_span_create*` calls.

    Args:
        filepath: Path to the file to inspect.

    Returns:
        List of `(line_number, message)` tuples for each violation found.
    """
    violations: List[Tuple[int, str]] = []
    with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
        content = f.read()

    for match in SPAN_CALL_RE.finditer(content):
        open_paren_idx = match.end() - 1
        args_str = extract_call_args(content, open_paren_idx)
        line_num = content.count(chr(10), 0, match.start()) + 1
        for pattern, msg in FORBIDDEN_ARG_PATTERNS:
            if pattern.search(args_str):
                violations.append((line_num, msg))
                break

    return violations


def main(argv: List[str] = None) -> int:
    """CLI entry point for checking `az_span_create*` cast smells.

    Args:
        argv: Optional list of command-line arguments.

    Returns:
        0 if no violations are found, 1 otherwise.
    """
    args = argv if argv is not None else sys.argv[1:]
    target_paths = args if args else ["src", "include"]

    total_violations = 0
    for filepath in iter_source_files(target_paths):
        for line_num, msg in check_file(filepath):
            print(f"{filepath}:{line_num}: {msg}", file=sys.stderr)
            total_violations += 1

    if total_violations > 0:
        print(
            f"Found {total_violations} forbidden az_span_create* cast/wrapper violation(s).",
            file=sys.stderr,
        )
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
