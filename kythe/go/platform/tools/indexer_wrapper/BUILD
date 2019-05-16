load("//tools:build_rules/shims.bzl", "go_binary")

go_binary(
    name = "indexer_wrapper",
    srcs = [
        "local.go",
        "main.go",
    ],
    deps = [
        "//kythe/go/platform/analysis",
        "//kythe/go/platform/analysis/driver",
        "//kythe/go/platform/analysis/local",
        "//kythe/go/platform/delimited",
        "//kythe/go/platform/kzip",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:storage_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
    ],
)
