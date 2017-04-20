/*
 * Copyright 2016 Google Inc. All rights reserved.
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

// This file uses the Clang style conventions.

#ifndef KYTHE_CXX_COMMON_INDEXING_MAYBE_FEW_H_
#define KYTHE_CXX_COMMON_INDEXING_MAYBE_FEW_H_

#include <functional>
#include <utility>
#include <vector>

#include "glog/logging.h"

namespace kythe {

/// \brief None() may be used to construct an empty `MaybeFew<T>`.
struct None {};

/// \brief A class containing zero or more instances of copyable `T`. When there
/// are more than zero elements, the first is distinguished as the `primary`.
///
/// This class is primarily used to hold type NodeIds, as the process for
/// deriving one may fail (if the type is unsupported) or may return more than
/// one possible ID (if the type is noncanonical).
template <typename T> class MaybeFew {
 public:
  /// \brief Constructs a new `MaybeFew` holding zero or more elements.
  /// If there are more than zero elements, the first one provided is primary.
  template <typename... Ts>
  MaybeFew(Ts &&... Init) : Content({std::forward<T>(Init)...}) {}

  /// \brief Constructs an empty `MaybeFew`.
  ///
  /// This is meant to be used in contexts where you would like the compiler
  /// to deduce the MaybeFew's template parameter; for example, in a function
  /// returning `MaybeFew<S>`, you may `return None()` without repeating `S`.
  MaybeFew(None) {}

  /// \brief Returns true iff there are elements.
  explicit operator bool() const { return !Content.empty(); }

  /// \brief Returns the primary element (the first one provided during
  /// construction).
  const T &primary() const {
    CHECK(!Content.empty());
    return Content[0];
  }

  /// \brief Take all of the elements previously stored. The `MaybeFew` remains
  /// in an indeterminate state afterward.
  std::vector<T> &&takeAll() { return std::move(Content); }

  /// \brief Returns a reference to the internal vector holding the elements.
  /// If the vector is non-empty, the first element is primary.
  const std::vector<T> &all() { return Content; }

  /// \brief Creates a new `MaybeFew` by applying `Fn` to each element of this
  /// one. Order is preserved (st `A.Map(Fn).all()[i] = Fn(A.all()[i])`); also,
  /// `A.Map(Fn).all().size() == A.all().size()`.
  template <typename S>
  MaybeFew<S> Map(const std::function<S(const T &)> &Fn) const {
    MaybeFew<S> Result;
    Result.Content.reserve(Content.size());
    for (const auto &E : Content) {
      Result.Content.push_back(Fn(E));
    }
    return Result;
  }

  /// \brief Apply `Fn` to each stored element starting with the primary.
  void Iter(const std::function<void(const T &)> &Fn) const {
    for (const auto &E : Content) {
      Fn(E);
    }
  }

 private:
  /// The elements stored in this value, where `Content[0]` is the primary
  /// element (if any elements exist at all).
  std::vector<T> Content;
};

/// \brief Constructs a `MaybeFew<T>` given a single `T`.
template <typename T> MaybeFew<T> Some(T &&t) {
  return MaybeFew<T>(std::forward<T>(t));
}

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_INDEXING_MAYBE_FEW_H_
