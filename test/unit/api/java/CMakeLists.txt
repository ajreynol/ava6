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

find_package(Java REQUIRED)
include(UseJava)
find_package(JUnit REQUIRED)

get_target_property(AVA6_JAR_PATH ava6jar JAR_FILE)
get_target_property(AVA6_JNI_LIBDIR ava6jni LIBRARY_OUTPUT_DIRECTORY)

# specify source files for junit tests
set(java_test_src_files
  ${CMAKE_CURRENT_SOURCE_DIR}/CommandTest.java
  ${CMAKE_CURRENT_SOURCE_DIR}/DatatypeTest.java
  ${CMAKE_CURRENT_SOURCE_DIR}/FiniteFieldTest.java
  ${CMAKE_CURRENT_SOURCE_DIR}/GrammarTest.java
  ${CMAKE_CURRENT_SOURCE_DIR}/InputParserTest.java
  ${CMAKE_CURRENT_SOURCE_DIR}/LifetimeTest.java
  ${CMAKE_CURRENT_SOURCE_DIR}/OpTest.java
  ${CMAKE_CURRENT_SOURCE_DIR}/ParserLifetimeTest.java
  ${CMAKE_CURRENT_SOURCE_DIR}/PairTest.java
  ${CMAKE_CURRENT_SOURCE_DIR}/ParserTest.java
  ${CMAKE_CURRENT_SOURCE_DIR}/ResultTest.java
  ${CMAKE_CURRENT_SOURCE_DIR}/SolverTest.java
  ${CMAKE_CURRENT_SOURCE_DIR}/SortTest.java
  ${CMAKE_CURRENT_SOURCE_DIR}/SymbolManagerTest.java
  ${CMAKE_CURRENT_SOURCE_DIR}/SynthResultTest.java
  ${CMAKE_CURRENT_SOURCE_DIR}/TermTest.java
  ${CMAKE_CURRENT_SOURCE_DIR}/TermManagerTest.java
  ${CMAKE_CURRENT_SOURCE_DIR}/TripletTest.java
  ${CMAKE_CURRENT_SOURCE_DIR}/ProofTest.java
  ${CMAKE_CURRENT_SOURCE_DIR}/UtilsTest.java
)

if(WIN32)
  set(PATH_SEP ";")
else()
  set(PATH_SEP ":")
endif()

# build junit tests
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tests/SolverTest.class
  DEPENDS JUnit-EP-jar ava6jar ${java_test_src_files}
  COMMAND
    ${Java_JAVAC_EXECUTABLE} ${java_test_src_files}
    -cp "${JUnit_JAR}${PATH_SEP}${AVA6_JAR_PATH}" # add JUnit and ava6 jar files to the class path
    -d . # specify the output directory for the generated .class files
  COMMENT "Build junit tests"
  VERBATIM
)
add_custom_target(build-junit-tests
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests/SolverTest.class
)
add_dependencies(build-units build-junit-tests)

macro(ava6_add_java_api_test name)
  add_test (NAME unit/api/java/${name}
    COMMAND
      ${ENV_PATH_CMD}
      ${Java_JAVA_EXECUTABLE}
      -Djava.library.path=${AVA6_JNI_LIBDIR}
      -jar ${JUnit_JAR}
      -cp "${JUnit_JAR}${PATH_SEP}${AVA6_JAR_PATH}${PATH_SEP}."
      -select-package tests
      -n tests.${name}
  )
  set_tests_properties(unit/api/java/${name} PROPERTIES LABELS "unit java")
endmacro()

ava6_add_java_api_test(CommandTest)
ava6_add_java_api_test(DatatypeTest)
ava6_add_java_api_test(GrammarTest)
ava6_add_java_api_test(InputParserTest)
ava6_add_java_api_test(LifetimeTest)
ava6_add_java_api_test(ParserLifetimeTest)
ava6_add_java_api_test(OpTest)
ava6_add_java_api_test(PairTest)
ava6_add_java_api_test(ResultTest)
ava6_add_java_api_test(SolverTest)
ava6_add_java_api_test(SortTest)
ava6_add_java_api_test(SymbolManagerTest)
ava6_add_java_api_test(SynthResultTest)
ava6_add_java_api_test(TermTest)
ava6_add_java_api_test(TermManagerTest)
ava6_add_java_api_test(TripletTest)
ava6_add_java_api_test(ProofTest)
ava6_add_java_api_test(UtilsTest)

# suppress deprecated warnings to not error in CI
set_source_files_properties(UncoveredTest.cpp
  PROPERTIES COMPILE_OPTIONS
  "-Wno-deprecated-declarations;-Wno-error=deprecated-declarations")
ava6_add_unit_test_black(UncoveredTest api/java)
