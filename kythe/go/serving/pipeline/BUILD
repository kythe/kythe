load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "pipeline",
    srcs = [
        "beam.go",
        "pipeline.go",
    ],
    deps = [
        "//kythe/go/services/filetree",
        "//kythe/go/services/graphstore",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/filetree",
        "//kythe/go/serving/graph",
        "//kythe/go/serving/pipeline/nodes",
        "//kythe/go/serving/xrefs",
        "//kythe/go/serving/xrefs/assemble",
        "//kythe/go/storage/keyvalue",
        "//kythe/go/storage/stream",
        "//kythe/go/storage/table",
        "//kythe/go/util/disksort",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/go/util/schema/nodes",
        "//kythe/go/util/sortutil",
        "//kythe/proto:filetree_go_proto",
        "//kythe/proto:internal_go_proto",
        "//kythe/proto:pipeline_go_proto",
        "//kythe/proto:schema_go_proto",
        "//kythe/proto:serving_go_proto",
        "//kythe/proto:storage_go_proto",
        "@com_github_apache_beam//sdks/go/pkg/beam:go_default_library",
        "@com_github_golang_protobuf//proto:go_default_library",
    ],
)

go_test(
    name = "beam_test",
    srcs = ["beam_test.go"],
    library = ":pipeline",
    deps = [
        "//kythe/proto:common_go_proto",
        "@com_github_apache_beam//sdks/go/pkg/beam/testing/passert:go_default_library",
        "@com_github_apache_beam//sdks/go/pkg/beam/testing/ptest:go_default_library",
        "@com_github_apache_beam//sdks/go/pkg/beam/x/debug:go_default_library",
    ],
)
