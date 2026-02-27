# Publishing cdd-c

## Package Registry (vcpkg / Conan / System Packages)
For C/C++ libraries, the "most popular location" is often system package managers (apt, pacman, brew) or C++ specific package managers like `vcpkg` or `Conan`.

Since `cdd-c` uses `CMake` and `vcpkg` (based on `vcpkg.json` and the CI logs), the primary way to "publish" it for developers to consume is by creating a `vcpkg` port.

To add `cdd-c` to the official `vcpkg` registry:
1. Fork the `microsoft/vcpkg` repository.
2. Create a new port directory: `ports/cdd-c/`.
3. Create a `vcpkg.json` and a `portfile.cmake` inside that directory.
4. The `portfile.cmake` would download the release archive from GitHub and use `vcpkg_cmake_configure` and `vcpkg_cmake_install`.
5. Submit a Pull Request to `microsoft/vcpkg`.

Example `portfile.cmake`:
```cmake
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO SamuelMarks/c_cdd
    REF v0.0.2
    SHA512 <hash>
    HEAD_REF master
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME c_cdd)
file(INSTALL "${SOURCE_PATH}/LICENSE-MIT" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
```

## Documentation

### Generating Local Static Docs
For C projects, `Doxygen` is the standard for generating HTML documentation from source code comments.

1. Install Doxygen (`sudo apt install doxygen` or `brew install doxygen`).
2. Run `doxygen -g` to generate a `Doxyfile` configuration file.
3. Edit `Doxyfile`:
   - Set `PROJECT_NAME = "cdd-c"`
   - Set `INPUT = src/`
   - Set `RECURSIVE = YES`
   - Set `GENERATE_HTML = YES`
   - Set `HTML_OUTPUT = docs_html`
4. Run `doxygen Doxyfile`.
5. The generated HTML will be in the `docs_html` folder. You can serve this locally using Python:
   ```bash
   python3 -m http.server -d docs_html
   ```

### Publishing Docs (GitHub Pages)
The most popular place to host documentation for open-source projects is GitHub Pages.

You can automate this with a GitHub Action (`.github/workflows/docs.yml`):

```yaml
name: Deploy Doxygen to GitHub Pages

on:
  push:
    branches: ["master"]

jobs:
  build-and-deploy:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Install Doxygen
        run: sudo apt-get install doxygen
      - name: Build Docs
        run: doxygen Doxyfile
      - name: Deploy to GitHub Pages
        uses: peaceiris/actions-gh-pages@v3
        with:
          github_token: ${{ secrets.GITHUB_TOKEN }}
          publish_dir: ./docs_html
```
