    #########################
    # Dependencies download #
    #########################

    set(GREATEST_CRC32C "c66c379f")
    set(GREATEST_SHA256 "b3f89ecad2301c64c580f328cc03a74d92f4cd4bf42ab18d3d4bfb7b8be5b948")
    set(DOWNLOAD_DIR "${PROJECT_BINARY_DIR}/test_downloads")

    file(TO_NATIVE_PATH "${DOWNLOAD_DIR}" DOWNLOAD_DIR)
    if (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MSYS AND NOT CYGWIN)
        string(REPLACE "\\" "\\\\" DOWNLOAD_DIR "${DOWNLOAD_DIR}")
    endif (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MSYS AND NOT CYGWIN)

    file(MAKE_DIRECTORY "${DOWNLOAD_DIR}")
    set(GREATEST_BASEFILENAME "greatest.h")
    set(GREATEST_FILE "${DOWNLOAD_DIR}/${GREATEST_BASEFILENAME}")
    file(TO_NATIVE_PATH "${GREATEST_FILE}" GREATEST_FILE)
    if (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MSYS AND NOT CYGWIN)
        string(REPLACE "\\" "\\\\" GREATEST_FILE "${GREATEST_FILE}")
    endif ()

    set(GREATEST_URL "https://raw.githubusercontent.com/SamuelMarks/greatest/cmake-and-msvc/greatest.h")
    file(DOWNLOAD "${GREATEST_URL}" "${GREATEST_FILE}"
            EXPECTED_HASH "SHA256=${GREATEST_SHA256}")

    ##################
    # Top-level test #
    ##################

    set(EXEC_NAME "test_${CMAKE_PROJECT_NAME}")

    set(Source_Files "${CMAKE_PROJECT_NAME}.h")
    source_group("${EXEC_NAME} Source Files" FILES "${Source_Files}")

    add_executable("${EXEC_NAME}" "${Source_Files}")

    target_include_directories(
            "${EXEC_NAME}"
            PRIVATE
            "$<BUILD_INTERFACE:${DOWNLOAD_DIR}>"
    )
    set_target_properties(
            "${EXEC_NAME}"
            PROPERTIES
            LINKER_LANGUAGE
            C
    )
    target_link_libraries("${EXEC_NAME}" PUBLIC "${CMAKE_PROJECT_NAME}")

    add_test(NAME "${EXEC_NAME}" COMMAND "${EXEC_NAME}")

  endif (BUILD_TESTING)
endif()
