/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Replacement for ffs() for systems without it (like Win32).
 */

#include "ava6_private.h"

#ifndef AVA6__LIB__FFS_H
#define AVA6__LIB__FFS_H

// We include this for HAVE_FFS
#include "base/ava6config.h"

#ifdef HAVE_FFS

// available in strings.h
#include <strings.h>

#else /* ! HAVE_FFS */

#include "lib/replacements.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int ffs(int i);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* HAVE_FFS */
#endif /* AVA6__LIB__FFS_H */
