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
// Started from the calc++ example code as part of the Bison-3.0 distribution.
#include "kythe/cxx/verifier/assertions.h"
#include "kythe/cxx/verifier/parser.yy.hh"
#include <assert.h>

// The offset of the current token (as byte offset).
static size_t loc_ofs;
%}
%option noyywrap nounput batch debug noinput bison-bridge
id    [a-zA-Z/][a-zA-Z_0-9/]*
int   [0-9]+
blank [ \t]

%{
  // Code run each time a pattern is matched.
  #define YY_USER_ACTION  yylloc->columns(yyleng); loc_ofs += yyleng;
%}

/* The lexer has the following states:
 *   INITIAL: We aren't sure whether this line is relevant to parsing
 *            rules; check after every character whether to switch states.
 *   IGNORED: This line is definitely not one that contains input for our
 *            parser. While still updating the file location, wait until
 *            an endline.
 *    NORMAL: This line contains input that must be passed on to the parser. */
%s IGNORED NORMAL
%%

%{
  // Code run each time yylex is called.
  yylloc->step();
%}

<INITIAL>{
\n       yylloc->lines(yyleng); yylloc->step(); context.ResetLexCheck();
.        {
            int lex_check = context.NextLexCheck(yytext);
            if (lex_check < 0) {
              BEGIN(IGNORED);
            } else if (lex_check > 0) {
              BEGIN(NORMAL);
            }
         }
}  /* INITIAL state */

<IGNORED>{
\n       {
            // Resolve locations after the first endline.
            if (!context.ResolveLocations(*yylloc, loc_ofs, false)) {
              context.Error(*yylloc, "could not resolve all locations");
            }
            yylloc->lines(yyleng);
            yylloc->step();
            BEGIN(INITIAL);
         }
[^\n]*   context.AppendToLine(yytext);
}  /* IGNORED state */

<NORMAL>{
{blank}+ yylloc->step();
\n       {
          yylloc->lines(yyleng);
          yylloc->step();
          context.ResetLexCheck();
          BEGIN(INITIAL);
         }
"("      return yy::AssertionParserImpl::token::LPAREN;
")"      return yy::AssertionParserImpl::token::RPAREN;
","      return yy::AssertionParserImpl::token::COMMA;
"_"      return yy::AssertionParserImpl::token::DONTCARE;
"'"      return yy::AssertionParserImpl::token::APOSTROPHE;
"@^"     return yy::AssertionParserImpl::token::AT_HAT;
"@$"     return yy::AssertionParserImpl::token::AT_CASH;
"@"      return yy::AssertionParserImpl::token::AT;
"."      return yy::AssertionParserImpl::token::DOT;
"?"      return yy::AssertionParserImpl::token::WHAT;
"="      return yy::AssertionParserImpl::token::EQUALS;
"{"      return yy::AssertionParserImpl::token::LBRACE;
"}"      return yy::AssertionParserImpl::token::RBRACE;
"!"      return yy::AssertionParserImpl::token::BANG;
":"      return yy::AssertionParserImpl::token::COLON;
"+"      return yy::AssertionParserImpl::token::PLUS;
"#"      return yy::AssertionParserImpl::token::HASH;
{int}    yylval->string = yytext; return yy::AssertionParserImpl::token::NUMBER;
{id}     yylval->string = yytext; return yy::AssertionParserImpl::token::IDENTIFIER;
\"(\\.|[^\\"])*\" {
                   std::string out;
                   if (!context.Unescape(yytext, &out)) {
                     context.Error(*yylloc, "invalid literal string");
                   }
                   yylval->string = out;
                   return yy::AssertionParserImpl::token::STRING;
                 }
.        context.Error(*yylloc, "invalid character");
}  /* NORMAL state */

<<EOF>>  {
            context.save_eof(*yylloc, loc_ofs);
            return yy::AssertionParserImpl::token::END;
         }
%%

namespace kythe {
namespace verifier {

static YY_BUFFER_STATE stringBufferState = nullptr;
static std::string *kNoFile = new std::string("no-file");

void AssertionParser::ScanBeginString(const std::string &data,
                                      bool trace_scanning) {
  BEGIN(INITIAL);
  loc_ofs = 0;
  yy_flex_debug = trace_scanning;
  assert(stringBufferState == nullptr);
  stringBufferState = yy_scan_bytes(data.c_str(), data.size());
}

void AssertionParser::ScanBeginFile(bool trace_scanning) {
  BEGIN(INITIAL);
  loc_ofs = 0;
  yy_flex_debug = trace_scanning;
  if (file().empty() || file() == "-") {
    yyin = stdin;
  } else if (!(yyin = fopen(file().c_str(), "r"))) {
    Error("cannot open " + file() + ": " + strerror(errno));
    exit(EXIT_FAILURE);
  }
}

void AssertionParser::ScanEnd(const yy::location &eof_loc,
                              size_t eof_loc_ofs) {
  // Imagine that all files end with an endline.
  if (!ResolveLocations(eof_loc, eof_loc_ofs + 1, true)) {
    Error(eof_loc, "could not resolve all locations at end of file");
  }
  if (stringBufferState) {
    yy_delete_buffer(stringBufferState);
    stringBufferState = nullptr;
  } else {
    fclose(yyin);
  }
}

}
}
