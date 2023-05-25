/*
 * Copyright 2020 The Kythe Authors. All rights reserved.
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

#include "kythe/cxx/verifier/souffle_interpreter.h"

#include <array>
#include <memory>

#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "glog/logging.h"
#include "interpreter/Engine.h"
#include "kythe/cxx/verifier/assertions_to_souffle.h"
#include "kythe/cxx/verifier/verifier.h"
#include "souffle/RamTypes.h"
#include "souffle/io/IOSystem.h"
#include "third_party/souffle/parse_transform.h"

namespace kythe::verifier {
namespace {
class KytheReadStream : public souffle::ReadStream {
 public:
  explicit KytheReadStream(
      const std::map<std::string, std::string>& rw_operation,
      souffle::SymbolTable& symbol_table, souffle::RecordTable& record_table,
      const AnchorMap& anchors, const Database& database)
      : souffle::ReadStream(rw_operation, symbol_table, record_table),
        anchors_(anchors),
        database_(database) {
    if (auto op = rw_operation.find("anchors"); op != rw_operation.end()) {
      anchor_it_ = anchors_.cbegin();
      database_it_ = database_.cend();
    } else {
      database_it_ = database_.cbegin();
      anchor_it_ = anchors_.cend();
    }
    std::array<souffle::RamDomain, 5> fields = {0, 0, 0, 0, 0};
    empty_vname_ = record_table.pack(fields.data(), fields.size());
  }

 protected:
  souffle::Own<souffle::RamDomain[]> readNextTuple() override {
    if (database_it_ != database_.end()) {
      auto* t = (*database_it_)->AsApp()->rhs()->AsTuple();
      auto tuple = std::make_unique<souffle::RamDomain[]>(5);
      souffle::RamDomain vname[5];
      CopyVName(t->element(0)->AsApp()->rhs()->AsTuple(), vname);
      tuple[0] = recordTable.pack(vname, 5);
      tuple[1] = t->element(1)->AsIdentifier()->symbol();
      if (auto* id = t->element(2)->AsIdentifier(); id != nullptr) {
        tuple[2] = id->symbol();
      } else {
        CopyVName(t->element(2)->AsApp()->rhs()->AsTuple(), vname);
        tuple[2] = recordTable.pack(vname, 5);
      }
      tuple[3] = t->element(3)->AsIdentifier()->symbol();
      tuple[4] = t->element(4)->AsIdentifier()->symbol();
      ++database_it_;
      return tuple;
    } else if (anchor_it_ != anchors_.end()) {
      auto tuple = std::make_unique<souffle::RamDomain[]>(3);
      tuple[0] = anchor_it_->first.first;
      tuple[1] = anchor_it_->first.second;
      souffle::RamDomain vname[5];
      CopyVName(anchor_it_->second->AsApp()->rhs()->AsTuple(), vname);
      tuple[2] = recordTable.pack(vname, 5);
      ++anchor_it_;
      return tuple;
    }
  }

 private:
  void CopyVName(kythe::verifier::Tuple* vname, souffle::RamDomain* target) {
    // output: sig, corp, path, root, lang
    // input: sig, corp, root, path, lang
    target[0] = vname->element(0)->AsIdentifier()->symbol();
    target[1] = vname->element(1)->AsIdentifier()->symbol();
    target[2] = vname->element(3)->AsIdentifier()->symbol();
    target[3] = vname->element(2)->AsIdentifier()->symbol();
    target[4] = vname->element(4)->AsIdentifier()->symbol();
  }
  const AnchorMap& anchors_;
  const Database& database_;
  AnchorMap::const_iterator anchor_it_;
  Database::const_iterator database_it_;
  souffle::RamDomain empty_vname_;
};

class KytheReadStreamFactory : public souffle::ReadStreamFactory {
 public:
  KytheReadStreamFactory(const AnchorMap& anchors, const Database& database)
      : anchors_(anchors), database_(database) {}

  souffle::Own<souffle::ReadStream> getReader(
      const std::map<std::string, std::string>& rw_operation,
      souffle::SymbolTable& symbol_table,
      souffle::RecordTable& record_table) override {
    return souffle::mk<KytheReadStream>(rw_operation, symbol_table,
                                        record_table, anchors_, database_);
  }

  const std::string& getName() const override {
    static const std::string name = "kythe";
    return name;
  }

 private:
  const AnchorMap& anchors_;
  const Database& database_;
};

class KytheWriteStream : public souffle::WriteStream {
 public:
  explicit KytheWriteStream(
      const std::map<std::string, std::string>& rw_operation,
      const souffle::SymbolTable& symbol_table,
      const souffle::RecordTable& record_table, size_t* output)
      : souffle::WriteStream(rw_operation, symbol_table, record_table),
        output_(output) {}

 protected:
  void writeNullary() override { (*output_)++; }

  void writeNextTuple(const souffle::RamDomain* tuple) override {
    (*output_)++;
  }

 private:
  size_t* output_;
};

class KytheWriteStreamFactory : public souffle::WriteStreamFactory {
 public:
  souffle::Own<souffle::WriteStream> getWriter(
      const std::map<std::string, std::string>& rw_operation,
      const souffle::SymbolTable& symbol_table,
      const souffle::RecordTable& record_table) override {
    auto output = rw_operation.find("id");
    CHECK(output != rw_operation.end());
    size_t output_id;
    CHECK(absl::SimpleAtoi(output->second, &output_id));
    CHECK(output_id < outputs_.size());
    return souffle::mk<KytheWriteStream>(rw_operation, symbol_table,
                                         record_table, &outputs_[output_id]);
  }

  const std::string& getName() const override {
    static const std::string name = "kythe";
    return name;
  }

  size_t NewOutput() {
    outputs_.push_back(0);
    return outputs_.size() - 1;
  }

  size_t GetOutput(size_t id) const { return outputs_[id]; }

 private:
  std::vector<size_t> outputs_;
};
}  // anonymous namespace

SouffleResult RunSouffle(
    const SymbolTable& symbol_table, const std::vector<GoalGroup>& goal_groups,
    const Database& database, const AnchorMap& anchors,
    const std::vector<AssertionParser::Inspection>& inspections,
    std::function<bool(const AssertionParser::Inspection&)> inspect) {
  SouffleResult result{};
  SouffleProgram program;
  if (!program.Lower(symbol_table, goal_groups)) {
    return result;
  }
  auto write_stream_factory = std::make_shared<KytheWriteStreamFactory>();
  size_t output_id = write_stream_factory->NewOutput();
  souffle::IOSystem::getInstance().registerWriteStreamFactory(
      write_stream_factory);
  souffle::IOSystem::getInstance().registerReadStreamFactory(
      std::make_shared<KytheReadStreamFactory>(anchors, database));
  auto ram_tu = souffle::ParseTransform(absl::StrCat(
      program.code(), ".output result(IO=kythe, id=", output_id, ")\n"));
  if (ram_tu == nullptr) {
    return result;
  }
  souffle::Own<souffle::interpreter::Engine> interpreter(
      souffle::mk<souffle::interpreter::Engine>(*ram_tu));
  interpreter->executeMain();
  size_t actual = write_stream_factory->GetOutput(output_id);
  result.success = (actual != 0);
  return result;
}
}  // namespace kythe::verifier
