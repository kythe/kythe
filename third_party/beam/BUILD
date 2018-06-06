package(default_visibility = ["//visibility:public"])

licenses(["notice"])

exports_files(["LICENSE"])

load("//tools:build_rules/shims.bzl", "go_library")

go_library(
    name = "runner_disksort",
    srcs = glob(["sdks/go/pkg/beam/runners/disksort/*.go"]),
    importpath = "github.com/apache/beam/sdks/go/pkg/beam/runners/disksort",
    deps = [
        "//kythe/go/util/disksort",
        "//kythe/go/util/sortutil",
        "@com_github_apache_beam//sdks/go/pkg/beam:go_default_library",
        "@com_github_apache_beam//sdks/go/pkg/beam/core/graph:go_default_library",
        "@com_github_apache_beam//sdks/go/pkg/beam/core/graph/mtime:go_default_library",
        "@com_github_apache_beam//sdks/go/pkg/beam/core/graph/window:go_default_library",
        "@com_github_apache_beam//sdks/go/pkg/beam/core/metrics:go_default_library",
        "@com_github_apache_beam//sdks/go/pkg/beam/core/runtime/exec:go_default_library",
        "@com_github_apache_beam//sdks/go/pkg/beam/core/typex:go_default_library",
        "@com_github_apache_beam//sdks/go/pkg/beam/log:go_default_library",
        "@com_github_apache_beam//sdks/go/pkg/beam/runners/direct:go_default_library",
        "@com_github_golang_protobuf//proto:go_default_library",
        "@com_github_golang_protobuf//ptypes:go_default_library",
        "@org_golang_x_sync//errgroup:go_default_library",
    ],
)
