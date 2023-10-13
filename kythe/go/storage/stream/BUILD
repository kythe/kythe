load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "stream",
    srcs = ["stream.go"],
    importpath = "kythe.io/kythe/go/storage/stream",
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/util/log",
        "//kythe/go/util/schema/facts",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:storage_go_proto",
        "@org_golang_google_protobuf//encoding/protojson",
        "@org_golang_google_protobuf//proto",
    ],
)

go_test(
    name = "stream_test",
    size = "small",
    srcs = ["stream_test.go"],
    library = ":stream",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/util/compare",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:storage_go_proto",
        "@org_golang_google_protobuf//encoding/protojson",
        "@org_golang_google_protobuf//proto",
    ],
)
