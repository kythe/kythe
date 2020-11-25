#ifndef THIRD_PARTY_SOUFFLE_PARSE_TRANSFORM_H_
#define THIRD_PARTY_SOUFFLE_PARSE_TRANSFORM_H_

#include "ram/TranslationUnit.h"

#include <memory>
#include <string>

namespace souffle {
/// \brief Parses and transforms `code` into a `ram::TranslationUnit`.
/// \param code the Souffle program to transform.
/// \return the resulting translation unit or null on error.
std::unique_ptr<ram::TranslationUnit> ParseTransform(const std::string& code);
}  // namespace souffle

#endif  // defined(THIRD_PARTY_SOUFFLE_PARSE_TRANSFORM_H_)