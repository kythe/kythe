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

#include "cxx_extractor.h"

#include <map>
#include <memory>

#include "absl/log/check.h"
#include "absl/log/initialize.h"
#include "absl/log/log.h"
#include "absl/memory/memory.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/Tooling.h"
#include "gtest/gtest.h"
#include "kythe/cxx/common/path_utils.h"
#include "kythe/proto/analysis.pb.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"

namespace kythe {
namespace {

class CxxExtractorTest : public testing::Test {
 protected:
  CxxExtractorTest() {
    // We use the real filesystem here so that we can test real filesystem
    // features, like symlinks.
    CHECK_EQ(0,
             llvm::sys::fs::createUniqueDirectory("cxx_extractor_test", root_)
                 .value());
    directories_to_remove_.insert(std::string(root_.str()));
  }

  ~CxxExtractorTest() {
    // Do the best we can to clean up the temporary files we've made.
    std::error_code err;
    for (const auto& file : files_to_remove_) {
      err = llvm::sys::fs::remove(file);
      if (err.value()) {
        LOG(WARNING) << "Couldn't remove " << file << ": " << err.message();
      }
    }
    // ::remove refuses to remove non-empty directories and we'd like to avoid
    // recursive traversal out of fear of being bitten by symlinks.
    bool progress = true;
    while (progress) {
      progress = false;
      for (const auto& dir : directories_to_remove_) {
        if (!llvm::sys::fs::remove(llvm::Twine(dir)).value()) {
          directories_to_remove_.erase(dir);
          progress = true;
          break;
        }
      }
    }
    if (!directories_to_remove_.empty()) {
      LOG(WARNING) << "Couldn't remove temporary directories.";
    }
  }

  /// Creates all the non-existent directories in `path` and destroys them
  /// when the test completes. Ported from `llvm::sys::fs::create_directories`.
  void UndoableCreateDirectories(const llvm::Twine& path) {
    llvm::SmallString<128> path_storage;
    const auto& path_string = path.toStringRef(path_storage);

    // Be optimistic and try to create the directory
    std::error_code err = llvm::sys::fs::create_directory(path_string, true);
    // Optimistically check if we succeeded.
    if (err != std::errc::no_such_file_or_directory) {
      ASSERT_EQ(0, err.value());
      directories_to_remove_.insert(path.str());
      return;
    }

    // We failed because of a no_such_file_or_directory, try to create the
    // parent.
    llvm::StringRef parent = llvm::sys::path::parent_path(path_string);
    ASSERT_FALSE(parent.empty());
    UndoableCreateDirectories(parent);
    ASSERT_EQ(0, llvm::sys::fs::create_directory(path_string).value());
    directories_to_remove_.insert(path_string.str());
  }

  /// \param path Absolute path, beginning with / (or B:\ or \\, etc), to the
  /// file to create.
  /// \param code Code to write at the file named by `path`.
  void AddAbsoluteSourceFile(llvm::StringRef path, const std::string& code) {
    int write_fd;
    UndoableCreateDirectories(path);
    ASSERT_EQ(0, llvm::sys::fs::remove(path).value());
    ASSERT_EQ(0, llvm::sys::fs::openFileForWrite(path, write_fd,
                                                 llvm::sys::fs::CD_CreateAlways,
                                                 llvm::sys::fs::OF_Text)
                     .value());
    ASSERT_EQ(code.size(), ::write(write_fd, code.c_str(), code.size()));
    ASSERT_EQ(0, ::close(write_fd));
    files_to_remove_.insert(std::string(path));
  }

  /// \brief Creates a link named `from` pointing at `to`. Destroys it
  /// on exit.
  ///
  /// This uses `llvm::sys::fs::create_link`, which does different things
  /// depending on the platform (symlinks on Linux, something else on Windows)
  void CreateLink(const llvm::Twine& to, const llvm::Twine& from) {
    ASSERT_EQ(0, llvm::sys::fs::create_link(to, from).value());
    files_to_remove_.insert(from.str());
  }

  /// \brief Returns an absolute path from the test temporary directory
  /// ending with `relative_path`.
  std::string GetRootedPath(const llvm::Twine& relative_path) {
    llvm::SmallString<256> appended_path;
    llvm::sys::path::append(appended_path, llvm::Twine(root_), relative_path);
    CHECK_EQ(0, llvm::sys::fs::make_absolute(appended_path).value());
    return std::string(appended_path.str());
  }

  /// \brief Adds a file at relative path `path` with content `code`.
  void AddSourceFile(const llvm::Twine& path, const std::string& code) {
    std::string absolute_path = GetRootedPath(path);
    AddAbsoluteSourceFile(absolute_path, code);
  }

  /// \brief An `CompilationWriterSink` that only holds protocol buffers in
  /// memory.
  class CapturingCompilationWriterSink : public kythe::CompilationWriterSink {
   public:
    void OpenIndex(const std::string& unit_hash) override {}
    void WriteHeader(const kythe::proto::CompilationUnit& header) override {
      units_.push_back(header);
    }
    void WriteFileContent(const kythe::proto::FileData& content) override {
      data_.push_back(content);
    }
    const std::vector<kythe::proto::CompilationUnit>& units() const {
      return units_;
    }
    const std::vector<kythe::proto::FileData>& data() const { return data_; }

