load("//tools:build_rules/shims.bzl", "go_test", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "bazel",
    srcs = [
        "extractor.go",
        "settings.go",
        "utils.go",
    ],
    deps = [
        "//kythe/go/extractors/golang",
        "//kythe/go/extractors/govname",
        "//kythe/go/platform/indexpack",
        "//kythe/go/platform/kindex",
        "//kythe/go/util/ptypes",
        "//kythe/go/util/vnameutil",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:buildinfo_go_proto",
        "//kythe/proto:go_go_proto",
        "//kythe/proto:storage_go_proto",
        "//third_party/bazel:extra_actions_base_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
        "@go_shell//:shell",
        "@go_stringset//:stringset",
        "@go_sync//:errgroup",
    ],
)

go_test(
    name = "bazel_test",
    size = "small",
    srcs = [
        "extractor_test.go",
        "settings_test.go",
    ],
    library = ":bazel",
)
