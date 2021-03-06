set(ioTests
  loggerTest
  ResourceSetTest
  unitImportExportJSON
)

foreach (test ${ioTests})
  add_executable(${test}  ${test}.cxx)
  target_link_libraries(${test} SMTKCore)
  add_test(${test} ${EXECUTABLE_OUTPUT_PATH}/${test})
endforeach()


# ResourceSetWriterTest uses input files in SMTKTestData
if (SMTK_DATA_DIR)
  set(writer_test ResourceSetWriterTest)
  add_executable(${writer_test}  ${writer_test}.cxx)
  target_link_libraries(${writer_test} SMTKCore ${Boost_LIBRARIES})
  add_test(${writer_test}
    ${EXECUTABLE_OUTPUT_PATH}/${writer_test}
    ${SMTK_DATA_DIR}/smtk/attribute/Basic2DFluid.sbt
    ${SMTK_DATA_DIR}/smtk/attribute/copyAttributeTest.sbi
    writer_output.crf
  )

  set(reader_test ResourceSetReaderTest)
  add_executable(${reader_test}  ${reader_test}.cxx)
  target_link_libraries(${reader_test} SMTKCore ${Boost_LIBRARIES})
  add_test(${reader_test}
    ${EXECUTABLE_OUTPUT_PATH}/${reader_test}
    ${SMTK_DATA_DIR}/smtk/attribute/resourceTest/resources.xml
    2
  )
endif()
