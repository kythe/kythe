package(default_visibility = ["@//visibility:public"])

load("@//third_party:go/build.bzl", "external_go_package")

licenses(["notice"])

exports_files(["LICENSE"])

external_go_package(
    name = "benchmark/parse",
    base_pkg = "golang.org/x/tools",
)

external_go_package(
    name = "go/gcimporter15",
    base_pkg = "golang.org/x/tools",
    exclude_srcs = [
        "setname16.go",
        "gcimporter16_test.go",
    ],
)
