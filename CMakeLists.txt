cmake_minimum_required(VERSION 3.9)

project(hdlc
  VERSION 1.0.0
  LANGUAGES CXX
  )

# Check for conan cmake and download if needded.
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
                  "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

# Include the downloaded conan file.
include(${CMAKE_BINARY_DIR}/conan.cmake)

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

conan_cmake_run(
  BASIC_SETUP
  REQUIRES spdlog/1.2.1@bincrafters/stable
  REQUIRES boost/1.68.0@conan/stable
  REQUIRES catch2/2.4.1@bincrafters/stable
  REQUIRES fmt/5.2.1@bincrafters/stable
  CMAKE_TARGETS
  BUILD missing
)

set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
message(${CMAKE_BINARY_DIR})

add_subdirectory(${PROJECT_NAME})

add_subdirectory(vendor/serial) #serial lib required for the example code.
add_subdirectory(example)

add_subdirectory(test)




