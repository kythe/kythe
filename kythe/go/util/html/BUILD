load("//tools/build_rules/go", "go_library", "go_test")

package(default_visibility = ["//visibility:public"])

go_library(
    name = "html",
    srcs = [
        "html.go",
    ],
    deps = [
        "//third_party/go:html",
    ],
)

go_test(
    name = "html_test",
    srcs = [
        "html_test.go",
    ],
    library = ":html",
    deps = [
        "//third_party/go:html",
    ],
)
