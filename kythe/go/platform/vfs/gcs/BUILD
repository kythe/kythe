load("/tools/build_rules/go", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    test_deps = ["//kythe/go/platform/vfs"],
    deps = [
        "//third_party/go/src/golang.org/x/net/context",
        "//third_party/go/src/google.golang.org/cloud/storage",
    ],
)
