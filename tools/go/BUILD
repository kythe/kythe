load("/tools/build_rules/go", "go_binary")

package(default_visibility = ["//visibility:public"])

exports_files(["go"])

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
        "//third_party/go:benchmark_parse",
    ],
)
