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

#ifndef KYTHE_CXX_VERIFIER_ASSERTION_AST_H_
#define KYTHE_CXX_VERIFIER_ASSERTION_AST_H_

#include <ctype.h>

#include <algorithm>
#include <optional>
#include <unordered_map>
#include <vector>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/strings/escaping.h"
#include "absl/strings/str_cat.h"
#include "kythe/cxx/verifier/location.hh"
#include "pretty_printer.h"
#include "re2/re2.h"

namespace kythe {
namespace verifier {

/// \brief Given a `SymbolTable`, uniquely identifies some string of text.
/// If two `Symbol`s are equal, their original text is equal.
typedef size_t Symbol;

/// \brief Maps strings to `Symbol`s.
class SymbolTable {
 public:
  explicit SymbolTable() : id_regex_("[%#]?[_a-zA-Z/][a-zA-Z_0-9/]*") {}

  /// \brief Returns the `Symbol` associated with `string` or `nullopt`.
  std::optional<Symbol> FindInterned(absl::string_view string) const {
    const auto old = symbols_.find(std::string(string));
    if (old == symbols_.end()) return std::nullopt;
    return old->second;
  }

  /// \brief Returns the `Symbol` associated with `string`, or aborts.
  Symbol MustIntern(absl::string_view string) const {
    auto sym = FindInterned(string);
    CHECK(sym) << "no symbol for " << string;
    return *sym;
  }

  /// \brief Returns the `Symbol` associated with `string`, or makes a new one.
  Symbol intern(const std::string& string) {
    const auto old = symbols_.find(string);
    if (old != symbols_.end()) {
      return old->second;
    }
    Symbol next_symbol = reverse_map_.size();
    symbols_[string] = next_symbol;
    // Note that references to elements of `unordered_map` are not invalidated
    // upon insert (really, upon rehash), so keeping around pointers in
    // `reverse_map_` is safe.
    reverse_map_.push_back(&symbols_.find(string)->first);
    return next_symbol;
  }
  /// \brief Returns the text associated with `symbol`.
  const std::string& text(Symbol symbol) const { return *reverse_map_[symbol]; }

  /// \brief Returns a string associated with `symbol` that disambiguates
  /// nonces.
  std::string PrettyText(Symbol symbol) const {
    auto* text = reverse_map_[symbol];
    if (text == &unique_symbol_) {
      return absl::StrCat("(unique#", std::to_string(symbol), ")");
    } else if (!text->empty() && RE2::FullMatch(*text, id_regex_)) {
      return *text;
    } else {
      return absl::StrCat("\"", absl::CHexEscape(*text), "\"");
    }
  }

  /// \brief Returns a `Symbol` that can never be spelled (but which still has
  /// a printable name).
  Symbol unique() {
    reverse_map_.push_back(&unique_symbol_);
    return reverse_map_.size() - 1;
  }

 private:
  /// Maps text to unique `Symbol`s.
  std::unordered_map<std::string, Symbol> symbols_;
  /// Maps `Symbol`s back to their original text.
  std::vector<const std::string*> reverse_map_;
  /// The text to use for unique() symbols.
  std::string unique_symbol_ = "(unique)";
  /// Used for quoting strings - see assertions.lex:
  RE2 id_regex_;
};

/// \brief Performs bump-pointer allocation of pointer-aligned memory.
///
/// AST nodes do not need to be deallocated piecemeal. The interpreter
/// does not permit uncontrolled mutable state: `EVars` are always unset
/// when history is rewound, so links to younger memory that have leaked out
/// into older memory at a choice point are severed when that choice point is
/// reconsidered. This means that entire swaths of memory can safely be
/// deallocated at once without calling individual destructors.
///
/// \warning Since `Arena`-allocated objects never have their destructors
/// called, any non-POD members they have will in turn never be destroyed.
class Arena {
 public:
  Arena() : next_block_index_(0) {}

  ~Arena() {
    for (auto& b : blocks_) {
      delete[] b;
    }
  }

  /// \brief Allocate `bytes` bytes, aligned to `kPointerSize`, allocating
  /// new blocks from the system if necessary.
  void* New(size_t bytes) {
    // Align to kPointerSize bytes.
    bytes = (bytes + kPointerSize - 1) & kPointerSizeMask;
    CHECK(bytes < kBlockSize);
    offset_ += bytes;
    if (offset_ > kBlockSize) {
      if (next_block_index_ == blocks_.size()) {
        char* next_block = new char[kBlockSize];
        blocks_.push_back(next_block);
        current_block_ = next_block;
      } else {
        current_block_ = blocks_[next_block_index_];
      }
      ++next_block_index_;
      offset_ = bytes;
    }
    return current_block_ + offset_ - bytes;
  }

