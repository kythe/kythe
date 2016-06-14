// Started from the calc++ example code as part of the Bison-3.0 distribution.
// NOTE: This file must remain compatible with Bison 2.3.
%skeleton "lalr1.cc"
%defines
%define "parser_class_name" "AssertionParserImpl"
%{
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
#include <string>
#include "kythe/cxx/verifier/assertion_ast.h"
#ifdef yylex
#undef yylex
#endif
#define yylex kythe::verifier::AssertionParser::lex
namespace kythe {
namespace verifier {
class AssertionParser;
}
}
%}
%parse-param { ::kythe::verifier::AssertionParser &context }
%lex-param { ::kythe::verifier::AssertionParser &context }
%locations
%initial-action
{
  @$.initialize(&context.file());
};
%define "parse.trace"
%{
#include "kythe/cxx/verifier/assertions.h"
#define newAst new (context.arena_) kythe::verifier::
%}
%token
  END 0 "end of file"
  LPAREN  "("
  RPAREN  ")"
  LBRACE  "{"
  RBRACE  "}"
  BANG    "!"
  COMMA   ","
  DONTCARE "_"
  APOSTROPHE "'"
  AT "@"
  AT_HAT "@^"
  AT_CASH "@$"
  DOT "."
  WHAT "?"
  EQUALS "="
  COLON ":"
  PLUS "+"
  HASH "#"
;
%token <string> IDENTIFIER "identifier"
%token <string> STRING "string"
%token <string> NUMBER "number"
%type <node> exp
%type <node> atom
%type <node> goal
%type <node> exp_tuple_star
%type <int_> nested_goals
%type <string> string_or_identifier
%type <int_> location_spec
%type <int_> location_spec_hash
%type <size_t_> exp_tuple_plus
%error-verbose
%%
%start unit;
unit: goals  { };

goals:
  /* empty */ {}
| goals goal { context.AppendGoal(context.group_id(), $2); }
| goals LBRACE { context.EnterGoalGroup(@2, false); }
    nested_goals RBRACE { context.ExitGoalGroup(@5); }
| goals BANG LBRACE { context.EnterGoalGroup(@2, true); }
    nested_goals RBRACE { context.ExitGoalGroup(@5); }

nested_goals:
  nested_goals goal { context.AppendGoal(context.group_id(), $2); $$ = 0; }
| goal { context.AppendGoal(context.group_id(), $1); $$ = 0; }

string_or_identifier:
  "identifier" { $$ = $1; }
| "string" { $$ = $1; }

goal:
  exp string_or_identifier exp {
    $$ = context.CreateSimpleEdgeFact(@1 + @3, $1, $2, $3, nullptr);
  }
| exp "." string_or_identifier exp {
    $$ = context.CreateSimpleNodeFact(@1 + @4, $1, $3, $4);
  }
| exp string_or_identifier "." atom exp {
    $$ = context.CreateSimpleEdgeFact(@1 + @5, $1, $2, $5, $4);
  }

exp:
  atom exp_tuple_star { $$ = newAst App($1, $2); };
| atom { $$ = $1; };
| atom "=" exp {
    context.AppendGoal(context.group_id(),
                       context.CreateEqualityConstraint(@2, $1, $3));
    $$ = $1;
  };

atom:
    "identifier"       { $$ = context.CreateAtom(@1, $1); }
  | "string"           { $$ = context.CreateIdentifier(@1, $1); }
  | "_"                { $$ = context.CreateDontCare(@1); }
  | "number"           { $$ = context.CreateIdentifier(@1, $1); };
  | "@" location_spec_hash  { $$ = context.CreateAnchorSpec(@1); };
  | "@^" location_spec_hash { $$ = context.CreateOffsetSpec(@1, false); };
  | "@$" location_spec_hash { $$ = context.CreateOffsetSpec(@1, true); };
  | "identifier" "?"   { $$ = context.CreateInspect(@2, $1,
                                                    context.CreateAtom(@1, $1));
                       }
  | "_" "?"            { $$ = context.CreateInspect(@2, "_",
                                                    context.CreateDontCare(@1));
                       }

exp_tuple_plus:
    exp_tuple_plus "," exp { context.PushNode($3); $$ = $1 + 1; }
  | exp { context.PushNode($1); $$ = 1; }

exp_tuple_star:
    "(" ")" { $$ = newAst Tuple(@1, 0, nullptr); }
  | "(" exp_tuple_plus ")" {
    $$ = newAst Tuple(@1, $2, context.PopNodes($2));
  }

location_spec:
    string_or_identifier { context.PushLocationSpec($1); $$ = 0; }
  | ":" "number" string_or_identifier {
    context.PushAbsoluteLocationSpec($3, $2); $$ = 0;
  }
  | "+" "number" string_or_identifier {
    context.PushRelativeLocationSpec($3, $2); $$ = 0;
  }

location_spec_hash:
    location_spec { $$ = $1; }
  | HASH "number" location_spec {
    context.SetTopLocationSpecMatchNumber($2); $$ = $3;
  }

%%
void yy::AssertionParserImpl::error(const location_type &l,
                                    const std::string &m) {
  context.Error(l, m);
}
