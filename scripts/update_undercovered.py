import json

with open("build_cov/cov_brief.json") as f:
    data = json.load(f)

files = data.get("files", [])
undercovered = []
covered = []

for file in files:
    fn = file["filename"]
    if "/src/" in fn:
        fn = "src/" + fn.split("/src/", 1)[1]
    elif "/include/" in fn:
        fn = "include/" + fn.split("/include/", 1)[1]
    elif not (fn.startswith("src/") or fn.startswith("include/")):
        continue
    file["filename"] = fn
    lp = file.get("line_percent", 0.0) or 0.0
    fp = file.get("function_percent", 0.0) or 0.0
    bp = file.get("branch_percent", 0.0)
    bc = file.get("branch_covered", 0)
    bt = file.get("branch_total", 0)
    if bp is None:
        bp_str = "0/0 (N/A)"
        bp_val = 100.0
    else:
        bp_str = "{0}/{1} ({2:.1f}%)".format(bc, bt, bp)
        bp_val = bp

    lc = file.get("line_covered", 0)
    lt = file.get("line_total", 0)
    fc = file.get("function_covered", 0)
    ft = file.get("function_total", 0)

    entry = {
        "filename": file["filename"],
        "line_str": "{0}/{1} ({2:.1f}%)".format(lc, lt, lp),
        "func_str": "{0}/{1} ({2:.1f}%)".format(fc, ft, fp),
        "branch_str": bp_str,
    }

    is_test_runner = fn in ("src/tests/test_c_cdd.c", "src/tests/parse/test_simple_json.c")
    is_test_file = fn.startswith("src/tests/")
    if not is_test_runner and (lp < 100.0 or fp < 100.0 or (not is_test_file and bp_val < 100.0)):
        undercovered.append(entry)
    else:
        covered.append(entry)

undercovered.sort(key=lambda x: x["filename"])
covered.sort(key=lambda x: x["filename"])

lines = []
lines.append("# Undercovered Files (< 100% Test Coverage)")
lines.append("")
lines.append("This document tracks all files in `cdd-c` with less than **100% test coverage** across **functions**, **lines**, or **branches**.")
lines.append("")
lines.append("> **Coverage Methodology:** Measured using GCC coverage instrumentation (`--coverage`) and analyzed with `gcovr` across the full test suite (`test_c_cdd`, `test_simple_json`, `test_cdd_c_help`, `test_cdd_c_fail`).")
lines.append("")
lines.append("## Coverage Summary")
lines.append("")
lines.append("- **Total Files Evaluated:** {0}".format(len(files)))
lines.append("- **Undercovered Files (< 100% Coverage):** {0}".format(len(undercovered)))
lines.append("- **Fully Covered Files (100% Coverage):** {0}".format(len(covered)))
tot_lc = data.get("line_covered")
tot_lt = data.get("line_total")
tot_fc = data.get("function_covered")
tot_ft = data.get("function_total")
tot_bc = data.get("branch_covered")
tot_bt = data.get("branch_total")
lp = data.get("line_percent")
fp = data.get("function_percent")
bp = data.get("branch_percent")
lines.append("- **Total Lines:** {0:,} / {1:,} ({2:.1f}%)".format(tot_lc, tot_lt, lp))
lines.append("- **Total Functions:** {0:,} / {1:,} ({2:.1f}%)".format(tot_fc, tot_ft, fp))
lines.append("- **Total Branches:** {0:,} / {1:,} ({2:.1f}%)".format(tot_bc, tot_bt, bp))
lines.append("")
lines.append("---")
lines.append("")
lines.append("## Undercovered Files (< 100% Coverage)")
lines.append("")
for u in undercovered:
    lines.append("- [ ] `{0}` — Lines: {1}, Functions: {2}, Branches: {3}".format(
        u["filename"], u["line_str"], u["func_str"], u["branch_str"]))
lines.append("")
lines.append("---")
lines.append("")
lines.append("## Undercovered Files by Component")
lines.append("")

categories = [
    ("Headers (`include/`)", lambda fn: fn.startswith("include/")),
    ("Core Library API (`src/`)", lambda fn: fn.startswith("src/") and fn.count("/") == 1),
    ("CST Classes (`src/classes/`)", lambda fn: fn.startswith("src/classes/")),
    ("Docstrings (`src/docstrings/`)", lambda fn: fn.startswith("src/docstrings/")),
    ("Functions (`src/functions/`)", lambda fn: fn.startswith("src/functions/")),
    ("OpenAPI (`src/openapi/`)", lambda fn: fn.startswith("src/openapi/")),
    ("Routes (`src/routes/`)", lambda fn: fn.startswith("src/routes/")),
    ("Transformers (`src/transformers/`)", lambda fn: fn.startswith("src/transformers/")),
    ("Tests (`src/tests/`)", lambda fn: fn.startswith("src/tests/")),
]

for title, pred in categories:
    cat_files = [u for u in undercovered if pred(u["filename"])]
    if cat_files:
        lines.append("### {0}".format(title))
        lines.append("")
        for u in cat_files:
            lines.append("- [ ] `{0}` — Lines: {1}, Functions: {2}, Branches: {3}".format(
                u["filename"], u["line_str"], u["func_str"], u["branch_str"]))
        lines.append("")

lines.append("---")
lines.append("")
lines.append("## Fully Covered Files (100% Coverage)")
lines.append("")
for c in covered:
    lines.append("- [x] `{0}` — Lines: {1}, Functions: {2}, Branches: {3}".format(
        c["filename"], c["line_str"], c["func_str"], c["branch_str"]))
lines.append("")

with open("UNDERCOVERED.md", "w") as f:
    f.write("\n".join(lines))

print("UNDERCOVERED.md updated successfully!")
