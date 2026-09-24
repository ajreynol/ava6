/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Sets theory rewriter.
 */

#include "theory/sets/theory_sets_rewriter.h"

#include "expr/attribute.h"
#include "expr/dtype.h"
#include "expr/dtype_cons.h"
#include "expr/elim_shadow_converter.h"
#include "options/sets_options.h"
#include "theory/datatypes/tuple_utils.h"
#include "theory/sets/normal_form.h"
#include "util/rational.h"

using namespace ava6::internal::kind;
using namespace ava6::internal::theory::datatypes;

namespace ava6::internal {
namespace theory {
namespace sets {

TheorySetsRewriter::TheorySetsRewriter(NodeManager* nm,
                                       bool cardEnabled,
                                       bool relsEnabled)
    : TheoryRewriter(nm), d_cardEnabled(cardEnabled), d_relsEnabled(relsEnabled)
{
  // Needs to be a subcall in DSL reconstruction since set.is_empty is used
  // as a premise to test emptiness of a set.
  registerProofRewriteRule(ProofRewriteRule::SETS_INSERT_ELIM,
                           TheoryRewriteCtx::PRE_DSL);
  registerProofRewriteRule(ProofRewriteRule::SETS_EVAL_OP,
                           TheoryRewriteCtx::POST_DSL);
}

Node TheorySetsRewriter::rewriteViaRule(ProofRewriteRule id, const Node& n)
{
  switch (id)
  {
    case ProofRewriteRule::SETS_INSERT_ELIM:
    {
      if (n.getKind() == Kind::SET_INSERT)
      {
        NodeManager* nm = nodeManager();
        size_t setNodeIndex = n.getNumChildren() - 1;
        Node elems = n[setNodeIndex];
        for (size_t i = 0; i < setNodeIndex; ++i)
        {
          size_t ii = (setNodeIndex - i) - 1;
          Node singleton = nm->mkNode(Kind::SET_SINGLETON, n[ii]);
          elems = nm->mkNode(Kind::SET_UNION, singleton, elems);
        }
        return elems;
      }
    }
    break;
    case ProofRewriteRule::SETS_EVAL_OP:
    {
      if (n.getNumChildren() != 2 || !n[0].isConst() || !n[1].isConst())
      {
        return Node::null();
      }
      Kind k = n.getKind();
      if (k == Kind::SET_INTER)
      {
        std::set<Node> left = NormalForm::getElementsFromNormalConstant(n[0]);
        std::set<Node> right = NormalForm::getElementsFromNormalConstant(n[1]);
        std::set<Node> newSet;
        std::set_intersection(left.begin(),
                              left.end(),
                              right.begin(),
                              right.end(),
                              std::inserter(newSet, newSet.begin()));
        return NormalForm::elementsToSet(newSet, n.getType());
      }
      if (k == Kind::SET_MINUS)
      {
        std::set<Node> left = NormalForm::getElementsFromNormalConstant(n[0]);
        std::set<Node> right = NormalForm::getElementsFromNormalConstant(n[1]);
        std::set<Node> newSet;
        std::set_difference(left.begin(),
                            left.end(),
                            right.begin(),
                            right.end(),
                            std::inserter(newSet, newSet.begin()));
        return NormalForm::elementsToSet(newSet, n.getType());
      }
      if (k == Kind::SET_UNION)
      {
        std::set<Node> left = NormalForm::getElementsFromNormalConstant(n[0]);
        std::set<Node> right = NormalForm::getElementsFromNormalConstant(n[1]);
        std::set<Node> newSet;
        std::set_union(left.begin(),
                       left.end(),
                       right.begin(),
                       right.end(),
                       std::inserter(newSet, newSet.begin()));
        return NormalForm::elementsToSet(newSet, n.getType());
      }
    }
    break;
    default: break;
  }
  return Node::null();
}

bool TheorySetsRewriter::checkConstantMembership(TNode elementTerm,
                                                 TNode setTerm)
{
  if (setTerm.getKind() == Kind::SET_EMPTY)
  {
    return false;
  }

  if (setTerm.getKind() == Kind::SET_SINGLETON)
  {
    return elementTerm == setTerm[0];
  }

  Assert(setTerm.getKind() == Kind::SET_UNION
         && setTerm[0].getKind() == Kind::SET_SINGLETON)
      << "kind was " << setTerm.getKind() << ", term: " << setTerm;

  return elementTerm == setTerm[0][0]
         || checkConstantMembership(elementTerm, setTerm[1]);
}

// static
RewriteResponse TheorySetsRewriter::postRewrite(TNode node)
{
  NodeManager* nm = nodeManager();
  Kind kind = node.getKind();
  Trace("sets-postrewrite") << "Process: " << node << std::endl;

  if (node.isConst())
  {
    Trace("sets-rewrite-nf")
        << "Sets::rewrite: no rewrite (constant) " << node << std::endl;
    // Dare you touch the const and mangle it to something else.
    return RewriteResponse(REWRITE_DONE, node);
  }

  switch (kind)
  {
    case Kind::SET_MEMBER:
    {
      if (node[0].isConst() && node[1].isConst())
      {
        // both are constants
        TNode S = preRewrite(node[1]).d_node;
        bool isMember = checkConstantMembership(node[0], S);
        return RewriteResponse(REWRITE_DONE, nm->mkConst(isMember));
      }
      else if (node[1].getKind() == Kind::SET_EMPTY)
      {
        return RewriteResponse(REWRITE_DONE, nm->mkConst(false));
      }
      else if (node[1].getKind() == Kind::SET_SINGLETON)
      {
        return RewriteResponse(REWRITE_AGAIN_FULL,
                               nm->mkNode(Kind::EQUAL, node[0], node[1][0]));
      }
      else if (node[1].getKind() == Kind::SET_UNION
               || node[1].getKind() == Kind::SET_INTER
               || node[1].getKind() == Kind::SET_MINUS)
      {
        Node ret = rewriteMembershipBinaryOp(node);
        return RewriteResponse(REWRITE_AGAIN_FULL, ret);
      }
      break;
    }  // Kind::SET_MEMBER

    case Kind::SET_SUBSET:
    {
      DebugUnhandled()
          << "TheorySets::postRrewrite(): Subset is handled in preRewrite.";

      // but in off-chance we do end up here, let us do our best

      // rewrite (A subset-or-equal B) as (A union B = B)
      TNode A = node[0];
      TNode B = node[1];
      return RewriteResponse(
          REWRITE_AGAIN_FULL,
          nm->mkNode(Kind::EQUAL, nm->mkNode(Kind::SET_UNION, A, B), B));
    }  // Kind::SET_SUBSET

    case Kind::EQUAL:
    {
      // rewrite: t = t with true (t term)
      // rewrite: c = c' with c different from c' false (c, c' constants)
      // otherwise: sort them
      if (node[0] == node[1])
      {
        Trace("sets-postrewrite")
            << "Sets::postRewrite returning true" << std::endl;
        return RewriteResponse(REWRITE_DONE, nm->mkConst(true));
      }
      else if (node[0].isConst() && node[1].isConst())
      {
        Trace("sets-postrewrite")
            << "Sets::postRewrite returning false" << std::endl;
        return RewriteResponse(REWRITE_DONE, nm->mkConst(false));
      }
      else if (node[0] > node[1])
      {
        Node newNode = nm->mkNode(node.getKind(), node[1], node[0]);
        Trace("sets-postrewrite")
            << "Sets::postRewrite returning " << newNode << std::endl;
        return RewriteResponse(REWRITE_DONE, newNode);
      }
      break;
    }

    case Kind::SET_MINUS:
    {
      if (node[0] == node[1])
      {
        Node newNode = nm->mkConst(EmptySet(node[0].getType()));
        Trace("sets-postrewrite")
            << "Sets::postRewrite returning " << newNode << std::endl;
        return RewriteResponse(REWRITE_DONE, newNode);
      }
      else if (node[0].getKind() == Kind::SET_EMPTY
               || node[1].getKind() == Kind::SET_EMPTY)
      {
        Trace("sets-postrewrite")
            << "Sets::postRewrite returning " << node[0] << std::endl;
        return RewriteResponse(REWRITE_AGAIN, node[0]);
      }
      else if (node[0].isConst() && node[1].isConst())
      {
        Node newNode = rewriteViaRule(ProofRewriteRule::SETS_EVAL_OP, node);
        Assert(newNode.isConst());
        Trace("sets-postrewrite")
            << "Sets::postRewrite returning " << newNode << std::endl;
        return RewriteResponse(REWRITE_DONE, newNode);
      }
      break;
    }  // Kind::SET_MINUS

    case Kind::SET_INTER:
    {
      if (node[0] == node[1])
      {
        Trace("sets-postrewrite")
            << "Sets::postRewrite returning " << node[0] << std::endl;
        return RewriteResponse(REWRITE_AGAIN, node[0]);
      }
      else if (node[0].getKind() == Kind::SET_EMPTY)
      {
        return RewriteResponse(REWRITE_AGAIN, node[0]);
      }
      else if (node[1].getKind() == Kind::SET_EMPTY)
      {
        return RewriteResponse(REWRITE_AGAIN, node[1]);
      }
      else if (node[0].isConst() && node[1].isConst())
      {
        Node newNode = rewriteViaRule(ProofRewriteRule::SETS_EVAL_OP, node);
        Assert(newNode.isConst()
               && AVA6_EQUAL(newNode.getType(), node.getType()));
        Trace("sets-postrewrite")
            << "Sets::postRewrite returning " << newNode << std::endl;
        return RewriteResponse(REWRITE_DONE, newNode);
      }
      else if (node[0] > node[1])
      {
        Node newNode = nm->mkNode(node.getKind(), node[1], node[0]);
        return RewriteResponse(REWRITE_DONE, newNode);
      }
      // we don't merge non-constant intersections
      break;
    }  // Kind::INTERSECTION

    case Kind::SET_UNION:
    {
      // NOTE: case where it is CONST is taken care of at the top
      if (node[0] == node[1])
      {
        Trace("sets-postrewrite")
            << "Sets::postRewrite returning " << node[0] << std::endl;
        return RewriteResponse(REWRITE_AGAIN, node[0]);
      }
      else if (node[0].getKind() == Kind::SET_EMPTY)
      {
        return RewriteResponse(REWRITE_AGAIN, node[1]);
      }
      else if (node[1].getKind() == Kind::SET_EMPTY)
      {
        return RewriteResponse(REWRITE_AGAIN, node[0]);
      }
      else if (node[0].isConst() && node[1].isConst())
      {
        Node newNode = rewriteViaRule(ProofRewriteRule::SETS_EVAL_OP, node);
        Assert(newNode.isConst());
        Trace("sets-rewrite")
            << "Sets::rewrite: UNION_CONSTANT_MERGE: " << newNode << std::endl;
        return RewriteResponse(REWRITE_DONE, newNode);
      }
      else if (node[0] > node[1])
      {
        Node newNode = nm->mkNode(node.getKind(), node[1], node[0]);
        return RewriteResponse(REWRITE_DONE, newNode);
      }
      // we don't merge non-constant unions
      break;
    }  // Kind::SET_UNION
    case Kind::SET_CHOOSE:
    {
      if (node[0].getKind() == Kind::SET_SINGLETON)
      {
        //(= (choose (singleton x)) x) is a tautology
        // we return x for (choose (singleton x))
        return RewriteResponse(REWRITE_AGAIN, node[0][0]);
      }
      break;
    }  // Kind::SET_CHOOSE
    case Kind::SET_IS_EMPTY:
    {
      if (node[0].isConst())
      {
        // (set.is_empty c) ---> true if c is emptyset
        // (set.is_empty c) ---> false if c is a constant that is not the
        // emptyset
        return RewriteResponse(
            REWRITE_DONE,
            nodeManager()->mkConst(node[0].getKind() == Kind::SET_EMPTY));
      }
      // (set.is_empty x) ----> (= x (as set.empty (Set T))).
      Node eq = nodeManager()->mkNode(
          Kind::EQUAL,
          node[0],
          nodeManager()->mkConst(EmptySet(node[0].getType())));
      return RewriteResponse(REWRITE_AGAIN, eq);
    }
    case Kind::SET_IS_SINGLETON:
    {
      Kind nk = node[0].getKind();
      if (nk == Kind::SET_EMPTY)
      {
        return RewriteResponse(REWRITE_DONE, nodeManager()->mkConst(false));
      }
      if (nk == Kind::SET_SINGLETON)
      {
        //(= (is_singleton (singleton x)) is a tautology
        // we return true for (is_singleton (singleton x))
        return RewriteResponse(REWRITE_DONE, nodeManager()->mkConst(true));
      }
      break;
    }  // Kind::SET_IS_SINGLETON

    case Kind::SET_COMPREHENSION: return postRewriteComprehension(node); break;

    default: break;
  }

  return RewriteResponse(REWRITE_DONE, node);
}

Node TheorySetsRewriter::rewriteMembershipBinaryOp(const Node& node)
{
  Assert(node.getKind() == Kind::SET_MEMBER);
  Assert(node[1].getKind() == Kind::SET_UNION
         || node[1].getKind() == Kind::SET_INTER
         || node[1].getKind() == Kind::SET_MINUS);
  NodeManager* nm = nodeManager();
  std::vector<Node> children;
  for (size_t i = 0, nchild = node[1].getNumChildren(); i < nchild; i++)
  {
    Node nc = nm->mkNode(Kind::SET_MEMBER, node[0], node[1][i]);
    if (node[1].getKind() == Kind::SET_MINUS && i == 1)
    {
      nc = nc.negate();
    }
    children.push_back(nc);
  }
  return nm->mkNode(node[1].getKind() == Kind::SET_UNION ? Kind::OR : Kind::AND,
                    children);
}

// static
RewriteResponse TheorySetsRewriter::preRewrite(TNode node)
{
  NodeManager* nm = nodeManager();
  Kind k = node.getKind();
  if (k == Kind::EQUAL)
  {
    if (node[0] == node[1])
    {
      return RewriteResponse(REWRITE_DONE, nm->mkConst(true));
    }
  }
  else if (k == Kind::SET_INSERT)
  {
    Node ret = rewriteViaRule(ProofRewriteRule::SETS_INSERT_ELIM, node);
    return RewriteResponse(REWRITE_AGAIN, ret);
  }
  else if (k == Kind::SET_SUBSET)
  {
    // rewrite (A subset-or-equal B) as (A union B = B)
    return RewriteResponse(
        REWRITE_AGAIN,
        nm->mkNode(Kind::EQUAL,
                   nm->mkNode(Kind::SET_UNION, node[0], node[1]),
                   node[1]));
  }

  // could have an efficient normalizer for union here

  return RewriteResponse(REWRITE_DONE, node);
}

RewriteResponse TheorySetsRewriter::postRewriteComprehension(TNode n)
{
  Node ne = ElimShadowNodeConverter::eliminateShadow(n);
  if (ne != n)
  {
    return RewriteResponse(REWRITE_AGAIN_FULL, ne);
  }
  return RewriteResponse(REWRITE_DONE, n);
}

}  // namespace sets
}  // namespace theory
}  // namespace ava6::internal
