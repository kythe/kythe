load("//tools/build_rules/go", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "filetree",
    srcs = [
        "filetree.go",
    ],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/services/web",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema",
        "//kythe/proto:filetree_proto",
        "//kythe/proto:serving_proto",
        "//kythe/proto:storage_proto",
        "//third_party/go:context",
    ],
)
