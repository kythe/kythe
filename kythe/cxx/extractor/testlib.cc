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

#include "kythe/cxx/extractor/testlib.h"

#include <errno.h>
#include <stdlib.h>

#include <iostream>
#include <optional>
#include <utility>

#include "absl/container/flat_hash_map.h"
#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "gmock/gmock.h"
#include "google/protobuf/message.h"
#include "google/protobuf/text_format.h"
#include "gtest/gtest.h"
#include "kythe/cxx/common/kzip_reader.h"
#include "kythe/cxx/common/path_utils.h"
#include "kythe/proto/analysis.pb.h"
#include "kythe/proto/cxx.pb.h"
#include "kythe/proto/filecontext.pb.h"
#include "llvm/Support/Program.h"
#include "tools/cpp/runfiles/runfiles.h"

namespace kythe {
namespace {
namespace gpb = ::google::protobuf;
namespace kpb = ::kythe::proto;
using HashMap = ::absl::flat_hash_map<std::string, std::size_t>;
using ::bazel::tools::cpp::runfiles::Runfiles;

// Path prefix joined to runfiles to form the workspace-relative path.
constexpr absl::string_view kWorkspaceRoot = "io_kythe";

constexpr absl::string_view kExtractorPath =
    "kythe/cxx/extractor/cxx_extractor";

void CanonicalizeHash(HashMap* hashes, std::string* hash) {
  auto inserted = hashes->insert({*hash, hashes->size()});
  *hash = absl::StrCat("hash", inserted.first->second);
}

/// \brief Range wrapper around ContextDependentVersion, if any.
class MutableContextRows {
 public:
  using iterator = decltype(std::declval<kpb::ContextDependentVersion>()
                                .mutable_row()
                                ->begin());
  explicit MutableContextRows(kpb::CompilationUnit::FileInput* file_input) {
    for (gpb::Any& detail : *file_input->mutable_details()) {
      if (detail.UnpackTo(&context_)) {
        any_ = &detail;
      }
    }
  }

  ~MutableContextRows() {
    if (any_ != nullptr) {
      any_->PackFrom(context_);
    }
  }

  iterator begin() { return context_.mutable_row()->begin(); }
  iterator end() { return context_.mutable_row()->end(); }

 private:
  gpb::Any* any_ = nullptr;
  kpb::ContextDependentVersion context_;
};

// RAII class for changing working directory.
// Ideally, this would be done after fork(), but before exec() except this isn't
// supported by LLVM ExecuteAndWait.
class ScopedWorkingDirectory {
 public:
  explicit ScopedWorkingDirectory(std::string working_directory)
      : saved_working_directory_(
            working_directory.empty() ? "" : GetCurrentDirectory().value()) {
    if (working_directory.empty()) return;
    CHECK(::chdir(working_directory.c_str()) == 0)
        << "unable to chdir(" << working_directory << "): " << strerror(errno);
  }
  ~ScopedWorkingDirectory() {
    if (saved_working_directory_.empty()) return;

    CHECK(::chdir(saved_working_directory_.c_str()) == 0)
        << "unable to chdir(" << saved_working_directory_
        << "): " << strerror(errno);
  }

 private:
  std::string saved_working_directory_;
};

// RAII class which removes the designated file name on destruction.
class TemporaryKzipFile {
 public:
  explicit TemporaryKzipFile() : filename_(KzipFileName()) {}

  const std::string& filename() const { return filename_; };
  ~TemporaryKzipFile() {
    if (::unlink(filename_.c_str()) != 0) {
      LOG(ERROR) << "unable to remove " << filename_ << ": " << strerror(errno);
    }
    std::string::size_type slash = filename_.find_last_of('/');
    if (slash != std::string::npos) {
      std::string dirname = filename_.substr(0, slash);
      if (::rmdir(dirname.c_str()) != 0) {
        LOG(ERROR) << "unable to remove " << dirname << ": " << strerror(errno);
      }
    }
  }

 private:
  static std::string KzipFileName() {
    const testing::TestInfo* test_info =
        testing::UnitTest::GetInstance()->current_test_info();
    std::string testname =
        test_info
            ? absl::StrCat(test_info->test_suite_name(), "_", test_info->name())
            : "kzip_test";

    std::string base =
        absl::StrCat(JoinPath(testing::TempDir(), testname), "XXXXXX");

    CHECK(mkdtemp(&base.front()) != nullptr)
        << "unable to create temporary directory: " << strerror(errno);

    return JoinPath(base, "test.kzip");
  }

