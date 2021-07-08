cmake_minimum_required(VERSION 3.19)
project(leetcode)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

# google test
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()

# google benchmark
FetchContent_Declare(benchmark URL https://github.com/google/benchmark/archive/refs/tags/v1.5.5.zip)
FetchContent_MakeAvailable(benchmark)

set(SOURCE_FILES
        src/problems/26.cpp
        src/problems/44.cpp
        src/problems/88.cpp
        src/problems/146.cpp
        src/problems/173.cpp
        src/problems/278.cpp
        src/problems/341.cpp
        src/problems/384.cpp)

# benchmark exe
add_executable(leetcode_bench ${SOURCE_FILES})
target_link_libraries(leetcode_bench gtest benchmark::benchmark_main)

add_executable(leetcode_test ${SOURCE_FILES})
target_link_libraries(leetcode_test gtest_main)

include(GoogleTest)
gtest_discover_tests(leetcode_test)

