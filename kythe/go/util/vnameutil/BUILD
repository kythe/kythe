# gazelle:go_test file
load("//:visibility.bzl", "PUBLIC_VISIBILITY")
load("//tools:build_rules/shims.bzl", "go_binary", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "vnameutil",
    srcs = [
        "order.go",
        "rewrite.go",
    ],
    importpath = "kythe.io/kythe/go/util/vnameutil",
    visibility = [PUBLIC_VISIBILITY],
    deps = [
        "//kythe/proto:storage_go_proto",
        "@org_golang_google_protobuf//encoding/protojson",
        "@org_golang_google_protobuf//proto",
    ],
)

go_test(
    name = "rewrite_test",
    size = "small",
    srcs = ["rewrite_test.go"],
    library = ":vnameutil",
    deps = [
        "//kythe/proto:storage_go_proto",
        "@com_github_google_go_cmp//cmp",
        "@com_github_google_go_cmp//cmp/cmpopts",
        "@org_golang_google_protobuf//proto",
    ],
)

go_test(
    name = "order_test",
    size = "small",
    srcs = ["order_test.go"],
    library = ":vnameutil",
    deps = ["//kythe/proto:storage_go_proto"],
)

# A driver program for testing vname rewrite rule sets.
alias(
    name = "test_vname_rules",
    actual = "//kythe/go/util/vnameutil/test",
)
