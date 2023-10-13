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

#include "file_vname_generator.h"

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
#include "kythe/proto/storage.pb.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "re2/re2.h"

namespace kythe {
namespace {

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

absl::StatusOr<std::string> ParseTemplateMember(const RE2& pattern,
                                                const rapidjson::Value& parent,
                                                absl::string_view name) {
  const auto member =
      parent.FindMember(rapidjson::Value(name.data(), name.size()));
  if (member == parent.MemberEnd()) {
    return "";
  }
  if (!member->value.IsString()) {
    return absl::InvalidArgumentError(
        absl::StrCat("VName template ", name, " is not a string."));
  }
  return ParseTemplate(pattern, member->value.GetString());
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

absl::Status FileVNameGenerator::LoadJsonString(absl::string_view data) {
  using Value = rapidjson::Value;
  rapidjson::Document document;
  document.Parse(data.data(), data.size());
  if (document.HasParseError()) {
    return absl::InvalidArgumentError(
        absl::StrCat(rapidjson::GetParseError_En(document.GetParseError()),
                     " near offset ", document.GetErrorOffset()));
  }
  if (!document.IsArray()) {
    return absl::InvalidArgumentError("Root element in JSON was not an array.");
  }
  for (Value::ConstValueIterator rule = document.Begin();
       rule != document.End(); ++rule) {
    if (!rule->IsObject()) {
      return absl::InvalidArgumentError("Expected an array of objects.");
    }
    const auto regex = rule->FindMember("pattern");
    if (regex == rule->MemberEnd() || !regex->value.IsString()) {
      return absl::InvalidArgumentError("VName rule is missing its pattern.");
    }
    VNameRule next_rule;
    // RE2s don't act like values. Just box them.
    next_rule.pattern = std::make_shared<RE2>(regex->value.GetString());
    if (next_rule.pattern->error_code() != RE2::NoError) {
      return absl::InvalidArgumentError(next_rule.pattern->error());
    }
    const auto vname = rule->FindMember("vname");
    if (vname == rule->MemberEnd() || !vname->value.IsObject()) {
      return absl::InvalidArgumentError("VName rule is missing its template.");
    }

    absl::StatusOr<std::string> root =
        ParseTemplateMember(*next_rule.pattern, vname->value, "root");
    if (!root.ok()) {
      return root.status();
    }
    next_rule.root = *std::move(root);

    absl::StatusOr<std::string> corpus =
        ParseTemplateMember(*next_rule.pattern, vname->value, "corpus");
    if (!corpus.ok()) {
      return corpus.status();
    }
    next_rule.corpus = *std::move(corpus);

    absl::StatusOr<std::string> path =
        ParseTemplateMember(*next_rule.pattern, vname->value, "path");
    if (!path.ok()) {
      return path.status();
    }
    next_rule.path = *std::move(path);

    rules_.push_back(next_rule);
  }
  return absl::OkStatus();
}
}  // namespace kythe
