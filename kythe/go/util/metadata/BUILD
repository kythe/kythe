load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "metadata",
    srcs = ["metadata.go"],
    importpath = "kythe.io/kythe/go/util/metadata",
    deps = [
        "//kythe/go/util/schema/edges",
        "//kythe/proto:storage_go_proto",
        "@org_golang_google_protobuf//encoding/protojson",
        "@org_golang_google_protobuf//types/descriptorpb",
    ],
)

go_test(
    name = "metadata_test",
    size = "small",
    srcs = ["metadata_test.go"],
    library = ":metadata",
    deps = [
        "//kythe/go/util/compare",
        "//kythe/go/util/schema/edges",
        "//kythe/proto:storage_go_proto",
        "@org_golang_google_protobuf//proto",
        "@org_golang_google_protobuf//types/descriptorpb",
    ],
)
