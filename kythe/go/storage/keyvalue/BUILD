load("//tools/build_rules/go", "go_library")

package(default_visibility = ["//visibility:public"])

go_library(
    name = "keyvalue",
    srcs = [
        "keyvalue.go",
    ],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/proto:storage_proto",
    ],
)
