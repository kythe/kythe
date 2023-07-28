load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load("//:visibility.bzl", "PUBLIC_VISIBILITY")
load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "bazel",
    srcs = [
        "extractor.go",
        "settings.go",
        "utils.go",
    ],
    importpath = "kythe.io/kythe/go/extractors/bazel",
    visibility = [PUBLIC_VISIBILITY],
    deps = [
        "//kythe/go/extractors/bazel/treeset",
        "//kythe/go/platform/kzip",
        "//kythe/go/util/datasize",
        "//kythe/go/util/log",
        "//kythe/go/util/ptypes",
        "//kythe/go/util/vnameutil",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:buildinfo_go_proto",
        "//kythe/proto:storage_go_proto",
        "//third_party/bazel:extra_actions_base_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
        "@org_bitbucket_creachadair_stringset//:stringset",
        "@org_golang_x_sync//errgroup",
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
    deps = [
        "//kythe/go/platform/kzip",
        "//kythe/go/util/log",
        "//kythe/go/util/ptypes",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:buildinfo_go_proto",
        "//kythe/proto:storage_go_proto",
        "//third_party/bazel:extra_actions_base_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
    ],
)

bzl_library(
    name = "extractors_bzl",
    srcs = ["extractors.bzl"],
)
