load("//tools/build_rules/go", "go_library")

package(default_visibility = ["//visibility:public"])

go_library(
    name = "indexpack",
    srcs = [
        "fs.go",
        "indexpack.go",
        "zipfs.go",
    ],
    deps = [
        "//kythe/go/platform/analysis",
        "//third_party/go:context",
        "//third_party/go:uuid",
    ],
)
