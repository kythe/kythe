load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "http_server",
    srcs = ["http_server/http_server.go"],
    deps = [
        "//kythe/go/services/filetree",
        "//kythe/go/services/graph",
        "//kythe/go/services/web",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/filetree",
        "//kythe/go/serving/graph",
        "//kythe/go/serving/xrefs",
        "//kythe/go/storage/leveldb",
        "//kythe/go/storage/table",
    ],
)

go_binary(
    name = "xrefs_atomizer",
    srcs = ["xrefs_atomizer/xrefs_atomizer.go"],
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/api",
        "//kythe/go/test/services/xrefs",
        "//kythe/go/util/schema/facts",
        "//kythe/proto:storage_go_proto",
        "//kythe/proto:xref_go_proto",
    ],
)

go_binary(
    name = "empty_corpus_checker",
    srcs = ["empty_corpus_checker/empty_corpus_checker.go"],
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/storage/entryset",
        "//kythe/go/storage/stream",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema/facts",
        "//kythe/proto:internal_go_proto",
        "//kythe/proto:storage_go_proto",
        "@org_golang_google_protobuf//proto:go_default_library",
    ],
)
