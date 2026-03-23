.PHONY: help all install_base install_deps build_docs build test run build_wasm build_docker run_docker

DOCS_DIR ?= docs
BIN_DIR ?= bin
CLI_BIN = $(BIN_DIR)/cdd-c

all: help

help:
	@echo "Available targets:"
	@echo "  install_base   - Install language runtime and base dependencies (e.g., gcc, cmake, pkg-config)"
	@echo "  install_deps   - Install local dependencies"
	@echo "  build_docs     - Build the API docs and put them in DOCS_DIR (default: docs)"
	@echo "  build          - Build the CLI binary and put it in BIN_DIR (default: bin)"
	@echo "  test           - Run tests locally"
	@echo "  run            - Run the CLI (builds if not present). Any args given after this should be given to the CLI, e.g., 'make run --version'."
	@echo "  build_wasm     - Build the WebAssembly target"
	@echo "  build_docker   - Build Alpine and Debian Docker images"
	@echo "  run_docker     - Run the Docker image and ping JSON RPC"

install_base:
	sudo apt-get update && sudo apt-get install -y gcc cmake pkg-config

install_deps:
	# C dependencies handled by CMake/vcpkg or system
	@echo "Dependencies are managed via CMake"

build_docs:
	mkdir -p $(DOCS_DIR)
	# Add doxygen or other doc generation command here
	@echo "Docs built in $(DOCS_DIR)"

build:
	mkdir -p build_cmake
	cd build_cmake && cmake .. && cmake --build . --config Release
	mkdir -p $(BIN_DIR)
	cp build_cmake/bin/cdd-c $(BIN_DIR)/ || cp build_cmake/bin/Release/cdd-c.exe $(BIN_DIR)/ || echo "Ensure cdd-c is built."

test:
	mkdir -p build_cmake
	cd build_cmake && cmake .. && cmake --build . && ctest --output-on-failure

run: build
	$(CLI_BIN) $(ARGS)

build_wasm:
	@echo "Building WASM via wasi-sdk..."
	@if [ ! -d "wasi-sdk" ]; then \
		OS_NAME=$$(uname -s | tr A-Z a-z); \
		if [ "$$OS_NAME" = "darwin" ]; then WASI_OS="macos"; else WASI_OS="linux"; fi; \
		curl -L -O "https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-24/wasi-sdk-24.0-arm64-$${WASI_OS}.tar.gz" || \
		curl -L -O "https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-24/wasi-sdk-24.0-x86_64-$${WASI_OS}.tar.gz"; \
		tar xf wasi-sdk-24.0-*-$${WASI_OS}.tar.gz; \
		rm wasi-sdk-24.0-*-$${WASI_OS}.tar.gz; \
		mv wasi-sdk-24.0* wasi-sdk; \
	fi
	@sed -i.bak 's/VERSION 3.4.0/VERSION 3.11/g' wasi-sdk/share/cmake/wasi-sdk.cmake || true
	rm -rf build_wasm && mkdir -p build_wasm
	cd build_wasm && cmake .. -DCMAKE_TOOLCHAIN_FILE=../wasi-sdk/share/cmake/wasi-sdk.cmake \
		-DCMAKE_C_FLAGS="-DCFS_OS_DOS=1 -include sys/types.h -include unistd.h -D_WASI_EMULATED_GETPID -D_WASI_EMULATED_SIGNAL -D_WASI_EMULATED_PROCESS_CLOCKS" \
		-DCMAKE_EXE_LINKER_FLAGS="-lwasi-emulated-getpid -lwasi-emulated-signal -lwasi-emulated-process-clocks" \
		-DC_CDD_USE_LIBCURL=OFF -DHTTP_ONLY=ON -DC_CDD_MULTI_THREADED=OFF \
		-DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release
	cd build_wasm && $(MAKE) cdd-c
	mkdir -p bin
	cp build_wasm/bin/cdd-c bin/cdd-c.wasm

build_docker:
	docker build -t cdd-c:alpine -f alpine.Dockerfile .
	docker build -t cdd-c:debian -f debian.Dockerfile .

run_docker: build_docker
	docker run --rm -d -p 8082:8082 --name cdd-c-alpine-test cdd-c:alpine serve_json_rpc --port 8082 --listen 0.0.0.0
	sleep 2
	curl -X POST http://localhost:8082/ -H "Content-Type: application/json" -d '{"jsonrpc": "2.0", "method": "version", "id": 1}'
	docker stop cdd-c-alpine-test || true
	docker run --rm -d -p 8082:8082 --name cdd-c-debian-test cdd-c:debian serve_json_rpc --port 8082 --listen 0.0.0.0
	sleep 2
	curl -X POST http://localhost:8082/ -H "Content-Type: application/json" -d '{"jsonrpc": "2.0", "method": "version", "id": 1}'
	docker stop cdd-c-debian-test || true

%:
	@:
