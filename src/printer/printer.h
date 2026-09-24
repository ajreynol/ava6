/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Base of the pretty-printer interface.
 */

#include "ava6_private.h"

#ifndef AVA6__PRINTER__PRINTER_H
#define AVA6__PRINTER__PRINTER_H

#include <ava6/ava6_export.h>

#include <memory>
#include <string>
#include <vector>

#include "expr/kind.h"
#include "options/language.h"
#include "util/result.h"

namespace ava6::internal {

template <bool ref_count>
class NodeTemplate;
typedef NodeTemplate<true> Node;
typedef NodeTemplate<false> TNode;
class TypeNode;
class UnsatCore;
struct InstantiationList;
struct SkolemList;
class LetBinding;

namespace smt {
class Model;
}

class AVA6_EXPORT Printer
{
 public:
  /**
   * Since the printers are managed as unique_ptr, we need public acces to
   * the virtual destructor.
   */
  virtual ~Printer() {}

  /** Get the Printer for a given output stream */
  static Printer* getPrinter(std::ostream& out);

  /** Get the Printer for a given Language */
  static Printer* getPrinter(Language lang);

  /** Write a Node out to a stream with this Printer. */
  virtual void toStream(std::ostream& out, TNode n) const = 0;

  /**
   * Write a Node out to a stream with this Printer, with the provided
   * let binding.
   * @param out The output stream to write to.
   * @param n The node to print.
   * @param lbind The let binding, which determines which nodes are letified.
   * @param lbindTop If false, the topmost term in n does not take into account
   * lbind.
   */
  virtual void toStream(std::ostream& out,
                        TNode n,
                        const LetBinding* lbind,
                        bool lbindTop) const;

  /** Write a Kind out to a stream with this Printer. */
  virtual void toStream(std::ostream& out, Kind k) const = 0;

  /** Write a Model out to a stream with this Printer. */
  virtual void toStream(std::ostream& out, const smt::Model& m) const;

  /** Write an UnsatCore out to a stream with this Printer. */
  virtual void toStream(std::ostream& out, const UnsatCore& core) const;

  /** Write an instantiation list out to a stream with this Printer. */
  virtual void toStream(std::ostream& out, const InstantiationList& is) const;

  /** Write a skolem list out to a stream with this Printer. */
  virtual void toStream(std::ostream& out, const SkolemList& sks) const;

  /** Print command success status */
  virtual void toStreamCmdSuccess(std::ostream& out) const;

  /** Print command interrupted status */
  virtual void toStreamCmdInterrupted(std::ostream& out) const;

  /** Print command unsupported status */
  virtual void toStreamCmdUnsupported(std::ostream& out) const;

  /** Print command failure status */
  virtual void toStreamCmdFailure(std::ostream& out,
                                  const std::string& message) const;

  /** Print command recoverable failure status */
  virtual void toStreamCmdRecoverableFailure(std::ostream& out,
                                             const std::string& message) const;

  /** Print empty command */
  virtual void toStreamCmdEmpty(std::ostream& out,
                                const std::string& name) const;

  /** Print echo command */
  virtual void toStreamCmdEcho(std::ostream& out,
                               const std::string& output) const;

  /** Print assert command */
  virtual void toStreamCmdAssert(std::ostream& out, Node n) const;

  /** Print push command */
  virtual void toStreamCmdPush(std::ostream& out, uint32_t nscopes) const;

  /** Print pop command */
  virtual void toStreamCmdPop(std::ostream& out, uint32_t nscopes) const;

  /** Print declare-fun command */
  virtual void toStreamCmdDeclareFunction(std::ostream& out,
                                          const std::string& id,
                                          const std::vector<TypeNode>& argTypes,
                                          TypeNode type) const;
  /** Variant of above for a pre-existing variable */
  void toStreamCmdDeclareFunction(std::ostream& out, const Node& v) const;
  /** Print declare-oracle-fun command */
  virtual void toStreamCmdDeclareOracleFun(
      std::ostream& out,
      const std::string& id,
      const std::vector<TypeNode>& argTypes,
      TypeNode type,
      const std::string& binName) const;

  /** Print declare-sort command */
  virtual void toStreamCmdDeclareType(std::ostream& out,
                                      const std::string& id,
                                      size_t arity) const;
  /** Variant of above that takes the type */
  void toStreamCmdDeclareType(std::ostream& out, TypeNode type) const;

  /** Print define-sort command */
  virtual void toStreamCmdDefineType(std::ostream& out,
                                     const std::string& id,
                                     const std::vector<TypeNode>& params,
                                     TypeNode t) const;

  /** Print define-fun command */
  virtual void toStreamCmdDefineFunction(std::ostream& out,
                                         const std::string& id,
                                         const std::vector<Node>& formals,
                                         TypeNode range,
                                         Node formula) const;
  /** Variant of above that takes the definition */
  void toStreamCmdDefineFunction(std::ostream& out, Node v, Node lambda) const;

