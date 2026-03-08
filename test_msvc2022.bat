@echo off
mkdir build_test_msvc2022
cd build_test_msvc2022
call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
cmake -G Ninja -DC_CDD_BUILD_TESTING=ON ..
cmake --build .
ctest -V
