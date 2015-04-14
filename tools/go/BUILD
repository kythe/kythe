load("//tools/build_rules/go", "go_binary")

package(default_visibility = ["//visibility:public"])

filegroup(
    name = "go",
    srcs = ["go/bin/go"],
)

go_binary(
    name = "testmain_generator",
    srcs = ["testmain_generator.go"],
)
