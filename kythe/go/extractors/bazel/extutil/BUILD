load("//:visibility.bzl", "PUBLIC_VISIBILITY")
load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

# A library to support Bazel extractors switching between .kzip and .kindex
# output, for temporary use in migrating away from .kindex files.
# TODO: refactor this now that kindex is gone
go_library(
    name = "extutil",
    srcs = ["extutil.go"],
    importpath = "kythe.io/kythe/go/extractors/bazel/extutil",
    visibility = [PUBLIC_VISIBILITY],
    deps = ["//kythe/go/extractors/bazel"],
)
