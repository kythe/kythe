load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "compdb",
    srcs = ["compdb.go"],
    deps = [
        "@org_bitbucket_creachadair_shell//:go_default_library",
        "@org_golang_x_sync//errgroup:go_default_library",
        "@org_golang_x_sync//semaphore:go_default_library",
    ],
)

go_test(
    name = "compdb_test",
    srcs = ["compdb_test.go"],
    data = [
        "testdata/compilation_database.json",
        "testdata/test_file.cc",
        "//kythe/cxx/extractor:cxx_extractor",
    ],
    library = ":compdb",
    rundir = ".",  # Use Bazel conventions for PWD.
    deps = [
        "//kythe/go/platform/kzip",
    ],
)
