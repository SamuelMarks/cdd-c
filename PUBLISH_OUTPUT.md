# Publishing the Client SDK

This document describes how to continuously generate and publish the `C` client SDK produced by `cdd-c` based on your OpenAPI specifications.

## Continuous Integration via GitHub Actions

To ensure the client library is always up-to-date with your backend server OpenAPI spec, use a scheduled cron job or a webhook in a GitHub Action:

```yaml
name: Update API SDK

on:
  schedule:
    - cron: '0 0 * * *' # Every night
  workflow_dispatch:

jobs:
  generate:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Install Base
        run: sudo apt-get install -y flex bison cmake
      - name: Build cdd-c
        run: make build
      - name: Fetch Latest Spec
        run: curl -sL https://api.mybackend.com/openapi.json -o spec.json
      - name: Generate SDK
        run: ./bin/cdd-c from_openapi to_sdk -i spec.json -o src/models
      - name: Commit and Push Changes
        uses: stefanzweifel/git-auto-commit-action@v5
        with:
          commit_message: "chore: update C client SDK to match latest OpenAPI spec"
          branch: main
```

## Distribution

The resulting `src/models` files can either be embedded directly in a consumer repository, or you can commit them into a separate `my-api-client-c` repository that consumer applications fetch via `git submodule` or `CMake FetchContent`.
