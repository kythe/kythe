load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "beamtest",
    srcs = ["beamtest.go"],
    importpath = "kythe.io/kythe/go/serving/pipeline/beamtest",
    deps = [
        "@com_github_apache_beam//sdks/go/pkg/beam",
        "@com_github_apache_beam//sdks/go/pkg/beam/core/runtime",
        "@com_github_apache_beam//sdks/go/pkg/beam/core/util/reflectx",
        "@org_golang_google_protobuf//proto",
    ],
)
