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

#include "file_vname_generator.h"

#include "glog/logging.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

namespace kythe {

// FullMatchN supports "3..16 args"
static constexpr int kMaxRegexArgs = 16;

FileVNameGenerator::FileVNameGenerator() {
  CHECK_EQ(RE2::NoError, substitution_matcher_.error_code());
}

std::string FileVNameGenerator::ApplyRule(const StringConsRule &rule,
                                          const re2::StringPiece *argv,
                                          int argc) const {
  std::string ret;
  for (const auto &node : rule) {
    switch (node.kind) {
      case StringConsNode::Kind::kEmitText:
        ret.append(node.raw_text);
        break;
      case StringConsNode::Kind::kUseSubstitution:
        // Invariant: 0 <= node.capture_index < argc
        ret.append(argv[node.capture_index].ToString());
        break;
    }
  }
  return ret;
}

bool FileVNameGenerator::ParseRule(const std::string &rule, int max_capture,
                                   StringConsRule *result,
                                   std::string *error_text) {
  CHECK(result != nullptr);
  CHECK(error_text != nullptr);
  re2::StringPiece rule_left(rule);
  while (!rule_left.empty()) {
    re2::StringPiece prefix, substitution;
    if (!RE2::Consume(&rule_left, substitution_matcher_, &prefix,
                      &substitution)) {
      prefix = rule_left;
      rule_left.clear();
    }
    if (!prefix.empty()) {
      StringConsNode node;
      node.kind = StringConsNode::Kind::kEmitText;
      node.raw_text = prefix.ToString();
      result->push_back(node);
    }
    if (!substitution.empty()) {
      StringConsNode node;
      node.kind = StringConsNode::Kind::kUseSubstitution;
      node.capture_index = std::stoi(substitution.ToString());
      if (node.capture_index == 0 || node.capture_index > max_capture) {
        *error_text =
            "Capture index out of range: " + std::to_string(node.capture_index);
        return false;
      }
      // Make these zero-based.
      --node.capture_index;
      result->push_back(node);
    }
  }
  return true;
}

kythe::proto::VName FileVNameGenerator::LookupBaseVName(
    const std::string &path) const {
  re2::StringPiece argv[kMaxRegexArgs];
  RE2::Arg args[kMaxRegexArgs];
  RE2::Arg *arg_pointers[kMaxRegexArgs];
  for (size_t n = 0; n < kMaxRegexArgs; ++n) {
    args[n] = &argv[n];
    arg_pointers[n] = &args[n];
  }
  for (const auto &rule : rules_) {
    // Invariant: capture_groups <= kMaxRegexArgs
    // RE2 will fail to match if we provide more args than there are captures
    // for a given regex.
    int capture_groups = rule.pattern->NumberOfCapturingGroups();
    if (RE2::FullMatchN(path, *rule.pattern, arg_pointers, capture_groups)) {
      kythe::proto::VName result;
      if (rule.corpus.size()) {
        result.set_corpus(ApplyRule(rule.corpus, argv, capture_groups));
      }
      if (rule.root.size()) {
        result.set_root(ApplyRule(rule.root, argv, capture_groups));
      }
      if (rule.path.size()) {
        result.set_path(ApplyRule(rule.path, argv, capture_groups));
      }
      return result;
    }
  }
  return kythe::proto::VName();
}

kythe::proto::VName FileVNameGenerator::LookupVName(
    const std::string &path) const {
  kythe::proto::VName vname = LookupBaseVName(path);
  if (vname.path().empty()) {
    vname.set_path(path);
  }
  return vname;
}

bool FileVNameGenerator::LoadJsonString(const std::string &data,
                                        std::string *error_text) {
  CHECK(error_text != nullptr);
  using Value = rapidjson::Value;
  rapidjson::Document document;
  document.Parse(data.c_str());
  if (document.HasParseError()) {
    if (error_text) {
      *error_text = rapidjson::GetParseError_En(document.GetParseError());
      error_text->append(" near offset ");
      error_text->append(std::to_string(document.GetErrorOffset()));
    }
    return false;
  }
  if (!document.IsArray()) {
    *error_text = "Root element in JSON was not an array.";
    return false;
  }
  for (Value::ConstValueIterator rule = document.Begin();
       rule != document.End(); ++rule) {
    if (!rule->IsObject()) {
      *error_text = "Expected an array of objects.";
      return false;
    }
    const auto regex = rule->FindMember("pattern");
    if (regex == rule->MemberEnd() || !regex->value.IsString()) {
      *error_text = "VName rule is missing its pattern.";
      return false;
    }
    VNameRule next_rule;
    // RE2s don't act like values. Just box them.
    next_rule.pattern = std::shared_ptr<RE2>(new RE2(regex->value.GetString()));
    if (next_rule.pattern->error_code() != RE2::NoError) {
      *error_text = next_rule.pattern->error();
      return false;
    }
    int num_captures = next_rule.pattern->NumberOfCapturingGroups();
    CHECK(num_captures <= kMaxRegexArgs);
    const auto vname = rule->FindMember("vname");
    if (vname == rule->MemberEnd() || !vname->value.IsObject()) {
      *error_text = "VName rule is missing its template.";
      return false;
    }
    const auto root = vname->value.FindMember("root");
    if (root != vname->value.MemberEnd()) {
      if (!root->value.IsString()) {
        *error_text = "VName template root is not a string.";
        return false;
      }
      if (!ParseRule(root->value.GetString(), num_captures, &next_rule.root,
                     error_text)) {
        return false;
      }
    }
    const auto corpus = vname->value.FindMember("corpus");
    if (corpus != vname->value.MemberEnd()) {
      if (!corpus->value.IsString()) {
        *error_text = "VName template corpus is not a string.";
        return false;
      }
      if (!ParseRule(corpus->value.GetString(), num_captures, &next_rule.corpus,
                     error_text)) {
        return false;
      }
    }
    const auto path = vname->value.FindMember("path");
    if (path != vname->value.MemberEnd()) {
      if (!path->value.IsString()) {
        *error_text = "VName template path is not a string.";
        return false;
      }
      if (!ParseRule(path->value.GetString(), num_captures, &next_rule.path,
                     error_text)) {
        return false;
      }
    }
    rules_.push_back(next_rule);
  }
  return true;
}
}  // namespace kythe
