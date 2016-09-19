load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "govname",
    srcs = ["govname.go"],
    deps = [
        "//kythe/go/storage/vnameutil",
        "//kythe/go/util/kytheuri",
        "//kythe/proto:storage_proto_go",
        "@go_protobuf//:proto",
    ],
)

go_test(
    name = "govname_test",
    srcs = ["govname_test.go"],
    library = "govname",
    visibility = ["//visibility:private"],
)
