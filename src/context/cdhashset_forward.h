/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * This is a forward declaration header to declare the CDSet<>
 * template
 *
 * It's useful if you want to forward-declare CDSet<> without including the
 * full cdset.h header, for example, in a public header context.
 *
 * For CDSet<> in particular, it's difficult to forward-declare it
 * yourself, because it has a default template argument.
 */

#include "ava6_public.h"

#ifndef AVA6__CONTEXT__CDSET_FORWARD_H
#define AVA6__CONTEXT__CDSET_FORWARD_H

#include <functional>

namespace ava6::context {
template <class V, class HashFcn = std::hash<V> >
class CDHashSet;
}  // namespace ava6::context

#endif /* AVA6__CONTEXT__CDSET_FORWARD_H */
