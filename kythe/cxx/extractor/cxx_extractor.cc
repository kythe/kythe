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

#include "cxx_extractor.h"

#include <type_traits>
#include <unordered_map>

#include <fcntl.h>
#include <openssl/sha.h>
#include <sys/stat.h>

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/MacroArgs.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Tooling/Tooling.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "kythe/proto/analysis.pb.h"
#include "llvm/Support/Path.h"
#include "third_party/llvm/src/clang_builtin_headers.h"
#include "third_party/llvm/src/cxx_extractor_preprocessor_utils.h"

namespace kythe {
namespace {

// We need "the lowercase ascii hex SHA-256 digest of the file contents."
static constexpr char kHexDigits[] = "0123456789abcdef";

/// \brief Lowercase-string-hex-encodes the array sha_buf.
/// \param sha_buf The bytes of the hash.
static std::string LowercaseStringHexEncodeSha(
    const unsigned char(&sha_buf)[SHA256_DIGEST_LENGTH]) {
  std::string sha_text(SHA256_DIGEST_LENGTH * 2, '\0');
  for (unsigned i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
    sha_text[i * 2] = kHexDigits[(sha_buf[i] >> 4) & 0xF];
    sha_text[i * 2 + 1] = kHexDigits[sha_buf[i] & 0xF];
  }
  return sha_text;
}

/// \brief A SHA-256 hash accumulator.
class RunningHash {
 public:
  RunningHash() { ::SHA256_Init(&sha_context_); }
  /// \brief Update the hash.
  /// \param bytes Start of the memory to use to update.
  /// \param length Number of bytes to read.
  void Update(const void* bytes, size_t length) {
    ::SHA256_Update(&sha_context_,
                    reinterpret_cast<const unsigned char*>(bytes), length);
  }
  /// \brief Update the hash with a string.
  /// \param string The string to include in the hash.
  void Update(llvm::StringRef string) { Update(string.data(), string.size()); }
  /// \brief Update the hash with a `ConditionValueKind`.
  /// \param cvk The enumerator to include in the hash.
  void Update(clang::PPCallbacks::ConditionValueKind cvk) {
    // Make sure that `cvk` has scalar type. This ensures that we can safely
    // hash it by looking at its raw in-memory form without encountering
    // padding bytes with undefined value.
    static_assert(std::is_scalar<decltype(cvk)>::value,
                  "Expected a scalar type.");
    Update(&cvk, sizeof(cvk));
  }
  /// \brief Update the hash with some unsigned integer.
  /// \param u The unsigned integer to include in the hash.
  void Update(unsigned u) { Update(&u, sizeof(u)); }
  /// \brief Return the hash up to this point and reset internal state.
  std::string CompleteAndReset() {
    unsigned char sha_buf[SHA256_DIGEST_LENGTH];
    ::SHA256_Final(sha_buf, &sha_context_);
    ::SHA256_Init(&sha_context_);
    return LowercaseStringHexEncodeSha(sha_buf);
  }

 private:
  ::SHA256_CTX sha_context_;
};

/// \brief Returns the lowercase-string-hex-encoded sha256 digest of the first
/// `length` bytes of `bytes`.
static std::string Sha256(const void* bytes, size_t length) {
  unsigned char sha_buf[SHA256_DIGEST_LENGTH];
  ::SHA256(reinterpret_cast<const unsigned char*>(bytes), length, sha_buf);
  return LowercaseStringHexEncodeSha(sha_buf);
}

/// \brief The state shared among the extractor's various moving parts.
///
/// None of the fields in this struct are owned by the struct.
struct ExtractorState {
  clang::SourceManager* source_manager;
  clang::Preprocessor* preprocessor;
  std::string* main_source_file;
  std::string* main_source_file_transcript;
  std::unordered_map<std::string, SourceFile>* source_files;
};

/// \brief The state we've accumulated within a particular file.
struct FileState {
  std::string file_path;         ///< Clang's path for the file.
  RunningHash history;           ///< Some record of the preprocessor state.
  unsigned last_include_offset;  ///< The #include last seen in this file.
  /// \brief Maps `#include` directives to transcripts we've observed so far.
  std::map<unsigned, PreprocessorTranscript> transcripts;
};

/// \brief Hooks the Clang preprocessor to detect required include files.
class ExtractorPPCallbacks : public clang::PPCallbacks {
 public:
  ExtractorPPCallbacks(ExtractorState state)
      : source_manager_(state.source_manager),
        preprocessor_(state.preprocessor),
        main_source_file_transcript_(state.main_source_file_transcript),
        source_files_(state.source_files) {}

