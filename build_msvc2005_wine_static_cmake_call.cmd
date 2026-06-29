@echo off
set "FETCH_ARGS="
set FETCH_ARGS=%FETCH_ARGS% -DFETCHCONTENT_SOURCE_DIR_PARSON="Z:\home\samuel\repos\cdd-c\..\parson"
set FETCH_ARGS=%FETCH_ARGS% -DFETCHCONTENT_SOURCE_DIR_C_ABSTRACT_HTTP="Z:\home\samuel\repos\cdd-c\..\c-abstract-http"
set FETCH_ARGS=%FETCH_ARGS% -DFETCHCONTENT_SOURCE_DIR_C89STRINGUTILS="Z:\home\samuel\repos\cdd-c\..\c89stringutils"
set FETCH_ARGS=%FETCH_ARGS% -DFETCHCONTENT_SOURCE_DIR_CDD_C="Z:\home\samuel\repos\cdd-c\..\cdd-c"
set FETCH_ARGS=%FETCH_ARGS% -DFETCHCONTENT_SOURCE_DIR_C_STR_SPAN="Z:\home\samuel\repos\cdd-c\..\c-str-span"
set FETCH_ARGS=%FETCH_ARGS% -DFETCHCONTENT_SOURCE_DIR_C_ORM="Z:\home\samuel\repos\cdd-c\..\c-orm"
cmake -S "Z:\home\samuel\repos\cdd-c" -B "Z:\home\samuel\repos\cdd-c\build_msvc2005_wine_static" -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=OFF -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=OFF -DBUILD_TESTING=ON -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug %FETCH_ARGS% %*
