load("//tools/build_rules/go", "go_library")

package(default_visibility = ["//visibility:public"])

go_library(
    name = "search",
    srcs = [
        "search.go",
    ],
    deps = [
        "//kythe/go/storage/table",
        "//kythe/go/util/kytheuri",
        "//kythe/proto:serving_proto",
        "//kythe/proto:storage_proto",
    ],
)
