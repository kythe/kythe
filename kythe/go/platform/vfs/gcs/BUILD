load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "gcs",
    srcs = ["fs.go"],
    deps = [
        "//kythe/go/platform/vfs",
        "@go_gcloud//:storage",
        "@go_x_net//:context",
    ],
)

go_test(
    name = "gcs_test",
    srcs = ["fs_test.go"],
    library = "gcs",
    visibility = ["//visibility:private"],
)
