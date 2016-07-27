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

#include "verifier.h"

#include "glog/logging.h"
#include "google/protobuf/text_format.h"

#include "assertions.h"
#include "kythe/proto/storage.pb.h"

namespace kythe {
namespace verifier {
namespace {

typedef std::vector<AstNode *> Database;

/// \brief The return code from a verifier thunk.
using ThunkRet = size_t;
/// \brief The operation failed normally.
static ThunkRet kNoException = {0};
/// \brief There is no more work to do, so unwind.
static ThunkRet kSolved = {1};
/// \brief The program is invalid, so unwind.
static ThunkRet kInvalidProgram = {2};
/// \brief The goal group is known to be impossible to solve.
static ThunkRet kImpossible = {3};
/// \brief ThunkRets >= kFirstCut should unwind to the frame
/// establishing that cut without changing assignments.
static ThunkRet kFirstCut = {4};

typedef const std::function<ThunkRet()> &Thunk;

static std::string *kDefaultDatabase = new std::string("builtin");
static std::string *kStandardIn = new std::string("-");

static bool EncodedIdentEqualTo(AstNode *a, AstNode *b) {
  Identifier *ia = a->AsIdentifier();
  Identifier *ib = b->AsIdentifier();
  return ia->symbol() == ib->symbol();
}

static bool EncodedIdentLessThan(AstNode *a, AstNode *b) {
  Identifier *ia = a->AsIdentifier();
  Identifier *ib = b->AsIdentifier();
  return ia->symbol() < ib->symbol();
}

static bool EncodedVNameEqualTo(App *a, App *b) {
  Tuple *ta = a->rhs()->AsTuple();
  Tuple *tb = b->rhs()->AsTuple();
  for (int i = 0; i < 5; ++i) {
    if (!EncodedIdentEqualTo(ta->element(i), tb->element(i))) {
      return false;
    }
  }
  return true;
}

static bool EncodedVNameLessThan(App *a, App *b) {
  Tuple *ta = a->rhs()->AsTuple();
  Tuple *tb = b->rhs()->AsTuple();
  for (int i = 0; i < 4; ++i) {
    if (EncodedIdentLessThan(ta->element(i), tb->element(i))) {
      return true;
    }
    if (!EncodedIdentEqualTo(ta->element(i), tb->element(i))) {
      return false;
    }
  }
  return EncodedIdentLessThan(ta->element(4), tb->element(4));
}

static bool EncodedVNameOrIdentLessThan(AstNode *a, AstNode *b) {
  App *aa = a->AsApp();  // nullptr if a is not a vname
  App *ab = b->AsApp();  // nullptr if b is not a vname
  if (aa && ab) {
    return EncodedVNameLessThan(aa, ab);
  } else if (!aa && ab) {
    // Arbitrarily, vname < ident.
    return true;
  } else if (aa && !ab) {
    return false;
  } else {
    return EncodedIdentLessThan(a, b);
  }
}

static bool EncodedVNameOrIdentEqualTo(AstNode *a, AstNode *b) {
  App *aa = a->AsApp();  // nullptr if a is not a vname
  App *ab = b->AsApp();  // nullptr if b is not a vname
  if (aa && ab) {
    return EncodedVNameEqualTo(aa, ab);
  } else if (!aa && ab) {
    return false;
  } else if (aa && !ab) {
    return false;
  } else {
    return EncodedIdentEqualTo(a, b);
  }
}

/// \brief Sort entries such that those that set fact values are adjacent.
static bool EncodedFactLessThan(AstNode *a, AstNode *b) {
  Tuple *ta = a->AsApp()->rhs()->AsTuple();
  Tuple *tb = b->AsApp()->rhs()->AsTuple();
  if (EncodedVNameOrIdentLessThan(ta->element(0), tb->element(0))) {
    return true;
  }
  if (!EncodedVNameOrIdentEqualTo(ta->element(0), tb->element(0))) {
    return false;
  }
  if (EncodedIdentLessThan(ta->element(1), tb->element(1))) {
    return true;
  }
  if (!EncodedIdentEqualTo(ta->element(1), tb->element(1))) {
    return false;
  }
  if (EncodedVNameOrIdentLessThan(ta->element(2), tb->element(2))) {
    return true;
  }
  if (!EncodedVNameOrIdentEqualTo(ta->element(2), tb->element(2))) {
    return false;
  }
  if (EncodedIdentLessThan(ta->element(3), tb->element(3))) {
    return true;
  }
  if (!EncodedIdentEqualTo(ta->element(3), tb->element(3))) {
    return false;
  }
  if (EncodedIdentLessThan(ta->element(4), tb->element(4))) {
    return true;
  }
  return false;
}

static AstNode *DerefEVar(AstNode *node) {
  while (node) {
    if (auto *evar = node->AsEVar()) {
      node = evar->current();
    } else {
      break;
    }
  }
  return node;
}

static Identifier *SafeAsIdentifier(AstNode *node) {
  return node == nullptr ? nullptr : node->AsIdentifier();
}

struct AtomFactKey {
  Identifier *edge_kind;
  Identifier *fact_name;
  Identifier *fact_value;
  Identifier *source_vname[5] = {nullptr, nullptr, nullptr, nullptr, nullptr};
  Identifier *target_vname[5] = {nullptr, nullptr, nullptr, nullptr, nullptr};
  // fact_tuple is expected to be a full tuple from a Fact head
  AtomFactKey(AstNode *vname_head, Tuple *fact_tuple)
      : edge_kind(SafeAsIdentifier(DerefEVar(fact_tuple->element(1)))),
        fact_name(SafeAsIdentifier(DerefEVar(fact_tuple->element(3)))),
        fact_value(SafeAsIdentifier(DerefEVar(fact_tuple->element(4)))) {
    InitVNameFields(vname_head, fact_tuple->element(0), &source_vname[0]);
    InitVNameFields(vname_head, fact_tuple->element(2), &target_vname[0]);
  }
  void InitVNameFields(AstNode *vname_head, AstNode *maybe_vname,
                       Identifier **out) {
    maybe_vname = DerefEVar(maybe_vname);
    if (maybe_vname == nullptr) {
      return;
    }
    if (auto *app = maybe_vname->AsApp()) {
      if (DerefEVar(app->lhs()) != vname_head) {
        return;
      }
      AstNode *maybe_tuple = DerefEVar(app->rhs());
      if (maybe_tuple == nullptr) {
        return;
      }
      if (auto *tuple = maybe_tuple->AsTuple()) {
        if (tuple->size() != 5) {
          return;
        }
        for (size_t i = 0; i < 5; ++i) {
          out[i] = SafeAsIdentifier(DerefEVar(tuple->element(i)));
        }
      }
    }
  }
};

namespace {

enum class Order { LT, EQ, GT };

// How we order incomplete keys depends on whether we're looking for
// an upper or lower bound. See below for details. The node passed in
// must be an application of Fact to a full fact tuple.
static Order CompareFactWithKey(Order incomplete, AstNode *a, AtomFactKey *k) {
  Tuple *ta = a->AsApp()->rhs()->AsTuple();
  if (k->edge_kind == nullptr) {
    return incomplete;
  } else if (EncodedIdentLessThan(ta->element(1), k->edge_kind)) {
    return Order::LT;
  } else if (!EncodedIdentEqualTo(ta->element(1), k->edge_kind)) {
    return Order::GT;
  }
  if (k->fact_name == nullptr) {
    return incomplete;
  } else if (EncodedIdentLessThan(ta->element(3), k->fact_name)) {
    return Order::LT;
  } else if (!EncodedIdentEqualTo(ta->element(3), k->fact_name)) {
    return Order::GT;
  }
  if (k->fact_value == nullptr) {
    return incomplete;
  } else if (EncodedIdentLessThan(ta->element(4), k->fact_value)) {
    return Order::LT;
  } else if (!EncodedIdentEqualTo(ta->element(4), k->fact_value)) {
    return Order::GT;
  }
  auto vname_compare = [incomplete](Tuple *va, Identifier *tuple[5]) {
    for (size_t i = 0; i < 5; ++i) {
      if (tuple[i] == nullptr) {
        return incomplete;
      }
      if (EncodedIdentLessThan(va->element(i), tuple[i])) {
        return Order::LT;
      }
      if (!EncodedIdentEqualTo(va->element(i), tuple[i])) {
        return Order::GT;
      }
    }
    return Order::EQ;
  };
  if (Tuple *vs = ta->element(0)->AsApp()->rhs()->AsTuple()) {
    auto ord = vname_compare(vs, k->source_vname);
    if (ord != Order::EQ) {
      return ord;
    }
  }
  if (auto *app = ta->element(2)->AsApp()) {
    if (Tuple *vt = app->rhs()->AsTuple()) {
      auto ord = vname_compare(vt, k->target_vname);
      if (ord != Order::EQ) {
        return ord;
      }
    }
  }
  return Order::EQ;
}
}

// We want to be able to find the following bounds:
// (0,0,2,3) (0,1,2,3) (0,1,2,4) (1,1,2,4)
//          ^---  (0,1,_,_)  ---^

static bool FastLookupKeyLessThanFact(AtomFactKey *k, AstNode *a) {
  // This is used to find upper bounds, so keys with incomplete suffixes should
  // be ordered after all facts that share their complete prefixes.
  return CompareFactWithKey(Order::LT, a, k) == Order::GT;
}

static bool FastLookupFactLessThanKey(AstNode *a, AtomFactKey *k) {
  // This is used to find lower bounds, so keys with incomplete suffixes should
  // be ordered after facts with lower prefixes but before facts with complete
  // suffixes.
  return CompareFactWithKey(Order::GT, a, k) == Order::LT;
}

//  Sort entries in lexicographic order, collating as:
// `(edge_kind, fact_name, fact_value, source_node, target_node)`.
// In practice most unification was happening between tuples
// with the first three fields present; then source_node
// missing some of the time; then target_node missing most of
// the time.
static bool FastLookupFactLessThan(AstNode *a, AstNode *b) {
  Tuple *ta = a->AsApp()->rhs()->AsTuple();
  Tuple *tb = b->AsApp()->rhs()->AsTuple();
  if (EncodedIdentLessThan(ta->element(1), tb->element(1))) {
    return true;
  }
  if (!EncodedIdentEqualTo(ta->element(1), tb->element(1))) {
    return false;
  }
  if (EncodedIdentLessThan(ta->element(3), tb->element(3))) {
    return true;
  }
  if (!EncodedIdentEqualTo(ta->element(3), tb->element(3))) {
    return false;
  }
  if (EncodedIdentLessThan(ta->element(4), tb->element(4))) {
    return true;
  }
  if (!EncodedIdentEqualTo(ta->element(4), tb->element(4))) {
    return false;
  }
  if (EncodedVNameOrIdentLessThan(ta->element(0), tb->element(0))) {
    return true;
  }
  if (!EncodedVNameOrIdentEqualTo(ta->element(0), tb->element(0))) {
    return false;
  }
  if (EncodedVNameOrIdentLessThan(ta->element(2), tb->element(2))) {
    return true;
  }
  return false;
}

// The Solver acts in a closed world: any universal quantification can be
// exhaustively tested against database facts.
// Based on _A Semi-Functional Implementation of a Higher-Order Logic
// Programming Language_ by Conal Elliott and Frank Pfenning (draft of
// February 1990).
// It is not our intention to build a particularly performant or complete
// inference engine. If the solver starts to get too hairy we might want to
// look at deferring to a pre-existing system.
class Solver {
 public:
  using Inspection = AssertionParser::Inspection;

