set_property (DIRECTORY APPEND PROPERTY LABELS melatonin_perfetto)

include ("${CMAKE_CURRENT_LIST_DIR}/FindPackage/AddTests.cmake")
include ("${CMAKE_CURRENT_LIST_DIR}/AddSubdirectory/AddTests.cmake")
include ("${CMAKE_CURRENT_LIST_DIR}/MissingJUCE/AddTests.cmake")

add_subdirectory (PerfettoValue)
