load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "pack2kzip",
    srcs = ["pack2kzip.go"],
    deps = [
        "//kythe/go/platform/indexpack",
        "//kythe/go/platform/kcd/kythe",
        "//kythe/go/platform/kzip",
        "//kythe/proto:analysis_go_proto",
        "@com_github_golang_protobuf//jsonpb:go_default_library",
        "@org_bitbucket_creachadair_stringset//:go_default_library",
        "@org_golang_x_sync//errgroup:go_default_library",
        "@org_golang_x_sync//semaphore:go_default_library",
    ],
)
