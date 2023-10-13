/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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

#include "kythe/cxx/indexer/cxx/marked_source.h"

#include <optional>

#include "absl/flags/flag.h"
#include "absl/log/check.h"
#include "absl/log/log.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/DeclVisitor.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/Basic/FileManager.h"
#include "clang/Format/Format.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Sema/Template.h"
#include "kythe/cxx/common/scope_guard.h"
#include "kythe/cxx/indexer/cxx/clang_range_finder.h"
#include "kythe/cxx/indexer/cxx/clang_utils.h"

// TODO(shahms): This is part of the Abseil logging migration.
// Support for VLOG is planned, but not yet implemented.
// Elsewhere, VLOG(1) maps to DLOG(LEVEL(-1)) until it is,
// but VLOG_IS_ON is only used in this file, so just disable it entirely
// until VLOG is supported properly, at which point this will be removed.
#define VLOG_IS_ON(x) false

ABSL_FLAG(bool, experimental_new_marked_source, false,
          "Use new signature generation.");

namespace kythe {
namespace {
/// \return true if `range` is valid for use in annotations.
bool IsValidRange(const clang::SourceManager& source_manager,
                  const clang::SourceRange& range) {
  // Check that the range is valid and ends in macros xnor files.
  if (!range.isValid() ||
      range.getBegin().isFileID() != range.getEnd().isFileID()) {
    return false;
  }
  // Reject definitions in macro expansions or that span multiple files.
  if (!range.getBegin().isFileID() ||
      source_manager.getFileID(range.getBegin()) !=
          source_manager.getFileID(range.getEnd())) {
    return false;
  }
  return true;
}

clang::SourceRange NormalizeRange(const clang::SourceManager& source_manager,
                                  const clang::LangOptions& lang_options,
                                  const clang::SourceRange& range) {
  return ClangRangeFinder(&source_manager, &lang_options).NormalizeRange(range);
}

llvm::StringRef GetTextRange(const clang::SourceManager& source_manager,
                             const clang::SourceRange& range) {
  if (!IsValidRange(source_manager, range)) {
    return llvm::StringRef();
  }
  const char* begin = source_manager.getCharacterData(range.getBegin());
  const char* end = source_manager.getCharacterData(range.getEnd());
  if (begin > end) {
    return llvm::StringRef();
  }
  return llvm::StringRef(begin, end - begin);
}

/// \brief A span of source text with some attached properties.
struct Annotation {
  enum Kind : unsigned char {
    TokenText,
    ArgListWithParens,
    Type,
    QualifiedName,
    Init
  };
  Kind kind;
  size_t begin;
  size_t end;
  bool operator<(const Annotation& o) const {
    return std::tie(begin, o.end, kind) < std::tie(o.begin, end, o.kind);
  }
};

/// \brief Used to manage the process of building `MarkedSource` trees.
class NodeStack {
 public:
  /// \brief Copy data from `annotations` and `formatted_range` to
  /// `dest_source`.
  /// \return the MarkedSource node covering an identifier, or null.
  MarkedSource* ProcessAnnotations(absl::string_view formatted_range,
                                   const std::vector<Annotation>& annotations,
                                   MarkedSource* dest_source) {
    /// For certain kinds of annotations, we'll substitute our own special
    /// MarkedSource. When we enter one of these, cancel_count gets
    /// incremented; when we exit, it gets decremented.
    size_t cancel_count = 0;
    MarkedSource* ident_node = nullptr;
    size_t cursor = 0;
    // There's always at least one annotation. It spans the whole of
    // formatted_range_ and it's ordered before the other annotations.
    size_t annotation = 0;
    for (;;) {
      size_t next_begin = (annotation == annotations.size())
                              ? formatted_range.size()
                              : annotations[annotation].begin;
      while (!nodes_.empty() && nodes_.top().annotation->end <= next_begin) {
        if (cursor < nodes_.top().annotation->end) {
          AppendToTop(formatted_range, cancel_count, cursor,
                      nodes_.top().annotation->end, true);
          cursor = nodes_.top().annotation->end;
        }
        if (nodes_.top().annotation->kind == Annotation::QualifiedName ||
            nodes_.top().annotation->kind == Annotation::ArgListWithParens) {
          --cancel_count;
        }
        nodes_.pop();
      }
      if (annotation == annotations.size()) {
        break;
      }
      const auto& next = annotations[annotation++];
      if (cursor < next_begin) {
        AppendToTop(formatted_range, cancel_count, cursor, next_begin, false);
        cursor = next_begin;
      }
      nodes_.push(Node{&next, annotation == 1
                                  ? dest_source
                                  : nodes_.top().marked_source->add_child()});
      auto* child = nodes_.top().marked_source;
      switch (next.kind) {
        case Annotation::TokenText:
          break;
        case Annotation::ArgListWithParens: {
          child->set_kind(MarkedSource::PARAMETER_LOOKUP_BY_PARAM);
          child->set_pre_text("(");
          child->set_post_child_text(", ");
          child->set_post_text(")");
          ++cancel_count;
          break;
        }
        case Annotation::Type:
          child->set_kind(MarkedSource::TYPE);
          break;
        case Annotation::Init:
          child->set_kind(MarkedSource::INITIALIZER);
          break;
        case Annotation::QualifiedName:
          child->set_kind(MarkedSource::BOX);
          ident_node = child;
          ++cancel_count;
          break;
      }
    }
    return ident_node;
  }

