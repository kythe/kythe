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
#include <string>

#include "absl/container/flat_hash_map.h"
#include "absl/container/node_hash_map.h"
#include "absl/memory/memory.h"
#include "absl/strings/escaping.h"
#include "absl/strings/match.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "glog/logging.h"
#include "google/protobuf/compiler/code_generator.h"
#include "google/protobuf/compiler/cpp/cpp_generator.h"
#include "google/protobuf/compiler/plugin.h"
#include "google/protobuf/compiler/plugin.pb.h"
#include "google/protobuf/io/printer.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"

namespace kythe {
namespace {
using ::google::protobuf::FileDescriptor;
using ::google::protobuf::compiler::GeneratorContext;
using ::google::protobuf::compiler::cpp::CppGenerator;
using ::google::protobuf::io::Printer;
using ::google::protobuf::io::StringOutputStream;
using ::google::protobuf::io::ZeroCopyOutputStream;

constexpr absl::string_view kMetadataFileSuffix = ".meta";
constexpr absl::string_view kHeaderFileSuffix = ".h";

std::string WriteMetadata(absl::string_view metafile,
                          absl::string_view contents,
                          ZeroCopyOutputStream* stream) {
  if (!absl::EndsWith(metafile, kMetadataFileSuffix)) {
    return "Not a metadata file";
  }

  Printer printer(stream, '\n');
  printer.PrintRaw("/* ");
  printer.WriteRaw(metafile.data(), metafile.size());
  printer.PrintRaw("\n");
  std::string metadata;
  absl::Base64Escape(contents, &metadata);
  printer.WriteRaw(metadata.data(), metadata.size());
  printer.PrintRaw("\n*/\n");

  return "";
}

// Trivial ZeroCopyOutputStream wrapper which allows ownership to be retained
// by the context so that the metadata can be appended to the end.
class WrappedOutputStream : public ZeroCopyOutputStream {
 public:
  explicit WrappedOutputStream(ZeroCopyOutputStream* wrapped)
      : wrapped_(wrapped) {}
  bool Next(void** data, int* size) override {
    return wrapped_->Next(data, size);
  }
  void BackUp(int count) override { return wrapped_->BackUp(count); }
  google::protobuf::int64 ByteCount() const override {
    return wrapped_->ByteCount();
  }
  bool WriteAliasedRaw(const void* data, int size) override {
    return wrapped_->WriteAliasedRaw(data, size);
  }
  bool AllowsAliasing() const override { return wrapped_->AllowsAliasing(); }

 private:
  ZeroCopyOutputStream* wrapped_;
};

class WrappedContext : public GeneratorContext {
 public:
  explicit WrappedContext(GeneratorContext* wrapped) : wrapped_(wrapped) {}

  // Open the file for writing.
  ZeroCopyOutputStream* Open(const std::string& filename) override {
    // If it's a metadata file, preserve the contents in a string.
    if (absl::EndsWith(filename, kMetadataFileSuffix)) {
      return new StringOutputStream(&metadata_map_[filename]);
    }
    // If it's a header file, retain ownership of the stream so that we can
    // append to it later (OpenForAppend does not work be default).
    if (absl::EndsWith(filename, kHeaderFileSuffix)) {
      return new WrappedOutputStream(
          (stream_map_[filename] = absl::WrapUnique(wrapped_->Open(filename)))
              .get());
    }

    // Else, it's a source file.  Just pass it through.
    return wrapped_->Open(filename);
  }

  // Write the retained metadata in a comment at the end of the file.
  bool WriteEmbeddedMetadata(std::string* error) {
    for (const auto& entry : metadata_map_) {
      std::string header(absl::StripSuffix(entry.first, kMetadataFileSuffix));
      auto stream_iter = stream_map_.find(header);
      if (stream_iter != stream_map_.end()) {
        std::string err =
            WriteMetadata(entry.first, entry.second, stream_iter->second.get());
        if (!err.empty()) {
          *error = err;
          return false;
        }
      }
    }
    return true;
  }

  ZeroCopyOutputStream* OpenForAppend(const std::string&) override {
    return nullptr;
  }

  ZeroCopyOutputStream* OpenForInsert(const std::string&,
                                      const std::string&) override {
    return nullptr;
  }

  void ListParsedFiles(std::vector<const FileDescriptor*>* output) override {
    wrapped_->ListParsedFiles(output);
  }

  void GetCompilerVersion(
      google::protobuf::compiler::Version* version) const override {
    wrapped_->GetCompilerVersion(version);
  }

 private:
  GeneratorContext* wrapped_;
  // Map from header name -> metadata contents.
  absl::node_hash_map<std::string, std::string> metadata_map_;
  absl::flat_hash_map<std::string, std::unique_ptr<ZeroCopyOutputStream>>
      stream_map_;
};

class EmbeddedMetadataGenerator : public CppGenerator {
  bool Generate(const FileDescriptor* file, const std::string& parameter,
                GeneratorContext* context, std::string* error) const override {
    WrappedContext wrapper(context);
    if (CppGenerator::Generate(file, parameter, &wrapper, error)) {
      return wrapper.WriteEmbeddedMetadata(error);
    }
    return false;
  }
};

}  // namespace
}  // namespace kythe

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  kythe::EmbeddedMetadataGenerator generator;
  return google::protobuf::compiler::PluginMain(argc, argv, &generator);
}
