load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "empty_corpus_checker",
    srcs = ["empty_corpus_checker.go"],
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/storage/entryset",
        "//kythe/go/storage/stream",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema/facts",
        "//kythe/proto:internal_go_proto",
        "//kythe/proto:storage_go_proto",
        "@org_bitbucket_creachadair_stringset//:go_default_library",
        "@org_golang_google_protobuf//proto:go_default_library",
    ],
)

exports_files(["empty_corpus_test.sh"])
