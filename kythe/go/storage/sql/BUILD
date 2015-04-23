load("//tools/build_rules/go", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    test_deps = [
        "//kythe/go/services/graphstore/compare",
        "//kythe/proto:storage_proto",
    ],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/services/xrefs",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema",
        "//kythe/go/util/stringset",
        "//kythe/proto:storage_proto",
        "//kythe/proto:xref_proto",
        "//third_party/go:go-sqlite3",
    ],
)
