package(default_visibility = ["//visibility:public"])

load("/tools/build_rules/go", "go_binary")

licenses(["notice"])

go_binary(
    name = "protoc-gen-gofast",
    srcs = ["main.go"],
    deps = [
        "//third_party/go/src/github.com/gogo/protobuf/vanity",
        "//third_party/go/src/github.com/gogo/protobuf/vanity/command",
    ],
)
