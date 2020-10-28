/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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

#ifndef KYTHE_CXX_COMMON_INDEXING_KYTHE_OUTPUT_STREAM_H_
#define KYTHE_CXX_COMMON_INDEXING_KYTHE_OUTPUT_STREAM_H_

#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "kythe/proto/common.pb.h"
#include "kythe/proto/storage.pb.h"

namespace kythe {
/// \brief Code marked with semantic spans.
using MarkedSource = kythe::proto::common::MarkedSource;

/// A collection of references to the components of a VName.
class VNameRef {
 public:
  absl::string_view signature() const { return signature_; }
  absl::string_view corpus() const { return corpus_; }
  absl::string_view root() const { return root_; }
  absl::string_view path() const { return path_; }
  absl::string_view language() const { return language_; }
  void set_signature(absl::string_view s) { signature_ = s; }
  void set_corpus(absl::string_view s) { corpus_ = s; }
  void set_root(absl::string_view s) { root_ = s; }
  void set_path(absl::string_view s) { path_ = s; }
  void set_language(absl::string_view s) { language_ = s; }

  explicit VNameRef(const proto::VName& vname)
      : signature_(vname.signature().data(), vname.signature().size()),
        corpus_(vname.corpus().data(), vname.corpus().size()),
        root_(vname.root().data(), vname.root().size()),
        path_(vname.path().data(), vname.path().size()),
        language_(vname.language().data(), vname.language().size()) {}
  VNameRef() {}
  void Expand(proto::VName* vname) const {
    vname->mutable_signature()->assign(signature_.data(), signature_.size());
    vname->mutable_corpus()->assign(corpus_.data(), corpus_.size());
    vname->mutable_root()->assign(root_.data(), root_.size());
    vname->mutable_path()->assign(path_.data(), path_.size());
    vname->mutable_language()->assign(language_.data(), language_.size());
  }
  std::string DebugString() const {
    return absl::StrCat("{", corpus_, ",", root_, ",", path_, ",", signature_,
                        ",", language_, "}");
  }

 private:
  absl::string_view signature_;
  absl::string_view corpus_;
  absl::string_view root_;
  absl::string_view path_;
  absl::string_view language_;
};
/// A collection of references to the components of a single Kythe fact.
struct FactRef {
  const VNameRef* source;
  absl::string_view fact_name;
  absl::string_view fact_value;
  /// Overwrites all of the fields in `entry` that can differ between single
  /// facts.
  void Expand(proto::Entry* entry) const {
    source->Expand(entry->mutable_source());
    entry->mutable_fact_name()->assign(fact_name.data(), fact_name.size());
    entry->mutable_fact_value()->assign(fact_value.data(), fact_value.size());
  }
};
/// A collection of references to the components of a single Kythe edge.
struct EdgeRef {
  const VNameRef* source;
  absl::string_view edge_kind;
  const VNameRef* target;
  /// Overwrites all of the fields in `entry` that can differ between edges
  /// without ordinals.
  void Expand(proto::Entry* entry) const {
    source->Expand(entry->mutable_source());
    target->Expand(entry->mutable_target());
    entry->mutable_edge_kind()->assign(edge_kind.data(), edge_kind.size());
  }
};
/// A collection of references to the components of a single Kythe edge with an
/// ordinal.
struct OrdinalEdgeRef {
  const VNameRef* source;
  absl::string_view edge_kind;
  const VNameRef* target;
  uint32_t ordinal;
  /// Overwrites all of the fields in `entry` that can differ between edges with
  /// ordinals.
  void Expand(proto::Entry* entry) const {
    entry->set_edge_kind(absl::StrCat(edge_kind, ".", ordinal));
    source->Expand(entry->mutable_source());
    target->Expand(entry->mutable_target());
  }
};

// Interface for receiving Kythe data.
class KytheOutputStream {
 public:
  virtual void Emit(const FactRef& fact) = 0;
  virtual void Emit(const EdgeRef& edge) = 0;
  virtual void Emit(const OrdinalEdgeRef& edge) = 0;
  /// Add a buffer to the buffer stack to group facts, edges, and buffers
  /// together.
  virtual void PushBuffer() {}
  /// Pop the last buffer from the buffer stack.
  virtual void PopBuffer() {}
  virtual ~KytheOutputStream() {}
};

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_INDEXING_KYTHE_OUTPUT_STREAM_H_
