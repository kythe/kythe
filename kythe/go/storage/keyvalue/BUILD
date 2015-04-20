load("//tools/build_rules/go", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "keyvalue",
    srcs = [
        "keyvalue.go",
    ],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/proto:storage_proto",
    ],
)

go_test(
    name = "keyvalue_test",
    srcs = [
        "keyvalue_test.go",
    ],
    library = ":keyvalue",
    deps = [
        "//kythe/proto:storage_proto",
        "//third_party/go:protobuf",
    ],
)
