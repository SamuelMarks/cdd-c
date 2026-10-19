macro (acquire_google_test)
    include(FetchContent)
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY "https://github.com/google/googletest.git"
            GIT_TAG "master"
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endmacro (acquire_google_test)
