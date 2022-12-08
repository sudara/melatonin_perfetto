@PACKAGE_INIT@

find_dependency (JUCE 7.0.3)

include ("${CMAKE_CURRENT_LIST_DIR}/PerfettoTargets.cmake")

set_and_check (module_path "@PACKAGE_MP_INSTALL_DEST@/melatonin_perfetto")

juce_add_module ("${module_path}"
				 ALIAS_NAMESPACE Melatonin)

target_link_libraries (melatonin_perfetto INTERFACE perfetto::perfetto)

option (PERFETTO "Enable Perfetto tracing using the melatonin_perfetto module" OFF)

if(PERFETTO)
	target_compile_definitions (melatonin_perfetto INTERFACE PERFETTO=1)
endif()

check_required_components ("@PROJECT_NAME@")
