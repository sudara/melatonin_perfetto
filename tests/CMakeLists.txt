set_property (DIRECTORY APPEND PROPERTY LABELS melatonin_perfetto)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

include ("${CMAKE_CURRENT_LIST_DIR}/FindPackage/AddTests.cmake")
include ("${CMAKE_CURRENT_LIST_DIR}/AddSubdirectory/AddTests.cmake")
include ("${CMAKE_CURRENT_LIST_DIR}/MissingJUCE/AddTests.cmake")
