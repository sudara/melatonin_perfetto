set (base_name melatonin.perfetto.add_subdirectory)

set (binary_dir "${CMAKE_CURRENT_BINARY_DIR}/AddSubdirectoryTest")

# we don't want to redownload the JUCE source code again in the child CMake
if(juce_SOURCE_DIR)
	set (juce_arg -D "juce_SOURCE_DIR=${juce_SOURCE_DIR}")
elseif(JUCE_DIR)
	set (juce_arg -D "JUCE_DIR=${JUCE_DIR}")
else()
	unset (juce_arg)
endif()

add_test (NAME "${base_name}.configure"
		  COMMAND "${CMAKE_COMMAND}"
		  			-S "${CMAKE_CURRENT_LIST_DIR}"
		  			-B "${binary_dir}"
		  			-D "MELATONIN_PERFETTO_ROOT=${MelatoninPerfetto_SOURCE_DIR}"
		  			${juce_arg})

set_tests_properties ("${base_name}.configure" PROPERTIES FIXTURES_SETUP MelatoninPerfettoAddSubdirectoryConfigure)

add_test (NAME "${base_name}.build"
		  COMMAND "${CMAKE_COMMAND}"
		  			--build "${binary_dir}"
		  			--config "$<CONFIG>")

set_tests_properties ("${base_name}.build" PROPERTIES FIXTURES_REQUIRED MelatoninPerfettoAddSubdirectoryConfigure)

set_property (TEST "${base_name}.configure" "${base_name}.build"
			  APPEND PROPERTY LABELS add_subdirectory)
