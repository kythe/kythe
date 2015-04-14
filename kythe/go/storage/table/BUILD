load("//tools/build_rules/go", "go_library")

package(default_visibility = ["//visibility:public"])

go_library(
    name = "table",
    srcs = [
        "table.go",
    ],
    deps = [
        "//kythe/go/storage/keyvalue",
        "//third_party/go:protobuf",
    ],
)
