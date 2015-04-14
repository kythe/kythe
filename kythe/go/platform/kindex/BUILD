load("//tools/build_rules/go", "go_library")

package(default_visibility = ["//visibility:public"])

go_library(
    name = "kindex",
    srcs = [
        "kindex.go",
    ],
    deps = [
        "//kythe/go/platform/analysis",
        "//kythe/go/platform/delimited",
        "//kythe/proto:analysis_proto",
        "//third_party/go:protobuf",
    ],
)
