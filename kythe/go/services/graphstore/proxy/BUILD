load("//tools/build_rules/go", "go_library", "go_test")

package(default_visibility = ["//visibility:public"])

go_library(
    name = "proxy",
    srcs = [
        "proxy.go",
    ],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/services/graphstore/compare",
        "//kythe/proto:storage_proto",
    ],
)

go_test(
    name = "proxy_test",
    srcs = [
        "proxy_test.go",
    ],
    library = ":proxy",
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/proto:storage_proto",
        "//third_party/go:protobuf",
    ],
)
