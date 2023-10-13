load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "compdb",
    srcs = ["compdb.go"],
    importpath = "kythe.io/kythe/go/extractors/config/runextractor/compdb",
    deps = [
        "//kythe/go/util/log",
        "@org_bitbucket_creachadair_shell//:shell",
        "@org_golang_x_sync//semaphore",
    ],
)

go_test(
    name = "compdb_test",
    srcs = ["compdb_test.go"],
    data = [
        "testdata/compilation_database.json",
        "testdata/compilation_database_arguments.json",
        "testdata/test_file.cc",
        "//kythe/cxx/extractor:cxx_extractor",
    ],
    library = ":compdb",
    rundir = ".",  # Use Bazel conventions for PWD.
    deps = [
        "//kythe/go/platform/kzip",
    ],
)