  /** Print define-fun-rec command */
  virtual void toStreamCmdDefineFunctionRec(
      std::ostream& out,
      const std::vector<Node>& funcs,
      const std::vector<std::vector<Node>>& formals,
      const std::vector<Node>& formulas) const;
  /** Variant of above that takes the definition */
  void toStreamCmdDefineFunctionRec(std::ostream& out,
                                    const std::vector<Node>& funcs,
                                    const std::vector<Node>& lambdas) const;

  /** Print set-user-attribute command */
  void toStreamCmdSetUserAttribute(std::ostream& out,
                                   const std::string& attr,
                                   Node n) const;

  /** Print check-sat command */
  virtual void toStreamCmdCheckSat(std::ostream& out) const;

  /** Print check-sat-assuming command */
  virtual void toStreamCmdCheckSatAssuming(
      std::ostream& out, const std::vector<Node>& nodes) const;

  /** Print query command */
  virtual void toStreamCmdQuery(std::ostream& out, Node n) const;

  /** Print simplify command */
  virtual void toStreamCmdSimplify(std::ostream& out, Node n) const;

  /** Print get-value command */
  virtual void toStreamCmdGetValue(std::ostream& out,
                                   const std::vector<Node>& nodes) const;

  /** Print get-model-domain-elements command */
  virtual void toStreamCmdGetModelDomainElements(std::ostream& out,
                                                 TypeNode type) const;

  /** Print get-assignment command */
  virtual void toStreamCmdGetAssignment(std::ostream& out) const;

  /** Print get-model command */
  virtual void toStreamCmdGetModel(std::ostream& out) const;

  /** Print get-proof command */
  virtual void toStreamCmdGetProof(std::ostream& out,
                                   modes::ProofComponent c) const;

  /** Print get-instantiations command */
  void toStreamCmdGetInstantiations(std::ostream& out) const;

  /** Print get-unsat-assumptions command */
  virtual void toStreamCmdGetUnsatAssumptions(std::ostream& out) const;

  /** Print get-unsat-core command */
  virtual void toStreamCmdGetUnsatCore(std::ostream& out) const;

  /** Print get-assertions command */
  virtual void toStreamCmdGetAssertions(std::ostream& out) const;

  /** Print set-logic command */
  virtual void toStreamCmdSetBenchmarkLogic(std::ostream& out,
                                            const std::string& logic) const;

  /** Print set-info command */
  virtual void toStreamCmdSetInfo(std::ostream& out,
                                  const std::string& flag,
                                  const std::string& value) const;

  /** Print get-info command */
  virtual void toStreamCmdGetInfo(std::ostream& out,
                                  const std::string& flag) const;

  /** Print set-option command */
  virtual void toStreamCmdSetOption(std::ostream& out,
                                    const std::string& flag,
                                    const std::string& value) const;

  /** Print get-option command */
  virtual void toStreamCmdGetOption(std::ostream& out,
                                    const std::string& flag) const;

  /** Print set-expression-name command */
  void toStreamCmdSetExpressionName(std::ostream& out,
                                    Node n,
                                    const std::string& name) const;

  /** Print declare-datatype(s) command */
  virtual void toStreamCmdDatatypeDeclaration(
      std::ostream& out, const std::vector<TypeNode>& datatypes) const;

  /** Print reset command */
  virtual void toStreamCmdReset(std::ostream& out) const;

  /** Print reset-assertions command */
  virtual void toStreamCmdResetAssertions(std::ostream& out) const;

  /** Print quit command */
  virtual void toStreamCmdQuit(std::ostream& out) const;

  /** Declare heap command */
  virtual void toStreamCmdDeclareHeap(std::ostream& out,
                                      TypeNode locType,
                                      TypeNode dataType) const;

 protected:
  /** Derived classes can construct, but no one else. */
  Printer() {}

  /**
   * To stream model sort. This prints the appropriate output for type
   * tn declared via declare-sort.
   */
  virtual void toStreamModelSort(std::ostream& out,
                                 TypeNode tn,
                                 const std::vector<Node>& elements) const = 0;

  /**
   * To stream model term. This prints the appropriate output for term
   * n declared via declare-fun.
   */
  virtual void toStreamModelTerm(std::ostream& out,
                                 const Node& n,
                                 const Node& value) const = 0;

  /**
   * Write an error to `out` stating that command status `name` is not supported
   * by this printer.
   */
  void printUnknownCommandStatus(std::ostream& out,
                                 const std::string& name) const;

  /**
   * Write an error to `out` stating that command `name` is not supported by
   * this printer.
   */
  void printUnknownCommand(std::ostream& out, const std::string& name) const;

 private:
  /** Disallow copy, assignment  */
  Printer(const Printer&) = delete;
  Printer& operator=(const Printer&) = delete;

  /** Make a Printer for a given Language */
  static std::unique_ptr<Printer> makePrinter(Language lang);

}; /* class Printer */

}  // namespace ava6::internal

#endif /* AVA6__PRINTER__PRINTER_H */
