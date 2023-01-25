set (base_name melatonin.perfetto.add_subdirectory)

set (binary_dir "${CMAKE_CURRENT_BINARY_DIR}/AddSubdirectoryTest")

add_test (NAME "${base_name}.configure"
		  COMMAND "${CMAKE_COMMAND}"
		  			-S "${CMAKE_CURRENT_LIST_DIR}"
		  			-B "${binary_dir}"
		  			-D "MELATONIN_PERFETTO_ROOT=${MelatoninPerfetto_SOURCE_DIR}"
		  			-D "FETCHCONTENT_SOURCE_DIR_JUCE=${juce_SOURCE_DIR}"
		  			-D "FETCHCONTENT_SOURCE_DIR_PERFETTO=${perfetto_SOURCE_DIR}")

set_tests_properties ("${base_name}.configure" 
						PROPERTIES 
							FIXTURES_SETUP MelatoninPerfettoAddSubdirectoryConfigure
							ENVIRONMENT_MODIFICATION "MP_PERFETTO_SHOULD_BE_ON=unset:")

add_test (NAME "${base_name}.build"
		  COMMAND "${CMAKE_COMMAND}"
		  			--build "${binary_dir}"
		  			--config "$<CONFIG>")

set_tests_properties ("${base_name}.build" PROPERTIES FIXTURES_REQUIRED MelatoninPerfettoAddSubdirectoryConfigure)

set_property (TEST "${base_name}.configure" "${base_name}.build"
			  APPEND PROPERTY LABELS add_subdirectory)
