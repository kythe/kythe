load("//tools/build_rules/go", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

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

go_test(
    name = "kytheuri_test",
    srcs = [
        "uri_test.go",
    ],
    library = ":kytheuri",
)
