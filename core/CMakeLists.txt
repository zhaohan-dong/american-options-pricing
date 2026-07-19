cmake_minimum_required(VERSION 3.10)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


# Version comes from the release tag via -DPROJECT_VERSION=x.y.z; defaults for local builds
if(NOT PROJECT_VERSION)
    set(PROJECT_VERSION "0.0.0")
endif()

# Set the project name and version
project(BinomialAmericanOption VERSION ${PROJECT_VERSION})


# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O2")

include_directories(${CMAKE_SOURCE_DIR}/include)

# Set output file to be in build/out
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)

# Add the executable
add_executable(BinomialAmericanOption src/main.cpp src/bopm.cpp)

target_include_directories(BinomialAmericanOption PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Tests
enable_testing()
add_executable(TestBopm tests/test_bopm.cpp src/bopm.cpp)
target_include_directories(TestBopm PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME bopm_tests COMMAND TestBopm)
