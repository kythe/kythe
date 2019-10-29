load("@rules_java//java:defs.bzl", "java_library")

package(default_visibility = ["//visibility:public"])

java_library(
    name = "stream_fact_emitter",
    srcs = ["StreamFactEmitter.java"],
    deps = [
        ":entryset",
        "//kythe/proto:storage_java_proto",
        "@com_google_protobuf//:protobuf_java",
    ],
)

java_library(
    name = "config",
    srcs = ["IndexerConfig.java"],
    deps = ["@maven//:com_beust_jcommander"],
)

java_library(
    name = "schema",
    srcs = [
        "EdgeKind.java",
        "NodeKind.java",
    ],
    deps = [
        "//kythe/java/com/google/devtools/kythe/util/schema",
        "//kythe/proto:schema_java_proto",
        "//third_party/guava",
        "@maven//:com_google_code_findbugs_jsr305",
        "@maven//:org_checkerframework_checker_qual",
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
        "//kythe/java/com/google/devtools/kythe/common:flogger",
        "//kythe/java/com/google/devtools/kythe/platform/shared:statistics",
        "//kythe/java/com/google/devtools/kythe/util:kythe_uri",
        "//kythe/java/com/google/devtools/kythe/util:span",
        "//kythe/proto:analysis_java_proto",
        "//kythe/proto:common_java_proto",
        "//kythe/proto:storage_java_proto",
        "//third_party/guava",
        "@com_google_protobuf//:protobuf_java",
        "@maven//:com_google_code_findbugs_jsr305",
        "@maven//:org_checkerframework_checker_qual",
    ],
)

java_library(
    name = "compilation_analyzer",
    srcs = ["AbstractCompilationAnalyzer.java"],
    deps = [
        ":entryset",
        "//kythe/java/com/google/devtools/kythe/common:flogger",
        "//kythe/java/com/google/devtools/kythe/platform/shared:analysis_exception",
        "//kythe/java/com/google/devtools/kythe/platform/shared:file_data",
        "//kythe/java/com/google/devtools/kythe/platform/shared:statistics",
        "//kythe/proto:analysis_java_proto",
        "//kythe/proto:storage_java_proto",
        "//third_party/guava",
        "@com_google_protobuf//:protobuf_java",
    ],
)

# //kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata:corner_case.kindex is derived
# based on the sources in //kythe/java/com/google/devtools/kythe/analyzers/java:analyzer which
# depend on this class.
filegroup(
    name = "corner_case_srcs",
    srcs = ["KytheEntrySets.java"],
    visibility = ["//kythe/javatests/com/google/devtools/kythe/analyzers/java:__subpackages__"],
)
