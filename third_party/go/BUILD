package(default_visibility = ["//visibility:public"])

load("@//tools:build_rules/go.bzl", "go_package")

licenses(["notice"])

filegroup(
    name = "licenses",
    srcs = glob(
        [
            "**/*LICENSE*",
            "**/COPYING*",
            "**/license*",
            "**/copying*",
        ],
    ) + [
        "@go_snappy//:LICENSE",
        "@go_uuid//:LICENSE",
        "@go_gapi//:LICENSE",
        "@go_gcloud//:LICENSE",
        "@go_grpc//:LICENSE",
        "@go_shell//:LICENSE",
        "@go_x_net//:LICENSE",
        "@go_x_oauth2//:LICENSE",
        "@go_x_text//:LICENSE",
        "@go_x_tools//:LICENSE",
    ],
)

go_package(
    name = "levigo",
    srcs = "src/github.com/jmhodges/levigo",
    cc_deps = ["//third_party/leveldb"],
    go_build = 1,
    package = "github.com/jmhodges/levigo",
    tests = 0,
)
