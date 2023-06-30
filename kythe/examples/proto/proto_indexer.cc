/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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

// proto_indexer is a simple example indexer for protobuf.
// usage: proto_indexer proto.descriptor
// proto.descriptor is expected to have been built using
//   --include_imports --include_source_info

#include <fcntl.h>

#include <map>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/initialize.h"
#include "absl/log/log.h"
#include "google/protobuf/descriptor.pb.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "kythe/cxx/common/indexing/KytheCachingOutput.h"
#include "kythe/cxx/common/indexing/KytheGraphRecorder.h"
#include "kythe/cxx/common/protobuf_metadata_file.h"

ABSL_FLAG(std::string, corpus_name, "kythe", "Use this corpus in VNames.");

namespace kythe {
namespace {
namespace gpb = google::protobuf;
/// ProtoFiles creates `file` nodes for proto source files. It also maps back
/// from proto line, col locations to file offsets. It assumes input files are
/// ASCII and do not use tabs.
class ProtoFiles {
 public:
  /// \brief Prepare the file at `path` for indexing and emit its text to
  /// `recorder`.
  ///
  /// File data will only be emitted once. This class looks up `path` on the
  /// local file system.
  bool IndexFile(const std::string& path, KytheGraphRecorder* recorder);

  /// \brief Looks up the byte offset for a `line` and `col` in `file`.
  /// \return -1 if the file can't be found or `line` is out of range.
  int64_t anchor_offset(const std::string& file, int line, int col) const;

 private:
  /// \brief Store the file at `path` with text `buffer`. Identifies the
  /// starting byte for each of its lines.
  void InsertFile(const std::string& path, std::string&& buffer);

  struct FileRecord {
    /// The content of the file.
    std::string content;
    /// A vector of byte offsets where line_starts[i] is the byte starting
    /// the ith line.
    std::vector<size_t> line_starts;
  };
  /// Maps from filenames to `FileRecord`s.
  std::map<std::string, FileRecord> files_;
};

/// \brief Reads the contents of the file at `path` into `buffer`.
/// \return true on success; false on failure.
bool ReadFile(const std::string& path, std::string* buffer) {
  int in_fd = ::open(path.c_str(), O_RDONLY);
  if (in_fd < 0) {
    LOG(ERROR) << "Couldn't open " << path;
    return false;
  }
  google::protobuf::io::FileInputStream file_stream(in_fd);
  const void* data;
  int size;
  while (file_stream.Next(&data, &size)) {
    buffer->append(static_cast<const char*>(data), size);
  }
  if (file_stream.GetErrno() != 0) {
    return false;
  }
  if (!file_stream.Close()) {
    LOG(ERROR) << "Couldn't close " << path;
    return false;
  }
  return true;
}

bool ProtoFiles::IndexFile(const std::string& path,
                           KytheGraphRecorder* recorder) {
  auto file = files_.find(path);
  if (file != files_.end()) {
    return true;
  }
  std::string buffer;
  if (!ReadFile(path, &buffer)) {
    return false;
  }
  proto::VName file_vname;
  file_vname.set_path(path);
  file_vname.set_corpus(absl::GetFlag(FLAGS_corpus_name));
  recorder->AddFileContent(kythe::VNameRef(file_vname), buffer);
  InsertFile(path, std::move(buffer));
  return true;
}

int64_t ProtoFiles::anchor_offset(const std::string& file, int line,
                                  int col) const {
  const auto& file_pair = files_.find(file);
  if (file_pair == files_.end()) {
    return -1;
  }
  if (line >= file_pair->second.line_starts.size()) {
    return -1;
  }
  return file_pair->second.line_starts[line] + col;
}

void ProtoFiles::InsertFile(const std::string& path, std::string&& buffer) {
  std::vector<size_t> lookup;
  lookup.push_back(0);
  for (int i = 0; i < buffer.size(); ++i) {
    // This assumes input files are in ASCII (and use no tabs or alternate
    // line endings).
    if (buffer[i] == '\n') {
      lookup.push_back(i + 1);
    }
  }
  files_.emplace(path, FileRecord{std::move(buffer), std::move(lookup)});
}

/// ProtoTreeCursor walks around inside proto descriptors and emits Kythe facts.
class ProtoTreeCursor {
 public:
  ProtoTreeCursor(ProtoFiles* proto_files, KytheGraphRecorder* recorder)
      : proto_files_(proto_files), recorder_(recorder) {}

