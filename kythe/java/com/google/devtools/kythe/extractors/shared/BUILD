load("@rules_java//java:defs.bzl", "java_library")

package(default_visibility = ["//visibility:public"])

java_library(
    name = "shared",
    srcs = [
        "CompilationDescription.java",
        "ExtractionException.java",
        "ExtractorUtils.java",
        "IndexInfoUtils.java",
    ],
    deps = [
        ":environment",
        ":file_vnames",
        "//kythe/java/com/google/devtools/kythe/platform/kzip",
        "//kythe/proto:analysis_java_proto",
        "//kythe/proto:storage_java_proto",
        "//third_party/guava",
        "@com_google_protobuf//:protobuf_java",
    ],
)

java_library(
    name = "environment",
    srcs = [
        "EnvironmentUtils.java",
    ],
    deps = [
        "//third_party/guava",
        "@com_google_common_flogger//api",
    ],
)

java_library(
    name = "file_vnames",
    srcs = ["FileVNames.java"],
    deps = [
        ":environment",
        "//kythe/proto:storage_java_proto",
        "//third_party/guava",
        "@com_google_protobuf//:protobuf_java",
        "@maven//:com_google_code_gson_gson",
        "@maven//:com_google_re2j_re2j",
    ],
)
