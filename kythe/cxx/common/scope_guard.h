/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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

#ifndef KYTHE_CXX_COMMON_SCOPE_GUARD_H
#define KYTHE_CXX_COMMON_SCOPE_GUARD_H

#include <utility>

#include "absl/base/attributes.h"
#include "absl/log/check.h"

namespace kythe {

/// \brief A move-only RAII object that calls a stored cleanup functor when
/// destroyed.
template <typename F>
class ABSL_MUST_USE_RESULT ScopeGuard {
 public:
  explicit ScopeGuard(F&& Fn) : Fn(std::forward<F>(Fn)) {}
  ScopeGuard(ScopeGuard&& O) : Released(O.Released), Fn(std::move(O.Fn)) {
    O.Released = true;
  }
  ~ScopeGuard() {
    if (!Released) Fn();
  }

 private:
  bool Released = false;
  F Fn;
};

/// \brief Returns a type-deduced ScopeGuard for the provided function object.
template <typename F>
ScopeGuard<F> MakeScopeGuard(F&& Fn) {
  return ScopeGuard<F>(std::forward<F>(Fn));
}

/// \brief Restores the type of a stacklike container of `ElementType`.
template <typename StackType>
struct StackSizeRestorer {
  void operator()() const {
    CHECK_LE(Size, Target->size());
    while (Size < Target->size()) {
      Target->pop_back();
    }
  }

  StackType* Target;
  decltype(Target->size()) Size;
};

/// \brief Handles the restoration of a stacklike container.
///
/// \example
/// \code
///   auto R = RestoreStack(SomeStack);
/// \endcode
template <typename StackType>
ScopeGuard<StackSizeRestorer<StackType>> RestoreStack(StackType& S) {
  return ScopeGuard<StackSizeRestorer<StackType>>({&S, S.size()});
}

/// \brief Pops the final element off the Target stack at destruction.
template <typename StackType>
struct BackPopper {
  void operator()() const { Target->pop_back(); }

  StackType* Target;
};

/// \brief Pushes the value onto the stack and returns a sentinel which will
/// remove it at destruction.
template <typename StackType,
          typename ValueType = typename StackType::value_type>
ScopeGuard<BackPopper<StackType>> PushScope(StackType& Target,
                                            ValueType&& Value) {
  Target.push_back(std::forward<ValueType>(Value));
  return ScopeGuard<BackPopper<StackType>>({&Target});
}

/// \brief Restores the recorded value.
template <typename ValueType>
struct ValueRestorer {
  void operator()() const { *Target = Value; }

  ValueType* Target;
  ValueType Value;
};

/// \brief Records the current value of the target and returns a sentinel
/// which will restore it upon destruction.
template <typename ValueType>
ScopeGuard<ValueRestorer<ValueType>> RestoreValue(ValueType& Value) {
  return ScopeGuard<ValueRestorer<ValueType>>({&Value, Value});
}

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_SCOPE_GUARD_H
