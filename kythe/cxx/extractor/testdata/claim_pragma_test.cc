#include <algorithm>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "absl/memory/memory.h"
#include "absl/log/check.h"
#include "gmock/gmock.h"
#include "google/protobuf/text_format.h"
#include "google/protobuf/util/field_comparator.h"
#include "google/protobuf/util/message_differencer.h"
#include "gtest/gtest.h"
#include "kythe/cxx/extractor/cxx_extractor.h"
#include "kythe/cxx/extractor/language.h"

namespace kythe {
namespace {

using ::google::protobuf::FieldDescriptor;
using ::google::protobuf::Message;
using ::google::protobuf::TextFormat;
using ::google::protobuf::util::MessageDifferencer;
using ::google::protobuf::util::SimpleFieldComparator;
using ::testing::ElementsAre;

constexpr char kExpectedContents[] = R"(
v_name {
  language: "c++"
}
required_input {
  v_name {
    path: "kythe/cxx/extractor/testdata/claim_main.cc"
  }
  info {
    path: "kythe/cxx/extractor/testdata/claim_main.cc"
    digest: "4b19bb44ad66bc14a2b29694604420990d94e2b27bb55d10ce9ad5a93f6a6bde"
  }
  details {
    [type.googleapis.com/kythe.proto.ContextDependentVersion] {
      row {
        source_context: "hash0"
        column {
          offset: 12
          linked_context: "hash1"
        }
        column {
          offset: 33
          linked_context: "hash1"
        }
        always_process: true
      }
    }
  }
}
required_input {
  v_name {
    path: "kythe/cxx/extractor/testdata/claim_b.h"
  }
  info {
    path: "kythe/cxx/extractor/testdata/claim_b.h"
    digest: "a3d03965930673eced0d8ad50753f1933013a27a06b8be57443781275f1f937f"
  }
  details {
    [type.googleapis.com/kythe.proto.ContextDependentVersion] {
      row {
        source_context: "hash1"
        always_process: true
      }
    }
  }
}
required_input {
  v_name {
    path: "kythe/cxx/extractor/testdata/claim_a.h"
  }
  info {
    path: "kythe/cxx/extractor/testdata/claim_a.h"
    digest: "2c339c36aa02459955c6d5be9e73ebe030baf3b74dc1123439af8613844d0b1f"
  }
  details {
    [type.googleapis.com/kythe.proto.ContextDependentVersion] {
      row {
        source_context: "hash1"
      }
    }
  }
}
argument: "./kythe/cxx/extractor/cxx_extractor"
argument: "-target"
argument: "{ignored target}"
argument: "-DKYTHE_IS_RUNNING=1"
argument: "-resource-dir"
argument: "/kythe_builtins"
argument: "--driver-mode=g++"
argument: "-I./kythe/cxx/extractor/testdata"
argument: "./kythe/cxx/extractor/testdata/claim_main.cc"
argument: "-fsyntax-only"
source_file: "kythe/cxx/extractor/testdata/claim_main.cc"
entry_context: "hash0"
)";

// Returns the `FieldDescriptor*` for the nested field found
// by following the provided path components or null, e.g.
//   auto pb = Parse(R"(
//     message {
//       inner {
//         field: 10
//       }
//     })");
//   const FieldDescriptor* message_inner_field =
//       FindFieldByLowercasePath(pb.GetDescriptor(),
//                                {"message", "inner", "field"});
const FieldDescriptor* FindNestedFieldByLowercasePath(
    const google::protobuf::Descriptor* descriptor,
    const std::vector<std::string>& field_names) {
  const FieldDescriptor* field = nullptr;
  for (const auto& name : field_names) {
    if (descriptor == nullptr) return nullptr;
    field = descriptor->FindFieldByLowercaseName(name);
    if (field == nullptr) return nullptr;
    descriptor = field->message_type();
  }
  return field;
}

std::vector<const FieldDescriptor*> CanonicalizedHashFields(
    const kythe::proto::CompilationUnit& unit,
    const kythe::proto::ContextDependentVersion& context) {
  const auto* unit_desc = unit.GetDescriptor();
  const auto* context_desc = context.GetDescriptor();
  return {
      FindNestedFieldByLowercasePath(unit_desc, {"entry_context"}),
      FindNestedFieldByLowercasePath(context_desc, {"row", "source_context"}),
      FindNestedFieldByLowercasePath(context_desc,
                                     {"row", "column", "linked_context"}),
  };
}

class CanonicalHashComparator : public SimpleFieldComparator {
 public:
  bool CanonicalizeHashField(const FieldDescriptor* field) {
    CHECK(field && field->cpp_type() == FieldDescriptor::CPPTYPE_STRING)
        << "Field must be bytes or string.";
    auto result = canonical_fields_.insert(field);
    return result.second;
  }