 private:
  /// \brief called to append raw text to the annotation span on the top of
  /// the node stack.
  ///
  /// This will happen in two cases:
  ///   - there is text between the current cursor position and the end of
  ///     the current span, and that span is going to be popped from the stack
  ///     immediately after append_to_top finishes (at_end is true).
  ///     No more children will be added to this span before it's popped.
  ///   - there is text between the current cursor position and the start
  ///     of the next span, and that next span will be pushed to the stack
  ///     immediately after append_to_top finishes (at_end is false).
  ///
  /// \param formatted_range the formatted text range
  /// \param cancel_count the number of cancelling annotations we're
  /// underneath
  /// \param start the start offset in the formatted text range
  /// \param end the end offset (exclusive) in the formatted text range
  /// \param at_end whether the span on the top of the stack is about to be
  /// popped because there are no other spans that get opened before the
  /// current span closes.
  void AppendToTop(absl::string_view formatted_range, size_t cancel_count,
                   size_t start, size_t end, bool at_end) {
    absl::string_view text = formatted_range.substr(start, end - start);
    CHECK(!nodes_.empty());
    if (cancel_count != 0) {
      return;
    }
    const auto* annotation = nodes_.top().annotation;
    auto* node = nodes_.top().marked_source;
    if (at_end) {
      if (node->child().empty() && node->post_text().empty()) {
        node->mutable_pre_text()->append(text);
      } else {
        node->mutable_post_text()->append(text);
      }
    } else {
      // If there are children before this node, we need to add a new BOX
      // to hold this token.
      if (node->child().empty()) {
        node->mutable_pre_text()->append(text);
      } else {
        auto* new_node = node->add_child();
        new_node->mutable_pre_text()->append(text);
      }
    }
  }
  /// A currently-entered annotation.
  struct Node {
    const Annotation* annotation;
    MarkedSource* marked_source;
  };
  /// The stack of currently entered annotations and their corresponding
  /// MarkedSource messages.
  std::stack<Node> nodes_;
};

/// \brief Try to get a return type range for a function that returns a function
/// pointer.
///
/// getReturnTypeSourceRange doesn't work for functions that return
/// pointers to functions. This makes some sense, given that these look
/// like this:
///   float (*bam(short function_arg))(int function_ptr_arg)
/// We still want to grovel through the type to pick the right bit out
/// of, e.g.:
///   virtual float (*bam(short function_arg) const)(int function_ptr_arg) = 0;
clang::SourceRange GetReturnTypeSourceRangeForFunctionPointerReturningFunction(
    const clang::FunctionDecl* decl) {
  if (const auto* type_info = decl->getTypeSourceInfo()) {
    if (auto function_type = type_info->getTypeLoc()
                                 .IgnoreParens()
                                 .getAs<clang::FunctionTypeLoc>()) {
      if (auto return_loc = function_type.getReturnLoc()) {
        if (auto as_ptr =
                return_loc.IgnoreParens().getAs<clang::PointerTypeLoc>()) {
          if (auto as_fn = as_ptr.getPointeeLoc()
                               .IgnoreParens()
                               .getAs<clang::FunctionTypeLoc>()) {
            return as_fn.getSourceRange();
          }
        }
      }
    }
  }
  return {};
}
bool SameFileRangesOverlapOpenInterval(const clang::SourceRange& outer,
                                       const clang::SourceRange& inner) {
  return outer.isValid() && inner.isValid() &&
         outer.getBegin().getRawEncoding() <=
             inner.getBegin().getRawEncoding() &&
         outer.getEnd().getRawEncoding() > inner.getBegin().getRawEncoding();
}
/// \brief Walks the AST to annotate source text.
///
/// The AST refers to source locations in an un-reformatted buffer, so we need
/// to transform them (using `replacements`) to refer to offsets in the
/// reformatted buffer.
class DeclAnnotator : public clang::DeclVisitor<DeclAnnotator> {
 public:
  DeclAnnotator(MarkedSourceCache* cache,
                clang::tooling::Replacements* replacements,
                clang::SourceLocation original_begin,
                const std::string& formatted_range, MarkedSource* marked_source,
                const clang::SourceRange& default_name_range)
      : cache_(cache),
        replacements_(replacements),
        original_begin_(original_begin),
        formatted_range_(formatted_range),
        marked_source_(marked_source),
        name_range_(default_name_range) {
    // Make it invariant that we should always be inside an annotation.
    annotations_.push_back(
        Annotation{Annotation::TokenText, 0, formatted_range.size()});
    // Remember the location of the qualified name.
    InsertAnnotation(default_name_range, Annotation{Annotation::QualifiedName});
  }
  MarkedSource* ident_node() {
    return ident_node_ ? ident_node_ : marked_source_;
  }
  // \brief Insert one or more type annotations.
  // \param type_range the source range covering the type.
  // \param arg_list the source range covering the argument list for functions,
  // or an invalid range otherwise.
  //
  // This function will split the type annotation range if the identifier for
  // the decl being annotated is inside the range (e.g., char x[] = {1};).
  void InsertTypeAnnotation(const clang::SourceRange& type_range,
                            clang::SourceRange arg_list) {
    clang::SourceRange type_lhs = type_range;
    clang::SourceRange type_rhs = type_lhs;
    // By default, we assume that nested ranges imply nested annotation nodes.
    // If we come across (e.g.) a function returning a function pointer, this
    // will give suboptimal results. We'll instead split the type range if we
    // find that it contains parameters or the decl name.
    //
    // TODO(zarko): Even if we perform this split, we'll end up with a
    // type signature for
    //   virtual float (*bam(short a) const)(int b) = 0;
    // that looks like `float (* const)(int b)`
    // There are other places this can happen too, like:
    //   float (*bam(short a) const &)(int b) = 0;
    // which will yield `float (* const &)(int b)`.
    // It appears that we'll need to parse the text manually looking for
    // the closing paren to determine where type_rhs should begin.
    if (SameFileRangesOverlapOpenInterval(type_lhs, name_range_)) {
      type_rhs = clang::SourceRange(name_range_.getEnd(), type_lhs.getEnd());
      type_lhs =
          clang::SourceRange(type_lhs.getBegin(), name_range_.getBegin());
    }
    if (SameFileRangesOverlapOpenInterval(type_rhs, arg_list)) {
      if (type_lhs == type_rhs) {
        type_lhs = clang::SourceRange(type_lhs.getBegin(), arg_list.getBegin());
      }
      type_rhs = clang::SourceRange(arg_list.getEnd(), type_rhs.getEnd());
    }
    if (type_lhs != type_rhs) {
      InsertAnnotation(type_rhs, Annotation{Annotation::Type});
    }
    InsertAnnotation(type_lhs, Annotation{Annotation::Type});
  }
  void VisitVarDecl(clang::VarDecl* decl) {
    if (const auto* type_source_info = decl->getTypeSourceInfo()) {
      if (!ShouldSkipDecl(decl, type_source_info->getType(),
                          type_source_info->getTypeLoc().getSourceRange())) {
        auto type_loc =
            NormalizeRange(cache_->source_manager(), cache_->lang_options(),
                           type_source_info->getTypeLoc().getSourceRange());
        InsertTypeAnnotation(type_loc, clang::SourceRange{});
      }
    }
    if (absl::GetFlag(FLAGS_experimental_new_marked_source)) {
      if (const auto* init = decl->getInit()) {
        auto init_range =
            NormalizeRange(cache_->source_manager(), cache_->lang_options(),
                           init->getSourceRange());
        InsertAnnotation(init_range, Annotation{Annotation::Init});
      }
    }
  }
  void VisitFieldDecl(clang::FieldDecl* decl) {
    if (const auto* type_source_info = decl->getTypeSourceInfo()) {
      if (!ShouldSkipDecl(decl, type_source_info->getType(),
                          type_source_info->getTypeLoc().getSourceRange())) {
        auto type_loc =
            NormalizeRange(cache_->source_manager(), cache_->lang_options(),
                           type_source_info->getTypeLoc().getSourceRange());
        InsertTypeAnnotation(type_loc, clang::SourceRange{});
      }
    }
  }
  void VisitObjCPropertyDecl(clang::ObjCPropertyDecl* decl) {
    if (const auto* type_source_info = decl->getTypeSourceInfo()) {
      if (!ShouldSkipDecl(decl, type_source_info->getType(),
                          type_source_info->getTypeLoc().getSourceRange())) {
        auto type_loc =
            NormalizeRange(cache_->source_manager(), cache_->lang_options(),
                           type_source_info->getTypeLoc().getSourceRange());
        InsertTypeAnnotation(type_loc, clang::SourceRange{});
      }
    }
  }
  void VisitFunctionDecl(clang::FunctionDecl* decl) {
    clang::SourceRange arg_list;
    if (const auto* type_info = decl->getTypeSourceInfo()) {
      if (!ShouldSkipDecl(decl, type_info->getType(),
                          type_info->getTypeLoc().getSourceRange())) {
        if (auto function_type = type_info->getTypeLoc()
                                     .IgnoreParens()
                                     .getAs<clang::FunctionTypeLoc>()) {
          arg_list =
              NormalizeRange(cache_->source_manager(), cache_->lang_options(),
                             function_type.getParensRange());
          InsertAnnotation(arg_list, Annotation{Annotation::ArgListWithParens});
        }
      }
    }
    auto type_range = decl->getReturnTypeSourceRange();
    if (!type_range.isValid()) {
      type_range =
          GetReturnTypeSourceRangeForFunctionPointerReturningFunction(decl);
    }
    if (!ShouldSkipDecl(decl, decl->getReturnType(), type_range) &&
        type_range.isValid()) {
      InsertTypeAnnotation(NormalizeRange(cache_->source_manager(),
                                          cache_->lang_options(), type_range),
                           arg_list);
    }
  }

