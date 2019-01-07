load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "kythe",
    srcs = ["units.go"],
    deps = [
        "//kythe/go/platform/kcd",
        "//kythe/go/util/ptypes",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:buildinfo_go_proto",
        "//kythe/proto:storage_go_proto",
        "@com_github_golang_protobuf//jsonpb:go_default_library_gen",
        "@com_github_golang_protobuf//proto:go_default_library",
    ],
)

go_test(
    name = "kythe_test",
    size = "small",
    srcs = ["units_test.go"],
    library = "kythe",
    visibility = ["//visibility:private"],
    deps = ["@io_bazel_rules_go//proto/wkt:any_go_proto"],
)
