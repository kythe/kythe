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

#include "kythe/cxx/common/file_vname_generator.h"

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "kythe/proto/storage.pb.h"
#include "protobuf-matchers/protocol-buffer-matchers.h"

extern const char kTestFile[];
extern const char kSharedTestFile[];

namespace kythe {
namespace {
using ::protobuf_matchers::EqualsProto;

TEST(FileVNameGenerator, ParsesFile) {
  FileVNameGenerator generator;
  std::string error_text;
  EXPECT_TRUE(generator.LoadJsonString(kTestFile, &error_text))
      << "Couldn't parse: " << error_text;
  EXPECT_TRUE(generator.LoadJsonString(kSharedTestFile, &error_text))
      << "Couldn't parse: " << error_text;
}

TEST(FileVNameGenerator, EmptyLookup) {
  FileVNameGenerator generator;
  kythe::proto::VName default_vname;
  ASSERT_THAT(generator.LookupBaseVName(""), EqualsProto(default_vname));
}

TEST(FileVNameGenerator, DefaultLookup) {
  FileVNameGenerator generator;
  kythe::proto::VName default_vname;
  default_vname.set_root("root");
  default_vname.set_corpus("corpus");
  generator.set_default_base_vname(default_vname);
  ASSERT_THAT(generator.LookupBaseVName(""), EqualsProto(default_vname));
}

TEST(FileVNameGenerator, LookupStatic) {
  FileVNameGenerator generator;
  std::string error_text;
  ASSERT_TRUE(generator.LoadJsonString(kSharedTestFile, &error_text))
      << "Couldn't parse: " << error_text;
  kythe::proto::VName test_vname;
  test_vname.set_root("root");
  test_vname.set_corpus("static");
  EXPECT_THAT(generator.LookupBaseVName("static/path"),
              EqualsProto(test_vname));
}

TEST(FileVNameGenerator, LookupOrdered) {
  FileVNameGenerator generator;
  std::string error_text;
  ASSERT_TRUE(generator.LoadJsonString(kSharedTestFile, &error_text))
      << "Couldn't parse: " << error_text;
  kythe::proto::VName first_vname;
  first_vname.set_corpus("first");
  kythe::proto::VName second_vname;
  second_vname.set_corpus("second");
  EXPECT_THAT(generator.LookupBaseVName("dup/path"), EqualsProto(first_vname));
  EXPECT_THAT(generator.LookupBaseVName("dup/path2"),
              EqualsProto(second_vname));
}

TEST(FileVNameGenerator, LookupGroups) {
  FileVNameGenerator generator;
  std::string error_text;
  ASSERT_TRUE(generator.LoadJsonString(kSharedTestFile, &error_text))
      << "Couldn't parse: " << error_text;
  kythe::proto::VName corpus_vname;
  corpus_vname.set_corpus("corpus");
  EXPECT_THAT(generator.LookupBaseVName("corpus/some/path/here"),
              EqualsProto(corpus_vname));
  kythe::proto::VName grp1_vname;
  grp1_vname.set_corpus("grp1/grp1/endingGroup");
  grp1_vname.set_root("12345");
  EXPECT_THAT(generator.LookupBaseVName("grp1/12345/endingGroup"),
              EqualsProto(grp1_vname));
  kythe::proto::VName kythe_java_vname;
  kythe_java_vname.set_corpus("kythe");
  kythe_java_vname.set_root("java");
  EXPECT_THAT(generator.LookupBaseVName(
                  "bazel-bin/kythe/java/some/path/A.jar!/some/path/A.class"),
              EqualsProto(kythe_java_vname));
  EXPECT_THAT(generator.LookupBaseVName(
                  "kythe/java/com/google/devtools/kythe/util/KytheURI.java"),
              EqualsProto(kythe_java_vname));
  kythe::proto::VName other_java_vname;
  other_java_vname.set_corpus("otherCorpus");
  other_java_vname.set_root("java");
  EXPECT_THAT(
      generator.LookupBaseVName(
          "otherCorpus/java/com/google/devtools/kythe/util/KytheURI.java"),
      EqualsProto(other_java_vname));
}

TEST(FileVNameGenerator, ActualConfigTests) {
  FileVNameGenerator generator;
  std::string error_text;
  ASSERT_TRUE(generator.LoadJsonString(kTestFile, &error_text))
      << "Couldn't parse: " << error_text;
  kythe::proto::VName test_file;
  test_file.set_corpus("kythe");
  test_file.set_path(
      "kythe/cxx/extractor/testdata/extract_verify_std_string_test.cc");
  EXPECT_THAT(
      generator.LookupVName(
          "kythe/cxx/extractor/testdata/extract_verify_std_string_test.cc"),
      EqualsProto(test_file));

  kythe::proto::VName stdlib_file;
  stdlib_file.set_corpus("cstdlib");
  stdlib_file.set_path("alloca.h");
  stdlib_file.set_root("/usr/include");
  EXPECT_THAT(generator.LookupVName("/usr/include/alloca.h"),
              EqualsProto(stdlib_file));

  kythe::proto::VName compiler_file;
  compiler_file.set_corpus("cstdlib");
  compiler_file.set_path("stdint.h");
  compiler_file.set_root("third_party/llvm/lib/clang/3.6.0/include");
  EXPECT_THAT(generator.LookupVName(
                  "third_party/llvm/lib/clang/3.6.0/include/stdint.h"),
              EqualsProto(compiler_file));
}

}  // namespace
}  // namespace kythe

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();
  return result;
}