  void VisitObjCMethodDecl(clang::ObjCMethodDecl* decl) {
    // TODO(salguarneri) Do something sensible for selectors and arguments.
    // Selectors are effectively the name of the method, but the selectors are
    // interrupted in source code by parameters, so we don't have a single range
    // for the method name or the method parameters. For example:
    // -(void) myFunc:(int)size withTimeout:(int)time. The "name" should be
    // myFunc:withTimeout and the arguments should be something like
    // "(int)size, (int)time".
    auto ret_type_range =
        NormalizeRange(cache_->source_manager(), cache_->lang_options(),
                       decl->getReturnTypeSourceRange());
    if (ret_type_range.isValid()) {
      InsertAnnotation(ret_type_range, Annotation{Annotation::Type});
    } else {
      DLOG(LEVEL(-1)) << "Invalid return type range for "
                      << decl->getNameAsString();
    }
  }

  void Annotate(const clang::NamedDecl* named_decl) {
    Visit(const_cast<clang::NamedDecl*>(named_decl));
    CompleteMarkedSource();
  }

 private:
  /// \brief Convert the annotations we've found to `MarkedSource`.
  void CompleteMarkedSource() {
    // We can get overlapping annotation ranges because of (for example)
    // the bizarre concrete syntax for function pointers.
    std::sort(annotations_.begin(), annotations_.end());
    NodeStack node_stack;
    if (auto* ident_node = node_stack.ProcessAnnotations(
            formatted_range_, annotations_, marked_source_)) {
      ident_node_ = ident_node;
    }
  }

