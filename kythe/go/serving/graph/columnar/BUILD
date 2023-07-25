load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "columnar",
    srcs = ["columnar_encoding.go"],
    importpath = "kythe.io/kythe/go/serving/graph/columnar",
    deps = [
        "//kythe/go/util/keys",
        "//kythe/proto:graph_serving_go_proto",
        "//kythe/proto:schema_go_proto",
        "//kythe/proto:storage_go_proto",
        "@org_golang_google_protobuf//proto",
    ],
)

go_test(
    name = "columnar_test",
    size = "small",
    srcs = ["columnar_test.go"],
    library = ":columnar",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/util/compare",
        "//kythe/go/util/keys",
        "//kythe/proto:graph_serving_go_proto",
        "//kythe/proto:schema_go_proto",
        "//kythe/proto:storage_go_proto",
    ],
)
