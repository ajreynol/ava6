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

#-----------------------------------------------------------------------------#
# libava6parser source files

set(libava6parserapi_src_files
  ../api/cpp/ava6_parser.cpp
)

set(libava6parser_src_files
  commands.cpp
  commands.h
  command_status.cpp
  command_status.h
  input.cpp
  input.h
  lexer.cpp
  lexer.h
  parser.cpp
  parser.h
  parse_op.cpp
  parse_op.h
  parser_state.cpp
  parser_state.h
  parser_utils.cpp
  parser_utils.h
  smt2/smt2_state.cpp
  smt2/smt2_state.h
  smt2/smt2_cmd_parser.cpp
  smt2/smt2_cmd_parser.h
  smt2/smt2_lexer.cpp
  smt2/smt2_lexer.h
  smt2/smt2_cmd_parser.cpp
  smt2/smt2_cmd_parser.h
  smt2/smt2_parser.cpp
  smt2/smt2_parser.h
  smt2/smt2_term_parser.cpp
  smt2/smt2_term_parser.h
  sym_manager.cpp
  sym_manager.h
  symbol_table.cpp
  symbol_table.h
  tokens.cpp
  tokens.h
)

#-----------------------------------------------------------------------------#
# libava6parser configuration

add_library(ava6parser-objs OBJECT ${libava6parser_src_files})
set_target_properties(ava6parser-objs PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(ava6parser-objs PUBLIC -D__BUILDING_AVA6PARSERLIB -Dava6_obj_EXPORTS)

add_library(ava6parserapi-objs OBJECT ${libava6parserapi_src_files})
set_target_properties(ava6parserapi-objs PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(ava6parserapi-objs PUBLIC -D__BUILDING_AVA6LIB -Dava6_obj_EXPORTS)

add_dependencies(ava6parser-objs gen-expr gen-options)
add_dependencies(ava6parserapi-objs ava6parser-objs gen-expr)


add_library(ava6parser
  $<TARGET_OBJECTS:ava6parser-objs>
  $<TARGET_OBJECTS:ava6parserapi-objs>
)
if(BUILD_SHARED_LIBS)
  set_target_properties(ava6parser PROPERTIES
    VERSION ${AVA6_VERSION_NUMBER} SOVERSION ${AVA6_SOVERSION})
endif()

set_target_properties(ava6parser PROPERTIES OUTPUT_NAME ava6parser)
target_link_libraries(ava6parser PRIVATE ava6)

# On Windows, CMake's default install action places
# DLLs into the runtime path (by default "bin")
install(TARGETS ava6parser EXPORT ava6-targets)
