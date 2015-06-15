load("/tools/build_rules/go", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    test_deps = [
        "//kythe/go/services/graphstore",
        "//kythe/proto:storage_proto_go",
        "//third_party/go:protobuf",
    ],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/services/graphstore/compare",
        "//kythe/go/storage/gsutil",
        "//kythe/proto:storage_proto_go",
        "//third_party/go:context",
    ],
)