  Solver(Verifier *context, Database &database,
         std::multimap<std::pair<size_t, size_t>, AstNode *> &anchors,
         std::function<bool(Verifier *, const Inspection &)> &inspect)
      : context_(*context),
        database_(database),
        anchors_(anchors),
        inspect_(inspect) {}

  ThunkRet UnifyTuple(Tuple *st, Tuple *tt, size_t ofs, size_t max,
                      ThunkRet cut, Thunk f) {
    if (ofs == max) return f();
    return Unify(st->element(ofs), tt->element(ofs), cut,
                 [this, st, tt, ofs, max, cut, &f]() {
                   return UnifyTuple(st, tt, ofs + 1, max, cut, f);
                 });
  }

  ThunkRet Unify(AstNode *s, AstNode *t, ThunkRet cut, Thunk f) {
    if (EVar *e = s->AsEVar()) {
      return UnifyEVar(e, t, cut, f);
    } else if (EVar *e = t->AsEVar()) {
      return UnifyEVar(e, s, cut, f);
    } else if (Identifier *si = s->AsIdentifier()) {
      if (Identifier *ti = t->AsIdentifier()) {
        if (si->symbol() == ti->symbol()) {
          return f();
        }
      }
    } else if (App *sa = s->AsApp()) {
      if (App *ta = t->AsApp()) {
        return Unify(sa->lhs(), ta->lhs(), cut, [this, sa, ta, cut, &f]() {
          return Unify(sa->rhs(), ta->rhs(), cut, f);
        });
      }
    } else if (Tuple *st = s->AsTuple()) {
      if (Tuple *tt = t->AsTuple()) {
        if (st->size() != tt->size()) {
          return kNoException;
        }
        return UnifyTuple(st, tt, 0, st->size(), cut, f);
      }
    } else if (Range *sr = s->AsRange()) {
      if (Range *tr = t->AsRange()) {
        if (sr->begin() == tr->begin() && sr->end() == tr->end()) {
          return f();
        }
      }
    }
    return kNoException;
  }

