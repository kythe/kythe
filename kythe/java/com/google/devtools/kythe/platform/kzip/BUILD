load("@rules_java//java:defs.bzl", "java_library")

package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "kzip",
    srcs = [
        "KZip.java",
        "KZipException.java",
        "KZipReader.java",
        "KZipWriter.java",
    ],
    deps = [
        "//kythe/java/com/google/devtools/kythe/common:autovalue",
        "//kythe/java/com/google/devtools/kythe/common:flogger",
        "//kythe/java/com/google/devtools/kythe/platform/shared:units",
        "//kythe/java/com/google/devtools/kythe/util:json",
        "//kythe/proto:analysis_java_proto",
        "//third_party/guava",
        "@com_google_code_gson_gson//jar",
        "@com_google_protobuf//:protobuf_java",
    ],
)
