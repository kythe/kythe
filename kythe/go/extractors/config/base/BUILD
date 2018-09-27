load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

filegroup(
    name = "testdatafiles",
    srcs = glob(["testdata/**"]),
)

go_library(
    name = "base",
    srcs = [
        "base.go",
    ],
    deps = [
        "//kythe/go/extractors/config/base/gradle",
        "//kythe/go/extractors/config/base/mvn",
    ],
)

go_test(
    name = "base_test",
    srcs = ["base_test.go"],
    data = glob(["testdata/**"]),
    library = ":base",
    deps = [
        "//kythe/go/extractors/config/base/gradle",
        "//kythe/go/extractors/config/base/mvn",
        "//kythe/go/test/testutil",
    ],
)
