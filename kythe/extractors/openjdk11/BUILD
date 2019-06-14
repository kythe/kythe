load("//tools:build_rules/shims.bzl", "go_binary")

go_binary(
    name = "java-wrapper",
    srcs = ["java-wrapper.go"],
    data = [
        "//kythe/java/com/google/devtools/kythe/extractors/java/standalone:javac9_extractor_deploy.jar",
    ],
    deps = [
        "@io_bazel_rules_go//go/tools/bazel:go_default_library",
        "@org_bitbucket_creachadair_shell//:go_default_library",
        "@org_golang_x_sys//unix:go_default_library",
    ],
)

go_binary(
    name = "extract",
    srcs = ["extract.go"],
    data = [
        "vnames.json",
        ":java-wrapper",
    ],
    deps = [
        "//kythe/go/util/flagutil",
        "@io_bazel_rules_go//go/tools/bazel:go_default_library",
    ],
)
