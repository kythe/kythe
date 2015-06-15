load("/tools/build_rules/go", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    test_deps = ["//kythe/go/platform/vfs"],
    deps = [
        "//third_party/go:cloud_storage",
        "//third_party/go:context",
    ],
)
