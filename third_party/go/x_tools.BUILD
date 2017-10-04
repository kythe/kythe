package(default_visibility = ["@//visibility:public"])

load("@io_kythe//third_party:go/build.bzl", "external_go_package")

licenses(["notice"])

exports_files(["LICENSE"])

external_go_package(
    name = "benchmark/parse",
    base_pkg = "golang.org/x/tools",
)

external_go_package(
    name = "go/gcexportdata",
    base_pkg = "golang.org/x/tools",
    deps = [":go/gcimporter15"],
)

external_go_package(
    name = "go/gcimporter15",
    base_pkg = "golang.org/x/tools",
)

external_go_package(
    name = "go/types/typeutil",
    base_pkg = "golang.org/x/tools",
)