   private:
    std::vector<kythe::proto::CompilationUnit> units_;
    std::vector<kythe::proto::FileData> data_;
  };

  /// \brief An `CompilationWriterSink` that forwards all calls to another sink.
  class ForwardingCompilationWriterSink : public kythe::CompilationWriterSink {
   public:
    ForwardingCompilationWriterSink(
        kythe::CompilationWriterSink* underlying_sink)
        : underlying_sink_(underlying_sink) {}
    void OpenIndex(const std::string& unit_hash) override {
      underlying_sink_->OpenIndex(unit_hash);
    }
    void WriteHeader(const kythe::proto::CompilationUnit& header) override {
      underlying_sink_->WriteHeader(header);
    }
    void WriteFileContent(const kythe::proto::FileData& content) override {
      underlying_sink_->WriteFileContent(content);
    }

   private:
    kythe::CompilationWriterSink* underlying_sink_;
  };

  /// \brief Runs the extractor on a given source file, directing its output
  /// to a particular sink.
  /// \param source_file Path to the main source file.
  /// \param arguments Arguments (not including the executable name or main
  /// source file).
  /// \param required_inputs Paths to required input files. (TODO(zarko): keep?)
  /// \param revision Revision ID for this index (TODO(zarko): keep?)
  /// \param output Output file for the compilation.
  /// \param sink Where to send the result protobufs.
  void FillCompilationUnit(const std::string& source_file,
                           const std::vector<std::string>& arguments,
                           const std::vector<std::string>& required_inputs,
                           const int revision, const std::string& output,
                           CapturingCompilationWriterSink* sink) {
    std::vector<std::string> final_arguments = {"tool", "-o", output,
                                                "-fsyntax-only", source_file};
    final_arguments.insert(final_arguments.end(), arguments.begin(),
                           arguments.end());
    kythe::CompilationWriter index_writer;
    index_writer.set_root_directory(std::string(root_.str()));
    index_writer.set_args(final_arguments);
    kythe::proto::CompilationUnit unit;
    clang::FileSystemOptions file_system_options;
    file_system_options.WorkingDir = std::string(root_.str());
    llvm::IntrusiveRefCntPtr<clang::FileManager> file_manager(
        new clang::FileManager(file_system_options));
    auto extractor = kythe::NewExtractor(
        &index_writer,
        [&index_writer, &sink](
            const std::string& main_source_file,
            const PreprocessorTranscript& transcript,
            const std::unordered_map<std::string, SourceFile>& source_files,
            const HeaderSearchInfo* header_search_info, bool had_errors) {
          index_writer.WriteIndex(
              supported_language::Language::kCpp,
              std::make_unique<ForwardingCompilationWriterSink>(sink),
              main_source_file, transcript, source_files, header_search_info,
              had_errors);
        });
    clang::tooling::ToolInvocation invocation(
        final_arguments, std::move(extractor), file_manager.get());
    invocation.run();
  }

  /// \brief Checks that all the files referenced by the compilation unit
  /// seen by `sink` have also been written out in `sink`.
  void VerifyCompilationUnit(const CapturingCompilationWriterSink& sink,
                             const int revision, const std::string& output) {
    ASSERT_EQ(1, sink.units().size());
    // TODO(zarko): use or delete revision and output.
    const kythe::proto::CompilationUnit& unit = sink.units().front();
    for (const auto& file_input : unit.required_input()) {
      const std::string& path = file_input.info().path();
      bool found_data = false;
      for (const auto& file_data : sink.data()) {
        if (file_data.info().path() == path) {
          found_data = true;
          break;
        }
      }
      EXPECT_TRUE(found_data) << "Missing data for " << path;
    }
  }

  void FillAndVerifyCompilationUnit(
      const std::string& path, const std::vector<std::string>& arguments,
      const std::vector<std::string>& required_inputs, const int revision,
      const std::string& output) {
    // TODO(zarko): Is it useful to preserve the old test suite's
    // `required_inputs` and `revision`, or should they be eliminated?
    CapturingCompilationWriterSink sink;
    FillCompilationUnit(path, arguments, required_inputs, revision, output,
                        &sink);
    VerifyCompilationUnit(sink, revision, output);
  }

  void FillAndVerifyCompilationUnit(
      const std::string& path, const std::vector<std::string>& arguments,
      const std::vector<std::string>& required_inputs, const int revision) {
    FillAndVerifyCompilationUnit(path, arguments, required_inputs, revision,
                                 "output.o");
  }

  void FillAndVerifyCompilationUnit(
      const std::string& path, const std::vector<std::string>& arguments,
      const std::vector<std::string>& required_inputs) {
    FillAndVerifyCompilationUnit(path, arguments, required_inputs, 1);
  }

