#!/bin/bash
cmake -B build -DBUILD_TESTING=ON -DC_CDD_BUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build
cd build
ctest -T memcheck --overwrite MemoryCheckCommand=/usr/bin/valgrind
