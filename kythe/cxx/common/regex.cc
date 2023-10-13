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

#include "kythe/cxx/common/regex.h"

#include <memory>
#include <utility>
#include <vector>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "re2/re2.h"

namespace kythe {
namespace {

template <typename T>
union NoDestructor {
  T value;
  ~NoDestructor() {}
};

std::shared_ptr<const RE2> DefaultRegex() {
  static const NoDestructor<std::shared_ptr<const RE2>> kEmpty = {
      std::make_shared<RE2>("")};
  return kEmpty.value;
}

std::shared_ptr<const RE2::Set> DefaultSet() {
  static const NoDestructor<std::shared_ptr<const RE2::Set>> kEmpty = {[] {
    auto set = std::make_shared<RE2::Set>(RE2::Options(), RE2::UNANCHORED);
    set->Compile();
    return set;
  }()};
  return kEmpty.value;
}

RE2::Set CheckCompiled(RE2::Set set) {
  RE2::Set::ErrorInfo error;
  if (!set.Match("", nullptr, &error)) {
    if (error.kind == RE2::Set::kNotCompiled) {
      DLOG(FATAL) << "Uncompiled RE2::Set passed to RegexSet";
      CHECK(set.Compile()) << "Failed to compile RE2::Set";
    }
  }
  return set;
}
}  // namespace

absl::StatusOr<Regex> Regex::Compile(absl::string_view pattern,
                                     const RE2::Options& options) {
  std::shared_ptr<const RE2> re = std::make_shared<RE2>(pattern, options);
  if (!re->ok()) {
    return absl::InvalidArgumentError(re->error());
  }
  return Regex(std::move(re));
}

Regex::Regex() : re_(DefaultRegex()) {}

Regex::Regex(const RE2& re)
    : re_(std::make_shared<RE2>(re.pattern(), re.options())) {
  CHECK(re_->ok()) << "Cannot initialize Regex from invalid RE2: "
                   << re_->error();
}

Regex::Regex(Regex&& other) noexcept : re_(std::move(other.re_)) {
  other.re_ = DefaultRegex();
}

Regex& Regex::operator=(Regex&& other) noexcept {
  re_ = std::move(other.re_);
  other.re_ = DefaultRegex();
  return *this;
}

RegexSet::RegexSet() : set_(DefaultSet()) {}
RegexSet::RegexSet(RE2::Set set)
    : set_(std::make_shared<RE2::Set>(CheckCompiled(std::move(set)))) {}

RegexSet::RegexSet(RegexSet&& other) noexcept : set_(std::move(other.set_)) {
  other.set_ = DefaultSet();
}

RegexSet& RegexSet::operator=(RegexSet&& other) noexcept {
  set_ = std::move(other.set_);
  other.set_ = DefaultSet();
  return *this;
}

absl::StatusOr<std::vector<int>> RegexSet::ExplainMatch(
    absl::string_view value) const {
  RE2::Set::ErrorInfo error;
  std::vector<int> matches;
  if (set_->Match(value, &matches, &error)) {
    return matches;
  }
  matches.clear();
  switch (error.kind) {
    case RE2::Set::kNoError:
      return matches;  // Empty match == no error, but no matches.
    case RE2::Set::kNotCompiled:
      // Shouldn't happen.
      return absl::InternalError("Match() called on uncompiled Set");
    case RE2::Set::kOutOfMemory:
      return absl::ResourceExhaustedError("Match() ran out of memory");
    case RE2::Set::kInconsistent:
      return absl::InternalError("RE2::Match() had inconsistent result");
  }
  return absl::UnknownError("");
}

}  // namespace kythe
