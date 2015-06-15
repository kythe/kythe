load("/tools/build_rules/go", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "gotool",
    srcs = [
        "gotool.go",
    ],
    deps = [
        "//kythe/go/extractors/golang",
        "//kythe/go/platform/indexpack",
        "//kythe/proto:analysis_proto_go",
        "//third_party/go:context",
    ],
)
