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
#include "souffle/RamTypes.h"
#include "souffle/io/IOSystem.h"
#include "third_party/souffle/parse_transform.h"

#ifdef __APPLE__
extern "C" {
// TODO(zarko): fix these on darwin
void ffi_call() { abort(); }
void ffi_prep_cif_machdep() { abort(); }
void ffi_prep_closure_loc() { abort(); }
}
#endif  // defined(__APPLE__)

namespace kythe::verifier {
namespace {
constexpr std::array<int, 4> kInputData = {1, 2, 2, 3};

class KytheReadStream : public souffle::ReadStream {
 public:
  explicit KytheReadStream(
      const std::map<std::string, std::string>& rw_operation,
      souffle::SymbolTable& symbol_table, souffle::RecordTable& record_table)
      : souffle::ReadStream(rw_operation, symbol_table, record_table) {}

 protected:
  souffle::Own<souffle::RamDomain[]> readNextTuple() override {
    if (pos_ >= kInputData.size()) {
      return nullptr;
    }
    auto tuple = std::make_unique<souffle::RamDomain[]>(2);
    tuple[0] = kInputData[pos_++];
    tuple[1] = kInputData[pos_++];
    return tuple;
  }

 private:
  int pos_ = 0;
};

class KytheReadStreamFactory : public souffle::ReadStreamFactory {
 public:
  souffle::Own<souffle::ReadStream> getReader(
      const std::map<std::string, std::string>& rw_operation,
      souffle::SymbolTable& symbol_table,
      souffle::RecordTable& record_table) override {
    return souffle::mk<KytheReadStream>(rw_operation, symbol_table,
                                        record_table);
  }

  const std::string& getName() const override {
    static const std::string name = "kythe";
    return name;
  }
};

class KytheWriteStream : public souffle::WriteStream {
 public:
  explicit KytheWriteStream(
      const std::map<std::string, std::string>& rw_operation,
      const souffle::SymbolTable& symbol_table,
      const souffle::RecordTable& record_table,
      std::vector<std::pair<int, int>>* output)
      : souffle::WriteStream(rw_operation, symbol_table, record_table),
        output_(output) {}

 protected:
  void writeNullary() override {}

  void writeNextTuple(const souffle::RamDomain* tuple) override {
    output_->push_back({tuple[0], tuple[1]});
  }

 private:
  std::vector<std::pair<int, int>>* output_;
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
    outputs_.push_back({});
    return outputs_.size() - 1;
  }

  const std::vector<std::pair<int, int>>& GetOutput(size_t id) const {
    return outputs_[id];
  }

 private:
  std::vector<std::vector<std::pair<int, int>>> outputs_;
};
}  // anonymous namespace

// TODO(zarko): This is a temporary hack that only demonstrates that the
// Souffle library is working. It bakes in the inputs and outputs for a
// simple example program and checks that the interpreter calculates the
// correct result. `Kythe{Write,Read}StreamFactory` will need to be moved
// to separate files and updated to accept the Datalog relations that the
// verifier -> datalog compiler produces.
SouffleResult RunSouffle(
    const SymbolTable& symbol_table, const std::vector<GoalGroup>& goal_groups,
    const Database& database,
    const std::vector<AssertionParser::Inspection>& inspections,
    Verifier* verifier,
    std::function<bool(Verifier*, const AssertionParser::Inspection&)>
        inspect) {
  SouffleResult result{};
  auto write_stream_factory = std::make_shared<KytheWriteStreamFactory>();
  size_t output_id = write_stream_factory->NewOutput();
  souffle::IOSystem::getInstance().registerWriteStreamFactory(
      write_stream_factory);
  souffle::IOSystem::getInstance().registerReadStreamFactory(
      std::make_shared<KytheReadStreamFactory>());
  auto ram_tu = souffle::ParseTransform(absl::StrCat(R"(
        .decl edge(x:number, y:number)
        .input edge(IO=kythe)

        .decl path(x:number, y:number)
        .output path(IO=kythe, id=)",
                                                     output_id, R"()

        path(x, y) :- edge(x, y).
        path(x, y) :- path(x, z), edge(z, y).
    )"));
  if (ram_tu == nullptr) {
    return result;
  }
  souffle::Own<souffle::interpreter::Engine> interpreter(
      souffle::mk<souffle::interpreter::Engine>(*ram_tu));
  interpreter->executeMain();
  std::set<std::pair<int, int>> expected = {{1, 2}, {1, 3}, {2, 3}};
  const auto& actual = write_stream_factory->GetOutput(output_id);
  std::set<std::pair<int, int>> actual_set(actual.begin(), actual.end());
  result.success = (expected == actual_set);
  return result;
}
}  // namespace kythe::verifier
