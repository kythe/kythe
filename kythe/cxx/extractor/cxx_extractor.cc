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
#include "clang/Lex/MacroArgs.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Tooling/Tooling.h"

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "kythe/cxx/common/CommandLineUtils.h"
#include "kythe/cxx/common/json_proto.h"
#include "kythe/cxx/common/path_utils.h"
#include "kythe/cxx/common/proto_conversions.h"
#include "kythe/proto/analysis.pb.h"
#include "kythe/proto/cxx.pb.h"
#include "llvm/Support/TargetSelect.h"
#include "third_party/llvm/src/clang_builtin_headers.h"
#include "third_party/llvm/src/cxx_extractor_preprocessor_utils.h"

namespace kythe {
namespace {

// We need "the lowercase ascii hex SHA-256 digest of the file contents."
static constexpr char kHexDigits[] = "0123456789abcdef";

/// \brief Lowercase-string-hex-encodes the array sha_buf.
/// \param sha_buf The bytes of the hash.
static std::string LowercaseStringHexEncodeSha(
    const unsigned char (&sha_buf)[SHA256_DIGEST_LENGTH]) {
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
  /// \brief Update the hash with the relevant values from a `LanguageOptions`
  /// \param options The options to include in the hash.
  void Update(const clang::LangOptions& options) {
    // These configuration options change the way definitions are interpreted
    // (see clang::Builtin::Context::BuiltinIsSupported).
    Update(options.NoBuiltin ? "no_builtin" : "builtin");
    Update(options.NoMathBuiltin ? "no_math_builtin" : "math_builtin");
    Update(options.Freestanding ? "freestanding" : "not_freestanding");
    Update(options.GNUMode ? "GNUmode" : "not_GNUMode");
    Update(options.MicrosoftExt ? "MSMode" : "not_MSMode");
    Update(options.ObjC1 ? "ObjC1" : "not_ObjC1");
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
  IndexWriter* index_writer;
  clang::SourceManager* source_manager;
  clang::Preprocessor* preprocessor;
  std::string* main_source_file;
  std::string* main_source_file_transcript;
  std::unordered_map<std::string, SourceFile>* source_files;
  std::string* main_source_file_stdin_alternate;
};

/// \brief The state we've accumulated within a particular file.
struct FileState {
  std::string file_path;  ///< Clang's path for the file.
  /// The default claim behavior for this version.
  ClaimDirective default_behavior;
  RunningHash history;           ///< Some record of the preprocessor state.
  unsigned last_include_offset;  ///< The #include last seen in this file.
  /// \brief Maps `#include` directives (identified as byte offsets from the
  /// start of the file to the #) to transcripts we've observed so far.
  std::map<unsigned, PreprocessorTranscript> transcripts;
};

/// \brief Hooks the Clang preprocessor to detect required include files.
class ExtractorPPCallbacks : public clang::PPCallbacks {
 public:
  ExtractorPPCallbacks(ExtractorState state);

  /// \brief Common utility to pop a file off the file stack.
  ///
  /// Needed because FileChanged(ExitFile) isn't raised when we leave the main
  /// file. Returns the value of the file's transcript.
  PreprocessorTranscript PopFile();

  /// \brief Records the content of `file` (with spelled path `path`)
  /// if it has not already been recorded.
  void AddFile(const clang::FileEntry* file, const std::string& path);

  /// \brief Records the content of `file` if it has not already been recorded.
  std::string AddFile(const clang::FileEntry* file, llvm::StringRef file_name,
                      llvm::StringRef search_path,
                      llvm::StringRef relative_path);

  /// \brief Amends history to include a macro expansion.
  /// \param expansion_loc Where the expansion occurred. Must be in a file.
  /// \param definition_loc Where the expanded macro was defined.
  /// May be invalid.
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

  /// \brief Records `loc` as an offset along with its vname.
  void RecordSpecificLocation(clang::SourceLocation loc);

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
                    const clang::MacroDefinition& macro_definition,
                    clang::SourceRange range,
                    const clang::MacroArgs* macro_args) override;

  void MacroDefined(const clang::Token& macro_name,
                    const clang::MacroDirective* macro_directive) override;

  void MacroUndefined(const clang::Token& macro_name,
                      const clang::MacroDefinition& macro_definition) override;

  void Defined(const clang::Token& macro_name,
               const clang::MacroDefinition& macro_definition,
               clang::SourceRange range) override;

  void Elif(clang::SourceLocation location, clang::SourceRange condition_range,
            clang::PPCallbacks::ConditionValueKind value,
            clang::SourceLocation elif_loc) override;

  void If(clang::SourceLocation location, clang::SourceRange condition_range,
          clang::PPCallbacks::ConditionValueKind value) override;

  void Ifdef(clang::SourceLocation location, const clang::Token& macro_name,
             const clang::MacroDefinition& macro_definition) override;

  void Ifndef(clang::SourceLocation location, const clang::Token& macro_name,
              const clang::MacroDefinition& macro_definition) override;

  void InclusionDirective(
      clang::SourceLocation HashLoc, const clang::Token& IncludeTok,
      llvm::StringRef FileName, bool IsAngled, clang::CharSourceRange Range,
      const clang::FileEntry* File, llvm::StringRef SearchPath,
      llvm::StringRef RelativePath, const clang::Module* Imported) override;

  /// \brief Run by a `clang::PragmaHandler` to handle the `kythe_claim` pragma.
  ///
  /// This has the same semantics as `clang::PragmaHandler::HandlePragma`.
  /// We pass Clang a throwaway `PragmaHandler` instance that delegates to
  /// this member function.
  ///
  /// \sa clang::PragmaHandler::HandlePragma
  void HandleKytheClaimPragma(clang::Preprocessor& preprocessor,
                              clang::PragmaIntroducerKind introducer,
                              clang::Token& first_token);

  /// \brief Run by a `clang::PragmaHandler` to handle the `kythe_metadata`
  /// pragma.
  ///
  /// This has the same semantics as `clang::PragmaHandler::HandlePragma`.
  /// We pass Clang a throwaway `PragmaHandler` instance that delegates to
  /// this member function.
  ///
  /// \sa clang::PragmaHandler::HandlePragma
  void HandleKytheMetadataPragma(clang::Preprocessor& preprocessor,
                                 clang::PragmaIntroducerKind introducer,
                                 clang::Token& first_token);

 private:
  /// \brief Returns the main file for this compile action.
  const clang::FileEntry* GetMainFile();

  /// \brief Return the active `RunningHash` for preprocessor events.
  RunningHash* history();

  /// \brief Ensures that the main source file, if read from stdin,
  /// is given the correct name for VName generation.
  ///
  /// Files read from standard input still must be distinguished
  /// from one another. We name these files as "<stdin:hash>",
  /// where the hash is taken from the file's content at the time
  /// of extraction.
  ///
  /// \param file The file entry of the main source file.
  /// \param path The path as known to Clang.
  /// \return The path that should be used to generate VNames.
  std::string FixStdinPath(const clang::FileEntry* file,
                           const std::string& path);

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
  /// The active IndexWriter.
  IndexWriter* index_writer_;
  /// Non-empty if the main source file was stdin ("-") and we have chosen
  /// a new name for it.
  std::string* main_source_file_stdin_alternate_;
};

ExtractorPPCallbacks::ExtractorPPCallbacks(ExtractorState state)
    : source_manager_(state.source_manager),
      preprocessor_(state.preprocessor),
      main_source_file_transcript_(state.main_source_file_transcript),
      source_files_(state.source_files),
      index_writer_(state.index_writer),
      main_source_file_stdin_alternate_(
          state.main_source_file_stdin_alternate) {
  class ClaimPragmaHandlerWrapper : public clang::PragmaHandler {
   public:
    ClaimPragmaHandlerWrapper(ExtractorPPCallbacks* context)
        : PragmaHandler("kythe_claim"), context_(context) {}
    void HandlePragma(clang::Preprocessor& preprocessor,
                      clang::PragmaIntroducerKind introducer,
                      clang::Token& first_token) override {
      context_->HandleKytheClaimPragma(preprocessor, introducer, first_token);
    }

   private:
    ExtractorPPCallbacks* context_;
  };
  // Clang takes ownership.
  preprocessor_->AddPragmaHandler(new ClaimPragmaHandlerWrapper(this));

  class MetadataPragmaHandlerWrapper : public clang::PragmaHandler {
   public:
    MetadataPragmaHandlerWrapper(ExtractorPPCallbacks* context)
        : PragmaHandler("kythe_metadata"), context_(context) {}
    void HandlePragma(clang::Preprocessor& preprocessor,
                      clang::PragmaIntroducerKind introducer,
                      clang::Token& first_token) override {
      context_->HandleKytheMetadataPragma(preprocessor, introducer,
                                          first_token);
    }

   private:
    ExtractorPPCallbacks* context_;
  };
  // Clang takes ownership.
  preprocessor_->AddPragmaHandler(new MetadataPragmaHandlerWrapper(this));
}

void ExtractorPPCallbacks::FileChanged(
    clang::SourceLocation /*Loc*/, FileChangeReason Reason,
    clang::SrcMgr::CharacteristicKind /*FileType*/, clang::FileID /*PrevFID*/) {
  if (Reason == EnterFile) {
    if (last_inclusion_directive_path_.empty()) {
      current_files_.push(FileState{GetMainFile()->getName(),
                                    ClaimDirective::NoDirectivesFound});
    } else {
      CHECK(!current_files_.empty());
      current_files_.top().last_include_offset = last_inclusion_offset_;
      current_files_.push(FileState{last_inclusion_directive_path_,
                                    ClaimDirective::NoDirectivesFound});
    }
    history()->Update(preprocessor_->getLangOpts());
  } else if (Reason == ExitFile) {
    auto transcript = PopFile();
    if (!current_files_.empty()) {
      history()->Update(transcript);
    }
  }
}

PreprocessorTranscript ExtractorPPCallbacks::PopFile() {
  CHECK(!current_files_.empty());
  PreprocessorTranscript top_transcript =
      current_files_.top().history.CompleteAndReset();
  ClaimDirective top_directive = current_files_.top().default_behavior;
  auto file_data = source_files_->find(current_files_.top().file_path);
  if (file_data == source_files_->end()) {
    // We pop the main source file before doing anything interesting.
    return top_transcript;
  }
  auto old_record = file_data->second.include_history.insert(std::make_pair(
      top_transcript, SourceFile::FileHandlingAnnotations{
                          top_directive, current_files_.top().transcripts}));
  if (!old_record.second) {
    if (old_record.first->second.out_edges !=
        current_files_.top().transcripts) {
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

std::string ExtractorPPCallbacks::FixStdinPath(const clang::FileEntry* file,
                                               const std::string& in_path) {
  if (in_path == "-" || in_path == "<stdin>") {
    if (main_source_file_stdin_alternate_->empty()) {
      const llvm::MemoryBuffer* buffer =
          source_manager_->getMemoryBufferForFile(file);
      std::string hashed_name =
          Sha256(buffer->getBufferStart(),
                 buffer->getBufferEnd() - buffer->getBufferStart());
      *main_source_file_stdin_alternate_ = "<stdin:" + hashed_name + ">";
    }
    return *main_source_file_stdin_alternate_;
  }
  return in_path;
}

void ExtractorPPCallbacks::AddFile(const clang::FileEntry* file,
                                   const std::string& in_path) {
  std::string path = FixStdinPath(file, in_path);
  auto contents =
      source_files_->insert(std::make_pair(in_path, SourceFile{std::string()}));
  if (contents.second) {
    const llvm::MemoryBuffer* buffer =
        source_manager_->getMemoryBufferForFile(file);
    contents.first->second.file_content.assign(buffer->getBufferStart(),
                                               buffer->getBufferEnd());
    contents.first->second.vname.CopyFrom(index_writer_->VNameForPath(
        RelativizePath(path, index_writer_->root_directory())));
    LOG(INFO) << "added content for " << path << ": mapped to "
              << contents.first->second.vname.DebugString() << "\n";
  }
}

void ExtractorPPCallbacks::RecordMacroExpansion(
    clang::SourceLocation expansion_loc, llvm::StringRef unexpanded,
    llvm::StringRef expanded) {
  RecordSpecificLocation(expansion_loc);
  history()->Update(unexpanded);
  history()->Update(expanded);
}

void ExtractorPPCallbacks::MacroExpands(
    const clang::Token& macro_name,
    const clang::MacroDefinition& macro_definition, clang::SourceRange range,
    const clang::MacroArgs* macro_args) {
  // We do care about inner macro expansions: the indexer will
  // emit transitive macro expansion edges, and if we don't distinguish
  // expansion paths, we will leave edges out of the graph.
  const auto* macro_info = macro_definition.getMacroInfo();
  if (macro_info) {
    clang::SourceLocation def_loc = macro_info->getDefinitionLoc();
    RecordSpecificLocation(def_loc);
  }
  if (!range.getBegin().isFileID()) {
    auto begin = source_manager_->getExpansionLoc(range.getBegin());
    if (begin.isFileID()) {
      RecordSpecificLocation(begin);
    }
  }
  if (macro_name.getLocation().isFileID()) {
    llvm::StringRef macro_name_string =
        macro_name.getIdentifierInfo()->getName().str();
    RecordMacroExpansion(
        macro_name.getLocation(),
        getMacroUnexpandedString(range, *preprocessor_, macro_name_string,
                                 macro_info),
        getMacroExpandedString(*preprocessor_, macro_name_string, macro_info,
                               macro_args));
  }
}

void ExtractorPPCallbacks::Defined(
    const clang::Token& macro_name,
    const clang::MacroDefinition& macro_definition, clang::SourceRange range) {
  if (macro_definition && macro_definition.getMacroInfo()) {
    RecordSpecificLocation(macro_definition.getMacroInfo()->getDefinitionLoc());
  }
  clang::SourceLocation macro_location = macro_name.getLocation();
  RecordMacroExpansion(macro_location, getSourceString(*preprocessor_, range),
                       macro_definition ? "1" : "0");
}

void ExtractorPPCallbacks::RecordSpecificLocation(clang::SourceLocation loc) {
  if (loc.isValid() && loc.isFileID() &&
      source_manager_->getFileID(loc) != preprocessor_->getPredefinesFileID()) {
    history()->Update(source_manager_->getFileOffset(loc));
    const auto filename_ref = source_manager_->getFilename(loc);
    const auto* file_ref =
        source_manager_->getFileEntryForID(source_manager_->getFileID(loc));
    if (file_ref) {
      auto vname = index_writer_->VNameForPath(
          RelativizePath(FixStdinPath(file_ref, filename_ref),
                         index_writer_->root_directory()));
      history()->Update(ToStringRef(vname.signature()));
      history()->Update(ToStringRef(vname.corpus()));
      history()->Update(ToStringRef(vname.root()));
      history()->Update(ToStringRef(vname.path()));
      history()->Update(ToStringRef(vname.language()));
    } else {
      LOG(WARNING) << "No FileRef for " << filename_ref.str() << " (location "
                   << loc.printToString(*source_manager_) << ")";
    }
  }
}

void ExtractorPPCallbacks::MacroDefined(
    const clang::Token& macro_name,
    const clang::MacroDirective* macro_directive) {
  clang::SourceLocation macro_location = macro_name.getLocation();
  if (!macro_location.isFileID()) {
    return;
  }
  llvm::StringRef macro_name_string =
      macro_name.getIdentifierInfo()->getName().str();
  history()->Update(source_manager_->getFileOffset(macro_location));
  history()->Update(macro_name_string);
}

void ExtractorPPCallbacks::MacroUndefined(
    const clang::Token& macro_name,
    const clang::MacroDefinition& macro_definition) {
  clang::SourceLocation macro_location = macro_name.getLocation();
  if (!macro_location.isFileID()) {
    return;
  }
  llvm::StringRef macro_name_string =
      macro_name.getIdentifierInfo()->getName().str();
  history()->Update(source_manager_->getFileOffset(macro_location));
  if (macro_definition) {
    // We don't just care that a macro was undefined; we care that
    // a *specific* macro definition was undefined.
    RecordSpecificLocation(macro_definition.getLocalDirective()->getLocation());
  }
  history()->Update("#undef");
  history()->Update(macro_name_string);
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

void ExtractorPPCallbacks::Ifdef(
    clang::SourceLocation location, const clang::Token& macro_name,
    const clang::MacroDefinition& macro_definition) {
  RecordCondition(location, "#ifdef",
                  macro_definition
                      ? clang::PPCallbacks::ConditionValueKind::CVK_True
                      : clang::PPCallbacks::ConditionValueKind::CVK_False,
                  macro_name.getIdentifierInfo()->getName().str());
}

void ExtractorPPCallbacks::Ifndef(
    clang::SourceLocation location, const clang::Token& macro_name,
    const clang::MacroDefinition& macro_definition) {
  RecordCondition(location, "#ifndef",
                  macro_definition
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
  last_inclusion_directive_path_ =
      AddFile(File, FileName, SearchPath, RelativePath);
  last_inclusion_offset_ = source_manager_->getFileOffset(HashLoc);
}

std::string ExtractorPPCallbacks::AddFile(const clang::FileEntry* file,
                                          llvm::StringRef file_name,
                                          llvm::StringRef search_path,
                                          llvm::StringRef relative_path) {
  CHECK(!current_files_.top().file_path.empty());
  const auto* search_path_entry =
      source_manager_->getFileManager().getDirectory(search_path);
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
  llvm::SmallString<1024> out_name;
  if (search_path_entry == current_file_parent_entry) {
    auto parent =
        llvm::sys::path::parent_path(current_files_.top().file_path.c_str())
            .str();

    // If the file is a top level file ("file.cc"), we normalize to a path
    // relative to "./".
    if (parent.empty() || parent == "/") {
      parent = ".";
    }

    // Otherwise we take the literal path as we stored it for the current
    // file, and append the relative path.
    out_name = parent;
    llvm::sys::path::append(out_name, relative_path);
  } else if (!search_path.empty()) {
    out_name = search_path;
    llvm::sys::path::append(out_name, relative_path);
  } else {
    CHECK(llvm::sys::path::is_absolute(file_name)) << file_name.str();
    out_name = file_name;
  }
  std::string out_name_string = out_name.str();
  AddFile(file, out_name_string);
  return out_name_string;
}

const clang::FileEntry* ExtractorPPCallbacks::GetMainFile() {
  return source_manager_->getFileEntryForID(source_manager_->getMainFileID());
}

RunningHash* ExtractorPPCallbacks::history() {
  CHECK(!current_files_.empty());
  return &current_files_.top().history;
}

void ExtractorPPCallbacks::HandleKytheClaimPragma(
    clang::Preprocessor& preprocessor, clang::PragmaIntroducerKind introducer,
    clang::Token& first_token) {
  CHECK(!current_files_.empty());
  current_files_.top().default_behavior = ClaimDirective::AlwaysClaim;
}

void ExtractorPPCallbacks::HandleKytheMetadataPragma(
    clang::Preprocessor& preprocessor, clang::PragmaIntroducerKind introducer,
    clang::Token& first_token) {
  CHECK(!current_files_.empty());
  llvm::SmallString<1024> search_path;
  llvm::SmallString<1024> relative_path;
  llvm::SmallString<1024> filename;
  if (const clang::FileEntry* file = LookupFileForIncludePragma(
          &preprocessor, &search_path, &relative_path, &filename)) {
    AddFile(file, filename, search_path, relative_path);
  }
}

class ExtractorAction : public clang::PreprocessorFrontendAction {
 public:
  explicit ExtractorAction(IndexWriter* index_writer,
                           ExtractorCallback callback)
      : callback_(callback), index_writer_(index_writer) {}

  void ExecuteAction() override {
    const auto inputs = getCompilerInstance().getFrontendOpts().Inputs;
    CHECK_EQ(1, inputs.size()) << "Expected to see only one TU; instead saw "
                               << inputs.size() << ".";
    main_source_file_ = inputs[0].getFile();
    auto* preprocessor = &getCompilerInstance().getPreprocessor();
    preprocessor->addPPCallbacks(
        llvm::make_unique<ExtractorPPCallbacks>(ExtractorState{
            index_writer_, &getCompilerInstance().getSourceManager(),
            preprocessor, &main_source_file_, &main_source_file_transcript_,
            &source_files_, &main_source_file_stdin_alternate_}));
    preprocessor->EnterMainSourceFile();
    clang::Token token;
    do {
      preprocessor->Lex(token);
    } while (token.isNot(clang::tok::eof));
  }

  void EndSourceFileAction() override {
    main_source_file_ = main_source_file_stdin_alternate_.empty()
                            ? main_source_file_
                            : main_source_file_stdin_alternate_;
    // Include information about the header search state in the CU.
    const auto& header_search_options =
        getCompilerInstance().getHeaderSearchOpts();
    const auto& header_search_info =
        getCompilerInstance().getPreprocessor().getHeaderSearchInfo();
    HeaderSearchInfo info;
    bool info_valid = info.CopyFrom(header_search_options, header_search_info);
    RecordModuleInfo(&header_search_info.getModuleMap());
    callback_(main_source_file_, main_source_file_transcript_, source_files_,
              info_valid ? &info : nullptr,
              getCompilerInstance().getDiagnostics().hasErrorOccurred());
  }

 private:
  void RecordModuleInfo(const clang::ModuleMap* module_map) {
    // TODO(zarko): Record module flags (::DisableModuleHash, ::ModuleMaps)
    // from HeaderSearchOptions; support "apple-style headermaps" (see
    // Clang's InitHeaderSearch.cpp.)
    auto* source_manager = &getCompilerInstance().getSourceManager();
    for (auto modules = module_map->module_begin(),
              modules_end = module_map->module_end();
         modules != modules_end; ++modules) {
      auto* module = modules->second;
      if (module->DefinitionLoc.isInvalid() ||
          !module->DefinitionLoc.isFileID()) {
        LOG(WARNING) << "Module " << module->Name
                     << " has an invalid or non-file definition location.";
        continue;
      }
      auto file_id = source_manager->getFileID(module->DefinitionLoc);
      if (file_id.isInvalid()) {
        LOG(WARNING) << "Module " << module->Name << " has an invalid file ID.";
        continue;
      }
      if (const auto* file = source_manager->getFileEntryForID(file_id)) {
        auto contents = source_files_.insert(
            std::make_pair(file->getName(), SourceFile{std::string()}));
        if (contents.second) {
          const llvm::MemoryBuffer* buffer =
              source_manager->getMemoryBufferForFile(file);
          contents.first->second.file_content.assign(buffer->getBufferStart(),
                                                     buffer->getBufferEnd());
          contents.first->second.vname.CopyFrom(
              index_writer_->VNameForPath(RelativizePath(
                  file->getName(), index_writer_->root_directory())));
          LOG(INFO) << "added module map content for " << file->getName();
        }
      } else {
        LOG(WARNING) << "Module " << module->Name << " is missing a FileEntry.";
      }
    }
  }

  ExtractorCallback callback_;
  /// The main source file for the compilation (assuming only one).
  std::string main_source_file_;
  /// The transcript of the main source file.
  std::string main_source_file_transcript_;
  /// Contents of the files we've used, indexed by normalized path.
  std::unordered_map<std::string, SourceFile> source_files_;
  /// The active IndexWriter.
  IndexWriter* index_writer_;
  /// Nonempty if the main source file was stdin ("-") and we have chosen
  /// an alternate name for it.
  std::string main_source_file_stdin_alternate_;
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
  std::string file_path =
      force_path_.empty() ? directory + "/" + hash + ".kindex" : force_path_;
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
  if (out.corpus().empty()) {
    out.set_corpus(corpus_);
  }
  return out;
}

void IndexWriter::FillFileInput(
    const std::string& clang_path, const SourceFile& source_file,
    kythe::proto::CompilationUnit_FileInput* file_input) {
  CHECK(source_file.vname.language().empty());
  file_input->mutable_v_name()->CopyFrom(source_file.vname);
  // This path is distinct from the VName path. It is used by analysis tools
  // to configure Clang's virtual filesystem.
  auto* file_info = file_input->mutable_info();
  // We need to use something other than "-", since clang special-cases
  // it. (clang also refers to standard input as <stdin>, so we're
  // consistent there.)
  file_info->set_path(clang_path == "-" ? "<stdin>" : clang_path);
  file_info->set_digest(Sha256(source_file.file_content.c_str(),
                               source_file.file_content.size()));
  for (const auto& row : source_file.include_history) {
    auto* row_pb = file_input->add_context();
    row_pb->set_source_context(row.first);
    if (row.second.default_claim == ClaimDirective::AlwaysClaim) {
      row_pb->set_always_process(true);
    }
    for (const auto& col : row.second.out_edges) {
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
    const HeaderSearchInfo* header_search_info, bool had_errors,
    const std::string& clang_working_dir) {
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
  unit_vname->set_language("c++");
  unit_vname->set_signature("cu#" + identifying_blob_digest);
  unit_vname->clear_path();

  if (header_search_info != nullptr) {
    kythe::proto::CxxCompilationUnitDetails cxx_details;
    header_search_info->CopyTo(&cxx_details);
    PackAny(cxx_details, kCxxCompilationUnitDetailsURI, unit.add_details());
  }

  for (const auto& file : source_files) {
    FillFileInput(file.first, file.second, unit.add_required_input());
  }
  unit.set_entry_context(entry_context);
  unit.set_has_compile_errors(had_errors);
  unit.add_source_file(main_source_file);
  llvm::SmallString<256> absolute_working_directory(
      llvm::StringRef(clang_working_dir.data(), clang_working_dir.size()));
  std::error_code err =
      llvm::sys::fs::make_absolute(absolute_working_directory);
  if (err) {
    LOG(WARNING) << "Can't get working directory: " << err.message();
  } else {
    unit.set_working_directory(absolute_working_directory.c_str());
  }
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
    IndexWriter* index_writer, ExtractorCallback callback) {
  return std::unique_ptr<clang::FrontendAction>(
      new ExtractorAction(index_writer, callback));
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

/// \brief Loads all data from a file or terminates the process.
static std::string LoadFileOrDie(const std::string& file) {
  FILE* handle = fopen(file.c_str(), "rb");
  CHECK(handle != nullptr) << "Couldn't open input file " << file;
  CHECK_EQ(fseek(handle, 0, SEEK_END), 0) << "Couldn't seek " << file;
  long size = ftell(handle);
  CHECK_GE(size, 0) << "Bad size for " << file;
  CHECK_EQ(fseek(handle, 0, SEEK_SET), 0) << "Couldn't seek " << file;
  std::string content;
  content.resize(size);
  CHECK_EQ(fread(&content[0], size, 1, handle), 1) << "Couldn't read " << file;
  CHECK_NE(fclose(handle), EOF) << "Couldn't close " << file;
  return content;
}

/// When a -resource-dir is not specified, map builtin versions of compiler
/// headers to this directory.
constexpr char kBuiltinResourceDirectory[] = "/kythe_builtins";

void ExtractorConfiguration::SetVNameConfig(const std::string& path) {
  if (!index_writer_.SetVNameConfiguration(LoadFileOrDie(path))) {
    fprintf(stderr, "Couldn't configure vnames from %s\n", path.c_str());
    exit(1);
  }
}

void ExtractorConfiguration::SetArgs(const std::vector<std::string>& args) {
  final_args_ = args;
  std::string executable = final_args_.size() ? final_args_[0] : "";
  if (final_args_.size() >= 3 && final_args_[1] == "--with_executable") {
    executable = final_args_[2];
    final_args_.erase(final_args_.begin() + 1, final_args_.begin() + 3);
  }
  // TODO(zarko): Does this really need to be InitializeAllTargets()?
  // We may have made the precondition too strict.
  llvm::InitializeAllTargetInfos();
  clang::tooling::addTargetAndModeForProgramName(final_args_, executable);
  final_args_ = common::GCCArgsToClangSyntaxOnlyArgs(final_args_);
  // Check to see if an alternate resource-dir was specified; otherwise,
  // invent one. We need this to find stddef.h and friends.
  for (const auto& arg : final_args_) {
    // Handle both -resource-dir=foo and -resource-dir foo.
    if (llvm::StringRef(arg).startswith("-resource-dir")) {
      map_builtin_resources_ = false;
      break;
    }
  }
  if (map_builtin_resources_) {
    final_args_.insert(final_args_.begin() + 1, kBuiltinResourceDirectory);
    final_args_.insert(final_args_.begin() + 1, "-resource-dir");
  }
  final_args_.insert(final_args_.begin() + 1, "-DKYTHE_IS_RUNNING=1");
  // Store the arguments post-filtering.
  index_writer_.set_args(final_args_);
}

void ExtractorConfiguration::InitializeFromEnvironment() {
  if (const char* env_corpus = getenv("KYTHE_CORPUS")) {
    index_writer_.set_corpus(env_corpus);
  }
  if (const char* vname_file = getenv("KYTHE_VNAMES")) {
    SetVNameConfig(vname_file);
  }
  if (const char* env_root_directory = getenv("KYTHE_ROOT_DIRECTORY")) {
    index_writer_.set_root_directory(env_root_directory);
  }
  if (const char* env_index_pack = getenv("KYTHE_INDEX_PACK")) {
    using_index_packs_ = (strlen(env_index_pack) != 0);
  }
  if (const char* env_output_directory = getenv("KYTHE_OUTPUT_DIRECTORY")) {
    index_writer_.set_output_directory(env_output_directory);
  }
  if (const char* env_output_file = getenv("KYTHE_OUTPUT_FILE")) {
    SetKindexOutputFile(env_output_file);
  }
}

void ExtractorConfiguration::Extract() {
  llvm::IntrusiveRefCntPtr<clang::FileManager> file_manager(
      new clang::FileManager(file_system_options_));
  auto extractor = NewExtractor(
      &index_writer_,
      [this](const std::string& main_source_file,
             const PreprocessorTranscript& transcript,
             const std::unordered_map<std::string, SourceFile>& source_files,
             const HeaderSearchInfo* header_search_info, bool had_errors) {
        std::unique_ptr<IndexWriterSink> sink;
        if (using_index_packs_) {
          sink.reset(new IndexPackWriterSink());
        } else {
          sink.reset(new KindexWriterSink(kindex_path_));
        }
        index_writer_.WriteIndex(std::move(sink), main_source_file, transcript,
                                 source_files, header_search_info, had_errors,
                                 file_system_options_.WorkingDir);
      });
  clang::tooling::ToolInvocation invocation(final_args_, extractor.release(),
                                            file_manager.get());
  if (map_builtin_resources_) {
    MapCompilerResources(&invocation, kBuiltinResourceDirectory);
  }
  invocation.run();
}

}  // namespace kythe
