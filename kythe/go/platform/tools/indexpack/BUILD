load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "indexpack",
    srcs = ["indexpack.go"],
    deprecation = "Please use //kythe/go/platform/kzip",
    deps = [
        "//kythe/go/platform/indexpack",
        "//kythe/go/platform/kindex",
        "//kythe/go/platform/vfs",
        "//kythe/go/util/build",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:filecontext_go_proto",
        "//kythe/proto:buildinfo_go_proto",
        "//kythe/proto:cxx_go_proto",
        "//kythe/proto:go_go_proto",
        "//kythe/proto:java_go_proto",
    ],
)
