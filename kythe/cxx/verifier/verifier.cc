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

#include "verifier.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <memory>
#include <optional>
#include <string_view>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/strings/strip.h"
#include "assertions.h"
#include "google/protobuf/text_format.h"
#include "google/protobuf/util/json_util.h"
#include "kythe/cxx/common/kythe_uri.h"
#include "kythe/cxx/common/scope_guard.h"
#include "kythe/cxx/verifier/souffle_interpreter.h"
#include "kythe/proto/common.pb.h"
#include "kythe/proto/storage.pb.h"

namespace kythe {
namespace verifier {
namespace {

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

typedef const std::function<ThunkRet()>& Thunk;

static std::string* kDefaultDatabase = new std::string("builtin");
static std::string* kStandardIn = new std::string("-");

static bool EncodedIdentEqualTo(AstNode* a, AstNode* b) {
  Identifier* ia = a->AsIdentifier();
  Identifier* ib = b->AsIdentifier();
  return ia->symbol() == ib->symbol();
}

static bool EncodedIdentLessThan(AstNode* a, AstNode* b) {
  Identifier* ia = a->AsIdentifier();
  Identifier* ib = b->AsIdentifier();
  return ia->symbol() < ib->symbol();
}

static bool EncodedVNameEqualTo(App* a, App* b) {
  Tuple* ta = a->rhs()->AsTuple();
  Tuple* tb = b->rhs()->AsTuple();
  for (int i = 0; i < 5; ++i) {
    if (!EncodedIdentEqualTo(ta->element(i), tb->element(i))) {
      return false;
    }
  }
  return true;
}

static bool EncodedVNameLessThan(App* a, App* b) {
  Tuple* ta = a->rhs()->AsTuple();
  Tuple* tb = b->rhs()->AsTuple();
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

static bool EncodedVNameOrIdentLessThan(AstNode* a, AstNode* b) {
  App* aa = a->AsApp();  // nullptr if a is not a vname
  App* ab = b->AsApp();  // nullptr if b is not a vname
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

static bool EncodedVNameOrIdentEqualTo(AstNode* a, AstNode* b) {
  App* aa = a->AsApp();  // nullptr if a is not a vname
  App* ab = b->AsApp();  // nullptr if b is not a vname
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
static bool EncodedFactLessThan(AstNode* a, AstNode* b) {
  Tuple* ta = a->AsApp()->rhs()->AsTuple();
  Tuple* tb = b->AsApp()->rhs()->AsTuple();
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

static AstNode* DerefEVar(AstNode* node) {
  while (node) {
    if (auto* evar = node->AsEVar()) {
      node = evar->current();
    } else {
      break;
    }
  }
  return node;
}

static Identifier* SafeAsIdentifier(AstNode* node) {
  return node == nullptr ? nullptr : node->AsIdentifier();
}

struct AtomFactKey {
  Identifier* edge_kind;
  Identifier* fact_name;
  Identifier* fact_value;
  Identifier* source_vname[5] = {nullptr, nullptr, nullptr, nullptr, nullptr};
  Identifier* target_vname[5] = {nullptr, nullptr, nullptr, nullptr, nullptr};
  // fact_tuple is expected to be a full tuple from a Fact head
  AtomFactKey(AstNode* vname_head, Tuple* fact_tuple)
      : edge_kind(SafeAsIdentifier(DerefEVar(fact_tuple->element(1)))),
        fact_name(SafeAsIdentifier(DerefEVar(fact_tuple->element(3)))),
        fact_value(SafeAsIdentifier(DerefEVar(fact_tuple->element(4)))) {
    InitVNameFields(vname_head, fact_tuple->element(0), &source_vname[0]);
    InitVNameFields(vname_head, fact_tuple->element(2), &target_vname[0]);
  }
  void InitVNameFields(AstNode* vname_head, AstNode* maybe_vname,
                       Identifier** out) {
    maybe_vname = DerefEVar(maybe_vname);
    if (maybe_vname == nullptr) {
      return;
    }
    if (auto* app = maybe_vname->AsApp()) {
      if (DerefEVar(app->lhs()) != vname_head) {
        return;
      }
      AstNode* maybe_tuple = DerefEVar(app->rhs());
      if (maybe_tuple == nullptr) {
        return;
      }
      if (auto* tuple = maybe_tuple->AsTuple()) {
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

enum class Order { LT, EQ, GT };

// How we order incomplete keys depends on whether we're looking for
// an upper or lower bound. See below for details. The node passed in
// must be an application of Fact to a full fact tuple.
static Order CompareFactWithKey(Order incomplete, AstNode* a, AtomFactKey* k) {
  Tuple* ta = a->AsApp()->rhs()->AsTuple();
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
  auto vname_compare = [incomplete](Tuple* va, Identifier* tuple[5]) {
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
  if (Tuple* vs = ta->element(0)->AsApp()->rhs()->AsTuple()) {
    auto ord = vname_compare(vs, k->source_vname);
    if (ord != Order::EQ) {
      return ord;
    }
  }
  if (auto* app = ta->element(2)->AsApp()) {
    if (Tuple* vt = app->rhs()->AsTuple()) {
      auto ord = vname_compare(vt, k->target_vname);
      if (ord != Order::EQ) {
        return ord;
      }
    }
  }
  return Order::EQ;
}

// We want to be able to find the following bounds:
// (0,0,2,3) (0,1,2,3) (0,1,2,4) (1,1,2,4)
//          ^---  (0,1,_,_)  ---^

static bool FastLookupKeyLessThanFact(AtomFactKey* k, AstNode* a) {
  // This is used to find upper bounds, so keys with incomplete suffixes should
  // be ordered after all facts that share their complete prefixes.
  return CompareFactWithKey(Order::LT, a, k) == Order::GT;
}

static bool FastLookupFactLessThanKey(AstNode* a, AtomFactKey* k) {
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
static bool FastLookupFactLessThan(AstNode* a, AstNode* b) {
  Tuple* ta = a->AsApp()->rhs()->AsTuple();
  Tuple* tb = b->AsApp()->rhs()->AsTuple();
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
  Solver(Verifier* context, Database& database, AnchorMap& anchors,
         std::function<bool(Verifier*, const Inspection&)>& inspect)
      : context_(*context),
        database_(database),
        anchors_(anchors),
        inspect_(inspect) {}

  ThunkRet UnifyTuple(Tuple* st, Tuple* tt, size_t ofs, size_t max,
                      ThunkRet cut, Thunk f) {
    if (ofs == max) return f();
    return Unify(st->element(ofs), tt->element(ofs), cut,
                 [this, st, tt, ofs, max, cut, &f]() {
                   return UnifyTuple(st, tt, ofs + 1, max, cut, f);
                 });
  }

  ThunkRet Unify(AstNode* s, AstNode* t, ThunkRet cut, Thunk f) {
    if (EVar* e = s->AsEVar()) {
      return UnifyEVar(e, t, cut, f);
    } else if (EVar* e = t->AsEVar()) {
      return UnifyEVar(e, s, cut, f);
    } else if (Identifier* si = s->AsIdentifier()) {
      if (Identifier* ti = t->AsIdentifier()) {
        if (si->symbol() == ti->symbol()) {
          return f();
        }
      }
    } else if (App* sa = s->AsApp()) {
      if (App* ta = t->AsApp()) {
        return Unify(sa->lhs(), ta->lhs(), cut, [this, sa, ta, cut, &f]() {
          return Unify(sa->rhs(), ta->rhs(), cut, f);
        });
      }
    } else if (Tuple* st = s->AsTuple()) {
      if (Tuple* tt = t->AsTuple()) {
        if (st->size() != tt->size()) {
          return kNoException;
        }
        return UnifyTuple(st, tt, 0, st->size(), cut, f);
      }
    } else if (Range* sr = s->AsRange()) {
      if (Range* tr = t->AsRange()) {
        if (*sr == *tr) {
          return f();
        }
      }
    }
    return kNoException;
  }

  bool Occurs(EVar* e, AstNode* t) {
    if (App* a = t->AsApp()) {
      return Occurs(e, a->lhs()) || Occurs(e, a->rhs());
    } else if (EVar* ev = t->AsEVar()) {
      return ev->current() ? Occurs(e, ev->current()) : e == ev;
    } else if (Tuple* tu = t->AsTuple()) {
      for (size_t i = 0, c = tu->size(); i != c; ++i) {
        if (Occurs(e, tu->element(i))) {
          return true;
        }
      }
      return false;
    } else if (Range* r = t->AsRange()) {
      return false;
    } else {
      CHECK(t->AsIdentifier() && "Inexhaustive match.");
      return false;
    }
    return true;
  }

  ThunkRet UnifyEVar(EVar* e, AstNode* t, ThunkRet cut, Thunk f) {
    if (AstNode* ec = e->current()) {
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

  ThunkRet MatchAtomVersusDatabase(AstNode* atom, ThunkRet cut, Thunk f) {
    if (auto* app = atom->AsApp()) {
      if (app->lhs() == context_.fact_id()) {
        if (auto* tuple = app->rhs()->AsTuple()) {
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
  Tuple* MatchEqualsArgs(AstNode* atom) {
    if (App* a = atom->AsApp()) {
      if (Identifier* id = a->lhs()->AsIdentifier()) {
        if (id->symbol() == context_.eq_id()->symbol()) {
          if (Tuple* tu = a->rhs()->AsTuple()) {
            if (tu->size() == 2) {
              return tu;
            }
          }
        }
      }
    }
    return nullptr;
  }

  ThunkRet MatchAtom(AstNode* atom, AstNode* program, ThunkRet cut, Thunk f) {
    // We only have the database and eq-constraints right now.
    assert(program == nullptr);
    if (auto* tu = MatchEqualsArgs(atom)) {
      if (Range* r = tu->element(0)->AsRange()) {
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

  ThunkRet SolveGoal(AstNode* goal, ThunkRet cut, Thunk f) {
    // We only have atomic goals right now.
    if (App* a = goal->AsApp()) {
      return MatchAtom(goal, nullptr, cut, f);
    } else {
      // TODO(zarko): Replace with a configurable PrettyPrinter.
      LOG(ERROR) << "Invalid AstNode in goal-expression.";
      return kInvalidProgram;
    }
  }

  ThunkRet SolveGoalArray(GoalGroup* group, size_t cur, ThunkRet cut, Thunk f) {
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
    for (const auto& inspection : context_.parser()->inspections()) {
      if (!inspect_(&context_, inspection)) {
        return false;
      }
    }
    return true;
  }

  ThunkRet SolveGoalGroups(AssertionParser* context, Thunk f) {
    for (size_t cur = 0, cut = kFirstCut; cur < context->groups().size();
         ++cur, ++cut) {
      auto* group = &context->groups()[cur];
      if (cur > highest_group_reached_) {
        highest_goal_reached_ = 0;
        highest_group_reached_ = cur;
      }
      ThunkRet result = SolveGoalArray(group, 0, cut, [cut]() { return cut; });
      // Lots of unwinding later...
      if (result == cut) {
        // That last goal group succeeded.
        if (group->accept_if != GoalGroup::kNoneMayFail) {
          return PerformInspection() ? kNoException : kInvalidProgram;
        }
      } else if (result == kNoException || result == kImpossible) {
        // That last goal group failed.
        if (group->accept_if != GoalGroup::kSomeMustFail) {
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
  Verifier& context_;
  Database& database_;
  AnchorMap& anchors_;
  std::function<bool(Verifier*, const Inspection&)>& inspect_;
  size_t highest_group_reached_ = 0;
  size_t highest_goal_reached_ = 0;
};

enum class NodeKind { kFile, kAnchor, kOther };

struct NodeFacts {
  NodeKind kind = NodeKind::kOther;
  absl::Span<AstNode* const> facts;
};

NodeFacts ReadNodeFacts(absl::Span<AstNode* const> entries, Verifier& ctx) {
  NodeFacts result = {
      .kind = NodeKind::kOther,
      .facts = entries,
  };

  if (entries.empty()) {
    return result;
  }

  Tuple* head = entries.front()->AsApp()->rhs()->AsTuple();
  for (size_t i = 0; i < entries.size(); ++i) {
    Tuple* current = entries[i]->AsApp()->rhs()->AsTuple();
    if (!EncodedVNameOrIdentEqualTo(current->element(0), head->element(0)) ||
        current->element(1) != ctx.empty_string_id()) {
      // Moved past the fact block or moved to a different source node;
      // we're done.
      result.facts = entries.subspan(0, i);
      break;
    }
    if (EncodedIdentEqualTo(current->element(3), ctx.kind_id())) {
      if (EncodedIdentEqualTo(current->element(4), ctx.anchor_id())) {
        result.kind = NodeKind::kAnchor;
      } else if (EncodedIdentEqualTo(current->element(4), ctx.file_id())) {
        result.kind = NodeKind::kFile;
      }
    }
  }
  return result;
}
}  // namespace

Verifier::Verifier(bool trace_lex, bool trace_parse)
    : parser_(this, trace_lex, trace_parse),
      builtin_location_name_("builtins") {
  builtin_location_.initialize(&builtin_location_name_);
  builtin_location_.begin.column = 1;
  builtin_location_.end.column = 1;
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
  file_id_ = IdentifierFor(builtin_location_, "file");
  text_id_ = IdentifierFor(builtin_location_, "/kythe/text");
  code_id_ = IdentifierFor(builtin_location_, "/kythe/code");
  code_json_id_ = IdentifierFor(builtin_location_, "/kythe/code/json");
  marked_source_child_id_ =
      IdentifierFor(builtin_location_, "/kythe/edge/child");
  marked_source_box_id_ = IdentifierFor(builtin_location_, "BOX");
  marked_source_type_id_ = IdentifierFor(builtin_location_, "TYPE");
  marked_source_parameter_id_ = IdentifierFor(builtin_location_, "PARAMETER");
  marked_source_identifier_id_ = IdentifierFor(builtin_location_, "IDENTIFIER");
  marked_source_context_id_ = IdentifierFor(builtin_location_, "CONTEXT");
  marked_source_initializer_id_ =
      IdentifierFor(builtin_location_, "INITIALIZER");
  marked_source_modifier_id_ = IdentifierFor(builtin_location_, "MODIFIER");
  marked_source_parameter_lookup_by_param_id_ =
      IdentifierFor(builtin_location_, "PARAMETER_LOOKUP_BY_PARAM");
  marked_source_lookup_by_param_id_ =
      IdentifierFor(builtin_location_, "LOOKUP_BY_PARAM");
  marked_source_parameter_lookup_by_tparam_id_ =
      IdentifierFor(builtin_location_, "PARAMETER_LOOKUP_BY_TPARAM");
  marked_source_lookup_by_tparam_id_ =
      IdentifierFor(builtin_location_, "LOOKUP_BY_TPARAM");
  marked_source_parameter_lookup_by_param_with_defaults_id_ = IdentifierFor(
      builtin_location_, "PARAMETER_LOOKUP_BY_PARAM_WITH_DEFAULTS");
  marked_source_lookup_by_typed_id_ =
      IdentifierFor(builtin_location_, "LOOKUP_BY_TYPED");
  marked_source_kind_id_ = IdentifierFor(builtin_location_, "/kythe/kind");
  marked_source_pre_text_id_ =
      IdentifierFor(builtin_location_, "/kythe/pre_text");
  marked_source_post_child_text_id_ =
      IdentifierFor(builtin_location_, "/kythe/post_child_text");
  marked_source_post_text_id_ =
      IdentifierFor(builtin_location_, "/kythe/post_text");
  marked_source_lookup_index_id_ =
      IdentifierFor(builtin_location_, "/kythe/lookup_index");
  marked_source_default_children_count_id_ =
      IdentifierFor(builtin_location_, "/kythe/default_children_count");
  marked_source_add_final_list_token_id_ =
      IdentifierFor(builtin_location_, "/kythe/add_final_list_token");
  marked_source_link_id_ = IdentifierFor(builtin_location_, "/kythe/edge/link");
  marked_source_true_id_ = IdentifierFor(builtin_location_, "true");
  marked_source_code_edge_id_ =
      IdentifierFor(builtin_location_, "/kythe/edge/code");
  marked_source_false_id_ = IdentifierFor(builtin_location_, "false");
  known_file_sym_ = symbol_table_.unique();
  known_not_file_sym_ = symbol_table_.unique();
  SetGoalCommentPrefix("//-");
}

void Verifier::SetGoalCommentPrefix(const std::string& it) {
  std::string error;
  auto escaped = RE2::QuoteMeta(it);
  CHECK(SetGoalCommentRegex("\\s*" + escaped + "(.*)", &error)) << error;
}

bool Verifier::SetGoalCommentRegex(const std::string& regex,
                                   std::string* error) {
  auto re2 = std::make_unique<RE2>(regex);
  if (re2->error_code() != RE2::NoError) {
    if (error) {
      *error = re2->error();
      return false;
    }
  }
  if (re2->NumberOfCapturingGroups() != 1) {
    if (error) {
      *error = "Wrong number of capture groups in goal comment regex ";
      // This is useful to show, since the shell might unexpectedly shred
      // regexes.
      error->append(regex);
      error->append("(want 1).");
      return false;
    }
  }
  goal_comment_regex_ = std::move(re2);
  return true;
}

bool Verifier::LoadInlineProtoFile(const std::string& file_data,
                                   absl::string_view path,
                                   absl::string_view root,
                                   absl::string_view corpus) {
  kythe::proto::Entries entries;
  bool ok = google::protobuf::TextFormat::ParseFromString(file_data, &entries);
  if (!ok) {
    // TODO(zarko): Replace with a configurable PrettyPrinter.
    LOG(ERROR) << "Unable to parse text protobuf.";
    return false;
  }
  for (int i = 0; i < entries.entries_size(); ++i) {
    if (!AssertSingleFact(kDefaultDatabase, i, entries.entries(i))) {
      return false;
    }
  }
  Symbol empty = symbol_table_.intern("");
  return parser_.ParseInlineRuleString(
      file_data, *kStandardIn, symbol_table_.intern(std::string(path)),
      symbol_table_.intern(std::string(root)),
      symbol_table_.intern(std::string(corpus)), "\\s*\\#\\-(.*)");
}

bool Verifier::LoadInlineRuleFile(const std::string& filename) {
  int fd = ::open(filename.c_str(), 0);
  if (fd < 0) {
    LOG(ERROR) << "Can't open " << filename;
    return false;
  }
  auto guard = MakeScopeGuard([&] { ::close(fd); });
  struct stat fd_stat;
  if (::fstat(fd, &fd_stat) < 0) {
    LOG(ERROR) << "Can't stat " << filename;
    return false;
  }
  std::string content;
  content.resize(fd_stat.st_size);
  if (::read(fd, const_cast<char*>(content.data()), fd_stat.st_size) !=
      fd_stat.st_size) {
    LOG(ERROR) << "Can't read " << filename;
    return false;
  }
  Symbol content_sym = symbol_table_.intern(content);
  if (file_vnames_) {
    auto vname = content_to_vname_.find(content_sym);
    if (vname == content_to_vname_.end()) {
      LOG(ERROR) << "Could not find a file node for " << filename;
      return false;
    }
    return LoadInMemoryRuleFile(filename, vname->second, content_sym);
  } else {
    kythe::proto::VName empty;
    auto* vname = ConvertVName(yy::location{}, empty);
    return LoadInMemoryRuleFile(filename, vname, content_sym);
  }
}

bool Verifier::LoadInMemoryRuleFile(const std::string& filename, AstNode* vname,
                                    Symbol text) {
  Tuple* checked_tuple = nullptr;
  if (auto* app = vname->AsApp()) {
    if (auto* tuple = app->rhs()->AsTuple()) {
      if (tuple->size() == 5 && tuple->element(1)->AsIdentifier() &&
          tuple->element(2)->AsIdentifier() &&
          tuple->element(3)->AsIdentifier()) {
        checked_tuple = tuple;
      }
    }
  }
  if (checked_tuple == nullptr) {
    return false;
  }
  StringPrettyPrinter printer;
  vname->Dump(symbol_table_, &printer);
  fake_files_[printer.str()] = text;
  return parser_.ParseInlineRuleString(
      symbol_table_.text(text), filename.empty() ? printer.str() : filename,
      checked_tuple->element(3)->AsIdentifier()->symbol(),
      checked_tuple->element(2)->AsIdentifier()->symbol(),
      checked_tuple->element(1)->AsIdentifier()->symbol(),
      *goal_comment_regex_);
}

void Verifier::IgnoreDuplicateFacts() { ignore_dups_ = true; }

void Verifier::IgnoreCodeConflicts() { ignore_code_conflicts_ = true; }

void Verifier::SaveEVarAssignments() {
  saving_assignments_ = true;
  parser_.InspectAllEVars();
}

void Verifier::ShowGoals() {
  FileHandlePrettyPrinter printer(stdout);
  for (auto& group : parser_.groups()) {
    if (group.accept_if == GoalGroup::kNoneMayFail) {
      printer.Print("group:\n");
    } else {
      printer.Print("negated group:\n");
    }
    for (auto* goal : group.goals) {
      printer.Print("  goal: ");
      goal->Dump(symbol_table_, &printer);
      printer.Print("\n");
    }
  }
}

static bool PrintInMemoryFileSection(const std::string& file_text,
                                     size_t start_line, size_t start_ix,
                                     size_t end_line, size_t end_ix,
                                     PrettyPrinter* printer) {
  size_t current_line = 0;
  size_t pos = 0;
  auto walk_lines = [&](size_t until_line) {
    if (until_line == current_line) {
      return pos;
    }
    do {
      auto endline = file_text.find('\n', pos);
      if (endline == std::string::npos) {
        return std::string::npos;
      }
      pos = endline + 1;
    } while (++current_line < start_line);
    return pos;
  };
  auto begin = walk_lines(start_line);
  auto end = begin == std::string::npos ? begin : walk_lines(end_line);
  auto begin_ofs = begin + start_ix;
  auto end_ofs = end + end_ix;
  if (begin == std::string::npos || end == std::string::npos ||
      begin_ofs > file_text.size() || end_ofs > file_text.size()) {
    printer->Print("(error line out of bounds)");
    return false;
  }
  printer->Print(file_text.substr(begin_ofs, end_ofs - begin_ofs));
  return true;
}

static bool PrintFileSection(FILE* file, size_t start_line, size_t start_ix,
                             size_t end_line, size_t end_ix,
                             PrettyPrinter* printer) {
  if (!file) {
    printer->Print("(null file)\n");
    return false;
  }
  char* lineptr = nullptr;
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
    if (parser_.groups()[group].accept_if == GoalGroup::kSomeMustFail) {
      printer.Print("negated ");
    }
    printer.Print("group, whose last goal was)\n  ");
    --index;
  }
  auto* goal = parser_.groups()[group].goals[index];
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
    printer.Print(std::to_string(goal_begin.line) + ":" +
                  std::to_string(goal_begin.column));
  }
  printer.Print("-");
  if (goal_end.filename) {
    printer.Print(std::to_string(goal_end.line) + ":" +
                  std::to_string(goal_end.column));
  }
  bool printed_goal = false;
  printer.Print(" ");
  if (goal_end.filename) {
    auto has_symbol = fake_files_.find(*goal_end.filename);
    if (has_symbol != fake_files_.end()) {
      printed_goal = PrintInMemoryFileSection(
          symbol_table_.text(has_symbol->second), goal_begin.line - 1,
          goal_begin.column - 1, goal_end.line - 1, goal_end.column - 1,
          &printer);
    } else if (*goal_end.filename != *kStandardIn &&
               *goal_begin.filename == *goal_end.filename) {
      FILE* f = fopen(goal_end.filename->c_str(), "r");
      if (f != nullptr) {
        printed_goal =
            PrintFileSection(f, goal_begin.line - 1, goal_begin.column - 1,
                             goal_end.line - 1, goal_end.column - 1, &printer);
        fclose(f);
      }
    }
  }
  printer.Print("\n  Goal: ");
  goal->Dump(symbol_table_, &printer);
  printer.Print("\n");
}

bool Verifier::VerifyAllGoals(
    std::function<bool(Verifier*, const Inspection&, std::string_view)>
        inspect) {
  if (use_fast_solver_) {
    auto result = RunSouffle(
        symbol_table_, parser_.groups(), facts_, anchors_,
        parser_.inspections(),
        [&](const Inspection& i, std::string_view o) {
          return inspect(this, i, o);
        },
        [&](Symbol s) { return symbol_table_.PrettyText(s); });
    highest_goal_reached_ = result.highest_goal_reached;
    highest_group_reached_ = result.highest_group_reached;
    return result.success;
  } else {
    if (!PrepareDatabase()) {
      return false;
    }
    std::function<bool(Verifier*, const Inspection&)> wi =
        [&](Verifier* v, const Inspection& i) {
          return inspect(v, i, v->InspectionString(i));
        };
    Solver solver(this, facts_, anchors_, wi);
    bool result = solver.Solve();
    highest_goal_reached_ = solver.highest_goal_reached();
    highest_group_reached_ = solver.highest_group_reached();
    return result;
  }
}

bool Verifier::VerifyAllGoals() {
  return VerifyAllGoals([this](Verifier* context,
                               const Inspection& inspection) {
    if (inspection.kind == Inspection::Kind::EXPLICIT) {
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

Identifier* Verifier::IdentifierFor(const yy::location& location,
                                    const std::string& token) {
  Symbol symbol = symbol_table_.intern(token);
  return new (&arena_) Identifier(location, symbol);
}

Identifier* Verifier::IdentifierFor(const yy::location& location, int integer) {
  Symbol symbol = symbol_table_.intern(std::to_string(integer));
  return new (&arena_) Identifier(location, symbol);
}

AstNode* Verifier::MakePredicate(const yy::location& location, AstNode* head,
                                 absl::Span<AstNode* const> values) {
  size_t values_count = values.size();
  AstNode** body = (AstNode**)arena_.New(values_count * sizeof(AstNode*));
  size_t vn = 0;
  for (AstNode* v : values) {
    body[vn] = v;
    ++vn;
  }
  AstNode* tuple = new (&arena_) Tuple(location, values_count, body);
  return new (&arena_) App(location, head, tuple);
}

/// \brief Sort nodes such that nodes and facts are grouped.
static bool GraphvizSortOrder(AstNode* a, AstNode* b) {
  Tuple* ta = a->AsApp()->rhs()->AsTuple();
  Tuple* tb = b->AsApp()->rhs()->AsTuple();
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

static bool EncodedFactEqualTo(AstNode* a, AstNode* b) {
  Tuple* ta = a->AsApp()->rhs()->AsTuple();
  Tuple* tb = b->AsApp()->rhs()->AsTuple();
  return EncodedVNameOrIdentEqualTo(ta->element(0), tb->element(0)) &&
         EncodedIdentEqualTo(ta->element(1), tb->element(1)) &&
         EncodedVNameOrIdentEqualTo(ta->element(2), tb->element(2)) &&
         EncodedIdentEqualTo(ta->element(3), tb->element(3)) &&
         EncodedIdentEqualTo(ta->element(4), tb->element(4));
}

static bool EncodedVNameHasValidForm(Verifier* cxt, AstNode* a) {
  Tuple* ta = a->AsApp()->rhs()->AsTuple();
  return ta->element(0) != cxt->empty_string_id() ||
         ta->element(1) != cxt->empty_string_id() ||
         ta->element(2) != cxt->empty_string_id() ||
         ta->element(3) != cxt->empty_string_id() ||
         ta->element(4) != cxt->empty_string_id();
}

static bool EncodedFactHasValidForm(Verifier* cxt, AstNode* a) {
  Tuple* ta = a->AsApp()->rhs()->AsTuple();
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
      const std::string& ordinal_val =
          cxt->symbol_table()->text(ta->element(4)->AsIdentifier()->symbol());
      // TODO: check if valid int
      return true;
    }
  }
}

Verifier::InternedVName Verifier::InternVName(AstNode* node) {
  auto* tuple = node->AsApp()->rhs()->AsTuple();
  return {tuple->element(0)->AsIdentifier()->symbol(),
          tuple->element(1)->AsIdentifier()->symbol(),
          tuple->element(2)->AsIdentifier()->symbol(),
          tuple->element(3)->AsIdentifier()->symbol(),
          tuple->element(4)->AsIdentifier()->symbol()};
}

bool Verifier::ProcessFactTupleForFastSolver(Tuple* tuple) {
  // TODO(zarko): None of the text processing supports non-UTF8 encoded files.
  if (tuple->element(1) == empty_string_id_ &&
      tuple->element(2) == empty_string_id_) {
    if (EncodedIdentEqualTo(tuple->element(3), kind_id_)) {
      auto vname = InternVName(tuple->element(0));
      if (EncodedIdentEqualTo(tuple->element(4), file_id_)) {
        auto sym = fast_solver_files_.insert({vname, known_file_sym_});
        if (!sym.second && sym.first->second != known_not_file_sym_) {
          if (assertions_from_file_nodes_) {
            return LoadInMemoryRuleFile("", tuple->element(0),
                                        sym.first->second);
          } else {
            content_to_vname_[sym.first->second] = tuple->element(0);
          }
        }
      } else {
        fast_solver_files_[vname] = known_not_file_sym_;
      }
    } else if (EncodedIdentEqualTo(tuple->element(3), text_id_)) {
      auto vname = InternVName(tuple->element(0));
      auto content = tuple->element(4)->AsIdentifier()->symbol();
      auto file = fast_solver_files_.insert({vname, content});
      if (!file.second && file.first->second == known_file_sym_) {
        if (assertions_from_file_nodes_) {
          return LoadInMemoryRuleFile("", tuple->element(0), content);
        } else {
          content_to_vname_[content] = tuple->element(0);
        }
      }
    }
  }
  return true;
}

bool Verifier::PrepareDatabase() {
  if (database_prepared_) {
    return true;
  }
  if (use_fast_solver_) {
    LOG(WARNING) << "PrepareDatabase() called when fast solver was enabled";
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
  AstNode* last_anchor_vname = nullptr;
  AstNode* last_file_vname = nullptr;
  size_t last_anchor_start = ~0;
  for (size_t f = 0; f < facts_.size(); ++f) {
    AstNode* fb = facts_[f];

    if (!EncodedFactHasValidForm(this, fb)) {
      printer.Print("Fact has invalid form:\n  ");
      fb->Dump(symbol_table_, &printer);
      printer.Print("\n");
      is_ok = false;
      continue;
    }
    Tuple* tb = fb->AsApp()->rhs()->AsTuple();
    if (tb->element(1) == empty_string_id_ &&
        tb->element(2) == empty_string_id_) {
      bool is_kind_fact = EncodedIdentEqualTo(tb->element(3), kind_id_);
      // Check to see if this fact entry describes part of a file.
      // NB: kind_id_ is ordered before text_id_.
      if (is_kind_fact) {
        if (EncodedIdentEqualTo(tb->element(4), file_id_)) {
          last_file_vname = tb->element(0);
        } else {
          last_file_vname = nullptr;
        }
      } else if (last_file_vname != nullptr &&
                 EncodedIdentEqualTo(tb->element(3), text_id_)) {
        if (EncodedVNameOrIdentEqualTo(last_file_vname, tb->element(0))) {
          if (assertions_from_file_nodes_) {
            if (!LoadInMemoryRuleFile(
                    "", tb->element(0),
                    tb->element(4)->AsIdentifier()->symbol())) {
              is_ok = false;
            }
          } else {
            content_to_vname_[tb->element(4)->AsIdentifier()->symbol()] =
                tb->element(0);
          }
        }
        last_file_vname = nullptr;
      }
      // Check to see if this fact entry describes part of an anchor.
      // We've arranged via EncodedFactLessThan to sort kind_id_ before
      // start_id_ and start_id_ before end_id_ and to group all node facts
      // together in uninterrupted runs.
      if (is_kind_fact && EncodedIdentEqualTo(tb->element(4), anchor_id_)) {
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

    AstNode* fa = facts_[f - 1];
    if (!ignore_dups_ && EncodedFactEqualTo(fa, fb)) {
      printer.Print("Two facts were equal:\n  ");
      fa->Dump(symbol_table_, &printer);
      printer.Print("\n  ");
      fb->Dump(symbol_table_, &printer);
      printer.Print("\n");
      is_ok = false;
      continue;
    }
    Tuple* ta = fa->AsApp()->rhs()->AsTuple();
    if (EncodedVNameEqualTo(ta->element(0)->AsApp(), tb->element(0)->AsApp()) &&
        ta->element(1) == empty_string_id_ &&
        tb->element(1) == empty_string_id_ &&
        ta->element(2) == empty_string_id_ &&
        tb->element(2) == empty_string_id_ &&
        EncodedIdentEqualTo(ta->element(3), tb->element(3)) &&
        !EncodedIdentEqualTo(ta->element(4), tb->element(4))) {
      if (EncodedIdentEqualTo(ta->element(3), code_id_) ||
          EncodedIdentEqualTo(ta->element(3), code_json_id_)) {
        if (!ignore_code_conflicts_) {
          // TODO(#1553): (closed?) Add documentation for these new edges.
          printer.Print(
              "Two /kythe/code facts about a node differed in value:\n  ");
          ta->element(0)->Dump(symbol_table_, &printer);
          printer.Print("\n  ");
          printer.Print("\nThe decoded values were:\n");
          auto print_decoded = [&](AstNode* value) {
            if (auto* ident = value->AsIdentifier()) {
              proto::common::MarkedSource marked_source;
              if (!marked_source.ParseFromString(
                      symbol_table_.text(ident->symbol()))) {
                printer.Print("(failed to decode)\n");
              } else {
                printer.Print(absl::StrCat(marked_source));
                printer.Print("\n");
              }
            } else {
              printer.Print("(not an identifier)\n");
            }
          };
          print_decoded(ta->element(4));
          printer.Print("\n -----------------  versus  ----------------- \n\n");
          print_decoded(tb->element(4));
          is_ok = false;
        }
      } else {
        printer.Print("Two facts about a node differed in value:\n  ");
        fa->Dump(symbol_table_, &printer);
        printer.Print("\n  ");
        fb->Dump(symbol_table_, &printer);
        printer.Print("\n");
        is_ok = false;
      }
    }
  }
  if (is_ok) {
    std::sort(facts_.begin(), facts_.end(), FastLookupFactLessThan);
  }
  database_prepared_ = is_ok;
  return is_ok;
}

std::string Verifier::InspectionString(const Inspection& i) {
  StringPrettyPrinter printer;
  if (i.evar == nullptr) {
    printer.Print("nil");
  } else {
    i.evar->Dump(symbol_table_, &printer);
  }
  return printer.str();
}

AstNode* Verifier::ConvertVName(const yy::location& loc,
                                const kythe::proto::VName& vname) {
  AstNode** values = (AstNode**)arena_.New(sizeof(AstNode*) * 5);
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
  AstNode* tuple = new (&arena_) Tuple(loc, 5, values);
  return new (&arena_) App(vname_id_, tuple);
}

AstNode* Verifier::NewUniqueVName(const yy::location& loc) {
  return MakePredicate(
      loc, vname_id_,
      {new (&arena_) Identifier(loc, symbol_table_.unique()), empty_string_id_,
       empty_string_id_, empty_string_id_, empty_string_id_});
}

AstNode* Verifier::ConvertCodeFact(const yy::location& loc,
                                   const std::string& code_data) {
  proto::common::MarkedSource marked_source;
  if (!marked_source.ParseFromString(code_data)) {
    LOG(ERROR) << loc << ": can't parse code protobuf" << std::endl;
    return nullptr;
  }
  return ConvertMarkedSource(loc, marked_source);
}

AstNode* Verifier::ConvertCodeJsonFact(const yy::location& loc,
                                       const std::string& code_data) {
  proto::common::MarkedSource marked_source;
  if (!google::protobuf::util::JsonStringToMessage(code_data, &marked_source)
           .ok()) {
    LOG(ERROR) << loc << ": can't parse code/json protobuf" << std::endl;
    return nullptr;
  }
  return ConvertMarkedSource(loc, marked_source);
}

AstNode* Verifier::ConvertMarkedSource(
    const yy::location& loc, const proto::common::MarkedSource& source) {
  // Explode each MarkedSource message into a node with an unutterable vname.
  auto* vname = NewUniqueVName(loc);
  for (int child = 0; child < source.child_size(); ++child) {
    auto* child_vname = ConvertMarkedSource(loc, source.child(child));
    if (child_vname == nullptr) {
      return nullptr;
    }
    facts_.push_back(MakePredicate(
        loc, fact_id_,
        {vname, marked_source_child_id_, child_vname, ordinal_id_,
         IdentifierFor(builtin_location_, std::to_string(child))}));
  }
  for (const auto& link : source.link()) {
    if (link.definition_size() != 1) {
      std::cerr << loc << ": bad link: want one definition" << std::endl;
      return nullptr;
    }
    auto from_uri = URI::FromString(link.definition(0));
    if (!from_uri.first) {
      std::cerr << loc << ": bad URI in link" << std::endl;
      return nullptr;
    }
    facts_.push_back(MakePredicate(loc, fact_id_,
                                   {vname, marked_source_link_id_,
                                    ConvertVName(loc, from_uri.second.v_name()),
                                    root_id_, empty_string_id_}));
  }
  auto emit_fact = [&](AstNode* fact_id, AstNode* fact_value) {
    facts_.push_back(MakePredicate(
        loc, fact_id_,
        {vname, empty_string_id_, empty_string_id_, fact_id, fact_value}));
  };
  switch (source.kind()) {
    case proto::common::MarkedSource::BOX:
      emit_fact(marked_source_kind_id_, marked_source_box_id_);
      break;
    case proto::common::MarkedSource::TYPE:
      emit_fact(marked_source_kind_id_, marked_source_type_id_);
      break;
    case proto::common::MarkedSource::PARAMETER:
      emit_fact(marked_source_kind_id_, marked_source_parameter_id_);
      break;
    case proto::common::MarkedSource::IDENTIFIER:
      emit_fact(marked_source_kind_id_, marked_source_identifier_id_);
      break;
    case proto::common::MarkedSource::CONTEXT:
      emit_fact(marked_source_kind_id_, marked_source_context_id_);
      break;
    case proto::common::MarkedSource::INITIALIZER:
      emit_fact(marked_source_kind_id_, marked_source_initializer_id_);
      break;
    case proto::common::MarkedSource::MODIFIER:
      emit_fact(marked_source_kind_id_, marked_source_modifier_id_);
      break;
    case proto::common::MarkedSource::PARAMETER_LOOKUP_BY_PARAM:
      emit_fact(marked_source_kind_id_,
                marked_source_parameter_lookup_by_param_id_);
      break;
    case proto::common::MarkedSource::LOOKUP_BY_PARAM:
      emit_fact(marked_source_kind_id_, marked_source_lookup_by_param_id_);
      break;
    case proto::common::MarkedSource::PARAMETER_LOOKUP_BY_TPARAM:
      emit_fact(marked_source_kind_id_,
                marked_source_parameter_lookup_by_tparam_id_);
      break;
    case proto::common::MarkedSource::LOOKUP_BY_TPARAM:
      emit_fact(marked_source_kind_id_, marked_source_lookup_by_tparam_id_);
      break;
    case proto::common::MarkedSource::PARAMETER_LOOKUP_BY_PARAM_WITH_DEFAULTS:
      emit_fact(marked_source_kind_id_,
                marked_source_parameter_lookup_by_param_with_defaults_id_);
      break;
    case proto::common::MarkedSource::LOOKUP_BY_TYPED:
      emit_fact(marked_source_kind_id_, marked_source_lookup_by_typed_id_);
      break;
    // The proto enum is polluted with enumerators like
    // MarkedSource_Kind_MarkedSource_Kind_INT_MIN_SENTINEL_DO_NOT_USE_.
    default:
      std::cerr << loc << ": unknown source kind for MarkedSource" << std::endl;
  }
  emit_fact(marked_source_pre_text_id_, IdentifierFor(loc, source.pre_text()));
  emit_fact(marked_source_post_child_text_id_,
            IdentifierFor(loc, source.post_child_text()));
  emit_fact(marked_source_post_text_id_,
            IdentifierFor(loc, source.post_text()));
  emit_fact(marked_source_lookup_index_id_,
            IdentifierFor(loc, std::to_string(source.lookup_index())));
  emit_fact(
      marked_source_default_children_count_id_,
      IdentifierFor(loc, std::to_string(source.default_children_count())));
  emit_fact(marked_source_add_final_list_token_id_,
            source.add_final_list_token() ? marked_source_true_id_
                                          : marked_source_false_id_);
  return vname;
}

bool Verifier::AssertSingleFact(std::string* database, unsigned int fact_id,
                                const kythe::proto::Entry& entry) {
  yy::location loc;
  loc.initialize(database);
  loc.begin.column = 1;
  loc.begin.line = fact_id;
  loc.end = loc.begin;
  Symbol code_symbol = code_id_->AsIdentifier()->symbol();
  Symbol code_json_symbol = code_json_id_->AsIdentifier()->symbol();
  AstNode** values = (AstNode**)arena_.New(sizeof(AstNode*) * 5);
  values[0] =
      entry.has_source() ? ConvertVName(loc, entry.source()) : empty_string_id_;
  // We're removing support for ordinal facts. Support them during the
  // transition, but also support the new dot-separated edge kinds that serve
  // the same purpose.
  auto dot_pos = entry.edge_kind().rfind('.');
  bool is_code = false;
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
    if (values[3]->AsIdentifier()->symbol() == code_symbol &&
        convert_marked_source_) {
      // Code facts are turned into subgraphs, so this fact entry will turn
      // into an edge entry.
      if ((values[2] = ConvertCodeFact(loc, entry.fact_value())) == nullptr) {
        return false;
      }
      values[1] = marked_source_code_edge_id_;
      values[3] = root_id_;
      values[4] = empty_string_id_;
      is_code = true;
    } else if (values[3]->AsIdentifier()->symbol() == code_json_symbol &&
               convert_marked_source_) {
      // Code facts are turned into subgraphs, so this fact entry will turn
      // into an edge entry.
      if ((values[2] = ConvertCodeJsonFact(loc, entry.fact_value())) ==
          nullptr) {
        return false;
      }
      values[1] = marked_source_code_edge_id_;
      values[3] = root_id_;
      values[4] = empty_string_id_;
      is_code = true;
    } else {
      values[4] = entry.fact_value().empty()
                      ? empty_string_id_
                      : IdentifierFor(loc, entry.fact_value());
    }
  }
  if (!is_code) {
    values[2] = entry.has_target() ? ConvertVName(loc, entry.target())
                                   : empty_string_id_;
  }

  Tuple* tuple = new (&arena_) Tuple(loc, 5, values);
  AstNode* fact = new (&arena_) App(fact_id_, tuple);

  database_prepared_ = false;
  facts_.push_back(fact);
  if (use_fast_solver_) {
    return ProcessFactTupleForFastSolver(tuple);
  }
  return true;
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
  auto DumpAsJson = [this, &printer, &escaping_printer](const char* label,
                                                        AstNode* node) {
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
  auto DumpVName = [this, &printer, &DumpAsJson](const char* label,
                                                 AstNode* node) {
    printer.Print(label);
    if (node == empty_string_id()) {
      printer.Print("null");
    } else {
      Tuple* vname = node->AsApp()->rhs()->AsTuple();
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
    AstNode* fact = facts_[i];
    Tuple* t = fact->AsApp()->rhs()->AsTuple();
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
  for (const auto& [label, vname] : saved_assignments_) {
    if (!vname) {
      continue;
    }
    StringPrettyPrinter printer;
    QuoteEscapingPrettyPrinter quote_printer(printer);
    vname->Dump(symbol_table_, &printer);
    auto old_label = vname_labels.find(printer.str());
    if (old_label == vname_labels.end()) {
      vname_labels[printer.str()] = label;
    } else {
      old_label->second += ", " + label;
    }
  }
  auto GetLabel = [&](AstNode* node) {
    if (!node) {
      return std::string();
    }
    StringPrettyPrinter id_string;
    QuoteEscapingPrettyPrinter id_quote(id_string);
    node->Dump(symbol_table_, &id_string);
    const auto& label = vname_labels.find(id_string.str());
    if (label != vname_labels.end()) {
      return label->second;
    } else {
      return std::string();
    }
  };
  auto ElideNode = [&](AstNode* node) {
    if (show_unlabeled_) {
      return false;
    }
    return GetLabel(node).empty();
  };

  std::sort(facts_.begin(), facts_.end(), GraphvizSortOrder);
  FileHandlePrettyPrinter printer(stdout);
  QuoteEscapingPrettyPrinter quote_printer(printer);
  HtmlEscapingPrettyPrinter html_printer(printer);
  FileHandlePrettyPrinter dprinter(stderr);

  auto PrintQuotedNodeId = [&](AstNode* node) {
    printer.Print("\"");
    if (std::string label = GetLabel(node);
        show_labeled_vnames_ || label.empty()) {
      node->Dump(symbol_table_, &quote_printer);
    } else {
      quote_printer.Print(label);
    }
    printer.Print("\"");
  };

  auto FactName = [this](AstNode* node) {
    StringPrettyPrinter printer;
    node->Dump(symbol_table_, &printer);
    if (show_fact_prefix_) {
      return printer.str();
    }
    return std::string(absl::StripPrefix(printer.str(), "/kythe/"));
  };

  auto EdgeName = [this](AstNode* node) {
    StringPrettyPrinter printer;
    node->Dump(symbol_table_, &printer);
    if (show_fact_prefix_) {
      return printer.str();
    }
    return std::string(absl::StripPrefix(printer.str(), "/kythe/edge/"));
  };

  printer.Print("digraph G {\n");
  for (size_t i = 0; i < facts_.size(); ++i) {
    AstNode* fact = facts_[i];
    Tuple* t = fact->AsApp()->rhs()->AsTuple();
    if (t->element(1) == empty_string_id()) {
      // Node. We sorted these above st all the facts should come subsequent.
      // Figure out if the node is an anchor.
      NodeFacts info =
          ReadNodeFacts(absl::MakeConstSpan(facts_).subspan(i), *this);
      if (!info.facts.empty()) {
        // Skip over facts which correspond to this node.
        i += info.facts.size() - 1;
      }
      if (ElideNode(t->element(0))) {
        continue;
      }
      PrintQuotedNodeId(t->element(0));
      std::string label = GetLabel(t->element(0));
      if (info.kind == NodeKind::kAnchor && !show_anchors_) {
        printer.Print(" [ shape=circle, label=\"");
        if (label.empty()) {
          printer.Print("@");
        } else {
          printer.Print(label);
          printer.Print("\", color=\"blue");
        }
        printer.Print("\" ];\n");
      } else {
        printer.Print(" [ label=<<TABLE>");
        printer.Print("<TR><TD COLSPAN=\"2\">");
        Tuple* nt = info.facts.front()->AsApp()->rhs()->AsTuple();
        if (label.empty() || show_labeled_vnames_) {
          // Since all of our facts are well-formed, we know this is a vname.
          nt->element(0)->AsApp()->rhs()->Dump(symbol_table_, &html_printer);
        }
        if (!label.empty()) {
          if (show_labeled_vnames_) {
            html_printer.Print(" = ");
          }
          html_printer.Print(label);
        }
        printer.Print("</TD></TR>");
        for (AstNode* fact : info.facts) {
          Tuple* nt = fact->AsApp()->rhs()->AsTuple();
          printer.Print("<TR><TD>");
          html_printer.Print(FactName(nt->element(3)));
          printer.Print("</TD><TD>");
          if (info.kind == NodeKind::kFile &&
              EncodedIdentEqualTo(nt->element(3), text_id_)) {
            // Don't clutter the graph with file content.
            printer.Print("...");
          } else if (EncodedIdentEqualTo(nt->element(3), code_id_)) {
            // Don't print encoded proto data.
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
    } else {
      // Edge.
      if (ElideNode(t->element(0)) || ElideNode(t->element(2))) {
        continue;
      }
      PrintQuotedNodeId(t->element(0));
      printer.Print(" -> ");
      PrintQuotedNodeId(t->element(2));
      printer.Print(" [ label=\"");
      quote_printer.Print(EdgeName(t->element(1)));
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