  bool Occurs(EVar *e, AstNode *t) {
    if (App *a = t->AsApp()) {
      return Occurs(e, a->lhs()) || Occurs(e, a->rhs());
    } else if (EVar *ev = t->AsEVar()) {
      return ev->current() ? Occurs(e, ev->current()) : e == ev;
    } else if (Tuple *tu = t->AsTuple()) {
      for (size_t i = 0, c = tu->size(); i != c; ++i) {
        if (Occurs(e, tu->element(i))) {
          return true;
        }
      }
      return false;
    } else if (Range *r = t->AsRange()) {
      return false;
    } else {
      CHECK(t->AsIdentifier() && "Inexhaustive match.");
      return false;
    }
    return true;
  }

  ThunkRet UnifyEVar(EVar *e, AstNode *t, ThunkRet cut, Thunk f) {
    if (AstNode *ec = e->current()) {
      return Unify(ec, t, cut, f);
    }
    if (t->AsEVar() == e) {
      return f();
    }
    if (Occurs(e, t)) {
      FileHandlePrettyPrinter printer(stderr);
      printer.Print("Detected a cycle involving ");
      e->Dump(*context_.symbol_table(), &printer);
      printer.Print(" while unifying it with ");
      t->Dump(*context_.symbol_table(), &printer);
      printer.Print(".\n");
      return kInvalidProgram;
    }
    e->set_current(t);
    ThunkRet f_ret = f();
    if (f_ret != cut) {
      e->set_current(nullptr);
    }
    return f_ret;
  }

  ThunkRet MatchAtomVersusDatabase(AstNode *atom, ThunkRet cut, Thunk f) {
    if (auto *app = atom->AsApp()) {
      if (app->lhs() == context_.fact_id()) {
        if (auto *tuple = app->rhs()->AsTuple()) {
          if (tuple->size() == 5) {
            AtomFactKey key(context_.vname_id(), tuple);
            // Make use of the fast lookup sort order.
            auto begin = std::lower_bound(database_.begin(), database_.end(),
                                          &key, FastLookupFactLessThanKey);
            auto end = std::upper_bound(database_.begin(), database_.end(),
                                        &key, FastLookupKeyLessThanFact);
            for (auto i = begin; i != end; ++i) {
              ThunkRet exc = Unify(atom, *i, cut, f);
              if (exc != kNoException) {
                return exc;
              }
            }
            return kNoException;
          }
        }
      }
    }
    // Not enough information to filter by.
    for (size_t fact = 0; fact < database_.size(); ++fact) {
      ThunkRet exc = Unify(atom, database_[fact], cut, f);
      if (exc != kNoException) {
        return exc;
      }
    }
    return kNoException;
  }

  /// \brief If `atom` has the syntactic form =(a, b), returns the tuple (a, b).
  /// Otherwise returns `null`.
  Tuple *MatchEqualsArgs(AstNode *atom) {
    if (App *a = atom->AsApp()) {
      if (Identifier *id = a->lhs()->AsIdentifier()) {
        if (id->symbol() == context_.eq_id()->symbol()) {
          if (Tuple *tu = a->rhs()->AsTuple()) {
            if (tu->size() == 2) {
              return tu;
            }
          }
        }
      }
    }
    return nullptr;
  }

  ThunkRet MatchAtom(AstNode *atom, AstNode *program, ThunkRet cut, Thunk f) {
    // We only have the database and eq-constraints right now.
    assert(program == nullptr);
    if (auto *tu = MatchEqualsArgs(atom)) {
      if (Range *r = tu->element(0)->AsRange()) {
        auto anchors =
            anchors_.equal_range(std::make_pair(r->begin(), r->end()));
        if (anchors.first == anchors.second) {
          // There's no anchor with this range in the database.
          // This goal can therefore never succeed.
          return kImpossible;
        }
        for (auto anchor = anchors.first; anchor != anchors.second; ++anchor) {
          ThunkRet unify_ret = Unify(anchor->second, tu->element(1), cut, f);
          if (unify_ret != kNoException) {
            return unify_ret;
          }
        }
        return kNoException;
      }
      // =(a, b) succeeds if unify(a, b) succeeds.
      return Unify(tu->element(0), tu->element(1), cut, f);
    }
    return MatchAtomVersusDatabase(atom, cut, f);
  }