 private:
  /// The size of a pointer on this machine. We support only machines with
  /// power-of-two address size and alignment requirements.
  const size_t kPointerSize = sizeof(void*);
  /// `kPointerSize` (a power of two) sign-extended from its first set bit.
  const size_t kPointerSizeMask = ((~kPointerSize) + 1);
  /// The size of allocation requests to make from the normal heap.
  const size_t kBlockSize = 1024 * 64;

  /// The next offset in the current block to allocate. Should always be
  /// `<= kBlockSize`. If it is `== kBlockSize`, the current block is
  /// exhausted and the `Arena` moves on to the next block, allocating one
  /// if necessary.
  size_t offset_ = kBlockSize;
  /// The index of the next block to allocate from. Should always be
  /// `<= blocks_.size()`. If it is `== blocks_.size()`, a new block is
  /// allocated before the next `New` request completes.
  size_t next_block_index_;
  /// The block from which the `Arena` is currently making allocations. May
  /// be `nullptr` if no allocations have yet been made.
  char* current_block_;
  /// All blocks that the `Arena` has allocated so far.
  std::vector<char*> blocks_;
};

/// \brief An object that can be allocated inside an `Arena`.
class ArenaObject {
 public:
  void* operator new(size_t size, Arena* arena) { return arena->New(size); }
  void operator delete(void*, size_t) {
    LOG(FATAL) << "Don't delete ArenaObjects.";
  }
  void operator delete(void* ptr, Arena* arena) {
    LOG(FATAL) << "Don't delete ArenaObjects.";
  }
};

class App;
class EVar;
class Identifier;
class Range;
class Tuple;

/// \brief An object that is manipulated by the verifier during the course of
/// interpretation.
///
/// Some `AstNode`s are dynamically created as part of the execution process.
/// Others are created during parsing. `AstNode`s are generally treated as
/// immutable after their initial construction phase except for certain notable
/// exceptions like `EVar`.
class AstNode : public ArenaObject {
 public:
  explicit AstNode(const yy::location& location) : location_(location) {}

  /// \brief Returns the location where the `AstNode` was found if it came
  /// from source text.
  const yy::location& location() const { return location_; }

  /// \brief Dumps the `AstNode` to `printer`.
  virtual void Dump(const SymbolTable& symbol_table, PrettyPrinter* printer) {}

  virtual App* AsApp() { return nullptr; }
  virtual EVar* AsEVar() { return nullptr; }
  virtual Identifier* AsIdentifier() { return nullptr; }
  virtual Range* AsRange() { return nullptr; }
  virtual Tuple* AsTuple() { return nullptr; }

 private:
  /// \brief The location where the `AstNode` can be found in source text, if
  /// any.
  yy::location location_;
};

/// \brief A range specification that can unify with one or more ranges.
class Range : public AstNode {
 public:
  Range(const yy::location& location, size_t begin, size_t end, Symbol path,
        Symbol root, Symbol corpus)
      : AstNode(location),
        begin_(begin),
        end_(end),
        path_(path),
        root_(root),
        corpus_(corpus) {}
  Range* AsRange() override { return this; }
  void Dump(const SymbolTable&, PrettyPrinter*) override;
  size_t begin() const { return begin_; }
  size_t end() const { return end_; }
  size_t path() const { return path_; }
  size_t corpus() const { return corpus_; }
  size_t root() const { return root_; }

 private:
  /// The start of the range in bytes.
  size_t begin_;
  /// The end of the range in bytes.
  size_t end_;
  /// The source file path.
  Symbol path_;
  /// The source file root.
  Symbol root_;
  /// The source file corpus.
  Symbol corpus_;
};

inline bool operator==(const Range& l, const Range& r) {
  return l.begin() == r.begin() && l.end() == r.end() && l.path() == r.path() &&
         l.root() == r.root() && l.corpus() == r.corpus();
}

inline bool operator!=(const Range& l, const Range& r) { return !(l == r); }

/// \brief A tuple of zero or more elements.
class Tuple : public AstNode {
 public:
  /// \brief Constructs a new `Tuple`
  /// \param location Mark with this location
  /// \param element_count The number of elements in the tuple
  /// \param elements A preallocated buffer of `AstNode*` such that
  /// the total size of the buffer is equal to
  /// `element_count * sizeof(AstNode *)`
  Tuple(const yy::location& location, size_t element_count, AstNode** elements)
      : AstNode(location), element_count_(element_count), elements_(elements) {}
  Tuple* AsTuple() override { return this; }
  void Dump(const SymbolTable&, PrettyPrinter*) override;
  /// \brief Returns the number of elements in the `Tuple`.
  size_t size() const { return element_count_; }
  /// \brief Returns the `index`th element of the `Tuple`, counting from zero.
  AstNode* element(size_t index) const {
    CHECK(index < element_count_);
    return elements_[index];
  }

