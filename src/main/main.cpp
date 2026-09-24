/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Main driver for ava6 executable.
 */
#include "main/main.h"

#include <ava6/ava6.h>

#include <iostream>

#include "base/configuration.h"
#include "main/command_executor.h"
#include "options/option_exception.h"

using namespace ava6::internal;
using namespace ava6::main;

/**
 * ava6's main() routine is just an exception-safe wrapper around runAva6.
 */
int main(int argc, char* argv[])
{
  ava6::TermManager tm;
  std::unique_ptr<ava6::Solver> solver = std::make_unique<ava6::Solver>(tm);
  try
  {
    return runAva6(argc, argv, solver);
  }
  catch (ava6::Ava6ApiOptionException& e)
  {
#ifdef AVA6_COMPETITION_MODE
    solver->getDriverOptions().out() << "unknown" << std::endl;
#endif
    std::cerr << "(error \"" << e.getMessage() << "\")" << std::endl
              << std::endl
              << "Please use --help to get help on command-line options."
              << std::endl;
  }
  catch (OptionException& e)
  {
#ifdef AVA6_COMPETITION_MODE
    solver->getDriverOptions().out() << "unknown" << std::endl;
#endif
    std::cerr << "(error \"" << e.getMessage() << "\")" << std::endl
              << std::endl
              << "Please use --help to get help on command-line options."
              << std::endl;
  }
  catch (ava6::Ava6ApiException& e)
  {
#ifdef AVA6_COMPETITION_MODE
    solver->getDriverOptions().out() << "unknown" << std::endl;
#endif
    if (solver->getOption("output-language") == "LANG_SMTLIB_V2_6")
    {
      solver->getDriverOptions().out()
          << "(error \"" << e << "\")" << std::endl;
    }
    else
    {
      solver->getDriverOptions().err()
          << "(error \"" << e << "\")" << std::endl;
    }
    if (solver->getOptionInfo("stats").boolValue() && pExecutor != nullptr)
    {
      pExecutor->printStatistics(solver->getDriverOptions().err());
    }
  }
  // Make sure that the command executor is destroyed before the node manager.
  pExecutor.reset();
  exit(1);
}
