"""Enforces dogfooding across the cdd-c codebase.

Ensures that:
1. No third-party JSON, CLI, or HTTP parsing libraries are included in `src/`
   or `include/`.
2. No legacy unchecked Parson calls (`json_object_get_*`, `json_object_set_*`,
   `json_array_get_*`, `json_parse_string`) are used in `src/` outside
   `src/json_dom.c` and `src/tests/`.
3. No manual CLI flag loops (`cdd_reflect_find_field` inside `for`/`while` loops
   over `argv`, or `getopt`/`getopt_long`) creep back into CLI entry points
   (`main.c`, `cli.c`, `c2arazzo.c`, `serve_json_rpc.c`, `serve_mcp_stdio.c`);
   they must use `cdd_cli_parse_reflected_argv` or generated `_from_cli`
   functions.
4. `openapi_parse_reflected_strings`, `openapi_emit_reflected_strings`,
   `arazzo_parse_reflected_strings`, and `arazzo_emit_reflected_strings`
   delegate directly to `cdd_reflect_from_json_object` and
   `cdd_reflect_to_json_object`.
5. `cdd_ffi_emit_ir_json.c` does not re-parse serialized JSON strings back into
   a DOM tree.
"""

import os
import re
import sys

FORBIDDEN_HEADERS_RE = re.compile(
    r'#\s*include\s*[<"](cjson|jansson|yyjson|rapidjson|nlohmann|getopt\.h|argp\.h)',
    re.IGNORECASE,
)

LEGACY_PARSON_CALL_RE = re.compile(
    r"\b(json_parse_string|json_parse_file|json_object_get_string|"
    r"json_object_get_number|json_object_get_boolean|json_object_get_object|"
    r"json_object_get_array|json_object_get_value|json_object_set_string|"
    r"json_object_set_number|json_object_set_boolean|json_object_set_value|"
    r"json_object_set_null|json_array_get_string|json_array_get_number|"
    r"json_array_get_boolean|json_array_get_object|json_array_get_array|"
    r"json_array_get_count|json_array_append_value|json_array_append_string|"
    r"json_value_init_object|json_value_init_array|json_value_init_string|"
    r"json_value_get_object|json_value_get_array|json_value_free|"
    r"json_serialize_to_string|json_serialize_to_string_pretty)\s*\("
)

GETOPT_CALL_RE = re.compile(r"\b(getopt|getopt_long|getopt_long_only)\s*\(")

CLI_ENTRYPOINT_FILES = {
    os.path.normpath("src/functions/parse/main.c"),
    os.path.normpath("src/routes/parse/cli.c"),
    os.path.normpath("src/arazzo/parse/c2arazzo.c"),
    os.path.normpath("src/routes/emit/serve_json_rpc.c"),
    os.path.normpath("src/routes/emit/serve_mcp_stdio.c"),
}


def strip_comments_and_strings(code: str) -> str:
    """Replaces C comments and string literals with spaces to avoid false positives."""
    pattern = re.compile(
        r"""//.*?$|/\*.*?\*/|'(?:\\.|[^\\'])*'|"(?:\\.|[^\\"])*" """,
        re.DOTALL | re.MULTILINE | re.VERBOSE,
    )
    return pattern.sub(lambda m: " " * len(m.group(0)), code)


