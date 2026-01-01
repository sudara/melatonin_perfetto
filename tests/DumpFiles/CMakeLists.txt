add_executable (MelatoninDumpFileTest)

target_sources (MelatoninDumpFileTest PRIVATE main.cpp)

target_compile_definitions (MelatoninDumpFileTest PRIVATE JUCE_USE_CURL=0)

target_link_libraries (MelatoninDumpFileTest PRIVATE Melatonin::Perfetto)

add_test (NAME melatonin.perfetto.dump_file_creation
		  COMMAND MelatoninDumpFileTest)
