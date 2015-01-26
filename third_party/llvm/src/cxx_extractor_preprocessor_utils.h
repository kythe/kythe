// cxx_extractor_preprocessor_utils.h - Preprocessor utilities -*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===--------------------------------------------------------------------===//
/// \file
/// This file contains code from PreprocessorTracker.cpp from the modularize
/// project in clang-tools-extra; more details on its provenance and license
/// are available in //third_party/llvm.
//===--------------------------------------------------------------------===//
// This file uses the Clang style conventions.
#ifndef KYTHE_CXX_EXTRACTOR_EXTRACTOR_PREPROCESSOR_UTILS_H_
#define KYTHE_CXX_EXTRACTOR_EXTRACTOR_PREPROCESSOR_UTILS_H_

#include "clang/Lex/MacroArgs.h"
#include "clang/Lex/Preprocessor.h"

namespace kythe {
/// \brief Get the string for the Unexpanded macro instance.
///
/// The sourceRange is expected to end at the last token
/// for the macro instance, which in the case of a function-style
/// macro will be a ')', but for an object-style macro, it
/// will be the macro name itself.
std::string getMacroUnexpandedString(clang::SourceRange Range,
                                     clang::Preprocessor &PP,
                                     llvm::StringRef MacroName,
                                     const clang::MacroInfo *MI);

/// \brief Get the expansion for a macro instance, given the information
/// provided by PPCallbacks.
///
/// FIXME: This doesn't support function-style macro instances
/// passed as arguments to another function-style macro. However,
/// since it still expands the inner arguments, it still
/// allows modularize to effectively work with respect to macro
/// consistency checking, although it displays the incorrect
/// expansion in error messages.
std::string getMacroExpandedString(clang::Preprocessor &PP,
                                   llvm::StringRef MacroName,
                                   const clang::MacroInfo *MI,
                                   const clang::MacroArgs *Args);

/// \brief Returns the source text in `Range` with whitespace trimmed off.
/// \param Range The source range to return text from.
/// \param PP The preprocessor instance to interrogate.
std::string getSourceString(clang::Preprocessor &PP, clang::SourceRange Range);
}

#endif
