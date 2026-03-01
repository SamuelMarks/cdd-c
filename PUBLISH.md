# Publishing `cdd-c`

The `cdd-c` library is a native C package designed to be distributed primarily via source and package managers like vcpkg or apt/brew tap channels.

## Distributing Source Releases
Since C libraries aren't distributed on npm or crates.io, we use GitHub Releases.
1. Bump the version in `cmake/config.h.in` and `CMakeLists.txt`.
2. Tag the release: `git tag v1.0.0`
3. Push tags: `git push origin v1.0.0`
4. GitHub Actions will build the artifacts (Linux, Windows, macOS, and WASM) and attach them to the release.

## Publishing Documentation
To host the generated `to_docs_json` examples statically:

```sh
# Generate the docs payload
make build_docs docs/

# This creates docs/docs.json. You can serve this via GitHub Pages or a simple HTTP server:
python3 -m http.server -d docs/
```

To sync with a centralized docs repository:
```sh
# Copy to central repo
cp docs/docs.json ../docs-site/content/cdd-c-examples.json
cd ../docs-site && git commit -am "Update C API docs" && git push
```