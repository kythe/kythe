load("//tools/build_rules/go", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

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

go_test(
    name = "indexpack_test",
    srcs = [
        "indexpack_test.go",
    ],
    library = ":indexpack",
    deps = [
        "//kythe/proto:analysis_proto",
        "//kythe/proto:storage_proto",
        "//third_party/go:context",
        "//third_party/go:protobuf",
    ],
)
