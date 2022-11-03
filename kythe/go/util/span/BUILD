load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "span",
    srcs = ["span.go"],
    deps = [
        "//kythe/proto:common_go_proto",
        "//kythe/proto:serving_go_proto",
        "//kythe/proto:xref_go_proto",
        "@com_github_sergi_go_diff//diffmatchpatch:go_default_library",
        "@org_golang_google_protobuf//proto:go_default_library",
    ],
)

go_test(
    name = "span_test",
    size = "small",
    srcs = ["span_test.go"],
    library = "span",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/test/testutil",
        "//kythe/go/util/compare",
        "@org_golang_google_protobuf//proto:go_default_library",
    ],
)
