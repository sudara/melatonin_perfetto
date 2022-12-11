@PACKAGE_INIT@

include (CMakeFindDependencyMacro)

find_dependency (JUCE)

include ("${CMAKE_CURRENT_LIST_DIR}/PerfettoTargets.cmake")

juce_add_module ("${CMAKE_CURRENT_LIST_DIR}/melatonin_perfetto"
				 ALIAS_NAMESPACE Melatonin)

target_link_libraries (melatonin_perfetto INTERFACE perfetto::perfetto)

option (PERFETTO "Enable Perfetto tracing using the melatonin_perfetto module" OFF)

if(PERFETTO)
	target_compile_definitions (melatonin_perfetto INTERFACE PERFETTO=1)
endif()

check_required_components ("@PROJECT_NAME@")
