load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "keyvalue",
    srcs = ["keyvalue.go"],
    importpath = "kythe.io/kythe/go/storage/keyvalue",
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/util/datasize",
        "//kythe/go/util/log",
        "//kythe/proto:storage_go_proto",
    ],
)

go_test(
    name = "keyvalue_test",
    size = "small",
    srcs = ["keyvalue_test.go"],
    library = ":keyvalue",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/proto:storage_go_proto",
        "@org_golang_google_protobuf//proto",
    ],
)
