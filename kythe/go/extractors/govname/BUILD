load("//tools/build_rules/go", "go_library")

package(default_visibility = ["//visibility:public"])

go_library(
    name = "govname",
    srcs = [
        "govname.go",
    ],
    deps = [
        "//kythe/go/storage/vnameutil",
        "//kythe/proto:storage_proto",
    ],
)
