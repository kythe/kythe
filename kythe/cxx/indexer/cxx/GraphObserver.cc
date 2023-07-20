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

#include "absl/log/check.h"
#include "absl/strings/escaping.h"
#include "absl/strings/str_format.h"
#include "kythe/cxx/common/sha256_hasher.h"

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

FileHashRecorder::FileHashRecorder(absl::string_view path)
    : out_file_(::fopen(std::string(path).c_str(), "w")) {
  if (out_file_ == nullptr) ::perror("Error in fopen");
  CHECK(out_file_ != nullptr)
      << "Couldn't open file " << path << " for hash recording.";
}

void FileHashRecorder::RecordHash(absl::string_view hash,
                                  absl::string_view web64hash,
                                  absl::string_view original) {
  auto result = hashes_.insert(std::string(hash));
  if (!result.second) return;
  absl::FPrintF(out_file_, "%s\t%s\n", web64hash, original);
}
FileHashRecorder::~FileHashRecorder() {
  CHECK(::fclose(out_file_) == 0) << "Couldn't close file for hash recording.";
}

std::string GraphObserver::ForceEncodeString(absl::string_view InString) const {
  auto hash = Sha256Hasher(InString).FinishBinString();
  std::string result;
  // Use web-safe escaping because vnames are frequently URI-encoded. This
  // doesn't include padding ('=') or the characters + or /, all of which will
  // expand to three-byte sequences in such an encoding.
  absl::WebSafeBase64Escape(hash, &result);
  if (hash_recorder_ != nullptr) {
    hash_recorder_->RecordHash(hash, result, InString);
  }
  return result;
}

std::string GraphObserver::CompressAnchorSignature(
    absl::string_view InSignature) const {
  if (InSignature.size() <= kSha256DigestBase64MaxEncodingLength) {
    return std::string(InSignature);
  }
  return absl::WebSafeBase64Escape(Sha256Hasher(InSignature).FinishBinString());
}

std::string GraphObserver::CompressString(absl::string_view InString) const {
  if (InString.size() <= kSha256DigestBase64MaxEncodingLength) {
    return std::string(InString);
  }
  return ForceEncodeString(InString);
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
