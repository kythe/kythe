load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "write_tables",
    srcs = ["write_tables.go"],
    deps = [
        "//kythe/go/platform/vfs",
        "//kythe/go/services/graphstore",
        "//kythe/go/services/graphstore/proxy",
        "//kythe/go/serving/pipeline",
        "//kythe/go/serving/pipeline/beamio",
        "//kythe/go/serving/xrefs",
        "//kythe/go/storage/gsutil",
        "//kythe/go/storage/leveldb",
        "//kythe/go/storage/stream",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/log",
        "//kythe/go/util/profile",
        "//kythe/proto:storage_go_proto",
        "//third_party/beam:runner_disksort",
        "@com_github_apache_beam//sdks/go/pkg/beam",
        "@com_github_apache_beam//sdks/go/pkg/beam/transforms/stats",
        "@com_github_apache_beam//sdks/go/pkg/beam/x/beamx",
    ],
)
