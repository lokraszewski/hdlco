cmake_minimum_required(VERSION 3.9)


project(hdlc
  VERSION 1.0.0
  LANGUAGES CXX
  )

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(CMAKE_VERBOSE_MAKEFILE ON)

# Enable/disable testing
option(TESTS_ENABLED "Enable automatic tests" OFF)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

set(CMAKE_CXX_FLAGS_DEBUG "-Wall -O0 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -O2")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, defaulting to Debug")
    set(CMAKE_BUILD_TYPE "Debug")
endif()


add_subdirectory(${PROJECT_NAME})
add_subdirectory(example)




