load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "stream",
    srcs = ["stream.go"],
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/util/schema/facts",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:storage_go_proto",
        "@org_golang_google_protobuf//encoding/protojson:go_default_library",
        "@org_golang_google_protobuf//proto:go_default_library",
    ],
)

go_test(
    name = "stream_test",
    size = "small",
    srcs = ["stream_test.go"],
    library = "stream",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/util/compare",
    ],
)