  /// \brief Common utility to pop a file off the file stack.
  ///
  /// Needed because FileChanged(ExitFile) isn't raised when we leave the main
  /// file. Returns the value of the file's transcript.
  PreprocessorTranscript PopFile();

  /// \brief Records the content of `file` (with spelled path `path`)
  /// if it has not already been recorded.
  void AddFile(const clang::FileEntry* file, const std::string& path);

  /// \brief Amends history to include a macro expansion.
  /// \param expansion_loc Where the expansion occurred. Must be in a file.
  /// \param unexpanded The unexpanded form of the macro.
  /// \param expanded The fully expanded form of the macro.
  ///
  /// Note that we expect `expansion_loc` to be a real location. We ignore
  /// mid-macro macro expansions because they have no effect on the resulting
  /// state of the preprocessor. For example:
  ///
  /// ~~~
  /// #define FOO(A, B) A
  /// #define BAR(A, B, C) FOO(A, B)
  /// int x = BAR(1, 2, 3);
  /// ~~~
  ///
  /// We only record that `BAR(1, 2, 3)` was expanded and that it expanded to
  /// `1`.
  void RecordMacroExpansion(clang::SourceLocation expansion_loc,
                            llvm::StringRef unexpanded,
                            llvm::StringRef expanded);

  /// \brief Amends history to include a conditional expression.
  /// \param instance_loc Where the conditional occurred. Must be in a file.
  /// \param directive_kind The directive kind ("#if", etc).
  /// \param value_evaluated What the condition evaluated to.
  /// \param value_unevaluated The unexpanded form of the value.
  void RecordCondition(clang::SourceLocation instance_loc,
                       llvm::StringRef directive_kind,
                       clang::PPCallbacks::ConditionValueKind value_evaluated,
                       llvm::StringRef value_unevaluated);

  void FileChanged(clang::SourceLocation /*Loc*/, FileChangeReason Reason,
                   clang::SrcMgr::CharacteristicKind /*FileType*/,
                   clang::FileID /*PrevFID*/) override;

  void EndOfMainFile() override;

  void MacroExpands(const clang::Token& macro_name,
                    const clang::MacroDirective* macro_directive,
                    clang::SourceRange range,
                    const clang::MacroArgs* macro_args) override;

  void Defined(const clang::Token& macro_name,
               const clang::MacroDirective* macro_directive,
               clang::SourceRange range) override;

  void Elif(clang::SourceLocation location, clang::SourceRange condition_range,
            clang::PPCallbacks::ConditionValueKind value,
            clang::SourceLocation elif_loc) override;

  void If(clang::SourceLocation location, clang::SourceRange condition_range,
          clang::PPCallbacks::ConditionValueKind value) override;

  void Ifdef(clang::SourceLocation location, const clang::Token& macro_name,
             const clang::MacroDirective* macro_directive) override;

  void Ifndef(clang::SourceLocation location, const clang::Token& macro_name,
              const clang::MacroDirective* macro_directive) override;

  void InclusionDirective(
      clang::SourceLocation HashLoc, const clang::Token& IncludeTok,
      llvm::StringRef FileName, bool IsAngled, clang::CharSourceRange Range,
      const clang::FileEntry* File, llvm::StringRef SearchPath,
      llvm::StringRef RelativePath, const clang::Module* Imported) override;

 private:
  /// \brief Returns the main file for this compile action.
  const clang::FileEntry* GetMainFile();

