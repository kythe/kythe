load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "config",
    srcs = [
        "bazel.go",
        "common.go",
        "converter.go",
        "gradle.go",
        "maven.go",
    ],
    deps = [
        "//kythe/go/extractors/constants",
        "//kythe/proto:repo_go_proto",
        "@org_golang_google_protobuf//encoding/protojson:go_default_library",
        "@io_k8s_sigs_yaml//:go_default_library",
        "@org_golang_google_api//cloudbuild/v1:go_default_library",
    ],
)

go_test(
    name = "config_test",
    srcs = ["converter_test.go"],
    data = [
        ":testdatafiles",
        "//kythe/extractors/gcp/examples:yaml_examples",
    ],
    library = ":config",
    deps = [
        "//kythe/go/test/testutil",
    ],
)

filegroup(
    name = "testdatafiles",
    srcs = glob(["testdata/**"]),
)
