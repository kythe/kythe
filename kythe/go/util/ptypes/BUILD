load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "ptypes",
    srcs = ["ptypes.go"],
    importpath = "kythe.io/kythe/go/util/ptypes",
    deps = [
        "@com_github_golang_protobuf//proto:go_default_library",
        "@com_github_golang_protobuf//ptypes:go_default_library_gen",
        "@com_github_golang_protobuf//ptypes/any",
    ],
)

go_test(
    name = "ptypes_test",
    size = "small",
    srcs = ["ptypes_test.go"],
    library = ":ptypes",
    visibility = ["//visibility:private"],
    deps = [
        "@com_github_golang_protobuf//proto:go_default_library",
        "@com_github_golang_protobuf//ptypes/any",
    ],
)
