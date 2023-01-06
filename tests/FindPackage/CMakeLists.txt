cmake_minimum_required (VERSION 3.24 FATAL_ERROR)

project (PerfettoTest)

include (FetchContent)

FetchContent_Declare (JUCE 
					  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
					  GIT_TAG origin/master
					  GIT_SHALLOW TRUE
					  GIT_PROGRESS TRUE
					  FIND_PACKAGE_ARGS 7.0.3)

FetchContent_MakeAvailable (JUCE)

find_package (MelatoninPerfetto REQUIRED)

juce_add_console_app (test VERSION 1.0.0)

target_sources (test PRIVATE main.cpp)

target_link_libraries (test PRIVATE Melatonin::Perfetto)
