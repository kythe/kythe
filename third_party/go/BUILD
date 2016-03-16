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
        "//third_party/go/src/bitbucket.org/creachadair/shell:LICENSE",
        "//third_party/go/src/github.com/golang/snappy:LICENSE",
        "//third_party/go/src/github.com/pborman/uuid:LICENSE",
        "//third_party/go/src/golang.org/x/oauth2:LICENSE",
        "//third_party/go/src/google.golang.org/grpc:LICENSE",
        "//third_party/go/src/google.golang.org/cloud:LICENSE",
        "//third_party/go/src/google.golang.org/api/googleapi/internal/uritemplates:LICENSE",
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
