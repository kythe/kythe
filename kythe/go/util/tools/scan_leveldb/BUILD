load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "scan_leveldb",
    srcs = ["scan_leveldb.go"],
    deps = [
        "//kythe/go/storage/leveldb",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/log",
        "//kythe/proto:serving_go_proto",
        "//kythe/proto:storage_go_proto",
        "@org_golang_google_protobuf//proto",
        "@org_golang_google_protobuf//reflect/protoreflect",
        "@org_golang_google_protobuf//reflect/protoregistry",
    ],
)
