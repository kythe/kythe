load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "entryset",
    srcs = ["entryset.go"],
    deps = [
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema/edges",
        "//kythe/proto:entryset_go_proto",
        "//kythe/proto:internal_go_proto",
        "//kythe/proto:storage_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
    ],
)

go_test(
    name = "entryset_test",
    size = "small",
    srcs = ["entryset_test.go"],
    library = "entryset",
    deps = [
        "//kythe/go/test/testutil",
        "//kythe/go/util/compare",
        "//kythe/go/util/kytheuri",
    ],
)
