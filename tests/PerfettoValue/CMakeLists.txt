add_library (PerfettoValueTest STATIC EXCLUDE_FROM_ALL)

if(PERFETTO)
	target_sources (PerfettoValueTest PRIVATE should_be_on.cpp)
else()
	target_sources (PerfettoValueTest PRIVATE should_be_off.cpp)
endif()

target_link_libraries (PerfettoValueTest PRIVATE Melatonin::Perfetto)

add_test (NAME melatonin.perfetto.PERFETTO_symbol_value
		  COMMAND "${CMAKE_COMMAND}" 
		  			--build "${CMAKE_BINARY_DIR}" 
		  			--target PerfettoValueTest
		  			--config "$<CONFIG>")
