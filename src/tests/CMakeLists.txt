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

################ 
# Test library # 
################ 

set(LIBRARY_NAME "test_c_cdd_lib") 

set(Header_Files
        "parse/test_analysis.h" 
        "parse/test_c_cdd_integration.h" 
        "parse/test_c_inspector_types.h" 
        "parse/test_code2schema.h" 
        "emit/test_codegen_build.h" 
        "emit/test_generate_build_system.h" 
        "emit/test_codegen_client_body.h"
        "emit/test_codegen_client_sig.h"
        "emit/test_codegen_defaults.h" 
        "emit/test_codegen_eq.h" 
        "emit/test_codegen_json.h"
        "emit/test_codegen_make.h" 
        "emit/test_codegen_security.h"
        "emit/test_codegen_struct.h"
        "emit/test_codegen_types.h"
        "emit/test_codegen_url.h"
        "emit/test_codegen_validation.h" 
        "emit/test_codegen_root_arrays.h" 
        "emit/test_codegen_sdk_tests.h" 
        "parse/test_cst_parser.h" 
        "parse/test_crypto.h"
        "parse/test_dataclasses.h" 
        "parse/test_declarator_parser.h" 
        "parse/test_fs.h" 
        "emit/test_http_curl.h"
        "emit/test_http_types.h"
        "emit/test_http_winhttp.h"
        "emit/test_http_wininet.h"
        "parse/test_json_from_and_to.h" 
        "emit/test_openapi_client_gen.h" 
        "parse/test_openapi_loader.h"
        "parse/test_parsing.h" 
        "emit/test_rewriter_body.h" 
        "emit/test_rewriter_sig.h" 
        "parse/test_refactor_api_sync.h"
        "emit/test_schema2tests.h" 
        "emit/test_schema_codegen.h" 
        "parse/test_schema_constraints.h"
        "parse/test_schema_enum_required.h"
        "parse/test_simple_json.h" 
        "parse/test_str_utils.h"
        "emit/test_sync_code.h" 
        "emit/test_text_patcher.h"
        "parse/test_tokenizer.h" 
        "parse/test_transport_factory.h" 
        "emit/test_url_utils.h"
        # New Tests
        "emit/test_openapi_writer.h"
        "parse/test_doc_parser.h"
        "parse/test_c_mapping.h"
        "parse/test_c2openapi_op.h"
        "emit/test_aggregator.h"
        "parse/test_c2openapi_schema.h"
        "parse/test_integration_c2openapi.h"
) 
source_group("${LIBRARY_NAME} Header Files" FILES "${Header_Files}") 

add_library("${LIBRARY_NAME}" INTERFACE) 
add_custom_target("${LIBRARY_NAME}-target" SOURCES "${Header_Files}") 
set_property(TARGET "${LIBRARY_NAME}-target" PROPERTY FOLDER "${CMAKE_CURRENT_SOURCE_DIR}") 

target_include_directories( 
        "${LIBRARY_NAME}" 
        INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>" 
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>" 
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/mocks>" 
        "$<BUILD_INTERFACE:${DOWNLOAD_DIR}>" 
) 
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.13") 
    set_target_properties( 
            "${LIBRARY_NAME}" 
            PROPERTIES
            LINKER_LANGUAGE
            C
    ) 
endif () 

add_subdirectory("cdd_test_helpers") 
target_link_libraries("${LIBRARY_NAME}" INTERFACE "cdd_test_helpers") 

################## 
# Top-level test # 
################## 

set(EXEC_NAME "test_c_cdd") 

set(Source_Files "test_c_cdd.c") 
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
target_link_libraries("${EXEC_NAME}" PUBLIC "${LIBRARY_NAME}") 

add_test(NAME "${EXEC_NAME}" COMMAND "${EXEC_NAME}") 

# ... (Previous Mock targets remain unchanged) ...
add_subdirectory("mocks") 
target_link_libraries("${LIBRARY_NAME}" INTERFACE "simple_mocks") 
# ...
set(EXEC_NAME "test_simple_json")
set(Header_Files "parse/${EXEC_NAME}.h")
source_group("${EXEC_NAME} Header Files" FILES "${Header_Files}")
set(Source_Files "parse/${EXEC_NAME}.c")
source_group("${EXEC_NAME} Source Files" FILES "${Source_Files}")
add_executable("${EXEC_NAME}" "${Header_Files}" "${Source_Files}")
target_include_directories("${EXEC_NAME}" PRIVATE "$<BUILD_INTERFACE:${DOWNLOAD_DIR}>" "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/mocks>")
set_target_properties("${EXEC_NAME}" PROPERTIES LINKER_LANGUAGE C)
target_link_libraries("${EXEC_NAME}" PUBLIC "simple_mocks")
add_test(NAME "${EXEC_NAME}" COMMAND "${EXEC_NAME}")

set(EXEC_NAME "test_integration_full_sdk")
set(Source_Files "emit/${EXEC_NAME}.c")
add_executable("${EXEC_NAME}" "${Source_Files}")
target_include_directories("${EXEC_NAME}" PRIVATE "$<BUILD_INTERFACE:${DOWNLOAD_DIR}>")
set_target_properties("${EXEC_NAME}" PROPERTIES LINKER_LANGUAGE C)
target_link_libraries("${EXEC_NAME}" PUBLIC "${LIBRARY_NAME}")
add_test(NAME "${EXEC_NAME}" COMMAND "${EXEC_NAME}")
