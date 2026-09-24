/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Implementation of rewriter for the theory of (co)inductive datatypes.
 */

#include "theory/datatypes/datatypes_rewriter.h"

#include "expr/ascription_type.h"
#include "expr/dtype.h"
#include "expr/dtype_cons.h"
#include "expr/elim_shadow_converter.h"
#include "expr/node_algorithm.h"
#include "expr/skolem_manager.h"
#include "options/datatypes_options.h"
#include "theory/datatypes/project_op.h"
#include "theory/datatypes/theory_datatypes_utils.h"
#include "tuple_utils.h"
#include "util/rational.h"

using namespace ava6::internal;
using namespace ava6::internal::kind;

namespace ava6::internal {
namespace theory {
namespace datatypes {

DatatypesRewriter::DatatypesRewriter(NodeManager* nm) : TheoryRewriter(nm)
{
  registerProofRewriteRule(ProofRewriteRule::DT_INST,
                           TheoryRewriteCtx::PRE_DSL);
  registerProofRewriteRule(ProofRewriteRule::DT_COLLAPSE_SELECTOR,
                           TheoryRewriteCtx::PRE_DSL);
  registerProofRewriteRule(ProofRewriteRule::DT_COLLAPSE_TESTER,
                           TheoryRewriteCtx::PRE_DSL);
  registerProofRewriteRule(ProofRewriteRule::DT_COLLAPSE_TESTER_SINGLETON,
                           TheoryRewriteCtx::PRE_DSL);
  // DT_CONS_EQ and DT_CONS_EQ_CLASH are part of the reconstruction of
  // MACRO_DT_CONS_EQ.
  registerProofRewriteRule(ProofRewriteRule::MACRO_DT_CONS_EQ,
                           TheoryRewriteCtx::PRE_DSL);
  registerProofRewriteRule(ProofRewriteRule::DT_COLLAPSE_UPDATER,
                           TheoryRewriteCtx::PRE_DSL);
  registerProofRewriteRule(ProofRewriteRule::DT_UPDATER_ELIM,
                           TheoryRewriteCtx::PRE_DSL);
  registerProofRewriteRule(ProofRewriteRule::DT_MATCH_ELIM,
                           TheoryRewriteCtx::PRE_DSL);
  registerProofRewriteRule(ProofRewriteRule::DT_CYCLE,
                           TheoryRewriteCtx::PRE_DSL);
}

Node DatatypesRewriter::rewriteViaRule(ProofRewriteRule id, const Node& n)
{
  switch (id)
  {
    case ProofRewriteRule::DT_INST:
    {
      if (n.getKind() != Kind::APPLY_TESTER)
      {
        return Node::null();
      }
      Node t = n[0];
      TypeNode tn = t.getType();
      Assert(tn.isDatatype());
      const DType& dt = tn.getDType();
      size_t i = utils::indexOf(n.getOperator());
      // Note that we set shared selectors to false. This proof rule will
      // be (unintentionally) unsuccessful when reconstructing proofs of the
      // rewriter when using shared selectors.
      Node ticons = utils::getInstCons(t, dt, i);
      return t.eqNode(ticons);
    }
    case ProofRewriteRule::DT_COLLAPSE_SELECTOR:
    {
      if (n.getKind() != Kind::APPLY_SELECTOR
          || n[0].getKind() != Kind::APPLY_CONSTRUCTOR)
      {
        return Node::null();
      }
      Node selector = n.getOperator();
      // shared selectors are not supported
      
      size_t constructorIndex = utils::indexOf(n[0].getOperator());
      const DType& dt = utils::datatypeOf(selector);
      const DTypeConstructor& c = dt[constructorIndex];
      int selectorIndex = c.getSelectorIndexInternal(selector);
      if (selectorIndex >= 0)
      {
        Assert(static_cast<size_t>(selectorIndex) < c.getNumArgs());
        return n[0][selectorIndex];
      }
    }
    break;
    case ProofRewriteRule::DT_COLLAPSE_TESTER:
    {
      if (n.getKind() != Kind::APPLY_TESTER
          || n[0].getKind() != Kind::APPLY_CONSTRUCTOR)
      {
        return Node::null();
      }
      bool result = AVA6_EQUAL(utils::indexOf(n.getOperator()),
                               utils::indexOf(n[0].getOperator()));
      NodeManager* nm = nodeManager();
      return nm->mkConst(result);
    }
    break;
    case ProofRewriteRule::DT_COLLAPSE_TESTER_SINGLETON:
    {
      if (n.getKind() != Kind::APPLY_TESTER)
      {
        return Node::null();
      }
      const DType& dt = n[0].getType().getDType();
      if (dt.getNumConstructors() == 1)
      {
        NodeManager* nm = nodeManager();
        return nm->mkConst(true);
      }
    }
    break;
    case ProofRewriteRule::MACRO_DT_CONS_EQ:
    {
      if (n.getKind() == Kind::EQUAL)
      {
        Node nn;
        std::vector<Node> rew;
        if (utils::checkClash(n[0], n[1], rew))
        {
          nn = nodeManager()->mkConst(false);
        }
        else if (!rew.empty())
        {
          nn = nodeManager()->mkAnd(rew);
        }
        else
        {
          return Node::null();
        }
        // In the "else" case above will n if this rewrite does not apply. We
        // do not return the reflexive equality in this case.
        if (nn != n)
        {
          return nn;
        }
      }
    }
    break;
    case ProofRewriteRule::DT_CONS_EQ:
    {
      if (n.getKind() != Kind::EQUAL
          || n[0].getKind() != Kind::APPLY_CONSTRUCTOR
          || n[1].getKind() != Kind::APPLY_CONSTRUCTOR)
      {
        return Node::null();
      }
      if (n[0].getOperator() == n[1].getOperator())
      {
        Assert(n[0].getNumChildren() == n[1].getNumChildren());
        std::vector<Node> children;
        for (size_t i = 0, size = n[0].getNumChildren(); i < size; i++)
        {
          children.push_back(n[0][i].eqNode(n[1][i]));
        }
        return nodeManager()->mkAnd(children);
      }
    }
    break;
    case ProofRewriteRule::DT_CONS_EQ_CLASH:
    {
      if (n.getKind() != Kind::EQUAL
          || n[0].getKind() != Kind::APPLY_CONSTRUCTOR
          || n[1].getKind() != Kind::APPLY_CONSTRUCTOR)
      {
        return Node::null();
      }
      // do not look for constant clashing equality between non-datatypes
      std::vector<Node> rew;
      if (utils::checkClash(n[0], n[1], rew, false))
      {
        return nodeManager()->mkConst(false);
      }
    }
    break;
    case ProofRewriteRule::DT_UPDATER_ELIM:
    {
      if (n.getKind() == Kind::APPLY_UPDATER)
      {
        return expandUpdater(n);
      }
    }
    break;
    case ProofRewriteRule::DT_COLLAPSE_UPDATER:
    {
      if (n.getKind() != Kind::APPLY_UPDATER
          || n[0].getKind() != Kind::APPLY_CONSTRUCTOR)
      {
        return Node::null();
      }
      Node op = n.getOperator();
      size_t cindex = utils::indexOf(n[0].getOperator());
      size_t cuindex = utils::cindexOf(op);
      if (cindex == cuindex)
      {
        size_t updateIndex = utils::indexOf(op);
        std::vector<Node> children(n[0].begin(), n[0].end());
        children[updateIndex] = n[1];
        children.insert(children.begin(), n[0].getOperator());
        return d_nm->mkNode(Kind::APPLY_CONSTRUCTOR, children);
      }
      return n[0];
    }
    break;
    case ProofRewriteRule::DT_MATCH_ELIM:
    {
      if (n.getKind() == Kind::MATCH)
      {
        return expandMatch(n);
      }
    }
    break;
    case ProofRewriteRule::DT_CYCLE:
    {
      if (n.getKind() == Kind::EQUAL && n[0] != n[1])
      {
        std::unordered_set<TNode> visited;
        std::vector<TNode> visit;
        TNode cur;
        visit.push_back(n[1]);
        do
        {
          cur = visit.back();
          visit.pop_back();
          if (visited.find(cur) == visited.end())
          {
            visited.insert(cur);
            if (cur == n[0])
            {
              return d_nm->mkConst(false);
            }
            if (cur.getKind() == Kind::APPLY_CONSTRUCTOR)
            {
              visit.insert(visit.end(), cur.begin(), cur.end());
            }
          }
        } while (!visit.empty());
      }
    }
    break;
    default: break;
  }
  return Node::null();
}

RewriteResponse DatatypesRewriter::postRewrite(TNode in)
{
  Trace("datatypes-rewrite-debug") << "post-rewriting " << in << std::endl;
  Kind kind = in.getKind();
  NodeManager* nm = nodeManager();
  if (kind == Kind::APPLY_CONSTRUCTOR)
  {
    return rewriteConstructor(in);
  }
  else if (kind == Kind::APPLY_SELECTOR)
  {
    return rewriteSelector(in);
  }
  else if (kind == Kind::APPLY_TESTER)
  {
    return rewriteTester(in);
  }
  else if (kind == Kind::APPLY_UPDATER)
  {
    return rewriteUpdater(in);
  }
  else if (kind == Kind::MATCH_BIND_CASE)
  {
    // eliminate shadowing
    Node retElimShadow = ElimShadowNodeConverter::eliminateShadow(in);
    if (retElimShadow != in)
    {
      return RewriteResponse(REWRITE_AGAIN_FULL, retElimShadow);
    }
  }
  else if (kind == Kind::TUPLE_PROJECT)
  {
    // returns a tuple that represents
    // (tuple ((_ tuple_select i_1) t) ... ((_ tuple_select i_n) t))
    // where each i_j is less than the length of t

    Trace("dt-rewrite-project") << "Rewrite project: " << in << std::endl;

    ProjectOp op = in.getOperator().getConst<ProjectOp>();
    std::vector<uint32_t> indices = op.getIndices();
    Node tuple = in[0];
    Node ret = TupleUtils::getTupleProjection(indices, tuple);

    Trace("dt-rewrite-project")
        << "Rewrite project: " << in << " ... " << ret << std::endl;
    return RewriteResponse(REWRITE_AGAIN_FULL, ret);
  }

  if (kind == Kind::EQUAL)
  {
    if (in[0] == in[1])
    {
      return RewriteResponse(REWRITE_DONE, nm->mkConst(true));
    }
    std::vector<Node> rew;
    if (utils::checkClash(in[0], in[1], rew))
    {
      Trace("datatypes-rewrite")
          << "Rewrite clashing equality " << in << " to false" << std::endl;
      return RewriteResponse(REWRITE_DONE, nm->mkConst(false));
    }
    else if (in[1] < in[0])
    {
      Node ins = nm->mkNode(in.getKind(), in[1], in[0]);
      Trace("datatypes-rewrite")
          << "Swap equality " << in << " to " << ins << std::endl;
      return RewriteResponse(REWRITE_DONE, ins);
    }
    Trace("datatypes-rewrite-debug")
        << "Did not rewrite equality " << in << " " << in[0].getKind() << " "
        << in[1].getKind() << std::endl;
  }

  return RewriteResponse(REWRITE_DONE, in);
}
Node DatatypesRewriter::expandMatch(Node in)
{
  Assert(in.getKind() == Kind::MATCH);
  NodeManager* nm = in.getNodeManager();
  // ensure we've type checked
  TypeNode tin = in.getType();
  Node h = in[0];
  std::vector<Node> cases;
  std::vector<Node> rets;
  TypeNode t = h.getType();
  const DType& dt = t.getDType();
  for (size_t k = 1, nchild = in.getNumChildren(); k < nchild; k++)
  {
    Node c = in[k];
    Node cons;
    Kind ck = c.getKind();
    if (ck == Kind::MATCH_CASE)
    {
      Assert(c[0].getKind() == Kind::APPLY_CONSTRUCTOR);
      cons = c[0].getOperator();
    }
    else if (ck == Kind::MATCH_BIND_CASE)
    {
      if (c[1].getKind() == Kind::APPLY_CONSTRUCTOR)
      {
        cons = c[1].getOperator();
      }
    }
    else
    {
      AlwaysAssert(false) << "Bad case for match term";
    }
    size_t cindex = 0;
    // cons is null in the default case
    if (!cons.isNull())
    {
      cindex = utils::indexOf(cons);
    }
    Node body;
    if (ck == Kind::MATCH_CASE)
    {
      body = c[1];
    }
    else if (ck == Kind::MATCH_BIND_CASE)
    {
      std::vector<Node> vars;
      std::vector<Node> subs;
      if (cons.isNull())
      {
        Assert(c[1].getKind() == Kind::BOUND_VARIABLE);
        vars.push_back(c[1]);
        subs.push_back(h);
      }
      else
      {
        for (size_t i = 0, vsize = c[0].getNumChildren(); i < vsize; i++)
        {
          vars.push_back(c[0][i]);
          Node sc =
              nm->mkNode(Kind::APPLY_SELECTOR, dt[cindex][i].getSelector(), h);
          subs.push_back(sc);
        }
      }
      body =
          c[2].substitute(vars.begin(), vars.end(), subs.begin(), subs.end());
    }
    if (!cons.isNull())
    {
      cases.push_back(utils::mkTester(h, cindex, dt));
    }
    else
    {
      // variables have no constraints
      cases.push_back(nm->mkConst(true));
    }
    rets.push_back(body);
  }
  Assert(!cases.empty());
  // now make the ITE
  std::reverse(cases.begin(), cases.end());
  std::reverse(rets.begin(), rets.end());
  Node ret = rets[0];
  // notice that due to our type checker, either there is a variable pattern
  // or all constructors are present in the match.
  for (size_t i = 1, ncases = cases.size(); i < ncases; i++)
  {
    ret = nm->mkNode(Kind::ITE, cases[i], rets[i], ret);
  }
  return ret;
}

RewriteResponse DatatypesRewriter::preRewrite(TNode in)
{
  Trace("datatypes-rewrite-debug") << "pre-rewriting " << in << std::endl;
  // must prewrite to apply type ascriptions since rewriting does not preserve
  // types
  if (in.getKind() == Kind::APPLY_CONSTRUCTOR)
  {
    TypeNode tn = in.getType();

    // check for parametric datatype constructors
    // to ensure a normal form, all parameteric datatype constructors must have
    // a type ascription
    if (tn.isParametricDatatype())
    {
      if (in.getOperator().getKind() != Kind::APPLY_TYPE_ASCRIPTION)
      {
        Trace("datatypes-rewrite-debug")
            << "Ascribing type to parametric datatype constructor " << in
            << std::endl;
        Node op = in.getOperator();
        // Use opIndex to ensure deterministic node ID assignments
        size_t opIndex = utils::indexOf(op);
        // get the constructor object
        const DTypeConstructor& dtc = utils::datatypeOf(op)[opIndex];
        // create ascribed constructor type
        Node op_new = dtc.getInstantiatedConstructor(tn);
        // make new node
        std::vector<Node> children;
        children.push_back(op_new);
        children.insert(children.end(), in.begin(), in.end());
        Node inr = nodeManager()->mkNode(Kind::APPLY_CONSTRUCTOR, children);
        Trace("datatypes-rewrite-debug") << "Created " << inr << std::endl;
        return RewriteResponse(REWRITE_DONE, inr);
      }
    }
  }
  return RewriteResponse(REWRITE_DONE, in);
}

RewriteResponse DatatypesRewriter::rewriteConstructor(TNode in)
{

  return RewriteResponse(REWRITE_DONE, in);
}

RewriteResponse DatatypesRewriter::rewriteSelector(TNode in)
{
  Assert(in.getKind() == Kind::APPLY_SELECTOR);
  if (in[0].getKind() == Kind::APPLY_CONSTRUCTOR)
  {
    // Have to be careful not to rewrite well-typed expressions
    // where the selector doesn't match the constructor,
    // e.g. "pred(zero)".
    Node selector = in.getOperator();
    TNode constructor = in[0].getOperator();
    size_t constructorIndex = utils::indexOf(constructor);
    const DType& dt = utils::datatypeOf(selector);
    const DTypeConstructor& c = dt[constructorIndex];
    Trace("datatypes-rewrite-debug")
        << "Rewriting collapsable selector : " << in;
    Trace("datatypes-rewrite-debug")
        << ", cindex = " << constructorIndex << ", selector is " << selector
        << std::endl;
    // The argument that the selector extracts, or -1 if the selector is
    // is wrongly applied.
    // The argument index of internal selectors is obtained by
    // getSelectorIndexInternal.
    int selectorIndex = c.getSelectorIndexInternal(selector);
    Trace("datatypes-rewrite-debug")
        << "Internal selector index is " << selectorIndex << std::endl;
    if (selectorIndex >= 0)
    {
      Assert(selectorIndex < (int)c.getNumArgs());
      {
        Trace("datatypes-rewrite")
            << "DatatypesRewriter::postRewrite: "
            << "Rewrite trivial selector " << in << std::endl;
        return RewriteResponse(REWRITE_DONE, in[0][selectorIndex]);
      }
    }
  }
  return RewriteResponse(REWRITE_DONE, in);
}

RewriteResponse DatatypesRewriter::rewriteTester(TNode in)
{
  size_t i = utils::indexOf(in.getOperator());
  if (in[0].getKind() == Kind::APPLY_CONSTRUCTOR)
  {
    bool result = (i == utils::indexOf(in[0].getOperator()));
    Trace("datatypes-rewrite")
        << "DatatypesRewriter::postRewrite: "
        << "Rewrite trivial tester " << in << " " << result << std::endl;
    return RewriteResponse(REWRITE_DONE, nodeManager()->mkConst(result));
  }
  TypeNode tn = in[0].getType();
  const DType& dt = tn.getDType();
  {
    if (dt[i].getNumArgs() == 0)
    {
      // If a constant, then e.g. ((_ is nil) x) ---> (= x nil).
      // This is only done for constant constructors since it does not
      // introduce any new (selector) terms.
      Node cc = utils::mkApplyCons(tn, dt, i, {});
      Node eq = nodeManager()->mkNode(Kind::EQUAL, in[0], cc);
      return RewriteResponse(REWRITE_AGAIN_FULL, eq);
    }
    if (dt.getNumConstructors() == 1)
    {
      // only one constructor, so it must be
      Trace("datatypes-rewrite")
          << "DatatypesRewriter::postRewrite: "
          << "only one ctor for " << dt.getName() << " and that is "
          << dt[0].getName() << std::endl;
      return RewriteResponse(REWRITE_DONE, nodeManager()->mkConst(true));
    }
  }
  // could try dt.getNumConstructors()==2 && indexOf(in.getOperator())==1 ?
  return RewriteResponse(REWRITE_DONE, in);
}

RewriteResponse DatatypesRewriter::rewriteUpdater(TNode in)
{
  Assert(in.getKind() == Kind::APPLY_UPDATER);
  if (in[0].getKind() == Kind::APPLY_CONSTRUCTOR)
  {
    Node op = in.getOperator();
    size_t cindex = utils::indexOf(in[0].getOperator());
    size_t cuindex = utils::cindexOf(op);
    if (cindex == cuindex)
    {
      NodeManager* nm = nodeManager();
      size_t updateIndex = utils::indexOf(op);
      std::vector<Node> children(in[0].begin(), in[0].end());
      children[updateIndex] = in[1];
      children.insert(children.begin(), in[0].getOperator());
      return RewriteResponse(REWRITE_DONE,
                             nm->mkNode(Kind::APPLY_CONSTRUCTOR, children));
    }
    return RewriteResponse(REWRITE_DONE, in[0]);
  }
  return RewriteResponse(REWRITE_DONE, in);
}

Node DatatypesRewriter::expandDefinition(Node n)
{
  Node ret;
  switch (n.getKind())
  {
    case Kind::APPLY_UPDATER:
    {
      ret = expandUpdater(n);
      Trace("dt-expand") << "return " << ret << std::endl;
      break;
    }
    default: break;
  }
  if (!ret.isNull() && n != ret)
  {
    return ret;
  }
  return Node::null();
}

Node DatatypesRewriter::expandUpdater(const Node& n)
{
  NodeManager* nm = nodeManager();
  TypeNode tn = n.getType();
  Node ret;
  Assert(tn.isDatatype());
  const DType& dt = tn.getDType();
  Node op = n.getOperator();
  size_t updateIndex = utils::indexOf(op);
  size_t cindex = utils::cindexOf(op);
  const DTypeConstructor& dc = dt[cindex];
  NodeBuilder b(nm, Kind::APPLY_CONSTRUCTOR);
  if (tn.isParametricDatatype())
  {
    b << dc.getInstantiatedConstructor(n[0].getType());
  }
  else
  {
    b << dc.getConstructor();
  }
  Trace("dt-expand") << "Expand updater " << n << std::endl;
  Trace("dt-expand") << "expr is " << n << std::endl;
  Trace("dt-expand") << "updateIndex is " << updateIndex << std::endl;
  Trace("dt-expand") << "t is " << tn << std::endl;
  for (size_t i = 0, size = dc.getNumArgs(); i < size; ++i)
  {
    if (i == updateIndex)
    {
      b << n[1];
    }
    else
    {
      b << utils::applySelector(dc, i, n[0]);
    }
  }
  ret = b;
  // note it may be that this dt has one constructor, in which case this
  // tester will rewrite to true.
  // must be the right constructor to update
  Node tester = nm->mkNode(Kind::APPLY_TESTER, dc.getTester(), n[0]);
  return nm->mkNode(Kind::ITE, tester, ret, n[0]);
}
}  // namespace datatypes
}  // namespace theory
}  // namespace ava6::internal
