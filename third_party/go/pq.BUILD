package(default_visibility = ["@//visibility:public"])

load("@//third_party:go/build.bzl", "external_go_package")

licenses(["notice"])

exports_files(["LICENSE"])

external_go_package(
    name = "oid",
    base_pkg = "github.com/lib/pq",
    exclude_srcs = ["gen.go"],
)

external_go_package(
    base_pkg = "github.com/lib/pq",
    exclude_srcs = ["*_windows.go"],
    deps = [":oid"],
)
