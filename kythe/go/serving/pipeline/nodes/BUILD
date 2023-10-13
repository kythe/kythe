load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "nodes",
    srcs = ["nodes.go"],
    importpath = "kythe.io/kythe/go/serving/pipeline/nodes",
    deps = [
        "//kythe/go/util/compare",
        "//kythe/go/util/schema",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/proto:schema_go_proto",
        "//kythe/proto:storage_go_proto",
        "@com_github_apache_beam//sdks/go/pkg/beam",
    ],
)

go_test(
    name = "nodes_test",
    srcs = ["nodes_test.go"],
    library = ":nodes",
    deps = [
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/go/util/schema/nodes",
        "//kythe/proto:schema_go_proto",
        "//kythe/proto:storage_go_proto",
        "@com_github_apache_beam//sdks/go/pkg/beam",
        "@com_github_apache_beam//sdks/go/pkg/beam/testing/passert",
        "@com_github_apache_beam//sdks/go/pkg/beam/testing/ptest",
        "@com_github_apache_beam//sdks/go/pkg/beam/x/debug",
    ],
)
