load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "localrun",
    srcs = ["localrun.go"],
    importpath = "kythe.io/kythe/go/localrun",
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/platform/delimited/dedup",
        "//kythe/go/serving/pipeline",
        "//kythe/go/serving/pipeline/beamio",
        "//kythe/go/serving/xrefs",
        "//kythe/go/util/datasize",
        "//kythe/go/util/log",
        "//third_party/bazel:build_event_stream_go_proto",
        "@com_github_apache_beam//sdks/go/pkg/beam",
        "@com_github_apache_beam//sdks/go/pkg/beam/transforms/stats",
        "@com_github_apache_beam//sdks/go/pkg/beam/x/beamx",
    ],
)

go_test(
    name = "localrun_test",
    srcs = ["localrun_test.go"],
    library = ":localrun",
)
