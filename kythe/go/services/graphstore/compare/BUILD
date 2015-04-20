load("//tools/build_rules/go", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "compare",
    srcs = [
        "compare.go",
    ],
    deps = [
        "//kythe/proto:storage_proto",
    ],
)

go_test(
    name = "compare_test",
    srcs = [
        "compare_test.go",
    ],
    library = ":compare",
    deps = [
        "//kythe/proto:storage_proto",
    ],
)
