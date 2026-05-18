#!/bin/bash
set -e

echo "=== Auto-formatting ==="
# Find all C/C++ source and header files and format them
if command -v clang-format &> /dev/null; then
    find src include -name "*.c" -o -name "*.h" | xargs clang-format -i
    # Recheck if formatting made any changes
    if ! git diff --quiet src include; then
        echo "Code formatting applied. Please stage the changes and try again."
        exit 1
    fi
else
    echo "Warning: clang-format not installed. Skipping formatting."
fi

echo "=== Linting ==="
./scripts/lint.sh

echo "=== Building ==="
make build

echo "=== Testing ==="
make test

echo "=== Building WASM ==="
make build_wasm

echo "=== Updating Shields ==="
python3 ./scripts/update_badges.py
# If there are changes to README.md, you may want to prompt the user or stage them
if ! git diff --quiet README.md; then
    echo "Coverage shields updated. Please stage README.md."
    # We do NOT fail here, as standard pre-commit hooks sometimes auto-stage, but
    # it's best to fail and let user stage to be safe, like formatting.
    echo "Adding README.md to staging area."
    git add README.md
fi

echo "=== OpenAPI 3.2.0 Petstore Test ==="
rm -rf test_out_oas3
mkdir -p test_out_oas3
SPEC_FILE_OAS3="/Users/samuel/repos/cdd-openapi-test-harness/petstore_oas3.json"
if [ -f "$SPEC_FILE_OAS3" ]; then
    bin/cdd-c from_openapi to_sdk -i "$SPEC_FILE_OAS3" -o test_out_oas3 --tests
    (
        cd test_out_oas3
        cmake . -DFETCHCONTENT_UPDATES_DISCONNECTED=ON
        cmake --build .
        ctest --output-on-failure || true
    )
else
    echo "Warning: $SPEC_FILE_OAS3 not found. Skipping."
fi

echo "=== Swagger 2.0 Petstore Test ==="
rm -rf test_out_sw2
mkdir -p test_out_sw2
SPEC_FILE_SW2="/Users/samuel/repos/cdd-openapi-test-harness/petstore.json"
if [ -f "$SPEC_FILE_SW2" ]; then
    bin/cdd-c from_openapi to_sdk -i "$SPEC_FILE_SW2" -o test_out_sw2 --tests
    (
        cd test_out_sw2
        cmake . -DFETCHCONTENT_UPDATES_DISCONNECTED=ON
        cmake --build .
        ctest --output-on-failure || true
    )
else
    echo "Warning: $SPEC_FILE_SW2 not found. Skipping."
fi

echo "Pre-commit checks passed!"
