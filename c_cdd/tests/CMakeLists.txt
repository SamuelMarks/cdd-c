# Get test runner dependency
include(FindGTest)
find_package(GTest QUIET COMPONENTS "gtest_main")
if (GTest_FOUND)
    set(gtest_lib "GTest::gtest_main")
else ()
    include("${CMAKE_SOURCE_DIR}/cmake/modules/AcquireGoogleTest.cmake")
    acquire_google_test()
    set(gtest_lib "gtest_main")
endif ()

##################
# Top-level test #
##################

set(EXEC_NAME "test_c_cdd")

set(Source_Files "${EXEC_NAME}.cpp")
source_group("${EXEC_NAME} Source Files" FILES "${Source_Files}")

add_executable("${EXEC_NAME}" "${Source_Files}")
set_target_properties(
        "${EXEC_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        CXX
)
target_link_libraries("${EXEC_NAME}" PUBLIC "${gtest_lib}" "${PROJECT_NAME}")

add_test(NAME "${EXEC_NAME}" COMMAND "${EXEC_NAME}")