 private:
  /// The number of `AstNode *`s in `elements_`
  size_t element_count_;
  /// Storage for the `Tuple`'s elements.
  AstNode** elements_;
};

/// \brief An application (eg, `f(g)`).
///
/// Generally an `App` will combine some `Identifier` head with a `Tuple` body,
/// but this is not necessarily the case.
class App : public AstNode {
 public:
  /// \brief Constructs a new `App` node, taking its location from the
  /// location of the left-hand side.
  /// \param lhs The left-hand side of the application (eg, an `Identifier`).
  /// \param rhs The right-hand side of the application (eg, a `Tuple`).
  App(AstNode* lhs, AstNode* rhs)
      : AstNode(lhs->location()), lhs_(lhs), rhs_(rhs) {}
  /// \brief Constructs a new `App` node with an explicit `location`.
  /// \param `location` The location to use for this `App`.
  /// \param lhs The left-hand side of the application (eg, an `Identifier`).
  /// \param rhs The right-hand side of the application (eg, a `Tuple`).
  App(const yy::location& location, AstNode* lhs, AstNode* rhs)
      : AstNode(location), lhs_(lhs), rhs_(rhs) {}

  App* AsApp() override { return this; }
  void Dump(const SymbolTable&, PrettyPrinter*) override;

  /// \brief The left-hand side (`f` in `f(g)`) of this `App`
  AstNode* lhs() const { return lhs_; }
  /// \brief The right-hand side (`(g)` in `f(g)`) of this `App
  AstNode* rhs() const { return rhs_; }

 private:
  AstNode* lhs_;
  AstNode* rhs_;
};

/// \brief An identifier (corresponding to some `Symbol`).
class Identifier : public AstNode {
 public:
  Identifier(const yy::location& location, Symbol symbol)
      : AstNode(location), symbol_(symbol) {}
  /// \brief The `Symbol` this `Identifier` represents.
  Symbol symbol() const { return symbol_; }
  Identifier* AsIdentifier() override { return this; }
  void Dump(const SymbolTable&, PrettyPrinter*) override;

 private:
  Symbol symbol_;
};

/// \brief An existential variable.
///
/// `EVars` are given assignments while the verifier solves for its goals.
/// Once an `EVar` is given an assignment, that assignment will not change,
/// unless it is undone by backtracking.
class EVar : public AstNode {
 public:
  /// Constructs a new `EVar` with no assignment.
  explicit EVar(const yy::location& location)
      : AstNode(location), current_(nullptr) {}
  EVar* AsEVar() override { return this; }
  void Dump(const SymbolTable&, PrettyPrinter*) override;

  /// \brief Returns current assignment, or `nullptr` if one has not been made.
  AstNode* current() { return current_; }

  /// \brief Assigns this `EVar`.
  void set_current(AstNode* node) { current_ = node; }

 private:
  /// The `EVar`'s current assignment or `nullptr`.
  AstNode* current_;
};

/// \brief A set of goals to be handled atomically.
struct GoalGroup {
  enum AcceptanceCriterion {
    kNoneMayFail,  ///< For this group to pass, no goals may fail.
    kSomeMustFail  ///< For this group to pass, some goals must fail.
  };
  AcceptanceCriterion accept_if;  ///< How this group is handled.
  std::vector<AstNode*> goals;    ///< Grouped goals, implicitly conjoined.
};

/// \brief A database of fact-shaped AstNodes.
using Database = std::vector<AstNode*>;

/// \brief Multimap from anchor offsets to anchor VName tuples.
using AnchorMap = std::multimap<std::pair<size_t, size_t>, AstNode*>;

/// An EVar whose assignment is interesting to display.
struct Inspection {
 public:
  enum class Kind {
    EXPLICIT,  ///< The user requested this inspection (with "?").
    IMPLICIT   ///< This inspection was added by default.
  };
  std::string label;  ///< A label for user reference.
  EVar* evar;         ///< The EVar to inspect.
  Kind kind;          ///< Whether this inspection was added by default.
  Inspection(const std::string& label, EVar* evar, Kind kind)
      : label(label), evar(evar), kind(kind) {}
};

}  // namespace verifier
}  // namespace kythe

// Required by generated code.
#define YY_DECL                                            \
  int kythe::verifier::AssertionParser::lex(               \
      YySemanticValue* yylval_param, yy::location* yylloc, \
      ::kythe::verifier::AssertionParser& context)
namespace kythe {
namespace verifier {
class AssertionParser;
}
}  // namespace kythe
struct YySemanticValue {
  std::string string;
  kythe::verifier::AstNode* node;
  int int_;
  size_t size_t_;
};
#define YYSTYPE YySemanticValue

#endif  // KYTHE_CXX_VERIFIER_ASSERTION_AST_H_
