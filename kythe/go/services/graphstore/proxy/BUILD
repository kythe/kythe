load("//tools/build_rules/go", "go_library")

package(default_visibility = ["//visibility:public"])

go_library(
    name = "proxy",
    srcs = [
        "proxy.go",
    ],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/services/graphstore/compare",
        "//kythe/proto:storage_proto",
    ],
)
