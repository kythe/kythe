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

#ifndef KYTHE_CXX_VERIFIER_PRETTY_PRINTER_H_
#define KYTHE_CXX_VERIFIER_PRETTY_PRINTER_H_

#include <string>
#include <sstream>

namespace kythe {
namespace verifier {

/// \brief Prints human-readable representations of various objects.
class PrettyPrinter {
 public:
  /// \brief Prints `string`.
  virtual void Print(const std::string &string) = 0;

  /// \brief Prints `string`.
  virtual void Print(const char *string) = 0;

  /// \brief Prints `ptr` in hex with a 0x prefix (or 0 for null pointers).
  virtual void Print(const void *ptr) = 0;

  virtual ~PrettyPrinter();
};

/// \brief A `PrettyPrinter` using a `string` as its backing store.
class StringPrettyPrinter : public PrettyPrinter {
 public:
  /// \copydoc PrettyPrinter::Print(const std::string&)
  void Print(const std::string &string) override;
  /// \copydoc PrettyPrinter::Print(const char *)
  void Print(const char *string) override;
  /// \copydoc PrettyPrinter::Print(const void *)
  void Print(const void *ptr) override;
  /// Returns the `string` printed to thus far.
  std::string str() { return data_.str(); }

 private:
  /// The `string` storing this `PrettyPrinter`'s data.
  std::stringstream data_;
};

/// \brief A `PrettyPrinter` that directs its output to a file handle.
class FileHandlePrettyPrinter : public PrettyPrinter {
 public:
  /// \param file The file handle to print to.
  explicit FileHandlePrettyPrinter(FILE *file) : file_(file) {}
  /// \copydoc PrettyPrinter::Print(const std::string&)
  void Print(const std::string &string) override;
  /// \copydoc PrettyPrinter::Print(const char *)
  void Print(const char *string) override;
  /// \copydoc PrettyPrinter::Print(const void *)
  void Print(const void *ptr) override;

 private:
  FILE *file_;
};

/// \brief A `PrettyPrinter` that wraps another `PrettyPrinter` but escapes
/// to a C/JavaScript-style quotable form.
class QuoteEscapingPrettyPrinter : public PrettyPrinter {
 public:
  /// \param wrapped The `PrettyPrinter` to which transformed text should be
  /// sent.
  explicit QuoteEscapingPrettyPrinter(PrettyPrinter &wrapped)
      : wrapped_(wrapped) {}
  /// \copydoc PrettyPrinter::Print(const std::string&)
  void Print(const std::string &string) override;
  /// \copydoc PrettyPrinter::Print(const char *)
  void Print(const char *string) override;
  /// \copydoc PrettyPrinter::Print(const void *)
  void Print(const void *ptr) override;

 private:
  PrettyPrinter &wrapped_;
};

/// \brief A `PrettyPrinter` that wraps another `PrettyPrinter` but escapes
/// HTML special characters ("&<>) to HTML entities.
class HtmlEscapingPrettyPrinter : public PrettyPrinter {
 public:
  /// \param wrapped The `PrettyPrinter` to which transformed text should be
  /// sent.
  explicit HtmlEscapingPrettyPrinter(PrettyPrinter &wrapped)
      : wrapped_(wrapped) {}
  /// \copydoc PrettyPrinter::Print(const std::string&)
  void Print(const std::string &string) override;
  /// \copydoc PrettyPrinter::Print(const char *)
  void Print(const char *string) override;
  /// \copydoc PrettyPrinter::Print(const void *)
  void Print(const void *ptr) override;

 private:
  PrettyPrinter &wrapped_;
};

}  // namespace verifier
}  // namespace kythe

#endif  // KYTHE_CXX_VERIFIER_PRETTY_PRINTER_H_
