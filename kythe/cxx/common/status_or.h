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
#include "absl/types/optional.h"
#include "glog/logging.h"
#include "kythe/cxx/common/status.h"

namespace kythe {

/// \brief StatusOr contains either an error Status or a success T value.
template <typename T>
class ABSL_MUST_USE_RESULT StatusOr final {
 public:
  /// \brief `StatusOr<T>` is constructible from either `Status` or `T`.
  StatusOr(const Status& status) : status_(status) { DCHECK(!ok()); }
  StatusOr(Status&& status) : status_(std::move(status)) { DCHECK(!ok()); }
  StatusOr(const T& value) : value_(value) {}
  StatusOr(T&& value) : value_(std::move(value)) {}

  /// \brief `StatusOr` is both copyable and movable if `T` is.
  StatusOr(const StatusOr&) = default;
  StatusOr(StatusOr&&) = default;
  StatusOr& operator=(const StatusOr&) = default;
  StatusOr& operator=(StatusOr&&) = default;

  /// \brief `StatusOr<T>` is assignable from both `Status` or `T`.
  StatusOr& operator=(const Status& status);
  StatusOr& operator=(Status&& status);
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

  ABSL_MUST_USE_RESULT bool ok() const { return this->status_.ok(); }

#if defined(__clang__) && __clang_major__ == 3 && __clang_minor__ == 5
  // clang 3.5 will choose the incorrect overload if the ref-qualifiers
  // are present.
  // TODO: either bump the minimum version of clang we require or find a
  // reasonable fix for this.
  const Status& status() const { return this->status_; }
  Status status() { return this->status_; }
#else
  const Status& status() const & { return this->status_; }
  Status status() && { return std::move(this->status_); }
#endif

  explicit operator bool() const { return this->ok(); }

  const T* operator->() const { return this->value_.operator->(); }
  T* operator->() { return this->value_.operator->(); }

  const T& operator*() const& { return *this->value_; }
  T& operator*() & { return *this->value_; }
  const T&& operator*() const&& { return *std::move(this->value_); }
  T&& operator*() && { return *std::move(this->value_); }

 private:
  template <typename U>
  friend class StatusOr;

  // As we always have to have a Status reference for the accessor,
  // unconditionally include such a member.
  Status status_ = OkStatus();
  // But we also need to support non-default constructible T so defer
  // to `absl::optional` for the heavy lifting there.
  absl::optional<T> value_;
};

template <typename T>
StatusOr<T>& StatusOr<T>::operator=(const Status& status) {
  this->status_ = status;
  this->value_.reset();
  DCHECK(!this->ok());
  return *this;
}

template <typename T>
StatusOr<T>& StatusOr<T>::operator=(Status&& status) {
  this->status_ = std::move(status);
  this->value_.reset();
  DCHECK(!this->ok());
  return *this;
}

template <typename T>
StatusOr<T>& StatusOr<T>::operator=(const T& value) {
  this->value_ = value;
  this->status_ = OkStatus();
  return *this;
}

template <typename T>
StatusOr<T>& StatusOr<T>::operator=(T&& value) {
  this->value_ = std::move(value);
  this->status_ = OkStatus();
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

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_STATUS_OR_H_
