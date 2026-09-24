/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Header for main ava6 driver.
 */

#include <ava6/ava6.h>

#include <memory>
#include <string>

#include "base/ava6config.h"

#ifndef AVA6__MAIN__MAIN_H
#define AVA6__MAIN__MAIN_H

namespace ava6::main {

class CommandExecutor;

/** Full argv[0] */
extern const char* progPath;

/** Just the basename component of argv[0] */
extern std::string progName;

/** A reference for use by the signal handlers to print statistics */
extern std::unique_ptr<CommandExecutor> pExecutor;

/**
 * If true, will not spin on segfault even when AVA6_DEBUG is on.
 * Useful for nightly regressions, noninteractive performance runs
 * etc.  See util.cpp.
 */
extern bool segvSpin;

}  // namespace ava6::main

/** Actual ava6 driver functions **/
int runAva6(int argc, char* argv[], std::unique_ptr<ava6::Solver>&);

#endif /* AVA6__MAIN__MAIN_H */
