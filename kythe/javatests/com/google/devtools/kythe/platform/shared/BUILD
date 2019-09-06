load("@rules_java//java:defs.bzl", "java_test")

package(default_visibility = ["//kythe:default_visibility"])

java_test(
    name = "CompilationUnitsTest",
    size = "small",
    srcs = ["CompilationUnitsTest.java"],
    data = [
        "//kythe/cxx/common/testdata:56bf5044e1b5c4c1cc7c4b131ac2fb979d288460e63352b10eef80ca35bd0a7b.pbtxt",
        "//kythe/cxx/common/testdata:bb761979683e7c268e967eb5bcdedaa7fa5d1d472b0826b00b69acafbaad7ee6.pbtxt",
        "//kythe/cxx/common/testdata:e9e170dcfca53c8126755bbc8b703994dedd3af32584291e01fba164ab5d3f32.pbtxt",
    ],
    deps = [
        "//kythe/java/com/google/devtools/kythe/analyzers/base:schema",
        "//kythe/java/com/google/devtools/kythe/common:autovalue",
        "//kythe/java/com/google/devtools/kythe/common:flogger",
        "//kythe/java/com/google/devtools/kythe/platform/shared:test_util",
        "//kythe/java/com/google/devtools/kythe/platform/shared:units",
        "//kythe/proto:analysis_java_proto",
        "//kythe/proto:storage_java_proto",
        "//third_party/guava",
        "//third_party/truth",
        "@com_google_code_findbugs_jsr305//jar",
        "@com_google_protobuf//:protobuf_java",
        "@junit_junit//jar",
    ],
)

java_test(
    name = "FileDataCacheTest",
    size = "small",
    srcs = ["FileDataCacheTest.java"],
    deps = [
        "//kythe/java/com/google/devtools/kythe/platform/shared:file_data",
        "//kythe/proto:analysis_java_proto",
        "//third_party/guava",
        "//third_party/truth",
        "@com_google_protobuf//:protobuf_java",
        "@junit_junit//jar",
    ],
)

java_test(
    name = "FileDataDirectoryTest",
    size = "small",
    srcs = ["FileDataDirectoryTest.java"],
    deps = [
        "//kythe/java/com/google/devtools/kythe/platform/shared:file_data",
        "//third_party/guava",
        "//third_party/truth",
        "@junit_junit//jar",
    ],
)
