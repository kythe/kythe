# gazelle:go_test file
load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "pipeline",
    srcs = [
        "beam.go",
        "encoding.go",
        "filetree.go",
        "pipeline.go",
    ],
    importpath = "kythe.io/kythe/go/serving/pipeline",
    deps = [
        "//kythe/go/services/filetree",
        "//kythe/go/services/graphstore",
        "//kythe/go/serving/filetree",
        "//kythe/go/serving/graph",
        "//kythe/go/serving/graph/columnar",
        "//kythe/go/serving/pipeline/nodes",
        "//kythe/go/serving/xrefs",
        "//kythe/go/serving/xrefs/assemble",
        "//kythe/go/serving/xrefs/columnar",
        "//kythe/go/storage/keyvalue",
        "//kythe/go/storage/stream",
        "//kythe/go/storage/table",
        "//kythe/go/util/compare",
        "//kythe/go/util/disksort",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/log",
        "//kythe/go/util/schema",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/go/util/schema/nodes",
        "//kythe/go/util/sortutil",
        "//kythe/go/util/span",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:filetree_go_proto",
        "//kythe/proto:graph_serving_go_proto",
        "//kythe/proto:internal_go_proto",
        "//kythe/proto:pipeline_go_proto",
        "//kythe/proto:schema_go_proto",
        "//kythe/proto:serving_go_proto",
        "//kythe/proto:storage_go_proto",
        "//kythe/proto:xref_serving_go_proto",
        "@com_github_apache_beam//sdks/go/pkg/beam",
        "@com_github_apache_beam//sdks/go/pkg/beam/transforms/filter",
        "@org_bitbucket_creachadair_stringset//:stringset",
        "@org_golang_google_protobuf//proto",
    ],
)

go_test(
    name = "beam_test",
    srcs = ["beam_test.go"],
    library = ":pipeline",
    deps = [
        "//kythe/go/serving/pipeline/beamtest",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:pipeline_go_proto",
        "//kythe/proto:schema_go_proto",
        "//kythe/proto:serving_go_proto",
        "//kythe/proto:storage_go_proto",
        "@com_github_apache_beam//sdks/go/pkg/beam",
        "@com_github_apache_beam//sdks/go/pkg/beam/testing/passert",
        "@com_github_apache_beam//sdks/go/pkg/beam/testing/ptest",
        "@com_github_apache_beam//sdks/go/pkg/beam/x/debug",
        "@com_github_google_go_cmp//cmp",
        "@com_github_google_go_cmp//cmp/cmpopts",
        "@org_golang_google_protobuf//proto",
    ],
)

go_test(
    name = "beam_integration_test",
    srcs = ["beam_integration_test.go"],
    library = ":pipeline",
    deps = [
        "//kythe/go/services/graph",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/graph",
        "//kythe/go/serving/xrefs",
        "//kythe/go/storage/inmemory",
        "//kythe/go/storage/keyvalue",
        "//kythe/go/util/compare",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/go/util/schema/nodes",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:graph_go_proto",
        "//kythe/proto:schema_go_proto",
        "//kythe/proto:storage_go_proto",
        "//kythe/proto:xref_go_proto",
        "@com_github_apache_beam//sdks/go/pkg/beam",
        "@com_github_apache_beam//sdks/go/pkg/beam/testing/ptest",
        "@org_golang_google_protobuf//proto",
    ],
)

go_test(
    name = "filetree_test",
    srcs = ["filetree_test.go"],
    library = ":pipeline",
    deps = [
        "//kythe/proto:schema_go_proto",
        "//kythe/proto:serving_go_proto",
        "//kythe/proto:storage_go_proto",
        "@com_github_apache_beam//sdks/go/pkg/beam",
        "@com_github_apache_beam//sdks/go/pkg/beam/testing/passert",
        "@com_github_apache_beam//sdks/go/pkg/beam/testing/ptest",
        "@com_github_apache_beam//sdks/go/pkg/beam/x/debug",
    ],
)
