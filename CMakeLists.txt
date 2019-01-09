cmake_minimum_required(VERSION 3.9)

project(hdlc
  VERSION 1.0.0
  LANGUAGES CXX
  )

# Enable/disable testing
option(TESTS_ENABLED "Enable automatic tests" OFF)
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options("-Wall" "-Wextra" "-Werror" "$<$<CONFIG:RELEASE>:-O3>")

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, defaulting to Debug")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
message(${CMAKE_BINARY_DIR})

find_package(fmt)
find_package(boost)
find_package(spdlog)
find_package(catch2)

add_subdirectory(${PROJECT_NAME})
add_subdirectory(vendor/serial)   #serial lib required for the example code.
add_subdirectory(example)
add_subdirectory(test)
