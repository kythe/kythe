load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "kzip",
    srcs = ["kzip.go"],
    deps = [
        "//kythe/go/platform/kcd/kythe",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:buildinfo_go_proto",
        "//kythe/proto:cxx_go_proto",
        "//kythe/proto:filecontext_go_proto",
        "//kythe/proto:go_go_proto",
        "//kythe/proto:java_go_proto",
        "@com_github_golang_protobuf//jsonpb:go_default_library_gen",
        "@com_github_golang_protobuf//proto:go_default_library",
        "@org_bitbucket_creachadair_stringset//:go_default_library",
        "@org_golang_x_sync//errgroup:go_default_library",
    ],
)

go_test(
    name = "kzip_test",
    srcs = ["kzip_test.go"],
    data = [
        "//kythe/cxx/common/testdata:missing-pbunit.kzip",
        "//kythe/cxx/common/testdata:missing-unit.kzip",
    ],
    deps = [
        ":kzip",
        "//kythe/go/test/testutil",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:storage_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
    ],
)
