load("/tools/build_rules/go", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    test_deps = [
        "//kythe/proto:analysis_proto_go",
        "//kythe/proto:storage_proto_go",
        "//third_party/go/src/golang.org/x/net/context",
        "//third_party/go/src/github.com/golang/protobuf/proto",
    ],
    deps = [
        "//kythe/go/platform/analysis",
        "//kythe/go/platform/vfs",
        "//kythe/go/platform/vfs/zip",
        "//third_party/go/src/github.com/pborman/uuid",
        "//third_party/go/src/golang.org/x/net/context",
    ],
)
