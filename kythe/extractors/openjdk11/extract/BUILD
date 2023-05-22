load("//tools:build_rules/shims.bzl", "go_binary")

go_binary(
    name = "extract",
    srcs = ["extract.go"],
    data = [
        "//kythe/extractors/openjdk11:vnames.json",
        "//kythe/extractors/openjdk11/java_wrapper",
        "//kythe/java/com/google/devtools/kythe/extractors/java/standalone:javac_extractor_deploy.jar",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//kythe/go/util/flagutil",
        "//kythe/go/util/log",
        "@io_bazel_rules_go//go/tools/bazel:go_default_library",
    ],
)
