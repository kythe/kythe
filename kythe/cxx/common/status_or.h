/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

#ifndef KYTHE_CXX_COMMON_STATUS_OR_H_
#define KYTHE_CXX_COMMON_STATUS_OR_H_

#include "absl/base/attributes.h"
#include "absl/status/status.h"
#include "absl/types/optional.h"
#include "glog/logging.h"

namespace kythe {

/// \brief StatusOr contains either an error Status or a success T value.
template <typename T>
class ABSL_MUST_USE_RESULT StatusOr final {
 public:
  /// \brief `StatusOr<T>` is constructible from either `Status` or `T`.
  StatusOr(const absl::Status& status) : status_(status) { DCHECK(!ok()); }
  StatusOr(absl::Status&& status) : status_(std::move(status)) { DCHECK(!ok()); }
  StatusOr(const T& value) : value_(value) {}
  StatusOr(T&& value) : value_(std::move(value)) {}

  /// \brief `StatusOr` is both copyable and movable if `T` is.
  StatusOr(const StatusOr&) = default;
  StatusOr(StatusOr&&) = default;
  StatusOr& operator=(const StatusOr&) = default;
  StatusOr& operator=(StatusOr&&) = default;

  /// \brief `StatusOr<T>` is assignable from both `Status` or `T`.
  StatusOr& operator=(const absl::Status& status);
  StatusOr& operator=(absl::Status&& status);
  StatusOr& operator=(const T& value);
  StatusOr& operator=(T&& value);

  /// \brief Allow conversion from a compatible StatusOr<U>.
  template <typename U>
  StatusOr(const StatusOr<U>& other);
  template <typename U>
  StatusOr(StatusOr<U>&& other);

  /// \brief Allow conversion from a compatible StatusOr<U>.
  template <typename U>
  StatusOr& operator=(const StatusOr<U>& other);
  template <typename U>
  StatusOr& operator=(StatusOr<U>&& other);

  /// \brief Returns whether or not the status is OK (and thus the StatusOr
  ///        contains a value).
  ABSL_MUST_USE_RESULT bool ok() const { return this->status().ok(); }

  /// \brief Returns the contained status.
  const absl::Status& status() const& { return this->status_; }
  absl::Status status() && { return std::move(this->status_); }

  /// \brief Equivalent to this->ok().
  explicit operator bool() const { return this->ok(); }

  /// \brief Accesses the contained value. Undefined behavior if !ok().
  const T* operator->() const { return this->value_.operator->(); }
  T* operator->() { return this->value_.operator->(); }

  /// \brief Accesses the contained value. Undefined behavior if !ok().
  const T& operator*() const& { return *this->value_; }
  T& operator*() & { return *this->value_; }
  const T&& operator*() const&& { return *std::move(this->value_); }
  T&& operator*() && { return *std::move(this->value_); }

  /// \brief Accesses the contained value.  CHECK-fail if !ok().
  const T& ValueOrDie() const& {
    this->EnsureOk();
    return *this->value_;
  }
  T& ValueOrDie() & {
    this->EnsureOk();
    return *this->value_;
  }
  const T&& ValueOrDie() const&& {
    this->EnsureOk();
    return *std::move(this->value_);
  }
  T&& ValueOrDie() && {
    this->EnsureOk();
    return *std::move(this->value_);
  }

  // Returns a copy of the current value if this->ok() == true. Otherwise
  // returns a default value.
  template <typename U>
  T value_or(U&& default_value) const&;
  template <typename U>
  T value_or(U&& default_value) &&;

 private:
  template <typename U>
  friend class StatusOr;

  // CHECK-fail with a reasonable message if !ok().
  void EnsureOk() const;

  // As we always have to have a Status reference for the accessor,
  // unconditionally include such a member.
  absl::Status status_ = absl::OkStatus();
  // But we also need to support non-default constructible T so defer
  // to `absl::optional` for the heavy lifting there.
  absl::optional<T> value_;
};

template <typename T>
StatusOr<T>& StatusOr<T>::operator=(const absl::Status& status) {
  this->status_ = status;
  this->value_.reset();
  DCHECK(!this->ok());
  return *this;
}

template <typename T>
StatusOr<T>& StatusOr<T>::operator=(absl::Status&& status) {
  this->status_ = std::move(status);
  this->value_.reset();
  DCHECK(!this->ok());
  return *this;
}

template <typename T>
StatusOr<T>& StatusOr<T>::operator=(const T& value) {
  this->value_ = value;
  this->status_ = absl::OkStatus();
  return *this;
}

template <typename T>
StatusOr<T>& StatusOr<T>::operator=(T&& value) {
  this->value_ = std::move(value);
  this->status_ = absl::OkStatus();
  return *this;
}

template <typename T>
template <typename U>
StatusOr<T>::StatusOr(const StatusOr<U>& other)
    : status_(other.status_), value_(other.value_) {}

template <typename T>
template <typename U>
StatusOr<T>::StatusOr(StatusOr<U>&& other)
    : status_(std::move(other.status_)), value_(std::move(other.value_)) {}

template <typename T>
template <typename U>
StatusOr<T>& StatusOr<T>::operator=(const StatusOr<U>& other) {
  this->status_ = other.status_;
  this->value_ = other.value_;
}

template <typename T>
template <typename U>
StatusOr<T>& StatusOr<T>::operator=(StatusOr<U>&& other) {
  this->status_ = std::move(other.status_);
  this->value_ = std::move(other.value_);
  return *this;
}

template <typename T>
template <typename U>
T StatusOr<T>::value_or(U&& default_value) const& {
  if (ok()) {
    return **this;
  }
  return std::forward<U>(default_value);
}

template <typename T>
template <typename U>
T StatusOr<T>::value_or(U&& default_value) && {
  if (ok()) {
    return *std::move(*this);
  }
  return std::forward<U>(default_value);
}

template <typename T>
void StatusOr<T>::EnsureOk() const {
  CHECK(ok()) << "Attempting to fetch value instead of handling error "
              << status();
}

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_STATUS_OR_H_
