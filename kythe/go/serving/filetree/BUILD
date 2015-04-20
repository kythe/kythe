load("//tools/build_rules/go", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "filetree",
    srcs = [
        "filetree.go",
    ],
    deps = [
        "//kythe/go/storage/table",
        "//kythe/proto:filetree_proto",
        "//kythe/proto:serving_proto",
    ],
)
