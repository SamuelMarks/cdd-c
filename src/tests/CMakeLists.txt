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
        "test_analysis.h" 
        "test_c_cdd_integration.h" 
        "test_c_inspector_types.h" 
        "test_code2schema.h" 
        "test_codegen_build.h" 
        "test_codegen_client_body.h"
        "test_codegen_client_sig.h"
        "test_codegen_defaults.h" 
        "test_codegen_eq.h" 
        "test_codegen_json.h"
        "test_codegen_make.h" 
        "test_codegen_security.h"
        "test_codegen_struct.h"
        "test_codegen_types.h"
        "test_codegen_url.h"
        "test_codegen_validation.h" 
        "test_codegen_root_arrays.h" 
        "test_codegen_sdk_tests.h" 
        "test_cst_parser.h" 
        "test_crypto.h"
        "test_dataclasses.h" 
        "test_declarator_parser.h" 
        "test_fs.h" 
        "test_http_curl.h"
        "test_http_types.h"
        "test_http_winhttp.h"
        "test_http_wininet.h"
        "test_json_from_and_to.h" 
        "test_openapi_client_gen.h" 
        "test_openapi_loader.h"
        "test_parsing.h" 
        "test_rewriter_body.h" 
        "test_rewriter_sig.h" 
        "test_refactor_api_sync.h"
        "test_schema2tests.h" 
        "test_schema_codegen.h" 
        "test_schema_constraints.h"
        "test_schema_enum_required.h"
        "test_simple_json.h" 
        "test_str_utils.h"
        "test_sync_code.h" 
        "test_text_patcher.h"
        "test_tokenizer.h" 
        "test_transport_factory.h" 
        "test_url_utils.h"
        # New Tests
        "test_openapi_writer.h"
        "test_doc_parser.h"
        "test_c_mapping.h"
        "test_c2openapi_op.h"
        "test_aggregator.h"
        "test_c2openapi_schema.h"
        "test_integration_c2openapi.h"
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

set(Source_Files "${EXEC_NAME}.c") 
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
set(Header_Files "${EXEC_NAME}.h")
source_group("${EXEC_NAME} Header Files" FILES "${Header_Files}")
set(Source_Files "${EXEC_NAME}.c")
source_group("${EXEC_NAME} Source Files" FILES "${Source_Files}")
add_executable("${EXEC_NAME}" "${Header_Files}" "${Source_Files}")
target_include_directories("${EXEC_NAME}" PRIVATE "$<BUILD_INTERFACE:${DOWNLOAD_DIR}>" "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/mocks>")
set_target_properties("${EXEC_NAME}" PROPERTIES LINKER_LANGUAGE C)
target_link_libraries("${EXEC_NAME}" PUBLIC "simple_mocks")
add_test(NAME "${EXEC_NAME}" COMMAND "${EXEC_NAME}")

set(EXEC_NAME "test_integration_full_sdk")
set(Source_Files "${EXEC_NAME}.c")
add_executable("${EXEC_NAME}" "${Source_Files}")
target_include_directories("${EXEC_NAME}" PRIVATE "$<BUILD_INTERFACE:${DOWNLOAD_DIR}>")
set_target_properties("${EXEC_NAME}" PROPERTIES LINKER_LANGUAGE C)
target_link_libraries("${EXEC_NAME}" PUBLIC "${LIBRARY_NAME}")
add_test(NAME "${EXEC_NAME}" COMMAND "${EXEC_NAME}")
