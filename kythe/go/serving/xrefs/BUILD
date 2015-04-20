load("//tools/build_rules/go", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "xrefs",
    srcs = [
        "xrefs.go",
    ],
    deps = [
        "//kythe/go/services/xrefs",
        "//kythe/go/storage/table",
        "//kythe/go/util/stringset",
        "//kythe/proto:serving_proto",
        "//kythe/proto:xref_proto",
        "//third_party/go:protobuf",
    ],
)
