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

// A simple unit test for KytheIndexer.

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <utility>

#include "IndexerFrontendAction.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "kythe/cxx/common/indexing/KytheGraphRecorder.h"
#include "kythe/cxx/common/indexing/RecordingOutputStream.h"
#include "kythe/cxx/indexer/cxx/IndexerASTHooks.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "protobuf-matchers/protocol-buffer-matchers.h"

namespace kythe {
namespace {
using ::protobuf_matchers::EqualsProto;

using clang::SourceLocation;

class AnchorMarkTest : public ::testing::Test {
 protected:
  class EmptyToken : public GraphObserver::ClaimToken {
   public:
    std::string StampIdentity(const std::string&) const override { return ""; }
    uintptr_t GetClass() const override { return 0; }
    bool operator==(const ClaimToken&) const override { return false; }
    bool operator!=(const ClaimToken&) const override { return true; }
  };
  MiniAnchor MakeMini(size_t begin, size_t end, const std::string& id) {
    return MiniAnchor{
        begin, end,
        GraphObserver::NodeId::CreateUncompressed(&empty_token_, id)};
  }
  EmptyToken empty_token_;
};

TEST_F(AnchorMarkTest, Empty) {
  std::vector<MiniAnchor> empty;
  std::string empty_text = "";
  InsertAnchorMarks(empty_text, empty);
  EXPECT_TRUE(empty.empty());
  EXPECT_TRUE(empty_text.empty());
}

TEST_F(AnchorMarkTest, Escape) {
  std::vector<MiniAnchor> empty;
  std::string escape_text = R"(\][)";
  InsertAnchorMarks(escape_text, empty);
  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(R"(\\\]\[)", escape_text);
}

TEST_F(AnchorMarkTest, Bad) {
  std::vector<MiniAnchor> bad = {MakeMini(3, 0, "aaa"), MakeMini(1, 1, "bbb")};
  std::string bad_text = "ccc";
  InsertAnchorMarks(bad_text, bad);
  EXPECT_TRUE(bad.empty());
  EXPECT_EQ("ccc", bad_text);
}

TEST_F(AnchorMarkTest, FullAnchor) {
  std::vector<MiniAnchor> full = {MakeMini(0, 3, "bar")};
  std::string full_text = "foo";
  InsertAnchorMarks(full_text, full);
  EXPECT_EQ("[foo]", full_text);
  ASSERT_EQ(1, full.size());
  EXPECT_EQ("bar", full[0].AnchoredTo.ToString());
}

TEST_F(AnchorMarkTest, RightPadAnchor) {
  std::vector<MiniAnchor> offset = {MakeMini(0, 3, "bar")};
  std::string offset_text = "foo ";
  InsertAnchorMarks(offset_text, offset);
  EXPECT_EQ("[foo] ", offset_text);
  ASSERT_EQ(1, offset.size());
  EXPECT_EQ("bar", offset[0].AnchoredTo.ToString());
}

TEST_F(AnchorMarkTest, LeftPadAnchor) {
  std::vector<MiniAnchor> offset = {MakeMini(1, 4, "bar")};
  std::string offset_text = " foo";
  InsertAnchorMarks(offset_text, offset);
  EXPECT_EQ(" [foo]", offset_text);
  ASSERT_EQ(1, offset.size());
  EXPECT_EQ("bar", offset[0].AnchoredTo.ToString());
}

TEST_F(AnchorMarkTest, MiddleNest) {
  std::vector<MiniAnchor> nest = {MakeMini(1, 2, "bbb"), MakeMini(0, 3, "aaa")};
  std::string nest_text = "aba";
  InsertAnchorMarks(nest_text, nest);
  EXPECT_EQ("[a[b]a]", nest_text);
  ASSERT_EQ(2, nest.size());
  EXPECT_EQ("aaa", nest[0].AnchoredTo.ToString());
  EXPECT_EQ("bbb", nest[1].AnchoredTo.ToString());
}

TEST_F(AnchorMarkTest, InterposedNest) {
  std::vector<MiniAnchor> nest = {MakeMini(1, 2, "bbb"), MakeMini(0, 5, "aaa"),
                                  MakeMini(3, 4, "ccc")};
  std::string nest_text = "ababa";
  InsertAnchorMarks(nest_text, nest);
  EXPECT_EQ("[a[b]a[b]a]", nest_text);
  ASSERT_EQ(3, nest.size());
  EXPECT_EQ("aaa", nest[0].AnchoredTo.ToString());
  EXPECT_EQ("bbb", nest[1].AnchoredTo.ToString());
  EXPECT_EQ("ccc", nest[2].AnchoredTo.ToString());
}

TEST_F(AnchorMarkTest, LeftOverlappingNest) {
  std::vector<MiniAnchor> nest = {MakeMini(0, 2, "bbb"), MakeMini(0, 3, "aaa")};
  std::string nest_text = "aba";
  InsertAnchorMarks(nest_text, nest);
  EXPECT_EQ("[[ab]a]", nest_text);
  ASSERT_EQ(2, nest.size());
  EXPECT_EQ("aaa", nest[0].AnchoredTo.ToString());
  EXPECT_EQ("bbb", nest[1].AnchoredTo.ToString());
}

TEST_F(AnchorMarkTest, Juxtaposed) {
  std::vector<MiniAnchor> nest = {MakeMini(1, 2, "bbb"), MakeMini(0, 1, "aaa")};
  std::string nest_text = "ab";
  InsertAnchorMarks(nest_text, nest);
  EXPECT_EQ("[a][b]", nest_text);
  ASSERT_EQ(2, nest.size());
  EXPECT_EQ("aaa", nest[0].AnchoredTo.ToString());
  EXPECT_EQ("bbb", nest[1].AnchoredTo.ToString());
}

TEST_F(AnchorMarkTest, OverlappingMiddleNest) {
  std::vector<MiniAnchor> nest = {MakeMini(1, 2, "bbb"), MakeMini(0, 3, "aaa"),
                                  MakeMini(1, 2, "ccc")};
  std::string nest_text = "aba";
  InsertAnchorMarks(nest_text, nest);
  EXPECT_EQ("[a[[b]]a]", nest_text);
  ASSERT_EQ(3, nest.size());
  EXPECT_EQ("aaa", nest[0].AnchoredTo.ToString());
  auto second = nest[1].AnchoredTo.ToString();
  auto third = nest[2].AnchoredTo.ToString();
  EXPECT_TRUE((second == "bbb" && third == "ccc") ||
              (second == "ccc" && third == "bbb"));
}

TEST(KytheIndexerUnitTest, GraphRecorderNodeKind) {
  RecordingOutputStream stream;
  KytheGraphRecorder recorder(&stream);
  kythe::proto::VName vname;
  vname.set_signature("sig1");
  vname.set_corpus("corpus1");
  vname.set_language("lang1");
  vname.set_path("path1");
  vname.set_root("root1");
  recorder.AddProperty(VNameRef(vname), NodeKindID::kFile);
  ASSERT_EQ(1, stream.entries().size());
  const auto& entry = stream.entries()[0];
  ASSERT_EQ("file", entry.fact_value());
  ASSERT_EQ("/kythe/node/kind", entry.fact_name());
  ASSERT_TRUE(entry.edge_kind().empty());
  ASSERT_FALSE(entry.has_target());
  ASSERT_TRUE(entry.has_source());
  ASSERT_THAT(vname, EqualsProto(entry.source()));
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
  recorder.AddProperty(VNameRef(vname), NodeKindID::kAnchor);
  recorder.AddProperty(VNameRef(vname), PropertyID::kLocationUri,
                       "test://file");
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
    ASSERT_THAT(vname, EqualsProto(entry.source()));
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
  recorder.AddEdge(VNameRef(vname_source), EdgeKindID::kDefinesBinding,
                   VNameRef(vname_target));
  ASSERT_EQ(1, stream.entries().size());
  const auto& entry = stream.entries()[0];
  EXPECT_TRUE(entry.fact_value().empty());
  EXPECT_EQ("/", entry.fact_name());
  ASSERT_EQ("/kythe/edge/defines/binding", entry.edge_kind());
  ASSERT_TRUE(entry.has_target());
  ASSERT_TRUE(entry.has_source());
  ASSERT_THAT(vname_source, EqualsProto(entry.source()));
  ASSERT_THAT(vname_target, EqualsProto(entry.target()));
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
  recorder.AddEdge(VNameRef(vname_source), EdgeKindID::kDefinesBinding,
                   VNameRef(vname_target), 42);
  ASSERT_EQ(1, stream.entries().size());
  const auto& entry = stream.entries()[0];
  EXPECT_TRUE(entry.fact_value().empty());
  EXPECT_EQ("/", entry.fact_name());
  EXPECT_EQ("/kythe/edge/defines/binding.42", entry.edge_kind());
  ASSERT_TRUE(entry.has_target());
  ASSERT_TRUE(entry.has_source());
  EXPECT_THAT(vname_source, EqualsProto(entry.source()));
  EXPECT_THAT(vname_target, EqualsProto(entry.target()));
}

static void WriteStringToStackAndBuffer(const std::string& value,
                                        kythe::BufferStack* stack,
                                        std::string* buffer) {
  unsigned char* bytes = stack->WriteToTop(value.size());
  memcpy(bytes, value.data(), value.size());
  if (buffer) {
    buffer->append(value);
  }
}

TEST(KytheIndexerUnitTest, BufferStackWrite) {
  kythe::BufferStack stack;
  std::string expected, actual;
  {
    google::protobuf::io::StringOutputStream stream(&actual);
    stack.Push(0);
    WriteStringToStackAndBuffer("hello", &stack, &expected);
    EXPECT_FALSE(stack.empty());
    EXPECT_EQ(5, stack.top_size());
    stack.CopyTopToStream(&stream);
    stack.Pop();
    EXPECT_TRUE(stack.empty());
  }
  ASSERT_EQ(expected, actual);
}

TEST(KytheIndexerUnitTest, BufferStackMergeDown) {
  kythe::BufferStack stack;
  std::string actual;
  {
    google::protobuf::io::StringOutputStream stream(&actual);
    stack.Push(0);
    stack.Push(0);
    WriteStringToStackAndBuffer("1", &stack, nullptr);  // ; 1
    stack.Push(0);
    WriteStringToStackAndBuffer("2", &stack, nullptr);  // ; 1; 2
    stack.Push(0);
    WriteStringToStackAndBuffer("3", &stack, nullptr);   // ; 1; 2; 3
    ASSERT_TRUE(stack.MergeDownIfTooSmall(1024, 2048));  // ; 1; 2+3
    ASSERT_EQ(2, stack.top_size());
    stack.Push(0);
    WriteStringToStackAndBuffer("4", &stack, nullptr);   // ; 1; 2+3; 4
    ASSERT_TRUE(stack.MergeDownIfTooSmall(1024, 2048));  // ; 1; 2+34
    ASSERT_EQ(3, stack.top_size());
    ASSERT_TRUE(stack.MergeDownIfTooSmall(1024, 2048));  // ; 1+234
    ASSERT_EQ(4, stack.top_size());
    ASSERT_TRUE(stack.MergeDownIfTooSmall(1024, 2048));  // +1234
    ASSERT_EQ(4, stack.top_size());
    WriteStringToStackAndBuffer("0", &stack, nullptr);  // 0+1234
    stack.CopyTopToStream(&stream);
    kythe::HashCache::Hash hash_expected = {
        0xc5, 0x65, 0xfe, 0x03, 0xca, 0x9b, 0x62, 0x42, 0xe0, 0x1d, 0xfd,
        0xde, 0xfe, 0x9b, 0xba, 0x3d, 0x98, 0xb2, 0x70, 0xe1, 0x9c, 0xd0,
        0x2f, 0xd8, 0x5c, 0xea, 0xf7, 0x5e, 0x2b, 0x25, 0xbf, 0x12};
    kythe::HashCache::Hash hash_actual;
    stack.HashTop(&hash_actual);
    for (size_t i = 0; i < sizeof(hash_actual); ++i) {
      EXPECT_EQ(hash_expected[i], hash_actual[i]) << "byte " << i;
    }
    stack.Pop();
    EXPECT_TRUE(stack.empty());
  }
  ASSERT_EQ("01234", actual);
}

TEST(KytheIndexerUnitTest, BufferStackMergeFailures) {
  kythe::BufferStack stack;
  std::string actual;
  {
    google::protobuf::io::StringOutputStream stream(&actual);
    ASSERT_FALSE(stack.MergeDownIfTooSmall(0, 2048));  // too few on the stack
    stack.Push(0);
    ASSERT_FALSE(stack.MergeDownIfTooSmall(0, 2048));  // too few on the stack
    stack.Push(0);
    ASSERT_FALSE(stack.MergeDownIfTooSmall(0, 2048));    // top >= min_size
    WriteStringToStackAndBuffer("01", &stack, nullptr);  // ;01
    ASSERT_FALSE(stack.MergeDownIfTooSmall(0, 1));       // would be too big
    ASSERT_TRUE(stack.MergeDownIfTooSmall(1024, 2048));  // +01
    stack.Push(0);
    ASSERT_TRUE(stack.MergeDownIfTooSmall(1024, 2048));  // +01,
    stack.Push(0);
    WriteStringToStackAndBuffer("23", &stack, nullptr);  // +01,;23
    ASSERT_TRUE(stack.MergeDownIfTooSmall(1024, 2048));  // +01,,23
    ASSERT_EQ(4, stack.top_size());
    stack.Push(0);
    WriteStringToStackAndBuffer("45", &stack, nullptr);  // +01,,23;45
    stack.Push(0);
    WriteStringToStackAndBuffer("678", &stack, nullptr);  // +01,,23;45;678
    ASSERT_TRUE(stack.MergeDownIfTooSmall(1024, 2048));   // +01,,23;45+678
    ASSERT_EQ(5, stack.top_size());
    ASSERT_FALSE(stack.MergeDownIfTooSmall(1024, 9));  // would hit max_size
    ASSERT_TRUE(stack.MergeDownIfTooSmall(1024, 10));
    ASSERT_EQ(9, stack.top_size());
    stack.CopyTopToStream(&stream);
    stack.Pop();
    EXPECT_TRUE(stack.empty());
  }
  ASSERT_EQ("012345678", actual);
}

TEST(KytheIndexerUnitTest, TrivialHappyCase) {
  NullGraphObserver observer;
  LibrarySupports no_supports;
  IndexerOptions options{};
  std::unique_ptr<clang::FrontendAction> Action =
      std::make_unique<IndexerFrontendAction>(&observer, nullptr, &no_supports,
                                              options);
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
      FileNames.push(std::string(file_entry->getName()));
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
  LibrarySupports no_supports;
  IndexerOptions options{};
  std::unique_ptr<clang::FrontendAction> Action =
      std::make_unique<IndexerFrontendAction>(&Observer, nullptr, &no_supports,
                                              options);
  ASSERT_TRUE(RunToolOnCode(std::move(Action), "int i;", "main.cc"));
  ASSERT_FALSE(Observer.hadUnderrun());
  ASSERT_EQ(0, Observer.getFileNameStackSize());
  ASSERT_LE(1, Observer.getAllPushedFiles().size());
  ASSERT_EQ("main.cc", Observer.getAllPushedFiles()[0]);
}

}  // anonymous namespace
}  // namespace kythe

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();
  google::protobuf::ShutdownProtobufLibrary();
  return result;
}
