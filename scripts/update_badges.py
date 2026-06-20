#!/usr/bin/env python3
import os
import re
import subprocess

def get_color(pct):
    if pct >= 90: return 'brightgreen'
    if pct >= 80: return 'green'
    if pct >= 70: return 'yellowgreen'
    if pct >= 60: return 'yellow'
    if pct >= 50: return 'orange'
    return 'red'

def get_doc_coverage():
    repo_root = os.path.join(os.path.dirname(__file__), '..')
    include_dir = os.path.join(repo_root, 'include')
    src_dir = os.path.join(repo_root, 'src')

    total_funcs = 0
    documented_funcs = 0

    func_pattern = re.compile(r'^[a-zA-Z_][a-zA-Z0-9_*\s]*\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\([^;]*\)\s*;', re.MULTILINE)
    doc_pattern = re.compile(r'/\*\*[\s\S]*?\*/\s*[a-zA-Z_]', re.MULTILINE)

    # Very basic doc coverage: check how many headers have doxygen-like comments
    # or just parse all .h files.
    # For a more robust approach without an external parser, we count functions and doc blocks.

    for root_dir in [include_dir, src_dir]:
        if not os.path.exists(root_dir): continue
        for root, _, files in os.walk(root_dir):
            for file in files:
                if file.endswith('.h') or file.endswith('.c'):
                    with open(os.path.join(root, file), 'r', encoding='utf-8') as f:
                        content = f.read()

                        # count functions (heuristic)
                        funcs = func_pattern.findall(content)
                        total_funcs += len(funcs)

                        # count doc comments
                        docs = len(re.findall(r'/\*\*[\s\S]*?\*/', content))
                        documented_funcs += docs

    if total_funcs == 0:
        return 100 # default if no functions found

    # Cap at 100% since doc comments might exceed function declarations in this simple heuristic
    cov = min(100, int((documented_funcs / total_funcs) * 100)) if total_funcs > 0 else 100
    return cov

def main():
    readme_path = os.path.join(os.path.dirname(__file__), '..', 'README.md')
    if not os.path.exists(readme_path):
        return
    try:
        os.makedirs("build_cov", exist_ok=True)
        subprocess.run(["cmake", "..", "-DCMAKE_C_FLAGS=--coverage", "-DCMAKE_EXE_LINKER_FLAGS=--coverage"], cwd="build_cov", capture_output=True, text=True)
        subprocess.run(["cmake", "--build", "."], cwd="build_cov", capture_output=True, text=True)
        subprocess.run(["ctest"], cwd="build_cov", capture_output=True, text=True)
        res = subprocess.run(["ctest", "-T", "Coverage"], cwd="build_cov", capture_output=True, text=True)
        out = res.stdout + res.stderr
        m = re.search(r'Coverage:\s+([0-9.]+)%', out)
        if not m:
            m = re.search(r'\s+([0-9.]+)%\s+covered', out)

        if m:
            test_cov = float(m.group(1))
        else:
            test_cov = 0
    except Exception as e:
        print(f'Coverage calculation failed: {e}')
        test_cov = 0

    doc_cov = get_doc_coverage()

    test_color = get_color(test_cov)
    doc_color = get_color(doc_cov)

    with open(readme_path, 'r', encoding='utf-8', newline='\n') as f:
        content = f.read()

    content = re.sub(
        r'\[\!\[Test Coverage\]\(https://img\.shields\.io/badge/test_coverage-[0-9.]+%25-[a-z]+\.svg\)\]\(#\)',
        f'[![Test Coverage](https://img.shields.io/badge/test_coverage-{test_cov}%25-{test_color}.svg)](#)',
        content
    )

    content = re.sub(
        r'\[\!\[Doc Coverage\]\(https://img\.shields\.io/badge/doc_coverage-[0-9.]+%25-[a-z]+\.svg\)\]\(#\)',
        f'[![Doc Coverage](https://img.shields.io/badge/doc_coverage-{doc_cov}%25-{doc_color}.svg)](#)',
        content
    )

    with open(readme_path, 'w', encoding='utf-8', newline='\n') as f:
        f.write(content)

if __name__ == '__main__':
    main()
