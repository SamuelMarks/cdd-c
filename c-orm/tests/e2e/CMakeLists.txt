cmake_minimum_required(VERSION 3.10)

# Generate using our own SQL to C codegen inside cdd-c (which we'll do manually for now as testing strategy)
# Wait, cdd-c isn't linked as a command here yet if c-orm is separate. Let's just assume we pre-generate it for this E2E test.
# Actually, the user asked for: 101. Integrate cdd-c as a custom CMake command to generate models dynamically before c-orm compilation.

# But wait, cdd-c is not built here. We can reference it from the parent project.
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Models.h ${CMAKE_CURRENT_BINARY_DIR}/Models.c
    COMMAND "${CMAKE_SOURCE_DIR}/../build_msvc2026_static_vcpkg/Debug/cdd-c.exe" sql2c "${CMAKE_CURRENT_SOURCE_DIR}/schema.sql" "${CMAKE_CURRENT_BINARY_DIR}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/schema.sql"
    COMMENT "Generating C Models from schema.sql using cdd-c..."
)

add_executable(c_orm_e2e_tests 
    test_e2e.c
    ${CMAKE_CURRENT_BINARY_DIR}/Models.c
)

target_include_directories(c_orm_e2e_tests PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/../build_msvc2026_static_vcpkg/test_downloads # For greatest.h
)

target_link_libraries(c_orm_e2e_tests PRIVATE c-orm)

add_test(NAME e2e_test COMMAND c_orm_e2e_tests)
