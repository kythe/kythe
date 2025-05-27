load("//tools:build_rules/shims.bzl", "go_binary", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "rust_project_to_kzip",
    srcs = ["rust_project_to_kzip.go"],
    deps = [
        "//kythe/go/platform/kzip",
        "//kythe/go/platform/vfs",
        "//kythe/go/util/log",
        "//kythe/go/util/vnameutil",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:storage_go_proto",
    ],
)

go_test(
    name = "rust_project_to_kzip_test",
    size = "small",
    srcs = [
        "rust_project_to_kzip.go",
        "rust_project_to_kzip_test.go",
    ],
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/platform/kzip",
        "//kythe/go/platform/vfs",
        "//kythe/go/util/vnameutil",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:storage_go_proto",
        "@com_github_google_go_cmp//cmp",
        "@com_github_google_go_cmp//cmp/cmpopts",
        "@org_golang_google_protobuf//proto",
        "@org_golang_google_protobuf//testing/protocmp",
    ],
)