  /// \brief Emits information about the proto objects in `fd`.
  /// \return false on failure.
  bool IndexDescriptor(const gpb::FileDescriptorProto& fd);

 private:
  /// \brief Emits information about the message descriptor `d`.
  void IndexDescriptor(const gpb::DescriptorProto& d);

  /// \brief Emits the anchor pointed to by `path_` and returns its VName.
  /// If no such anchor exists, emits nothing and returns null.
  ///
  /// The return value is valid only until the next call to `anchor_vname` or
  /// `EmitAnchor`.
  kythe::VNameRef* EmitAnchor();

  /// \brief Returns the VName for the anchor pointed to by `path_`, or
  /// null if there is no such anchor.
  ///
  /// The return value is valid only until the next call to `anchor_vname` or
  /// `EmitAnchor`.
  kythe::VNameRef* anchor_vname();

  /// \brief Returns the Kythe signature corresponding to the current path.
  std::string PathToSignature() const;

  /// \brief Breadcrumbs maintain the path that `ProtoTreeCursor` is currently
  /// at. They can emit anchors for the syntactic locations associated with the
  /// current path.
  class Breadcrumb {
   public:
    Breadcrumb(ProtoTreeCursor* thiz) : thiz_(thiz) {}
    Breadcrumb(Breadcrumb&& o) : thiz_(o.thiz_) { o.thiz_ = nullptr; }
    ~Breadcrumb() {
      if (thiz_) {
        thiz_->path_.pop_back();
      }
    }
    VNameRef* EmitAnchor() { return thiz_->EmitAnchor(); }

   private:
    friend class ProtoTreeCursor;
    ProtoTreeCursor* thiz_;
  };

  Breadcrumb EnterField(int field_id) {
    path_.push_back(field_id);
    return Breadcrumb(this);
  }

