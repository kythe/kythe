package(default_visibility = ["@//visibility:public"])

load("@//third_party:go/build.bzl", "external_go_package")

licenses(["notice"])

exports_files(["LICENSE.txt"])

external_go_package(
    name = "diffmatchpatch",
    base_pkg = "github.com/sergi/go-diff",
)