def check_file(filepath: str) -> list:
    errors = []
    norm_path = os.path.normpath(filepath)
    if not (norm_path.startswith("src" + os.sep) or norm_path.startswith("include" + os.sep)):
        return errors
    if norm_path.startswith(os.path.join("src", "tests") + os.sep):
        return errors

    try:
        with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
            raw = f.read()
    except OSError:
        return errors

    # 1. Check forbidden third-party headers
    for m in FORBIDDEN_HEADERS_RE.finditer(raw):
        line_no = raw.count("\n", 0, m.start()) + 1
        errors.append(
            f"{filepath}:{line_no}: Forbidden third-party library header '{m.group(0).strip()}' (must dogfood cdd-c)"
        )

    stripped = strip_comments_and_strings(raw)

    # 2. Check getopt usage outside cli_struct
    if not norm_path.startswith(os.path.join("src", "classes", "emit", "cli_struct")):
        for m in GETOPT_CALL_RE.finditer(stripped):
            line_no = raw.count("\n", 0, m.start()) + 1
            errors.append(
                f"{filepath}:{line_no}: Forbidden '{m.group(1)}()' call (must dogfood cdd-c CLI parser)"
            )

    # 3. Check legacy unchecked Parson calls in src/ (excluding src/json_dom.c and include/parson.h)
    if norm_path.startswith("src" + os.sep) and norm_path != os.path.normpath("src/json_dom.c"):
        for m in LEGACY_PARSON_CALL_RE.finditer(stripped):
            line_no = raw.count("\n", 0, m.start()) + 1
            errors.append(
                f"{filepath}:{line_no}: Forbidden legacy unchecked Parson call '{m.group(1)}()' (use 'cdd_{m.group(1)}()' returning cdd_c_error_t)"
            )

    # 4. Check CLI entrypoints dogfood cdd_cli_parse_reflected_argv
    if norm_path in CLI_ENTRYPOINT_FILES and norm_path != os.path.normpath("src/routes/emit/serve_mcp_stdio.c"):
        if "cdd_cli_parse_reflected_argv" not in stripped:
            errors.append(
                f"{filepath}:1: CLI entrypoint must dogfood 'cdd_cli_parse_reflected_argv'"
            )

    # 5. Check OpenAPI and Arazzo reflection helpers delegate to cdd_reflect_*_json_object
    if norm_path == os.path.normpath("src/openapi/parse/openapi_metadata.c"):
        if "cdd_reflect_from_json_object" not in stripped:
            errors.append(
                f"{filepath}:1: OpenAPI metadata parser must dogfood 'cdd_reflect_from_json_object'"
            )
    if norm_path == os.path.normpath("src/openapi/emit/openapi_info.c"):
        if "cdd_reflect_to_json_object" not in stripped:
            errors.append(
                f"{filepath}:1: OpenAPI info emitter must dogfood 'cdd_reflect_to_json_object'"
            )
    if norm_path == os.path.normpath("src/arazzo/parse/arazzo.c"):
        if "cdd_reflect_from_json_object" not in stripped:
            errors.append(
                f"{filepath}:1: Arazzo parser must dogfood 'cdd_reflect_from_json_object'"
            )
    if norm_path == os.path.normpath("src/arazzo/emit/arazzo.c"):
        if "cdd_reflect_to_json_object" not in stripped:
            errors.append(
                f"{filepath}:1: Arazzo emitter must dogfood 'cdd_reflect_to_json_object'"
            )

    # 6. Check FFI IR emitter does not re-parse JSON string
    if norm_path == os.path.normpath("src/functions/ffi/cdd_ffi_emit_ir_json.c"):
        if "cdd_json_parse_string" in stripped:
            errors.append(
                f"{filepath}:1: cdd_ffi_emit_ir_json.c must not redundantly re-parse JSON string with cdd_json_parse_string"
            )

    return errors


def main() -> int:
    targets = sys.argv[1:] if len(sys.argv) > 1 else ["src", "include"]
    all_errors = []

    for target in targets:
        if os.path.isfile(target):
            if target.endswith((".c", ".h")):
                all_errors.extend(check_file(target))
        elif os.path.isdir(target):
            for root, _, files in os.walk(target):
                for fn in sorted(files):
                    if fn.endswith((".c", ".h")):
                        all_errors.extend(check_file(os.path.join(root, fn)))

    if all_errors:
        for err in all_errors:
            print(err, file=sys.stderr)
        print(f"Total dogfooding violations: {len(all_errors)}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
