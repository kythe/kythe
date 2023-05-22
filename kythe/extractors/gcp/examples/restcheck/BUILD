load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "rest_auth_check",
    srcs = ["rest_auth_check.go"],
    deps = [
        "//kythe/go/util/log",
        "@org_golang_google_api//cloudbuild/v1:go_default_library",
        "@org_golang_x_oauth2//google:go_default_library",
    ],
)
