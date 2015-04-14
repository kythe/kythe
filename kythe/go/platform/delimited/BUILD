load("//tools/build_rules/go", "go_library", "go_test")

package(default_visibility = ["//visibility:public"])

go_library(
    name = "delimited",
    srcs = [
        "delimited.go",
    ],
    deps = [
        "//third_party/go:protobuf",
    ],
)

go_test(
    name = "delimited_test",
    srcs = [
        "delimited_test.go",
    ],
    library = ":delimited",
)
