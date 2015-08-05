/*
 * Copyright 2014 Google Inc. All rights reserved.
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

#ifndef KYTHE_CXX_INDEXER_CXX_KYTHE_OUTPUT_STREAM_H_
#define KYTHE_CXX_INDEXER_CXX_KYTHE_OUTPUT_STREAM_H_

#include <memory>
#include <vector>

#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"

#include "gflags/gflags.h"
#include "kythe/proto/storage.pb.h"
#include "llvm/ADT/StringRef.h"

DECLARE_bool(flush_after_each_entry);

namespace kythe {

/// A collection of references to the components of a VName.
struct VNameRef {
  llvm::StringRef signature;
  llvm::StringRef corpus;
  llvm::StringRef root;
  llvm::StringRef path;
  llvm::StringRef language;
  explicit VNameRef(const proto::VName &vname)
      : signature(vname.signature().data(), vname.signature().size()),
        corpus(vname.corpus().data(), vname.corpus().size()),
        root(vname.root().data(), vname.root().size()),
        path(vname.path().data(), vname.path().size()),
        language(vname.language().data(), vname.language().size()) {}
  VNameRef() {}
  void Expand(proto::VName *vname) const {
    vname->mutable_signature()->assign(signature.data(), signature.size());
    vname->mutable_corpus()->assign(corpus.data(), corpus.size());
    vname->mutable_root()->assign(root.data(), root.size());
    vname->mutable_path()->assign(path.data(), path.size());
    vname->mutable_language()->assign(language.data(), language.size());
  }
};
/// A collection of references to the components of a single Kythe fact.
struct FactRef {
  const VNameRef *source;
  llvm::StringRef fact_name;
  llvm::StringRef fact_value;
  /// Overwrites all of the fields in `entry` that can differ between single facts.
  void Expand(proto::Entry *entry) const {
    source->Expand(entry->mutable_source());
    entry->mutable_fact_name()->assign(fact_name.data(), fact_name.size());
    entry->mutable_fact_value()->assign(fact_value.data(), fact_value.size());
  }
};
/// A collection of references to the components of a single Kythe edge.
struct EdgeRef {
  const VNameRef *source;
  llvm::StringRef edge_kind;
  const VNameRef *target;
  /// Overwrites all of the fields in `entry` that can differ between edges without ordinals.
  void Expand(proto::Entry *entry) const {
    source->Expand(entry->mutable_source());
    target->Expand(entry->mutable_target());
    entry->mutable_edge_kind()->assign(edge_kind.data(), edge_kind.size());
  }
};
/// A collection of references to the components of a single Kythe edge with an ordinal.
struct OrdinalEdgeRef {
  const VNameRef *source;
  llvm::StringRef edge_kind;
  const VNameRef *target;
  uint32_t ordinal;
  /// Overwrites all of the fields in `entry` that can differ between edges with ordinals.
  void Expand(proto::Entry *entry) const {
    source->Expand(entry->mutable_source());
    target->Expand(entry->mutable_target());
    *(entry->mutable_fact_value()) = std::to_string(ordinal);
    entry->mutable_edge_kind()->assign(edge_kind.data(), edge_kind.size());
  }
};
// Interface for receiving Kythe data.
class KytheOutputStream {
 public:
  virtual void Emit(const FactRef &fact) = 0;
  virtual void Emit(const EdgeRef &edge) = 0;
  virtual void Emit(const OrdinalEdgeRef &edge) = 0;
  virtual ~KytheOutputStream() {}
};

// A `KytheOutputStream` that records `Entry` instances to a
// `FileOutputStream`.
class FileOutputStream : public KytheOutputStream {
 public:
  FileOutputStream(google::protobuf::io::FileOutputStream *stream)
      : stream_(stream) {
    edge_entry_.set_fact_name("/");
    ordinal_edge_entry_.set_fact_name("/kythe/ordinal");
  }

  void Emit(const FactRef &fact) override {
    fact.Expand(&fact_entry_);
    Emit(fact_entry_);
  }
  void Emit(const EdgeRef &edge) override {
    edge.Expand(&edge_entry_);
    Emit(edge_entry_);
  }
  void Emit(const OrdinalEdgeRef &edge) override {
    edge.Expand(&ordinal_edge_entry_);
    Emit(ordinal_edge_entry_);
  }

 private:
  /// The output stream to write on.
  google::protobuf::io::FileOutputStream *stream_;
  /// A prototypical Kythe fact, used only to build other Kythe facts.
  proto::Entry fact_entry_;
  /// A prototypical ordinalless Kythe edge, used only to build same.
  proto::Entry edge_entry_;
  /// A prototypical ordinalful Kythe edge, used only to build same.
  proto::Entry ordinal_edge_entry_;

  void Emit(const kythe::proto::Entry &entry) {
    {
      google::protobuf::io::CodedOutputStream coded_stream(stream_);
      coded_stream.WriteVarint32(entry.ByteSize());
      entry.SerializeToCodedStream(&coded_stream);
    }
    if (FLAGS_flush_after_each_entry) {
      stream_->Flush();
    }
  }
};

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_KYTHE_OUTPUT_STREAM_H_
