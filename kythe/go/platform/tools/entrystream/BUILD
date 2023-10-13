load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "entrystream",
    srcs = ["entrystream.go"],
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/storage/entryset",
        "//kythe/go/storage/stream",
        "//kythe/go/util/compare",
        "//kythe/go/util/disksort",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/log",
        "//kythe/go/util/riegeli",
        "//kythe/proto:storage_go_proto",
        "@org_golang_google_protobuf//proto",
    ],
)
