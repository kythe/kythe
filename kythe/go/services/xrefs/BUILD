load("//tools/build_rules/go", "go_library")

package(default_visibility = ["//visibility:public"])

go_library(
    name = "xrefs",
    srcs = [
        "xrefs.go",
    ],
    deps = [
        "//kythe/go/services/web",
        "//kythe/proto:xref_proto",
        "//third_party/go:context",
    ],
)
