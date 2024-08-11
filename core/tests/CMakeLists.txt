cmake_minimum_required(VERSION 3.14)
project(TestBinomialAmericanOption)

# Use the same C++ standard as the main project
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include GoogleTest using FetchContent
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

include_directories(${CMAKE_SOURCE_DIR}/include)

# Add the test executable
add_executable(test_bopm test_bopm.cpp ${CMAKE_SOURCE_DIR}/src/bopm.cpp)

target_link_libraries(test_bopm GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_bopm)
