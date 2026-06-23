call "..\c-ci\vcvarsalls_wine.cmd"
cd build_msvc2005_wine_static
nmake clean
nmake
