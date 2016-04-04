// cxx_extractor_preprocessor_utils.cc - Preprocessor utilities -*- C++ -*--==//
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
#include "cxx_extractor_preprocessor_utils.h"

#include "clang/Lex/Preprocessor.h"

namespace kythe {

std::string getMacroUnexpandedString(clang::SourceRange Range,
                                     clang::Preprocessor &PP,
                                     llvm::StringRef MacroName,
                                     const clang::MacroInfo *MI) {
  clang::SourceLocation BeginLoc(Range.getBegin());
  const char *BeginPtr = PP.getSourceManager().getCharacterData(BeginLoc);
  size_t Length;
  std::string Unexpanded;
  if (MI->isFunctionLike()) {
    clang::SourceLocation EndLoc(Range.getEnd());
    const char *EndPtr = PP.getSourceManager().getCharacterData(EndLoc) + 1;
    Length = (EndPtr - BeginPtr) + 1; // +1 is ')' width.
  } else
    Length = MacroName.size();
  return llvm::StringRef(BeginPtr, Length).trim().str();
}

// Changed vs. modularize: added Visited set.
std::string getMacroExpandedStringInternal(
    clang::Preprocessor &PP, llvm::StringRef MacroName,
    const clang::MacroInfo *MI, const clang::MacroArgs *Args,
    std::set<const clang::MacroInfo *> &Visited) {
  if (!Visited.insert(MI).second) {
    return MacroName.str();
  }
  std::string Expanded;
  // Walk over the macro Tokens.
  typedef clang::MacroInfo::tokens_iterator Iter;
  for (Iter I = MI->tokens_begin(), E = MI->tokens_end(); I != E; ++I) {
    clang::IdentifierInfo *II = I->getIdentifierInfo();
    int ArgNo = (II && Args ? MI->getArgumentNum(II) : -1);
    if (ArgNo == -1) {
      // This isn't an argument, just add it.
      if (II == nullptr)
        Expanded += PP.getSpelling((*I)); // Not an identifier.
      else {
        // Token is for an identifier.
        std::string Name = II->getName().str();
        // Check for nexted macro references.
        clang::MacroInfo *MacroInfo = PP.getMacroInfo(II);
        if (MacroInfo)
          Expanded += getMacroExpandedStringInternal(PP, Name, MacroInfo,
                                                     nullptr, Visited);
        else
          Expanded += Name;
      }
      continue;
    }
    // We get here if it's a function-style macro with arguments.
    const clang::Token *ResultArgToks;
    const clang::Token *ArgTok = Args->getUnexpArgument(ArgNo);
    if (Args->ArgNeedsPreexpansion(ArgTok, PP))
      ResultArgToks = &(const_cast<clang::MacroArgs *>(Args))
                           ->getPreExpArgument(ArgNo, MI, PP)[0];
    else
      ResultArgToks = ArgTok; // Use non-preexpanded Tokens.
    // If the arg token didn't expand into anything, ignore it.
    if (ResultArgToks->is(clang::tok::eof))
      continue;
    unsigned NumToks = clang::MacroArgs::getArgLength(ResultArgToks);
    // Append the resulting argument expansions.
    for (unsigned ArgumentIndex = 0; ArgumentIndex < NumToks; ++ArgumentIndex) {
      const clang::Token &AT = ResultArgToks[ArgumentIndex];
      clang::IdentifierInfo *II = AT.getIdentifierInfo();
      if (II == nullptr)
        Expanded += PP.getSpelling(AT); // Not an identifier.
      else {
        // It's an identifier.  Check for further expansion.
        std::string Name = II->getName().str();
        clang::MacroInfo *MacroInfo = PP.getMacroInfo(II);
        if (MacroInfo)
          Expanded += getMacroExpandedStringInternal(PP, Name, MacroInfo,
                                                     nullptr, Visited);
        else
          Expanded += Name;
      }
    }
  }
  return Expanded;
}

std::string getMacroExpandedString(clang::Preprocessor &PP,
                                   llvm::StringRef MacroName,
                                   const clang::MacroInfo *MI,
                                   const clang::MacroArgs *Args) {
  std::set<const clang::MacroInfo *> Visited;
  return getMacroExpandedStringInternal(PP, MacroName, MI, Args, Visited);
}

std::string getSourceString(clang::Preprocessor &PP, clang::SourceRange Range) {
  clang::SourceLocation BeginLoc = Range.getBegin();
  clang::SourceLocation EndLoc = Range.getEnd();
  if (!BeginLoc.isValid() || !EndLoc.isValid() ||
      BeginLoc.isFileID() != EndLoc.isFileID()) {
    return llvm::StringRef();
  }
  const char *BeginPtr = PP.getSourceManager().getCharacterData(BeginLoc);
  const char *EndPtr = PP.getSourceManager().getCharacterData(EndLoc);
  if (BeginPtr > EndPtr) {
    return llvm::StringRef();
  }
  size_t Length = EndPtr - BeginPtr;
  return llvm::StringRef(BeginPtr, Length).trim().str();
}
}
