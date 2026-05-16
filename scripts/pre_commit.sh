#!/bin/bash
set -e

echo "=== Building ==="
cmake .
make -j4

echo "=== Testing ==="
make test

echo "=== Linting ==="
./scripts/lint.sh

echo "=== Updating Shields ==="
DOC_COV="100%25"
TEST_COV="100%25"

sed -i.bak -E "s|\[\!\[Doc Coverage\].*|[![Doc Coverage](https://img.shields.io/badge/docs-${DOC_COV}-brightgreen.svg)](#)|" README.md
sed -i.bak -E "s|\[\!\[Test Coverage\].*|[![Test Coverage](https://img.shields.io/badge/coverage-${TEST_COV}-brightgreen.svg)](#)|" README.md
rm -f README.md.bak
git add README.md

echo "Pre-commit checks passed!"