  ThunkRet SolveGoal(AstNode *goal, ThunkRet cut, Thunk f) {
    // We only have atomic goals right now.
    if (App *a = goal->AsApp()) {
      return MatchAtom(goal, nullptr, cut, f);
    } else {
      // TODO(zarko): Replace with a configurable PrettyPrinter.
      LOG(ERROR) << "Invalid AstNode in goal-expression.";
      return kInvalidProgram;
    }
  }

  ThunkRet SolveGoalArray(AssertionParser::GoalGroup *group, size_t cur,
                          ThunkRet cut, Thunk f) {
    if (cur > highest_goal_reached_) {
      highest_goal_reached_ = cur;
    }
    if (cur == group->goals.size()) {
      return f();
    }
    return SolveGoal(group->goals[cur], cut, [this, group, cur, cut, &f]() {
      return SolveGoalArray(group, cur + 1, cut, f);
    });
  }

  bool PerformInspection() {
    for (const auto &inspection : context_.parser()->inspections()) {
      if (!inspect_(&context_, inspection)) {
        return false;
      }
    }
    return true;
  }

  ThunkRet SolveGoalGroups(AssertionParser *context, Thunk f) {
    for (size_t cur = 0, cut = kFirstCut; cur < context->groups().size();
         ++cur, ++cut) {
      auto *group = &context->groups()[cur];
      if (cur > highest_group_reached_) {
        highest_goal_reached_ = 0;
        highest_group_reached_ = cur;
      }
      ThunkRet result = SolveGoalArray(
          group, 0, cut,
          [this, context, group, cur, cut, &f]() { return cut; });
      // Lots of unwinding later...
      if (result == cut) {
        // That last goal group succeeded.
        if (group->accept_if != AssertionParser::GoalGroup::kNoneMayFail) {
          return PerformInspection() ? kNoException : kInvalidProgram;
        }
      } else if (result == kNoException || result == kImpossible) {
        // That last goal group failed.
        if (group->accept_if != AssertionParser::GoalGroup::kSomeMustFail) {
          return PerformInspection() ? kNoException : kInvalidProgram;
        }
      } else {
        return result;
      }
    }
    return PerformInspection() ? f() : kInvalidProgram;
  }

  bool Solve() {
    ThunkRet exn = SolveGoalGroups(context_.parser(), []() { return kSolved; });
    return exn == kSolved;
  }

  size_t highest_group_reached() const { return highest_group_reached_; }

  size_t highest_goal_reached() const { return highest_goal_reached_; }

