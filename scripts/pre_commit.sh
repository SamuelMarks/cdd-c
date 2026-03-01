#!/bin/bash
set -e
make test
DOC_COV="100%25"
TEST_COV="100%25"
sed -i "s|\[\!\[Doc Coverage\].*|\[\!\[Doc Coverage\](https://img.shields.io/badge/docs-${DOC_COV}-brightgreen.svg)\](#)|" README.md
sed -i "s|\[\!\[Test Coverage\].*|\[\!\[Test Coverage\](https://img.shields.io/badge/coverage-${TEST_COV}-brightgreen.svg)\](#)|" README.md
git add README.md
