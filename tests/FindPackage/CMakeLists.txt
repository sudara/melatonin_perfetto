cmake_minimum_required (VERSION 3.24 FATAL_ERROR)

project (PerfettoTest)

find_package (MelatoninPerfetto REQUIRED)

juce_add_console_app (test VERSION 1.0.0)

target_sources (test PRIVATE main.cpp)

target_link_libraries (test PRIVATE Melatonin::melatonin_perfetto)
