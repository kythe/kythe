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

// A simple unit test for KytheIndexer.

#include "IndexerFrontendAction.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <memory>
#include <set>
#include <string>
#include <utility>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"

#include "google/protobuf/stubs/common.h"
#include "gtest/gtest.h"

#include "KytheGraphRecorder.h"
#include "RecordingOutputStream.h"

namespace kythe {
namespace {

using clang::SourceLocation;
using llvm::StringRef;

TEST(KytheIndexerUnitTest, GraphRecorderNodeKind) {
  RecordingOutputStream stream;
  KytheGraphRecorder recorder(&stream);
  kythe::proto::VName vname;
  vname.set_signature("sig1");
  vname.set_corpus("corpus1");
  vname.set_language("lang1");
  vname.set_path("path1");
  vname.set_root("root1");
  recorder.BeginNode(vname, NodeKindID::kFile);
  recorder.EndNode();
  ASSERT_EQ(1, stream.entries().size());
  const auto& entry = stream.entries()[0];
  ASSERT_EQ("file", entry.fact_value());
  ASSERT_EQ("/kythe/node/kind", entry.fact_name());
  ASSERT_TRUE(entry.edge_kind().empty());
  ASSERT_FALSE(entry.has_target());
  ASSERT_TRUE(entry.has_source());
  ASSERT_EQ(vname.DebugString(), entry.source().DebugString());
}

TEST(KytheIndexerUnitTest, GraphRecorderNodeProperty) {
  RecordingOutputStream stream;
  KytheGraphRecorder recorder(&stream);
  kythe::proto::VName vname;
  vname.set_signature("sig1");
  vname.set_corpus("corpus1");
  vname.set_language("lang1");
  vname.set_path("path1");
  vname.set_root("root1");
  recorder.BeginNode(vname, NodeKindID::kAnchor);
  recorder.AddProperty(PropertyID::kLocationUri, "test://file");
  recorder.EndNode();
  ASSERT_EQ(2, stream.entries().size());
  bool found_kind_fact = false, found_property_fact = false;
  for (const auto& entry : stream.entries()) {
    if (entry.fact_name() == "/kythe/loc/uri") {
      ASSERT_EQ("test://file", entry.fact_value());
      ASSERT_FALSE(found_property_fact);
      found_property_fact = true;
    } else if (entry.fact_name() == "/kythe/node/kind") {
      ASSERT_EQ("anchor", entry.fact_value());
      ASSERT_FALSE(found_kind_fact);
      found_kind_fact = true;
    }
    ASSERT_TRUE(entry.edge_kind().empty());
    ASSERT_FALSE(entry.has_target());
    ASSERT_TRUE(entry.has_source());
    ASSERT_EQ(vname.DebugString(), entry.source().DebugString());
  }
  ASSERT_TRUE(found_kind_fact);
  ASSERT_TRUE(found_property_fact);
}

TEST(KytheIndexerUnitTest, GraphRecorderEdge) {
  RecordingOutputStream stream;
  KytheGraphRecorder recorder(&stream);
  kythe::proto::VName vname_source;
  vname_source.set_signature("sig1");
  vname_source.set_corpus("corpus1");
  vname_source.set_language("lang1");
  vname_source.set_path("path1");
  vname_source.set_root("root1");
  kythe::proto::VName vname_target;
  vname_target.set_signature("sig2");
  vname_target.set_corpus("corpus2");
  vname_target.set_language("lang2");
  vname_target.set_path("path2");
  vname_target.set_root("root2");
  recorder.AddEdge(vname_source, EdgeKindID::kDefines, vname_target);
  ASSERT_EQ(1, stream.entries().size());
  const auto& entry = stream.entries()[0];
  EXPECT_TRUE(entry.fact_value().empty());
  EXPECT_EQ("/", entry.fact_name());
  ASSERT_EQ("/kythe/edge/defines", entry.edge_kind());
  ASSERT_TRUE(entry.has_target());
  ASSERT_TRUE(entry.has_source());
  ASSERT_EQ(vname_source.DebugString(), entry.source().DebugString());
  ASSERT_EQ(vname_target.DebugString(), entry.target().DebugString());
}

TEST(KytheIndexerUnitTest, GraphRecorderEdgeOrdinal) {
  RecordingOutputStream stream;
  KytheGraphRecorder recorder(&stream);
  kythe::proto::VName vname_source, vname_target;
  vname_source.set_signature("sig1");
  vname_source.set_corpus("corpus1");
  vname_source.set_language("lang1");
  vname_source.set_path("path1");
  vname_source.set_root("root1");
  vname_target.set_signature("sig2");
  vname_target.set_corpus("corpus2");
  vname_target.set_language("lang2");
  vname_target.set_path("path2");
  vname_target.set_root("root2");
  recorder.AddEdge(vname_source, EdgeKindID::kDefines, vname_target, 42);
  ASSERT_EQ(1, stream.entries().size());
  const auto& entry = stream.entries()[0];
  ASSERT_EQ("42", entry.fact_value());
  ASSERT_EQ("/kythe/ordinal", entry.fact_name());
  ASSERT_EQ("/kythe/edge/defines", entry.edge_kind());
  ASSERT_TRUE(entry.has_target());
  ASSERT_TRUE(entry.has_source());
  ASSERT_EQ(vname_source.DebugString(), entry.source().DebugString());
  ASSERT_EQ(vname_target.DebugString(), entry.target().DebugString());
}

TEST(KytheIndexerUnitTest, TrivialHappyCase) {
  NullGraphObserver observer;
  HeaderSearchInfo info;
  info.is_valid = false;
  std::unique_ptr<clang::FrontendAction> Action(
      new IndexerFrontendAction(&observer, info));
  ASSERT_TRUE(
      RunToolOnCode(std::move(Action), "int main() {}", "valid_main.cc"));
}

/// \brief A `GraphObserver` that checks the sematics of `pushFile` and
/// `popFile`.
///
/// This class checks whether `pushFile` and `popFile` ever cause a stack
/// underrun. It also provides access to a vector of all the filenames that
/// have ever been pushed to the stack in a way that persists after running the
/// top-level FrontendAction.
class PushPopLintingGraphObserver : public NullGraphObserver {
 public:
  void pushFile(clang::SourceLocation BlameLocation,
                clang::SourceLocation Location) override {
    if (!Location.isFileID()) {
      FileNames.push("not-file-id");
      return;
    }
    clang::FileID File = SourceManager->getFileID(Location);
    if (File.isInvalid()) {
      FileNames.push("invalid-file");
    }
    if (const clang::FileEntry* file_entry =
            SourceManager->getFileEntryForID(File)) {
      FileNames.push(file_entry->getName());
    } else {
      FileNames.push("null-file");
    }
    AllPushedFiles.push_back(FileNames.top());
  }

