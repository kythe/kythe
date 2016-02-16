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
// This file uses the Clang style conventions.

#include "CommandLineUtils.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Regex.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <mutex>
#include <string>
#include <vector>

namespace kythe {
namespace common {
namespace {

/// \brief A `llvm::Regex` wrapper that only performs full matches on
/// non-empty strings.
///
/// The second restriction makes it easier to write long chains of 'or'-ed
/// regular expressions which may contain empty options without those silently
/// matching empty strings.
class FullMatchRegex {
public:
  /// \param Regex an extended-syntax regex to match.
  explicit FullMatchRegex(llvm::StringRef Regex)
      : InnerRegex("^(" + Regex.str() + ")$", llvm::Regex::NoFlags) {
    std::string st;
    if (!InnerRegex.isValid(st)) {
      fprintf(stderr, "%s (regex was %s)\n", st.c_str(), Regex.str().c_str());
      assert(0 && "!InnerRegex.isValid()");
    }
  }

  /// \return true if `String` is nonempty and a full match of this regex.
  bool FullMatch(llvm::StringRef String) const {
    std::lock_guard<std::mutex> MutexLock(RegexMutex);
    llvm::SmallVector<llvm::StringRef, 1> Matches;
    return String.size() > 0 && InnerRegex.match(String, &Matches);
  }

private:
  mutable llvm::Regex InnerRegex;
  /// This mutex protects `InnerRegex`, since `llvm::Regex` is not threadsafe.
  mutable std::mutex RegexMutex;
};

} // anonymous namespace

// Decide what will the driver do based on the inputs found on the command
// line.
DriverAction DetermineDriverAction(const std::vector<std::string> &args) {
  const FullMatchRegex c_file_re("[^-].*\\.(c|i)");
  const FullMatchRegex cxx_file_re("[^-].*\\.(C|c\\+\\+|cc|cp|cpp|cxx|CPP|ii)");
  const FullMatchRegex fortran_file_re(
      "[^-].*\\.(f|for|ftn|F|FOR|fpp|FPP|FTN|f90|f95|f03|f08|F90|F95|F03|F08)");
  const FullMatchRegex go_file_re("[^-].*\\.go");
  const FullMatchRegex asm_file_re("[^-].*\\.(s|S|sx)");

  enum DriverAction action = UNKNOWN;
  bool is_link = true;
  for (size_t i = 0; i < args.size(); ++i) {
    const std::string &arg = args[i];
    if (arg == "-c") {
      is_link = false;
    } else if (arg == "-x" && i < args.size() - 1) {
      // If we find -x, the language is being overridden by the user.
      const std::string &language = args[i + 1];
      if (language == "c++" || language == "c++-header" ||
          language == "c++-cpp-output")
        action = CXX_COMPILE;
      else if (language == "c" || language == "c-header" ||
               language == "cpp-output")
        action = C_COMPILE;
      else if (language == "assembler" || language == "assembler-with-cpp")
        action = ASSEMBLY;
      else if (language == "f77" || language == "f77-cpp-input" ||
               language == "f95" || language == "f95-cpp-input")
        action = FORTRAN_COMPILE;
      else if (language == "go")
        action = GO_COMPILE;
    } else if (action == UNKNOWN) {
      // If we still have not recognized the input language, try to
      // recognize it from the input file (in order of relative frequency).
      if (cxx_file_re.FullMatch(arg)) {
        action = CXX_COMPILE;
      } else if (c_file_re.FullMatch(arg)) {
        action = C_COMPILE;
      } else if (asm_file_re.FullMatch(arg)) {
        action = ASSEMBLY;
      } else if (go_file_re.FullMatch(arg)) {
        action = GO_COMPILE;
      } else if (fortran_file_re.FullMatch(arg)) {
        action = FORTRAN_COMPILE;
      }
    }
  }

  // If the user did not specify -c, then the linker will be invoked.
  // Note that if the command line was something like "clang foo.cc",
  // it will be considered a LINK action.
  if (is_link)
    return LINK;

  return action;
}

// Returns true if a C or C++ source file (or other files we want Clang
// diagnostics for) appears in the given command line or args.
bool HasCxxInputInCommandLineOrArgs(
    const std::vector<std::string> &command_line_or_args) {
  const enum DriverAction action = DetermineDriverAction(command_line_or_args);
  return action == CXX_COMPILE || action == C_COMPILE;
}

// Returns a copy of the input vector with every string which matches the
// regular expression removed.
static std::vector<std::string>
CopyOmittingMatches(const FullMatchRegex &re,
                    const std::vector<std::string> &input) {
  std::vector<std::string> output;
  std::remove_copy_if(
      input.begin(), input.end(), back_inserter(output),
      [&re](const std::string &arg) { return re.FullMatch(arg); });
  return output;
}

// Returns a copy of the input vector after removing each string which matches
// the regular expression and one string immediately following the matching
// string.
static std::vector<std::string>
CopyOmittingMatchesAndFollowers(const FullMatchRegex &re,
                                const std::vector<std::string> &input) {
  std::vector<std::string> output;
  for (size_t i = 0; i < input.size(); ++i) {
    if (!re.FullMatch(input[i])) {
      output.push_back(input[i]);
    } else {
      ++i; // Skip the matching string *and* the next string.
    }
  }
  return output;
}

// Returns a copy of the input vector with the supplied prefix string removed
// from any element of which it was a prefix.
static std::vector<std::string>
StripPrefix(const std::string &prefix, const std::vector<std::string> &input) {
  std::vector<std::string> output;
  const size_t prefix_size = prefix.size();
  for (const auto &arg : input) {
    if (arg.compare(0, prefix_size, prefix) == 0) {
      output.push_back(arg.substr(prefix_size));
    } else {
      output.push_back(arg);
    }
  }
  return output;
}

std::vector<std::string>
GCCArgsToClangArgs(const std::vector<std::string> &gcc_args) {
  // These are GCC-specific arguments which Clang does not yet understand or
  // support without issuing ugly warnings, and cannot otherwise be suppressed.
  const FullMatchRegex unsupported_args_re(
      "-W(no-)?(error=)?coverage-mismatch"
      "|-W(no-)?(error=)?frame-larger-than.*"
      "|-W(no-)?(error=)?maybe-uninitialized"
      "|-W(no-)?(error=)?thread-safety"
      "|-W(no-)?(error=)?thread-unsupported-lock-name"
      "|-W(no-)?(error=)?unused-but-set-parameter"
      "|-W(no-)?(error=)?unused-but-set-variable"
      "|-W(no-)?(error=)?unused-local-typedefs"
      "|-Xgcc-only=.*"
      "|-enable-libstdcxx-debug"
      "|-f(no-)?align-functions.*"
      "|-f(no-)?asynchronous-unwind-tables"
      "|-f(no-)?builtin-.*"
      "|-f(no-)?callgraph-profiles-sections"
      "|-f(no-)?float-store"
      "|-f(no-)?canonical-system-headers"
      "|-f(no-)?eliminate-unused-debug-types"
      "|-f(no-)?gcse"
      "|-f(no-)?ident"
      "|-f(no-)?inline-small-functions"
      "|-f(no-)?ivopts"
      "|-f(no-)?non-call-exceptions"
      "|-f(no-)?optimize-locality"
      "|-f(no-)?permissive"
      "|-f(no-)?plugin-arg-.*"
      "|-f(no-)?plugin=.*"
      "|-f(no-)?prefetch-loop-arrays"
      "|-f(no-)?profile-correction"
      "|-f(no-)?profile-dir.*"
      "|-f(no-)?profile-generate.*"
      "|-f(no-)?profile-use.*"
      "|-f(no-)?profile-reusedist"
      "|-f(no-)?profile-values"
      "|-f(no-)?record-compilation-info-in-elf"
      "|-f(no-)?reorder-functions=.*"
      "|-f(no-)?rounding-math"
      "|-f(no-)?ripa"
      "|-f(no-)?ripa-disallow-asm-modules"
      "|-f(no-)?see"
      "|-f(no-)?strict-enum-precision"
      "|-f(no-)?tracer"
      "|-f(no-)?tree-.*"
      "|-f(no-)?unroll-all-loops"
      "|-f(no-)?warn-incomplete-patterns" // Why do we see this haskell flag?
      "|-g(:lines,source|gdb)"
      "|-m(no-)?align-double"
      "|-m(no-)?fpmath=.*"
      "|-m(no-)?cld"
      "|-m(no-)?red-zone"
      "|--param=.*"
      "|-mcpu=.*"  // For -mcpu=armv7-a, this leads to an assertion failure
                   // in llvm::ARM::getSubArch (and an error about an
                   // unsupported -mcpu); for cortex-a15, we get no such
                   // failure. TODO(zarko): Leave this filtered out for now,
                   // but figure out what to do to make this work properly.
      "|-mapcs-frame"
      "|-pass-exit-codes");
  const FullMatchRegex unsupported_args_with_values_re("-wrapper");

  return StripPrefix("-Xclang-only=",
                     CopyOmittingMatchesAndFollowers(
                         unsupported_args_with_values_re,
                         CopyOmittingMatches(unsupported_args_re, gcc_args)));
}

std::vector<std::string>
GCCArgsToClangSyntaxOnlyArgs(const std::vector<std::string> &gcc_args) {
  return AdjustClangArgsForSyntaxOnly(GCCArgsToClangArgs(gcc_args));
}

std::vector<std::string>
GCCArgsToClangAnalyzeArgs(const std::vector<std::string> &gcc_args) {
  return AdjustClangArgsForAnalyze(GCCArgsToClangArgs(gcc_args));
}

std::vector<std::string>
AdjustClangArgsForSyntaxOnly(const std::vector<std::string> &clang_args) {
  // These are arguments which are inapplicable to '-fsyntax-only' behavior, but
  // are applicable to regular compilation.
  const FullMatchRegex inapplicable_args_re(
      "--analyze"
      "|-CC?"
      "|-E"
      "|-L.*"
      "|-MM?D"
      "|-M[MGP]?"
      "|-S"
      "|-W[al],.*"
      "|-c"
      "|-f(no-)?data-sections"
      "|-f(no-)?function-sections"
      "|-f(no-)?omit-frame-pointer"
      "|-f(no-)?profile-arcs"
      "|-f(no-)?stack-protector(-all)?"
      "|-f(no-)?strict-aliasing"
      "|-f(no-)?test-coverage"
      "|-f(no-)?unroll-loops"
      "|-fsyntax-only" // We don't want multiple -fsyntax-only args.
      "|-g.+"
      "|-nostartfiles"
      "|-s"
      "|-shared");
  const FullMatchRegex inapplicable_args_with_values_re("-M[FTQ]"
                                                        "|-o");

  std::vector<std::string> result = CopyOmittingMatchesAndFollowers(
      inapplicable_args_with_values_re,
      CopyOmittingMatches(inapplicable_args_re, clang_args));
  result.push_back("-fsyntax-only");

  return result;
}

std::vector<std::string>
AdjustClangArgsForAnalyze(const std::vector<std::string> &clang_args) {
  // --analyze is just like -fsyntax-only, except for the name of the
  // flag itself.
  std::vector<std::string> args = AdjustClangArgsForSyntaxOnly(clang_args);
  std::replace(args.begin(), args.end(), std::string("-fsyntax-only"),
               std::string("--analyze"));

  // cfg-temporary-dtors is still off by default in the analyzer, but analyzing
  // that way would give us lots of false positives. This can go away once the
  // temporary destructors support switches to on.
  args.insert(args.end(), {"-Xanalyzer", "-analyzer-config", "-Xanalyzer",
                           "cfg-temporary-dtors=true"});

  return args;
}

std::vector<std::string>
ClangArgsToGCCArgs(const std::vector<std::string> &clang_args) {
  // These are Clang-specific args which GCC does not understand.
  const FullMatchRegex unsupported_args_re(
      "--target=.*"
      "|-W(no-)?(error=)?ambiguous-member-template"
      "|-W(no-)?(error=)?bind-to-temporary-copy"
      "|-W(no-)?(error=)?bool-conversions"
      "|-W(no-)?(error=)?c\\+\\+0x-static-nonintegral-init"
      "|-W(no-)?(error=)?constant-conversion"
      "|-W(no-)?(error=)?constant-logical-operand"
      "|-W(no-)?(error=)?gnu"
      "|-W(no-)?(error=)?gnu-designator"
      "|-W(no-)?(error=)?initializer-overrides"
      "|-W(no-)?(error=)?invalid-noreturn"
      "|-W(no-)?(error=)?local-type-template-args"
      "|-W(no-)?(error=)?mismatched-tags"
      "|-W(no-)?(error=)?null-dereference"
      "|-W(no-)?(error=)?out-of-line-declaration"
      "|-W(no-)?(error=)?really-dont-use-clang-diagnostics"
      "|-W(no-)?(error=)?tautological-compare"
      "|-W(no-)?(error=)?unknown-attributes"
      "|-W(no-)?(error=)?unnamed-type-template-args"
      "|-W(no-)?(error=)?thread-safety-.*"
      "|-Xclang=.*"
      "|-Xclang-only=.*"
      "|-f(no-)?assume-sane-operator-new"
      "|-f(no-)?caret-diagnostics"
      "|-f(no-)?catch-undefined-behavior"
      "|-f(no-)?color-diagnostics"
      "|-f(no-)?diagnostics-fixit-info"
      "|-f(no-)?diagnostics-parseable-fixits"
      "|-f(no-)?diagnostics-print-source-range-info"
      "|-f(no-)?diagnostics-show-category.*"
      "|-f(no-)?heinous-gnu-extensions"
      "|-f(no-)?macro-backtrace-limit.*"
      "|-f(no-)?sanitize-address-zero-base-shadow"
      "|-f(no-)?sanitize-blacklist"
      "|-f(no-)?sanitize-memory-track-origins"
      "|-f(no-)?sanitize-recover"
      "|-f(no-)?sanitize=.*"
      "|-f(no-)?show-overloads.*"
      "|-f(no-)?use-init-array"
      "|-f(no-)?template-backtrace-limit.*"

      // TODO(zarko): Are plugin arguments sensible to keep?
      "|-fplugin=.*"
      "|-fplugin-arg-.*"
      "|-gline-tables-only");
  const FullMatchRegex unsupported_args_with_values_re("-Xclang"
                                                       "|-target");

  // It's important to remove the matches that have followers first -- those
  // followers might match one of the flag regular expressions, and removing
  // just the follower completely changes the semantics of the command.
  return StripPrefix(
      "-Xgcc-only=",
      CopyOmittingMatches(unsupported_args_re,
                          CopyOmittingMatchesAndFollowers(
                              unsupported_args_with_values_re, clang_args)));
}

std::vector<std::string>
AdjustClangArgsForAddressSanitizer(const std::vector<std::string> &input) {
  const FullMatchRegex inapplicable_flags_re("-static");
  const FullMatchRegex inapplicable_flags_with_shared_re("-pie");

  for (const auto &arg : input) {
    if (arg == "-shared") {
      return CopyOmittingMatches(
          inapplicable_flags_with_shared_re,
          CopyOmittingMatches(inapplicable_flags_re, input));
    }
  }

  return CopyOmittingMatches(inapplicable_flags_re, input);
}

std::vector<char *> CommandLineToArgv(const std::vector<std::string> &command) {
  std::vector<char *> result;
  result.reserve(command.size() + 1);
  for (const auto &arg : command) {
    result.push_back(const_cast<char *>(arg.c_str()));
  }
  result.push_back(nullptr);
  return result;
}

} // namespace driver
} // namespace clang
