#!/bin/bash
if command -v clang-tidy &> /dev/null; then
    echo "Running clang-tidy..."
    find src include -name "*.c" -o -name "*.h" | xargs clang-tidy -p build --quiet
elif command -v cppcheck &> /dev/null; then
    echo "Running cppcheck..."
    cppcheck --enable=all --suppress=missingIncludeSystem src include
else
    echo "Warning: Neither clang-tidy nor cppcheck is installed. Skipping linting."
fi
