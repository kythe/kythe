load("/tools/build_rules/go", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    test_deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/storage/inmemory",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema",
        "//kythe/go/test/testutil",
        "//kythe/proto:storage_proto_go",
        "//kythe/proto:xref_proto_go",
        "//third_party/go/src/golang.org/x/net/context",
    ],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/services/xrefs",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema",
        "//kythe/go/util/stringset",
        "//kythe/proto:storage_proto_go",
        "//kythe/proto:xref_proto_go",
        "//third_party/go/src/golang.org/x/net/context",
    ],
)
