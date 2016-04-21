package(default_visibility = ["@//visibility:public"])

load("@//tools:build_rules/go.bzl", "go_build")

licenses(["notice"])

exports_files(["LICENSE"])

go_build(
    name = "levigo",
    srcs = glob(["*.go"]),
    cc_deps = ["@//third_party/leveldb"],
    package = "github.com/jmhodges/levigo",
)
