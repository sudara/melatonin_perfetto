cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

project(MelatoninPerfetto
        VERSION 1.0.0
        LANGUAGES CXX
        DESCRIPTION "JUCE module for profiling with Perfetto"
        HOMEPAGE_URL "https://github.com/sudara/melatonin_perfetto")

set(missing_juce_error_message "JUCE must be added to your project before melatonin_perfetto!")

include(FetchContent)

if (MelatoninPerfetto_IS_TOP_LEVEL)
    FetchContent_Declare(JUCE
            GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
            GIT_TAG origin/master
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
            FIND_PACKAGE_ARGS 7.0.3)

    FetchContent_MakeAvailable(JUCE)
endif ()

if (NOT COMMAND juce_add_module)
    message(FATAL_ERROR "${missing_juce_error_message}")
endif ()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

include(GNUInstallDirs)

set(MP_INSTALL_DEST "${CMAKE_INSTALL_LIBDIR}/cmake/melatonin_perfetto"
        CACHE STRING
        "Path below the install prefix where melatonin_perfetto package files will be installed to")

FetchContent_Declare(Perfetto
        GIT_REPOSITORY https://android.googlesource.com/platform/external/perfetto
        GIT_TAG v25.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE)

FetchContent_MakeAvailable(Perfetto)

# we need to manually set up a target for Perfetto
add_library(perfetto STATIC)

target_sources(perfetto
        PRIVATE
        "$<BUILD_INTERFACE:${perfetto_SOURCE_DIR}/sdk/perfetto.cc>"
        PUBLIC
        "$<BUILD_INTERFACE:${perfetto_SOURCE_DIR}/sdk/perfetto.h>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/perfetto/perfetto.h>"
        )

target_include_directories(perfetto PUBLIC
        "$<BUILD_INTERFACE:${perfetto_SOURCE_DIR}/sdk>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/perfetto>")

target_compile_definitions(perfetto PUBLIC NOMINMAX=1 WIN32_LEAN_AND_MEAN=1)

set_target_properties(perfetto PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

if (MSVC)
    target_compile_options(perfetto PRIVATE /bigobj)
endif ()

add_library(perfetto::perfetto ALIAS perfetto)

install(FILES "${perfetto_SOURCE_DIR}/sdk/perfetto.h"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/perfetto"
        COMPONENT perfetto)

install(TARGETS perfetto
        EXPORT PerfettoTargets
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT perfetto
        NAMELINK_COMPONENT perfetto
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT perfetto
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT perfetto
        INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/perfetto")

install(EXPORT PerfettoTargets
        NAMESPACE perfetto::
        DESTINATION "${MP_INSTALL_DEST}"
        EXPORT_LINK_INTERFACE_LIBRARIES
        COMPONENT perfetto)

include(CPackComponent)

cpack_add_component(perfetto
        GROUP MelatoninPerfetto
        INSTALL_TYPES Developer
        DISPLAY_NAME "Perfetto"
        DESCRIPTION "The Perfetto profiling library")

juce_add_module("${CMAKE_CURRENT_LIST_DIR}/melatonin_perfetto")

target_link_libraries(melatonin_perfetto INTERFACE perfetto::perfetto)

option(PERFETTO "Enable Perfetto tracing using the melatonin_perfetto module" OFF)

if (PERFETTO)
    target_compile_definitions(melatonin_perfetto INTERFACE PERFETTO=1)
endif ()

add_library(Melatonin::Perfetto ALIAS melatonin_perfetto)

install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/melatonin_perfetto"
        DESTINATION "${MP_INSTALL_DEST}"
        COMPONENT melatonin_perfetto)

cpack_add_component(melatonin_perfetto
        GROUP MelatoninPerfetto
        INSTALL_TYPES Developer
        DISPLAY_NAME "melatonin_perfetto"
        DESCRIPTION "The melatonin_perfetto JUCE module"
        DEPENDS perfetto)

cpack_add_component_group(MelatoninPerfetto
        DISPLAY_NAME "melatonin_perfetto"
        DESCRIPTION "The melatonin_perfetto JUCE module and the Perfetto library itself"
        BOLD_TITLE)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(MelatoninPerfettoConfigVersion.cmake
        VERSION "${MelatoninPerfetto_VERSION}"
        COMPATIBILITY SameMajorVersion)

configure_package_config_file(cmake/package_config.cmake MelatoninPerfettoConfig.cmake
        INSTALL_DESTINATION "${MP_INSTALL_DEST}"
        NO_SET_AND_CHECK_MACRO)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/MelatoninPerfettoConfigVersion.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/MelatoninPerfettoConfig.cmake"
        DESTINATION "${MP_INSTALL_DEST}"
        COMPONENT melatonin_perfetto)

option(MP_TESTS "Build the melatonin_perfetto tests" "${MelatoninPerfetto_IS_TOP_LEVEL}")

if (MP_TESTS)
    enable_testing()

    add_subdirectory(tests)

    if (MelatoninPerfetto_IS_TOP_LEVEL)
        include(CTest)
    endif ()
endif ()
