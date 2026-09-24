/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * A test of SMT-LIBv2 commands, checks for compliant output.
 */

#include <ava6/ava6.h>
#include <ava6/ava6_parser.h>

#include <cassert>
#include <iostream>
#include <sstream>

using namespace ava6;
using namespace ava6::internal;
using namespace ava6::parser;
using namespace std;

void testGetInfo(ava6::Solver* solver, const char* s);

void testGetInfo(ava6::Solver& solver, const char* s)
{
  SymbolManager sm(solver.getTermManager());
  InputParser p(&solver, &sm);
  std::stringstream ssi;
  ssi << "(get-info " << s << ")";
  p.setStreamInput(modes::InputLanguage::SMT_LIB_2_6, ssi, "<internal>");
  Command c = p.nextCommand();
  assert(!c.isNull());
  std::cout << c << std::endl;
  std::stringstream ss;
  c.invoke(&solver, &sm, ss);
  c = p.nextCommand();
  assert(c.isNull());
  std::cout << ss.str();
}

int main()
{
  ava6::TermManager tm;
  ava6::Solver solver(tm);
  solver.setOption("input-language", "smtlib2");
  solver.setOption("output-language", "smtlib2");
  testGetInfo(solver, ":error-behavior");
  testGetInfo(solver, ":name");
  testGetInfo(solver, ":authors");
  testGetInfo(solver, ":version");
  testGetInfo(solver, ":status");
  testGetInfo(solver, ":reason-unknown");
  testGetInfo(solver, ":arbitrary-undefined-keyword");
  testGetInfo(solver, ":<=");  // legal
  testGetInfo(solver, ":->");  // legal
  testGetInfo(solver, ":all-statistics");

  return 0;
}
