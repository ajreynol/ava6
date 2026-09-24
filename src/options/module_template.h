/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Contains code for handling command-line options.
 *
 * For each <module>_options.toml configuration file, mkoptions.py
 * expands this template and generates a <module>_options.h file.
 */

#include "ava6_private.h"

// clang-format off
#ifndef AVA6__OPTIONS__${id_cap}$_H
#define AVA6__OPTIONS__${id_cap}$_H

#include "options/options.h"

${includes}$

namespace ava6::internal::options {

namespace ${id}$::longName {
  ${long_name_decl}$
}

${modes_decl}$
  // clang-format on

#if defined(AVA6_MUZZLED) || defined(AVA6_COMPETITION_MODE)
#define DO_SEMANTIC_CHECKS_BY_DEFAULT false
#else /* AVA6_MUZZLED || AVA6_COMPETITION_MODE */
#define DO_SEMANTIC_CHECKS_BY_DEFAULT true
#endif /* AVA6_MUZZLED || AVA6_COMPETITION_MODE */

      // clang-format off
struct Holder${id_cap}$
{
  ${holder_decl}$
};
  // clang-format on

#undef DO_SEMANTIC_CHECKS_BY_DEFAULT

}  // namespace ava6::internal::options

#endif /* AVA6__OPTIONS__${id_cap}$_H */
