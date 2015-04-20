load("//tools/build_rules/go", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "xrefs",
    srcs = [
        "xrefs.go",
    ],
    deps = [
        "//kythe/go/services/web",
        "//kythe/proto:xref_proto",
        "//third_party/go:context",
    ],
)

go_test(
    name = "xrefs_test",
    srcs = [
        "xrefs_test.go",
    ],
    library = ":xrefs",
    deps = [
        "//kythe/go/util/schema",
    ],
)
