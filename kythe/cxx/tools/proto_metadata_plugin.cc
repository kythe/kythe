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
#include <cstdlib>
#include <string>

#include "absl/container/flat_hash_map.h"
#include "absl/container/node_hash_map.h"
#include "absl/log/log.h"
#include "absl/memory/memory.h"
#include "absl/strings/escaping.h"
#include "absl/strings/match.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "google/protobuf/compiler/code_generator.h"
#include "google/protobuf/compiler/cpp/generator.h"
#include "google/protobuf/compiler/plugin.h"
#include "google/protobuf/compiler/plugin.pb.h"
#include "google/protobuf/io/gzip_stream.h"
#include "google/protobuf/io/printer.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "kythe/cxx/common/init.h"

namespace kythe {
namespace {
using ::google::protobuf::FileDescriptor;
using ::google::protobuf::compiler::GeneratorContext;
using ::google::protobuf::compiler::cpp::CppGenerator;
using ::google::protobuf::io::GzipOutputStream;
using ::google::protobuf::io::Printer;
using ::google::protobuf::io::StringOutputStream;
using ::google::protobuf::io::ZeroCopyOutputStream;

constexpr absl::string_view kMetadataFileSuffix = ".meta";
constexpr absl::string_view kHeaderFileSuffix = ".h";
constexpr absl::string_view kCompressMetadataParam = "compress_metadata";
constexpr absl::string_view kAnnotateHeaderParam = "annotate_headers";
constexpr absl::string_view kAnnotationGuardParam = "annotation_guard_name";
constexpr absl::string_view kAnnotationGuardDefault = "KYTHE_IS_RUNNING";
constexpr absl::string_view kAnnotationPragmaParam = "annotation_pragma_name";
constexpr absl::string_view kAnnotationPragmaInline = "kythe_inline_metadata";
constexpr absl::string_view kAnnotationPragmaCompress =
    "kythe_inline_gzip_metadata";
constexpr int kMetadataLineLength = 76;

absl::string_view NextChunk(absl::string_view* data, int size) {
  if (data->empty()) {
    return {};
  }
  absl::string_view result = data->substr(0, size);
  data->remove_prefix(result.size());
  return result;
}

void WriteLines(absl::string_view metadata, Printer* printer) {
  while (!metadata.empty()) {
    absl::string_view chunk = NextChunk(&metadata, kMetadataLineLength);
    printer->WriteRaw(chunk.data(), chunk.size());
    printer->PrintRaw("\n");
  }
}

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
  WriteLines(metadata, &printer);
  printer.PrintRaw("*/\n");

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
  int64_t ByteCount() const override { return wrapped_->ByteCount(); }
  bool WriteAliasedRaw(const void* data, int size) override {
    return wrapped_->WriteAliasedRaw(data, size);
  }
  bool AllowsAliasing() const override { return wrapped_->AllowsAliasing(); }

 private:
  ZeroCopyOutputStream* wrapped_;
};

class GzipStringOutputStream : public ZeroCopyOutputStream {
 public:
  explicit GzipStringOutputStream(std::string* output)
      : string_stream_(output) {}

  bool Next(void** data, int* size) override {
    return gzip_stream_.Next(data, size);
  }
  void BackUp(int count) override { return gzip_stream_.BackUp(count); }
  int64_t ByteCount() const override { return gzip_stream_.ByteCount(); }
  bool WriteAliasedRaw(const void* data, int size) override {
    return gzip_stream_.WriteAliasedRaw(data, size);
  }
  bool AllowsAliasing() const override { return gzip_stream_.AllowsAliasing(); }

 private:
  StringOutputStream string_stream_;
  GzipOutputStream gzip_stream_{&string_stream_};
};

class WrappedContext : public GeneratorContext {
 public:
  explicit WrappedContext(GeneratorContext* wrapped, bool compress_metadata)
      : compress_metadata_(compress_metadata), wrapped_(wrapped) {}

  // Open the file for writing.
  ZeroCopyOutputStream* Open(const std::string& filename) override {
    // If it's a metadata file, preserve the contents in a string.
    if (absl::EndsWith(filename, kMetadataFileSuffix)) {
      if (compress_metadata_) {
        return new GzipStringOutputStream(&metadata_map_[filename]);
      } else {
        return new StringOutputStream(&metadata_map_[filename]);
      }
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
  bool compress_metadata_;
  GeneratorContext* wrapped_;
  // Map from header name -> metadata contents.
  absl::node_hash_map<std::string, std::string> metadata_map_;
  absl::flat_hash_map<std::string, std::unique_ptr<ZeroCopyOutputStream>>
      stream_map_;
};

// Returns true if the compress_metadata parameter is present.
// Adjusts the parameter argument to remove compress_metadata and include
// requisite options for metadata generation.
bool CompressMetadata(std::string* parameter) {
  bool has_guard_name = false;
  bool compress_metadata = false;
  std::vector<absl::string_view> parts = {kAnnotateHeaderParam};
  for (absl::string_view param : absl::StrSplit(*parameter, ',')) {
    if (absl::StartsWith(param, kAnnotationPragmaParam) ||
        param == kAnnotateHeaderParam) {
      continue;
    }
    if (param == kCompressMetadataParam) {
      compress_metadata = true;
      continue;
    }
    if (absl::StartsWith(param, kAnnotationGuardParam)) {
      has_guard_name = true;
    }
    parts.push_back(param);
  }
  *parameter = absl::StrJoin(parts, ",");
  absl::StrAppend(
      parameter, ",", kAnnotationPragmaParam, "=",
      compress_metadata ? kAnnotationPragmaCompress : kAnnotationPragmaInline);
  if (!has_guard_name) {
    absl::StrAppend(parameter, ",", kAnnotationGuardParam, "=",
                    kAnnotationGuardDefault);
  }
  return compress_metadata;
}

class EmbeddedMetadataGenerator : public CppGenerator {
  bool Generate(const FileDescriptor* file, const std::string& parameter,
                GeneratorContext* context, std::string* error) const override {
    std::string cpp_parameter = parameter;
    WrappedContext wrapper(context, CompressMetadata(&cpp_parameter));
    if (CppGenerator::Generate(file, cpp_parameter, &wrapper, error)) {
      return wrapper.WriteEmbeddedMetadata(error);
    }
    return false;
  }
};

}  // namespace
}  // namespace kythe

int main(int argc, char** argv) {
  kythe::InitializeProgram(argv[0]);
  kythe::EmbeddedMetadataGenerator generator;
  return google::protobuf::compiler::PluginMain(argc, argv, &generator);
}
