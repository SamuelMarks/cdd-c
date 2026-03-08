call "C:\Program Files (x86)\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86
mkdir build_2005
cd build_2005
cmake -G "NMake Makefiles" -DC_CDD_BUILD_TESTING=ON ..
cmake --build .
ctest -V
cd ..
