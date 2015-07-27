load("/tools/build_rules/go", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    test_deps = [
        "//kythe/go/test/testutil",
        "//kythe/proto:storage_proto_go",
    ],
    deps = [
        "//kythe/go/platform/analysis",
        "//kythe/proto:analysis_proto_go",
        "//third_party/go:context",
    ],
)
