/*
 * Copyright 2020 The Kythe Authors. All rights reserved.
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

#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "kythe/proto/storage.pb.h"
// clang-format off
// IOSystem.h depends on ContainerUtil.h without transitively including it.
#include "souffle/utility/ContainerUtil.h"
#include "souffle/io/IOSystem.h"
// clang-format on

namespace kythe {
namespace {

/// Number of elements in the vname representation and their offsets.
constexpr int kVNameElements = 5;
constexpr int kSignatureEntry = 0;
constexpr int kCorpusEntry = 1;
constexpr int kPathEntry = 2;
constexpr int kRootEntry = 3;
constexpr int kLanguageEntry = 4;
/// Number of elements in the entry representation and their offsets.
constexpr int kEntryElements = 4;
constexpr int kSourceEntry = 0;
constexpr int kKindEntry = 1;
constexpr int kTargetEntry = 2;
constexpr int kValueEntry = 3;

class KytheEntryStreamReadStream : public souffle::ReadStream {
 public:
  explicit KytheEntryStreamReadStream(
      const std::map<std::string, std::string>& rw_operation,
      souffle::SymbolTable& symbol_table, souffle::RecordTable& record_table)
      : souffle::ReadStream(rw_operation, symbol_table, record_table),
        raw_input_(STDIN_FILENO) {
    // Check that the relation has the correct type.
    if (typeAttributes.size() == kEntryElements &&
        typeAttributes[0] == "r:vname" && typeAttributes[1] == "s:symbol" &&
        typeAttributes[2] == "r:vname" && typeAttributes[3] == "s:symbol") {
      const auto& record_info = types["records"]["r:vname"];
      size_t arity = record_info["arity"].long_value();
      const auto& types = record_info["types"];
      if (arity == kVNameElements) {
        relation_ok_ = true;
        for (size_t i = 0; i < arity; ++i) {
          if (types[i].string_value() != "s:symbol") {
            relation_ok_ = false;
            break;
          }
        }
      }
    }
    if (!relation_ok_) {
      fprintf(stderr, R"(error: bad relation for kythe_entrystream. Use:

.type vname = [
  signature:symbol,
  corpus:symbol,
  path:symbol,
  root:symbol,
  language:symbol
]

.decl entry(source:vname, kind:symbol, target:vname, value:symbol)

Note that this is an experimental feature and this declaration may change.
)");
    }
    empty_symbol_ = symbolTable.unsafeEncode("");
    std::array<souffle::RamDomain, 5> fields = {empty_symbol_, empty_symbol_,
                                                empty_symbol_, empty_symbol_,
                                                empty_symbol_};
    empty_vname_ = record_table.pack(fields.data(), fields.size());
  }

 protected:
  souffle::Own<souffle::RamDomain[]> readNextTuple() override {
    if (!relation_ok_) {
      return nullptr;
    }
    google::protobuf::uint32 byte_size;
    google::protobuf::io::CodedInputStream coded_input(&raw_input_);
    coded_input.SetTotalBytesLimit(INT_MAX);
    if (!coded_input.ReadVarint32(&byte_size)) {
      return nullptr;
    }
    coded_input.PushLimit(byte_size);
    kythe::proto::Entry entry;
    if (!entry.ParseFromCodedStream(&coded_input)) {
      fprintf(stderr, "error reading input stream\n");
      return nullptr;
    }
    auto tuple = std::make_unique<souffle::RamDomain[]>(kEntryElements);
    souffle::RamDomain vname[kVNameElements];
    CopyVName(entry.source(), vname);
    tuple[kSourceEntry] = recordTable.pack(vname, kVNameElements);
    if (entry.has_target()) {
      tuple[kKindEntry] = symbolTable.unsafeEncode(entry.edge_kind());
      CopyVName(entry.target(), vname);
      tuple[kTargetEntry] = recordTable.pack(vname, kVNameElements);
      tuple[kValueEntry] = empty_symbol_;
    } else {
      tuple[kKindEntry] = symbolTable.unsafeEncode(entry.fact_name());
      tuple[kTargetEntry] = empty_vname_;
      tuple[kValueEntry] = symbolTable.unsafeEncode(entry.fact_value());
    }

    return tuple;
  }

 private:
  /// Copies the fields from `vname` to `target`.
  void CopyVName(const kythe::proto::VName& vname, souffle::RamDomain* target) {
    target[kSignatureEntry] = symbolTable.unsafeEncode(vname.signature());
    target[kCorpusEntry] = symbolTable.unsafeEncode(vname.corpus());
    target[kPathEntry] = symbolTable.unsafeEncode(vname.path());
    target[kRootEntry] = symbolTable.unsafeEncode(vname.root());
    target[kLanguageEntry] = symbolTable.unsafeEncode(vname.language());
  }
  /// This `FileInputStream` reads from stdin.
  google::protobuf::io::FileInputStream raw_input_;
  /// True when the relation we're reading has the expected type.
  bool relation_ok_ = false;
  /// The empty symbol.
  souffle::RamDomain empty_symbol_;
  /// An empty VName.
  souffle::RamDomain empty_vname_;
};

class KytheEntryStreamReadFactory : public souffle::ReadStreamFactory {
 public:
  souffle::Own<souffle::ReadStream> getReader(
      const std::map<std::string, std::string>& rw_operation,
      souffle::SymbolTable& symbol_table,
      souffle::RecordTable& record_table) override {
    return souffle::mk<KytheEntryStreamReadStream>(rw_operation, symbol_table,
                                                   record_table);
  }

  const std::string& getName() const override {
    static const std::string name = "kythe_entrystream";
    return name;
  }
};

struct AutoRegisterReader {
  AutoRegisterReader() {
    souffle::IOSystem::getInstance().registerReadStreamFactory(
        std::make_shared<KytheEntryStreamReadFactory>());
  }
} register_reader;

}  // anonymous namespace
}  // namespace kythe
