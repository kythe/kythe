load("//tools/build_rules/go", "go_library", "go_test")

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

go_test(
    name = "kindex_test",
    srcs = [
        "kindex_test.go",
    ],
    library = ":kindex",
    deps = [
        "//kythe/proto:analysis_proto",
        "//kythe/proto:storage_proto",
        "//third_party/go:protobuf",
    ],
)
