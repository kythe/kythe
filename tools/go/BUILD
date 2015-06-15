load("/tools/build_rules/go", "go_binary")

package(default_visibility = ["//visibility:public"])

exports_files(["go"])

go_binary(
    name = "testmain_generator",
    srcs = ["testmain_generator.go"],
)
