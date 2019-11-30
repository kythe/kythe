load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "localrun",
    srcs = glob(["*.go"]),
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/platform/delimited/dedup",
        "//kythe/go/services/graphstore",
        "//kythe/go/serving/pipeline",
        "//kythe/go/serving/pipeline/beamio",
        "//kythe/go/serving/xrefs",
        "//kythe/go/storage/gsutil",
        "//kythe/go/storage/stream",
        "//kythe/go/util/datasize",
        "//kythe/proto:storage_go_proto",
        "//third_party/bazel:build_event_stream_go_proto",
        "@com_github_apache_beam//sdks/go/pkg/beam:go_default_library",
        "@com_github_apache_beam//sdks/go/pkg/beam/x/beamx:go_default_library",
        "@com_github_golang_protobuf//proto:go_default_library",
        "@org_golang_x_sync//errgroup:go_default_library",
    ],
)
