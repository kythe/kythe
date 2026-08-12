load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "pebble",
    srcs = ["pebble.go"],
    importpath = "kythe.io/kythe/go/storage/pebble",
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/storage/gsutil",
        "//kythe/go/storage/keyvalue",
        "@com_github_cockroachdb_pebble//:pebble",
        "@com_github_cockroachdb_pebble//vfs",
    ],
)

go_test(
    name = "pebble_test",
    srcs = ["pebble_test.go"],
    library = ":pebble",
    deps = [
        "//kythe/go/test/services/graphstore",
        "//kythe/go/test/storage/keyvalue",
        "@com_github_google_go_cmp//cmp",
    ],
)
