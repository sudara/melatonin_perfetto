set (test_name melatonin.perfetto.missing_juce)

add_test (NAME "${test_name}"
		  COMMAND "${CMAKE_COMMAND}"
		  			-S "${CMAKE_CURRENT_LIST_DIR}"
		  			-B "${CMAKE_CURRENT_BINARY_DIR}/MissingJUCETest"
		  			-D "MELATONIN_PERFETTO_ROOT=${MelatoninPerfetto_SOURCE_DIR}"
		  			-D "FETCHCONTENT_SOURCE_DIR_JUCE=${juce_SOURCE_DIR}"
		  			-D "FETCHCONTENT_SOURCE_DIR_PERFETTO=${perfetto_SOURCE_DIR}")

set_property (TEST "${test_name}" APPEND PROPERTY LABELS missing_juce)

set_tests_properties ("${test_name}" PROPERTIES PASS_REGULAR_EXPRESSION "${missing_juce_error_message}")
