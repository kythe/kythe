/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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

// Implementation notes:
// The proto indexer and the proto compiler collaborate through metadata to link
// generated code back to the protobuf definitions. In our case, we care about
// the fact that generated getters are linked to to the original fields.
//
// The idea is that we're not going to refer to the original proto fields
// directly. Instead, we're going to emit references from sections of the string
// literal being parsed to the corresponding getters of generated cpp classes.
// Because the proto indexer links these getters to the original fields, we get
// the behaviour we want.
// 1. We start by getting the cpp message decl from the type T of the message
//    being parsed, using "ParseProtoHelper::operator T()".
// 2. To index a field named "blah", we just need to emit references to T::blah.
// 3. If we are accessing a subfield "inner_blah", we need to get the type U for
//    this field. We can do that without Kythe knowing about the proto because
//    we can get the type from the return value of the accessor T::inner_blah
//    (that returns a const U&). Then we can apply (2) again.

#include "ProtoLibrarySupport.h"

#include <map>

#include "absl/flags/flag.h"
#include "absl/log/log.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/ExprCXX.h"
#include "google/protobuf/io/tokenizer.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#include "kythe/cxx/indexer/cxx/IndexerASTHooks.h"

ABSL_FLAG(std::string, parseprotohelper_full_name,
          "proto2::contrib::parse_proto::internal::ParseProtoHelper",
          "Full name of the ParseProtoHelper class.");

namespace kythe {

namespace {

using google::protobuf::io::Tokenizer;

using ParseCallback =
    std::function<void(const clang::CXXMethodDecl&, const clang::SourceRange&)>;

// A proto tokenizer Error collector that outputs to LOG(ERROR).
class LogErrors : public google::protobuf::io::ErrorCollector {
  void AddError(int Line, int Column, const std::string& Message) override {
    LOG(ERROR) << "l. " << Line << " c. " << Column << ": " << Message;
  }
};

// A class that parses a text proto without checking for field existence. The
// big difference between this and text_format.h is that this parser knows
// nothing about the proto being parsed.
class ParseTextProtoHandler {
 public:
  // Parses the message and returns true on success.
  static bool Parse(const ParseCallback& FoundField,
                    const clang::StringLiteral* Literal,
                    const clang::CXXRecordDecl& MsgDecl,
                    const clang::ASTContext& Context,
                    const clang::LangOptions& LangOpts);

 private:
  struct LineColumnPair {
    LineColumnPair(int Line, int Column) : Line(Line), Column(Column) {}
    bool operator<(const LineColumnPair& O) const {
      return std::tie(Line, Column) < std::tie(O.Line, O.Column);
    }
    int Line = 0;
    int Column = 0;
  };

  // Creates a ParseTextProtoHandler that parses the given value and calls
  // found_field on findings. All objects should remain valid for the
  // lifetime of the handler.
  ParseTextProtoHandler(const ParseCallback& FoundField,
                        const clang::StringLiteral* Literal,
                        const clang::ASTContext& Context,
                        const clang::LangOptions& LangOpts);

  // Parses fields of a message with the given decl. Returns false on error. If
  // nested is true, then hitting a '}' token will return without error.
  bool ParseMsg(const clang::CXXRecordDecl& MsgDecl, bool Nested);

  // Parses a field value, including the separator, e.g.
  //    ": 'literal'"
  // or
  //    "{ field1: 3 field2: 'value' }"
  bool ParseFieldValue(const clang::CXXMethodDecl& accessor_decl);

  // Returns the source location/range of a given position/token.
  clang::SourceLocation GetSourceLocation(
      const LineColumnPair& LineColumn) const;
  clang::SourceRange GetTokenSourceRange(const Tokenizer::Token& Token) const;

