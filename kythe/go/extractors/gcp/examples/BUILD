load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "helloworld",
    srcs = ["helloworld.go"],
)

go_binary(
    name = "rest_test",
    srcs = ["rest_test.go"],
    deps = [
        "@org_golang_google_api//cloudbuild/v1:go_default_library",
        "@org_golang_x_oauth2//google:go_default_library",
    ],
)
