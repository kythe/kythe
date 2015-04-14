load("//tools/build_rules/go", "go_library", "go_test")

package(default_visibility = ["//visibility:public"])

go_library(
    name = "rdf",
    srcs = [
        "rdf.go",
    ],
)

go_test(
    name = "rdf_test",
    srcs = [
        "rdf_test.go",
    ],
    library = ":rdf",
)
