/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Interactive shell for ava6.
 */

#ifndef AVA6__INTERACTIVE_SHELL_H
#define AVA6__INTERACTIVE_SHELL_H

#include <ava6/ava6_types.h>

#include <iosfwd>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace ava6 {

class Solver;

namespace parser {
class Command;
class InputParser;
class SymManager;
}  // namespace parser

namespace main {
class CommandExecutor;
}

namespace internal {

class InteractiveShell
{
 public:
  InteractiveShell(main::CommandExecutor* cexec,
                   std::istream& in,
                   std::ostream& out,
                   bool isInteractive = true);

  /**
   * Close out the interactive session.
   */
  ~InteractiveShell();

  /**
   * Read a list of commands from the interactive shell. This will read as
   * many lines as necessary to parse at least one well-formed command,
   * and execute them.
   */
  bool readAndExecCommands();

  /**
   * Return the internal parser being used.
   */
  ava6::parser::InputParser* getParser() { return d_parser.get(); }

 private:
  main::CommandExecutor* d_cexec;
  Solver* d_solver;
  ava6::parser::SymManager* d_symman;
  std::istream& d_in;
  std::ostream& d_out;
  std::unique_ptr<ava6::parser::InputParser> d_parser;
  /** Only true if we are actually asking the user for input */
  bool d_isInteractive;
  bool d_quit;
  bool d_usingEditline;
  /** The language */
  modes::InputLanguage d_lang;

  std::string d_historyFilename;

  static const std::string INPUT_FILENAME;
  static const unsigned s_historyLimit = 500;
}; /* class InteractiveShell */

}  // namespace internal
}  // namespace ava6

#endif /* AVA6__INTERACTIVE_SHELL_H */
