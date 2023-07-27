load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "identifiers",
    srcs = ["identifiers.go"],
    importpath = "kythe.io/kythe/go/serving/identifiers",
    deps = [
        "//kythe/go/services/web",
        "//kythe/go/storage/table",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/log",
        "//kythe/proto:identifier_go_proto",
        "//kythe/proto:serving_go_proto",
        "@org_golang_google_protobuf//proto",
    ],
)

go_test(
    name = "identifiers_test",
    size = "small",
    srcs = ["identifiers_test.go"],
    library = "identifiers",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/storage/table",
        "//kythe/go/test/testutil",
        "//kythe/proto:identifier_go_proto",
        "//kythe/proto:serving_go_proto",
        "@org_golang_google_protobuf//proto",
    ],
)
