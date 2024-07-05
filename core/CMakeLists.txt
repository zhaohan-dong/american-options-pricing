cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(BinomialAmericanOption VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set output file to be in build/out
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O2")

include_directories(${CMAKE_SOURCE_DIR}/include)

# Add the executable
add_executable(BinomialAmericanOption src/main.cpp src/bopm.cpp)

target_include_directories(BinomialAmericanOption PRIVATE ${CMAKE_SOURCE_DIR}/include)