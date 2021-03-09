load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "beamtest",
    srcs = ["beamtest.go"],
    deps = [
        "@com_github_apache_beam//sdks/go/pkg/beam:go_default_library",
        "@com_github_apache_beam//sdks/go/pkg/beam/core/runtime:go_default_library",
        "@com_github_apache_beam//sdks/go/pkg/beam/core/util/reflectx:go_default_library",
        "@org_golang_google_protobuf//proto:go_default_library",
    ],
)
