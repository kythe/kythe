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
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/log/check.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "interpreter/Engine.h"
#include "kythe/cxx/verifier/assertion_ast.h"
#include "kythe/cxx/verifier/assertions_to_souffle.h"
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
      const AnchorMap& anchors, const Database& database,
      const SymbolTable& dst)
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
#ifdef DEBUG_LOWERING
    FileHandlePrettyPrinter dprinter(stderr);
    for (const auto& d : database) {
      dprinter.Print("- ");
      d->Dump(dst, &dprinter);
      dprinter.Print("\n");
    }
    for (const auto& a : anchors) {
      dprinter.Print(
          absl::StrCat("@ ", a.first.first, ", ", a.first.second, " "));
      a.second->Dump(dst, &dprinter);
      dprinter.Print("\n");
    }
#endif
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
        // TODO(zarko): this should possibly be an explicit nil.
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
    return nullptr;
  }

 private:
  void CopyVName(kythe::verifier::Tuple* vname,
                 souffle::RamDomain (&target)[5]) {
    // sig, corp, root, path, lang
    target[0] = vname->element(0)->AsIdentifier()->symbol();
    target[1] = vname->element(1)->AsIdentifier()->symbol();
    target[2] = vname->element(2)->AsIdentifier()->symbol();
    target[3] = vname->element(3)->AsIdentifier()->symbol();
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
  KytheReadStreamFactory(const AnchorMap& anchors, const Database& database,
                         const SymbolTable& dst)
      : anchors_(anchors), database_(database), dst_(dst) {}

  souffle::Own<souffle::ReadStream> getReader(
      const std::map<std::string, std::string>& rw_operation,
      souffle::SymbolTable& symbol_table,
      souffle::RecordTable& record_table) override {
    return souffle::mk<KytheReadStream>(
        rw_operation, symbol_table, record_table, anchors_, database_, dst_);
  }

  const std::string& getName() const override {
    static const std::string name = "kythe";
    return name;
  }

 private:
  const AnchorMap& anchors_;
  const Database& database_;
  const SymbolTable& dst_;
};

struct KytheOutput {
  std::vector<std::string> outputs;
};

class KytheWriteStream : public souffle::WriteStream {
 public:
  explicit KytheWriteStream(
      const std::map<std::string, std::string>& rw_operation,
      const souffle::SymbolTable& symbol_table,
      const souffle::RecordTable& record_table,
      const std::vector<EVarType>& output_types,
      const std::function<std::string(size_t)>& get_symbol, KytheOutput* output)
      : souffle::WriteStream(rw_operation, symbol_table, record_table),
        output_types_(output_types),
        get_symbol_(get_symbol),
        output_(output) {}

 protected:
  void writeNullary() override { output_->outputs.push_back(""); }

  void writeNextTuple(const souffle::RamDomain* tuple) override {
    if (output_types_.empty()) {
      output_->outputs.push_back("");
      return;
    }
    for (size_t ox = 0; ox < output_types_.size(); ++ox) {
      output_->outputs.push_back("");
      auto* o = &output_->outputs.back();
      auto type = output_types_[ox];
      if (tuple[ox] == 0) {
        absl::StrAppend(o, "nil");
        continue;
      }
      switch (type) {
        case EVarType::kVName: {
          const auto* v = recordTable.unpack(tuple[ox], 5);
          absl::StrAppend(o, "vname(", get_symbol_(v[0]), ", ",
                          get_symbol_(v[1]), ", ", get_symbol_(v[2]), ", ",
                          get_symbol_(v[3]), ", ", get_symbol_(v[4]), ")");
        } break;
        case EVarType::kSymbol: {
          *o = get_symbol_(tuple[ox]);
        } break;
        default:
          *o = "unknown-type";
      }
    }
  }

 private:
  std::vector<EVarType> output_types_;
  std::function<std::string(size_t)> get_symbol_;
  KytheOutput* output_;
};

class KytheWriteStreamFactory : public souffle::WriteStreamFactory {
 public:
  KytheWriteStreamFactory(const std::vector<EVarType>& output_types,
                          const std::function<std::string(size_t)>& get_symbol)
      : output_types_(output_types), get_symbol_(get_symbol) {}
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
                                         record_table, output_types_,
                                         get_symbol_, &outputs_[output_id]);
  }

  const std::string& getName() const override {
    static const std::string name = "kythe";
    return name;
  }

  size_t NewOutput() {
    outputs_.push_back({});
    return outputs_.size() - 1;
  }

  const KytheOutput& GetOutput(size_t id) const { return outputs_[id]; }

 private:
  std::vector<KytheOutput> outputs_;
  std::vector<EVarType> output_types_;
  std::function<std::string(size_t)> get_symbol_;
};
}  // anonymous namespace

SouffleResult RunSouffle(
    const SymbolTable& symbol_table, const std::vector<GoalGroup>& goal_groups,
    const Database& database, const AnchorMap& anchors,
    const std::vector<Inspection>& inspections,
    std::function<bool(const Inspection&, std::string_view)> inspect,
    std::function<std::string(size_t)> get_symbol) {
  SouffleResult result{};
  SouffleProgram program;
  if (!program.Lower(symbol_table, goal_groups, inspections)) {
    return result;
  }
  std::vector<EVarType> output_types;
  absl::flat_hash_map<EVar*, size_t> output_indices;
  for (const auto& ev : program.inspections()) {
    auto t = program.EVarTypeFor(ev);
    output_indices[ev] = output_types.size();
    output_types.push_back(t ? *t : EVarType::kUnknown);
  }
  auto write_stream_factory =
      std::make_shared<KytheWriteStreamFactory>(output_types, get_symbol);
  size_t output_id = write_stream_factory->NewOutput();
  souffle::IOSystem::getInstance().registerWriteStreamFactory(
      write_stream_factory);
  souffle::IOSystem::getInstance().registerReadStreamFactory(
      std::make_shared<KytheReadStreamFactory>(anchors, database,
                                               symbol_table));
  auto ram_tu = souffle::ParseTransform(absl::StrCat(
      program.code(), ".output result(IO=kythe, id=", output_id, ")\n"));
  if (ram_tu == nullptr) {
    LOG(ERROR) << "no ram_tu for program <" << program.code() << ">";
    return result;
  }
  souffle::Own<souffle::interpreter::Engine> interpreter(
      souffle::mk<souffle::interpreter::Engine>(*ram_tu));
  interpreter->executeMain();
  const auto& actual = write_stream_factory->GetOutput(output_id);
  result.success = (!actual.outputs.empty());
  if (!result.success) return result;
  for (const auto& i : inspections) {
    auto ix = output_indices.find(i.evar);
    if (ix != output_indices.end()) {
      if (!inspect(i, actual.outputs[ix->second])) {
        break;
      }
    } else {
      LOG(ERROR) << "missing output index for inspection with label "
                 << i.label;
    }
  }
  return result;
}
}  // namespace kythe::verifier
