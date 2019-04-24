load("//:visibility.bzl", "PUBLIC_VISIBILITY")
load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "govname",
    srcs = [
        "govname.go",
        "types.go",
    ],
    visibility = [PUBLIC_VISIBILITY],
    deps = [
        "//kythe/proto:storage_go_proto",
        "@org_golang_x_tools//go/vcs:go_default_library",
    ],
)

go_test(
    name = "govname_test",
    size = "small",
    srcs = ["govname_test.go"],
    library = "govname",
    tags = ["local"],
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/util/kytheuri",
        "@com_github_golang_protobuf//proto:go_default_library",
    ],
)
