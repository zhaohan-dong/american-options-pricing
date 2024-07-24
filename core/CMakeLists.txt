cmake_minimum_required(VERSION 3.10)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


# Read project version from ./VERSION
function(read_version_from_file VERSION_FILE VERSION_VAR)
    if(EXISTS ${VERSION_FILE})
        file(READ ${VERSION_FILE} VERSION_CONTENTS)
        string(STRIP ${VERSION_CONTENTS} VERSION_CONTENTS)
        set(${VERSION_VAR} ${VERSION_CONTENTS} PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Version file ${VERSION_FILE} not found.")
    endif()
endfunction()
set(VERSION_FILE "${CMAKE_SOURCE_DIR}/../VERSION")
read_version_from_file(${VERSION_FILE} PROJECT_VERSION)

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
