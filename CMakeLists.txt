cmake_minimum_required(VERSION 3.14)
project(diagnostic_manager VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_ROOT "${CMAKE_SOURCE_DIR}")
set(SRC_DIR "${PROJECT_ROOT}/src/source")
set(INC_DIR "${PROJECT_ROOT}/src/include")
set(TEST_DIR "${PROJECT_ROOT}/tests")
set(OUT_DIR "${PROJECT_ROOT}/build/bin")

file(GLOB_RECURSE DM_SOURCES CONFIGURE_DEPENDS "${SRC_DIR}/*.cpp")

message(STATUS "Looking for sources in: ${SRC_DIR}")
message(STATUS "Found sources: ${DM_SOURCES}")

if(NOT DM_SOURCES)
  message(FATAL_ERROR "No source files found under: ${SRC_DIR}")
endif()

add_executable(diagnostic_manager ${DM_SOURCES})
target_include_directories(diagnostic_manager PRIVATE "${INC_DIR}")
set_target_properties(diagnostic_manager PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUT_DIR}")

if(UNIX)
  find_package(Threads REQUIRED)
  target_link_libraries(diagnostic_manager PRIVATE Threads::Threads)
endif()

option(ENABLE_TESTS "Enable unit tests" ON)

if(ENABLE_TESTS)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.14.0
  )
  FetchContent_Populate(googletest)
  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)

  file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "${TEST_DIR}/test_*.cpp")
  if(TEST_SOURCES)
    add_executable(diagnostic_manager_tests ${TEST_SOURCES})
    target_include_directories(diagnostic_manager_tests PRIVATE "${INC_DIR}" "${TEST_DIR}")
    target_link_libraries(diagnostic_manager_tests PRIVATE gtest gtest_main)
    if(UNIX)
      target_link_libraries(diagnostic_manager_tests PRIVATE Threads::Threads)
    endif()
    set_target_properties(diagnostic_manager_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUT_DIR}")
    enable_testing()
    add_test(NAME unit_tests COMMAND diagnostic_manager_tests)
  else()
    message(STATUS "No test sources found under ${TEST_DIR}")
  endif()
endif()

file(GLOB_RECURSE HARNESS_SOURCES CONFIGURE_DEPENDS "${TEST_DIR}/harness_*.cpp")
foreach(hsrc IN LISTS HARNESS_SOURCES)
  get_filename_component(hname ${hsrc} NAME_WE)
  add_executable(${hname} ${hsrc})
  target_include_directories(${hname} PRIVATE "${INC_DIR}" "${TEST_DIR}")
  if(UNIX)
    target_link_libraries(${hname} PRIVATE Threads::Threads)
  endif()
  set_target_properties(${hname} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUT_DIR}")
endforeach()
