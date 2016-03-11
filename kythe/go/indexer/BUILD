load("//tools:build_rules/go.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "indexer",
    srcs = ["indexer.go"],
    deps = [
        "//kythe/go/extractors/govname",
        "//kythe/proto:analysis_proto_go",
        "//kythe/proto:storage_proto_go",
        "//third_party/go/src/github.com/golang/protobuf/proto",
        "//third_party/go/src/golang.org/x/net/context",
        "//third_party/go/src/golang.org/x/tools/go/gcimporter15",
    ],
)

go_test(
    name = "indexer_test",
    srcs = ["indexer_test.go"],
    # TODO(fromberger): Build this with a library rule.
    data = [":testdata/foo.a"],
    library = ":indexer",
    deps = ["//third_party/go/src/github.com/golang/protobuf/proto"],
)
