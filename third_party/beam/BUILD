# gazelle:exclude *
# gazelle:ignore
load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])

exports_files(["LICENSE"])

go_library(
    name = "runner_disksort",
    srcs = [
        "sdks/go/pkg/beam/runners/disksort/buffer.go",
        "sdks/go/pkg/beam/runners/disksort/disksort.go",
        "sdks/go/pkg/beam/runners/disksort/gbk.go",
    ],
    importpath = "kythe.io/third_party/beam/sdks/go/pkg/beam/runners/disksort",
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
        "@com_github_golang_protobuf//ptypes:go_default_library_gen",
    ],
)

go_test(
    name = "gbk_test",
    size = "small",
    srcs = ["sdks/go/pkg/beam/runners/disksort/gbk_test.go"],
    library = ":runner_disksort",
    deps = [
        "@com_github_apache_beam//sdks/go/pkg/beam/testing/passert:go_default_library",
        "@com_github_apache_beam//sdks/go/pkg/beam/testing/ptest:go_default_library",
        "@com_github_apache_beam//sdks/go/pkg/beam/x/debug:go_default_library",
    ],
)