  /// Path to a directory for test files.
  llvm::SmallString<256> root_;
  /// Files to clean up after the test ends.
  std::set<std::string> files_to_remove_;
  /// Directories to clean up after the test ends.
  std::set<std::string> directories_to_remove_;
};

TEST_F(CxxExtractorTest, SimpleCase) {
  AddSourceFile("b.cc", "#include \"b.h\"\nint main() { return 0; }");
  AddSourceFile("./b.h", "int x;");
  FillAndVerifyCompilationUnit("b.cc", {}, {"./b.h", "b.cc"});
}

TEST_F(CxxExtractorTest, CanIndexIncludesInSubdirectories) {
  AddSourceFile("a/b.cc", "#include \"c/b.h\"\nint main() { return 0; }");
  AddSourceFile("a/c/b.h", "int x;");
  FillAndVerifyCompilationUnit("a/b.cc", {"-Ia"}, {"a/b.cc", "a/c/b.h"}, 5);
}

TEST_F(CxxExtractorTest, StoresAllAccessPatternsForIncludes) {
  AddSourceFile("./lib/b/b.h", "#include \"../a/a.h\"");
  AddSourceFile("./lib/a/a.h", "int a();");
  AddSourceFile("./lib/b/../a/a.h", "int a();");
  AddSourceFile("b.cc",
                "#include \"lib/b/b.h\"\n"
                "#include \"lib/a/a.h\"\n"
                "int main() { return 0; }");
  FillAndVerifyCompilationUnit(
      "b.cc", {"-Ilib/b"},
      {"./lib/a/a.h", "./lib/b/../a/a.h", "./lib/b/b.h", "b.cc"});
}

TEST_F(CxxExtractorTest, StoresSameAccessPatternsForReversedIncludes) {
  AddSourceFile("./lib/b/b.h", "#include \"../a/a.h\"");
  AddSourceFile("./lib/a/a.h", "int a();");
  AddSourceFile("./lib/b/../a/a.h", "int a();");
  AddSourceFile("b.cc",
                "#include \"lib/a/a.h\"\n"
                "#include \"lib/b/b.h\"\n"
                "int main() { return 0; }");
  FillAndVerifyCompilationUnit(
      "b.cc", {"-Ilib/b"},
      {"./lib/a/a.h", "./lib/b/../a/a.h", "./lib/b/b.h", "b.cc"});
}

TEST_F(CxxExtractorTest, SupportsIncludeFilesAccessViaDifferentPaths) {
  AddSourceFile("lib/a/a.h", "class A;");
  AddSourceFile("./lib/a/b.h", "class B;");
  AddSourceFile("./lib/a/c.h", "#include \"b.h\"");
  AddSourceFile("lib/a/z.cc",
                "#include \"a.h\"\n"
                "#include \"lib/a/c.h\"\n"
                "int main() { return 0; }");
  FillAndVerifyCompilationUnit(
      "lib/a/z.cc", {"-I."},
      {"./lib/a/b.h", "./lib/a/c.h", "lib/a/a.h", "lib/a/z.cc"});
}

TEST_F(CxxExtractorTest, SupportsAbsoluteIncludes) {
  std::string absolute_header = GetRootedPath("/a/a.h");
  AddAbsoluteSourceFile(absolute_header, "class A;");

  AddSourceFile("z.cc", "#include \"" + absolute_header +
                            "\"\n"
                            "int main() { return 0; }");
  FillAndVerifyCompilationUnit("z.cc", {"-I."},
                               {absolute_header.c_str(), "z.cc"});
}

TEST_F(CxxExtractorTest, SupportsIncludeFilesViaSymlinkedSearchPaths) {
  AddSourceFile("a/sub/a.h", "class A;");
  CreateLink(llvm::Twine(GetRootedPath("a/sub")),
             llvm::Twine(GetRootedPath("b")));

  AddSourceFile("c/z.cc",
                "#include \"a.h\"\n"
                "#include \"sub/a.h\"\n"
                "int main() { return 0; }");
  FillAndVerifyCompilationUnit("c/z.cc", {"-Ia", "-Ib"},
                               {"a/sub/a.h", "b/a.h", "c/z.cc"});
}

TEST_F(CxxExtractorTest, DoesNotBreakForCompilerErrors) {
  AddSourceFile("b.cc", "#include \"b.h\"\nsome ; & garbage");
  AddSourceFile("./b.h", "more garbage");
  FillAndVerifyCompilationUnit("b.cc", {}, {"./b.h", "b.cc"});
}

TEST_F(CxxExtractorTest, DoesNotBreakForMissingIncludes) {
  AddSourceFile("b.cc", "#include \"D.h\"\nint main() { return 0;}");
  AddSourceFile("./b.h", "class A;");
  FillAndVerifyCompilationUnit("b.cc", {}, {"./b.h", "b.cc"});
}

}  // anonymous namespace
}  // namespace kythe

int main(int argc, char** argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  absl::InitializeLog();
  ::testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();
  return result;
}
