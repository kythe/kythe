package(default_visibility = ["//visibility:public"])

java_library(
    name = "schema",
    srcs = [
        "EdgeKind.java",
        "NodeKind.java",
    ],
)

java_library(
    name = "entryset",
    srcs = [
        "CorpusPath.java",
        "EntrySet.java",
        "FactEmitter.java",
        "KytheEntrySets.java",
    ],
    deps = [
        ":schema",
        "//kythe/java/com/google/devtools/kythe/common",
        "//kythe/java/com/google/devtools/kythe/util",
        "//kythe/proto:analysis_proto",
        "//kythe/proto:storage_proto",
        "//third_party:protobuf",
        "//third_party/guava",
    ],
)
