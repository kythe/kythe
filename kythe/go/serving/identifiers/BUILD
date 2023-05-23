load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "identifiers",
    srcs = ["identifiers.go"],
    deps = [
        "//kythe/go/services/web",
        "//kythe/go/services/xrefs",
        "//kythe/go/storage/table",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/log",
        "//kythe/go/util/schema/tickets",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:identifier_go_proto",
        "//kythe/proto:internal_go_proto",
        "//kythe/proto:serving_go_proto",
        "@org_golang_google_protobuf//proto:go_default_library",
    ],
)

go_test(
    name = "identifiers_test",
    size = "small",
    srcs = ["identifiers_test.go"],
    library = "identifiers",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/test/testutil",
        "@org_golang_google_protobuf//proto:go_default_library",
        "@org_golang_x_text//encoding:go_default_library",
        "@org_golang_x_text//encoding/unicode:go_default_library",
        "@org_golang_x_text//transform:go_default_library",
    ],
)
