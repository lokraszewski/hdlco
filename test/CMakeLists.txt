add_executable(hdlc_test hdlc_test.cpp )

target_link_libraries(hdlc_test
  ${PROJECT_NAME}
  ${CONAN_LIBS}
)

set_target_properties(hdlc_test
  PROPERTIES
    CXX_STANDARD 17
)
