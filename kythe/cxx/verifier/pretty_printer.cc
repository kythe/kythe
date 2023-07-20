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

#include "pretty_printer.h"

#include <bitset>
#include <string_view>

#include "absl/strings/str_format.h"

namespace kythe {
namespace verifier {

PrettyPrinter::~PrettyPrinter() {}

void StringPrettyPrinter::Print(std::string_view string) { data_ << string; }
void StringPrettyPrinter::Print(const char* string) { data_ << string; }

void StringPrettyPrinter::Print(const void* ptr) {
  if (ptr) {
    data_ << ptr;
  } else {
    data_ << "0";
  }
}

void FileHandlePrettyPrinter::Print(std::string_view string) {
  absl::FPrintF(file_, "%s", string);
}

void FileHandlePrettyPrinter::Print(const char* string) {
  absl::FPrintF(file_, "%s", string);
}

void FileHandlePrettyPrinter::Print(const void* ptr) {
  absl::FPrintF(file_, "0x%016llx", reinterpret_cast<unsigned long long>(ptr));
}

void QuoteEscapingPrettyPrinter::Print(std::string_view string) {
  for (char ch : string) {
    if (ch == '\"') {
      wrapped_.Print("\\\"");
    } else if (ch == '\n') {
      wrapped_.Print("\\n");
    } else if (ch == '\'') {
      wrapped_.Print("\\\'");
    } else {
      wrapped_.Print({&ch, 1});
    }
  }
}

void QuoteEscapingPrettyPrinter::Print(const char* string) {
  Print(std::string_view(string));
}

void QuoteEscapingPrettyPrinter::Print(const void* ptr) { wrapped_.Print(ptr); }

void HtmlEscapingPrettyPrinter::Print(std::string_view string) {
  for (char ch : string) {
    if (ch == '\"') {
      wrapped_.Print("&quot;");
    } else if (ch == '&') {
      wrapped_.Print("&amp;");
    } else if (ch == '<') {
      wrapped_.Print("&lt;");
    } else if (ch == '>') {
      wrapped_.Print("&gt;");
    } else {
      wrapped_.Print({&ch, 1});
    }
  }
}

void HtmlEscapingPrettyPrinter::Print(const char* string) {
  Print(std::string_view(string));
}

void HtmlEscapingPrettyPrinter::Print(const void* ptr) { wrapped_.Print(ptr); }

}  // namespace verifier
}  // namespace kythe
