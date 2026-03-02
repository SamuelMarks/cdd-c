@echo off
setlocal ENABLEDELAYEDEXPANSION

if "%1"=="" goto help
if "%1"=="help" goto help
if "%1"=="all" goto help
if "%1"=="install_base" goto install_base
if "%1"=="install_deps" goto install_deps
if "%1"=="build_docs" goto build_docs
if "%1"=="build" goto build
if "%1"=="test" goto test
if "%1"=="run" goto run
if "%1"=="build_wasm" goto build_wasm
if "%1"=="build_docker" goto build_docker
if "%1"=="run_docker" goto run_docker

:help
echo Available targets:
echo   install_base   - Install language runtime and base dependencies (e.g., cmake)
echo   install_deps   - Install local dependencies
echo   build_docs     - Build the API docs and put them in DOCS_DIR (default: docs)
echo   build          - Build the CLI binary
echo   test           - Run tests locally
echo   run            - Run the CLI (builds if not present)
echo   build_wasm     - Build the WebAssembly target
echo   build_docker   - Build Alpine and Debian Docker images
echo   run_docker     - Run the Docker image and ping JSON RPC
goto end

:install_base
echo Install cmake, Visual Studio build tools
goto end

:install_deps
echo Dependencies are managed via CMake
goto end

:build_docs
set DOCS_DIR=docs
if not "%2"=="" set DOCS_DIR=%2
if not exist "%DOCS_DIR%" mkdir "%DOCS_DIR%"
echo Docs built in %DOCS_DIR%
goto end

:build
set BIN_DIR=bin
if not "%2"=="" set BIN_DIR=%2
if not exist "build_cmake" mkdir build_cmake
cd build_cmake
cmake ..
cmake --build . --config Release
cd ..
if not exist "%BIN_DIR%" mkdir "%BIN_DIR%"
copy build_cmake\bin\Release\cdd-c.exe "%BIN_DIR%\"
goto end

:test
if not exist "build_cmake" mkdir build_cmake
cd build_cmake
cmake ..
cmake --build .
ctest --output-on-failure
cd ..
goto end

:run
call :build
set ARGS=
shift
:run_args
if "%1"=="" goto run_exec
set ARGS=%ARGS% %1
shift
goto run_args
:run_exec
bin\cdd-c.exe %ARGS%
goto end

:build_wasm
if not exist "build_wasm" mkdir build_wasm
cd build_wasm
call emcmake cmake ..
cmake --build .
cd ..
goto end

:build_docker
docker build -t cdd-c:alpine -f alpine.Dockerfile .
docker build -t cdd-c:debian -f debian.Dockerfile .
goto end

:run_docker
call :build_docker
docker run --rm -d -p 8082:8082 --name cdd-c-alpine-test cdd-c:alpine serve_json_rpc --port 8082 --listen 0.0.0.0
timeout /t 2 /nobreak
curl -X POST http://localhost:8082/ -H "Content-Type: application/json" -d "{\"jsonrpc\": \"2.0\", \"method\": \"version\", \"id\": 1}"
docker stop cdd-c-alpine-test
docker run --rm -d -p 8082:8082 --name cdd-c-debian-test cdd-c:debian serve_json_rpc --port 8082 --listen 0.0.0.0
timeout /t 2 /nobreak
curl -X POST http://localhost:8082/ -H "Content-Type: application/json" -d "{\"jsonrpc\": \"2.0\", \"method\": \"version\", \"id\": 1}"
docker stop cdd-c-debian-test
goto end

:end
endlocal
