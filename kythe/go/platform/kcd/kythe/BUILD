load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "kythe",
    srcs = ["units.go"],
    deps = [
        "//kythe/go/platform/kcd",
        "//kythe/go/util/ptypes",
        "//kythe/proto:analysis_proto_go",
        "//kythe/proto:buildinfo_proto_go",
        "//kythe/proto:storage_proto_go",
        "@go_protobuf//:proto",
    ],
)

go_test(
    name = "kythe_test",
    size = "small",
    srcs = ["units_test.go"],
    library = "kythe",
    visibility = ["//visibility:private"],
    deps = ["//third_party/proto:any_proto_go"],
)
