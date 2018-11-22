add_executable(hdlc_test hdlc_test.cpp )

target_include_directories(hdlc_test
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(hdlc_test
  ${PROJECT_NAME}
  ${CONAN_LIBS}
)

set_target_properties(hdlc_test
  PROPERTIES
    CXX_STANDARD 14
)