  const clang::StringLiteral* const Literal;
  const clang::ASTContext& Context;
  const clang::LangOptions& LangOpts;
  const ParseCallback FoundField;
  google::protobuf::io::ArrayInputStream IStream;
  LogErrors Errors;
  Tokenizer TextTokenizer;
  // Index of token (line,column) to byte offset in the string literal. See
  // comment in constructor.
  std::map<LineColumnPair, int> LineColumnToOffset;
};

const clang::CXXMethodDecl* FindAccessorDeclWithName(
    const clang::CXXRecordDecl& MsgDecl, llvm::StringRef Name) {
  for (const clang::CXXMethodDecl* Method : MsgDecl.methods()) {
    // Accessors are user-provided, skip any compiler-generated or
    // non-identifier operator/ctor.
    if (const auto* II = Method->getIdentifier();
        II && Method->isUserProvided()) {
      const auto MethodName = II->getName();
      // Field accessors will either be the same as the field name or, if they
      // conflict with a language keyword, the field name with a trailing
      // underscore.
      if (MethodName == Name ||
          (MethodName.size() == Name.size() + 1 &&
           MethodName.startswith(Name) && MethodName.endswith("_"))) {
        return Method;
      }
    }
  }
  return nullptr;
}

ParseTextProtoHandler::ParseTextProtoHandler(
    const ParseCallback& FoundField, const clang::StringLiteral* Literal,
    const clang::ASTContext& Context, const clang::LangOptions& LangOpts)
    : Literal(Literal),
      Context(Context),
      LangOpts(LangOpts),
      FoundField(FoundField),
      IStream(Literal->getBytes().data(), Literal->getBytes().size()),
      TextTokenizer(&IStream, &Errors) {
  // We're building this table so that we can map io::Tokenizer lines and
  // columns back to byte offsets in the string literal. See
  // Tokenizer::NextChar() for why we're doing this.
  // TODO(courbet): It would be much better to add support for byte offset in
  // the tokenizer directly.
  LineColumnPair LineColumn(0, 0);
  constexpr const int kTokenizerTabWidth = 8;
  LineColumnToOffset[LineColumn] = 0;
  for (int ByteOffset = 0; ByteOffset < Literal->getBytes().size();
       ++ByteOffset) {
    const char c = Literal->getBytes()[ByteOffset];
    if (c == '\n') {
      ++LineColumn.Line;
      LineColumn.Column = 0;
    } else if (c == '\t') {
      LineColumn.Column +=
          kTokenizerTabWidth - LineColumn.Column % kTokenizerTabWidth;
    } else {
      ++LineColumn.Column;
    }
    LineColumnToOffset[LineColumn] = ByteOffset + 1;
  }
}

bool ParseTextProtoHandler::Parse(const ParseCallback& FoundField,
                                  const clang::StringLiteral* Literal,
                                  const clang::CXXRecordDecl& MsgDecl,
                                  const clang::ASTContext& Context,
                                  const clang::LangOptions& LangOpts) {
  ParseTextProtoHandler handler(FoundField, Literal, Context, LangOpts);
  return handler.ParseMsg(MsgDecl, false);
}

bool ParseTextProtoHandler::ParseMsg(const clang::CXXRecordDecl& MsgDecl,
                                     bool nested) {
  while (TextTokenizer.Next()) {
    const Tokenizer::Token& Token = TextTokenizer.current();
    switch (Token.type) {
      case Tokenizer::TYPE_IDENTIFIER: {
        // Assume that this is a field name.
        const auto* AccessorDecl =
            FindAccessorDeclWithName(MsgDecl, Token.text);
        if (!AccessorDecl) {
          LOG(ERROR) << "Cannot find field " << Token.text << " for message "
                     << MsgDecl.getDeclName().getAsString();
          return false;
        }
        if (Token.line < 0) {
          return false;
        }
        FoundField(*AccessorDecl, GetTokenSourceRange(Token));
        if (!ParseFieldValue(*AccessorDecl)) {
          return false;
        }
        break;
      }
      case Tokenizer::TYPE_INTEGER:
      case Tokenizer::TYPE_FLOAT:
      case Tokenizer::TYPE_STRING:
        LOG(ERROR) << "Expected field, got literal " << Token.text;
        return false;
      case Tokenizer::TYPE_SYMBOL:
        if (nested && Token.text == "}") {
          // Exit current message.
          return true;
        }
        LOG(ERROR) << "Expected field name or EOM, got " << Token.text;
        return false;
      case Tokenizer::TYPE_START:
      case Tokenizer::TYPE_END:
        LOG(FATAL) << "cannot happen";
        break;
      default:
        break;
    }
  }
  return true;
}

bool ParseTextProtoHandler::ParseFieldValue(
    const clang::CXXMethodDecl& accessor_decl) {
  if (!TextTokenizer.Next()) {
    LOG(ERROR) << "Expected field value, got EOF";
    return false;
  }
  const Tokenizer::Token& Token = TextTokenizer.current();
  switch (Token.type) {
    case Tokenizer::TYPE_IDENTIFIER:
      LOG(ERROR) << "Unexpected identifier " << Token.text;
      break;
    case Tokenizer::TYPE_INTEGER:
    case Tokenizer::TYPE_FLOAT:
    case Tokenizer::TYPE_STRING:
      LOG(ERROR) << "Expected separator, got " << Token.text;
      return false;
    case Tokenizer::TYPE_SYMBOL:
      if (Token.text == "{") {
        // Enter message: Use the accessor's return type as new base.
        const auto* SubMsgDecl =
            accessor_decl.getReturnType()->getPointeeCXXRecordDecl();
        if (!SubMsgDecl) {
          LOG(ERROR) << "Expected msg subfield, got "
                     << accessor_decl.getName().str();
          return false;
        }
        return ParseMsg(*SubMsgDecl, true);
      } else if (Token.text == ":") {
        // Parse one literal.
        TextTokenizer.Next();
        const Tokenizer::Token& LiteralToken = TextTokenizer.current();
        if (!(LiteralToken.type == Tokenizer::TYPE_INTEGER ||
              LiteralToken.type == Tokenizer::TYPE_FLOAT ||
              LiteralToken.type == Tokenizer::TYPE_STRING ||
              LiteralToken.type == Tokenizer::TYPE_IDENTIFIER)) {
          LOG(ERROR) << "Expected literal, got " << LiteralToken.text;
          return false;
        }
        return true;
      }
      LOG(ERROR) << "Expected separator, got " << Token.text;
      return false;
    case Tokenizer::TYPE_START:
    case Tokenizer::TYPE_END:
      LOG(FATAL) << "cannot happen";
      break;
    default:
      break;
  }
  return true;
}

clang::SourceLocation ParseTextProtoHandler::GetSourceLocation(
    const LineColumnPair& LineColumn) const {
  const auto OffsetIt = LineColumnToOffset.find(LineColumn);
  if (OffsetIt == LineColumnToOffset.end()) {
    return clang::SourceLocation();
  }
  return Context.getSourceManager().getSpellingLoc(
      Literal->getLocationOfByte(OffsetIt->second, Context.getSourceManager(),
                                 LangOpts, Context.getTargetInfo()));
}

clang::SourceRange ParseTextProtoHandler::GetTokenSourceRange(
    const Tokenizer::Token& Token) const {
  return clang::SourceRange(GetSourceLocation({Token.line, Token.column}),
                            GetSourceLocation({Token.line, Token.end_column}));
}

const clang::RecordDecl* LookupRecordDecl(const clang::ASTContext& ASTContext,
                                          const clang::DeclContext* Context,
                                          llvm::StringRef FullName) {
  while (Context && !FullName.empty()) {
    const std::pair<llvm::StringRef, llvm::StringRef> Parts =
        FullName.split("::");
    clang::IdentifierInfo& Identifier = ASTContext.Idents.get(Parts.first);
    const auto result = Context->lookup(&Identifier);
    if (result.empty() || result.front()->isInvalidDecl()) {
      return nullptr;
    }
    Context =
        clang::dyn_cast<clang::DeclContext>(result.front()->getCanonicalDecl());
    FullName = Parts.second;
  }
  return clang::dyn_cast<clang::RecordDecl>(Context);
}

}  // namespace

bool GoogleProtoLibrarySupport::CompilationUnitHasParseProtoHelperDecl(
    const clang::ASTContext& ASTContext, const clang::CallExpr& Expr) {
  if (!Initialized) {
    Initialized = true;
    // Find the root namespace.
    const clang::DeclContext* const TranslationUnitContext =
        Expr.getCalleeDecl()->getTranslationUnitDecl();
    // Look for ParseProtoHelper.
    ParseProtoHelperDecl =
        LookupRecordDecl(ASTContext, TranslationUnitContext,
                         absl::GetFlag(FLAGS_parseprotohelper_full_name));
  }
  return ParseProtoHelperDecl != nullptr;
}

void GoogleProtoLibrarySupport::InspectCallExpr(
    IndexerASTVisitor& V, const clang::CallExpr* CallExpr,
    const GraphObserver::Range& Range, const GraphObserver::NodeId& CalleeId) {
  if (!CompilationUnitHasParseProtoHelperDecl(V.getASTContext(), *CallExpr)) {
    // Return early if there is no ParseProtoHelper in the compilation unit.
    return;
  }

  // We are looking for the call to ParseProtoHelper::operator T(). This is the
  // only place where we know the target type (the type of the proto). We then
  // work backwards from there to the decl of the proto.
  const auto* const Expr = clang::dyn_cast<clang::CXXMemberCallExpr>(CallExpr);
  if (!Expr) {
    return;
  }

  if (Expr->getRecordDecl()->getCanonicalDecl() != ParseProtoHelperDecl) {
    return;
  }

  // TODO(courbet): Check that this is a call to a cast operator.

  // Messages are record types.
  if (!Expr->getType()->isRecordType()) {
    return;
  }

  const auto* ParseProtoExpr =
      Expr->getImplicitObjectArgument()->IgnoreParenImpCasts();
  // See through CXXBindTemporaryExpr if one should exist.
  if (const auto* const BindTemporary =
          clang::dyn_cast<clang::CXXBindTemporaryExpr>(ParseProtoExpr)) {
    ParseProtoExpr = BindTemporary->getSubExpr();
    if (ParseProtoExpr == nullptr) {
      return;
    }
  }

  const clang::StringLiteral* Literal = nullptr;
  if (const auto* const HelperCallExpr =
          clang::dyn_cast<clang::CallExpr>(ParseProtoExpr)) {
    // We're matching against a call to ParseTextProtoOrDie, a function
    // template that returns some proto type T via ParseProtoHelper's
    // operator T() (which performs the actual message parsing).
    // Get the inner string_view.
    if (HelperCallExpr->getNumArgs() < 1) {
      LOG(ERROR) << "Unknown signature for ParseTextProtoOrDie";
      return;
    }
    const auto* const StringViewCtorExpr =
        HelperCallExpr->getArg(0)->IgnoreParenImpCasts();
    // TODO(courbet): Handle the case when the string_view is not a temporary.
    if (const auto* const CxxConstruct =
            clang::dyn_cast<clang::CXXConstructExpr>(StringViewCtorExpr)) {
      // StringPiece(StringPiece&&) has a single parameter.
      if (CxxConstruct->getNumArgs() != 1) {
        return;
      }
      const auto* Arg = CxxConstruct->getArg(0)->IgnoreParenImpCasts();
      if (clang::isa<clang::CXXConstructExpr>(Arg)) {
        Arg = clang::dyn_cast<clang::CXXConstructExpr>(Arg)
                  ->getArg(0)
                  ->IgnoreParenImpCasts();
      }
      if (clang::isa<clang::StringLiteral>(Arg)) {
        Literal = clang::dyn_cast<clang::StringLiteral>(Arg);
      } else {
        // TODO(courbet): Handle the case when the input is not a string
        // literal.
        return;
      }
    }
  } else {
    // The intended ParseProtoHelper usage is a temporary contructed right
    // before calling the cast operator. We don't support other usages.
    return;
  }

  if (Literal == nullptr) {
    LOG(ERROR) << "No string literal found";
    return;
  }

  const auto Callback = [&V](const clang::CXXMethodDecl& AccessorDecl,
                             const clang::SourceRange& Range) {
    if (const auto RCC = V.ExplicitRangeInCurrentContext(Range)) {
      const auto NodeId = V.BuildNodeIdForDecl(&AccessorDecl);
      V.getGraphObserver().recordDeclUseLocation(
          *RCC, NodeId, GraphObserver::Claimability::Unclaimable,
          V.IsImplicit(*RCC));
    }
  };
  ParseTextProtoHandler::Parse(
      Callback, Literal, *Expr->getType()->getAsCXXRecordDecl(),
      V.getASTContext(), *V.getGraphObserver().getLangOptions());
}

}  // namespace kythe
