load("//tools/build_rules/go", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "gsutil",
    srcs = [
        "gsutil.go",
    ],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/services/graphstore/proxy",
        "//kythe/go/storage/inmemory",
        "//kythe/go/storage/leveldb",
        "//kythe/go/storage/sql",
    ],
)
