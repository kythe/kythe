/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

#ifndef KYTHE_CXX_COMMON_PATH_UTILS_H_
#define KYTHE_CXX_COMMON_PATH_UTILS_H_

#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "absl/container/flat_hash_map.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"

namespace kythe {

/// \brief PathCleaner relativizes paths against a root using CleanPath.
class PathCleaner {
 public:
  /// \brief Creates a PathCleaner using the given root.
  /// \param root The root against which to relativize.
  ///             Will be cleaned and made an absolute path.
  static absl::StatusOr<PathCleaner> Create(absl::string_view root);

  /// \brief Transforms the cleaned, absolute version of `path` into a path
  ///        relative to the configured root.
  /// \return A path relative to root, if `path` is, else a cleaned `path` or an
  ///         error if the current directory cannot be determined.
  absl::StatusOr<std::string> Relativize(absl::string_view path) const;

 private:
  explicit PathCleaner(std::string root) : root_(std::move(root)) {}

  std::string root_;
};

/// \brief PathRealizer relativizes paths against a root using RealPath.
class PathRealizer {
 public:
  /// \brief Creates a PathCleaner using the given root.
  /// \param root The root against which to relativize.
  ///             Will be resolved and made an absolute path.
  static absl::StatusOr<PathRealizer> Create(absl::string_view root);

  /// PathRealizer is copyable and movable.
  PathRealizer(const PathRealizer& other);
  PathRealizer& operator=(const PathRealizer& other);
  PathRealizer(PathRealizer&& other) = default;
  PathRealizer& operator=(PathRealizer&& other) = default;

  /// \brief Transforms the resolved, absolute version of `path` into a path
  ///        relative to the configured root.
  /// \return A path relative to root, if `path` is, else a resolved `path` or
  ///         an error if the path cannot be resolved.
  absl::StatusOr<std::string> Relativize(absl::string_view path) const;

 private:
  class PathCache {
   public:
    template <typename K, typename Fn>
    absl::StatusOr<std::string> FindOrInsert(K&& key, Fn&& make);

   private:
    absl::Mutex mu_;
    absl::flat_hash_map<std::string, absl::StatusOr<std::string>> cache_;
  };

  explicit PathRealizer(std::string root) : root_(std::move(root)) {}

  std::string root_;
  // A trivial lookup cache; if you're changing symlinks during the build you're
  // going to have a bad time.
  std::unique_ptr<PathCache> cache_ = std::make_unique<PathCache>();
};

/// \brief PathCanonicalizer relatives paths against a root.
class PathCanonicalizer {
 public:
  enum class Policy {
    kCleanOnly = 0,       ///< Only clean paths when applying canonicalizer.
    kPreferRelative = 1,  ///< Use clean paths if real path is absolute.
    kPreferReal = 2,      ///< Use real path, except if errors.
  };

  /// \brief Creates a new PathCanonicalizer with the given root and policy.
  static absl::StatusOr<PathCanonicalizer> Create(
      absl::string_view root, Policy policy = Policy::kCleanOnly);

  /// \brief Transforms the provided path into a relative path depending on
  ///        configured policy.
  absl::StatusOr<std::string> Relativize(absl::string_view path) const;

 private:
  explicit PathCanonicalizer(Policy policy, PathCleaner cleaner,
                             std::optional<PathRealizer> realizer)
      : policy_(policy), cleaner_(cleaner), realizer_(realizer) {}

  Policy policy_;
  PathCleaner cleaner_;
  std::optional<PathRealizer> realizer_;
};

/// \brief Parses a flag string as a PathCanonicalizer::Policy.
///
/// This is an extension point for the Abseil Flags library to allow
/// using PathCanonicalizer directly as a flag.
bool AbslParseFlag(absl::string_view text, PathCanonicalizer::Policy* policy,
                   std::string* error);

/// \brief Returns the flag string representation of PathCanonicalizer::Policy.
///
/// This is an extension point for the Abseil Flags library to allow
/// using PathCanonicalizer directly as a flag.
std::string AbslUnparseFlag(PathCanonicalizer::Policy policy);

/// \brief Parses a string into a PathCanonicalizer::Policy.
///
/// Parses either integeral values of enumerators or lower-case,
/// dash-separated names: "clean-only", "prefer-relative", "prefer-real".
std::optional<PathCanonicalizer::Policy> ParseCanonicalizationPolicy(
    absl::string_view policy);

/// \brief Append path `b` to path `a`, cleaning and returning the result.
std::string JoinPath(absl::string_view a, absl::string_view b);

/// \brief Returns the part of the path before the final '/'.
absl::string_view Dirname(absl::string_view path);

/// \brief Returns the part of the path after the final '/'.
absl::string_view Basename(absl::string_view path);

/// \brief Collapse duplicate "/"s, resolve ".." and "." path elements, remove
/// trailing "/".
std::string CleanPath(absl::string_view input);

// \brief Returns true if path is absolute.
bool IsAbsolutePath(absl::string_view path);

/// \brief Relativize `to_relativize` with respect to `relativize_against`.
///
/// If `to_relativize` does not name a path that is a child of
/// `relativize_against`, `RelativizePath` will return an absolute path
/// (resolved against the current working directory).
///
/// Note: arguments are assumed to be valid paths, but validity is not checked.
///
/// \param to_relativize Relative or absolute path to a file.
/// \param relativize_against Relative or absolute path to a directory.
std::string RelativizePath(absl::string_view to_relativize,
                           absl::string_view relativize_against);

/// \brief Convert `path` to an absolute path, eliminating `.` and `..`.
/// \param path The path to convert.
absl::StatusOr<std::string> MakeCleanAbsolutePath(absl::string_view path);

/// \brief Returns the process's current working directory or an error.
absl::StatusOr<std::string> GetCurrentDirectory();

/// \brief Returns the result of resolving symbolic links.
absl::StatusOr<std::string> RealPath(absl::string_view path);

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_PATH_UTILS_H_
