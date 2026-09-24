/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Implementation of skolemization utility.
 */

#include "theory/quantifiers/skolemize.h"

#include "expr/dtype.h"
#include "expr/dtype_cons.h"
#include "expr/skolem_manager.h"
#include "options/quantifiers_options.h"
#include "options/smt_options.h"
#include "proof/proof.h"
#include "proof/proof_node_manager.h"
#include "theory/quantifiers/quantifiers_attributes.h"
#include "theory/quantifiers/quantifiers_state.h"
#include "theory/quantifiers/term_registry.h"
#include "theory/quantifiers/term_util.h"
#include "theory/rewriter.h"
#include "util/rational.h"

using namespace ava6::internal::kind;

namespace ava6::internal {
namespace theory {
namespace quantifiers {

Skolemize::Skolemize(Env& env, QuantifiersState& qs, TermRegistry& tr)
    : EnvObj(env),
      d_qstate(qs),
      d_treg(tr),
      d_skolemized(userContext()),
      d_epg(!isProofEnabled()
                ? nullptr
                : new EagerProofGenerator(env, userContext(), "Skolemize::epg"))
{
}

TrustNode Skolemize::process(Node q)
{
  Assert(q.getKind() == Kind::FORALL);
  // do skolemization
  if (d_skolemized.find(q) != d_skolemized.end())
  {
    return TrustNode::null();
  }
  Node lem;
  ProofGenerator* pg = nullptr;
  if (isProofEnabled())
  {
    ProofNodeManager* pnm = d_env.getProofNodeManager();
    // Construct a justified skolemization.
    NodeManager* nm = d_env.getNodeManager();
    // cache the skolems in d_skolem_constants[q]
    std::vector<Node>& skolems = d_skolem_constants[q];
    skolems = getSkolemConstants(q);
    std::vector<Node> vars(q[0].begin(), q[0].end());
    Node res = q[1].substitute(
        vars.begin(), vars.end(), skolems.begin(), skolems.end());
    Node qnot = q.notNode();
    CDProof cdp(d_env);
    res = res.notNode();
    cdp.addStep(res, ProofRule::SKOLEMIZE, {qnot}, {});
    std::shared_ptr<ProofNode> pf = cdp.getProofFor(res);
    std::vector<Node> assumps;
    assumps.push_back(qnot);
    std::shared_ptr<ProofNode> pfs = pnm->mkScope({pf}, assumps);
    lem = nm->mkNode(Kind::IMPLIES, qnot, res);
    d_epg->setProofFor(lem, pfs);
    pg = d_epg.get();
    Trace("quantifiers-sk")
        << "Skolemize (with proofs) : " << d_skolem_constants[q] << " for "
        << std::endl;
    Trace("quantifiers-sk") << "   " << q << std::endl;
    Trace("quantifiers-sk") << "   " << res << std::endl;
  }
  else
  {
    // Without proofs, construct the skolemized body directly.
    Node body = mkSkolemizedBody(q, q[1], {}, d_skolem_constants[q]);
    NodeBuilder nb(nodeManager(), Kind::OR);
    nb << q << body.notNode();
    lem = nb;
  }
  d_skolemized[q] = lem;
  // triggered when skolemizing
  return TrustNode::mkTrustLemma(lem, pg);
}

std::vector<Node> Skolemize::getSkolemConstants(const Node& q)
{
  Assert(q.getKind() == Kind::FORALL);
  std::vector<Node> skolems;
  for (size_t i = 0, nvars = q[0].getNumChildren(); i < nvars; i++)
  {
    skolems.push_back(getSkolemConstant(q, i));
  }
  return skolems;
}

Node Skolemize::getSkolemConstant(const Node& q, size_t i)
{
  Assert(q.getKind() == Kind::FORALL);
  Assert(i < q[0].getNumChildren());
  NodeManager* nm = q.getNodeManager();
  SkolemManager* sm = nm->getSkolemManager();
  std::vector<Node> cacheVals{q, nm->mkConstInt(Rational(i))};
  return sm->mkSkolemFunction(SkolemId::QUANTIFIERS_SKOLEMIZE, cacheVals);
}

Node Skolemize::mkSkolemizedBody(Node q, Node body,
                                const std::vector<TNode>& fvs,
                                std::vector<Node>& skolems)
{
  Assert(q.getKind() == Kind::FORALL);
  Assert(skolems.empty() || skolems.size() == q[0].getNumChildren());
  NodeManager* nm = q.getNodeManager();
  std::vector<TypeNode> argTypes;
  for (TNode v : fvs) { argTypes.push_back(v.getType()); }
  std::vector<Node> vars(q[0].begin(), q[0].end());
  if (skolems.empty() && fvs.empty())
  {
    skolems = getSkolemConstants(q);
  }
  else if (skolems.empty())
  {
    for (Node v : vars)
    {
      Node op = NodeManager::mkDummySkolem(
          "skop", nm->mkFunctionType(argTypes, v.getType()));
      {
        std::vector<Node> args{op};
        args.insert(args.end(), fvs.begin(), fvs.end());
        op = nm->mkNode(Kind::APPLY_UF, args);
      }
      skolems.push_back(op);
    }
  }
  Node ret = body.substitute(
      vars.begin(), vars.end(), skolems.begin(), skolems.end());
  uint64_t level;
  if (QuantAttributes::getInstantiationLevel(q, level))
  {
    QuantAttributes::setInstantiationLevelAttr(ret, level);
  }
  return ret;
}

void Skolemize::getSkolemTermVectors(
    std::map<Node, std::vector<Node>>& sks) const
{
  std::unordered_map<Node, std::vector<Node>>::const_iterator itk;
  for (const auto& p : d_skolemized)
  {
    Node q = p.first;
    itk = d_skolem_constants.find(q);
    Assert(itk != d_skolem_constants.end());
    sks[q].insert(sks[q].end(), itk->second.begin(), itk->second.end());
  }
}

bool Skolemize::isProofEnabled() const
{
  return d_env.isTheoryProofProducing();
}

}  // namespace quantifiers
}  // namespace theory
}  // namespace ava6::internal
