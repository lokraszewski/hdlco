cmake_minimum_required(VERSION 3.9)

project(hdlc
  VERSION 1.0.0
  LANGUAGES CXX
  )

include(cmake/conan.cmake)

# Enable/disable testing
option(TESTS_ENABLED "Enable automatic tests" OFF)
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options("-Wall" "-Wextra" "-Werror" "$<$<CONFIG:RELEASE>:-O3>")

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, defaulting to Debug")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# message(${CMAKE_BINARY_DIR})

# find_package(fmt)
# find_package(boost)
# find_package(spdlog)
# find_package(catch2)

conan_cmake_run(
  REQUIRES fmt/5.2.1@bincrafters/stable
  REQUIRES spdlog/1.2.1@bincrafters/stable
  REQUIRES catch2/2.4.1@bincrafters/stable
  REQUIRES boost/1.68.0@conan/stable
  BASIC_SETUP
  CMAKE_TARGETS
  BUILD missing
)

add_subdirectory(${PROJECT_NAME})
add_subdirectory(vendor/serial)   #serial lib required for the example code.
add_subdirectory(example)
add_subdirectory(test)
