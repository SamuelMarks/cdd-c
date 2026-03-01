@echo off
setlocal ENABLEDELAYEDEXPANSION

set TARGET=%~1

if "%TARGET%"=="" goto help
if "%TARGET%"=="help" goto help
if "%TARGET%"=="all" goto help
if "%TARGET%"=="install_base" goto install_base
if "%TARGET%"=="install_deps" goto install_deps
if "%TARGET%"=="build_docs" goto build_docs
if "%TARGET%"=="build" goto build
if "%TARGET%"=="build_wasm" goto build_wasm
if "%TARGET%"=="test" goto test
if "%TARGET%"=="run" goto run

echo Unknown task: %TARGET%
goto help

:help
echo Available tasks:
echo   install_base : install language runtime and anything else relevant
echo   install_deps : install local dependencies (CMake automatically fetches via vcpkg/FetchContent)
echo   build_docs   : build the API docs and put them in the 'docs' directory. Usage: make build_docs [docs/path]
echo   build        : build the CLI binary. Usage: make build [build/path]
echo   build_wasm   : build the project as a WebAssembly module using Emscripten
echo   test         : run tests locally
echo   run          : run the CLI. If not built, it builds first. Any args after run are passed to CLI.
echo                  Usage: make run --version
goto end

:install_base
echo Please install CMake and Visual Studio (MSVC) or MinGW.
goto end

:install_deps
echo Dependencies are handled natively by CMake during the configure step.
goto end

:build_docs
set DOCS_DIR=%~2
if "%DOCS_DIR%"=="" set DOCS_DIR=docs
if not exist "%DOCS_DIR%" mkdir "%DOCS_DIR%"
echo Generating docs into %DOCS_DIR%...
.\build\bin\Debug\cdd-c.exe to_docs_json --no-imports --no-wrapping -i src\mocks\parse\petstore.openapi.json > "%DOCS_DIR%\docs.json"
if errorlevel 1 (
    .\build\bin\cdd-c.exe to_docs_json --no-imports --no-wrapping -i src\mocks\parse\petstore.openapi.json > "%DOCS_DIR%\docs.json"
)
goto end

:build
set BUILD_DIR=%~2
if "%BUILD_DIR%"=="" set BUILD_DIR=build
cmake -S . -B "%BUILD_DIR%" -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DC_CDD_BUILD_TESTING=ON
cmake --build "%BUILD_DIR%" -j
goto end

:build_wasm
echo Building WebAssembly target using Emscripten...
if not exist "..\emsdk\emsdk_env.bat" (
    echo emsdk not found at ..\emsdk. Please ensure it is installed.
    exit /b 1
)
call ..\emsdk\emsdk_env.bat
call emcmake cmake -S . -B build_wasm -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DC_CDD_BUILD_TESTING=OFF -DHTTP_ONLY=ON -DBUILD_CURL_EXE=OFF -DCMAKE_USE_OPENSSL=OFF -DCURL_USE_OPENSSL=OFF
call emmake cmake --build build_wasm -j
goto end

:test
set BUILD_DIR=%~2
if "%BUILD_DIR%"=="" set BUILD_DIR=build
call :build build "%BUILD_DIR%"
cd "%BUILD_DIR%"
ctest --output-on-failure
cd ..
goto end

:run
set BUILD_DIR=build
call :build build "%BUILD_DIR%"

rem Shift past "run"
shift

set ARGS=
:run_args
if "%~1"=="" goto run_exec
set ARGS=!ARGS! %1
shift
goto run_args

:run_exec
if exist "%BUILD_DIR%\bin\Debug\cdd-c.exe" (
    "%BUILD_DIR%\bin\Debug\cdd-c.exe" !ARGS!
) else (
    "%BUILD_DIR%\bin\cdd-c" !ARGS!
)
goto end

:end
endlocal