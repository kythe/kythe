load("/tools/build_rules/go", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/proto:analysis_proto_go",
        "//kythe/proto:storage_proto_go",
        "//third_party/go/src/github.com/golang/protobuf/proto",
        "//third_party/go/src/golang.org/x/net/context",
    ],
)
