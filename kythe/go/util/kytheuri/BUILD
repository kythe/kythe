load("//tools/build_rules/go", "go_library")

package(default_visibility = ["//visibility:public"])

go_library(
    name = "kytheuri",
    srcs = [
        "escape.go",
        "uri.go",
    ],
    deps = [
        "//kythe/proto:storage_proto",
    ],
)
