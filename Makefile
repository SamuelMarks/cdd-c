.PHONY: install_base install_deps build_docs build test run help all build_wasm

all: help

help:
	@echo "Available tasks:"
	@echo "  install_base : install language runtime and anything else relevant"
	@echo "  install_deps : install local dependencies (CMake automatically fetches via vcpkg/FetchContent)"
	@echo "  build_docs   : build the API docs and put them in the 'docs' directory. Usage: make build_docs [docs/path]"
	@echo "  build        : build the CLI binary. Usage: make build [build/path]"
	@echo "  build_wasm   : build the project as a WebAssembly module using Emscripten"
	@echo "  test         : run tests locally"
	@echo "  run          : run the CLI. If not built, it builds first. Any args after run are passed to CLI."
	@echo "                 Usage: make run --version"

install_base:
	@if [ -f /etc/debian_version ]; then \
		sudo apt-get update && sudo apt-get install -y cmake gcc g++ clang pkg-config; \
	elif [ -f /etc/redhat-release ]; then \
		sudo dnf install -y cmake gcc gcc-c++ clang pkgconf; \
	elif [ "$$(uname)" = "Darwin" ]; then \
		brew install cmake llvm pkg-config; \
	elif [ "$$(uname)" = "FreeBSD" ]; then \
		sudo pkg install -y cmake gcc clang pkgconf; \
	else \
		echo "Please install CMake, GCC/Clang, and pkg-config via your package manager."; \
	fi

install_deps:
	@echo "Dependencies are handled natively by CMake during the configure step."

build_docs:
	$(eval DOCS_DIR := $(word 2,$(MAKECMDGOALS)))
	@if [ -z "$(DOCS_DIR)" ]; then DOCS_DIR="docs"; fi; \
	mkdir -p $$DOCS_DIR; \
	echo "Generating docs into $$DOCS_DIR..."; \
	./build/bin/cdd-c to_docs_json --no-imports --no-wrapping -i src/mocks/parse/petstore.openapi.json > $$DOCS_DIR/docs.json

build:
	$(eval BUILD_DIR := $(word 2,$(MAKECMDGOALS)))
	@if [ -z "$(BUILD_DIR)" ] || [ "$(BUILD_DIR)" = "test" ] || [ "$(BUILD_DIR)" = "run" ] || [ "$(BUILD_DIR)" = "--version" ] || [ "$(BUILD_DIR)" = "--help" ] || [ "$(BUILD_DIR)" = "from_openapi" ] || [ "$(BUILD_DIR)" = "to_openapi" ] || [ "$(BUILD_DIR)" = "to_docs_json" ]; then BUILD_DIR="build"; fi; \
	cmake -S . -B $$BUILD_DIR -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DC_CDD_BUILD_TESTING=ON; \
	cmake --build $$BUILD_DIR -j

build_wasm:
	@echo "Building WebAssembly target using Emscripten..."
	@if [ ! -f ../emsdk/emsdk_env.sh ]; then \
		echo "emsdk not found at ../emsdk. Please ensure it is installed."; \
		exit 1; \
	fi
	bash -c "source ../emsdk/emsdk_env.sh && emcmake cmake -S . -B build_wasm -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DC_CDD_BUILD_TESTING=OFF -DHTTP_ONLY=ON -DBUILD_CURL_EXE=OFF -DCMAKE_USE_OPENSSL=OFF -DCURL_USE_OPENSSL=OFF && emmake cmake --build build_wasm -j"

test: build
	$(eval BUILD_DIR := $(word 2,$(MAKECMDGOALS)))
	@if [ -z "$(BUILD_DIR)" ]; then BUILD_DIR="build"; fi; \
	cd $$BUILD_DIR && ctest --output-on-failure

run: build
	$(eval RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS)))
	@if [ ! -f build/bin/cdd-c ]; then \
		echo "Binary not found, ensure build succeeds"; \
		exit 1; \
	fi; \
	./build/bin/cdd-c $(RUN_ARGS)

# Catch-all to prevent make from complaining about arguments
%:
	@:
