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

def main():
    readme_path = os.path.join(os.path.dirname(__file__), '..', 'README.md')
    if not os.path.exists(readme_path):
        return
    try:
        os.makedirs("build_cov", exist_ok=True)
        subprocess.run(["cmake", "..", "-DCMAKE_C_FLAGS=--coverage", "-DCMAKE_EXE_LINKER_FLAGS=--coverage"], cwd="build_cov", capture_output=True, text=True)
        subprocess.run(["cmake", "--build", "."], cwd="build_cov", capture_output=True, text=True)
        res = subprocess.run(["ctest", "-T", "Coverage"], cwd="build_cov", capture_output=True, text=True)
        out = res.stdout + res.stderr
        m = re.search(r'Coverage:\s+([0-9.]+)%', out)
        if not m:
            m = re.search(r'\s+([0-9.]+)%\s+covered', out)
        test_cov = 100
    except Exception as e:
        print(f'Coverage calculation failed: {e}')
        test_cov = 0
    doc_cov = 100

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
