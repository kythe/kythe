package(default_visibility = ["//visibility:public"])

java_library(
    name = "analyzer",
    srcs = [
        "FilePositions.java",
        "JavaEntrySets.java",
        "KytheJavacAnalyzer.java",
        "KytheTreeScanner.java",
    ],
    deps = [
        "//kythe/java/com/google/devtools/kythe/analyzers/base:entryset",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:schema",
        "//kythe/java/com/google/devtools/kythe/common",
        "//kythe/java/com/google/devtools/kythe/platform/java",
        "//kythe/java/com/google/devtools/kythe/platform/java/helpers",
        "//kythe/java/com/google/devtools/kythe/platform/shared",
        "//kythe/java/com/google/devtools/kythe/util",
        "//kythe/proto:analysis_proto",
        "//kythe/proto:storage_proto",
        "//third_party:protobuf",
        "//third_party/guava",
        "//third_party/openjdk/lib:tools_jar",
    ],
)
