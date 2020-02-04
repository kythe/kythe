load("//:visibility.bzl", "PUBLIC_VISIBILITY")
load("//tools:build_rules/shims.bzl", "go_binary", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "vnameutil",
    srcs = [
        "order.go",
        "rewrite.go",
    ],
    visibility = [PUBLIC_VISIBILITY],
    deps = [
        "//kythe/proto:storage_go_proto",
        "@com_github_golang_protobuf//jsonpb:go_default_library_gen",
        "@com_github_golang_protobuf//proto:go_default_library",
    ],
)

go_test(
    name = "rewrite_test",
    size = "small",
    srcs = ["rewrite_test.go"],
    library = "vnameutil",
    deps = [
        "//kythe/proto:storage_go_proto",
        "@com_github_google_go_cmp//cmp:go_default_library",
    ],
)

go_test(
    name = "order_test",
    size = "small",
    srcs = ["order_test.go"],
    library = "vnameutil",
    deps = ["//kythe/proto:storage_go_proto"],
)

# A driver program for testing vname rewrite rule sets.
go_binary(
    name = "test_vname_rules",
    testonly = 1,
    srcs = ["test/test_vname_rules.go"],
    deps = [
        ":vnameutil",
        "//kythe/proto:storage_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
    ],
)
