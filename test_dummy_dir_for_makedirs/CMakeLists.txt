cmake_minimum_required(VERSION 3.10)

project(MyLib C)

include(CTest)
if(BUILD_TESTING)
  enable_testing()
endif()

add_subdirectory(src)
