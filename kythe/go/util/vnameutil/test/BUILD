load("@io_bazel_rules_go//go:def.bzl", "go_binary")

go_binary(
    name = "test",
    testonly = True,
    srcs = ["test_vname_rules.go"],
    visibility = ["//visibility:public"],
    deps = [
        "//kythe/go/util/log",
        "//kythe/go/util/vnameutil",
        "//kythe/proto:storage_go_proto",
        "@org_golang_google_protobuf//proto",
    ],
)
