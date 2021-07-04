cmake_minimum_required(VERSION 3.19)
project(leetcode)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(leetcode_test
        src/problems/727.cpp
        src/problems/587.cpp
        src/problems/774.cpp
        src/problems/670.cpp
        src/problems/867.cpp)

target_link_libraries(leetcode_test gtest_main)

include(GoogleTest)
gtest_discover_tests(leetcode_test)

