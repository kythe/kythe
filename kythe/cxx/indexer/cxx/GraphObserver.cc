/*
 * Copyright 2019 The Kythe Authors. All rights reserved.
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

#include "kythe/cxx/indexer/cxx/GraphObserver.h"

#include <openssl/sha.h>  // for SHA256

#include "absl/strings/escaping.h"

namespace kythe {

// base64 has a 4:3 overhead and SHA256_DIGEST_LENGTH is 32. 32*4/3 = 42.66666
constexpr size_t kSha256DigestBase64MaxEncodingLength = 43;

std::string CompressString(absl::string_view InString, bool Force) {
  if (InString.size() <= kSha256DigestBase64MaxEncodingLength && !Force) {
    return std::string(InString);
  }
  ::SHA256_CTX Sha;
  ::SHA256_Init(&Sha);
  ::SHA256_Update(&Sha, reinterpret_cast<const unsigned char*>(InString.data()),
                  InString.size());
  std::string Hash(SHA256_DIGEST_LENGTH, '\0');
  ::SHA256_Final(reinterpret_cast<unsigned char*>(&Hash[0]), &Sha);
  std::string Result;
  // Use web-safe escaping because vnames are frequently URI-encoded. This
  // doesn't include padding ('=') or the characters + or /, all of which will
  // expand to three-byte sequences in such an encoding.
  absl::WebSafeBase64Escape(Hash, &Result);
  return Result;
}

}  // namespace kythe
