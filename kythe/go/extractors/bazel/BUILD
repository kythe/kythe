load("//tools:build_rules/go.bzl", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    deps = [
        "//kythe/go/extractors/golang",
        "//kythe/go/extractors/govname",
        "//kythe/go/platform/indexpack",
        "//kythe/go/platform/kindex",
        "//kythe/go/storage/vnameutil",
        "//kythe/proto:analysis_proto_go",
        "//kythe/proto:storage_proto_go",
        "//third_party/bazel:extra_actions_base_proto_go",
        "//third_party/go/src/bitbucket.org/creachadair/shell",
        "//third_party/go/src/github.com/golang/protobuf/proto",
        "//third_party/go/src/golang.org/x/net/context",
    ],
)
