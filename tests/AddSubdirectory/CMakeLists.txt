cmake_minimum_required (VERSION 3.24 FATAL_ERROR)

project (PerfettoTest)

if(NOT DEFINED MELATONIN_PERFETTO_ROOT)
	message (FATAL_ERROR "MELATONIN_PERFETTO_ROOT must be defined!")
endif()

add_subdirectory ("${MELATONIN_PERFETTO_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/melatonin_perfetto")

juce_add_console_app (test VERSION 1.0.0)

target_sources (test PRIVATE main.cpp)

target_link_libraries (test PRIVATE Melatonin::melatonin_perfetto)
