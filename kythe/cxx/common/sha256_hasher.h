/*
 * Copyright 2023 The Kythe Authors. All rights reserved.
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

#ifndef KYTHE_CXX_COMMON_SHA256_HASHER_H_
#define KYTHE_CXX_COMMON_SHA256_HASHER_H_

#include <openssl/sha.h>

#include <array>
#include <cstddef>
#include <string>

#include "absl/strings/string_view.h"
#include "absl/types/span.h"

namespace kythe {

/// \brief Encapsulates BoringSSL SHA256 hashing in a friendly interface.
class Sha256Hasher {
  /// \brief An internal type for handling a variety of valid "byte"
  /// representations without the need for client-side reinterpret casts.
  class ByteView {
    template <typename... Ts>
    struct first_convertible {
      template <typename From, typename T>
      struct is_convertible : std::is_convertible<From, T> {
        using type = T;
      };
      template <typename From>
      using from = typename std::disjunction<is_convertible<From, Ts>...>::type;
    };

   public:
    template <typename T, typename Dest = first_convertible<
                              absl::string_view, absl::Span<const std::byte>,
                              absl::Span<const unsigned char>,
                              absl::Span<const char>>::from<T>>
    ByteView(T&& data)  // NOLINT
        : ByteView(Dest(data)) {}

    ByteView(absl::string_view data) : ByteView(data.data(), data.size()) {}
    ByteView(absl::Span<const std::byte> data)
        : ByteView(data.data(), data.size()) {}
    ByteView(absl::Span<const unsigned char> data)
        : ByteView(data.data(), data.size()) {}
    ByteView(absl::Span<const char> data)
        : ByteView(data.data(), data.size()) {}
    ByteView(const std::byte* data, std::size_t size)
        : ByteView(reinterpret_cast<const unsigned char*>(data), size) {}
    ByteView(const char* data, std::size_t size)
        : ByteView(reinterpret_cast<const unsigned char*>(data), size) {}
    ByteView(const unsigned char* data, std::size_t size) : data_(data, size) {}

    auto data() const { return data_.data(); }
    auto size() const { return data_.size(); }

   private:
    absl::Span<const unsigned char> data_;
  };

 public:
  /// \brief Initializes a new SHA256 hash.
  Sha256Hasher();
  explicit Sha256Hasher(ByteView initial) : Sha256Hasher() { Update(initial); }

  /// \brief Copyable/moveable.
  Sha256Hasher(const Sha256Hasher&);
  Sha256Hasher& operator=(const Sha256Hasher&);
  Sha256Hasher(Sha256Hasher&&);
  Sha256Hasher& operator=(Sha256Hasher&&);

  /// \brief Update the hash with the provided data.
  Sha256Hasher& Update(ByteView data) &;
  Sha256Hasher&& Update(ByteView data) &&;

  /// \brief Finalize the hash and return it as a binary array.
  std::array<std::byte, SHA256_DIGEST_LENGTH> Finish() &&;
  void Finish(std::byte* out) &&;

  /// \brief Convenience function which finalizes the hash and returns the
  /// result as a binary-containing string.
  std::string FinishBinString() &&;

  /// \brief Convenience function which finalizes the hash and returns the
  /// result as a lower-case hex-encoded string.
  std::string FinishHexString() &&;

 private:
  ::SHA256_CTX context_;
};

// inline copy/move to silence triviality warning on Finish.
inline Sha256Hasher::Sha256Hasher(const Sha256Hasher&) = default;
inline Sha256Hasher& Sha256Hasher::operator=(const Sha256Hasher&) = default;
inline Sha256Hasher::Sha256Hasher(Sha256Hasher&&) = default;
inline Sha256Hasher& Sha256Hasher::operator=(Sha256Hasher&&) = default;

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_SHA256_HASHER_H_
