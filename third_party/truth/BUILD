load("@rules_java//java:defs.bzl", "java_library")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])

exports_files(["LICENSE"])

java_library(
    name = "truth",
    exports = ["@com_google_truth_truth//jar"],
    runtime_deps = ["@com_googlecode_java_diff_utils//jar"],
)
