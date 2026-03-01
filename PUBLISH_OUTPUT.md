# Publishing Client SDKs (Continuous Delivery)

The `cdd-c` tool makes it trivial to ensure your client libraries are always in perfect harmony with the latest server OpenAPI specifications.

## Automated Updates via GitHub Actions
You should set up a scheduled or webhook-triggered GitHub Action in your generated client repository to automatically fetch the latest API specification and run the `from_openapi` compiler.

```yaml
name: Sync C SDK

on:
  schedule:
    - cron: '0 0 * * *' # Run daily
  repository_dispatch:
    types: [openapi_updated] # Or trigger manually from your server repo

jobs:
  update-sdk:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Install cdd-c
        run: |
          wget https://github.com/offscale/cdd-c/releases/latest/download/cdd-c-linux-amd64
          chmod +x cdd-c-linux-amd64
      - name: Fetch Latest Spec
        run: curl -O https://api.example.com/openapi.json
      - name: Generate SDK
        run: ./cdd-c-linux-amd64 from_openapi -i openapi.json
      - name: Create Pull Request
        uses: peter-evans/create-pull-request@v5
        with:
          commit-message: "Automated SDK Sync via cdd-c"
          title: "[cdd-c] Sync Client SDK to latest OpenAPI specification"
          branch: chore/sync-sdk
```

This ensures your C applications, embedded systems, or hardware endpoints remain compatible with any structural API shifts the moment they happen.