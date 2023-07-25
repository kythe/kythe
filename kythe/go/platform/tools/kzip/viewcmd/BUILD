load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "viewcmd",
    srcs = [
        "protodeps.go",
        "viewcmd.go",
    ],
    importpath = "kythe.io/kythe/go/platform/tools/kzip/viewcmd",
    deps = [
        "//kythe/go/platform/kzip",
        "//kythe/go/platform/vfs",
        "//kythe/go/util/cmdutil",
        "//kythe/go/util/log",
        "//kythe/proto:buildinfo_go_proto",
        "//kythe/proto:cxx_go_proto",
        "//kythe/proto:go_go_proto",
        "//kythe/proto:java_go_proto",
        "@com_github_google_subcommands//:subcommands",
        "@org_golang_google_protobuf//encoding/protojson",
        "@org_golang_google_protobuf//proto",
        "@org_golang_x_sync//errgroup",
        "@org_golang_x_sync//semaphore",
    ],
)
