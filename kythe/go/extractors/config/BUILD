load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "config",
    srcs = [
        "config_parser.go",
        "config_selector.go",
        "extractor.go",
    ],
    deps = [
        "//kythe/go/extractors/config/base",
        "//kythe/proto:extraction_config_go_proto",
        "@com_github_golang_protobuf//jsonpb:go_default_library_gen",
        "@com_github_golang_protobuf//proto:go_default_library",
        "@org_bitbucket_creachadair_shell//:go_default_library",
    ],
)

go_test(
    name = "config_parser_test",
    srcs = ["config_parser_test.go"],
    data = ["//kythe/go/extractors/config/base:testdatafiles"],
    library = ":config",
    deps = [
        "//kythe/go/test/testutil",
        "//kythe/proto:extraction_config_go_proto",
        "@com_github_golang_protobuf//jsonpb:go_default_library_gen",
        "@com_github_golang_protobuf//proto:go_default_library",
    ],
)

go_test(
    name = "config_selector_test",
    srcs = ["config_selector_test.go"],
    data = ["//kythe/go/extractors/config/base:testdatafiles"],
    library = ":config",
    deps = [
        "//kythe/go/extractors/config/base",
        "//kythe/go/test/testutil",
        "//kythe/proto:extraction_config_go_proto",
        "@com_github_golang_protobuf//jsonpb:go_default_library_gen",
        "@com_github_golang_protobuf//proto:go_default_library",
    ],
)