const char kTestFile[] = R"d([
  {
    "pattern": "bazel-bin/([^/]+/javatests/.+/testdata)/.+\\.jar!/([^\\$]+)(\\$.+)?\\.class",
    "vname": {
      "corpus": "kythe",
      "path": "@1@/@2@.java"
    }
  },
  {
    "pattern": "bazel-bin/(.*)/(java|javatests)/.*\\.jar!/([^\\$]+)(\\$.+)?\\.class",
    "vname": {
      "corpus": "kythe",
      "path": "@1@/@2@/@3@.java"
    }
  },
  {
    "pattern": "(third_party/llvm/lib/clang/[^/]+/include)/(.*)",
    "vname": {
      "corpus": "cstdlib",
      "root": "@1@",
      "path": "@2@"
    }
  },
  {
    "pattern": "third_party/([^/]+)/.*\\.jar!/([^\\$]+)(\\$.+)?\\.class",
    "vname": {
      "corpus": "third_party",
      "root": "@1@",
      "path": "@2@.java"
    }
  },
  {
    "pattern": "([^/]+/java|javatests/[^\\$]+)(\\$.+)?\\.(class|java)",
    "vname": {
      "corpus": "kythe",
      "path": "@1@.java"
    }
  },
  {
    "pattern": "bazel-bin/[^/]+/proto/.*\\.jar!/([^\\$]+)(\\$.+)?\\.class",
    "vname": {
      "corpus": "kythe",
      "root": "GENERATED/proto/java",
      "path": "@1@.java"
    }
  },
  {
    "pattern": "(/usr/include/c\\+\\+/[^/]+)/(.*)",
    "vname": {
      "corpus": "libstdcxx",
      "root": "@1@",
      "path": "@2@"
    }
  },
  {
    "pattern": "/usr/include/(.*)",
    "vname": {
      "corpus": "cstdlib",
      "root": "/usr/include",
      "path": "@1@"
    }
  },
  {
    "pattern": "(.*)",
    "vname": {
      "corpus": "kythe",
      "path": "@1@"
    }
  }
])d";

// Mirror of test config in kythe/storage/go/filevnames/filevnames_test.go
const char kSharedTestFile[] = R"d([
  {
    "pattern": "static/path",
    "vname": {
      "root": "root",
      "corpus": "static"
    }
  },
  {
    "pattern": "dup/path",
    "vname": {
      "corpus": "first"
    }
  },
  {
    "pattern": "dup/path2",
    "vname": {
      "corpus": "second"
    }
  },
  {
    "pattern": "(?P<CORPUS>grp1)/(\\d+)/(.*)",
    "vname": {
      "root": "@2@",
      "corpus": "@1@/@CORPUS@/@3@"
    }
  },
  {
    "pattern": "bazel-bin/([^/]+)/java/.*[.]jar!/.*",
    "vname": {
      "root": "java",
      "corpus": "@1@"
    }
  },
  {
    "pattern": "third_party/([^/]+)/.*[.]jar!/.*",
    "vname": {
      "root": "@1@",
      "corpus": "third_party"
    }
  },
  {
    "pattern": "([^/]+)/java/.*",
    "vname": {
      "root": "java",
      "corpus": "@1@"
    }
  },
  {
    "pattern": "([^/]+)/.*",
    "vname": {
      "corpus": "@1@"
    }
  }
])d";
