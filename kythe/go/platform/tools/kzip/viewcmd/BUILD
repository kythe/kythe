load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "viewcmd",
    srcs = [
        "viewcmd.go",
    ],
    deps = [
        "//kythe/go/platform/kzip",
        "//kythe/go/util/cmdutil",
        "//kythe/proto:buildinfo_go_proto",
        "//kythe/proto:cxx_go_proto",
        "//kythe/proto:go_go_proto",
        "//kythe/proto:java_go_proto",
        "@com_github_golang_protobuf//jsonpb:go_default_library_gen",
        "@com_github_golang_protobuf//proto:go_default_library",
        "@com_github_google_subcommands//:go_default_library",
        "@org_golang_x_sync//errgroup:go_default_library",
        "@org_golang_x_sync//semaphore:go_default_library",
    ],
)
