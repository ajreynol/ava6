/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * "Ouroborous" test: does ava6 read its own output?
 *
 * The "Ouroborous" test, named after the serpent that swallows its
 * own tail, ensures that ava6 can parse some input, output it again
 * (in any of its languages) and then parse it again.  The result of
 * the first parse must be equal to the result of the second parse;
 * both strings and expressions are compared for equality.
 *
 * To add a new test, simply add a call to runTestString() under
 * runTest(), below.  If you don't specify an input language,
 * LANG_SMTLIB_V2 is used.  If your example depends on symbolic constants,
 * you'll need to declare them in the "declarations" global, just
 * below, in SMT-LIBv2 form (but they're good for all languages).
 */

#include <ava6/ava6.h>
#include <ava6/ava6_parser.h>

#include <cassert>
#include <iostream>
#include <string>

using namespace ava6;
using namespace ava6::internal;
using namespace ava6::parser;

int runTest();

int main()
{
  try
  {
    return runTest();
  }
  catch (ava6::Ava6ApiException& e)
  {
    std::cerr << e.getMessage() << std::endl;
  }
  catch (...)
  {
    std::cerr << "non-ava6 exception thrown" << std::endl;
  }
  return 1;
}

std::string parse(std::string instr,
                  std::string input_language,
                  std::string output_language)
{
  assert(input_language == "smt2");
  assert(output_language == "smt2");

  std::string declarations =
      "(set-logic ALL)\n"
      "(declare-sort U 0)\n"
      "(declare-fun f (U) U)\n"
      "(declare-fun x () U)\n"
      "(declare-fun y () U)\n"
      "(assert (= (f x) x))\n"
      "(declare-fun a () (Array U (Array U U)))\n";

  ava6::TermManager tm;
  ava6::Solver solver(tm);

  modes::InputLanguage ilang = modes::InputLanguage::SMT_LIB_2_6;

  solver.setOption("input-language", input_language);
  solver.setOption("output-language", output_language);
  SymbolManager symman(tm);
  InputParser parser(&solver, &symman);
  std::stringstream ss;
  ss << declarations;
  parser.setStreamInput(ilang, ss, "internal-buffer");
  // we don't need to execute the commands, but we DO need to parse them to
  // get the declarations
  std::stringstream tmp;
  Command c;
  while (true)
  {
    c = parser.nextCommand();
    if (c.isNull())
    {
      break;
    }
    // invoke the command, which may bind symbols
    c.invoke(&solver, &symman, tmp);
  }
  assert(parser.done());  // parser should be done
  std::stringstream ssi;
  ssi << instr;
  parser.setStreamInput(ilang, ss, "internal-buffer");
  ava6::Term e = parser.nextTerm();
  std::string s = e.toString();
  assert(parser.nextTerm().isNull());  // next expr should be null
  return s;
}

std::string translate(std::string instr,
                      std::string input_language,
                      std::string output_language)
{
  assert(input_language == "smt2");
  assert(output_language == "smt2");

  std::cout << "==============================================" << std::endl
            << "translating from " << input_language << " to "
            << output_language << " this string:" << std::endl
            << instr << std::endl;
  std::string outstr = parse(instr, input_language, output_language);
  std::cout << "got this:" << std::endl
            << outstr << std::endl
            << "reparsing as " << output_language << std::endl;
  std::string poutstr = parse(outstr, output_language, output_language);
  assert(outstr == poutstr);
  std::cout << "got same expressions " << outstr << " and " << poutstr
            << std::endl
            << "==============================================" << std::endl;
  return outstr;
}

void runTestString(std::string instr, std::string instr_language)
{
  std::cout << std::endl
            << "starting with: " << instr << std::endl
            << "   in language " << instr_language << std::endl;
  std::string smt2str = translate(instr, instr_language, "smt2");
  std::cout << "in SMT2      : " << smt2str << std::endl;
  std::string outstr = translate(smt2str, "smt2", "smt2");
  std::cout << "to SMT2 : " << outstr << std::endl << std::endl;

  assert(outstr == smt2str);  // differences in output
}

int32_t runTest()
{
  runTestString("(= (f (f y)) x)", "smt2");
  runTestString("(= ((_ extract 2 1) (bvnot (bvadd #b000 #b011))) #b10)",
                "smt2");
  runTestString("((_ extract 2 0) (bvnot (bvadd (bvmul #b001 #b011) #b011)))",
                "smt2");
  return 0;
}
