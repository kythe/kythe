load("//tools/build_rules/go", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "google",
    srcs = [
        "fs.go",
    ],
    deps = [
        "//third_party/go:cloud_storage",
        "//third_party/go:context",
    ],
)

go_test(
    name = "google_test",
    srcs = [
        "fs_test.go",
    ],
    library = ":google",
)