  std::string filename_;
};

// Transforms std::vector<std::string> into a null-terminated array of c-string
// pointers suitable for passing to posix_spawn.
std::vector<llvm::StringRef> VectorForExecute(
    const std::vector<std::string>& values) {
  std::vector<llvm::StringRef> result;
  result.reserve(values.size());
  for (const auto& arg : values) {
    result.push_back(arg.c_str());
  }
  return result;
}

// Transforms a map of {key, value} pairs into a flattened vector of "key=value"
// strings.
std::vector<std::string> FlattenEnvironment(
    const absl::flat_hash_map<std::string, std::string>& env) {
  std::vector<std::string> result;
  result.reserve(env.size());
  for (const auto& entry : env) {
    result.push_back(absl::StrCat(entry.first, "=", entry.second));
  }
  return result;
}

bool ExecuteAndWait(const std::string& program,
                    const std::vector<std::string>& argv,
                    const std::vector<std::string>& env,
                    const std::string& working_directory) {
  ScopedWorkingDirectory directory(working_directory);

  std::string error;
  bool execution_failed = false;
  int exit_code = llvm::sys::ExecuteAndWait(
      program, VectorForExecute(argv), VectorForExecute(env),
      /* Redirects */ std::nullopt,
      /* SecondsToWait */ 0, /* MemoryLimit */ 0, &error, &execution_failed);
  if (!error.empty() || execution_failed || exit_code != 0) {
    LOG(ERROR) << "unable to run extractor (" << exit_code << "): " << error;
    return false;
  }
  return true;
}
}  // namespace

void CanonicalizeHashes(kpb::CompilationUnit* unit) {
  HashMap hashes;
  CanonicalizeHash(&hashes, unit->mutable_entry_context());
  for (auto& input : *unit->mutable_required_input()) {
    for (auto& row : MutableContextRows(&input)) {
      CanonicalizeHash(&hashes, row.mutable_source_context());
      for (auto& column : *row.mutable_column()) {
        CanonicalizeHash(&hashes, column.mutable_linked_context());
      }
    }
  }
}

std::optional<std::vector<kpb::CompilationUnit>> ExtractCompilations(
    ExtractorOptions options) {
  gpb::LinkMessageReflection<kpb::CxxCompilationUnitDetails>();

  options.environment.insert({"KYTHE_EXCLUDE_EMPTY_DIRS", "1"});
  options.environment.insert({"KYTHE_EXCLUDE_AUTOCONFIGURATION_FILES", "1"});
  if (std::optional<std::string> extractor = ResolveRunfiles(kExtractorPath)) {
    TemporaryKzipFile output_file;
    options.environment.insert({"KYTHE_OUTPUT_FILE", output_file.filename()});

    // Add the extractor path as argv[0].
    options.arguments.insert(options.arguments.begin(), *extractor);
    if (ExecuteAndWait(*extractor, options.arguments,
                       FlattenEnvironment(options.environment),
                       options.working_directory)) {
      if (auto reader = KzipReader::Open(output_file.filename()); reader.ok()) {
        std::optional<std::vector<kpb::CompilationUnit>> result(
            absl::in_place);  // Default construct a result vector.

        auto status = reader->Scan([&](absl::string_view digest) {
          if (auto unit = reader->ReadUnit(digest); unit.ok()) {
            result->push_back(std::move(*unit->mutable_unit()));
            return true;
          } else {
            LOG(ERROR) << "Unable to read CompilationUnit " << digest << ": "
                       << unit.status();
            result.reset();
            return false;
          }
        });
        if (!status.ok()) {
          LOG(ERROR) << "Unable to read compilations: " << status;
          return std::nullopt;
        }
        return result;
      } else {
        LOG(ERROR) << "Unable to open " << output_file.filename() << ": "
                   << reader.status();
      }
      return std::nullopt;
    }

  } else {
    LOG(ERROR) << "Unable to resolve extractor path";
  }
  return std::nullopt;
}

std::optional<std::string> ResolveRunfiles(absl::string_view path) {
  std::string error;
  std::unique_ptr<Runfiles> runfiles(Runfiles::CreateForTest(&error));
  if (runfiles == nullptr) {
    LOG(ERROR) << error;
    return std::nullopt;
  }
  std::string resolved = runfiles->Rlocation(JoinPath(kWorkspaceRoot, path));
  if (resolved.empty()) {
    return std::nullopt;
  }
  return resolved;
}

kpb::CompilationUnit ExtractSingleCompilationOrDie(ExtractorOptions options) {
  if (std::optional<std::vector<kpb::CompilationUnit>> result =
          ExtractCompilations(std::move(options))) {
    CHECK(result->size() == 1)
        << "unexpected number of extracted compilations: " << result->size();
    return std::move(result->front());
  } else {
    LOG(FATAL) << "Unable to extract compilation";
  }
}

kpb::CompilationUnit ParseTextCompilationUnitOrDie(absl::string_view text) {
  kpb::CompilationUnit result;
  CHECK(gpb::TextFormat::ParseFromString(std::string(text), &result));
  return result;
}
}  // namespace kythe
