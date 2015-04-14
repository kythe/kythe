load("//tools/build_rules/go", "go_library")

package(default_visibility = ["//visibility:public"])

go_library(
    name = "web",
    srcs = [
        "web.go",
    ],
    deps = [
        "//kythe/go/util/httpencoding",
        "//third_party/go:protobuf",
    ],
)
