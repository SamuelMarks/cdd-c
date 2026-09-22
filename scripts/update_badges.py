#!/usr/bin/env python3
import os
import re
import subprocess


def get_color(pct):
    if pct >= 90:
        return "brightgreen"
    if pct >= 80:
        return "green"
    if pct >= 70:
        return "yellowgreen"
    if pct >= 60:
        return "yellow"
    if pct >= 50:
        return "orange"
    return "red"


def get_doc_coverage():
    repo_root = os.path.join(os.path.dirname(__file__), "..")
    include_dir = os.path.join(repo_root, "include")
    src_dir = os.path.join(repo_root, "src")

    total_funcs = 0
    documented_funcs = 0

    func_pattern = re.compile(
        r"^[a-zA-Z_][a-zA-Z0-9_*\s]*\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\([^;]*\)\s*;",
        re.MULTILINE,
    )
    doc_pattern = re.compile(r"/\*\*[\s\S]*?\*/\s*[a-zA-Z_]", re.MULTILINE)

    # Very basic doc coverage: check how many headers have doxygen-like comments
    # or just parse all .h files.
    # For a more robust approach without an external parser, we count functions and doc blocks.

    for root_dir in [include_dir, src_dir]:
        if not os.path.exists(root_dir):
            continue
        for root, _, files in os.walk(root_dir):
            for file in files:
                if file.endswith(".h") or file.endswith(".c"):
                    with open(os.path.join(root, file), "r", encoding="utf-8") as f:
                        content = f.read()

                        # count functions (heuristic)
                        funcs = func_pattern.findall(content)
                        total_funcs += len(funcs)

                        # count doc comments
                        docs = len(re.findall(r"/\*\*[\s\S]*?\*/", content))
                        documented_funcs += docs

    if total_funcs == 0:
        return 100  # default if no functions found

    # Cap at 100% since doc comments might exceed function declarations in this simple heuristic
    cov = (
        min(100, int((documented_funcs / total_funcs) * 100))
        if total_funcs > 0
        else 100
    )
    return cov


def get_test_coverage():
    repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    import shutil

    if not shutil.which("gcovr"):
        return None

    build_dirs = ["build_gcc", "build_cov", "build"]

    for bdir_name in build_dirs:
        bdir = os.path.join(repo_root, bdir_name)
        if not os.path.isdir(bdir):
            continue

        # Look for compiler in CMakeCache.txt
        compiler = None
        cache_path = os.path.join(bdir, "CMakeCache.txt")
        if os.path.exists(cache_path):
            try:
                with open(cache_path, "r", encoding="utf-8", errors="ignore") as f:
                    for line in f:
                        if line.startswith("CMAKE_C_COMPILER:"):
                            compiler = line.split("=", 1)[1].strip()
                            break
            except Exception:
                pass

        candidates = []
        if os.environ.get("GCOV"):
            candidates.append(os.environ["GCOV"])

        if compiler:
            c_basename = os.path.basename(compiler)
            c_dirname = os.path.dirname(compiler)
            if "gcc" in c_basename:
                gcov_name = c_basename.replace("gcc", "gcov")
                if c_dirname:
                    candidates.append(os.path.join(c_dirname, gcov_name))
                candidates.append(gcov_name)
            elif "clang" in c_basename:
                candidates.append("llvm-cov gcov")
                candidates.append("gcov")

        for v in range(20, 9, -1):
            candidates.append(f"gcov-{v}")
            candidates.append(f"/opt/homebrew/bin/gcov-{v}")
            candidates.append(f"/usr/local/bin/gcov-{v}")
        candidates.append("gcov")
        if shutil.which("llvm-cov"):
            candidates.append("llvm-cov gcov")

        seen = set()
        unique_candidates = []
        for c in candidates:
            if c not in seen:
                seen.add(c)
                unique_candidates.append(c)

        def try_run_gcovr():
            for candidate in unique_candidates:
                base_tool = candidate.split()[0]
                if not os.path.isabs(base_tool) and not shutil.which(base_tool):
                    continue
                if os.path.isabs(base_tool) and not os.path.exists(base_tool):
                    continue

                cmd = [
                    "gcovr",
                    "-r",
                    repo_root,
                    bdir,
                    "--filter",
                    os.path.join(repo_root, "src") + os.sep,
                    "--filter",
                    os.path.join(repo_root, "include") + os.sep,
                    "--exclude",
                    os.path.join(repo_root, "src", "tests") + os.sep,
                    "--exclude",
                    os.path.join(repo_root, "src", "mocks") + os.sep,
                    "--gcov-ignore-parse-errors=negative_hits.warn",
                    "--print-summary",
                ]
                if candidate != "gcov":
                    cmd.extend(["--gcov-executable", candidate])

                try:
                    res = subprocess.run(
                        cmd, cwd=bdir, capture_output=True, text=True
                    )
                    if res.returncode == 0:
                        m = re.search(
                            r"lines:\s+([0-9.]+)%\s+\((\d+)\s+out\s+of\s+(\d+)\)",
                            res.stdout,
                        )
                        if m and int(m.group(3)) > 0:
                            return float(m.group(1))
                        m_simple = re.search(r"lines:\s+([0-9.]+)%", res.stdout)
                        if m_simple and float(m_simple.group(1)) > 0.0:
                            return float(m_simple.group(1))
                except Exception:
                    pass
            return None

        cov = try_run_gcovr()
        if cov is not None:
            return cov

        # If coverage failed, maybe tests need to be run first
        test_bin = os.path.join(bdir, "bin", "test_c_cdd")
        has_run = False
        if os.path.exists(test_bin):
            try:
                subprocess.run([test_bin], cwd=bdir, capture_output=True)
                has_run = True
            except Exception:
                pass
        elif shutil.which("ctest"):
            try:
                subprocess.run(
                    ["ctest", "--output-on-failure"],
                    cwd=bdir,
                    capture_output=True,
                )
                has_run = True
            except Exception:
                pass

        if has_run:
            cov = try_run_gcovr()
            if cov is not None:
                return cov

    return None


