load("//tools/build_rules/go", "go_library", "go_test")

package(default_visibility = ["//visibility:public"])

go_library(
    name = "vnameutil",
    srcs = [
        "rewrite.go",
    ],
    deps = [
        "//kythe/proto:storage_proto",
    ],
)

go_test(
    name = "vnameutil_test",
    srcs = [
        "rewrite_test.go",
    ],
    library = ":vnameutil",
    deps = [
        "//kythe/proto:storage_proto",
        "//third_party/go:protobuf",
    ],
)
