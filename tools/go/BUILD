load("//tools:build_rules/go.bzl", "go_binary", "go_library")

package(default_visibility = ["//visibility:public"])

filegroup(
    name = "goroot",
    srcs = ["//external:goroot"],
)

filegroup(
    name = "go",
    srcs = ["//external:gotool"],
)

go_binary(
    name = "testmain_generator",
    srcs = ["testmain_generator.go"],
)

filegroup(
    name = "testmain_srcs",
    srcs = ["testmain.go"],
)

go_binary(
    name = "parse_test_output",
    srcs = ["parse_test_output.go"],
    deps = [
        ":testsuite",
        "@go_x_tools//:benchmark/parse",
    ],
)

go_library(
    name = "testsuite",
    srcs = ["testsuite.go"],
    package = "tools/go/testsuite",
)
