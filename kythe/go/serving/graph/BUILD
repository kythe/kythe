load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "graph",
    srcs = [
        "columnar.go",
        "graph.go",
    ],
    deps = [
        "//kythe/go/services/graph",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/graph/columnar",
        "//kythe/go/storage/keyvalue",
        "//kythe/go/storage/table",
        "//kythe/go/util/keys",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/log",
        "//kythe/go/util/schema",
        "//kythe/go/util/schema/facts",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:graph_go_proto",
        "//kythe/proto:graph_serving_go_proto",
        "//kythe/proto:internal_go_proto",
        "//kythe/proto:schema_go_proto",
        "//kythe/proto:serving_go_proto",
        "@org_bitbucket_creachadair_stringset//:go_default_library",
        "@org_golang_google_protobuf//proto:go_default_library",
        "@org_golang_x_net//trace:go_default_library",
    ],
)

go_test(
    name = "edges_test",
    size = "small",
    srcs = ["graph_test.go"],
    library = "graph",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/test/testutil",
        "//kythe/go/util/kytheuri",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:graph_go_proto",
        "//kythe/proto:serving_go_proto",
        "@org_golang_x_text//encoding:go_default_library",
        "@org_golang_x_text//encoding/unicode:go_default_library",
        "@org_golang_x_text//transform:go_default_library",
    ],
)

go_test(
    name = "columnar_test",
    size = "small",
    srcs = ["columnar_test.go"],
    library = ":graph",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/storage/inmemory",
        "//kythe/go/util/compare",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/nodes",
        "//kythe/proto:storage_go_proto",
    ],
)
