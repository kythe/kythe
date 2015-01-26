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

#include "file_vname_generator.h"
#include "gtest/gtest.h"

extern const char kTestFile[];
extern const char kSharedTestFile[];

namespace kythe {
namespace {
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
  ASSERT_EQ(default_vname.DebugString(),
            generator.LookupBaseVName("").DebugString());
}

TEST(FileVNameGenerator, DefaultLookup) {
  FileVNameGenerator generator;
  kythe::proto::VName default_vname;
  ASSERT_EQ(default_vname.DebugString(),
            generator.LookupBaseVName("").DebugString());
}

TEST(FileVNameGenerator, LookupStatic) {
  FileVNameGenerator generator;
  std::string error_text;
  ASSERT_TRUE(generator.LoadJsonString(kSharedTestFile, &error_text))
      << "Couldn't parse: " << error_text;
  kythe::proto::VName test_vname;
  test_vname.set_root("root");
  test_vname.set_corpus("static");
  EXPECT_EQ(test_vname.DebugString(),
            generator.LookupBaseVName("static/path").DebugString());
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
  EXPECT_EQ(first_vname.DebugString(),
            generator.LookupBaseVName("dup/path").DebugString());
  EXPECT_EQ(second_vname.DebugString(),
            generator.LookupBaseVName("dup/path2").DebugString());
}

TEST(FileVNameGenerator, LookupGroups) {
  FileVNameGenerator generator;
  std::string error_text;
  ASSERT_TRUE(generator.LoadJsonString(kSharedTestFile, &error_text))
      << "Couldn't parse: " << error_text;
  kythe::proto::VName corpus_vname;
  corpus_vname.set_corpus("corpus");
  EXPECT_EQ(corpus_vname.DebugString(),
            generator.LookupBaseVName("corpus/some/path/here").DebugString());
  kythe::proto::VName grp1_vname;
  grp1_vname.set_corpus("grp1/endingGroup");
  grp1_vname.set_root("12345");
  EXPECT_EQ(grp1_vname.DebugString(),
            generator.LookupBaseVName("grp1/12345/endingGroup").DebugString());
  kythe::proto::VName kythe_java_vname;
  kythe_java_vname.set_corpus("kythe");
  kythe_java_vname.set_root("java");
  EXPECT_EQ(
      kythe_java_vname.DebugString(),
      generator
          .LookupBaseVName(
               "campfire-out/bin/kythe/java/some/path/A.jar!/some/path/A.class")
          .DebugString());
  EXPECT_EQ(
      kythe_java_vname.DebugString(),
      generator.LookupBaseVName(
                    "kythe/java/com/google/devtools/kythe/util/KytheURI.java")
          .DebugString());
  kythe::proto::VName other_java_vname;
  other_java_vname.set_corpus("otherCorpus");
  other_java_vname.set_root("java");
  EXPECT_EQ(
      other_java_vname.DebugString(),
      generator
          .LookupBaseVName(
               "otherCorpus/java/com/google/devtools/kythe/util/KytheURI.java")
          .DebugString());
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
  EXPECT_EQ(
      test_file.DebugString(),
      generator
          .LookupVName(
               "kythe/cxx/extractor/testdata/extract_verify_std_string_test.cc")
          .DebugString());

  kythe::proto::VName stdlib_file;
  stdlib_file.set_corpus("cstdlib");
  stdlib_file.set_path("alloca.h");
  stdlib_file.set_root("/usr/include");
  EXPECT_EQ(stdlib_file.DebugString(),
            generator.LookupVName("/usr/include/alloca.h").DebugString());

  kythe::proto::VName compiler_file;
  compiler_file.set_corpus("cstdlib");
  compiler_file.set_path("stdint.h");
  compiler_file.set_root("third_party/llvm/lib/clang/3.6.0/include");
  EXPECT_EQ(
      compiler_file.DebugString(),
      generator.LookupVName("third_party/llvm/lib/clang/3.6.0/include/stdint.h")
          .DebugString());
}

}  // namespace
}  // namespace kythe

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();
  return result;
}

const char kTestFile[] = R"d([
  {
    "pattern": "campfire-out/[^/]+/([^/]+/javatests/.+/testdata)/.+\\.jar!/([^\\$]+)(\\$.+)?\\.class",
    "vname": {
      "corpus": "kythe",
      "path": "@1@/@2@.java"
    }
  },
  {
    "pattern": "campfire-out/[^/]+/(.*)/(java|javatests)/.*\\.jar!/([^\\$]+)(\\$.+)?\\.class",
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
    "pattern": "campfire-out/[^/]+/[^/]+/proto/.*\\.jar!/([^\\$]+)(\\$.+)?\\.class",
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
    "pattern": "(grp1)/(\\d+)/(.*)",
    "vname": {
      "root": "@2@",
      "corpus": "@1@/@3@"
    }
  },
  {
    "pattern": "campfire-out/[^/]+/([^/]+)/java/.*[.]jar!/.*",
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