  /// \brief adds an annotation to the annotation list, transforming
  /// offsets from original source to reformatted source.
  void InsertAnnotation(const clang::SourceRange& original_range,
                        Annotation&& annotation) {
    unsigned start_offset = original_range.getBegin().getRawEncoding() -
                            original_begin_.getRawEncoding();
    unsigned end_offset;
    if (original_range.getBegin() == original_range.getEnd()) {
      end_offset = start_offset;
    } else {
      end_offset = original_range.getEnd().getRawEncoding() -
                   original_begin_.getRawEncoding();
    }
    if (replacements_ != nullptr) {
      annotation.begin = replacements_->getShiftedCodePosition(start_offset);
      annotation.end = replacements_->getShiftedCodePosition(end_offset);
    } else {
      annotation.begin = start_offset;
      annotation.end = end_offset;
    }
    if (annotation.begin >= annotation.end ||
        annotation.end > formatted_range_.size()) {
      // TODO(#1632): This is a symptom of #1632. This check is here to avoid
      // clogging log output.
      if (annotation.kind != Annotation::QualifiedName &&
          IsValidRange(cache_->source_manager(), original_range)) {
        DLOG(LEVEL(-1))
            << "Invalid annotation range (" << annotation.kind << "): '"
            << original_range.getBegin().printToString(cache_->source_manager())
            << "' to '"
            << original_range.getEnd().printToString(cache_->source_manager())
            << "': became " << annotation.begin << " <= " << annotation.end
            << " <= " << formatted_range_.size()
            << " (text: " << formatted_range_ << ")";
      }
      return;
    }
    annotations_.push_back(annotation);
  }

