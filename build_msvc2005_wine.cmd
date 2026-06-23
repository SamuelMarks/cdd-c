@echo off
setlocal
set "SRC_DIR=%CD%\"
set "SRC_DIR=%SRC_DIR:~0,-1%"
set "BUILD_TYPE=Debug"

echo Setting up MSVC 2005 Wine environment...

:: MSVC 2005 paths from the user's OS1 mount
set "VSINSTALLDIR=Z:\media\samuel\OS1\Program Files (x86)\Microsoft Visual Studio 8"
set "VCINSTALLDIR=Z:\media\samuel\OS1\Program Files (x86)\Microsoft Visual Studio 8\VC"
set "FrameworkDir=Z:\media\samuel\OS1\WINDOWS\Microsoft.NET\Framework"
set "FrameworkVersion=v2.0.50727"
set "FrameworkSDKDir=Z:\media\samuel\OS1\Program Files (x86)\Microsoft Visual Studio 8\SDK\v2.0"
set "DevEnvDir=Z:\media\samuel\OS1\Program Files (x86)\Microsoft Visual Studio 8\Common7\IDE"

:: CMake from the user's OS1 mount
set "CMAKE_BIN=Z:\media\samuel\OS1\Program Files\CMake\bin"

set "PATH=%DevEnvDir%;%VCINSTALLDIR%\BIN;%VSINSTALLDIR%\Common7\Tools;%VSINSTALLDIR%\Common7\Tools\bin;%VCINSTALLDIR%\PlatformSDK\bin;%FrameworkSDKDir%\bin;%FrameworkDir%\%FrameworkVersion%;%VCINSTALLDIR%\VCPackages;%CMAKE_BIN%;%PATH%"
set "INCLUDE=%VCINSTALLDIR%\ATLMFC\INCLUDE;%VCINSTALLDIR%\INCLUDE;%VCINSTALLDIR%\PlatformSDK\include;%FrameworkSDKDir%\include;%INCLUDE%"
set "LIB=%VCINSTALLDIR%\ATLMFC\LIB;%VCINSTALLDIR%\LIB;%VCINSTALLDIR%\PlatformSDK\lib;%FrameworkSDKDir%\lib;%LIB%"
set "LIBPATH=%FrameworkDir%\%FrameworkVersion%;%VCINSTALLDIR%\ATLMFC\LIB"

echo ======================================================================
echo Win MSVC 2005 Wine ^| Static Lib (MTd) ^| ANSI ^| LTO OFF ^| Multi-thread ^| System ^| RTCs
echo ======================================================================
set "BUILD_DIR=%CD%\build_msvc2005_wine_static"
cmake -S "%SRC_DIR%" -B "%BUILD_DIR%" -G "NMake Makefiles" -DCMAKE_BUILD_TYPE="%BUILD_TYPE%" -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl -DCMAKE_C_FLAGS="/wd4005" -DCMAKE_CXX_FLAGS="/wd4005" -DBUILD_SHARED_LIBS=OFF -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=OFF -DCDD_CHARSET=ANSI -DCDD_THREADING=ON -DCDD_DEPS=SYSTEM -DBUILD_TESTING=ON -DCDD_MSVC_RTC=RTCs -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug %*
if errorlevel 1 exit /b 1
cmake --build "%BUILD_DIR%" --config "%BUILD_TYPE%"
if errorlevel 1 exit /b 1
pushd "%BUILD_DIR%"
echo Copying MSVC 2005 debug redistributables...
copy "%VCINSTALLDIR%\redist\Debug_NonRedist\x86\Microsoft.VC80.DebugCRT\*.*" .
set PATH=%BUILD_DIR%\%BUILD_TYPE%;%BUILD_DIR%\_deps\c89stringutils-build\%BUILD_TYPE%;%BUILD_DIR%\_deps\c_abstract_http-build\%BUILD_TYPE%;%PATH%
ctest -C "%BUILD_TYPE%" --output-on-failure
if errorlevel 1 exit /b 1
popd

echo MSVC 2005 Wine variation completed successfully.
