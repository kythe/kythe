load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "extractrepo",
    srcs = ["extractrepo.go"],
    deps = [
        "//kythe/go/extractors/config",
    ],
)

go_binary(
    name = "repotester",
    srcs = ["repotester.go"],
    deps = [
        "//kythe/go/extractors/config",
        "//kythe/go/platform/kindex",
        "@com_github_google_go_github//github:go_default_library",
        "@org_golang_x_oauth2//:go_default_library",
    ],
)
