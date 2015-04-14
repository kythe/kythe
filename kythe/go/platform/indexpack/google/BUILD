load("//tools/build_rules/go", "go_library")

package(default_visibility = ["//visibility:public"])

go_library(
    name = "google",
    srcs = [
        "fs.go",
    ],
    deps = [
        "//third_party/go:cloud_storage",
        "//third_party/go:context",
    ],
)
