# The unix example requires a serial library to work.
# find_package(serial 1.0 REQUIRED)


add_executable(example example.cpp )

target_link_libraries(example
  ${PROJECT_NAME}
  ${CONAN_LIBS}
  serial
)

set_target_properties(example
  PROPERTIES
    CXX_STANDARD 14
)
