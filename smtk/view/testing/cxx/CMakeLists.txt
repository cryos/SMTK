add_executable(basicViewWriterTest basicViewWriterTest.cxx)

target_link_libraries(basicViewWriterTest SMTKCore)

add_test(basicViewWriterTest ${EXECUTABLE_OUTPUT_PATH}/basicViewWriterTest)
