/*
 * Copyright 2022 The Kythe Authors. All rights reserved.
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

#ifndef KYTHE_CXX_INDEXER_CXX_STREAM_ADAPTER_H_
#define KYTHE_CXX_INDEXER_CXX_STREAM_ADAPTER_H_

#include "absl/base/attributes.h"
#include "absl/functional/any_invocable.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/Support/raw_ostream.h"

namespace kythe {

/// \brief StreamAdapter adapts llvm::raw_ostream-compatible types so that they
/// can be streamed to standard streams.
class StreamAdapter {
 public:
  /// \brief Adapts an object with a dump(llvm::raw_ostream&, ...) member
  /// function to stream the output on an OS stream, e.g.
  ///
  /// std::cerr << StreamAdapter::Dump(*Decl) << std::endl;
  ///
  /// Would call the equivalent of Decl->dump(std::cerr). Additional arguments
  /// are forwarded after the output stream.
  template <typename T, typename... Tail>
  static StreamAdapter Dump(T&& value ABSL_ATTRIBUTE_LIFETIME_BOUND,
                            Tail&&... tail ABSL_ATTRIBUTE_LIFETIME_BOUND) {
    return StreamAdapter([&](llvm::raw_ostream& OS) {
      std::forward<T>(value).dump(OS, std::forward<Tail>(tail)...);
    });
  }

  /// \brief Adapts an object with a print(llvm::raw_ostream&, ...) member
  /// function to stream the output on an OS stream, e.g.
  ///
  /// std::cerr << StreamAdapter::Print(*Decl) << std::endl;
  ///
  /// Would call the equivalent of Decl->print(std::cerr). Additional arguments
  /// are forwarded after the output stream.
  template <typename T, typename... Tail>
  static StreamAdapter Print(T&& value ABSL_ATTRIBUTE_LIFETIME_BOUND,
                             Tail&&... tail ABSL_ATTRIBUTE_LIFETIME_BOUND) {
    return StreamAdapter([&](llvm::raw_ostream& OS) {
      std::forward<T>(value).print(OS, std::forward<Tail>(tail)...);
    });
  }

  /// \brief Adapts an object via an underlying operator<<, e.g.
  ///
  /// std::cerr << StreamAdapter::Stream(*Decl) << std::endl;
  template <typename T>
  static StreamAdapter Stream(T&& value ABSL_ATTRIBUTE_LIFETIME_BOUND) {
    return StreamAdapter(
        [&](llvm::raw_ostream& OS) { OS << std::forward<T>(value); });
  }

  StreamAdapter(const StreamAdapter&) = delete;
  StreamAdapter& operator=(const StreamAdapter&) = delete;

 private:
  explicit StreamAdapter(
      absl::AnyInvocable<void(llvm::raw_ostream&) const> stream)
      : stream_(std::move(stream)) {}

  friend std::ostream& operator<<(std::ostream& out,
                                  const StreamAdapter& dumper) {
    llvm::raw_os_ostream OS(out);
    dumper.stream_(OS);
    return out;
  }

  absl::AnyInvocable<void(llvm::raw_ostream&) const> stream_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_STREAM_ADAPTER_H_