  /// \brief Return the active `RunningHash` for preprocessor events.
  RunningHash* history();

  /// The `SourceManager` used for the compilation.
  clang::SourceManager* source_manager_;
  /// The `Preprocessor` we're attached to.
  clang::Preprocessor* preprocessor_;
  /// The path of the file that was last referenced by an inclusion directive,
  /// normalized for includes that are relative to a different source file.
  std::string last_inclusion_directive_path_;
  /// The offset of the last inclusion directive in bytes from the beginning
  /// of the file containing the directive.
  unsigned last_inclusion_offset_;
  /// The stack of files we've entered. top() gives the current file.
  std::stack<FileState> current_files_;
  /// The transcript of the main source file.
  std::string* main_source_file_transcript_;
  /// Contents of the files we've used, indexed by normalized path.
  std::unordered_map<std::string, SourceFile>* const source_files_;
};

void ExtractorPPCallbacks::FileChanged(
    clang::SourceLocation /*Loc*/, FileChangeReason Reason,
    clang::SrcMgr::CharacteristicKind /*FileType*/, clang::FileID /*PrevFID*/) {
  if (Reason == EnterFile) {
    if (last_inclusion_directive_path_.empty()) {
      current_files_.push(FileState{GetMainFile()->getName()});
    } else {
      CHECK(!current_files_.empty());
      current_files_.top().last_include_offset = last_inclusion_offset_;
      current_files_.push(FileState{last_inclusion_directive_path_});
    }
  } else if (Reason == ExitFile) {
    PreprocessorTranscript transcript = PopFile();
    if (!current_files_.empty()) {
      history()->Update(transcript);
    }
  }
}

PreprocessorTranscript ExtractorPPCallbacks::PopFile() {
  CHECK(!current_files_.empty());
  PreprocessorTranscript top_transcript =
      current_files_.top().history.CompleteAndReset();
  auto file_data = source_files_->find(current_files_.top().file_path);
  if (file_data == source_files_->end()) {
    // We pop the main source file before doing anything interesting.
    return top_transcript;
  }
  auto old_record = file_data->second.include_history.insert(
      std::make_pair(top_transcript, current_files_.top().transcripts));
  if (!old_record.second) {
    if (old_record.first->second != current_files_.top().transcripts) {
      LOG(ERROR) << "Previous record for "
                 << current_files_.top().file_path.c_str() << " for transcript "
                 << top_transcript.c_str()
                 << " differs from the current one.\n";
    }
  }
  current_files_.pop();
  if (!current_files_.empty()) {
    // Backpatch the include information.
    auto& top_file = current_files_.top();
    top_file.transcripts[top_file.last_include_offset] = top_transcript;
  }
  return top_transcript;
}

void ExtractorPPCallbacks::EndOfMainFile() {
  AddFile(GetMainFile(), GetMainFile()->getName());
  *main_source_file_transcript_ = PopFile();
}

void ExtractorPPCallbacks::AddFile(const clang::FileEntry* file,
                                   const std::string& path) {
  auto contents =
      source_files_->insert(std::make_pair(path, SourceFile{std::string()}));
  if (contents.second) {
    const llvm::MemoryBuffer* buffer =
        source_manager_->getMemoryBufferForFile(file);
    contents.first->second.file_content.assign(buffer->getBufferStart(),
                                               buffer->getBufferEnd());
    LOG(INFO) << "added content for " << path << "\n";
  }
}

void ExtractorPPCallbacks::RecordMacroExpansion(
    clang::SourceLocation expansion_loc, llvm::StringRef unexpanded,
    llvm::StringRef expanded) {
  history()->Update(source_manager_->getFileOffset(expansion_loc));
  history()->Update(unexpanded);
  history()->Update(expanded);
}

void ExtractorPPCallbacks::MacroExpands(
    const clang::Token& macro_name,
    const clang::MacroDirective* macro_directive, clang::SourceRange range,
    const clang::MacroArgs* macro_args) {
  clang::SourceLocation macro_location = macro_name.getLocation();
  if (!macro_location.isFileID()) {
    // We don't care about inner macro expansions. We will record state
    // for the initial, unexpanded macro (SECOND_PARAM(A,B,C)) and we will
    // record the result of expansion (SECOND_PARAM(A,B,C) ->* B). It is
    // not necessary for us to record intermediate state because we care
    // only about observational equivalence.
    return;
  }
  const auto* macro_info = macro_directive->getMacroInfo();
  llvm::StringRef macro_name_string =
      macro_name.getIdentifierInfo()->getName().str();
  RecordMacroExpansion(macro_location,
                       getMacroUnexpandedString(range, *preprocessor_,
                                                macro_name_string, macro_info),
                       getMacroExpandedString(*preprocessor_, macro_name_string,
                                              macro_info, macro_args));
}

void ExtractorPPCallbacks::Defined(const clang::Token& macro_name,
                                   const clang::MacroDirective* macro_directive,
                                   clang::SourceRange range) {
  clang::SourceLocation macro_location = macro_name.getLocation();
  RecordMacroExpansion(macro_location, getSourceString(*preprocessor_, range),
                       macro_directive ? "1" : "0");
}

void ExtractorPPCallbacks::RecordCondition(
    clang::SourceLocation instance_loc, llvm::StringRef directive_kind,
    clang::PPCallbacks::ConditionValueKind value_evaluated,
    llvm::StringRef value_unevaluated) {
  history()->Update(source_manager_->getFileOffset(instance_loc));
  history()->Update(directive_kind);
  history()->Update(value_evaluated);
  history()->Update(value_unevaluated);
}

void ExtractorPPCallbacks::Elif(clang::SourceLocation location,
                                clang::SourceRange condition_range,
                                clang::PPCallbacks::ConditionValueKind value,
                                clang::SourceLocation elif_loc) {
  RecordCondition(location, "#elif", value,
                  getSourceString(*preprocessor_, condition_range));
}

void ExtractorPPCallbacks::If(clang::SourceLocation location,
                              clang::SourceRange condition_range,
                              clang::PPCallbacks::ConditionValueKind value) {
  RecordCondition(location, "#if", value,
                  getSourceString(*preprocessor_, condition_range));
}

void ExtractorPPCallbacks::Ifdef(clang::SourceLocation location,
                                 const clang::Token& macro_name,
                                 const clang::MacroDirective* macro_directive) {
  RecordCondition(location, "#ifdef",
                  macro_directive
                      ? clang::PPCallbacks::ConditionValueKind::CVK_True
                      : clang::PPCallbacks::ConditionValueKind::CVK_False,
                  macro_name.getIdentifierInfo()->getName().str());
}

void ExtractorPPCallbacks::Ifndef(
    clang::SourceLocation location, const clang::Token& macro_name,
    const clang::MacroDirective* macro_directive) {
  RecordCondition(location, "#ifndef",
                  macro_directive
                      ? clang::PPCallbacks::ConditionValueKind::CVK_False
                      : clang::PPCallbacks::ConditionValueKind::CVK_True,
                  macro_name.getIdentifierInfo()->getName().str());
}

void ExtractorPPCallbacks::InclusionDirective(
    clang::SourceLocation HashLoc, const clang::Token& IncludeTok,
    llvm::StringRef FileName, bool IsAngled, clang::CharSourceRange Range,
    const clang::FileEntry* File, llvm::StringRef SearchPath,
    llvm::StringRef RelativePath, const clang::Module* Imported) {
  if (File == nullptr) {
    LOG(WARNING) << "Found null file: " << FileName.str();
    LOG(WARNING) << "Search path was " << SearchPath.str();
    LOG(WARNING) << "Relative path was " << RelativePath.str();
    LOG(WARNING) << "Imported was set to " << Imported;
    const auto* options =
        &preprocessor_->getHeaderSearchInfo().getHeaderSearchOpts();
    LOG(WARNING) << "Resource directory is " << options->ResourceDir;
    for (const auto& entry : options->UserEntries) {
      LOG(WARNING) << "User entry: " << entry.Path;
    }
    for (const auto& prefix : options->SystemHeaderPrefixes) {
      LOG(WARNING) << "System entry: " << prefix.Prefix;
    }
    LOG(WARNING) << "Sysroot set to " << options->Sysroot;
    return;
  }
  CHECK(!current_files_.top().file_path.empty());
  const auto* search_path_entry =
      source_manager_->getFileManager().getDirectory(SearchPath);
  const auto* current_file_parent_entry =
      source_manager_->getFileManager()
          .getFile(current_files_.top().file_path.c_str())
          ->getDir();
  // If the include file was found relatively to the current file's parent
  // directory or a search path, we need to normalize it. This is necessary
  // because llvm internalizes the path by which an inode was first accessed,
  // and always returns that path afterwards. If we do not normalize this
  // we will get an error when we replay the compilation, as the virtual
  // file system is not aware of inodes.
  if (search_path_entry == current_file_parent_entry) {
    auto parent = llvm::sys::path::parent_path(
                      current_files_.top().file_path.c_str()).str();

    // If the file is a top level file ("file.cc"), we normalize to a path
    // relative to "./".
    if (parent.empty() || parent == "/") {
      parent = ".";
    }

    // Otherwise we take the literal path as we stored it for the current
    // file, and append the relative path.
    last_inclusion_directive_path_ = parent + "/" + RelativePath.str();
  } else if (!SearchPath.empty()) {
    last_inclusion_directive_path_ =
        SearchPath.str() + "/" + RelativePath.str();
  } else {
    CHECK(llvm::sys::path::is_absolute(FileName)) << FileName.str();
    last_inclusion_directive_path_ = FileName.str();
  }
  last_inclusion_offset_ = source_manager_->getFileOffset(HashLoc);
  AddFile(File, last_inclusion_directive_path_);
}

const clang::FileEntry* ExtractorPPCallbacks::GetMainFile() {
  return source_manager_->getFileEntryForID(source_manager_->getMainFileID());
}

RunningHash* ExtractorPPCallbacks::history() {
  CHECK(!current_files_.empty());
  return &current_files_.top().history;
}

class ExtractorAction : public clang::PreprocessorFrontendAction {
 public:
  explicit ExtractorAction(ExtractorCallback callback) : callback_(callback) {}

