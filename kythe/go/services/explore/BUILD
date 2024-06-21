load("//tools:build_rules/shims.bzl", "go_library")

package(
    default_visibility = ["//kythe:default_visibility"],
    licenses = ["notice"],
)

go_library(
    name = "explore",
    srcs = ["explore.go"],
    importpath = "kythe.io/kythe/go/services/explore",
    deps = ["//kythe/proto:explore_go_proto"],
)