  /// \brief determines if we should skip trying to record an annotation for
  /// this decl.
  ///
  /// In Objective-C, the nullability attribute is troublesome because:
  /// 1) The range we get for the Decl is backwards (goes from the e to the n in
  /// nullable).
  /// 2) The token is transformed to _Nullable by the time we analyze. nullable
  /// is placed to the left of types, _Nullable is placed to the right of types.
  bool ShouldSkipDecl(const clang::Decl* decl, const clang::QualType& qt,
                      const clang::SourceRange& sr) {
    std::optional<clang::NullabilityKind> k = qt->getNullability();
    return k && sr.getBegin().getRawEncoding() > sr.getEnd().getRawEncoding();
  }

  MarkedSourceCache* cache_;
  clang::tooling::Replacements* replacements_;
  clang::SourceLocation original_begin_;
  const std::string& formatted_range_;
  MarkedSource* marked_source_;
  const clang::SourceRange& name_range_;
  MarkedSource* ident_node_ = nullptr;
  std::vector<Annotation> annotations_;
};

std::string GetDeclName(const clang::LangOptions& lang_options,
                        const clang::NamedDecl* decl) {
  auto name = decl->getDeclName();
  auto identifier_info = name.getAsIdentifierInfo();
  if (identifier_info && !identifier_info->getName().empty()) {
    return std::string(identifier_info->getName());
  } else if (name.getCXXOverloadedOperator() != clang::OO_None) {
    return name.getAsString();
  } else if (const auto* method_decl =
                 llvm::dyn_cast<clang::CXXMethodDecl>(decl)) {
    if (llvm::isa<clang::CXXConstructorDecl>(method_decl)) {
      return "(ctor)";
    } else if (llvm::isa<clang::CXXDestructorDecl>(method_decl)) {
      return "(dtor)";
    } else if (const auto* conv_decl =
                   llvm::dyn_cast<clang::CXXConversionDecl>(method_decl)) {
      auto to_type = conv_decl->getConversionType();
      if (!to_type.isNull()) {
        std::string substring;
        llvm::raw_string_ostream substream(substring);
        substream << "operator ";
        to_type.print(substream, clang::PrintingPolicy(lang_options));
        substream.flush();
        return substring;
      }
    }
  } else if (isObjCSelector(name)) {
    const auto sel = name.getObjCSelector();
    return sel.getAsString();
  }
  return "";
}

void CleanMarkedSource(MarkedSource* to_clean) {
  switch (to_clean->kind()) {
    case MarkedSource::BOX: {
      if (to_clean->post_child_text().empty()) {
        to_clean->set_post_child_text(" ");
      }
      to_clean->clear_pre_text();
      to_clean->clear_post_text();
    } break;
    default:
      break;
  }
  for (auto& child : *to_clean->mutable_child()) {
    CleanMarkedSource(&child);
  }
}
}  // anonymous namespace

bool MarkedSourceGenerator::WillGenerateMarkedSource() const {
  // Be conservative in which kinds of marked source we'll generate.
  // We can enable more AST node flavors as necessary.
  if (decl_->isImplicit() || implicit_) {
    return false;
  }
  return llvm::isa<clang::FunctionDecl>(decl_) ||
         llvm::isa<clang::VarDecl>(decl_) ||
         llvm::isa<clang::NamespaceDecl>(decl_) ||
         llvm::isa<clang::TagDecl>(decl_) ||
         llvm::isa<clang::TypedefNameDecl>(decl_) ||
         llvm::isa<clang::FieldDecl>(decl_) ||
         llvm::isa<clang::EnumConstantDecl>(decl_) ||
         llvm::isa<clang::ObjCMethodDecl>(decl_) ||
         llvm::isa<clang::ObjCContainerDecl>(decl_) ||
         llvm::isa<clang::TemplateTypeParmDecl>(decl_) ||
         llvm::isa<clang::NonTypeTemplateParmDecl>(decl_) ||
         llvm::isa<clang::TemplateTemplateParmDecl>(decl_) ||
         llvm::isa<clang::ObjCTypeParamDecl>(decl_) ||
         llvm::isa<clang::ObjCPropertyDecl>(decl_);
}

void MarkedSourceGenerator::ReplaceMarkedSourceWithTemplateArgumentList(
    MarkedSource* marked_source_node,
    const clang::ClassTemplateSpecializationDecl* decl) {
  // While we try to figure out which template arguments are defaults, silence
  // diagnostics from the typechecker.
  clang::IgnoringDiagConsumer consumer;
  auto* diags = &cache_->source_manager().getDiagnostics();
  auto guard = MakeScopeGuard([diags = diags, client = diags->getClient(),
                               owned = diags->takeClient()]() mutable {
    diags->setClient(client, owned.release() != nullptr);
  });

  diags->setClient(&consumer, false);
  auto* template_decl = decl->getSpecializedTemplate();
  auto* template_params = template_decl->getTemplateParameters();
  auto cached_default = cache_->first_default_template_argument()->find(decl);
  const auto& template_args = decl->getTemplateArgs();
  unsigned noprint;
  if (cached_default != cache_->first_default_template_argument()->end()) {
    noprint = cached_default->second;
  } else {
    // Find a point N such that args[0..N-1] entirely predict args[N] and
    // beyond. Start by guessing that N = first_default (the case where all
    // default args are used).
    unsigned first_default = template_params->getMinRequiredArguments();
    clang::TemplateArgumentListInfo list_prefix;
    auto add_template_argument = [&](const clang::TemplateArgument& arg) {
      switch (arg.getKind()) {
        case clang::TemplateArgument::Null:
          // This argument has not been deduced.
          return false;
        case clang::TemplateArgument::Type:
          list_prefix.addArgument(clang::TemplateArgumentLoc(
              arg, cache_->sema()->getASTContext().getTrivialTypeSourceInfo(
                       arg.getAsType())));
          return true;
        case clang::TemplateArgument::Declaration:
        case clang::TemplateArgument::NullPtr:
        case clang::TemplateArgument::Integral:
        case clang::TemplateArgument::Template:
        case clang::TemplateArgument::TemplateExpansion:
        case clang::TemplateArgument::Expression:
        case clang::TemplateArgument::Pack:
          // TODO(zarko): Remaining cases.
          return false;
      }
    };
    for (unsigned n = 0; n < first_default; ++n) {
      if (!add_template_argument(template_args.get(n))) {
        // Abort if we can't complete a template_args list.
        first_default = template_args.size();
        break;
      }
    }
    llvm::SmallVector<clang::TemplateArgument, 4> out_arguments;
    llvm::SmallVector<clang::TemplateArgument, 4> sugared_arguments;
    noprint = first_default;
    for (; noprint < template_args.size(); ++noprint) {
      bool was_ok = !cache_->sema()->CheckTemplateArgumentList(
          template_decl, template_decl->getLocation(), list_prefix, false,
          out_arguments, sugared_arguments);
      if (was_ok) {
        if (out_arguments.size() != template_args.size()) {
          break;
        }
        unsigned arg_index = 0;
        for (const auto& arg : out_arguments) {
          // TODO(zarko): for certain kinds of declarations, source_arg may be
          // a tyvar reference ('type-parameter-0-0'). Can we thread through
          // the type context in those cases?
          const auto& source_arg = template_args.get(arg_index);
          if (arg.structurallyEquals(source_arg)) {
            ++arg_index;
          } else {
            break;
          }
        }
        if (arg_index == template_args.size()) {
          break;
        }
      }
      // Integral template arguments cause an assert failure inside clang.
      if (template_args.get(noprint).getKind() ==
          clang::TemplateArgument::Integral)
        break;
      add_template_argument(template_args.get(noprint));
    }
    (*cache_->first_default_template_argument())[decl] = noprint;
  }
  auto* typarams = marked_source_node->add_child();
  typarams->set_kind(MarkedSource::PARAMETER);
  typarams->set_pre_text("<");
  typarams->set_post_child_text(", ");
  typarams->set_post_text(">");
  typarams->set_default_children_count(template_args.size() - noprint);
  auto policy = clang::PrintingPolicy(cache_->lang_options());
  for (const auto& print_arg : template_args.asArray()) {
    auto* next_arg = typarams->add_child();
    typarams->set_kind(MarkedSource::BOX);
    // TODO(zarko): Call ReplaceMarkedSourceWithQualifiedName recursively
    // instead of using the pretty printer? If we do this, we'll need to update
    // the type context.
    // TODO(zarko): Pack expansions;
    // see TemplateSpecializationType::PrintTemplateArgumentList
    std::string pre_text;
    {
      llvm::raw_string_ostream stream(pre_text);
      print_arg.print(policy, stream, true);
    }
    *next_arg->mutable_pre_text() = pre_text;
  }
}

bool MarkedSourceGenerator::ReplaceMarkedSourceWithQualifiedName(
    MarkedSource* node) {
  // We could also consider populating the context dynamically at serving
  // or denormalization time, but doing this requires unbounded recursive
  // queries, so it's probably not worth it.
  // See also TypePrinter::AppendScope, NestedNameSpecifier::print,
  // NamedDecl::printQualifiedName (from which this code is derived).

  // Collect contexts.
  const auto* decl_context = decl_->getDeclContext();
  llvm::SmallVector<const clang::DeclContext*, 8> contexts;
  while (decl_context && llvm::isa<clang::NamedDecl>(decl_context)) {
    contexts.push_back(decl_context);
    decl_context = decl_context->getParent();
  }

  MarkedSource* self = node;
  if (!contexts.empty()) {
    // Avoid creating an unnecessary BOX if there are no context nodes.
    auto* parents = node->add_child();
    self = node->add_child();
    parents->set_kind(MarkedSource::CONTEXT);
    parents->set_add_final_list_token(true);
    parents->set_post_child_text("::");
    auto policy = clang::PrintingPolicy(cache_->lang_options());
    for (const auto* decl_context : reverse(contexts)) {
      auto* parent = parents->add_child();
      if (const auto* spec =
              llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(
                  decl_context)) {
        parent->set_kind(MarkedSource::BOX);
        auto* class_name = parent->add_child();
        class_name->set_kind(MarkedSource::IDENTIFIER);
        std::string pre_text;
        {
          llvm::raw_string_ostream stream(pre_text);
          stream << spec->getName();
        }
        *class_name->mutable_pre_text() = pre_text;
        ReplaceMarkedSourceWithTemplateArgumentList(parent->add_child(), spec);
      } else {
        parent->set_kind(MarkedSource::IDENTIFIER);
        std::string pre_text;
        {
          llvm::raw_string_ostream stream(pre_text);
          if (const auto* namespace_decl =
                  llvm::dyn_cast<clang::NamespaceDecl>(decl_context)) {
            if (namespace_decl->isAnonymousNamespace()) {
              stream << (policy.MSVCFormatting ? "`anonymous namespace\'"
                                               : "(anonymous namespace)");
            } else {
              stream << *namespace_decl;
            }
          } else if (const auto* record_decl =
                         llvm::dyn_cast<clang::RecordDecl>(decl_context)) {
            if (!record_decl->getIdentifier())
              stream << "(anonymous " << record_decl->getKindName() << ')';
            else
              stream << *record_decl;
          } else if (const auto* function_decl =
                         llvm::dyn_cast<clang::FunctionDecl>(decl_context)) {
            stream << *function_decl;
          } else if (const auto* enum_decl =
                         llvm::dyn_cast<clang::EnumDecl>(decl_context)) {
            stream << *enum_decl;
          } else if (const auto* cat_decl =
                         llvm::dyn_cast<clang::ObjCCategoryDecl>(
                             decl_context)) {
            // Print categories methods as
            // 'InterfaceName(CategoryName)::Method'.
            if (const auto* i = cat_decl->getClassInterface()) {
              stream << i->getName();
            }
            stream << "(" << cat_decl->getName() << ")";
          } else if (const auto* cat_impl =
                         llvm::dyn_cast<clang::ObjCCategoryImplDecl>(
                             decl_context)) {
            // Print categories methods as
            // 'InterfaceName(CategoryName)::Method'.
            if (const auto* decl = cat_impl->getCategoryDecl()) {
              if (const auto* i = decl->getClassInterface()) {
                stream << i->getName();
              }
            }
            stream << "(" << cat_impl->getName() << ")";
          } else {
            stream << *llvm::cast<clang::NamedDecl>(decl_context);
          }
        }
        *parent->mutable_pre_text() = pre_text;
      }
    }
  }
  self->set_kind(MarkedSource::IDENTIFIER);
  self->set_pre_text(GetDeclName(cache_->lang_options(), decl_));
  return true;
}

std::optional<MarkedSource>
MarkedSourceGenerator::GenerateMarkedSourceUsingSource(
    const GraphObserver::NodeId& decl_id) {
  auto start_loc = decl_->getSourceRange().getBegin();
  if (start_loc.isMacroID()) {
    start_loc = cache_->source_manager().getExpansionLoc(start_loc);
  }
  auto end_loc = end_loc_.isMacroID()
                     ? cache_->source_manager().getExpansionLoc(end_loc_)
                     : end_loc_;
  auto range = GetTextRange(cache_->source_manager(),
                            clang::SourceRange(start_loc, end_loc));
  if (range.empty()) {
    if (VLOG_IS_ON(1)) {
      DLOG(LEVEL(-1)) << "GetTextRange failed for " << decl_->getDeclKindName()
                      << " " << decl_->getQualifiedNameAsString() << "\n at "
                      << start_loc.printToString(cache_->source_manager())
                      << "\n to "
                      << end_loc.printToString(cache_->source_manager())
                      << "\n originally "
                      << decl_->getSourceRange().getBegin().printToString(
                             cache_->source_manager())
                      << "\n         to "
                      << end_loc_.printToString(cache_->source_manager());
    }
    return std::nullopt;
  }
  MarkedSource out_sig;
  auto range_string = range.str();
  DeclAnnotator annotator(cache_, nullptr, start_loc, range_string, &out_sig,
                          name_range_);
  annotator.Annotate(decl_);
  ReplaceMarkedSourceWithQualifiedName(annotator.ident_node());
  if (absl::GetFlag(FLAGS_experimental_new_marked_source)) {
    CleanMarkedSource(&out_sig);
  }
  return out_sig;
}

MarkedSource MarkedSourceGenerator::GenerateMarkedSourceForNamedDecl() {
  MarkedSource out;
  ReplaceMarkedSourceWithQualifiedName(&out);
  return out;
}

std::optional<MarkedSource> MarkedSourceGenerator::GenerateMarkedSource(
    const GraphObserver::NodeId& decl_id) {
  // MarkedSource generation is expensive. If we're not going to write out the
  // marked source later on, don't spend time on it.
  // TODO(zarko): Introduce a similar check for documentation.
  if (!WillGenerateMarkedSource()) {
    return std::nullopt;
  }
  if (llvm::isa<clang::VarDecl>(decl_) || llvm::isa<clang::FieldDecl>(decl_)) {
    return GenerateMarkedSourceUsingSource(decl_id);
  } else if (const auto* func = llvm::dyn_cast<clang::FunctionDecl>(decl_)) {
    return GenerateMarkedSourceUsingSource(decl_id);
  } else if (llvm::isa<clang::ObjCPropertyDecl>(decl_)) {
    return GenerateMarkedSourceUsingSource(decl_id);
  } else if (llvm::isa<clang::ObjCMethodDecl>(decl_)) {
    return GenerateMarkedSourceUsingSource(decl_id);
  } else if (llvm::isa<clang::TemplateTypeParmDecl>(decl_) ||
             llvm::isa<clang::NonTypeTemplateParmDecl>(decl_) ||
             llvm::isa<clang::TemplateTemplateParmDecl>(decl_) ||
             llvm::isa<clang::ObjCTypeParamDecl>(decl_)) {
    MarkedSource self;
    self.set_kind(MarkedSource::IDENTIFIER);
    self.set_pre_text(GetDeclName(cache_->lang_options(), decl_));
    return self;
  }
  return GenerateMarkedSourceForNamedDecl();
}
}  // namespace kythe
