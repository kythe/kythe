/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

#include "kythe/cxx/common/kythe_uri.h"

#include <string>

#include "absl/log/initialize.h"
#include "gmock/gmock.h"
#include "google/protobuf/stubs/common.h"
#include "gtest/gtest.h"
#include "kythe/proto/storage.pb.h"
#include "protobuf-matchers/protocol-buffer-matchers.h"

namespace kythe {
namespace {
using ::protobuf_matchers::EqualsProto;

struct MakeURI {
  const char* signature = "";
  const char* corpus = "";
  const char* root = "";
  const char* path = "";
  const char* language = "";

  MakeURI& Language(const char* v) {
    language = v;
    return *this;
  }
  MakeURI& Corpus(const char* v) {
    corpus = v;
    return *this;
  }
  MakeURI& Root(const char* v) {
    root = v;
    return *this;
  }
  MakeURI& Signature(const char* v) {
    signature = v;
    return *this;
  }
  MakeURI& Path(const char* v) {
    path = v;
    return *this;
  }

  kythe::proto::VName v_name() const {
    kythe::proto::VName v_name;
    v_name.set_signature(signature);
    v_name.set_corpus(corpus);
    v_name.set_root(root);
    v_name.set_path(path);
    v_name.set_language(language);
    return v_name;
  }

  URI uri() const { return URI(v_name()); }

