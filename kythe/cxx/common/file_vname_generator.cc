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

#include "kythe/cxx/common/file_vname_generator.h"

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#include "kythe/cxx/common/json_proto.h"
#include "kythe/proto/storage.pb.h"
#include "kythe/proto/vnames_config.pb.h"
#include "re2/re2.h"

namespace kythe {
namespace {
using ::google::protobuf::io::ArrayInputStream;
using ::google::protobuf::io::ConcatenatingInputStream;
using ::google::protobuf::io::ZeroCopyInputStream;

const LazyRE2 kSubstitutionsPattern = {R"(@\w+@)"};

std::string EscapeBackslashes(absl::string_view value) {
  return absl::StrReplaceAll(value, {{R"(\)", R"(\\)"}});
}

std::optional<absl::string_view> FindMatch(absl::string_view text,
                                           const RE2& pattern) {
  absl::string_view match;
  if (pattern.Match(text, 0, text.size(), RE2::UNANCHORED, &match, 1)) {
    return match;
  }
  return std::nullopt;
}

absl::StatusOr<std::string> ParseTemplate(const RE2& pattern,
                                          absl::string_view input) {
  std::string result;
  while (std::optional<absl::string_view> match =
             FindMatch(input, *kSubstitutionsPattern)) {
    absl::string_view group = match->substr(1, match->size() - 2);

    int index = 0;
    if (!absl::SimpleAtoi(group, &index)) {
      auto iter = pattern.NamedCapturingGroups().find(std::string(group));
      if (iter == pattern.NamedCapturingGroups().end()) {
        return absl::InvalidArgumentError(
            absl::StrCat("Unknown named capture: ", group));
      }
      index = iter->second;
    }
    if (index == 0 || index > pattern.NumberOfCapturingGroups()) {
      return absl::InvalidArgumentError(
          absl::StrCat("Capture index out of range: ", index));
    }
    absl::string_view prefix = input.substr(0, match->begin() - input.begin());
    absl::StrAppend(&result, EscapeBackslashes(prefix), "\\", index);
    input.remove_prefix(prefix.size() + match->size());
  }
  // Include the unmatched tail.
  absl::StrAppend(&result, EscapeBackslashes(input));
  return result;
}

absl::StatusOr<proto::VNamesConfiguration> ParseConfigurationFromRuleArray(
    ZeroCopyInputStream& input) {
  static constexpr absl::string_view kOpen = R"({ "rules": )";
  ArrayInputStream open(kOpen.data(), kOpen.size());

  static constexpr absl::string_view kClose = "}";
  ArrayInputStream close(kClose.data(), kClose.size());

  // The vnames.json format is as an array of Rules, so we need to enclose it
  // in a top-level object and parse it as a field.
  ZeroCopyInputStream* streams[] = {&open, &input, &close};
  ConcatenatingInputStream stream(streams, std::size(streams));

  proto::VNamesConfiguration config;
  if (absl::Status status = ParseFromJsonStream(&stream, &config);
      !status.ok()) {
    return status;
  }
  return config;
}

}  // namespace

kythe::proto::VName FileVNameGenerator::LookupBaseVName(
    absl::string_view path) const {
  for (const auto& rule : rules_) {
    std::vector<absl::string_view> captures(
        1 +
        std::max({RE2::MaxSubmatch(rule.corpus), RE2::MaxSubmatch(rule.root),
                  RE2::MaxSubmatch(rule.path)}));
    if (rule.pattern->Match(path, 0, path.size(), RE2::ANCHOR_BOTH,
                            captures.data(), captures.size())) {
      kythe::proto::VName result;
      if (!rule.corpus.empty()) {
        rule.pattern->Rewrite(result.mutable_corpus(), rule.corpus,
                              captures.data(), captures.size());
      }
      if (!rule.root.empty()) {
        rule.pattern->Rewrite(result.mutable_root(), rule.root, captures.data(),
                              captures.size());
      }
      if (!rule.path.empty()) {
        rule.pattern->Rewrite(result.mutable_path(), rule.path, captures.data(),
                              captures.size());
      }
      return result;
    }
  }
  return default_vname_;
}

kythe::proto::VName FileVNameGenerator::LookupVName(
    absl::string_view path) const {
  kythe::proto::VName vname = LookupBaseVName(path);
  if (vname.path().empty()) {
    vname.set_path(path);
  }
  return vname;
}

bool FileVNameGenerator::LoadJsonString(absl::string_view data,
                                        std::string* error_text) {
  absl::Status status = LoadJsonString(data);
  if (!status.ok() && error_text != nullptr) {
    *error_text = status.ToString();
  }
  return status.ok();
}

absl::Status FileVNameGenerator::LoadJsonStream(ZeroCopyInputStream& input) {
  absl::StatusOr<proto::VNamesConfiguration> config =
      ParseConfigurationFromRuleArray(input);
  if (!config.ok()) {
    return config.status();
  }
  for (const auto& entry : config->rules()) {
    if (entry.pattern().empty()) {
      return absl::InvalidArgumentError("VName rule is missing its pattern.");
    }

    auto pattern = std::make_shared<const RE2>(entry.pattern());
    if (pattern->error_code() != RE2::NoError) {
      return absl::InvalidArgumentError(pattern->error());
    }
    if (!entry.has_vname()) {
      return absl::InvalidArgumentError("VName rule is missing its template.");
    }

    absl::StatusOr<std::string> corpus =
        ParseTemplate(*pattern, entry.vname().corpus());
    if (!corpus.ok()) {
      return corpus.status();
    }
    absl::StatusOr<std::string> root =
        ParseTemplate(*pattern, entry.vname().root());
    if (!root.ok()) {
      return root.status();
    }
    absl::StatusOr<std::string> path =
        ParseTemplate(*pattern, entry.vname().path());
    if (!path.ok()) {
      return path.status();
    }
    rules_.push_back(VNameRule{
        .pattern = std::move(pattern),
        .corpus = *std::move(corpus),
        .root = *std::move(root),
        .path = *std::move(path),
    });
  }
  return absl::OkStatus();
}

absl::Status FileVNameGenerator::LoadJsonString(absl::string_view data) {
  ArrayInputStream stream(data.data(), data.size());
  return LoadJsonStream(stream);
}
}  // namespace kythe
