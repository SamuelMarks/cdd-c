cmake_minimum_required(VERSION 3.15...4.0)

if(NOT DEFINED TRANSFORM_BIN OR NOT EXISTS "${TRANSFORM_BIN}")
    message(FATAL_ERROR "TRANSFORM_BIN not found: ${TRANSFORM_BIN}")
endif()
if(NOT DEFINED TORTURE_INPUT OR NOT EXISTS "${TORTURE_INPUT}")
    message(FATAL_ERROR "TORTURE_INPUT not found: ${TORTURE_INPUT}")
endif()

file(MAKE_DIRECTORY "${WORK_DIR}")
set(_in_c "${WORK_DIR}/torture_input.c")
configure_file("${TORTURE_INPUT}" "${_in_c}" COPYONLY)

set(_cmd_prefix "")
if(DEFINED EMULATOR AND NOT "${EMULATOR}" STREQUAL "")
    set(_cmd_prefix "${EMULATOR}")
endif()

foreach(_mode IN ITEMS legacy modern iso-c89)
    set(_out_c "${WORK_DIR}/torture_${_mode}.c")
    execute_process(
        COMMAND ${_cmd_prefix} "${TRANSFORM_BIN}" transform
                --passes=gnu,c_lowering,msvc_port
                "--target-msvc=${_mode}"
                --vla-mode=malloc
                "${_in_c}" -o "${_out_c}"
        WORKING_DIRECTORY "${WORK_DIR}"
        RESULT_VARIABLE _tf_rc
        OUTPUT_VARIABLE _tf_out
        ERROR_VARIABLE _tf_err
    )
    if(NOT _tf_rc EQUAL 0)
        message(FATAL_ERROR "cdd-c-transform (${_mode}) failed (${_tf_rc}):
${_tf_out}
${_tf_err}")
    endif()
endforeach()

if(DEFINED C_COMPILER AND NOT "${C_COMPILER}" STREQUAL "")
    if(COMPILER_ID MATCHES "Clang|GNU")
        set(_exe "${WORK_DIR}/torture_iso89_bin")
        if(TARGET_WIN32)
            set(_exe "${WORK_DIR}/torture_iso89_bin.exe")
        endif()
        execute_process(
            COMMAND "${C_COMPILER}" -std=c89 -Wall -Wextra -pedantic -Werror
                    "-I${INCLUDE_DIR}" "${WORK_DIR}/torture_iso-c89.c" -o "${_exe}"
            WORKING_DIRECTORY "${WORK_DIR}"
            RESULT_VARIABLE _cc_rc
            OUTPUT_VARIABLE _cc_out
            ERROR_VARIABLE _cc_err
        )
        if(NOT _cc_rc EQUAL 0)
            message(FATAL_ERROR "Strict C89 compilation failed (${_cc_rc}):
${_cc_out}
${_cc_err}")
        endif()
        if(NOT TARGET_WIN32 OR NOT "${_cmd_prefix}" STREQUAL "")
            execute_process(
                COMMAND ${_cmd_prefix} "${_exe}"
                WORKING_DIRECTORY "${WORK_DIR}"
                RESULT_VARIABLE _run_rc
            )
            if(NOT _run_rc EQUAL 0)
                message(FATAL_ERROR "Torture binary exited with non-zero code: ${_run_rc}")
            endif()
        endif()
    elseif(COMPILER_ID STREQUAL "MSVC")
        execute_process(
            COMMAND "${C_COMPILER}" /nologo /W4 /WX "-I${INCLUDE_DIR}"
                    torture_legacy.c /Fetorture_legacy.exe
            WORKING_DIRECTORY "${WORK_DIR}"
            RESULT_VARIABLE _cl_leg_rc
            OUTPUT_VARIABLE _cl_leg_out
            ERROR_VARIABLE _cl_leg_err
        )
        if(NOT _cl_leg_rc EQUAL 0)
            message(FATAL_ERROR "MSVC legacy /W4 /WX compile failed:
${_cl_leg_out}
${_cl_leg_err}")
        endif()
        execute_process(
            COMMAND ${_cmd_prefix} "${WORK_DIR}/torture_legacy.exe"
            WORKING_DIRECTORY "${WORK_DIR}"
            RESULT_VARIABLE _run_leg_rc
        )
        if(NOT _run_leg_rc EQUAL 0)
            message(FATAL_ERROR "MSVC legacy torture binary failed: ${_run_leg_rc}")
        endif()

        foreach(_std_flag IN ITEMS "/std:c11" "/std:c17")
            string(REPLACE "/std:" "" _std_tag "${_std_flag}")
            execute_process(
                COMMAND "${C_COMPILER}" /nologo "${_std_flag}" /W4 /WX "-I${INCLUDE_DIR}"
                        torture_modern.c "/Fetorture_modern_${_std_tag}.exe"
                WORKING_DIRECTORY "${WORK_DIR}"
                RESULT_VARIABLE _cl_mod_rc
                OUTPUT_VARIABLE _cl_mod_out
                ERROR_VARIABLE _cl_mod_err
            )
            if(NOT _cl_mod_rc EQUAL 0)
                message(FATAL_ERROR "MSVC ${_std_flag} /W4 /WX compile failed:
${_cl_mod_out}
${_cl_mod_err}")
            endif()
            execute_process(
                COMMAND ${_cmd_prefix} "${WORK_DIR}/torture_modern_${_std_tag}.exe"
                WORKING_DIRECTORY "${WORK_DIR}"
                RESULT_VARIABLE _run_mod_rc
            )
            if(NOT _run_mod_rc EQUAL 0)
                message(FATAL_ERROR "MSVC ${_std_flag} torture binary failed: ${_run_mod_rc}")
            endif()
        endforeach()
    endif()
endif()