  operator URI() const { return URI(v_name()); }  // NOLINT
};

TEST(KytheUri, Parse) {
  struct {
    std::string input;
    URI expect;
  } tests[] = {
      {"", MakeURI()},
      {"kythe:", MakeURI()},
      {"kythe://", MakeURI()},

      // Individual components.
      {"#sig", MakeURI().Signature("sig")},
      {"kythe:#sig", MakeURI().Signature("sig")},
      {"kythe://corpus", MakeURI().Corpus("corpus")},
      {"kythe://corpus/", MakeURI().Corpus("corpus/")},
      {"kythe://corpus/with/path", MakeURI().Corpus("corpus/with/path")},
      {"//corpus/with/path", MakeURI().Corpus("corpus/with/path")},
      {"kythe:?root=R", MakeURI().Root("R")},
      {"kythe:?path=P", MakeURI().Path("P")},
      {"kythe:?lang=L", MakeURI().Language("L")},

      // Special characters.
      {"kythe:#-%2B_%2F", MakeURI().Signature("-+_/")},

      // Corner cases about relative paths. NB: MakeURI() goes through VNames.
      {"kythe://..", MakeURI().Corpus("..")},
      {"kythe://../", MakeURI().Corpus("../")},
      {"kythe://../..", MakeURI().Corpus("../..")},
      {"kythe://a/../b//c", MakeURI().Corpus("a/../b//c")},

      // Multiple attributes, with permutation of order.
      {"kythe:?lang=L?root=R", MakeURI().Root("R").Language("L")},
      {"kythe:?lang=L?path=P?root=R",
       MakeURI().Root("R").Language("L").Path("P")},
      {"kythe:?root=R?path=P?lang=L",
       MakeURI().Root("R").Language("L").Path("P")},

      // Corpora with slashes.
      {"kythe:///Users/foo", MakeURI().Corpus("/Users/foo")},
      {"kythe:///", MakeURI().Corpus("/")},
      {"kythe://kythe//branch", MakeURI().Corpus("kythe//branch")},

      // Corpus labels are not cleaned.
      {"//a//?lang=foo?path=b/c/..",
       MakeURI().Corpus("a//").Path("b").Language("foo")},
      {"kythe://a/./b/..//c/#sig",
       MakeURI().Signature("sig").Corpus("a/./b/..//c/")},

      // Everything.
      {"kythe://bitbucket.org/creachadair/"
       "stringset?path=stringset.go?lang=go?root=blah#sig",
       MakeURI()
           .Signature("sig")
           .Corpus("bitbucket.org/creachadair/stringset")
           .Root("blah")
           .Path("stringset.go")
           .Language("go")},

      // Regression: Escape sequences in the corpus specification.
      {"kythe://libstdc%2B%2B?lang=c%2B%2B?path=bits/basic_string.h?root=/usr/"
       "include/c%2B%2B/4.8",
       MakeURI()
           .Corpus("libstdc++")
           .Path("bits/basic_string.h")
           .Root("/usr/include/c++/4.8")
           .Language("c++")}};
  for (auto& test : tests) {
    auto parsed = URI::FromString(test.input);
    EXPECT_TRUE(parsed.first) << test.input;
    EXPECT_TRUE(parsed.second == test.expect)
        << parsed.second.ToString() << " got, expected "
        << test.expect.ToString();
  }
}

TEST(KytheUri, ParseErrors) {
  auto tests = {
      "invalid corpus",
      "http://unsupported-scheme",
      "?huh=bogus+attribute+key",
      "?path=",   // empty query value
      "?root=?",  // empty query value
      "//a/%x/bad-escaping",
      "kythe:///invalid-corpus?blah",
      "/another-invalid-corpus",
      "random/opaque/failure",
  };
  for (auto& test : tests) {
    auto parsed = URI::FromString(test);
    EXPECT_FALSE(parsed.first) << test;
  }
}

TEST(KytheUri, Equality) {
  struct {
    const char *a, *b;
  } equal[] = {
      // Various empty equivalencies.
      {"", ""},
      {"", "kythe:"},
      {"kythe://", ""},
      {"kythe://", "kythe:"},

      // Order of attributes is normalized.
      {"kythe:?root=R?path=P", "kythe://?path=P?root=R"},
      {"kythe:?root=R?path=P?lang=L", "kythe://?path=P?lang=L?root=R"},

      // Escaping is respected.
      {"kythe:?path=%50", "kythe://?path=P"},
      {"kythe:?lang=%4c?path=%50", "kythe://?lang=L?path=P"},

      // Paths are cleaned.
      {"kythe://a?path=b/../c#sig", "kythe://a?path=c#sig"},
      {"kythe://a?path=b/../d/./e/../../c#sig", "kythe://a?path=c#sig"},
      {"//a?path=b/c/../d?lang=%67%6F", "kythe://a?path=b/d?lang=go"}};
  for (auto& test : equal) {
    auto a_parse = URI::FromString(test.a);
    auto b_parse = URI::FromString(test.b);
    EXPECT_TRUE(a_parse.first) << test.a;
    EXPECT_TRUE(b_parse.first) << test.b;
    EXPECT_TRUE(a_parse.second == b_parse.second) << test.a << " != " << test.b;
  }
  struct {
    const char *a, *b;
  } nonequal[] = {{"kythe://a", "kythe://a?path=P"},
                  {"bogus", "bogus"},
                  {"bogus", "kythe://good"},
                  {"kythe://good", "bogus"}};
  for (auto& test : nonequal) {
    auto a_parse = URI::FromString(test.a);
    auto b_parse = URI::FromString(test.b);
    if (!a_parse.first || !b_parse.first) {
      continue;
    }
    EXPECT_TRUE(a_parse.second != b_parse.second) << test.a << " == " << test.b;
  }
  EXPECT_TRUE(URI() == URI());
  auto empty_parse = URI::FromString("kythe://");
  EXPECT_TRUE(empty_parse.first);
  EXPECT_TRUE(empty_parse.second == URI());
}

TEST(KytheUri, RoundTrip) {
  auto uri = MakeURI()
                 .Signature("magic carpet ride")
                 .Corpus("code.google.com/p/go.tools")
                 .Path("cmd/godoc/doc.go")
                 .Language("go");
  auto uri_roundtrip = URI::FromString(uri.uri().ToString());
  EXPECT_TRUE(uri_roundtrip.first);
  const auto& other_vname = uri_roundtrip.second.v_name();
  EXPECT_THAT(uri.v_name(), EqualsProto(other_vname));
}

TEST(KytheUri, OneSlashRoundTrip) {
  auto uri = MakeURI()
                 .Signature("/")
                 .Corpus("/Users/foo")
                 .Path("/Users/foo/bar")
                 .Language("go");
  auto uri_roundtrip = URI::FromString(uri.uri().ToString());
  EXPECT_TRUE(uri_roundtrip.first);
  const auto& other_vname = uri_roundtrip.second.v_name();
  EXPECT_THAT(uri.v_name(), EqualsProto(other_vname));
}

TEST(KytheUri, TwoSlashRoundTrip) {
  auto uri = MakeURI()
                 .Signature("/")
                 .Corpus("//Users/foo")
                 .Path("/Users/foo/bar")
                 .Language("go");
  auto uri_roundtrip = URI::FromString(uri.uri().ToString());
  EXPECT_TRUE(uri_roundtrip.first);
  const auto& other_vname = uri_roundtrip.second.v_name();
  EXPECT_THAT(uri.v_name(), EqualsProto(other_vname));
}

// TODO(zarko): Check the "////////" corpus once we settle whether corpus
// names should be path-cleaned.

TEST(KytheUri, Strings) {
  constexpr char empty[] = "kythe:";
  constexpr char canonical[] = "kythe:?lang=L?path=P?root=R";
  constexpr char cleaned[] = "kythe://a?path=c#sig";
  struct {
    const char *input, *want;
  } tests[] = {// Empty forms
               {"", empty},
               {"kythe:", empty},
               {"kythe://", empty},
               {"kythe:#", empty},
               {"kythe://#", empty},

               // Check ordering
               {"kythe:?root=R?path=P?lang=L", canonical},
               {"kythe:?root=R?lang=L?path=P", canonical},
               {"kythe:?lang=L?path=P?root=R", canonical},
               {"kythe://?lang=L?path=P?root=R#", canonical},

               // Check escaping
               {"kythe://?path=%50", "kythe:?path=P"},
               {"kythe://?path=%2B", "kythe:?path=%2B"},
               {"kythe://?path=a+b", "kythe:?path=a%2Bb"},
               {"kythe://?path=%20", "kythe:?path=%20"},
               {"kythe://?path=a/b", "kythe:?path=a/b"},

               // Path cleaning
               {"kythe://a?path=b/../c#sig", cleaned},
               {"kythe://a?path=./d/.././c#sig", cleaned},

               // Regression: Escape sequences in the corpus specification.
               {"kythe://libstdc%2B%2B?path=bits/"
                "basic_string.h?lang=c%2B%2B?root=/usr/include/c%2B%2B/4.8",
                "kythe://libstdc%2B%2B?lang=c%2B%2B?path=bits/"
                "basic_string.h?root=/usr/include/c%2B%2B/4.8"}};
  for (const auto& test : tests) {
    auto parsed = URI::FromString(test.input);
    EXPECT_TRUE(parsed.first);
    EXPECT_EQ(test.want, parsed.second.ToString());
  }
}

}  // anonymous namespace
}  // namespace kythe

int main(int argc, char** argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  absl::InitializeLog();
  ::testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();
  return result;
}
