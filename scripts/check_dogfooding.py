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
        scan_target = (
            raw
            if norm_path
            in {
                os.path.normpath("src/classes/emit/json.c"),
                os.path.normpath("src/routes/emit/cli_gen.c"),
                os.path.normpath("src/routes/emit/url_query.c"),
                os.path.normpath("src/routes/emit/url_serialization.c"),
            }
            else stripped
        )
        for m in LEGACY_PARSON_CALL_RE.finditer(scan_target):
            line_no = raw.count("\n", 0, m.start()) + 1
            errors.append(
                f"{filepath}:{line_no}: Forbidden legacy unchecked Parson call '{m.group(1)}()' (use 'cdd_{m.group(1)}()' returning cdd_c_error_t)"
            )

    # 3b. Check tautological error-swallowing returns (`return rc ? CDD_C_SUCCESS : CDD_C_SUCCESS;`)
    taut_re = re.compile(r"return\s+\w+\s*\?\s*CDD_C_SUCCESS\s*:\s*CDD_C_SUCCESS\s*;")
    for m in taut_re.finditer(stripped):
        line_no = raw.count("\n", 0, m.start()) + 1
        errors.append(
            f"{filepath}:{line_no}: Forbidden tautological error-swallowing return '{m.group(0)}' (must percolate error enum)"
        )

    # 4. Check CLI entrypoints dogfood cdd_cli_parse_reflected_argv
    if norm_path in CLI_ENTRYPOINT_FILES:
        if "cdd_cli_parse_reflected_argv" not in stripped:
            errors.append(
                f"{filepath}:1: CLI entrypoint must dogfood 'cdd_cli_parse_reflected_argv'"
            )
        if "cdd_reflect_find_field" in stripped:
            errors.append(
                f"{filepath}:1: CLI entrypoint must not manually call 'cdd_reflect_find_field' (use 'cdd_cli_parse_reflected_argv')"
            )

    # 5. Check OpenAPI and Arazzo reflection helpers delegate to cdd_reflect_*_json_object
    if norm_path == os.path.normpath("src/openapi/parse/openapi_metadata.c"):
        if (
            "cdd_reflect_from_json_object" not in stripped
            or "OpenAPI_SecurityScheme_type_info" not in stripped
        ):
            errors.append(
                f"{filepath}:1: OpenAPI metadata parser must dogfood 'cdd_reflect_from_json_object' and 'OpenAPI_SecurityScheme_type_info'"
            )
    if norm_path == os.path.normpath("src/openapi/emit/openapi_info.c"):
        if "cdd_reflect_to_json_object" not in stripped:
            errors.append(
                f"{filepath}:1: OpenAPI info emitter must dogfood 'cdd_reflect_to_json_object'"
            )
    if norm_path == os.path.normpath("src/arazzo/parse/arazzo.c"):
        if (
            "cdd_reflect_from_json_object" not in stripped
            or "Arazzo_RequestBody_type_info" not in stripped
            or "Arazzo_PayloadReplacement_type_info" not in stripped
        ):
            errors.append(
                f"{filepath}:1: Arazzo parser must dogfood 'cdd_reflect_from_json_object', 'Arazzo_RequestBody_type_info', and 'Arazzo_PayloadReplacement_type_info'"
            )
    if norm_path == os.path.normpath("src/arazzo/emit/arazzo.c"):
        if (
            "cdd_reflect_to_json_object" not in stripped
            or "Arazzo_RequestBody_type_info" not in stripped
            or "Arazzo_PayloadReplacement_type_info" not in stripped
        ):
            errors.append(
                f"{filepath}:1: Arazzo emitter must dogfood 'cdd_reflect_to_json_object', 'Arazzo_RequestBody_type_info', and 'Arazzo_PayloadReplacement_type_info'"
            )

    # 6. Check FFI IR emitter does not re-parse JSON string
    if norm_path == os.path.normpath("src/functions/ffi/cdd_ffi_emit_ir_json.c"):
        if "cdd_json_parse_string" in stripped:
            errors.append(
                f"{filepath}:1: cdd_ffi_emit_ir_json.c must not redundantly re-parse JSON string with cdd_json_parse_string"
            )

    # 7. Check emitted CLI option parsers return cdd_c_error_t
    if norm_path == os.path.normpath("src/routes/emit/cli_gen.c"):
        if (
            "static cdd_c_error_t parse_cli_global_options(" not in raw
            or "static cdd_c_error_t parse_cli_op_options(" not in raw
        ):
            errors.append(
                f"{filepath}:1: cli_gen.c must emit structured parse_cli_global_options and parse_cli_op_options returning cdd_c_error_t"
            )
    if norm_path == os.path.normpath("src/routes/emit/arazzo_cli_gen.c"):
        if "static cdd_c_error_t parse_workflow_cli_options(" not in raw:
            errors.append(
                f"{filepath}:1: arazzo_cli_gen.c must emit structured parse_workflow_cli_options returning cdd_c_error_t"
            )

    # 8. Check MCP response serialization dogfoods cdd_reflect_to_json_object
    if norm_path == os.path.normpath("src/routes/emit/serve_mcp_stdio.c"):
        if (
            "CddMcpResourceContent_type_info" not in stripped
            or "CddMcpTextContent_type_info" not in stripped
            or "cdd_reflect_to_json_object" not in stripped
        ):
            errors.append(
                f"{filepath}:1: serve_mcp_stdio.c must dogfood CddMcpResourceContent_type_info and CddMcpTextContent_type_info with cdd_reflect_to_json_object"
            )

    # 9. Forbid consecutive unchecked `rc = ...; rc = ...;` overwrites in audited modules
    is_transformer_mod = norm_path.startswith(os.path.normpath("src/transformers") + os.sep)
    if is_transformer_mod or norm_path in {
        os.path.normpath("src/routes/emit/serve_mcp_stdio.c"),
        os.path.normpath("src/routes/emit/serve_json_rpc.c"),
        os.path.normpath("src/openapi/emit/openapi.c"),
        os.path.normpath("src/openapi/emit/openapi_info.c"),
        os.path.normpath("src/arazzo/parse/arazzo_free.c"),
        os.path.normpath("src/arazzo/parse/arazzo_eval.c"),
    }:
        consec_rc_re = re.compile(
            r"(?<![\w])rc\s*=\s*(?![^;{}]*\bif\s*\()[^;{}]+;\s*\n[ \t]*rc\s*=(?!\s*(?:\(cdd_c_error_t\)\s*\(\s*\(int\)\s*rc\s*\||openapi_check_rc\s*\())"
        )
        for m in consec_rc_re.finditer(stripped):
            line_no = raw.count("\n", 0, m.start()) + 1
            errors.append(
                f"{filepath}:{line_no}: Consecutive unchecked 'rc = ...; rc = ...;' assignment overwrites previous error enum"
            )

    if is_transformer_mod:
        empty_swallow_re = re.compile(
            r"\bif\s*\([^{};]*!=\s*CDD_C_SUCCESS\s*\)\s*\{\s*\}", re.M
        )
        for m in empty_swallow_re.finditer(stripped):
            line_no = raw.count("\n", 0, m.start()) + 1
            errors.append(
                f"{filepath}:{line_no}: Forbidden empty '!= CDD_C_SUCCESS) {{}}' error-swallowing block"
            )
        void_cast_re = re.compile(r"\(void\)\s*[a-zA-Z_][a-zA-Z0-9_]*")
        for m in void_cast_re.finditer(stripped):
            line_no = raw.count("\n", 0, m.start()) + 1
            errors.append(
                f"{filepath}:{line_no}: Forbidden '(void)' discard in transformer module: {m.group(0)}"
            )

    # 10. Forbid empty `if ((_rc|api_rc) != CDD_C_SUCCESS) {}` blocks in code emitters
    if norm_path in {
        os.path.normpath("src/classes/emit/json.c"),
        os.path.normpath("src/functions/emit/client_body.c"),
    }:
        empty_err_re = re.compile(r"\b(_rc|api_rc)\s*!=\s*CDD_C_SUCCESS\s*\)\s*\{\s*\}")
        for m in empty_err_re.finditer(raw):
            line_no = raw.count("\n", 0, m.start()) + 1
            errors.append(
                f"{filepath}:{line_no}: Forbidden empty '!= CDD_C_SUCCESS) {{}}' error-swallowing block in generated code"
            )

    return errors


