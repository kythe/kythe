load("/tools/build_rules/go", "go_binary", "go_library")

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
        "//third_party/go/src/golang.org/x/tools/benchmark/parse",
    ],
)

go_library(
    name = "testsuite",
    srcs = ["testsuite.go"],
    package = "tools/go/testsuite",
)
