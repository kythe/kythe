load("//tools/build_rules/go", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "golang",
    srcs = [
        "golang.go",
    ],
    deps = [
        "//kythe/go/extractors/govname",
        "//kythe/go/platform/indexpack",
        "//kythe/proto:analysis_proto",
        "//kythe/proto:storage_proto",
        "//third_party/go:context",
    ],
)