  void popFile() override {
    if (FileNames.empty()) {
      HadUnderrun = true;
    } else {
      FileNames.pop();
    }
  }

  bool hadUnderrun() const { return HadUnderrun; }

  size_t getFileNameStackSize() const { return FileNames.size(); }

  const std::vector<std::string>& getAllPushedFiles() { return AllPushedFiles; }

 private:
  std::stack<std::string> FileNames;
  std::vector<std::string> AllPushedFiles;
  bool HadUnderrun = false;
};

TEST(KytheIndexerUnitTest, PushFilePopFileTracking) {
  PushPopLintingGraphObserver Observer;
  HeaderSearchInfo info;
  info.is_valid = false;
  std::unique_ptr<clang::FrontendAction> Action(
      new IndexerFrontendAction(&Observer, info));
  ASSERT_TRUE(RunToolOnCode(std::move(Action), "int i;", "main.cc"));
  ASSERT_FALSE(Observer.hadUnderrun());
  ASSERT_EQ(0, Observer.getFileNameStackSize());
  ASSERT_LE(1, Observer.getAllPushedFiles().size());
  ASSERT_EQ("main.cc", Observer.getAllPushedFiles()[0]);
}

}  // anonymous namespace
}  // namespace kythe

int main(int argc, char** argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  ::testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();
  google::protobuf::ShutdownProtobufLibrary();
  return result;
}
