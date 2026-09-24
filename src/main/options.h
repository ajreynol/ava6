/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Options utilities used in the driver.
 */

#ifndef AVA6__MAIN__OPTIONS_H
#define AVA6__MAIN__OPTIONS_H

#include <ava6/ava6.h>

#include <iosfwd>
#include <string>
#include <vector>

namespace ava6::main {

/**
 * Print overall command-line option usage message to the given output stream
 * with binary being the command to run ava6.
 */
void printUsage(const std::string& binary,
                std::ostream& os,
                bool printRegular = false);

/**
 * Print overall command-line options, partitioned into categories.
 */
void printUsageCategories(ava6::Solver& solver, std::ostream& os);

/**
 * Initialize the Options object options based on the given
 * command-line arguments given in argc and argv.  The return value
 * is what's left of the command line (that is, the non-option
 * arguments).
 *
 * This function uses getopt_long() and is not thread safe.
 *
 * Throws OptionException on failures.
 *
 * Preconditions: options and argv must be non-null.
 */
std::vector<std::string> parse(ava6::Solver& solver,
                               int argc,
                               char* argv[],
                               std::string& binaryName);

}  // namespace ava6::main

#endif
