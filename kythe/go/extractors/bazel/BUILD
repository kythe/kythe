load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "bazel",
    srcs = [
        "extractor.go",
        "utils.go",
    ],
    deps = [
        "//kythe/go/extractors/golang",
        "//kythe/go/extractors/govname",
        "//kythe/go/platform/indexpack",
        "//kythe/go/platform/kindex",
        "//kythe/go/util/ptypes",
        "//kythe/go/util/vnameutil",
        "//kythe/proto:analysis_proto_go",
        "//kythe/proto:buildinfo_proto_go",
        "//kythe/proto:go_proto_go",
        "//kythe/proto:storage_proto_go",
        "//third_party/bazel:extra_actions_base_proto_go",
        "@go_protobuf//:proto",
        "@go_shell//:shell",
        "@go_stringset//:stringset",
    ],
)

go_test(
    name = "bazel_test",
    size = "small",
    srcs = ["extractor_test.go"],
    library = ":bazel",
)
