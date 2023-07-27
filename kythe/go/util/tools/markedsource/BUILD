load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "markedsource",
    srcs = ["markedsource.go"],
    deps = [
        "//kythe/go/storage/stream",
        "//kythe/go/util/log",
        "//kythe/go/util/markedsource",
        "//kythe/proto:storage_go_proto",
        "@org_golang_google_protobuf//encoding/protojson",
    ],
)