def main():
    readme_path = os.path.join(os.path.dirname(__file__), "..", "README.md")
    if not os.path.exists(readme_path):
        return

    test_cov = None
    if os.name != "nt":
        try:
            test_cov = get_test_coverage()
        except Exception as e:
            print(f"Coverage calculation failed: {e}")

    doc_cov = get_doc_coverage()

    doc_color = get_color(doc_cov)

    with open(readme_path, "r", encoding="utf-8", newline="\n") as f:
        content = f.read()

    if test_cov is not None:
        test_color = get_color(test_cov)
        test_badge = f"[![Test Coverage](https://img.shields.io/badge/test_coverage-{test_cov}%25-{test_color}.svg)](#)"
        test_badge_pattern = r"\[\!\[Test Coverage\]\(.*?\)\]\(.*?\)"
        if re.search(test_badge_pattern, content):
            content = re.sub(test_badge_pattern, test_badge, content)
        else:
            doc_badge_pattern = r"(\[\!\[Doc Coverage\].*?\]\(.*?\)\n?)"
            if re.search(doc_badge_pattern, content):
                content = re.sub(doc_badge_pattern, r"\1" + test_badge + "\n", content, count=1)
            elif "[![License]" in content:
                content = re.sub(r"(\[!\[License\].*?\]\(.*?\)\n?)", r"\1" + test_badge + "\n", content, count=1)

    doc_badge = f"[![Doc Coverage](https://img.shields.io/badge/doc_coverage-{doc_cov}%25-{doc_color}.svg)](#)"
    doc_badge_pattern = r"\[\!\[Doc Coverage\]\(.*?\)\]\(.*?\)"
    if re.search(doc_badge_pattern, content):
        content = re.sub(doc_badge_pattern, doc_badge, content)
    else:
        if "[![License]" in content:
            content = re.sub(r"(\[!\[License\].*?\]\(.*?\)\n?)", r"\1" + doc_badge + "\n", content, count=1)

    with open(readme_path, "w", encoding="utf-8", newline="\n") as f:
        f.write(content)


if __name__ == "__main__":
    main()
