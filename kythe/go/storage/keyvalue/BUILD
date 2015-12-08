load("/tools/build_rules/go", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    test_deps = [
        "//kythe/proto:storage_proto_go",
        "//third_party/go/src/github.com/golang/protobuf/proto",
    ],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/util/datasize",
        "//kythe/proto:storage_proto_go",
        "//third_party/go/src/golang.org/x/net/context",
    ],
)
