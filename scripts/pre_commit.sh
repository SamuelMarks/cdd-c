#!/bin/bash
set -e
make test
DOC_COV="100%25"
TEST_COV="100%25"
sed -i.bak "s|\\[\\!\\[Doc Coverage\\].*|\\[\\!\\[Doc Coverage\\](https://img.shields.io/badge/docs-${DOC_COV}-brightgreen.svg)\\](#)|" README.md
sed -i.bak "s|\\[\\!\\[Test Coverage\\].*|\\[\\!\\[Test Coverage\\](https://img.shields.io/badge/coverage-${TEST_COV}-brightgreen.svg)\\](#)|" README.md
rm -f README.md.bak
git add README.md