  /// The path we've reached in the proto AST.
  std::vector<int> path_;
  /// A map from paths to proto source locations.
  std::map<std::vector<int>, const gpb::SourceCodeInfo::Location*> paths_;
  /// All proto files seen by the indexer.
  ProtoFiles* proto_files_;
  /// The destination for recorded artifacts.
  KytheGraphRecorder* recorder_;
  /// The filename of the source .proto file.
  std::string filename_;
  /// The VName for the source .proto file.
  proto::VName file_vname_;
  /// The corpus for emitted Kythe artifacts.
  const std::string corpus_ = "kythe";
  /// The language for emitted Kythe artifacts.
  const std::string language_ = "protobuf";
  /// anchor_vname_ref_'s signature.
  std::string anchor_vname_signature_;
  /// A reference to the current path's anchor's VName. Valid only after a call
  /// to anchor_vname() (that does not return nullptr).
  VNameRef anchor_vname_ref_;
  /// The current path's anchor's start position.
  int64_t anchor_start_;
  /// The current path's anchor's end position.
  int64_t anchor_end_;
};

bool ProtoTreeCursor::IndexDescriptor(const gpb::FileDescriptorProto& fd) {
  if (!fd.has_source_code_info()) {
    LOG(ERROR) << fd.name() << " (package " << fd.package()
               << ") has no SourceCodeInfo";
    return false;
  }
  if (!proto_files_->IndexFile(fd.name(), recorder_)) {
    LOG(ERROR) << fd.name() << " couldn't be found";
  }
  for (const auto& loc : fd.source_code_info().location()) {
    paths_.emplace(std::vector<int>(loc.path().begin(), loc.path().end()),
                   &loc);
  }
  filename_ = fd.name();
  file_vname_.set_corpus(corpus_);
  file_vname_.set_path(filename_);
  {
    auto ms = EnterField(gpb::FileDescriptorProto::kMessageTypeFieldNumber);
    for (int i = 0; i < fd.message_type_size(); ++i) {
      auto m = EnterField(i);
      IndexDescriptor(fd.message_type(i));
    }
  }
  return true;
}

void ProtoTreeCursor::IndexDescriptor(const gpb::DescriptorProto& d) {
  if (auto name =
          EnterField(gpb::DescriptorProto::kNameFieldNumber).EmitAnchor()) {
    proto::VName message_vname = VNameForProtoPath(file_vname_, path_);
    recorder_->AddEdge(*name, EdgeKindID::kDefinesBinding,
                       VNameRef(message_vname));
    recorder_->AddProperty(VNameRef(message_vname), NodeKindID::kRecord);
  }
}

kythe::VNameRef* ProtoTreeCursor::EmitAnchor() {
  if (auto vname = anchor_vname()) {
    recorder_->AddProperty(*vname, NodeKindID::kAnchor);
    recorder_->AddProperty(*vname, PropertyID::kLocationStartOffset,
                           anchor_start_);
    recorder_->AddProperty(*vname, PropertyID::kLocationEndOffset, anchor_end_);
    return vname;
  }
  return nullptr;
}

kythe::VNameRef* ProtoTreeCursor::anchor_vname() {
  const auto& location = paths_.find(path_);
  if (location == paths_.end()) {
    LOG(WARNING) << "path failed (" << PathToSignature() << ")";
    return nullptr;
  }
  auto spans = location->second->span_size();
  if (spans < 3) {
    LOG(WARNING) << "span failed";
    return nullptr;
  }
  if ((anchor_start_ = proto_files_->anchor_offset(
           filename_, location->second->span(0), location->second->span(1))) <
      0) {
    LOG(WARNING) << "start lookup failed";
    return nullptr;
  }
  // Spans in SourceCodeInfo.Location are stored in tuples of length 3 or 4:
  //   4: (start line, start column, end line, end column) or
  //   3: (line, start column, end column).
  if ((anchor_end_ = proto_files_->anchor_offset(
           filename_, location->second->span(spans == 3 ? 0 : 2),
           location->second->span(spans == 3 ? 2 : 3))) < 0) {
    LOG(WARNING) << "end lookup failed";
    return nullptr;
  }
  anchor_vname_signature_ =
      "@" + std::to_string(anchor_start_) + ":" + std::to_string(anchor_end_);
  anchor_vname_ref_.set_signature(anchor_vname_signature_);
  anchor_vname_ref_.set_path(filename_);
  anchor_vname_ref_.set_corpus(corpus_);
  anchor_vname_ref_.set_language(language_);
  return &anchor_vname_ref_;
}

std::string ProtoTreeCursor::PathToSignature() const {
  std::string path_sig;
  for (const auto& node : path_) {
    if (!path_sig.empty()) {
      path_sig += ":";
    }
    path_sig += std::to_string(node);
  }
  return path_sig;
}

}  // anonymous namespace

bool IndexDescriptorSet(const google::protobuf::FileDescriptorSet& fds,
                        KytheGraphRecorder* recorder) {
  ProtoFiles proto_files;
  for (const auto& descriptor : fds.file()) {
    ProtoTreeCursor cursor(&proto_files, recorder);
    if (!cursor.IndexDescriptor(descriptor)) {
      return false;
    }
  }
  return true;
}

int main(int argc, char* argv[]) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  absl::InitializeLog();
  std::vector<char*> remain = absl::ParseCommandLine(argc, argv);
  std::vector<std::string> final_args(remain.begin() + 1, remain.end());
  google::protobuf::io::FileOutputStream out_stream(STDOUT_FILENO);
  FileOutputStream stream(&out_stream);
  KytheGraphRecorder recorder(&stream);
  for (const auto& input : final_args) {
    int in_fd = ::open(input.c_str(), O_RDONLY);
    if (in_fd < 0) {
      LOG(ERROR) << "Couldn't open " << input;
      return 1;
    }
    google::protobuf::io::FileInputStream file_stream(in_fd);
    google::protobuf::io::CodedInputStream coded_input(&file_stream);
    google::protobuf::FileDescriptorSet file_descriptor_set;
    if (!file_descriptor_set.ParseFromCodedStream(&coded_input)) {
      LOG(ERROR) << "Couldn't parse " << input;
      return 1;
    }
    if (!IndexDescriptorSet(file_descriptor_set, &recorder)) {
      LOG(ERROR) << "Couldn't index " << input;
      return 1;
    }
    if (!file_stream.Close()) {
      LOG(ERROR) << "Couldn't close " << input;
      return 1;
    }
  }
  return 0;
}

}  // namespace kythe

int main(int argc, char* argv[]) { return kythe::main(argc, argv); }
