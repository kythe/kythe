/*
 * Copyright 2020 The Kythe Authors. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <memory>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "re2/re2.h"
#include "re2/set.h"

#ifndef KYTHE_CXX_COMMON_REGEX_H_
#define KYTHE_CXX_COMMON_REGEX_H_

namespace kythe {

/// \brief Regex is a Regular value type implemented on top of RE2.
class Regex {
 public:
  /// \brief Compiles the pattern into a Regex with the provided options.
  static absl::StatusOr<Regex> Compile(
      absl::string_view pattern,
      const RE2::Options& options = RE2::DefaultOptions);

  /// \brief Constructs a Regex from an already-compiled RE2 object.
  /// Requires: re.ok()
  explicit Regex(const RE2& re);

  /// \brief Constructs an empty Regex.
  Regex();

  /// \brief Regex is copyable.
  Regex(const Regex&) = default;
  Regex& operator=(const Regex&) = default;

  /// \brief Regex is movable.
  /// Moves leave the moved-from object in a default constructed state.
  Regex(Regex&&) noexcept;
  Regex& operator=(Regex&&) noexcept;

  /// \brief Retrieves the underlying RE2 object to be compatible with the RE2
  /// non-member functions.
  operator const RE2&() const { return *re_; }

 private:
  explicit Regex(std::shared_ptr<const RE2> re) : re_(std::move(re)) {}

  std::shared_ptr<const RE2> re_;  // non-null.
};

/// \brief RegexSet is a regular value-type wrapper around RE2::Set.
class RegexSet {
 public:
  /// \brief Builds a RegexSet from the list of patterns and options.
  template <typename Range = absl::Span<const absl::string_view>>
  static absl::StatusOr<RegexSet> Build(
      Range&& patterns, const RE2::Options& = RE2::DefaultOptions,
      RE2::Anchor = RE2::UNANCHORED);

  /// \brief Constructs a RegexSet from the extant RE2::Set.
  /// Requires: set has been compiled
  explicit RegexSet(RE2::Set set);

  /// \brief Default constructs an empty RegexSet. Matches nothing.
  RegexSet();

  /// \brief Regex set is copyable.
  RegexSet(const RegexSet& other) = default;
  RegexSet& operator=(const RegexSet& other) = default;

  /// \brief Regex set is movable.
  /// Moves leave the moved-from object in a default constructed state.
  RegexSet(RegexSet&& other) noexcept;
  RegexSet& operator=(RegexSet&& other) noexcept;

  /// \brief Returns true if the provided value matches one of the contained
  /// regular expressions.
  bool Match(absl::string_view value) const {
    return set_->Match(value, nullptr);
  }

  /// \brief Matches the input against the contained regular expressions,
  /// returning the indices at which the value matched or an empty vector if it
  /// did not.
  absl::StatusOr<std::vector<int>> ExplainMatch(absl::string_view value) const;

 private:
  std::shared_ptr<const RE2::Set> set_;  // non-null.
};

template <typename Range>
absl::StatusOr<RegexSet> RegexSet::Build(Range&& patterns,
                                         const RE2::Options& options,
                                         RE2::Anchor anchor) {
  RE2::Set set(options, anchor);
  for (const auto& value : std::forward<Range>(patterns)) {
    std::string error;
    if (set.Add(value, &error) == -1) {
      return absl::InvalidArgumentError(error);
    }
  }
  if (!set.Compile()) {
    return absl::ResourceExhaustedError(
        "Out of memory attempting to compile RegexSet");
  }
  return RegexSet(std::move(set));
}

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_REGEX_H_
