###############################################################################
# This file is part of the cvc5 project.
#
# Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
# in the top-level source directory and their institutional affiliations.
# All rights reserved.  See the file COPYING in the top-level source
# directory for licensing information.
# #############################################################################
#
# The build system configuration.
##

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/src/include)

#-----------------------------------------------------------------------------#
# Add target 'apitests', builds and runs
# > api tests

add_custom_target(build-cppapitests)
add_dependencies(build-apitests build-cppapitests)

# Disabled temporarily due to problems in the nightly builds
# # See the comment about 'make test' in test/regress/cli/CMakeLists.txt
# add_test(build_apitests_test "${CMAKE_COMMAND}" --build "${PROJECT_BINARY_DIR}" --config "$<CONFIG>" --target build-apitests)
# set_tests_properties(build_apitests_test PROPERTIES FIXTURES_SETUP build_apitests_fixture)

add_custom_target(cppapitests
  COMMAND ctest --output-on-failure -L "cppapi" -j${CTEST_NTHREADS} $$ARGS
  DEPENDS build-cppapitests)

set(AVA6_API_TEST_FLAGS -D__BUILDING_AVA6_API_TEST -Dava6_obj_EXPORTS)

macro(ava6_add_api_test name dir)
  set(test_bin_dir ${PROJECT_BINARY_DIR}/bin/test/api/cpp/${dir})
  add_executable(${name} ${name}.cpp)
  target_link_libraries(${name} PUBLIC main-test)
  target_compile_definitions(${name} PRIVATE ${AVA6_API_TEST_FLAGS})
  set_target_properties(${name}
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${test_bin_dir})
  if("${dir}" STREQUAL "")
    set(test_name api/cpp/${name})
  else()
    set(test_name api/cpp/${dir}/${name})
  endif()
  add_test(${test_name} ${ENV_PATH_CMD} ${test_bin_dir}/${name})
  set_tests_properties(${test_name} PROPERTIES LABELS "api cppapi")
  # set_tests_properties(api/cpp/${name} PROPERTIES FIXTURES_REQUIRED build_apitests_fixture)
  add_dependencies(build-cppapitests ${name})
endmacro()

ava6_add_api_test(boilerplate "")
ava6_add_api_test(ouroborous "")
ava6_add_api_test(reset_assertions "")
ava6_add_api_test(smt2_compliance "")
ava6_add_api_test(two_solvers "")



add_subdirectory(issues)
