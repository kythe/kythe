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
/// \file
/// \brief Functions to convert gcc command lines to clang command lines.
///
/// Utilities in this file convert from arguments to the gcc frontend to
/// arguments to the clang frontend. They are useful for building tools that act
/// on compilation logs that originally invoked gcc.
///
/// Terminology:
///
/// 'argc' and 'argv' have the same meaning as in
///     `int main(int argc, char* argv[]);`
/// i.e. `argv[]` contains `argc + 1` elements, where `argv[0]` is the path
/// to the program, and `argv[argc]` (the last element) is a `NULL`
/// sentinel.
///
/// We use the term 'command line' for the `std::vector`
///     `{ argv[0], argv[1], ..., argv[argc - 1] }`
/// i.e. `argv[]` minus the trailing `NULL`.
///
/// We use the term 'args' for the `std::vector`
///     `{ argv[1], argv[2], ..., argv[argc - 1] }`
/// i.e. the command line minus `argv[0]`.
///
//===----------------------------------------------------------------------===//

// This file uses the Clang style conventions.

#ifndef LLVM_CLANG_LIB_DRIVER_COMMANDLINE_UTILS_H
#define LLVM_CLANG_LIB_DRIVER_COMMANDLINE_UTILS_H

#include <string>
#include <vector>

namespace kythe {
namespace common {

// Returns true iff a C++ source file appears on the given command
// line or args.  This doesn't care whether the input contains argv[0]
// or not.
bool HasCxxInputInCommandLineOrArgs(
    const std::vector<std::string> &command_line_or_args);

// Converts GCC's arguments to Clang's arguments by dropping GCC args that
// Clang doesn't understand.
std::vector<std::string>
GCCArgsToClangArgs(const std::vector<std::string> &gcc_args);

// Converts GCC's arguments to Clang's arguments by dropping GCC args that
// Clang doesn't understand or that are not supported in -fsyntax-only mode
// and adding -fsyntax-only. The return value is guaranteed to contain exactly
// one -fsyntax-only flag.
std::vector<std::string>
GCCArgsToClangSyntaxOnlyArgs(const std::vector<std::string> &gcc_args);

// Converts GCC's arguments to Clang's arguments by dropping GCC args that
// Clang doesn't understand or that are not supported in --analyze mode
// and adding --analyze. The return value is guaranteed to contain exactly
// one --analyze flag.
std::vector<std::string>
GCCArgsToClangAnalyzeArgs(const std::vector<std::string> &gcc_args);

// Adds -fsyntax-only to the args, and removes args incompatible with
// -fsyntax-only.  The return value is guaranteed to contain exactly
// one -fsyntax-only flag.
std::vector<std::string>
AdjustClangArgsForSyntaxOnly(const std::vector<std::string> &clang_args);

// Adds --analyze to the args, and removes args incompatible with
// --analyze.  The return value is guaranteed to contain exactly
// one --analyze flag.
std::vector<std::string>
AdjustClangArgsForAnalyze(const std::vector<std::string> &clang_args);

// Converts Clang's arguments to GCC's arguments by dropping Clang args that
// GCC doesn't understand.
std::vector<std::string>
ClangArgsToGCCArgs(const std::vector<std::string> &clang_args);

// Removes and adjusts the flags to be valid for compiling with
// AddressSanitizer.
std::vector<std::string>
AdjustClangArgsForAddressSanitizer(const std::vector<std::string> &clang_args);

// Converts a std::string std::vector representing a command line into a C
// string std::vector representing the argv (including the trailing NULL).
// Note that the C strings in the result point into the argument std::vector.
// That argument must live and remain unchanged as long as the return
// std::vector lives.
//
// Note that the result std::vector contains char* rather than const char*,
// in order to work with legacy C-style APIs.  It's the caller's responsibility
// not to modify the contents of the C-strings.
std::vector<char *>
CommandLineToArgv(const std::vector<std::string> &command_line);

// `CommandLineToArgv` should not be used with temporaries.
void CommandLineToArgv(const std::vector<std::string> &&) = delete;

// Set of possible actions to be performed by the compiler driver.
enum DriverAction {
  ASSEMBLY,
  CXX_COMPILE,
  C_COMPILE,
  FORTRAN_COMPILE,
  GO_COMPILE,
  LINK,
  UNKNOWN
};

// Decides what will the driver do based on the inputs found on the command
// line.
DriverAction DetermineDriverAction(const std::vector<std::string> &args);

} // namespace driver
} // namespace clang

#endif // LLVM_CLANG_LIB_DRIVER_COMMANDLINE_UTILS_H
