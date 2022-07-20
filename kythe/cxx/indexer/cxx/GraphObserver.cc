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
#include "absl/strings/str_format.h"

namespace kythe {

// base64 has a 4:3 overhead and SHA256_DIGEST_LENGTH is 32. 32*4/3 = 42.66666
constexpr size_t kSha256DigestBase64MaxEncodingLength = 43;

namespace {
struct MintedVNameHeader {
  size_t path_offset;
  size_t root_offset;
  size_t corpus_offset;
  size_t language_offset;
};
}  // anonymous namespace

FileHashRecorder::FileHashRecorder(const std::string& path)
    : out_file_(::fopen(path.c_str(), "w")) {
  if (out_file_ == nullptr) ::perror("Error in fopen");
  CHECK(out_file_ != nullptr)
      << "Couldn't open file " << path << " for hash recording.";
}

void FileHashRecorder::RecordHash(const std::string& hash,
                                  absl::string_view web64hash,
                                  absl::string_view original) {
  auto result = hashes_.insert(hash);
  if (!result.second) return;
  absl::FPrintF(out_file_, "%s\t%s\n", web64hash, original);
}
FileHashRecorder::~FileHashRecorder() {
  CHECK(::fclose(out_file_) == 0) << "Couldn't close file for hash recording.";
}

std::string GraphObserver::CompressString(absl::string_view InString,
                                          bool Force, bool DontRecord) const {
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
  if (!DontRecord && hash_recorder_ != nullptr) {
    hash_recorder_->RecordHash(Hash, Result, InString);
  }
  return Result;
}

GraphObserver::NodeId GraphObserver::MintNodeIdForVName(
    const proto::VName& vname) {
  std::string id;
  MintedVNameHeader header;
  id.resize(sizeof(MintedVNameHeader));
  absl::StrAppend(&id, vname.signature());
  header.path_offset = id.size();
  absl::StrAppend(&id, vname.path());
  header.root_offset = id.size();
  absl::StrAppend(&id, vname.root());
  header.corpus_offset = id.size();
  absl::StrAppend(&id, vname.corpus());
  header.language_offset = id.size();
  absl::StrAppend(&id, vname.language());
  std::memcpy(id.data(), &header, sizeof(MintedVNameHeader));
  return NodeId::CreateUncompressed(getVNameClaimToken(), id);
}

VNameRef GraphObserver::DecodeMintedVName(const NodeId& id) const {
  const auto& bytes = id.getRawIdentity();
  VNameRef ref;
  CHECK(sizeof(MintedVNameHeader) <= bytes.size());
  MintedVNameHeader header;
  std::memcpy(&header, bytes.data(), sizeof(MintedVNameHeader));
  CHECK(sizeof(MintedVNameHeader) <= header.path_offset &&
        header.path_offset <= header.root_offset &&
        header.root_offset <= header.corpus_offset &&
        header.corpus_offset <= header.language_offset &&
        header.language_offset <= bytes.size());
  ref.set_signature({bytes.data() + sizeof(MintedVNameHeader),
                     header.path_offset - sizeof(MintedVNameHeader)});
  ref.set_path({bytes.data() + header.path_offset,
                header.root_offset - header.path_offset});
  ref.set_root({bytes.data() + header.root_offset,
                header.corpus_offset - header.root_offset});
  ref.set_corpus({bytes.data() + header.corpus_offset,
                  header.language_offset - header.corpus_offset});
  ref.set_language({bytes.data() + header.language_offset,
                    bytes.size() - header.language_offset});
  return ref;
}

}  // namespace kythe
