load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "xrefs",
    srcs = [
        "columnar.go",
        "xrefs.go",
        "xrefs_filter.go",
    ],
    deps = [
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/xrefs/columnar",
        "//kythe/go/storage/keyvalue",
        "//kythe/go/storage/table",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/keys",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/go/util/schema/tickets",
        "//kythe/go/util/span",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:graph_go_proto",
        "//kythe/proto:internal_go_proto",
        "//kythe/proto:schema_go_proto",
        "//kythe/proto:serving_go_proto",
        "//kythe/proto:xref_go_proto",
        "//kythe/proto:xref_serving_go_proto",
        "@com_github_golang_snappy//:go_default_library",
        "@com_github_google_codesearch//index:go_default_library",
        "@org_bitbucket_creachadair_stringset//:go_default_library",
        "@org_golang_google_grpc//codes:go_default_library",
        "@org_golang_google_grpc//status:go_default_library",
        "@org_golang_google_protobuf//proto:go_default_library",
        "@org_golang_x_net//trace:go_default_library",
        "@org_golang_x_sync//errgroup:go_default_library",
    ],
)

go_test(
    name = "xrefs_test",
    size = "small",
    srcs = ["xrefs_test.go"],
    library = "xrefs",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/test/testutil",
        "//kythe/go/util/compare",
        "@org_golang_google_protobuf//encoding/prototext:go_default_library",
        "@org_golang_x_text//encoding:go_default_library",
        "@org_golang_x_text//encoding/unicode:go_default_library",
        "@org_golang_x_text//transform:go_default_library",
    ],
)

go_test(
    name = "columnar_test",
    size = "small",
    srcs = ["columnar_test.go"],
    library = ":xrefs",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/storage/inmemory",
        "//kythe/go/util/compare",
        "//kythe/proto:storage_go_proto",
    ],
)
