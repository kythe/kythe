// Started from the calc++ example code as part of the Bison-3.0 distribution.
%skeleton "lalr1.cc"
%defines
%define parser_class_name {AssertionParserImpl}
%{
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
%parse-param { ::kythe::verifier::AssertionParser &parser_context }
%lex-param { ::kythe::verifier::AssertionParser &parser_context }
%locations
%initial-action
{
  @$.initialize(&parser_context.file());
  @$.begin.column = 1;
  @$.end.column = 1;
};
%define parse.trace
%{
#include "kythe/cxx/verifier/assertions.h"
#define newAst new (parser_context.arena_) kythe::verifier::
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
;
%token <string> IDENTIFIER "identifier"
%token <string> STRING "string"
%token <string> NUMBER "number"
%token <string> HASH_NUMBER "hashnumber"
%type <node> exp
%type <node> atom
%type <node> goal
%type <node> exp_tuple_star
%type <int_> nested_goals
%type <string> string_or_identifier
%type <int_> location_spec
%type <int_> location_spec_hash
%type <size_t_> exp_tuple_plus
%define parse.error verbose
%%
%start unit;
unit: goals  { };

goals:
  /* empty */ {}
| goals goal { parser_context.AppendGoal(parser_context.group_id(), $2); }
| goals LBRACE { parser_context.EnterGoalGroup(@2, false); }
    nested_goals RBRACE { parser_context.ExitGoalGroup(@5); }
| goals BANG LBRACE { parser_context.EnterGoalGroup(@2, true); }
    nested_goals RBRACE { parser_context.ExitGoalGroup(@5); }

nested_goals:
  nested_goals goal {
    parser_context.AppendGoal(parser_context.group_id(), $2);
    $$ = 0;
  }
| goal { parser_context.AppendGoal(parser_context.group_id(), $1); $$ = 0; }

string_or_identifier:
  "identifier" { $$ = $1; }
| "string" { $$ = $1; }

goal:
  exp string_or_identifier exp {
    $$ = parser_context.CreateSimpleEdgeFact(@1 + @3, $1, $2, $3, nullptr);
  }
| exp "." string_or_identifier exp {
    $$ = parser_context.CreateSimpleNodeFact(@1 + @4, $1, $3, $4);
  }
| exp string_or_identifier "." atom exp {
    $$ = parser_context.CreateSimpleEdgeFact(@1 + @5, $1, $2, $5, $4);
  }

exp:
  atom exp_tuple_star { $$ = newAst App($1, $2); };
| atom { $$ = $1; };
| atom "=" exp {
    parser_context.AppendGoal(
        parser_context.group_id(),
        parser_context.CreateEqualityConstraint(@2, $1, $3)
    );
    $$ = $1;
  };

atom:
    "identifier"       { $$ = parser_context.CreateAtom(@1, $1); }
  | "string"           { $$ = parser_context.CreateIdentifier(@1, $1); }
  | "_"                { $$ = parser_context.CreateDontCare(@1); }
  | "number"           { $$ = parser_context.CreateIdentifier(@1, $1); };
  | "@" location_spec_hash  { $$ = parser_context.CreateAnchorSpec(@1); };
  | "@^" location_spec_hash {
      $$ = parser_context.CreateOffsetSpec(@1, false);
    };
  | "@$" location_spec_hash {
      $$ = parser_context.CreateOffsetSpec(@1, true);
    };
  | "identifier" "?" {
      $$ = parser_context.CreateInspect(
          @2,
          $1,
          parser_context.CreateAtom(@1, $1)
      );
    }
  | "_" "?" {
      $$ = parser_context.CreateInspect(
          @2,
          "_",
          parser_context.CreateDontCare(@1)
      );
    }

exp_tuple_plus:
    exp_tuple_plus "," exp { parser_context.PushNode($3); $$ = $1 + 1; }
  | exp { parser_context.PushNode($1); $$ = 1; }

exp_tuple_star:
    "(" ")" { $$ = newAst Tuple(@1, 0, nullptr); }
  | "(" exp_tuple_plus ")" {
    $$ = newAst Tuple(@1, $2, parser_context.PopNodes($2));
  }

location_spec:
    string_or_identifier { parser_context.PushLocationSpec($1); $$ = 0; }
  | ":" "number" string_or_identifier {
    parser_context.PushAbsoluteLocationSpec($3, $2); $$ = 0;
  }
  | "+" "number" string_or_identifier {
    parser_context.PushRelativeLocationSpec($3, $2); $$ = 0;
  }

location_spec_hash:
    location_spec { $$ = $1; }
  | HASH_NUMBER location_spec {
    parser_context.SetTopLocationSpecMatchNumber($1); $$ = $2;
  }

%%
void yy::AssertionParserImpl::error(const location_type &l,
                                    const std::string &m) {
  parser_context.Error(l, m);
}
