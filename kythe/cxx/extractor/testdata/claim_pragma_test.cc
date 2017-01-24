#include "kythe/cxx/extractor/cxx_extractor.h"

#include <algorithm>
#include <memory>
#include <unordered_set>
#include <vector>

#include "gmock/gmock.h"
#include "google/protobuf/text_format.h"
#include "google/protobuf/util/message_differencer.h"
#include "gtest/gtest.h"
#include "kythe/cxx/common/language.h"

namespace kythe {
namespace {

using ::google::protobuf::TextFormat;
using ::google::protobuf::util::MessageDifferencer;
using ::testing::ElementsAre;

// TODO(shahms): Compare canonicalized hashes again.
constexpr char kExpectedContents[] = R"(
v_name {
  language: "c++"
}
required_input {
  v_name {
    path: "kythe/cxx/extractor/testdata/claim_main.cc"
  }
  info {
    path: "./kythe/cxx/extractor/testdata/claim_main.cc"
    digest: "4b19bb44ad66bc14a2b29694604420990d94e2b27bb55d10ce9ad5a93f6a6bde"
  }
  context {
    row {
      #source_context: "hash0"
      column {
        offset: 12
        #linked_context: "hash1"
      }
      column {
        offset: 33
        #linked_context: "hash1"
      }
      always_process: true
    }
  }
}
required_input {
  v_name {
    path: "kythe/cxx/extractor/testdata/claim_b.h"
  }
  info {
    path: "./kythe/cxx/extractor/testdata/claim_b.h"
    digest: "a3d03965930673eced0d8ad50753f1933013a27a06b8be57443781275f1f937f"
  }
  context {
    row {
      #source_context: "hash1"
      always_process: true
    }
  }
}
required_input {
  v_name {
    path: "kythe/cxx/extractor/testdata/claim_a.h"
  }
  info {
    path: "./kythe/cxx/extractor/testdata/claim_a.h"
    digest: "2c339c36aa02459955c6d5be9e73ebe030baf3b74dc1123439af8613844d0b1f"
  }
  context {
    row {
      #source_context: "hash1"
    }
  }
}
argument: "-target"
argument: "{ignored target}"
argument: "./kythe/cxx/extractor/cxx_extractor"
argument: "-DKYTHE_IS_RUNNING=1"
argument: "-resource-dir"
argument: "/kythe_builtins"
argument: "--driver-mode=g++"
argument: "-I./kythe/cxx/extractor/testdata"
argument: "./kythe/cxx/extractor/testdata/claim_main.cc"
argument: "-fsyntax-only"
source_file: "./kythe/cxx/extractor/testdata/claim_main.cc"
#entry_context: "hash0"
)";

class FakeIndexWriterSink : public kythe::IndexWriterSink {
 public:
  explicit FakeIndexWriterSink(int* call_count) : call_count_(call_count) {}

 private:
  void WriteFileContent(const kythe::proto::FileData&) override {}
  void OpenIndex(const std::string&, const std::string&) override {}
  void WriteHeader(const kythe::proto::CompilationUnit& unit) override {
    (*call_count_)++;

    const auto& desc = *unit.GetDescriptor();

    MessageDifferencer diff;
    diff.set_message_field_comparison(MessageDifferencer::EQUIVALENT);
    diff.set_scope(MessageDifferencer::PARTIAL);
    diff.TreatAsSet(desc.FindFieldByLowercaseName("required_input"));
    // We need to ignore the value of the '-target' argument, so
    // do the comparison separately.
    diff.IgnoreField(desc.FindFieldByLowercaseName("argument"));

    kythe::proto::CompilationUnit expected;
    ASSERT_TRUE(TextFormat::ParseFromString(kExpectedContents, &expected));
    google::protobuf::string diffs;
    diff.ReportDifferencesToString(&diffs);
    EXPECT_TRUE(diff.Compare(expected, unit)) << diffs;

    EXPECT_THAT(
        unit.argument(),
        ElementsAre("-target", ::testing::_, "dummy-executable",
                    "-DKYTHE_IS_RUNNING=1", "-resource-dir", "/kythe_builtins",
                    "--driver-mode=g++", "-I./kythe/cxx/extractor/testdata",
                    "./kythe/cxx/extractor/testdata/claim_main.cc",
                    "-fsyntax-only"));
  }

  int* call_count_;
};

TEST(ClaimPragmaTest, ClaimPragmaIsSupported) {
  kythe::ExtractorConfiguration extractor;
  extractor.SetArgs({
      "dummy-executable", "--with_executable", "/dummy/path/to/g++",
      "-I./kythe/cxx/extractor/testdata",
      "./kythe/cxx/extractor/testdata/claim_main.cc",
  });

  int call_count = 0;
  std::unique_ptr<kythe::IndexWriterSink> sink(
      new FakeIndexWriterSink(&call_count));
  EXPECT_TRUE(
      extractor.Extract(supported_language::Language::kCpp, std::move(sink)));
  EXPECT_EQ(call_count, 1);
}

}  // namespace
}  // namespace kythe
