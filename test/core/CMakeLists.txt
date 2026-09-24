# Small integration suite used by the single debug CI job.
add_executable(core-api-smoke EXCLUDE_FROM_ALL core_api.cpp)
target_link_libraries(core-api-smoke PRIVATE ava6 ava6parser)
add_dependencies(build-tests core-api-smoke)
add_test(NAME core/api COMMAND core-api-smoke)
add_test(NAME core/smt
  COMMAND ${Python_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tools/check_core.py
          $<TARGET_FILE:ava6-bin>)
set_tests_properties(core/api core/smt PROPERTIES LABELS core TIMEOUT 180)
