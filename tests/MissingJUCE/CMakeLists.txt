cmake_minimum_required (VERSION 3.24 FATAL_ERROR)

project (PerfettoTest)

if(NOT DEFINED MELATONIN_PERFETTO_ROOT)
	message (FATAL_ERROR "MELATONIN_PERFETTO_ROOT must be defined!")
endif()

add_subdirectory ("${MELATONIN_PERFETTO_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/melatonin_perfetto")
