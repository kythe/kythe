load("/tools/build_rules/go", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    test_deps = ["//kythe/go/test/testutil"],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/pager",
        "//kythe/go/util/schema",
        "//kythe/go/util/stringset",
        "//kythe/proto:serving_proto_go",
        "//kythe/proto:storage_proto_go",
        "//third_party/go/src/golang.org/x/net/context",
    ],
)
