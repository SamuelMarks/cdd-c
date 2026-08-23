@echo off
set "FETCH_ARGS="
set FETCH_ARGS=%FETCH_ARGS% -DFETCHCONTENT_SOURCE_DIR_PARSON="/Users/samuel/repos/cdd-openapi-test-harness/cdd-c\..\parson"
set FETCH_ARGS=%FETCH_ARGS% -DFETCHCONTENT_SOURCE_DIR_C_ABSTRACT_HTTP="/Users/samuel/repos/cdd-openapi-test-harness/cdd-c\..\c-abstract-http"
set FETCH_ARGS=%FETCH_ARGS% -DFETCHCONTENT_SOURCE_DIR_C89STRINGUTILS="/Users/samuel/repos/cdd-openapi-test-harness/cdd-c\..\c89stringutils"
set FETCH_ARGS=%FETCH_ARGS% -DFETCHCONTENT_SOURCE_DIR_CDD_C="/Users/samuel/repos/cdd-openapi-test-harness/cdd-c\..\cdd-c"
set FETCH_ARGS=%FETCH_ARGS% -DFETCHCONTENT_SOURCE_DIR_C_STR_SPAN="/Users/samuel/repos/cdd-openapi-test-harness/cdd-c\..\c-str-span"
set FETCH_ARGS=%FETCH_ARGS% -DFETCHCONTENT_SOURCE_DIR_C_ORM="/Users/samuel/repos/cdd-openapi-test-harness/cdd-c\..\c-orm"
cmake -S "/Users/samuel/repos/cdd-openapi-test-harness/cdd-c" -B "/Users/samuel/repos/cdd-openapi-test-harness/cdd-c\build_msvc2005_wine_static" -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=OFF -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=OFF -DBUILD_TESTING=ON -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug %FETCH_ARGS% %*