 private:
  Verifier &context_;
  Database &database_;
  std::multimap<std::pair<size_t, size_t>, AstNode *> &anchors_;
  std::function<bool(Verifier *, const Inspection &)> &inspect_;
  size_t highest_group_reached_ = 0;
  size_t highest_goal_reached_ = 0;
};
}  // anonymous namespace

Verifier::Verifier(bool trace_lex, bool trace_parse)
    : parser_(this, trace_lex, trace_parse),
      builtin_location_name_("builtins") {
  builtin_location_.initialize(&builtin_location_name_);
  empty_string_id_ = IdentifierFor(builtin_location_, "");
  fact_id_ = IdentifierFor(builtin_location_, "fact");
  vname_id_ = IdentifierFor(builtin_location_, "vname");
  kind_id_ = IdentifierFor(builtin_location_, "/kythe/node/kind");
  anchor_id_ = IdentifierFor(builtin_location_, "anchor");
  start_id_ = IdentifierFor(builtin_location_, "/kythe/loc/start");
  end_id_ = IdentifierFor(builtin_location_, "/kythe/loc/end");
  root_id_ = IdentifierFor(builtin_location_, "/");
  eq_id_ = IdentifierFor(builtin_location_, "=");
  ordinal_id_ = IdentifierFor(builtin_location_, "/kythe/ordinal");
}

bool Verifier::LoadInlineProtoFile(const std::string &file_data) {
  kythe::proto::Entries entries;
  bool ok = google::protobuf::TextFormat::ParseFromString(file_data, &entries);
  if (!ok) {
    // TODO(zarko): Replace with a configurable PrettyPrinter.
    LOG(ERROR) << "Unable to parse text protobuf.";
    return false;
  }
  for (int i = 0; i < entries.entries_size(); ++i) {
    AssertSingleFact(kDefaultDatabase, i, entries.entries(i));
  }
  bool parsed = parser_.ParseInlineRuleString(file_data, *kStandardIn, "#-");
  if (!parsed) {
    return false;
  }
  return true;
}

bool Verifier::LoadInlineRuleFile(const std::string &filename) {
  // TODO(zarko): figure out comment prefix from file extension
  bool parsed =
      parser_.ParseInlineRuleFile(filename, goal_comment_marker_.c_str());
  if (!parsed) {
    return false;
  }
  return true;
}

void Verifier::IgnoreDuplicateFacts() { ignore_dups_ = true; }

void Verifier::SaveEVarAssignments() {
  saving_assignments_ = true;
  parser_.InspectAllEVars();
}

void Verifier::ShowGoals() {
  FileHandlePrettyPrinter printer(stdout);
  for (auto &group : parser_.groups()) {
    if (group.accept_if == AssertionParser::GoalGroup::kNoneMayFail) {
      printer.Print("group:\n");
    } else {
      printer.Print("negated group:\n");
    }
    for (auto *goal : group.goals) {
      printer.Print("  goal: ");
      goal->Dump(symbol_table_, &printer);
      printer.Print("\n");
    }
  }
}

static bool PrintFileSection(FILE *file, size_t start_line, size_t start_ix,
                             size_t end_line, size_t end_ix,
                             PrettyPrinter *printer) {
  if (!file) {
    printer->Print("(null file)\n");
    return false;
  }
  char *lineptr = nullptr;
  size_t buf_length = 0;
  ssize_t line_length = 0;
  size_t line_number = 0;
  while ((line_length = getline(&lineptr, &buf_length, file)) != -1) {
    if (line_number >= start_line && line_number <= end_line) {
      std::string text(lineptr);
      size_t line_begin = 0, line_end = text.size();
      if (line_number == start_line) {
        line_begin = start_ix;
      }
      if (line_number == end_line) {
        line_end = end_ix;
      }
      if (line_end - line_begin > text.size()) {
        printer->Print("(error line too big for actual line)\n");
      } else {
        text = text.substr(line_begin, line_end - line_begin);
        printer->Print(text);
      }
    }
    if (line_number == end_line) {
      free(lineptr);
      return true;
    }
    ++line_number;
  }
  printer->Print("(error line out of bounds)\n");
  free(lineptr);
  return false;
}

void Verifier::DumpErrorGoal(size_t group, size_t index) {
  FileHandlePrettyPrinter printer(stderr);
  if (group >= parser_.groups().size()) {
    printer.Print("(invalid group index ");
    printer.Print(std::to_string(group));
    printer.Print(")\n");
  }
  if (index >= parser_.groups()[group].goals.size()) {
    if (index > parser_.groups()[group].goals.size() ||
        parser_.groups()[group].goals.empty()) {
      printer.Print("(invalid index ");
      printer.Print(std::to_string(group));
      printer.Print(":");
      printer.Print(std::to_string(index));
      printer.Print(")\n");
      return;
    }
    printer.Print("(past the end of a ");
    if (parser_.groups()[group].accept_if ==
        AssertionParser::GoalGroup::kSomeMustFail) {
      printer.Print("negated ");
    }
    printer.Print("group, whose last goal was)\n  ");
    --index;
  }
  auto *goal = parser_.groups()[group].goals[index];
  yy::location goal_location = goal->location();
  yy::position goal_begin = goal_location.begin;
  yy::position goal_end = goal_location.end;
  if (goal_end.filename) {
    printer.Print(*goal_end.filename);
  } else {
    printer.Print("-");
  }
  printer.Print(":");
  if (goal_begin.filename) {
    printer.Print(std::to_string(goal_begin.line + 1) + ":" +
                  std::to_string(goal_begin.column));
  }
  printer.Print("-");
  if (goal_end.filename) {
    printer.Print(std::to_string(goal_end.line + 1) + ":" +
                  std::to_string(goal_end.column));
  }
  bool printed_goal = false;
  printer.Print(" ");
  if (goal_end.filename && *goal_end.filename != *kStandardIn &&
      *goal_begin.filename == *goal_end.filename) {
    FILE *f = fopen(goal_end.filename->c_str(), "r");
    printed_goal =
        PrintFileSection(f, goal_begin.line - 1, goal_begin.column - 1,
                         goal_end.line - 1, goal_end.column - 1, &printer);
    fclose(f);
  }
  if (!printed_goal) {
    goal->Dump(symbol_table_, &printer);
  }
  printer.Print("\n");
}

bool Verifier::VerifyAllGoals(
    std::function<bool(Verifier *, const Solver::Inspection &)> inspect) {
  if (!PrepareDatabase()) {
    return false;
  }
  Solver solver(this, facts_, anchors_, inspect);
  bool result = solver.Solve();
  highest_goal_reached_ = solver.highest_goal_reached();
  highest_group_reached_ = solver.highest_group_reached();
  return result;
}

bool Verifier::VerifyAllGoals() {
  return VerifyAllGoals([this](Verifier *context,
                               const Solver::Inspection &inspection) {
    if (inspection.kind == Solver::Inspection::Kind::EXPLICIT) {
      FileHandlePrettyPrinter printer(saving_assignments_ ? stderr : stdout);
      printer.Print(inspection.label);
      printer.Print(": ");
      inspection.evar->Dump(symbol_table_, &printer);
      printer.Print("\n");
    }
    if (inspection.evar->current()) {
      saved_assignments_[inspection.label] = inspection.evar->current();
    }
    return true;
  });
}

Identifier *Verifier::IdentifierFor(const yy::location &location,
                                    const std::string &token) {
  Symbol symbol = symbol_table_.intern(token);
  return new (&arena_) Identifier(location, symbol);
}

Identifier *Verifier::IdentifierFor(const yy::location &location, int integer) {
  Symbol symbol = symbol_table_.intern(std::to_string(integer));
  return new (&arena_) Identifier(location, symbol);
}

AstNode *Verifier::MakePredicate(const yy::location &location, AstNode *head,
                                 std::initializer_list<AstNode *> values) {
  size_t values_count = values.size();
  AstNode **body = (AstNode **)arena_.New(values_count * sizeof(AstNode *));
  size_t vn = 0;
  for (AstNode *v : values) {
    body[vn] = v;
    ++vn;
  }
  AstNode *tuple = new (&arena_) Tuple(location, values_count, body);
  return new (&arena_) App(location, head, tuple);
}

/// \brief Sort nodes such that nodes and facts are grouped.
static bool GraphvizSortOrder(AstNode *a, AstNode *b) {
  Tuple *ta = a->AsApp()->rhs()->AsTuple();
  Tuple *tb = b->AsApp()->rhs()->AsTuple();
  if (EncodedVNameOrIdentLessThan(ta->element(0), tb->element(0))) {
    return true;
  }
  if (!EncodedVNameOrIdentEqualTo(ta->element(0), tb->element(0))) {
    return false;
  }
  if (EncodedIdentLessThan(ta->element(1), tb->element(1))) {
    return true;
  }
  if (!EncodedIdentEqualTo(ta->element(1), tb->element(1))) {
    return false;
  }
  if (EncodedVNameOrIdentLessThan(ta->element(2), tb->element(2))) {
    return true;
  }
  if (!EncodedVNameOrIdentEqualTo(ta->element(2), tb->element(2))) {
    return false;
  }
  if (EncodedIdentLessThan(ta->element(3), tb->element(3))) {
    return true;
  }
  if (!EncodedIdentEqualTo(ta->element(3), tb->element(3))) {
    return false;
  }
  if (EncodedIdentLessThan(ta->element(4), tb->element(4))) {
    return true;
  }
  return false;
}

static bool EncodedFactEqualTo(AstNode *a, AstNode *b) {
  Tuple *ta = a->AsApp()->rhs()->AsTuple();
  Tuple *tb = b->AsApp()->rhs()->AsTuple();
  return EncodedVNameOrIdentEqualTo(ta->element(0), tb->element(0)) &&
         EncodedIdentEqualTo(ta->element(1), tb->element(1)) &&
         EncodedVNameOrIdentEqualTo(ta->element(2), tb->element(2)) &&
         EncodedIdentEqualTo(ta->element(3), tb->element(3)) &&
         EncodedIdentEqualTo(ta->element(4), tb->element(4));
}

static bool EncodedVNameHasValidForm(Verifier *cxt, AstNode *a) {
  Tuple *ta = a->AsApp()->rhs()->AsTuple();
  return ta->element(0) != cxt->empty_string_id() ||
         ta->element(1) != cxt->empty_string_id() ||
         ta->element(2) != cxt->empty_string_id() ||
         ta->element(3) != cxt->empty_string_id() ||
         ta->element(4) != cxt->empty_string_id();
}

static bool EncodedFactHasValidForm(Verifier *cxt, AstNode *a) {
  Tuple *ta = a->AsApp()->rhs()->AsTuple();
  if (ta->element(0) == cxt->empty_string_id() ||
      !EncodedVNameHasValidForm(cxt, ta->element(0))) {
    // Always need a source.
    return false;
  }
  if (ta->element(1) == cxt->empty_string_id()) {
    // (source, "", "", string, _)
    return ta->element(2) == cxt->empty_string_id() &&
           ta->element(3) != cxt->empty_string_id();
  } else {
    // (source, edge, target, ...
    if (ta->element(2) == cxt->empty_string_id() ||
        !EncodedVNameHasValidForm(cxt, ta->element(2))) {
      return false;
    }
    if (EncodedIdentEqualTo(ta->element(3), cxt->root_id())) {
      // ... /, )
      return EncodedIdentEqualTo(ta->element(4), cxt->empty_string_id());
    } else {
      // ... /kythe/ordinal, base10string )
      if (!EncodedIdentEqualTo(ta->element(3), cxt->ordinal_id())) {
        return false;
      }
      const std::string &ordinal_val =
          cxt->symbol_table()->text(ta->element(4)->AsIdentifier()->symbol());
      // TODO: check if valid int
      return true;
    }
  }
}

bool Verifier::PrepareDatabase() {
  if (database_prepared_) {
    return true;
  }
  // TODO(zarko): Make this configurable.
  FileHandlePrettyPrinter printer(stderr);
  // First, sort the tuples. As an invariant, we know they will be of the form
  // fact (vname | ident, ident, vname | ident, ident, ident)
  // vname (ident, ident, ident, ident, ident)
  // and all idents will have been uniqued (so we can compare them purely
  // by symbol ID).
  std::sort(facts_.begin(), facts_.end(), EncodedFactLessThan);
  // Now we can do a simple pairwise check on each of the facts to see
  // whether the invariants hold.
  bool is_ok = true;
  AstNode *last_anchor_vname = nullptr;
  size_t last_anchor_start = ~0;
  for (size_t f = 0; f < facts_.size(); ++f) {
    AstNode *fb = facts_[f];

    if (!EncodedFactHasValidForm(this, fb)) {
      printer.Print("Fact has invalid form:\n  ");
      fb->Dump(symbol_table_, &printer);
      printer.Print("\n");
      is_ok = false;
      continue;
    }
    Tuple *tb = fb->AsApp()->rhs()->AsTuple();
    if (tb->element(1) == empty_string_id_ &&
        tb->element(2) == empty_string_id_) {
      // Check to see if this fact entry describes part of an anchor.
      // We've arranged via EncodedFactLessThan to sort kind_id_ before
      // start_id_ and start_id_ before end_id_ and to group all node facts
      // together in uninterrupted runs.
      if (EncodedIdentEqualTo(tb->element(3), kind_id_) &&
          EncodedIdentEqualTo(tb->element(4), anchor_id_)) {
        // Start tracking a new anchor.
        last_anchor_vname = tb->element(0);
        last_anchor_start = ~0;
      } else if (last_anchor_vname != nullptr &&
                 EncodedIdentEqualTo(tb->element(3), start_id_) &&
                 tb->element(4)->AsIdentifier()) {
        if (EncodedVNameOrIdentEqualTo(last_anchor_vname, tb->element(0))) {
          // This is a fact about the anchor we're tracking.
          std::stringstream(
              symbol_table_.text(tb->element(4)->AsIdentifier()->symbol())) >>
              last_anchor_start;
        } else {
          // This is a fact about node we're not tracking; given our sort order,
          // we'll never get enough information for the node we are tracking,
          // so stop tracking it.
          last_anchor_vname = nullptr;
          last_anchor_start = ~0;
        }
      } else if (last_anchor_start != ~0 &&
                 EncodedIdentEqualTo(tb->element(3), end_id_) &&
                 tb->element(4)->AsIdentifier()) {
        if (EncodedVNameOrIdentEqualTo(last_anchor_vname, tb->element(0))) {
          // We have enough information about the anchor we're tracking.
          size_t last_anchor_end = ~0;
          std::stringstream(
              symbol_table_.text(tb->element(4)->AsIdentifier()->symbol())) >>
              last_anchor_end;
          AddAnchor(last_anchor_vname, last_anchor_start, last_anchor_end);
        }
        last_anchor_vname = nullptr;
        last_anchor_start = ~0;
      }
    }
    if (f == 0) {
      continue;
    }

    AstNode *fa = facts_[f - 1];
    if (!ignore_dups_ && EncodedFactEqualTo(fa, fb)) {
      printer.Print("Two facts were equal:\n  ");
      fa->Dump(symbol_table_, &printer);
      printer.Print("\n  ");
      fb->Dump(symbol_table_, &printer);
      printer.Print("\n");
      is_ok = false;
      continue;
    }
    Tuple *ta = fa->AsApp()->rhs()->AsTuple();
    if (EncodedVNameEqualTo(ta->element(0)->AsApp(), tb->element(0)->AsApp()) &&
        ta->element(1) == empty_string_id_ &&
        tb->element(1) == empty_string_id_ &&
        ta->element(2) == empty_string_id_ &&
        tb->element(2) == empty_string_id_ &&
        EncodedIdentEqualTo(ta->element(3), tb->element(3)) &&
        !EncodedIdentEqualTo(ta->element(4), tb->element(4))) {
      printer.Print("Two facts about a node differed in value:\n  ");
      fa->Dump(symbol_table_, &printer);
      printer.Print("\n  ");
      fb->Dump(symbol_table_, &printer);
      printer.Print("\n");
      is_ok = false;
    }
  }
  if (is_ok) {
    std::sort(facts_.begin(), facts_.end(), FastLookupFactLessThan);
  }
  database_prepared_ = is_ok;
  return is_ok;
}

AstNode *Verifier::ConvertVName(const yy::location &loc,
                                const kythe::proto::VName &vname) {
  AstNode **values = (AstNode **)arena_.New(sizeof(AstNode *) * 5);
  values[0] = vname.signature().empty() ? empty_string_id_
                                        : IdentifierFor(loc, vname.signature());
  values[1] = vname.corpus().empty() ? empty_string_id_
                                     : IdentifierFor(loc, vname.corpus());
  values[2] = vname.root().empty() ? empty_string_id_
                                   : IdentifierFor(loc, vname.root());
  values[3] = vname.path().empty() ? empty_string_id_
                                   : IdentifierFor(loc, vname.path());
  values[4] = vname.language().empty() ? empty_string_id_
                                       : IdentifierFor(loc, vname.language());
  AstNode *tuple = new (&arena_) Tuple(loc, 5, values);
  return new (&arena_) App(vname_id_, tuple);
}

void Verifier::AssertSingleFact(std::string *database, unsigned int fact_id,
                                const kythe::proto::Entry &entry) {
  yy::location loc;
  loc.initialize(database);
  loc.begin.line = fact_id;
  loc.end.line = fact_id;
  AstNode **values = (AstNode **)arena_.New(sizeof(AstNode *) * 5);
  values[0] =
      entry.has_source() ? ConvertVName(loc, entry.source()) : empty_string_id_;
  // We're removing support for ordinal facts. Support them during the
  // transition, but also support the new dot-separated edge kinds that serve
  // the same purpose.
  auto dot_pos = entry.edge_kind().rfind('.');
  if (dot_pos != std::string::npos && dot_pos > 0 &&
      dot_pos < entry.edge_kind().size() - 1) {
    values[1] = IdentifierFor(loc, entry.edge_kind().substr(0, dot_pos));
    values[3] = ordinal_id_;
    values[4] = IdentifierFor(loc, entry.edge_kind().substr(dot_pos + 1));
  } else {
    values[1] = entry.edge_kind().empty()
                    ? empty_string_id_
                    : IdentifierFor(loc, entry.edge_kind());
    values[3] = entry.fact_name().empty()
                    ? empty_string_id_
                    : IdentifierFor(loc, entry.fact_name());
    values[4] = entry.fact_value().empty()
                    ? empty_string_id_
                    : IdentifierFor(loc, entry.fact_value());
  }
  values[2] =
      entry.has_target() ? ConvertVName(loc, entry.target()) : empty_string_id_;

  AstNode *tuple = new (&arena_) Tuple(loc, 5, values);
  AstNode *fact = new (&arena_) App(fact_id_, tuple);

  database_prepared_ = false;
  facts_.push_back(fact);
}

void Verifier::DumpAsJson() {
  if (!PrepareDatabase()) {
    return;
  }
  // Use the same sort order as we do with Graphviz.
  std::sort(facts_.begin(), facts_.end(), GraphvizSortOrder);
  FileHandlePrettyPrinter printer(stdout);
  QuoteEscapingPrettyPrinter escaping_printer(printer);
  FileHandlePrettyPrinter dprinter(stderr);
  auto DumpAsJson = [this, &printer, &escaping_printer](const char *label,
                                                        AstNode *node) {
    printer.Print(label);
    if (node == empty_string_id()) {
      // Canonicalize "" as null in the JSON output.
      printer.Print("null");
    } else {
      printer.Print("\"");
      node->Dump(symbol_table_, &escaping_printer);
      printer.Print("\"");
    }
  };
  auto DumpVName = [this, &printer, &DumpAsJson](const char *label,
                                                 AstNode *node) {
    printer.Print(label);
    if (node == empty_string_id()) {
      printer.Print("null");
    } else {
      Tuple *vname = node->AsApp()->rhs()->AsTuple();
      printer.Print("{");
      DumpAsJson("\"signature\":", vname->element(0));
      DumpAsJson(",\"corpus\":", vname->element(1));
      DumpAsJson(",\"root\":", vname->element(2));
      DumpAsJson(",\"path\":", vname->element(3));
      DumpAsJson(",\"language\":", vname->element(4));
      printer.Print("}");
    }
  };
  printer.Print("[");
  for (size_t i = 0; i < facts_.size(); ++i) {
    AstNode *fact = facts_[i];
    Tuple *t = fact->AsApp()->rhs()->AsTuple();
    printer.Print("{");
    DumpVName("\"source\":", t->element(0));
    DumpAsJson(",\"edge_kind\":", t->element(1));
    DumpVName(",\"target\":", t->element(2));
    DumpAsJson(",\"fact_name\":", t->element(3));
    DumpAsJson(",\"fact_value\":", t->element(4));
    printer.Print(i + 1 == facts_.size() ? "}" : "},");
  }
  printer.Print("]\n");
}

void Verifier::DumpAsDot() {
  if (!PrepareDatabase()) {
    return;
  }
  std::map<std::string, std::string> vname_labels;
  for (const auto &label_vname : saved_assignments_) {
    if (!label_vname.second) {
      continue;
    }
    if (App *a = label_vname.second->AsApp()) {
      if (Tuple *t = a->rhs()->AsTuple()) {
        StringPrettyPrinter printer;
        QuoteEscapingPrettyPrinter quote_printer(printer);
        label_vname.second->Dump(symbol_table_, &printer);
        auto old_label = vname_labels.find(printer.str());
        if (old_label == vname_labels.end()) {
          vname_labels[printer.str()] = label_vname.first;
        } else {
          old_label->second += ", " + label_vname.first;
        }
      }
    }
  }
  auto GetLabel = [&](AstNode *node) {
    if (!node) {
      return std::string();
    }
    StringPrettyPrinter id_string;
    QuoteEscapingPrettyPrinter id_quote(id_string);
    node->Dump(symbol_table_, &id_string);
    const auto &label = vname_labels.find(id_string.str());
    if (label != vname_labels.end()) {
      return label->second;
    } else {
      return std::string();
    }
  };
  AstNode *kind_id = IdentifierFor(builtin_location_, "/kythe/node/kind");
  AstNode *anchor_id = IdentifierFor(builtin_location_, "anchor");
  AstNode *file_id = IdentifierFor(builtin_location_, "file");
  AstNode *text_id = IdentifierFor(builtin_location_, "/kythe/text");
  std::sort(facts_.begin(), facts_.end(), GraphvizSortOrder);
  FileHandlePrettyPrinter printer(stdout);
  QuoteEscapingPrettyPrinter quote_printer(printer);
  HtmlEscapingPrettyPrinter html_printer(printer);
  FileHandlePrettyPrinter dprinter(stderr);
  printer.Print("digraph G {\n");
  for (size_t i = 0; i < facts_.size(); ++i) {
    AstNode *fact = facts_[i];
    Tuple *t = fact->AsApp()->rhs()->AsTuple();
    printer.Print("\"");
    t->element(0)->Dump(symbol_table_, &quote_printer);
    printer.Print("\"");
    if (t->element(1) == empty_string_id()) {
      std::string label = GetLabel(t->element(0));
      // Node. We sorted these above st all the facts should come subsequent.
      // Figure out if the node is an anchor.
      bool is_anchor_node = false;
      bool is_file_node = false;
      size_t first_fact = i, last_fact = facts_.size();
      for (; i < facts_.size(); ++i) {
        Tuple *nt = facts_[i]->AsApp()->rhs()->AsTuple();
        if (!EncodedVNameOrIdentEqualTo(nt->element(0), t->element(0)) ||
            nt->element(1) != empty_string_id()) {
          // Moved past the fact block or moved to a different source node.
          last_fact = i;
          break;
        }
        if (EncodedIdentEqualTo(nt->element(3), kind_id)) {
          if (EncodedIdentEqualTo(nt->element(4), anchor_id)) {
            // Keep on scanning to find the end of the fact block.
            is_anchor_node = true;
          } else if (EncodedIdentEqualTo(nt->element(4), file_id)) {
            is_file_node = true;
          }
        }
      }
      if (is_anchor_node) {
        printer.Print(" [ shape=circle, label=\"@");
        printer.Print(label);
        if (!label.empty()) {
          printer.Print("\", color=\"blue");
        }
        printer.Print("\" ];\n");
      } else {
        printer.Print(" [ label=<<TABLE>");
        printer.Print("<TR><TD COLSPAN=\"2\">");
        Tuple *nt = facts_[first_fact]->AsApp()->rhs()->AsTuple();
        // Since all of our facts are well-formed, we know this is a vname.
        nt->element(0)->AsApp()->rhs()->Dump(symbol_table_, &html_printer);
        if (!label.empty()) {
          html_printer.Print(" = ");
          html_printer.Print(label);
        }
        printer.Print("</TD></TR>");
        for (i = first_fact; i < last_fact; ++i) {
          Tuple *nt = facts_[i]->AsApp()->rhs()->AsTuple();
          printer.Print("<TR><TD>");
          nt->element(3)->Dump(symbol_table_, &html_printer);
          printer.Print("</TD><TD>");
          if (is_file_node && EncodedIdentEqualTo(nt->element(3), text_id)) {
            // Don't clutter the graph with file content.
            printer.Print("...");
          } else {
            nt->element(4)->Dump(symbol_table_, &html_printer);
          }
          printer.Print("</TD></TR>");
        }
        printer.Print("</TABLE>> shape=plaintext ");
        if (!label.empty()) {
          printer.Print(" color=blue ");
        }
        printer.Print("];\n");
      }
      --i;  // Don't skip the fact following the block.
    } else {
      // Edge.
      printer.Print(" -> \"");
      t->element(2)->Dump(symbol_table_, &quote_printer);
      printer.Print("\" [ label=\"");
      t->element(1)->Dump(symbol_table_, &quote_printer);
      if (t->element(4) != empty_string_id()) {
        printer.Print(".");
        t->element(4)->Dump(symbol_table_, &quote_printer);
      }
      printer.Print("\" ];\n");
    }
  }
  printer.Print("}\n");
}

}  // namespace verifier
}  // namespace kythe
