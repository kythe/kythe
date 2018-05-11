package(default_visibility = ["@//visibility:public"])

load("@//third_party:go/build.bzl", "external_go_package")

licenses(["notice"])

exports_files(["LICENSE"])

# See: http://godoc.org/golang.org/x/sync/errgroup
external_go_package(
    name = "errgroup",
    base_pkg = "golang.org/x/sync",
    deps = ["@go_x_net//:context"],
)

external_go_package(
    name = "semaphore",
    base_pkg = "golang.org/x/sync",
    deps = ["@go_x_net//:context"],
)
