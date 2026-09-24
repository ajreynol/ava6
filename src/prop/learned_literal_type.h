#include "ava6_private.h"
#ifndef AVA6__PROP__LEARNED_LITERAL_TYPE_H
#define AVA6__PROP__LEARNED_LITERAL_TYPE_H
#include <ostream>
namespace ava6::internal::prop {
/** Internal classification used by zero-level lemma preprocessing. */
enum class LearnedLitType
{
  PREPROCESS_SOLVED, PREPROCESS, INPUT, SOLVABLE, CONSTANT_PROP, INTERNAL, UNKNOWN
};
inline std::ostream& operator<<(std::ostream& out, LearnedLitType type)
{
  switch (type)
  {
    case LearnedLitType::PREPROCESS_SOLVED: return out << "preprocess_solved";
    case LearnedLitType::PREPROCESS: return out << "preprocess";
    case LearnedLitType::INPUT: return out << "input";
    case LearnedLitType::SOLVABLE: return out << "solvable";
    case LearnedLitType::CONSTANT_PROP: return out << "constant_prop";
    case LearnedLitType::INTERNAL: return out << "internal";
    case LearnedLitType::UNKNOWN: return out << "unknown";
  }
  return out;
}
}
#endif
