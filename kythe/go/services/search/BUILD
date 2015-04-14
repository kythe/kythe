load("//tools/build_rules/go", "go_library")

package(default_visibility = ["//visibility:public"])

go_library(
    name = "search",
    srcs = [
        "search.go",
    ],
    deps = [
        "//kythe/go/services/web",
        "//kythe/proto:storage_proto",
        "//third_party/go:context",
    ],
)
