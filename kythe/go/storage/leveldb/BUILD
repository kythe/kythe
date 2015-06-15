load("/tools/build_rules/go", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    test_deps = [
        "//kythe/go/test/services/graphstore",
        "//kythe/go/test/storage/keyvalue",
    ],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/storage/gsutil",
        "//kythe/go/storage/keyvalue",
        "//third_party/go:levigo",
    ],
)