  void ExecuteAction() override {
    const auto inputs = getCompilerInstance().getFrontendOpts().Inputs;
    CHECK_EQ(1, inputs.size()) << "Expected to see only one TU; instead saw "
                               << inputs.size() << ".";
    main_source_file_ = inputs[0].getFile();
    auto* preprocessor = &getCompilerInstance().getPreprocessor();
    preprocessor->addPPCallbacks(llvm::make_unique<ExtractorPPCallbacks>(
        ExtractorState{&getCompilerInstance().getSourceManager(), preprocessor,
                       &main_source_file_, &main_source_file_transcript_,
                       &source_files_}));
    preprocessor->EnterMainSourceFile();
    clang::Token token;
    do {
      preprocessor->Lex(token);
    } while (token.isNot(clang::tok::eof));
  }

  void EndSourceFileAction() override {
    callback_(main_source_file_, main_source_file_transcript_, source_files_,
              getCompilerInstance().getDiagnostics().hasErrorOccurred());
  }

 private:
  ExtractorCallback callback_;
  /// The main source file for the compilation (assuming only one).
  std::string main_source_file_;
  /// The transcript of the main source file.
  std::string main_source_file_transcript_;
  /// Contents of the files we've used, indexed by normalized path.
  std::unordered_map<std::string, SourceFile> source_files_;
};

}  // anonymous namespace

void IndexPackWriterSink::OpenIndex(const std::string& path,
                                    const std::string& hash) {
  CHECK(!pack_) << "Opening multiple index packs.";
  std::string error_text;
  auto filesystem = IndexPackPosixFilesystem::Open(
      path, IndexPackFilesystem::OpenMode::kReadWrite, &error_text);
  CHECK(filesystem) << "Couldn't open index pack in " << path << ": "
                    << error_text;
  pack_.reset(new IndexPack(std::move(filesystem)));
}

void IndexPackWriterSink::WriteHeader(
    const kythe::proto::CompilationUnit& header) {
  CHECK(pack_) << "Index pack not opened.";
  std::string error_text;
  CHECK(pack_->AddCompilationUnit(header, &error_text)) << error_text;
}

void IndexPackWriterSink::WriteFileContent(
    const kythe::proto::FileData& content) {
  CHECK(pack_) << "Index pack not opened.";
  std::string error_text;
  CHECK(pack_->AddFileData(content, &error_text)) << error_text;
}

void KindexWriterSink::OpenIndex(const std::string& directory,
                                 const std::string& hash) {
  using namespace google::protobuf::io;
  CHECK(open_path_.empty() && fd_ < 0)
      << "Reopening a KindexWriterSink (old fd:" << fd_
      << " old path: " << open_path_ << ")";
  std::string file_path = directory;
  file_path.append("/" + hash + ".kindex");
  fd_ =
      open(file_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
  CHECK_GE(fd_, 0) << "Couldn't open output file " << file_path;
  open_path_ = file_path;
  file_stream_.reset(new FileOutputStream(fd_));
  GzipOutputStream::Options options;
  // Accept the default compression level and compression strategy.
  options.format = GzipOutputStream::GZIP;
  gzip_stream_.reset(new GzipOutputStream(file_stream_.get(), options));
  coded_stream_.reset(new CodedOutputStream(gzip_stream_.get()));
}

KindexWriterSink::~KindexWriterSink() {
  CHECK(!coded_stream_->HadError()) << "Errors encountered writing to "
                                    << open_path_;
  coded_stream_.reset(nullptr);
  gzip_stream_.reset(nullptr);
  file_stream_.reset(nullptr);
  close(fd_);
}

void KindexWriterSink::WriteHeader(
    const kythe::proto::CompilationUnit& header) {
  coded_stream_->WriteVarint32(header.ByteSize());
  CHECK(header.SerializeToCodedStream(coded_stream_.get()))
      << "Couldn't write header to " << open_path_;
}

void KindexWriterSink::WriteFileContent(const kythe::proto::FileData& content) {
  coded_stream_->WriteVarint32(content.ByteSize());
  CHECK(content.SerializeToCodedStream(coded_stream_.get()))
      << "Couldn't write content to " << open_path_;
}

bool IndexWriter::SetVNameConfiguration(const std::string& json) {
  std::string error_text;
  if (!vname_generator_.LoadJsonString(json, &error_text)) {
    LOG(ERROR) << "Could not parse vname generator configuration: "
               << error_text;
    return false;
  }
  return true;
}

kythe::proto::VName IndexWriter::VNameForPath(const std::string& path) {
  kythe::proto::VName out = vname_generator_.LookupVName(path);
  out.set_language("c++");
  if (!out.has_corpus()) {
    out.set_corpus(corpus_);
  }
  return out;
}

std::string IndexWriter::MakeCleanAbsolutePath(const std::string& in_path) {
  using namespace llvm::sys::path;
  std::string abs_path = clang::tooling::getAbsolutePath(in_path);
  std::string root_part =
      (root_name(abs_path) + root_directory(abs_path)).str();
  llvm::SmallString<1024> out_path = llvm::StringRef(root_part);
  std::vector<llvm::StringRef> path_components;
  int skip_count = 0;
  for (auto node = rbegin(abs_path), node_end = rend(abs_path);
       node != node_end; ++node) {
    if (*node == "..") {
      ++skip_count;
    } else if (*node != ".") {
      if (skip_count > 0) {
        --skip_count;
      } else {
        path_components.push_back(*node);
      }
    }
  }
  for (auto node = path_components.crbegin(),
            node_end = path_components.crend();
       node != node_end; ++node) {
    append(out_path, *node);
  }
  return out_path.str();
}

std::string IndexWriter::RelativizePath(const std::string& to_relativize,
                                        const std::string& relativize_against) {
  std::string to_relativize_abs = MakeCleanAbsolutePath(to_relativize);
  std::string relativize_against_abs =
      MakeCleanAbsolutePath(relativize_against);
  llvm::StringRef to_relativize_parent =
      llvm::sys::path::parent_path(to_relativize_abs);
  std::string ret =
      to_relativize_parent.startswith(relativize_against_abs)
          ? to_relativize_abs.substr(relativize_against_abs.size() +
                                     llvm::sys::path::get_separator().size())
          : to_relativize_abs;
  return ret;
}

void IndexWriter::FillFileInput(
    const std::string& clang_path, const SourceFile& source_file,
    kythe::proto::CompilationUnit_FileInput* file_input) {
  file_input->mutable_v_name()->CopyFrom(
      VNameForPath(RelativizePath(clang_path, root_directory_)));
  // This path is distinct from the VName path. It is used by analysis tools
  // to configure Clang's virtual filesystem.
  auto* file_info = file_input->mutable_info();
  file_info->set_path(clang_path);
  file_info->set_digest(Sha256(source_file.file_content.c_str(),
                               source_file.file_content.size()));
  for (const auto& row : source_file.include_history) {
    auto* row_pb = file_input->add_context();
    row_pb->set_source_context(row.first);
    for (const auto& col : row.second) {
      auto* col_pb = row_pb->add_column();
      col_pb->set_offset(col.first);
      col_pb->set_linked_context(col.second);
    }
  }
}

void IndexWriter::WriteIndex(
    std::unique_ptr<IndexWriterSink> sink, const std::string& main_source_file,
    const std::string& entry_context,
    const std::unordered_map<std::string, SourceFile>& source_files,
    bool had_errors) {
  kythe::proto::CompilationUnit unit;
  std::string identifying_blob;
  identifying_blob.append(corpus_);
  for (const auto& arg : args_) {
    identifying_blob.append(arg);
    unit.add_argument(arg);
  }
  identifying_blob.append(main_source_file);
  std::string identifying_blob_digest =
      Sha256(identifying_blob.c_str(), identifying_blob.size());
  auto* unit_vname = unit.mutable_v_name();

  kythe::proto::VName main_vname = VNameForPath(main_source_file);
  unit_vname->CopyFrom(main_vname);
  unit_vname->set_signature("cu#" + identifying_blob_digest);
  unit_vname->clear_path();

  for (const auto& file : source_files) {
    FillFileInput(file.first, file.second, unit.add_required_input());
  }
  unit.set_entry_context(entry_context);
  unit.set_has_compile_errors(had_errors);
  unit.add_source_file(main_source_file);
  unit.set_working_directory(root_directory_);
  sink->OpenIndex(output_directory_, identifying_blob_digest);
  sink->WriteHeader(unit);
  unsigned info_index = 0;
  for (const auto& file : source_files) {
    kythe::proto::FileData file_content;
    file_content.set_content(file.second.file_content);
    file_content.mutable_info()->CopyFrom(
        unit.required_input(info_index++).info());
    sink->WriteFileContent(file_content);
  }
}

std::unique_ptr<clang::FrontendAction> NewExtractor(
    ExtractorCallback callback) {
  return std::unique_ptr<clang::FrontendAction>(new ExtractorAction(callback));
}

void MapCompilerResources(clang::tooling::ToolInvocation* invocation,
                          const char* map_directory) {
  llvm::StringRef map_directory_ref(map_directory);
  for (const auto* file = builtin_headers_create(); file->name; ++file) {
    llvm::SmallString<1024> out_path = map_directory_ref;
    llvm::sys::path::append(out_path, "include");
    llvm::sys::path::append(out_path, file->name);
    invocation->mapVirtualFile(out_path, file->data);
  }
}

}  // namespace kythe
