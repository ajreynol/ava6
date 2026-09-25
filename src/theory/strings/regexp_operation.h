/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Symbolic Regular Expression Operations
 */

#include "ava6_private.h"

#ifndef AVA6__THEORY__STRINGS__REGEXP__OPERATION_H
#define AVA6__THEORY__STRINGS__REGEXP__OPERATION_H

#include <map>
#include <unordered_map>
#include <vector>

#include "expr/node.h"
#include "smt/env_obj.h"
#include "theory/strings/skolem_cache.h"
#include "util/string.h"

namespace ava6::internal {
namespace theory {
namespace strings {

/**
 * Information on whether regular expressions contain constants or re.allchar.
 *
 * The order of this enumeration matters: the larger the value, the more
 * possible regular expressions could fit the description.
 */
enum RegExpConstType
{
  // the regular expression doesn't contain variables or re.comp,
  // re.allchar or re.range (call these three operators "non-concrete
  // operators"). Notice that re.comp is a non-concrete operator
  // since it can be seen as indirectly defined in terms of re.allchar.
  RE_C_CONCRETE_CONSTANT,
  // the regular expression doesn't contain variables, but may contain
  // re.comp, re.allchar or re.range
  RE_C_CONSTANT,
  // the regular expression may contain variables
  RE_C_VARIABLE,
  // the status of the regular expression is unknown (used internally)
  RE_C_UNKNOWN,
};

class RegExpOpr : protected EnvObj
{
  typedef std::pair<Node, Node> PairNodes;

 private:
  Node d_emptyString;


  /** A cache for simplify */
  std::map<Node, Node> d_simpCache;
  std::map<Node, std::pair<int, Node> > d_delta_cache;
  /** cache mapping regular expressions to whether they contain constants */
  std::unordered_map<Node, RegExpConstType> d_constCache;
  std::map<PairNodes, bool> d_inclusionCache;
  /**
   * Helper function for mkString, pretty prints constant or variable regular
   * expression r.
   */
  static std::string niceChar(Node r);

 public:
  RegExpOpr(Env& env, SkolemCache* sc);
  ~RegExpOpr();

  /**
   * Returns true if r is a "constant" regular expression, that is, a set
   * of regular expression operators whose subterms of the form (str.to.re t)
   * are such that t is a constant (or rewrites to one).
   */
  bool checkConstRegExp(Node r);
  /** get the constant type for regular expression r */
  RegExpConstType getRegExpConstType(Node r);
  /** Simplify
   *
   * This is the main method to simplify (unfold) a regular expression
   * membership. It is called where t is of the form (str.in_re s r),
   * and t (or (not t), when polarity=false) holds in the current context.
   * It returns the unfolded form of t.
   */
  Node simplify(Node t, bool polarity);
  /**
   * Given regular expression of the form
   *   (re.++ r_0 ... r_{n-1})
   * This returns a non-null node reLen and updates isRev such that
   *   RegExpEntail::getFixedLengthForRegexp(r_index) = reLen
   * where index is either 0 or n-1 when isRev is false or true respectively.
   */
  static Node getRegExpConcatFixed(Node r, bool& isRev);
  //------------------------ trusted reductions
  /**
   * Return the unfolded form of mem of the form (str.in_re s r).
   */
  static Node reduceRegExpPos(NodeManager* nm,
                              Node mem,
                              SkolemCache* sc,
                              std::vector<Node>& newSkolems);
  /**
   * Return the unfolded form of mem of the form (not (str.in_re s r)).
   */
  static Node reduceRegExpNeg(NodeManager* nm, Node mem);
  /**
   * Return the unfolded form of mem of the form
   *   (not (str.in_re s (re.++ r_0 ... r_{n-1})))
   * Called when RegExpEntail::getFixedLengthForRegexp(r_index) = reLen
   * where index is either 0 or n-1 where isRev is false or true respectively.
   *
   * This uses reLen as an optimization to improve the reduction. If reLen
   * is null, then this optimization is not applied.
   */
  static Node reduceRegExpNegConcatFixed(NodeManager* nm,
                                         Node mem,
                                         Node reLen,
                                         bool isRev);
  //------------------------ end trusted reductions
  /**
   * This method returns 1 if the empty string is in r, 2 if the empty string
   * is not in r, or 0 if it is unknown whether the empty string is in r.
   * TODO (project #2): refactor the return value of this function.
   *
   * If this method returns 0, then exp is updated to an explanation that
   * would imply that the empty string is in r.
   *
   * For example,
   * - delta( (re.inter (str.to.re x) (re.* "A")) ) returns 0 and sets exp to
   * x = "",
   * - delta( (re.++ (str.to.re "A") R) ) returns 2,
   * - delta( (re.union (re.* "A") R) ) returns 1.
   */
  int delta(Node r, Node& exp);
  /** Get the pretty printed version of the regular expression r */
  static std::string mkString(Node r);

  /**
   * Returns true if we can show that the regular expression `r1` includes
   * the regular expression `r2` (i.e. `r1` matches a superset of sequences
   * that `r2` matches). See documentation in RegExpEntail::regExpIncludes for
   * more details. This call caches the result (which is context-independent),
   * for performance reasons.
   */
  bool regExpIncludes(Node r1, Node r2);

 private:
  /** pointer to the skolem cache used by this class */
  SkolemCache* d_sc;
};

}  // namespace strings
}  // namespace theory
}  // namespace ava6::internal

#endif /* AVA6__THEORY__STRINGS__REGEXP__OPERATION_H */