 private:
  using HashMap = std::unordered_map<std::string, size_t>;

  ComparisonResult Compare(
      const Message& message_1, const Message& message_2,
      const FieldDescriptor* field, int index_1, int index_2,
      const google::protobuf::util::FieldContext* field_context) override {
    // Fall back to the default if this isn't a canonicalized field.
    if (canonical_fields_.find(field) == canonical_fields_.end()) {
      return SimpleCompare(message_1, message_2, field, index_1, index_2,
                           field_context);
    }
    const auto* reflection_1 = message_1.GetReflection();
    const auto* reflection_2 = message_2.GetReflection();
    if (field->is_repeated()) {
      // Allocate scratch strings to store the result if a conversion is
      // needed.
      std::string scratch1;
      std::string scratch2;
      return CompareCanonicalHash(reflection_1->GetRepeatedStringReference(
                                      message_1, field, index_1, &scratch1),
                                  reflection_2->GetRepeatedStringReference(
                                      message_2, field, index_2, &scratch2));
    } else {
      // Allocate scratch strings to store the result if a conversion is
      // needed.
      std::string scratch1;
      std::string scratch2;
      return CompareCanonicalHash(
          reflection_1->GetStringReference(message_1, field, &scratch1),
          reflection_2->GetStringReference(message_2, field, &scratch2));
    }
  }

  ComparisonResult CompareCanonicalHash(const std::string& string_1,
                                        const std::string& string_2) {
    return HashIndex(&left_message_hashes_, string_1) ==
                   HashIndex(&right_message_hashes_, string_2)
               ? SAME
               : DIFFERENT;
  }

  static size_t HashIndex(HashMap* canonical_map, const std::string& hash) {
    size_t index = canonical_map->size();
    // We use an index equivalent to the visitation order of the hashes.
    // This is potentially fragile as we really only care if a protocol buffer
    // is self-consistent, but will suffice for unit tests.
    auto result = canonical_map->insert({hash, index});
    return result.first->second;
  }

  std::unordered_set<const FieldDescriptor*> canonical_fields_;
  HashMap left_message_hashes_;
  HashMap right_message_hashes_;
};

class FakeCompilationWriterSink : public kythe::CompilationWriterSink {
 public:
  explicit FakeCompilationWriterSink(int* call_count)
      : call_count_(call_count) {}

 private:
  void WriteFileContent(const kythe::proto::FileData&) override {}
  void OpenIndex(const std::string&) override {}
  void WriteHeader(const kythe::proto::CompilationUnit& unit) override {
    (*call_count_)++;

    const auto& desc = *unit.GetDescriptor();

    kythe::proto::ContextDependentVersion context;
    CanonicalHashComparator hash_compare;
    for (const auto* field : CanonicalizedHashFields(unit, context)) {
      hash_compare.CanonicalizeHashField(field);
    }
    MessageDifferencer diff;
    diff.set_message_field_comparison(MessageDifferencer::EQUIVALENT);
    diff.set_scope(MessageDifferencer::PARTIAL);
    diff.set_field_comparator(&hash_compare);
    diff.TreatAsSet(desc.FindFieldByLowercaseName("required_input"));
    // We need to ignore the value of the '-target' argument, so
    // do the comparison separately.
    diff.IgnoreField(desc.FindFieldByLowercaseName("argument"));

    kythe::proto::CompilationUnit expected;
    ASSERT_TRUE(TextFormat::ParseFromString(kExpectedContents, &expected));
    std::string diffs;
    diff.ReportDifferencesToString(&diffs);
    EXPECT_TRUE(diff.Compare(expected, unit)) << diffs;

    EXPECT_THAT(
        unit.argument(),
        ElementsAre("/dummy/path/to/g++", "-target", ::testing::_,
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
      "dummy-executable",
      "--with_executable",
      "/dummy/path/to/g++",
      "-I./kythe/cxx/extractor/testdata",
      "./kythe/cxx/extractor/testdata/claim_main.cc",
  });

  int call_count = 0;
  EXPECT_TRUE(extractor.Extract(
      supported_language::Language::kCpp,
      std::make_unique<FakeCompilationWriterSink>(&call_count)));
  EXPECT_EQ(call_count, 1);
}

}  // namespace
}  // namespace kythe
