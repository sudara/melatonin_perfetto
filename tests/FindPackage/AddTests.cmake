set (base_name melatonin.perfetto.find_package)

set (prefix "${CMAKE_CURRENT_BINARY_DIR}/deploy/$<CONFIG>")

add_test (NAME "${base_name}.install"
		  COMMAND "${CMAKE_COMMAND}" 
		  			--install "${MelatoninPerfetto_BINARY_DIR}"
		  			--config "$<CONFIG>"
		  			--prefix "${prefix}")

set_tests_properties ("${base_name}.install" PROPERTIES FIXTURES_SETUP MelatoninPerfettoFindPackageInstall)

set (binary_dir "${CMAKE_CURRENT_BINARY_DIR}/FindPackageTest")

add_test (NAME "${base_name}.configure"
		  COMMAND "${CMAKE_COMMAND}"
		  			-S "${CMAKE_CURRENT_LIST_DIR}"
		  			-B "${binary_dir}"
		  			-G "${CMAKE_GENERATOR}"
		  			-D "MelatoninPerfetto_DIR=${prefix}/${CMAKE_INSTALL_LIBDIR}/cmake/melatonin_perfetto"
		  			-D "FETCHCONTENT_SOURCE_DIR_JUCE=${juce_SOURCE_DIR}"
		  			-D "FETCHCONTENT_SOURCE_DIR_PERFETTO=${perfetto_SOURCE_DIR}")

set_tests_properties ("${base_name}.configure" PROPERTIES
					  FIXTURES_SETUP MelatoninPerfettoFindPackageConfigure
					  FIXTURES_REQUIRED MelatoninPerfettoFindPackageInstall
					  ENVIRONMENT_MODIFICATION "MP_PERFETTO_SHOULD_BE_ON=unset:")

add_test (NAME "${base_name}.build"
		  COMMAND "${CMAKE_COMMAND}" 
		  			--build "${binary_dir}"
		  			--config "$<CONFIG>")

set_tests_properties ("${base_name}.build" PROPERTIES FIXTURES_REQUIRED MelatoninPerfettoFindPackageConfigure)

set_property (TEST "${base_name}.install" "${base_name}.configure" "${base_name}.build"
			  APPEND PROPERTY LABELS find_package)
