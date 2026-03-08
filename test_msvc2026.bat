call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
cmake -G Ninja -DC_CDD_BUILD_TESTING=ON ..
cmake --build .
ctest -V
