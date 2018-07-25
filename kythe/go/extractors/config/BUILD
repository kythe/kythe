load("//tools:build_rules/shims.bzl", "go_test", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "config",
    srcs = [
        "config_parser.go",
        "extractor.go",
    ],
    deps = [
        "//kythe/go/extractors/config/default/mvn",
        "//kythe/proto:extraction_config_go_proto",
        "@com_github_golang_protobuf//jsonpb:go_default_library",
        "@com_github_golang_protobuf//proto:go_default_library",
        "@org_bitbucket_creachadair_shell//:go_default_library",
    ],
)

go_test(
    name = "config_test",
    srcs = ["config_parser_test.go"],
    data = glob(["testdata/**"]),
    library = ":config",
    deps = [
        "//kythe/go/util/testutils",
        "//kythe/proto:extraction_config_go_proto",
        "@com_github_golang_protobuf//jsonpb:go_default_library",
        "@com_github_golang_protobuf//proto:go_default_library",
    ],
)
