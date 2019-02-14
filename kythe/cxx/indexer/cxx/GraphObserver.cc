#include "kythe/cxx/indexer/cxx/GraphObserver.h"

#include <openssl/sha.h>  // for SHA256

#include "absl/strings/escaping.h"

namespace kythe {

// base64 has a 4:3 overhead and SHA256_DIGEST_LENGTH is 32. 32*4/3 = 42.
constexpr size_t kSha256DigestBase64MaxEncodingLength = 42;

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
  absl::Base64Escape(Hash, &Result);
  return Result;
}

}  // namespace kythe
