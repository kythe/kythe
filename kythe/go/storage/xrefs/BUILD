load("//tools/build_rules/go", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "xrefs",
    srcs = [
        "xrefs.go",
    ],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/services/xrefs",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema",
        "//kythe/go/util/stringset",
        "//kythe/proto:storage_proto",
        "//kythe/proto:xref_proto",
    ],
)

go_test(
    name = "xrefs_test",
    srcs = [
        "xrefs_test.go",
    ],
    library = ":xrefs",
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/storage/inmemory",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema",
        "//kythe/proto:storage_proto",
        "//kythe/proto:xref_proto",
    ],
)
