load("//tools/build_rules/go", "go_library", "go_test")

package(default_visibility = ["//visibility:public"])

go_library(
    name = "sql",
    srcs = [
        "sql.go",
        "sql_xrefs.go",
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

go_test(
    name = "sql_test",
    srcs = [
        "sql_test.go",
    ],
    library = ":sql",
    deps = [
        "//kythe/go/services/graphstore/compare",
        "//kythe/proto:storage_proto",
    ],
)
