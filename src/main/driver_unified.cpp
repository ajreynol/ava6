/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Driver for ava6 executable (ava6).
 */

#include <ava6/ava6.h>
#include <ava6/ava6_parser.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <new>
#include <optional>

#include "base/configuration.h"
#include "base/ava6config.h"
#include "base/output.h"
#include "main/command_executor.h"
#include "main/interactive_shell.h"
#include "main/main.h"
#include "main/options.h"
#include "parser/commands.h"
#include "base/exception.h"
#include "main/signal_handlers.h"
#include "main/time_limit.h"
#include "smt/solver_engine.h"
#include "util/result.h"

using namespace std;
using namespace ava6::internal;
using namespace ava6::parser;
using namespace ava6::main;

namespace ava6::main {

/** Full argv[0] */
const char* progPath;

/** Just the basename component of argv[0] */
std::string progName;

/** A pointer to the CommandExecutor (the signal handlers need it) */
std::unique_ptr<CommandExecutor> pExecutor;

}  // namespace ava6::main

int runAva6(int argc, char* argv[], std::unique_ptr<ava6::Solver>& solver)
{
  // Initialize the signal handlers
  signal_handlers::install();

  progPath = argv[0];

  // Create the command executor to execute the parsed commands
  pExecutor = std::make_unique<CommandExecutor>(solver);
  ava6::DriverOptions dopts = solver->getDriverOptions();

  // Parse the options
  std::vector<string> filenames = parse(*solver, argc, argv, progName);
  if (solver->getOptionInfo("help").boolValue())
  {
    printUsage(progName, dopts.out());
    exit(1);
  }
  else if (solver->getOptionInfo("help-regular").boolValue())
  {
    printUsage(progName, dopts.out(), true);
    exit(1);
  }
  else if (solver->getOptionInfo("help-option-categories").boolValue())
  {
    printUsageCategories(*solver.get(), dopts.out());
    exit(1);
  }
  for (const auto& name :
       {"show-config", "copyright", "version"})
  {
    if (solver->getOptionInfo(name).boolValue())
    {
      std::exit(0);
    }
  }

  auto limit = install_time_limit(solver->getOptionInfo("tlimit").uintValue());
  segvSpin = false;

  // If in competition mode, set output stream option to flush immediately
#ifdef AVA6_COMPETITION_MODE
  dopts.out() << unitbuf;
#endif /* AVA6_COMPETITION_MODE */

  // We only accept one input file
  if (filenames.size() > 1)
  {
    throw Exception("Too many input files specified.");
  }

  // If no file supplied we will read from standard input
  const bool inputFromStdin = filenames.empty() || filenames[0] == "-";

  // If we're reading from stdin, use interactive mode if we are a TTY.
  if (!solver->getOptionInfo("interactive").setByUser)
  {
    pExecutor->setOptionInternal(
        "interactive",
        (inputFromStdin && isatty(fileno(stdin))) ? "true" : "false");
  }

  // Auto-detect input language by filename extension
  std::string filenameStr("<stdin>");
  if (!inputFromStdin)
  {
    filenameStr = std::move(filenames[0]);
  }
  const char* filename = filenameStr.c_str();
  ava6::modes::InputLanguage ilang;
  if (solver->getOption("input-language") == "LANG_AUTO")
  {
    if (inputFromStdin)
    {
      // We can't do any fancy detection on stdin
      pExecutor->setOptionInternal("input-language", "smt2");
    }
    else
    {
      size_t len = filenameStr.size();
      if (len >= 5 && !strcmp(".smt2", filename + len - 5))
      {
        pExecutor->setOptionInternal("input-language", "smt2");
      }

    }
  }
  {
    ilang = ava6::modes::InputLanguage::SMT_LIB_2_6;
  }

  if (solver->getOption("output-language") == "LANG_AUTO")
  {
    pExecutor->setOptionInternal("output-language",
                                 solver->getOption("input-language"));
  }

  // Determine which messages to show based on smtcomp_mode and verbosity
  if (Configuration::isMuzzledBuild())
  {
    TraceChannel.setStream(&ava6::internal::null_os);
    WarningChannel.setStream(&ava6::internal::null_os);
  }

  int returnValue = 0;
  {
    solver->setInfo("filename", filenameStr);

    // Parse and execute commands until we are done
    if (solver->getOptionInfo("interactive").boolValue() && inputFromStdin)
    {
      // We use the interactive shell when piping from stdin, even some cases
      // where the input stream is not a TTY. We do this to avoid memory issues
      // involving tokens that span multiple lines.
      // We compute whether the interactive shell is actually interactive
      // (via isatty). If we are not interactive, we disable certain output
      // information, e.g. for querying the user.
      bool isInteractive = isatty(fileno(stdin));
      // set incremental if we are in interactive mode
      if (!solver->getOptionInfo("incremental").setByUser)
      {
        pExecutor->setOptionInternal("incremental",
                                     isInteractive ? "true" : "false");
      }
      // now store options as original
      pExecutor->storeOptionsAsOriginal();
      InteractiveShell shell(
          pExecutor.get(), dopts.in(), dopts.out(), isInteractive);

      if (isInteractive)
      {
        auto& out = solver->getDriverOptions().out();
        out << Configuration::aboutAndCopyright();
      }

      while (true)
      {
        // read and execute all available commands
        if (!shell.readAndExecCommands())
        {
          break;
        }
      }
    }
    else
    {
      if (!solver->getOptionInfo("incremental").setByUser)
      {
        pExecutor->setOptionInternal("incremental", "false");
      }
      // we don't need to check that terms passed to API methods are well
      // formed, since this should be an invariant of the parser
      
      // now store options as original
      pExecutor->storeOptionsAsOriginal();

      std::unique_ptr<InputParser> parser(new InputParser(
          pExecutor->getSolver(), pExecutor->getSymbolManager()));
      if (inputFromStdin)
      {
        parser->setStreamInput(ilang, cin, filename);
      }
      else
      {
        parser->setFileInput(ilang, filename);
      }

      while (true)
      {
        Command command = parser->nextCommand();
        if (command.isNull()) break;
        bool quit = command.getCommandName() == "exit";
        if (!pExecutor->doCommand(&command))
        {
          returnValue = 1;
          break;
        }
        if (quit) break;
      }
    }

#ifdef AVA6_COMPETITION_MODE
    dopts.out() << std::flush;
    // exit, don't return (don't want destructors to run)
    // _exit() from unistd.h doesn't run global destructors
    // or other on_exit/atexit stuff.
    _exit(returnValue);
#endif /* AVA6_COMPETITION_MODE */

    pExecutor->flushOutputStreams();

#ifdef AVA6_DEBUG
    {
      auto info = solver->getOptionInfo("early-exit");
      if (info.boolValue() && info.setByUser)
      {
        _exit(returnValue);
      }
    }
#else  /* AVA6_DEBUG */
    if (solver->getOptionInfo("early-exit").boolValue())
    {
      _exit(returnValue);
    }
#endif /* AVA6_DEBUG */
  }

  pExecutor.reset();

  signal_handlers::cleanup();

  return returnValue;
}
