/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Sets theory type rules.
 */

#include "theory/sets/theory_sets_type_rules.h"

#include <sstream>

#include "expr/dtype.h"
#include "expr/dtype_cons.h"
#include "theory/datatypes/project_op.h"
#include "theory/datatypes/tuple_utils.h"
#include "theory/sets/normal_form.h"
#include "util/cardinality.h"

namespace ava6::internal {
namespace theory {
namespace sets {

using namespace ava6::internal::theory::datatypes;

bool checkFunctionTypeFor(const Node& n,
                          const TypeNode& functionType,
                          const TypeNode& setType,
                          std::ostream* errOut)
{
  // get the element type of the second argument, if it exists
  TypeNode elementType;
  if (setType.isSet())
  {
    elementType = setType.getSetElementType();
  }
  if (!functionType.isMaybeKind(Kind::FUNCTION_TYPE))
  {
    if (errOut)
    {
      (*errOut) << "Operator " << n.getKind()
                << " expects a function as a first argument. "
                << "Found a term of type '" << functionType << "'.";
    }
    return false;
  }
  // note that if functionType is abstract, we don't check whether it
  // matches the argument.
  if (functionType.isFunction())
  {
    std::vector<TypeNode> argTypes = functionType.getArgTypes();
    if (!(argTypes.size() == 1
          && (elementType.isNull() || argTypes[0].isComparableTo(elementType))))
    {
      if (errOut)
      {
        (*errOut) << "Operator " << n.getKind()
                  << " expects a function whose type is comparable to the "
                     "type of elements in the set";
        if (!elementType.isNull())
        {
          (*errOut) << " (" << elementType << ")";
        }
        (*errOut) << ". Found a function of type '" << functionType << "'.";
      }
      return false;
    }
  }
  return true;
}

TypeNode SetsBinaryOperatorTypeRule::preComputeType(AVA6_UNUSED NodeManager* nm,
                                                    AVA6_UNUSED TNode n)
{
  return TypeNode::null();
}
TypeNode SetsBinaryOperatorTypeRule::computeType(NodeManager* nodeManager,
                                                 TNode n,
                                                 bool check,
                                                 std::ostream* errOut)
{
  Assert(n.getKind() == Kind::SET_UNION || n.getKind() == Kind::SET_INTER
         || n.getKind() == Kind::SET_MINUS);
  TypeNode setType = n[0].getTypeOrNull();
  TypeNode secondSetType = n[1].getTypeOrNull();
  TypeNode retType = setType.leastUpperBound(secondSetType);
  if (check)
  {
    if (!setType.isMaybeKind(Kind::SET_TYPE)
        || !secondSetType.isMaybeKind(Kind::SET_TYPE))
    {
      if (errOut)
      {
        (*errOut) << "operator expects a set, argument is not";
      }
      return TypeNode::null();
    }
  }
  if (retType.isNull())
  {
    if (errOut)
    {
      (*errOut) << "Operator " << n.getKind()
                << " expects two sets of comparable type. Found types '"
                << setType << "' and '" << secondSetType << "'.";
    }
    return TypeNode::null();
  }
  // we are ?Set if both children are fully abstract
  if (retType.isFullyAbstract())
  {
    return nodeManager->mkAbstractType(Kind::SET_TYPE);
  }
  return retType;
}

bool SetsBinaryOperatorTypeRule::computeIsConst(
    AVA6_UNUSED NodeManager* nodeManager, TNode n)
{
  // only SET_UNION has a const rule in kinds.
  // SET_INTER and SET_MINUS are not used in the canonical representation
  // of sets and therefore they do not have const rules in kinds
  Assert(n.getKind() == Kind::SET_UNION);
  return NormalForm::checkNormalConstant(n);
}

TypeNode SubsetTypeRule::preComputeType(NodeManager* nm, AVA6_UNUSED TNode n)
{
  return nm->booleanType();
}
TypeNode SubsetTypeRule::computeType(NodeManager* nodeManager,
                                     TNode n,
                                     bool check,
                                     std::ostream* errOut)
{
  Assert(n.getKind() == Kind::SET_SUBSET);
  TypeNode setType = n[0].getTypeOrNull();
  if (check)
  {
    TypeNode secondSetType = n[1].getTypeOrNull();
    if (!setType.isMaybeKind(Kind::SET_TYPE)
        || !secondSetType.isMaybeKind(Kind::SET_TYPE))
    {
      if (errOut)
      {
        (*errOut) << "set subset operating on non-set";
      }
      return TypeNode::null();
    }
    if (!secondSetType.isComparableTo(setType))
    {
      if (errOut)
      {
        (*errOut) << "set subset operating on sets of incomparable types";
      }
      return TypeNode::null();
    }
  }
  return nodeManager->booleanType();
}

TypeNode MemberTypeRule::preComputeType(NodeManager* nm, AVA6_UNUSED TNode n)
{
  return nm->booleanType();
}
TypeNode MemberTypeRule::computeType(NodeManager* nodeManager,
                                     TNode n,
                                     bool check,
                                     std::ostream* errOut)
{
  Assert(n.getKind() == Kind::SET_MEMBER);
  TypeNode setType = n[1].getTypeOrNull();
  if (check)
  {
    if (!setType.isMaybeKind(Kind::SET_TYPE))
    {
      if (errOut)
      {
        (*errOut) << "checking for membership in a non-set";
      }
      return TypeNode::null();
    }
    TypeNode elementType = n[0].getTypeOrNull();
    if (!elementType.isComparableTo(setType.getSetElementType()))
    {
      if (errOut)
      {
        (*errOut) << "member operating on sets of different types:\n"
                  << "child type:  " << elementType << "\n"
                  << "not type: " << setType.getSetElementType() << "\n"
                  << "in term : " << n;
      }
      return TypeNode::null();
    }
  }
  return nodeManager->booleanType();
}

TypeNode SingletonTypeRule::preComputeType(AVA6_UNUSED NodeManager* nm,
                                           AVA6_UNUSED TNode n)
{
  return TypeNode::null();
}
TypeNode SingletonTypeRule::computeType(NodeManager* nodeManager,
                                        TNode n,
                                        AVA6_UNUSED bool check,
                                        AVA6_UNUSED std::ostream* errOut)
{
  Assert(n.getKind() == Kind::SET_SINGLETON);
  TypeNode type1 = n[0].getTypeOrNull();
  return nodeManager->mkSetType(type1);
}

bool SingletonTypeRule::computeIsConst(AVA6_UNUSED NodeManager* nodeManager,
                                       TNode n)
{
  Assert(n.getKind() == Kind::SET_SINGLETON);
  return n[0].isConst();
}

TypeNode EmptySetTypeRule::preComputeType(AVA6_UNUSED NodeManager* nm,
                                          AVA6_UNUSED TNode n)
{
  return TypeNode::null();
}
TypeNode EmptySetTypeRule::computeType(AVA6_UNUSED NodeManager* nodeManager,
                                       TNode n,
                                       AVA6_UNUSED bool check,
                                       AVA6_UNUSED std::ostream* errOut)
{
  Assert(n.getKind() == Kind::SET_EMPTY);
  EmptySet emptySet = n.getConst<EmptySet>();
  return emptySet.getType();
}

TypeNode ComprehensionTypeRule::preComputeType(AVA6_UNUSED NodeManager* nm,
                                               AVA6_UNUSED TNode n)
{
  return TypeNode::null();
}
TypeNode ComprehensionTypeRule::computeType(NodeManager* nodeManager,
                                            TNode n,
                                            bool check,
                                            std::ostream* errOut)
{
  Assert(n.getKind() == Kind::SET_COMPREHENSION);
  if (check)
  {
    if (n[0].getKind() != Kind::BOUND_VAR_LIST)
    {
      if (errOut)
      {
        (*errOut)
            << "first argument of set comprehension is not bound var list";
      }
      return TypeNode::null();
    }
    TypeNode bt = n[1].getTypeOrNull();
    if (!bt.isBoolean() && !bt.isFullyAbstract())
    {
      if (errOut)
      {
        (*errOut) << "body of set comprehension is not Boolean";
      }
      return TypeNode::null();
    }
  }
  return nodeManager->mkSetType(n[2].getTypeOrNull());
}

TypeNode ChooseTypeRule::preComputeType(AVA6_UNUSED NodeManager* nm,
                                        AVA6_UNUSED TNode n)
{
  return TypeNode::null();
}
TypeNode ChooseTypeRule::computeType(NodeManager* nodeManager,
                                     TNode n,
                                     bool check,
                                     std::ostream* errOut)
{
  Assert(n.getKind() == Kind::SET_CHOOSE);
  TypeNode setType = n[0].getTypeOrNull();
  if (check)
  {
    if (!setType.isMaybeKind(Kind::SET_TYPE))
    {
      if (errOut)
      {
        (*errOut) << "SET_CHOOSE operator expects a set, a non-set is found";
      }
      return TypeNode::null();
    }
  }
  if (setType.isAbstract())
  {
    // don't know the element type, return the fully abstract type
    return nodeManager->mkAbstractType(Kind::ABSTRACT_TYPE);
  }
  return setType.getSetElementType();
}

TypeNode IsSetTypeRule::preComputeType(NodeManager* nm, AVA6_UNUSED TNode n)
{
  return nm->booleanType();
}
TypeNode IsSetTypeRule::computeType(NodeManager* nodeManager,
                                    TNode n,
                                    bool check,
                                    std::ostream* errOut)
{
  Assert(n.getKind() == Kind::SET_IS_EMPTY
         || n.getKind() == Kind::SET_IS_SINGLETON);
  TypeNode setType = n[0].getTypeOrNull();
  if (check)
  {
    if (!setType.isMaybeKind(Kind::SET_TYPE))
    {
      if (errOut)
      {
        (*errOut) << n.getKind()
                  << " operator expects a set, a non-set is found";
      }
      return TypeNode::null();
    }
  }
  return nodeManager->booleanType();
}

TypeNode InsertTypeRule::preComputeType(AVA6_UNUSED NodeManager* nm,
                                        AVA6_UNUSED TNode n)
{
  return TypeNode::null();
}
TypeNode InsertTypeRule::computeType(NodeManager* nodeManager,
                                     TNode n,
                                     bool check,
                                     std::ostream* errOut)
{
  Assert(n.getKind() == Kind::SET_INSERT);
  size_t numChildren = n.getNumChildren();
  Assert(numChildren >= 2);
  TypeNode setType = n[numChildren - 1].getTypeOrNull();
  if (check)
  {
    if (!setType.isMaybeKind(Kind::SET_TYPE))
    {
      if (errOut)
      {
        (*errOut) << "inserting into a non-set";
      }
      return TypeNode::null();
    }
  }
  // returned element type, which is the join of all elements and the element
  // type of the set (if it exists).
  TypeNode retElementType;
  if (setType.isSet())
  {
    retElementType = setType.getSetElementType();
  }
  for (size_t i = 0; i < numChildren - 1; ++i)
  {
    TypeNode elementType = n[i].getTypeOrNull();
    retElementType = retElementType.isNull()
                         ? elementType
                         : retElementType.leastUpperBound(elementType);
    if (retElementType.isNull())
    {
      if (errOut)
      {
        (*errOut) << "type of element should be same as element type of set "
                     "being inserted into";
      }
      return TypeNode::null();
    }
  }
  Assert(!retElementType.isNull());
  return nodeManager->mkSetType(retElementType);
}

TypeNode SetMapTypeRule::preComputeType(AVA6_UNUSED NodeManager* nm,
                                        AVA6_UNUSED TNode n)
{
  return TypeNode::null();
}
TypeNode SetMapTypeRule::computeType(NodeManager* nodeManager,
                                     TNode n,
                                     bool check,
                                     std::ostream* errOut)
{
  Assert(n.getKind() == Kind::SET_MAP);
  TypeNode functionType = n[0].getTypeOrNull();
  TypeNode setType = n[1].getTypeOrNull();
  if (check)
  {
    if (!setType.isMaybeKind(Kind::SET_TYPE))
    {
      if (errOut)
      {
        (*errOut) << "set.map operator expects a set in the second argument, "
                     "a non-set is found";
      }
      return TypeNode::null();
    }
    if (!checkFunctionTypeFor(n, functionType, setType, errOut))
    {
      return TypeNode::null();
    }
  }
  TypeNode rangeType;
  if (functionType.isFunction())
  {
    rangeType = functionType.getRangeType();
  }
  else
  {
    // if an abstract function, the element type is fully abstract
    rangeType = nodeManager->mkAbstractType(Kind::ABSTRACT_TYPE);
  }
  return nodeManager->mkSetType(rangeType);
}

TypeNode SetFilterTypeRule::preComputeType(AVA6_UNUSED NodeManager* nm,
                                           AVA6_UNUSED TNode n)
{
  return TypeNode::null();
}
TypeNode SetFilterTypeRule::computeType(AVA6_UNUSED NodeManager* nodeManager,
                                        TNode n,
                                        bool check,
                                        std::ostream* errOut)
{
  Assert(n.getKind() == Kind::SET_FILTER);
  TypeNode functionType = n[0].getTypeOrNull();
  TypeNode setType = n[1].getTypeOrNull();
  if (check)
  {
    if (!setType.isMaybeKind(Kind::SET_TYPE))
    {
      if (errOut)
      {
        (*errOut) << "set.filter operator expects a set in the second "
                     "argument, a non-set is found";
      }
      return TypeNode::null();
    }
    if (!checkFunctionTypeFor(n, functionType, setType, errOut))
    {
      return TypeNode::null();
    }
    if (functionType.isFunction())
    {
      TypeNode rangeType = functionType.getRangeType();
      if (!rangeType.isBoolean() && !rangeType.isFullyAbstract())
      {
        if (errOut)
        {
          (*errOut) << "Operator set.filter expects a function returning Bool.";
        }
        return TypeNode::null();
      }
    }
  }
  return setType;
}

TypeNode SetAllSomeTypeRule::preComputeType(NodeManager* nm,
                                            AVA6_UNUSED TNode n)
{
  return nm->booleanType();
}

TypeNode SetAllSomeTypeRule::computeType(NodeManager* nodeManager,
                                         TNode n,
                                         bool check,
                                         std::ostream* errOut)
{
  Assert(n.getKind() == Kind::SET_ALL || n.getKind() == Kind::SET_SOME);
  std::string op = n.getKind() == Kind::SET_ALL ? "set.all" : "set.some";
  TypeNode functionType = n[0].getTypeOrNull();
  TypeNode setType = n[1].getTypeOrNull();
  if (check)
  {
    if (!setType.isMaybeKind(Kind::SET_TYPE))
    {
      if (errOut)
      {
        (*errOut) << op
                  << " operator expects a set in the second "
                     "argument, a non-set is found";
      }
      return TypeNode::null();
    }
    if (!checkFunctionTypeFor(n, functionType, setType, errOut))
    {
      return TypeNode::null();
    }
    if (functionType.isFunction())
    {
      TypeNode rangeType = functionType.getRangeType();
      if (!rangeType.isBoolean() && !rangeType.isFullyAbstract())
      {
        if (errOut)
        {
          (*errOut) << "Operator " << op
                    << " expects a function returning Bool.";
        }
        return TypeNode::null();
      }
    }
  }
  return nodeManager->booleanType();
}

TypeNode SetFoldTypeRule::preComputeType(AVA6_UNUSED NodeManager* nm,
                                         AVA6_UNUSED TNode n)
{
  return TypeNode::null();
}
TypeNode SetFoldTypeRule::computeType(NodeManager* nodeManager,
                                      TNode n,
                                      bool check,
                                      std::ostream* errOut)
{
  Assert(n.getKind() == Kind::SET_FOLD);
  TypeNode functionType = n[0].getTypeOrNull();
  TypeNode setType = n[2].getTypeOrNull();
  if (check)
  {
    if (!setType.isSet())
    {
      if (errOut)
      {
        (*errOut) << "set.fold operator expects a set in the third argument, "
                     "a non-set is found";
      }
      return TypeNode::null();
    }

    TypeNode elementType = setType.getSetElementType();

    if (!functionType.isFunction())
    {
      if (errOut)
      {
        (*errOut) << "Operator " << n.getKind()
                  << " expects a function of type  (-> " << elementType
                  << " T2 T2) as a first argument. "
                  << "Found a term of type '" << functionType << "'.";
      }
      return TypeNode::null();
    }
    std::vector<TypeNode> argTypes = functionType.getArgTypes();
    TypeNode rangeType = functionType.getRangeType();
    if (!(argTypes.size() == 2 && argTypes[0] == elementType
          && argTypes[1] == rangeType))
    {
      if (errOut)
      {
        (*errOut) << "Operator " << n.getKind()
                  << " expects a function of type  (-> " << elementType
                  << " T2 T2). "
                  << "Found a function of type '" << functionType << "'.";
      }
      return TypeNode::null();
    }
    TypeNode initialValueType = n[1].getTypeOrNull();
    if (rangeType != initialValueType)
    {
      if (errOut)
      {
        (*errOut) << "Operator " << n.getKind()
                  << " expects an initial value of type " << rangeType
                  << ". Found a term of type '" << initialValueType << "'.";
      }
      return TypeNode::null();
    }
  }
  if (functionType.isAbstract())
  {
    // if an abstract function, the element type is fully abstract
    return nodeManager->mkAbstractType(Kind::ABSTRACT_TYPE);
  }
  return functionType.getRangeType();
}

TypeNode SetEmptyOfTypeTypeRule::preComputeType(AVA6_UNUSED NodeManager* nm,
                                                AVA6_UNUSED TNode n)
{
  return TypeNode::null();
}

TypeNode SetEmptyOfTypeTypeRule::computeType(NodeManager* nm,
                                             AVA6_UNUSED TNode n,
                                             AVA6_UNUSED bool check,
                                             AVA6_UNUSED std::ostream* errOut)
{
  return nm->mkAbstractType(Kind::SET_TYPE);
}

Cardinality SetsProperties::computeCardinality(TypeNode type)
{
  Assert(type.getKind() == Kind::SET_TYPE);
  Cardinality elementCard = 2;
  elementCard ^= type[0].getCardinality();
  return elementCard;
}

bool SetsProperties::isWellFounded(TypeNode type)
{
  return type[0].isWellFounded();
}

Node SetsProperties::mkGroundTerm(TypeNode type)
{
  Assert(type.isSet());
  return type.getNodeManager()->mkConst(EmptySet(type));
}

}  // namespace sets
}  // namespace theory
}  // namespace ava6::internal
