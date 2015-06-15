load("/tools/build_rules/go", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    deps = [
        "//kythe/go/services/xrefs",
        "//kythe/go/storage/table",
        "//kythe/go/util/stringset",
        "//kythe/proto:serving_proto_go",
        "//kythe/proto:xref_proto_go",
        "//third_party/go:context",
        "//third_party/go:protobuf",
    ],
)
