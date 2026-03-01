# Publishing `cdd-c`

## Publishing the Package

In the C ecosystem, packages are often managed via package managers like `vcpkg` and `Conan`.

### 1. Vcpkg Registry
To publish to a `vcpkg` registry, update `vcpkg.json` and push it to your registry repository. Alternatively, open a PR on `microsoft/vcpkg` port.

```json
{
  "name": "cdd-c",
  "version": "0.0.1",
  "description": "Code-Driven Development compiler for OpenAPI and C",
  "homepage": "https://github.com/offscale/cdd-c"
}
```

### 2. Conan Center
To publish to `ConanCenter`, create a `conanfile.py` and run:

```bash
conan create .
conan upload cdd-c/0.0.1 -r my-remote
```

## Publishing the Docs

To build and host static documentation (e.g., using Doxygen or Sphinx):

### Local Serving
```bash
make build_docs
cd docs/html
python3 -m http.server 8000
```
Then navigate to `http://localhost:8000`.

### GitHub Pages (Most Popular Location)
Use the GitHub Actions workflow to publish the `docs/` folder directly to the `gh-pages` branch.
