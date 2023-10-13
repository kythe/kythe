load("//tools:build_rules/shims.bzl", "go_library", "go_test")

# N.B. This package is given proto visibility although it is not a proto,
# because it is impractical to use the Kythe protos without ticket support.
package(default_visibility = ["//kythe:proto_visibility"])

go_library(
    name = "kytheuri",
    srcs = [
        "escape.go",
        "uri.go",
    ],
    importpath = "kythe.io/kythe/go/util/kytheuri",
    deps = [
        "//kythe/proto:common_go_proto",
        "//kythe/proto:storage_go_proto",
    ],
)

go_test(
    name = "kytheuri_test",
    size = "small",
    srcs = [
        "bench_test.go",
        "uri_test.go",
    ],
    library = ":kytheuri",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/proto:common_go_proto",
        "//kythe/proto:storage_go_proto",
        "@org_golang_google_protobuf//proto",
    ],
)
