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

namespace kythe {
namespace verifier {

PrettyPrinter::~PrettyPrinter() {}

void StringPrettyPrinter::Print(const std::string& string) { data_ << string; }

void StringPrettyPrinter::Print(const char* string) { data_ << string; }

void StringPrettyPrinter::Print(const void* ptr) {
  if (ptr) {
    data_ << ptr;
  } else {
    data_ << "0";
  }
}

void FileHandlePrettyPrinter::Print(const std::string& string) {
  fprintf(file_, "%s", string.c_str());
}

void FileHandlePrettyPrinter::Print(const char* string) {
  fprintf(file_, "%s", string);
}

void FileHandlePrettyPrinter::Print(const void* ptr) {
  fprintf(file_, "0x%016llx", reinterpret_cast<unsigned long long>(ptr));
}

void QuoteEscapingPrettyPrinter::Print(const std::string& string) {
  Print(string.c_str());
}

void QuoteEscapingPrettyPrinter::Print(const char* string) {
  char buf[2];
  buf[1] = 0;
  while ((buf[0] = *string++)) {
    if (buf[0] == '\"') {
      wrapped_.Print("\\\"");
    } else if (buf[0] == '\n') {
      wrapped_.Print("\\n");
    } else if (buf[0] == '\'') {
      wrapped_.Print("\\\'");
    } else {
      wrapped_.Print(buf);
    }
  }
}

void QuoteEscapingPrettyPrinter::Print(const void* ptr) { wrapped_.Print(ptr); }

void HtmlEscapingPrettyPrinter::Print(const std::string& string) {
  Print(string.c_str());
}

void HtmlEscapingPrettyPrinter::Print(const char* string) {
  char buf[2];
  buf[1] = 0;
  while ((buf[0] = *string++)) {
    if (buf[0] == '\"') {
      wrapped_.Print("&quot;");
    } else if (buf[0] == '&') {
      wrapped_.Print("&amp;");
    } else if (buf[0] == '<') {
      wrapped_.Print("&lt;");
    } else if (buf[0] == '>') {
      wrapped_.Print("&gt;");
    } else {
      wrapped_.Print(buf);
    }
  }
}

void HtmlEscapingPrettyPrinter::Print(const void* ptr) { wrapped_.Print(ptr); }

}  // namespace verifier
}  // namespace kythe
