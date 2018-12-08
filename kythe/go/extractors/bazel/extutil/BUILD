load("//tools:build_rules/shims.bzl", "go_library")
load("//:visibility.bzl", "PUBLIC_VISIBILITY")

package(default_visibility = ["//kythe:default_visibility"])

# A library to support Bazel extractors switching between .kzip and .kindex
# output, for temporary use in migrating away from .kindex files.
go_library(
    name = "extutil",
    srcs = ["extutil.go"],
    visibility = [PUBLIC_VISIBILITY],
    deps = [
        "//kythe/go/extractors/bazel",
        "//kythe/go/platform/kindex",
        "//kythe/go/platform/kzip",
        "//third_party/bazel:extra_actions_base_go_proto",
    ],
)
