#[=======================================================================[

get_arch
--------

Function to get the architecture intended to be built for (Windows focus)

]=======================================================================]

macro (get_arch)
    if (NOT DEFINED TARGET_ARCH)
        # Needed by Windows SDK
        if (MSVC)
            find_program(MSVC_CL cl)
            if (MSVC_CL MATCHES "amd64/cl.exe$" OR MSVC_CL MATCHES "x64/cl.exe$")
                set(TARGET_ARCH "AMD64")
            elseif (MSVC_CL MATCHES "arm/cl.exe$" OR MSVC_CL MATCHES "arm64/cl.exe$")
                set(TARGET_ARCH "ARM")
            elseif (MSVC_CL MATCHES "bin/cl.exe$" OR MSVC_CL MATCHES "x86/cl.exe$")
                set(TARGET_ARCH "X86")
            endif ()
        elseif (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "i386" OR
                CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "i686" OR
                DEFINED CMAKE_GENERATOR_PLATFORM AND
                CMAKE_GENERATOR_PLATFORM MATCHES "^[Ww][Ii][Nn]32$")
            set(TARGET_ARCH "x86")
        elseif (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64" OR
                CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "AMD64" OR
                CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "amd64" OR
                DEFINED CMAKE_GENERATOR_PLATFORM AND
                CMAKE_GENERATOR_PLATFORM MATCHES "^[Xx]64$")
            set(TARGET_ARCH "AMD64")
        else ()
            set(TARGET_ARCH "ARM")
        endif (MSVC)
        set(TARGET_ARCH ${CMAKE_HOST_SYSTEM_PROCESSOR})
    endif (NOT DEFINED TARGET_ARCH)
endmacro (get_arch)
