load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "createcmd",
    srcs = [
        "createcmd.go",
        "flag.go",
    ],
    importpath = "kythe.io/kythe/go/platform/tools/kzip/createcmd",
    deps = [
        "//kythe/go/platform/kzip",
        "//kythe/go/platform/tools/kzip/flags",
        "//kythe/go/platform/vfs",
        "//kythe/go/util/cmdutil",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/vnameutil",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:storage_go_proto",
        "@com_github_golang_protobuf//ptypes/any",
        "@com_github_google_subcommands//:subcommands",
        "@org_golang_google_protobuf//encoding/protojson",
    ],
)
