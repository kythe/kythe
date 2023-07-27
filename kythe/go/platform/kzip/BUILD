load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "kzip",
    srcs = ["kzip.go"],
    importpath = "kythe.io/kythe/go/platform/kzip",
    deps = [
        "//kythe/go/platform/kcd/kythe",
        "//kythe/go/util/log",
        "//kythe/go/util/ptypes",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:buildinfo_go_proto",
        "//kythe/proto:cxx_go_proto",
        "//kythe/proto:filecontext_go_proto",
        "//kythe/proto:go_go_proto",
        "//kythe/proto:java_go_proto",
        "//kythe/proto:storage_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
        "@org_bitbucket_creachadair_stringset//:stringset",
        "@org_golang_google_protobuf//encoding/protojson",
        "@org_golang_x_sync//errgroup",
    ],
)

go_test(
    name = "kzip_test",
    srcs = ["kzip_test.go"],
    data = [
        "//kythe/testdata/platform:missing-pbunit.kzip",
        "//kythe/testdata/platform:missing-unit.kzip",
    ],
    deps = [
        ":kzip",
        "//kythe/go/test/testutil",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:storage_go_proto",
        "@org_golang_google_protobuf//proto",
    ],
)
