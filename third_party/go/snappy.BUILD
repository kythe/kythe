package(default_visibility = ["@//visibility:public"])

load("@//third_party:go/build.bzl", "external_go_package")

licenses(["notice"])

exports_files(["LICENSE"])

external_go_package(
    base_pkg = "github.com/golang/snappy",
    exclude_srcs = [
        "decode_amd64.go",
        "encode_amd64.go",
    ],
)
