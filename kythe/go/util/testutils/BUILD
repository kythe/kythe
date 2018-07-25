load("//tools:build_rules/shims.bzl", "go_library")

# N.B. This package is given proto visibility although it is not a proto,
# because it is impractical to use the Kythe protos without ticket support.
package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "testutils",
    srcs = ["comparisons.go"],
    deps = ["@com_github_google_go_cmp//cmp:go_default_library"],
)
