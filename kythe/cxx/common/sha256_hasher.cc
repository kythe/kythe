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

#include "kythe/cxx/common/sha256_hasher.h"

#include <openssl/sha.h>

#include <array>
#include <cstddef>
#include <string>
#include <utility>

#include "absl/strings/escaping.h"

namespace kythe {
Sha256Hasher::Sha256Hasher() { ::SHA256_Init(&context_); }

Sha256Hasher& Sha256Hasher::Update(Sha256Hasher::ByteView data) & {
  ::SHA256_Update(&context_, data.data(), data.size());
  return *this;
}

Sha256Hasher&& Sha256Hasher::Update(Sha256Hasher::ByteView data) && {
  return std::move(Update(data));
}

std::array<std::byte, SHA256_DIGEST_LENGTH> Sha256Hasher::Finish() && {
  std::array<std::byte, SHA256_DIGEST_LENGTH> result;
  std::move(*this).Finish(result.data());
  return result;
}

void Sha256Hasher::Finish(std::byte* out) && {
  ::SHA256_Final(reinterpret_cast<unsigned char*>(out), &context_);
}

std::string Sha256Hasher::FinishBinString() && {
  std::string result(SHA256_DIGEST_LENGTH, '\0');
  ::SHA256_Final(reinterpret_cast<unsigned char*>(result.data()), &context_);
  return result;
}

std::string Sha256Hasher::FinishHexString() && {
  return absl::BytesToHexString(std::move(*this).FinishBinString());
}

}  // namespace kythe