def check_cdd_api_cli_reflection() -> list[str]:
    errors = []
    h_path = os.path.join("src", "cdd_api.h")
    c_path = os.path.join("src", "cdd_api.c")
    if not (os.path.exists(h_path) and os.path.exists(c_path)):
        return errors

    with open(h_path, "r", encoding="utf-8") as f:
        h_text = f.read()
    with open(c_path, "r", encoding="utf-8") as f:
        c_text = f.read()

    struct_block_re = re.compile(
        r"@cdd\s+cli\([^)]*\)[\s\S]*?typedef\s+struct\s+\w+\s*\{([\s\S]*?)\}\s*(\w+)\s*;"
    )
    field_re = re.compile(
        r"/\*\*[^\n]*?@cdd\s+cli\(([^)]*)\)[^\n]*?\*/\s*\n\s*[^;]+\b(\w+)\s*;"
    )

    for m in struct_block_re.finditer(h_text):
        body, typedef_name = m.group(1), m.group(2)
        base_name = (
            typedef_name[:-2] if typedef_name.endswith("_t") else typedef_name
        )
        table_re = re.compile(
            rf"g_{re.escape(base_name)}_fields\[\]\s*=\s*\{{([\s\S]*?)\}}\s*;"
        )
        tm = table_re.search(c_text)
        if not tm:
            errors.append(
                f"{c_path}:1: Missing reflection table g_{base_name}_fields for @cdd cli struct {typedef_name}"
            )
            continue
        table_body = tm.group(1)
        for fm in field_re.finditer(body):
            cli_args, field_name = fm.group(1), fm.group(2)
            if "positional" in cli_args:
                continue
            long_m = re.search(r'long\s*=\s*"([^"]+)"', cli_args)
            short_m = re.search(r"short\s*=\s*'([^']+)'", cli_args)
            long_name = long_m.group(1) if long_m else field_name
            short_name = f'"{short_m.group(1)}"' if short_m else "NULL"
            entry_re = re.compile(
                rf'\{{\s*"{re.escape(field_name)}"\s*,\s*"{re.escape(long_name)}"[\s\S]*?,\s*{re.escape(short_name)}\s*,'
            )
            if not entry_re.search(table_body):
                errors.append(
                    f"{c_path}:1: Reflection field '{field_name}' in g_{base_name}_fields does not match @cdd cli(long=\"{long_name}\", short={short_name}) in {h_path}"
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

    all_errors.extend(check_cdd_api_cli_reflection())

    if all_errors:
        for err in all_errors:
            print(err, file=sys.stderr)
        print(f"Total dogfooding violations: {len(all_errors)}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
