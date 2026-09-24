/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * A class for ProjectOp operator.
 */

#include "ava6_public.h"

#ifndef AVA6__PROJECT_OP_H
#define AVA6__PROJECT_OP_H

#include <ostream>
#include <vector>

namespace ava6::internal {

class TypeNode;

/**
 * class for projection operators
 */
class ProjectOp
{
 public:
  explicit ProjectOp(std::vector<uint32_t> indices);
  ProjectOp(const ProjectOp& op) = default;

  /** return the indices of the projection */
  const std::vector<uint32_t>& getIndices() const;

  bool operator==(const ProjectOp& op) const;

 private:
  std::vector<uint32_t> d_indices;
}; /* class ProjectOp */

std::ostream& operator<<(std::ostream& out, const ProjectOp& op);

/**
 * Hash function for the ProjectOpHashFunction objects.
 */
struct ProjectOpHashFunction
{
  size_t operator()(const ProjectOp& op) const;
}; /* struct ProjectOpHashFunction */

}  // namespace ava6::internal

#endif /* AVA6__PROJECT_OP_H */
